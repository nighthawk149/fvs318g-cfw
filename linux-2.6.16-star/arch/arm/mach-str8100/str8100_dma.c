/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_dmac.h>

//#define DMA_DEBUG
#define STR8100_DMA_TEST

#define DMA_CHANNEL_MAX_ID		6 // channel 0 ~ 6

#define DMA_XFER_MAX_Q_LEN		32
#define DMA_BUSY_MAX_Q_LEN		7

#define ADDR_16BIT_ALIGN_MASK		0x1
#define ADDR_32BIT_ALIGN_MASK		0x3

#define DMA_LEN_SHIFT_8BIT_WIDTH	0
#define DMA_LEN_SHIFT_16BIT_WIDTH	1
#define DMA_LEN_SHIFT_32BIT_WIDTH	2
#define DMA_MAX_LEN_8BIT_WIDTH		0xfff
#define DMA_MAX_LEN_16BIT_WIDTH		(0xfff << 1)
#define DMA_MAX_LEN_32BIT_WIDTH		(0xfff << 2)

#define DMA_COPY_BUSY_WAIT_CHANNEL	7
#define DMA_COPY_BUSY_WAIT_LOOP		1000

static u32 dma_burst_size = DMAC_CH_SRC_BURST_SIZE_32;

#define DEFAULT_CH_PRIORITY		DMAC_CH_PRIORITY_3
#define DEFAULT_CH_BURST_SIZE		DMAC_CH_SRC_BURST_SIZE_128
#define DEFAULT_CH_SRC_WIDTH		DMAC_CH_SRC_WIDTH_8BIT
#define DEFAULT_CH_DST_WIDTH		DMAC_CH_SRC_WIDTH_8BIT
#define DEFAULT_CH_MODE			DMAC_CH_MODE_NORMAL
#define DEFAULT_CH_SRC_ADDR_CTL		DMAC_CH_SRC_ADDR_CTL_INC
#define DEFAULT_CH_DST_ADDR_CTL		DMAC_CH_DST_ADDR_CTL_INC
#define DEFAULT_CH_DST_SEL		DMAC_DST_SEL_MASTER0
#define DEFAULT_CH_SRC_SEL		DMAC_SRC_SEL_MASTER0

#define DEFAULT_DMA_CH_CTL \
	((0xf				<< DMAC_CH_HHST_SEL_BIT_INDEX) | \
	 (DEFAULT_CH_PRIORITY		<< DMAC_CH_PRIORITY_BIT_INDEX) | \
	 (DEFAULT_CH_BURST_SIZE		<< DMAC_CH_SRC_BURST_SIZE_BIT_INDEX) | \
	 (DEFAULT_CH_SRC_WIDTH		<< DMAC_CH_SRC_WIDTH_BIT_INDEX) | \
	 (DEFAULT_CH_DST_WIDTH		<< DMAC_CH_DST_WIDTH_BIT_INDEX) | \
	 (DEFAULT_CH_MODE		<< DMAC_CH_MODE_BIT_INDEX) | \
	 (DEFAULT_CH_SRC_ADDR_CTL	<< DMAC_CH_SRC_ADDR_CTL_BIT_INDEX) | \
	 (DEFAULT_CH_DST_ADDR_CTL	<< DMAC_CH_DST_ADDR_CTL_BIT_INDEX) | \
	 (DEFAULT_CH_DST_SEL		<< DMAC_CH_DST_SEL_BIT_INDEX) | \
	 (DEFAULT_CH_SRC_SEL		<< DMAC_CH_SRC_SEL_BIT_INDEX))

#define DMA_CH_CTL_8BIT_WIDTH \
	((0xf				<< DMAC_CH_HHST_SEL_BIT_INDEX) | \
	 (DMAC_CH_PRIORITY_3		<< DMAC_CH_PRIORITY_BIT_INDEX) | \
	 (dma_burst_size		<< DMAC_CH_SRC_BURST_SIZE_BIT_INDEX) | \
	 (DMAC_CH_SRC_WIDTH_8BIT	<< DMAC_CH_SRC_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_DST_WIDTH_8BIT	<< DMAC_CH_DST_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_MODE_NORMAL		<< DMAC_CH_MODE_BIT_INDEX) | \
	 (DMAC_CH_SRC_ADDR_CTL_INC	<< DMAC_CH_SRC_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_CH_DST_ADDR_CTL_INC	<< DMAC_CH_DST_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_DST_SEL_MASTER0		<< DMAC_CH_DST_SEL_BIT_INDEX) | \
	 (DMAC_SRC_SEL_MASTER1		<< DMAC_CH_SRC_SEL_BIT_INDEX))

#define DMA_CH_CTL_16BIT_WIDTH \
	((0xf				<< DMAC_CH_HHST_SEL_BIT_INDEX) | \
	 (DMAC_CH_PRIORITY_3		<< DMAC_CH_PRIORITY_BIT_INDEX) | \
	 (dma_burst_size		<< DMAC_CH_SRC_BURST_SIZE_BIT_INDEX) | \
	 (DMAC_CH_SRC_WIDTH_16BIT	<< DMAC_CH_SRC_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_DST_WIDTH_16BIT	<< DMAC_CH_DST_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_MODE_NORMAL		<< DMAC_CH_MODE_BIT_INDEX) | \
	 (DMAC_CH_SRC_ADDR_CTL_INC	<< DMAC_CH_SRC_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_CH_DST_ADDR_CTL_INC	<< DMAC_CH_DST_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_DST_SEL_MASTER0		<< DMAC_CH_DST_SEL_BIT_INDEX) | \
	 (DMAC_SRC_SEL_MASTER1		<< DMAC_CH_SRC_SEL_BIT_INDEX))


#define DMA_CH_CTL_32BIT_WIDTH \
	((0xf				<< DMAC_CH_HHST_SEL_BIT_INDEX) | \
	 (DMAC_CH_PRIORITY_3		<< DMAC_CH_PRIORITY_BIT_INDEX) | \
	 (dma_burst_size		<< DMAC_CH_SRC_BURST_SIZE_BIT_INDEX) | \
	 (DMAC_CH_SRC_WIDTH_32BIT	<< DMAC_CH_SRC_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_DST_WIDTH_32BIT	<< DMAC_CH_DST_WIDTH_BIT_INDEX) | \
	 (DMAC_CH_MODE_NORMAL		<< DMAC_CH_MODE_BIT_INDEX) | \
	 (DMAC_CH_SRC_ADDR_CTL_INC	<< DMAC_CH_SRC_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_CH_DST_ADDR_CTL_INC	<< DMAC_CH_DST_ADDR_CTL_BIT_INDEX) | \
	 (DMAC_DST_SEL_MASTER0		<< DMAC_CH_DST_SEL_BIT_INDEX) | \
	 (DMAC_SRC_SEL_MASTER1		<< DMAC_CH_SRC_SEL_BIT_INDEX))

typedef struct
{
	struct list_head	lh;
	dma_xfer_t		*xfer;
	dma_llp_descr_t		*llp_descr;
	u32			llp_descr_dma;
	int			done_status;
} dma_job_t;


static u8 dma_dev;
static spinlock_t dma_lock;
static u8 dma_busy;
static u8 dma_busy_q_len;
static unsigned int dma_xfer_q_len;
static unsigned int dma_done_q_len;
static struct list_head dma_xfer_q;
static struct list_head dma_done_q;
static dma_job_t *dma_running_job[DMA_CHANNEL_MAX_ID + 1];

static void *dma_mem_pool;
static u32 dma_mem_pool_dma;

static dma_job_t dma_job_pool[DMA_XFER_MAX_Q_LEN];
static struct list_head dma_job_q;
static spinlock_t dma_job_q_lock;

static void dma_process_xfer_job(void *data);
static void dma_process_done_job(void *data);

static DECLARE_WORK(dma_xfer_task, dma_process_xfer_job, (void *)&dma_xfer_q);
static DECLARE_WORK(dma_done_task, dma_process_done_job, (void *)&dma_done_q);

static int dma_job_q_init(void)
{
	int i;

	dma_mem_pool = (void *)pci_alloc_consistent(NULL,
		(DMA_XFER_MAX_Q_LEN * MAX_DMA_VEC * sizeof(dma_llp_descr_t)),
	       	&dma_mem_pool_dma);

	if (dma_mem_pool == NULL) {
		return -1;
	}

	INIT_LIST_HEAD(&dma_job_q);
	for (i = 0; i < DMA_XFER_MAX_Q_LEN; i++) {
		INIT_LIST_HEAD(&dma_job_pool[i].lh);
		dma_job_pool[i].llp_descr = (dma_llp_descr_t *)(dma_mem_pool + (i * (MAX_DMA_VEC * sizeof(dma_llp_descr_t))));
		dma_job_pool[i].llp_descr_dma = dma_mem_pool_dma + (i * (MAX_DMA_VEC * sizeof(dma_llp_descr_t)));
		list_add_tail(&dma_job_pool[i].lh, &dma_job_q);
	}

	return 0;
}

static dma_job_t *dma_job_alloc(void)
{
	dma_job_t *job;
	unsigned long flags;

	spin_lock_irqsave(&dma_job_q_lock, flags);
	if (list_empty(&dma_job_q)) {
		job = NULL;
	} else {
		job = list_entry(dma_job_q.next, dma_job_t, lh);
		list_del_init(&job->lh);
	}
	spin_unlock_irqrestore(&dma_job_q_lock, flags);

	return job;
}

static void dma_job_free(dma_job_t *job)
{
	unsigned long flags;

	spin_lock_irqsave(&dma_job_q_lock, flags);
	list_add(&job->lh, &dma_job_q);
	spin_unlock_irqrestore(&dma_job_q_lock, flags);
}

#ifdef DMA_DEBUG
void dma_dump_reg(void)
{
	printk("DMAC_BASE_ADDR+0x000: 0x%08x\n", DMAC_INT_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x004: 0x%08x\n", DMAC_INT_TC_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x008: 0x%08x\n", DMAC_INT_TC_STATUS_CLR_REG);
	printk("DMAC_BASE_ADDR+0x00C: 0x%08x\n", DMAC_INT_ERR_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x010: 0x%08x\n", DMAC_INT_ERR_STATUS_CLR_REG);
	printk("DMAC_BASE_ADDR+0x014: 0x%08x\n", DMAC_TC_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x018: 0x%08x\n", DMAC_ERR_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x01C: 0x%08x\n", DMAC_CH_ENABLE_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x020: 0x%08x\n", DMAC_CH_BUSY_STATUS_REG);
	printk("DMAC_BASE_ADDR+0x024: 0x%08x\n", DMAC_CSR_REG);
	printk("DMAC_BASE_ADDR+0x028: 0x%08x\n", DMAC_SYNC_REG);
	printk("DMAC_BASE_ADDR+0x100: 0x%08x\n", DMAC_CH0_CSR_REG);
	printk("DMAC_BASE_ADDR+0x104: 0x%08x\n", DMAC_CH0_CFG_REG);
	printk("DMAC_BASE_ADDR+0x108: 0x%08x\n", DMAC_CH0_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x10C: 0x%08x\n", DMAC_CH0_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x110: 0x%08x\n", DMAC_CH0_LLP_REG);
	printk("DMAC_BASE_ADDR+0x114: 0x%08x\n", DMAC_CH0_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x120: 0x%08x\n", DMAC_CH1_CSR_REG);
	printk("DMAC_BASE_ADDR+0x124: 0x%08x\n", DMAC_CH1_CFG_REG);
	printk("DMAC_BASE_ADDR+0x128: 0x%08x\n", DMAC_CH1_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x12C: 0x%08x\n", DMAC_CH1_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x130: 0x%08x\n", DMAC_CH1_LLP_REG);
	printk("DMAC_BASE_ADDR+0x134: 0x%08x\n", DMAC_CH1_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x140: 0x%08x\n", DMAC_CH2_CSR_REG);
	printk("DMAC_BASE_ADDR+0x144: 0x%08x\n", DMAC_CH2_CFG_REG);
	printk("DMAC_BASE_ADDR+0x148: 0x%08x\n", DMAC_CH2_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x14C: 0x%08x\n", DMAC_CH2_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x150: 0x%08x\n", DMAC_CH2_LLP_REG);
	printk("DMAC_BASE_ADDR+0x154: 0x%08x\n", DMAC_CH2_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x160: 0x%08x\n", DMAC_CH3_CSR_REG);
	printk("DMAC_BASE_ADDR+0x164: 0x%08x\n", DMAC_CH3_CFG_REG);
	printk("DMAC_BASE_ADDR+0x168: 0x%08x\n", DMAC_CH3_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x16C: 0x%08x\n", DMAC_CH3_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x170: 0x%08x\n", DMAC_CH3_LLP_REG);
	printk("DMAC_BASE_ADDR+0x174: 0x%08x\n", DMAC_CH3_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x180: 0x%08x\n", DMAC_CH4_CSR_REG);
	printk("DMAC_BASE_ADDR+0x184: 0x%08x\n", DMAC_CH4_CFG_REG);
	printk("DMAC_BASE_ADDR+0x188: 0x%08x\n", DMAC_CH4_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x18C: 0x%08x\n", DMAC_CH4_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x190: 0x%08x\n", DMAC_CH4_LLP_REG);
	printk("DMAC_BASE_ADDR+0x194: 0x%08x\n", DMAC_CH4_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x1A0: 0x%08x\n", DMAC_CH5_CSR_REG);
	printk("DMAC_BASE_ADDR+0x1A4: 0x%08x\n", DMAC_CH5_CFG_REG);
	printk("DMAC_BASE_ADDR+0x1A8: 0x%08x\n", DMAC_CH5_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1AC: 0x%08x\n", DMAC_CH5_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1B0: 0x%08x\n", DMAC_CH5_LLP_REG);
	printk("DMAC_BASE_ADDR+0x1B4: 0x%08x\n", DMAC_CH5_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x1C0: 0x%08x\n", DMAC_CH6_CSR_REG);
	printk("DMAC_BASE_ADDR+0x1C4: 0x%08x\n", DMAC_CH6_CFG_REG);
	printk("DMAC_BASE_ADDR+0x1C8: 0x%08x\n", DMAC_CH6_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1CC: 0x%08x\n", DMAC_CH6_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1D0: 0x%08x\n", DMAC_CH6_LLP_REG);
	printk("DMAC_BASE_ADDR+0x1D4: 0x%08x\n", DMAC_CH6_SIZE_REG);
	printk("DMAC_BASE_ADDR+0x1E0: 0x%08x\n", DMAC_CH7_CSR_REG);
	printk("DMAC_BASE_ADDR+0x1E4: 0x%08x\n", DMAC_CH7_CFG_REG);
	printk("DMAC_BASE_ADDR+0x1E8: 0x%08x\n", DMAC_CH7_SRC_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1EC: 0x%08x\n", DMAC_CH7_DST_ADDR_REG);
	printk("DMAC_BASE_ADDR+0x1F0: 0x%08x\n", DMAC_CH7_LLP_REG);
	printk("DMAC_BASE_ADDR+0x1F4: 0x%08x\n", DMAC_CH7_SIZE_REG);
}
#endif

static inline void dma_copy_busy_wait(void *dst, const void *src, size_t len, unsigned long dma_len_shift)
{
	int err = 0;
	int i;

	len = (len >> dma_len_shift);

	while ((DMAC_CH_BUSY_STATUS_REG & 0xFF) & (1 << DMA_COPY_BUSY_WAIT_CHANNEL)) {
		udelay(1);
	}

	DMAC_CH_SRC_ADDR_REG(DMA_COPY_BUSY_WAIT_CHANNEL)	= (u32)virt_to_phys((void *)src);
	DMAC_CH_DST_ADDR_REG(DMA_COPY_BUSY_WAIT_CHANNEL)	= (u32)virt_to_phys((void *)dst);
	DMAC_CH_SIZE_REG(DMA_COPY_BUSY_WAIT_CHANNEL)		= (u32)len;

#ifdef DMA_DEBUG
	dma_dump_reg();
#endif

	// enable the channel
	DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) |= 0x1;

	for (i = 0; i < DMA_COPY_BUSY_WAIT_LOOP; i++) {
		if (DMAC_TC_STATUS_REG & (1 << DMA_COPY_BUSY_WAIT_CHANNEL)) {
			break;
		}
		if (DMAC_ERR_STATUS_REG & (1 << DMA_COPY_BUSY_WAIT_CHANNEL)) {
			err = 1;
			break;
		}
		udelay(1);
	}

	// disable the channel
	DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) &= ~0x1;

	if (err || (i == DMA_COPY_BUSY_WAIT_LOOP)) {
		if (err) {
			// clear the ERROR status
			DMAC_INT_ERR_STATUS_CLR_REG |= (1 << DMA_COPY_BUSY_WAIT_CHANNEL);
		}
		memcpy(dst, src, (len << dma_len_shift));
	} else {
		// clear the TC status
		DMAC_INT_TC_STATUS_CLR_REG |= (1 << DMA_COPY_BUSY_WAIT_CHANNEL);
	}
}

void dma_memcpy_busy_wait(void *dst, const void *src, size_t len)
{
	void *pdst = dst;
	const void *psrc = src;
	unsigned long dma_len_shift;
	unsigned long dma_max_len;
	unsigned long flags;

#ifdef DMA_DEBUG
	printk("dst: 0x%08x\n", (u32)dst);
	printk("src: 0x%08x\n", (u32)src);
	printk("len: 0x%08x\n", (u32)len);
	printk("pdst: 0x%08x\n", (u32)pdst);
	printk("psrc: 0x%08x\n", (u32)psrc);
#endif

	local_irq_save(flags);

	consistent_sync((void *)src, len, PCI_DMA_TODEVICE);
	consistent_sync((void *)dst, len, PCI_DMA_FROMDEVICE);

	if ((((u32)psrc & ADDR_32BIT_ALIGN_MASK) == 0) &&
		(((u32)pdst & ADDR_32BIT_ALIGN_MASK) == 0) &&
		((len & 3) == 0)) {
		dma_len_shift = DMA_LEN_SHIFT_32BIT_WIDTH;
		dma_max_len = DMA_MAX_LEN_32BIT_WIDTH;
		DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) = DMA_CH_CTL_32BIT_WIDTH;
	} else if ((((u32)psrc & ADDR_16BIT_ALIGN_MASK) == 0) &&
		(((u32)pdst & ADDR_16BIT_ALIGN_MASK) == 0) &&
		((len & 1) == 0)) {
		dma_len_shift = DMA_LEN_SHIFT_16BIT_WIDTH;
		dma_max_len = DMA_MAX_LEN_16BIT_WIDTH;
		DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) = DMA_CH_CTL_16BIT_WIDTH;
	} else {
		dma_len_shift = DMA_LEN_SHIFT_8BIT_WIDTH;
		dma_max_len = DMA_MAX_LEN_8BIT_WIDTH;
		DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) = DMA_CH_CTL_8BIT_WIDTH;
	}

	while (len) {
		if (len > dma_max_len) {
			dma_copy_busy_wait(pdst, psrc, dma_max_len, dma_len_shift);
			pdst += dma_max_len;
			psrc += dma_max_len;
			len -= dma_max_len;
		} else {
			dma_copy_busy_wait(pdst, psrc, len, dma_len_shift);
			len = 0;
		}
	}

	local_irq_restore(flags);
}
EXPORT_SYMBOL(dma_memcpy_busy_wait);

void dma_copy(dma_xfer_t *dma_xfer)
{
	dma_job_t *dma_job;
	u32 size_shift = 0;
	unsigned long flags;
	int i;

	if (dma_xfer_q_len > DMA_XFER_MAX_Q_LEN) {
		dma_xfer->dma_end_io(dma_xfer, DMAC_RESPONSE_ERR);
		return;
	}

	dma_job = dma_job_alloc();
	if (dma_job == NULL) {
		dma_xfer->dma_end_io(dma_xfer, DMAC_RESPONSE_ERR);
		return;
	}

	memset(dma_job->llp_descr, 0, (dma_xfer->nr_vec * sizeof(dma_llp_descr_t)));
	for (i = 0; i < dma_xfer->nr_vec; i++) {
		consistent_sync((void *)dma_xfer->vec[i].src_addr, dma_xfer->vec[i].size, PCI_DMA_TODEVICE);
		consistent_sync((void *)dma_xfer->vec[i].dst_addr, dma_xfer->vec[i].size, PCI_DMA_FROMDEVICE);
		dma_job->llp_descr[i].src_addr = (u32)virt_to_phys((void *)dma_xfer->vec[i].src_addr);
		dma_job->llp_descr[i].dst_addr = (u32)virt_to_phys((void *)dma_xfer->vec[i].dst_addr);
		dma_job->llp_descr[i].dst_sel = dma_xfer->vec[i].dst_sel;
		dma_job->llp_descr[i].src_sel = dma_xfer->vec[i].src_sel;
		dma_job->llp_descr[i].dst_addr_ctl = dma_xfer->vec[i].dst_sel;
		dma_job->llp_descr[i].src_addr_ctl = dma_xfer->vec[i].src_sel;
		dma_job->llp_descr[i].dst_width = dma_xfer->vec[i].dst_width;
		dma_job->llp_descr[i].src_width = dma_xfer->vec[i].src_width;
		if (dma_xfer->vec[i].src_width == DMAC_CH_DST_WIDTH_32BIT) {
			size_shift = 2;
		} else if (dma_xfer->vec[i].src_width == DMAC_CH_DST_WIDTH_16BIT) {
			size_shift = 1;
		} else {
			size_shift = 0;
		}
		dma_job->llp_descr[i].tot_size = (dma_xfer->vec[i].size >> size_shift) & 0xFFF;
		if (i == (dma_xfer->nr_vec - 1)) {
			dma_job->llp_descr[i].llp = 0;
			dma_job->llp_descr[i].tc_mask = 0;
		} else {
			dma_job->llp_descr[i].llp = (u32)(dma_job->llp_descr_dma + ((i + 1) * sizeof(dma_llp_descr_t)));
			dma_job->llp_descr[i].tc_mask = 1;
		}
#ifdef DMA_DEBUG
		printk("in src_addr:  0x%08x\n", dma_xfer->vec[i].src_addr);
		printk("in dst_addr:  0x%08x\n", dma_xfer->vec[i].dst_addr);
		printk("src_addr:     0x%08x\n", dma_job->llp_descr[i].src_addr);
		printk("dst_addr:     0x%08x\n", dma_job->llp_descr[i].dst_addr);
		printk("llp:          0x%08x\n", dma_job->llp_descr[i].llp);
		printk("tot_size:     0x%08x\n", dma_job->llp_descr[i].tot_size);
		printk("dst_sel:      0x%08x\n", dma_job->llp_descr[i].dst_sel);
		printk("src_sel:      0x%08x\n", dma_job->llp_descr[i].src_sel);
		printk("dst_addr_ctl: 0x%08x\n", dma_job->llp_descr[i].dst_addr_ctl);
		printk("src_addr_ctl: 0x%08x\n", dma_job->llp_descr[i].src_addr_ctl);
		printk("dst_width:    0x%08x\n", dma_job->llp_descr[i].dst_width);
		printk("src_width:    0x%08x\n", dma_job->llp_descr[i].src_width);
		printk("tc_mask:      0x%08x\n", dma_job->llp_descr[i].tc_mask);
#endif
	}

	dma_job->xfer = dma_xfer;
	dma_job->done_status = 0;

	spin_lock_irqsave(&dma_lock, flags);
	list_add_tail(&dma_job->lh, &dma_xfer_q);
	dma_xfer_q_len++;
	spin_unlock_irqrestore(&dma_lock, flags);

	if (!dma_busy) {
		schedule_work(&dma_xfer_task);
	}
}
EXPORT_SYMBOL(dma_copy);

static void dma_process_xfer_job(void *data)
{
	struct list_head *l, *t;
	dma_job_t *dma_job;
	unsigned long csr_reg;
	int i;
	unsigned long flags;

	if (dma_busy) {
		return;
	}

	spin_lock_irqsave(&dma_lock, flags);
	list_for_each_safe(l, t, &dma_xfer_q) {
		dma_job = list_entry(l, dma_job_t, lh);
		for (i = 0; i <= 6; i++) {
			if (dma_running_job[i] != NULL) {
				continue;
			}
			printk("Insert dma xfer to channel(%d)\n", i);
			list_del_init(&dma_job->lh);
			dma_running_job[i] = dma_job;
			dma_xfer_q_len--;
			dma_busy_q_len++;
			csr_reg = DMAC_CH_CSR_REG(i);
			if (dma_job->llp_descr[0].tc_mask) {
				csr_reg |= (1 << 31);
			} else {
				csr_reg &= ~(1 << 31);
			}
			csr_reg &= ~(DMAC_CH_SRC_WIDTH_MASK << DMAC_CH_SRC_WIDTH_BIT_INDEX);
			csr_reg &= ~(DMAC_CH_DST_WIDTH_MASK << DMAC_CH_DST_WIDTH_BIT_INDEX);
			csr_reg &= ~(DMAC_CH_SRC_ADDR_CTL_MASK << DMAC_CH_SRC_ADDR_CTL_BIT_INDEX);
			csr_reg &= ~(DMAC_CH_DST_ADDR_CTL_MASK << DMAC_CH_DST_ADDR_CTL_BIT_INDEX);
			csr_reg |=
				((dma_job->llp_descr[0].src_width << DMAC_CH_SRC_WIDTH_BIT_INDEX) |
				 (dma_job->llp_descr[0].dst_width << DMAC_CH_DST_WIDTH_BIT_INDEX) |
				 (dma_job->llp_descr[0].src_addr_ctl << DMAC_CH_SRC_ADDR_CTL_BIT_INDEX) |
				 (dma_job->llp_descr[0].dst_addr_ctl << DMAC_CH_DST_ADDR_CTL_BIT_INDEX) |
				 (dma_job->llp_descr[0].src_sel << DMAC_CH_SRC_SEL_BIT_INDEX) |
				 (dma_job->llp_descr[0].dst_sel << DMAC_CH_DST_SEL_BIT_INDEX));
			DMAC_CH_CSR_REG(i) = csr_reg;

#if 0
			if (dma_job->llp_descr[0].tc_mask) {
				DMAC_CH_CFG_REG(i) |= 0x1;
			} else {
				DMAC_CH_CFG_REG(i) &= ~0x1;
			}
#endif

			DMAC_CH_SRC_ADDR_REG(i) = dma_job->llp_descr[0].src_addr;
			DMAC_CH_DST_ADDR_REG(i) = dma_job->llp_descr[0].dst_addr;
			DMAC_CH_LLP_REG(i) = dma_job->llp_descr[0].llp;
			DMAC_CH_SIZE_REG(i) = dma_job->llp_descr[0].tot_size;
			HAL_DMAC_ENABLE_CHANNEL(i);
			break;
#ifdef DMA_DEBUG
			//dma_dump_reg();
#endif
		}
		if (dma_busy_q_len == DMA_BUSY_MAX_Q_LEN) {
			dma_busy = 1;
			break;
		}
	}
	spin_unlock_irqrestore(&dma_lock, flags);
}

static void dma_process_done_job(void *data)
{
	dma_job_t *dma_job;
	struct list_head *l, *t;
	unsigned long flags;

	spin_lock_irqsave(&dma_lock, flags);
	list_for_each_safe(l, t, &dma_done_q) {
		dma_job = list_entry(l, dma_job_t, lh);
		list_del_init(&dma_job->lh);
		dma_done_q_len--;
		spin_unlock_irqrestore(&dma_lock, flags);
		dma_job->xfer->dma_end_io(dma_job->xfer, dma_job->done_status);
		dma_job_free(dma_job);
		spin_lock_irqsave(&dma_lock, flags);
	}
	spin_unlock_irqrestore(&dma_lock, flags);
}

irqreturn_t dma_tc_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	dma_job_t *dma_job;
	u32 tc_status;
	int i;

	tc_status = DMAC_INT_TC_STATUS_REG;

	for (i = 0; i <= 6; i++) {
		if (tc_status & (1 << i)) {
			HAL_DMAC_DISABLE_CHANNEL(i);
			dma_job = dma_running_job[i];
			if (dma_job) {
				dma_running_job[i] = NULL;
				dma_job->done_status = DMAC_RESPONSE_OK;
				list_add_tail(&dma_job->lh, &dma_done_q);
				dma_done_q_len++;
				dma_busy_q_len--;
				if (dma_busy) {
					dma_busy = 0;
				}
				schedule_work(&dma_done_task);
				if (dma_xfer_q_len) {
					schedule_work(&dma_xfer_task);
				}
			}
			DMAC_INT_TC_STATUS_CLR_REG |= (1 << i);
			break;
		}
	}

	return IRQ_HANDLED;
}

irqreturn_t dma_err_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	dma_job_t *dma_job;
	u32 err_status;
	int i;

	err_status = DMAC_INT_ERR_STATUS_REG;

	for (i = 0; i <= 6; i++) {
		if (err_status & (1 << i)) {
			HAL_DMAC_DISABLE_CHANNEL(i);
			dma_job = dma_running_job[i];
			if (dma_job) {
				dma_running_job[i] = NULL;
				dma_job->done_status = DMAC_RESPONSE_ERR;
				list_add_tail(&dma_job->lh, &dma_done_q);
				dma_done_q_len++;
				dma_busy_q_len--;
				if (dma_busy) {
					dma_busy = 0;
				}
				schedule_work(&dma_done_task);
				if (dma_xfer_q_len) {
					schedule_work(&dma_xfer_task);
				}
			}
			DMAC_INT_ERR_STATUS_CLR_REG |= (1 << i);
			break;
		}
	}

	return IRQ_HANDLED;
}

void dma_channel_init(void)
{
	int i;

	// disable the channel
	DMAC_CH_CSR_REG(DMA_COPY_BUSY_WAIT_CHANNEL) &= ~0x1;
	DMAC_CH_CFG_REG(DMA_COPY_BUSY_WAIT_CHANNEL) = 0x7;

	for (i = 0; i <= 6; i++) {
		HAL_DMAC_DISABLE_CHANNEL(i);
		DMAC_CH_CSR_REG(i) = DEFAULT_DMA_CH_CTL;
		DMAC_CH_CFG_REG(i) &= ~0x3;
	}

	return;
}

static int dma_init(void)
{
	int retval;

	HAL_PWRMGT_ENABLE_DMA_CLOCK();

	// Master0 & Master1 in Little Endian Mode, DMA Controller Enable
	DMAC_CSR_REG = 0x1;

	if (dma_job_q_init() != 0) {
		return -EFAULT;
	}

	dma_channel_init();

#ifdef DMA_DEBUG
	dma_dump_reg();
#endif

	spin_lock_init(&dma_lock);
	INIT_LIST_HEAD(&dma_xfer_q);
	INIT_LIST_HEAD(&dma_done_q);

	retval = request_irq(INTC_GDMAC_TC_BIT_INDEX, &dma_tc_isr, SA_INTERRUPT, "STAR DMA TC ISR", &dma_dev);
	if (retval) {
		printk("%s: unable to get IRQ %d (irqval=%d).\n", "STAR DMA", INTC_GDMAC_TC_BIT_INDEX, retval);
		return retval;
	}

	retval = request_irq(INTC_GDMAC_ERROR_BIT_INDEX, &dma_err_isr, SA_INTERRUPT, "STAR DMA ERR ISR", &dma_dev);
	if (retval) {
		printk("%s: unable to get IRQ %d (irqval=%d).\n", "STAR DMA", INTC_GDMAC_ERROR_BIT_INDEX, retval);
		free_irq(INTC_GDMAC_TC_BIT_INDEX, &dma_dev);
		return retval;
	}

	return 0;
}

static int __init str8100_dma_init(void)
{
	int retval;

	printk("STR8100 DMA driver init\n");
	retval = dma_init();
#ifdef STR8100_DMA_TEST
	if (retval == 0) {
		static int str8100_dmacopy_busywait_test(void);
		static int str8100_dmacopy_llp_test(void);
		(void)str8100_dmacopy_busywait_test();
		(void)str8100_dmacopy_llp_test();
	}
#endif
	return retval;
}

static void __exit str8100_dma_exit(void)
{
	return;
}

module_init(str8100_dma_init);
module_exit(str8100_dma_exit);

#ifdef STR8100_DMA_TEST
#define MEMSIZE_K	64
#define MEMSIZE		(MEMSIZE_K*1024)

static int str8100_dmacopy_busywait_test(void)
{
	u8 *src;
	u8 *dst;
	int i;
	int sjiffies;
	int err_cnt = 0;

	src = kmalloc(MEMSIZE, GFP_KERNEL);
	if (!src) goto out;
	dst = kmalloc(MEMSIZE, GFP_KERNEL);
	if (!dst) {
		kfree(src);
		goto out;
	}

	memset(src, 0xA9, MEMSIZE);
	memset(dst, 0, MEMSIZE);

	printk("STR8100 DMA Busy Wait Testing...\n");

	sjiffies = jiffies;
	for (i = 0; i < 1024; i++) {
		dma_memcpy_busy_wait(dst, src, MEMSIZE);
	}

	printk("STR8100 DMA Busy Wait Testing end\n");
	if (memcmp(src, dst, MEMSIZE) == 0) {
		printk("STR8100 DMA Busy Wait Testing success\n");
		printk("STR8100 DMA Busy Wait Testing speed: %dMB/s\n", (u32)((MEMSIZE_K) * HZ/(jiffies - sjiffies)));
	} else {
		printk("STR8100 DMA Busy Wait Testing failed\n");
		for (i = 0; i < MEMSIZE; i++) {
			if (src[i] != dst[i]) {
				err_cnt++;
			}
		}
		printk("err_cnt: %d\n", err_cnt);
	}

	kfree(src);
	kfree(dst);

out:
	return 0;
}

#define NUM_DMA_XFER		32
#define NUM_DMA_VEC		32
#define NUM_BYTES_PER_VEC	1024

static dma_xfer_t dma_xfer_test[NUM_DMA_XFER];
static u32 dma_xfer_finished;

static u8 *src_vec[NUM_DMA_XFER];
static u8 *dst_vec[NUM_DMA_XFER];

extern void dma_copy(dma_xfer_t *dma_xfer);
extern void dma_dump_reg(void);

static void dma_copy_end(dma_xfer_t *dma_xfer, int err)
{
	int i;
	int idx;

	idx = (int)dma_xfer->private;
	dma_xfer_finished++;

	if (err) {
		printk("STR8100 DMA Testing failed!!\n");
	} else {
		for (i = 0; i < NUM_DMA_VEC; i++) {
			if (memcmp((src_vec[idx] + i * NUM_BYTES_PER_VEC), (dst_vec[idx] + i * NUM_BYTES_PER_VEC), NUM_BYTES_PER_VEC) == 0) {
				printk("STR8100 DMA Testing success on xfer idx:%d started at offset: %04d:\n", idx, i * NUM_BYTES_PER_VEC);
			} else {
				int j;
				u8 *psrc;
				u8 *pdst;

				psrc = src_vec[idx] + i * NUM_BYTES_PER_VEC;
				pdst = dst_vec[idx] + i * NUM_BYTES_PER_VEC;

				printk("STR8100 DMA Testing error started at offset: %04d\n", i * NUM_BYTES_PER_VEC);

				for (j = 0; j < NUM_BYTES_PER_VEC; j++) {
					if ((j % 16) == 0) {
						printk("\n %08x: ", (u32)(psrc + j));
					}
					if (((j % 16) != 0) && ((j % 4) == 0)) {
						printk(" ");
					}
					printk("%02x", psrc[j]);
				}
				printk("\n");
	
				for (j = 0; j < NUM_BYTES_PER_VEC; j++) {
					if ((j % 16) == 0) {
						printk("\n %08x: ", (u32)(pdst + j));
					}
					if (((j % 16) != 0) && ((j % 4) == 0)) {
						printk(" ");
					}
					printk("%02x", pdst[j]);
				}
				printk("\n");
			}
		}
	}

	if (dma_xfer_finished == NUM_DMA_XFER) {
		for (i = 0; i < NUM_DMA_XFER; i++) {
			kfree(src_vec[i]);
			kfree(dst_vec[i]);
		}
	}
}

static int str8100_dmacopy_llp_test(void)
{
	int i, j;

	for (i = 0; i < NUM_DMA_XFER; i++) {
		src_vec[i] = kmalloc(MEMSIZE, GFP_KERNEL);
		if (!src_vec[i]) goto err_out;
		dst_vec[i] = kmalloc(MEMSIZE, GFP_KERNEL);
		if (!dst_vec[i]) goto err_out;
		memset(src_vec[i], 0xA9, MEMSIZE);
		memset(dst_vec[i], 0x00, MEMSIZE);
		printk("dmacopy_llp src_vec[%d]: 0x%08x\n", i, (u32)src_vec[i]);
		printk("dmacopy_llp dst_vec[%d]: 0x%08x\n", i, (u32)dst_vec[i]);
	}

	for (i = 0; i < NUM_DMA_XFER; i++) {
		dma_xfer_test[i].nr_vec = NUM_DMA_VEC;
		dma_xfer_test[i].dma_end_io = dma_copy_end;
		dma_xfer_test[i].private = (void *)i;
		for (j = 0; j < NUM_DMA_VEC; j++) {
			dma_xfer_test[i].vec[j].src_addr = (u32)(src_vec[i] + (j * NUM_BYTES_PER_VEC));
			dma_xfer_test[i].vec[j].dst_addr = (u32)(dst_vec[i] + (j * NUM_BYTES_PER_VEC));
			dma_xfer_test[i].vec[j].size = NUM_BYTES_PER_VEC;
			dma_xfer_test[i].vec[j].dst_sel = 0;
			dma_xfer_test[i].vec[j].src_sel = 0;
			dma_xfer_test[i].vec[j].dst_addr_ctl = 0;
			dma_xfer_test[i].vec[j].src_addr_ctl = 0;
			dma_xfer_test[i].vec[j].dst_width = DMAC_CH_DST_WIDTH_8BIT;
			dma_xfer_test[i].vec[j].src_width = DMAC_CH_SRC_WIDTH_8BIT;
		}
	}

	printk("STR8100 DMA Testing ...\n");

	for (i = 0; i < NUM_DMA_XFER; i++) {
		dma_copy(&dma_xfer_test[i]);
	}

	return 0;

err_out:
	for (i = 0; i < NUM_DMA_XFER; i++) {
		if (src_vec[i])
			kfree(src_vec[i]);
		if (dst_vec[i])
			kfree(dst_vec[i]);
	}
	return 0;
}

#endif // STR8100_DMA_TEST

