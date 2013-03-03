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
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/pci.h>

#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_i2s.h>
#include <asm/arch/star_demo_dma.h>

MODULE_AUTHOR("Mac Lin");
MODULE_LICENSE("Dual BSD/GPL");


#define CONFIG_I2S_USE_DMA
//Debug = left only
#define DEBUG	

static struct proc_dir_entry *star_i2s_proc_entry=NULL;
static u32 sampling_rate=44100;
static u32 sample_size=16;

module_param(sampling_rate, int, 0);
MODULE_PARM_DESC(gpio, "sampling_rate");

module_param(sample_size, int, 0);
MODULE_PARM_DESC(gpio, "sample_size");

#define DMA_TRANSFER_MAX_BYTE    (0xfff<<(sample_size>>4)) 
#define BUFSIZE (0xfff<<4) //twice the 32-bit DMA transfer byte size

static u8* lbuffer;
static u8* lbuffer_p;
static u32 llen=0;
static u32 lpos=0;
static u32 i2s_err_lur=0;

static u32 debug=0;
#define DEBUG_PRINT(arg...) if(debug) printk(arg);

//=================================================================================
//
#ifdef CONFIG_I2S_USE_DMA

static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8759_dma_handshake_left_tx;
static DMAC_HARDWARE_HANDSHAKE_OBJ_T    i2s_wm8759_dma_handshake_right_tx;

static u32                          i2s_wm8759_dma_right_tx_channel = 1;





void I2s_WM8759_Configure_DMA_Hardware_Handshake(void)
{
    /*
     * Configure DMA's channel setting for I2S's Left Channel Tx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    i2s_wm8759_dma_handshake_left_tx.channel_num = 0;
    i2s_wm8759_dma_handshake_left_tx.channel_id = DMAC_CH_ID(i2s_wm8759_dma_handshake_left_tx.channel_num);
    i2s_wm8759_dma_handshake_left_tx.target_select = DMAC_HW_HAND_SHAKE_I2S_TX_LEFT_ID;
    i2s_wm8759_dma_handshake_left_tx.src_addr = (u32)lbuffer_p;
    i2s_wm8759_dma_handshake_left_tx.src_master = DMAC_CH_SRC_SEL_M1;
    i2s_wm8759_dma_handshake_left_tx.dst_master = DMAC_CH_DST_SEL_M0;
    i2s_wm8759_dma_handshake_left_tx.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;
    i2s_wm8759_dma_handshake_left_tx.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;
    
	switch(sample_size){
	case 16:
		i2s_wm8759_dma_handshake_left_tx.dst_addr = (SYSPA_I2S_BASE_ADDR + 0xCA);
		i2s_wm8759_dma_handshake_left_tx.src_width = DMAC_CH_SRC_WIDTH_16_BITS;
		i2s_wm8759_dma_handshake_left_tx.dst_width = DMAC_CH_DST_WIDTH_16_BITS;
		break;
	case 32:
		i2s_wm8759_dma_handshake_left_tx.dst_addr = (SYSPA_I2S_BASE_ADDR + 0xC8);
		i2s_wm8759_dma_handshake_left_tx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
		i2s_wm8759_dma_handshake_left_tx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;
		break;
	}

//    i2s_wm8759_dma_handshake_left_tx.dst_width = DMAC_CH_DST_WIDTH_16_BITS;

    /*
     * Note here the total number of bytes for each DMA transfer is specified!!
     */
//    i2s_wm8759_dma_handshake_left_tx.transfer_bytes = DMA_TRANSFER_MAX_BYTE;
    i2s_wm8759_dma_handshake_left_tx.transfer_bytes = (llen<DMA_TRANSFER_MAX_BYTE)?llen:DMA_TRANSFER_MAX_BYTE;

    i2s_wm8759_dma_handshake_left_tx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;   

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8759_dma_handshake_left_tx);

#ifndef DEBUG 
    /*
     * Configure DMA's channel setting for I2S's Right Channel Tx
     * Specially pay attention to the settings of src_width, dst_width, and src_burst_size
     */
    i2s_wm8759_dma_handshake_right_tx.channel_id = DMAC_CH_ID(i2s_wm8759_dma_right_tx_channel);
    
    i2s_wm8759_dma_handshake_right_tx.target_select = DMAC_HW_HAND_SHAKE_I2S_TX_RIGHT_ID;

    i2s_wm8759_dma_handshake_right_tx.src_addr = (u32)rbuffer_p;

    i2s_wm8759_dma_handshake_right_tx.dst_addr = (SYSPA_I2S_BASE_ADDR + 0xC4);

    i2s_wm8759_dma_handshake_right_tx.src_master = DMAC_CH_SRC_SEL_M1;
    
    i2s_wm8759_dma_handshake_right_tx.dst_master = DMAC_CH_DST_SEL_M0;

    i2s_wm8759_dma_handshake_right_tx.srcad_ctl = DMAC_CH_SRCAD_CTL_INC;
    
    i2s_wm8759_dma_handshake_right_tx.dstad_ctl = DMAC_CH_DSTAD_CTL_FIX;

    i2s_wm8759_dma_handshake_right_tx.src_width = DMAC_CH_SRC_WIDTH_32_BITS;
    
    i2s_wm8759_dma_handshake_right_tx.dst_width = DMAC_CH_DST_WIDTH_32_BITS;

    /*
     * Note here the total number of bytes for each DMA transfer is specified!!
     */
    i2s_wm8759_dma_handshake_right_tx.transfer_bytes = DMA_TRANSFER_MAX_BYTE;

    i2s_wm8759_dma_handshake_right_tx.src_burst_size = DMAC_CH_SRC_BURST_SIZE_1;   

    Hal_Dmac_Configure_DMA_Handshake(&i2s_wm8759_dma_handshake_right_tx);
#endif
    return;
}
#endif //CONFIG_I2S_USE_DMA


//=================================================================================
//

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);

#ifdef CONFIG_I2S_USE_DMA
static irqreturn_t str8100_dma_tc_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 dma_tc_status,tot_size;
	u32 len;
//printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:

    HAL_DMAC_READ_TERMINAL_COUNT_INTERRUPT_STATUS(dma_tc_status);

    /*
     * For DMA's Tx for I2S Left Channel
     */
	if (dma_tc_status & DMAC_CH_ID(i2s_wm8759_dma_handshake_left_tx.channel_num))
	{                      
		HAL_DMAC_DISABLE_CHANNEL(i2s_wm8759_dma_handshake_left_tx.channel_num);
		HAL_DMAC_CLEAR_TERMINAL_COUNT_INTERRUPT_STATUS(DMAC_CH_ID(i2s_wm8759_dma_handshake_left_tx.channel_num));
		lpos+=i2s_wm8759_dma_handshake_left_tx.transfer_bytes;
		if(lpos<llen){
			/*
			 * Re-initialize DMA's channel for Left_Tx
			 */
			DMAC_CH_SRC_ADDR_REG(i2s_wm8759_dma_handshake_left_tx.channel_num) = (u32)lbuffer_p+lpos;
			DMAC_CH_DST_ADDR_REG(i2s_wm8759_dma_handshake_left_tx.channel_num) = (SYSPA_I2S_BASE_ADDR + 0xC8);        
			
			/*
			 * Note this macro DMAC_CH_SIZE is to configure TOT_SIZE field which is the total transfer
			 * number of source transfer width!
			 */        
			len=(llen - lpos);
			i2s_wm8759_dma_handshake_left_tx.transfer_bytes= (len<DMA_TRANSFER_MAX_BYTE)?len:DMA_TRANSFER_MAX_BYTE;

			if (i2s_wm8759_dma_handshake_left_tx.src_width == DMAC_CH_SRC_WIDTH_16_BITS)
			{
			    tot_size = (i2s_wm8759_dma_handshake_left_tx.transfer_bytes >> 1) + ((i2s_wm8759_dma_handshake_left_tx.transfer_bytes % 2) ? 1 : 0);
			}
			else if (i2s_wm8759_dma_handshake_left_tx.src_width == DMAC_CH_SRC_WIDTH_32_BITS)
			{
			    tot_size = (i2s_wm8759_dma_handshake_left_tx.transfer_bytes >> 2) + ((i2s_wm8759_dma_handshake_left_tx.transfer_bytes % 4) ? 1 : 0);
			}
			else
			{
			    tot_size = i2s_wm8759_dma_handshake_left_tx.transfer_bytes;
			}
			
			DMAC_CH_SIZE_REG(i2s_wm8759_dma_handshake_left_tx.channel_num) = tot_size & 0x0FFF;
			HAL_DMAC_ENABLE_CHANNEL(i2s_wm8759_dma_handshake_left_tx.channel_num);
//printk("%s: 32-bits transfer_bytes=%d, DMAC_CH_SIZE_REG(%d)=%.8x\n",__FUNCTION__,i2s_wm8759_dma_handshake_left_tx.transfer_bytes,i2s_wm8759_dma_handshake_left_tx.channel_num,DMAC_CH_SIZE_REG(i2s_wm8759_dma_handshake_left_tx.channel_num));

	}else{
		llen=lpos=0;
	}
	}


	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

#endif //CONFIG_I2S_USE_DMA

static irqreturn_t str8100_i2s_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	u32 interrupt_status;
	DEBUG_PRINT("%s: this_irq=%d, I2S_INTERRUPT_STATUS_REG=0x%.8x,llen=%d,lpos=%d\n",__FUNCTION__,this_irq,I2S_INTERRUPT_STATUS_REG,llen,lpos);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	//todo:
	interrupt_status = I2S_INTERRUPT_STATUS_REG;   
if((llen<lpos)||llen==0)     HAL_I2S_DISABLE_I2S();

#ifndef CONFIG_I2S_USE_DMA
		I2S_RIGHT_TRANSMIT_DATA_REG=0;

	if ((lpos<=llen) && (interrupt_status & I2S_TXBF_L_EMPTY_FLAG)){
		switch(sample_size){
			case 16:
				I2S_LEFT_TRANSMIT_DATA_REG=(*((u16*)(&lbuffer[lpos])))<<16;lpos+=2;
				break;
			case 32:
				I2S_LEFT_TRANSMIT_DATA_REG=*((u32*)(&lbuffer[lpos]));lpos+=4;
				break;
		}
		
		if(lpos>llen){
			// Disable Left Channel's Transmit Buffer Interrupt Sources
			I2S_INTERRUPT_ENABLE_REG &= ~(I2S_TXBF_L_EMPTY_FLAG | I2S_TXBF_L_UR_FLAG);
			lpos=llen=0;
		}
	}
#endif //!CONFIG_I2S_USE_DMA


	if (llen>=lpos && (interrupt_status & I2S_TXBF_L_UR_FLAG)){
		// Clear I2S interrupt status
		i2s_err_lur++;
		if(i2s_err_lur>10)
			I2S_INTERRUPT_ENABLE_REG &= ~(I2S_TXBF_L_UR_FLAG);
//			HAL_I2S_DISABLE_I2S();
		
		printk("%s: Left Channel Tx Underrun!,llen=%d, lpos=%d, interrupt_status=%.8x,i2s_err_lur=%d\n",__FUNCTION__,llen,lpos,interrupt_status,i2s_err_lur);
	}
	I2S_INTERRUPT_STATUS_REG &= 0xf0;

	
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}



//=================================================================================
static int proc_read_i2s(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int len=0;
	DEBUG_PRINT("%s:\n",__FUNCTION__);
	len += sprintf(buf,	"sampling_rate=%d\n"
				"sample_size=%d\n"
			,sampling_rate,sample_size); 
	*eof = 1;
	return len;
}

static proc_write_i2s(struct file *file, const char *buffer, unsigned long count, void *data){
	int len=0;
	DEBUG_PRINT("%s: sample size %d, sampling rate %d, count=%d\n",__FUNCTION__,sample_size,sampling_rate,count);
	
	//is buffer free?
	if(lpos!=llen){
		printk("%s: buffer not free\n");
		return -EBUSY;
	}
	if(sampling_rate!=32000&&sampling_rate!=44100&&sampling_rate!=48000){
		printk("%s: invalid sampling rate(%d)\n",sampling_rate);
		return -EFAULT;
	}
	if(sample_size!=16&&sample_size!=32){
		printk("%s: invalid sample size(%d)\n",sample_size);
		return -EFAULT;
	}

	//copy the raw data to local buffer	
	if(count>BUFSIZE) len=BUFSIZE;
	else len=count;
	
	if(copy_from_user(lbuffer,buffer,len)){
		return -EFAULT;	
	}
	llen=len;
	lpos=0;
	i2s_err_lur=0;
	
	str8100_i2s_init(sampling_rate,I2S_DATA_32_BIT, I2S_MASTER_MODE, I2S_I2S_MODE, I2S_CLOCK_256S_MODE);

#ifdef CONFIG_I2S_USE_DMA
    /*
     * Configure DMA for hardware handshake for I2S
     * Note the DMA is NOT enabled after invoking the following function, but the specified
     * DMA channels are enabled!!
     */
    i2s_wm8759_dma_right_tx_channel = 1;   

    I2s_WM8759_Configure_DMA_Hardware_Handshake();  

#endif //CONFIG_I2S_USE_DMA
	//fill the tx left/right registers

		switch(sample_size){
			case 16:
				I2S_LEFT_TRANSMIT_DATA_REG=(*((u16*)(&lbuffer[lpos])))<<16;lpos+=2;
				break;
			case 32:
				I2S_LEFT_TRANSMIT_DATA_REG=*((u32*)(&lbuffer[lpos]));lpos+=4;
				break;
		}

	I2S_RIGHT_TRANSMIT_DATA_REG=0;

#ifdef CONFIG_I2S_USE_DMA
	/*
	 * Enable I2S's interrupt sources
	 * Note for DMA hardware handshake, we only need to enable Left/Right Channel's 
	 * Transmit Buffer Underrun.
	 */
#ifdef DEBUG	
	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_L_UR_FLAG);
#else
	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG);
#endif
#else //CONFIG_I2S_USE_DMA
#ifdef DEBUG	
	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_L_UR_FLAG | I2S_TXBF_L_EMPTY_FLAG);
#else
	I2S_INTERRUPT_ENABLE_REG |= (I2S_TXBF_R_UR_FLAG | I2S_TXBF_L_UR_FLAG | I2S_TXBF_R_EMPTY_FLAG | I2S_TXBF_L_EMPTY_FLAG);
#endif
#endif //CONFIG_I2S_USE_DMA

	// Enable CPU interrupt
	local_irq_enable();
	
#ifdef CONFIG_I2S_USE_DMA
	/*
	 * Note DMA must be enabled first before I2S is enabled
	 */
	HAL_DMAC_ENABLE();
#endif //CONFIG_I2S_USE_DMA
	HAL_I2S_ENABLE_I2S();


	while (1){
		local_irq_disable();
		if (lpos>llen||llen==0){
			// Disable I2S
			HAL_I2S_DISABLE_I2S();
			break;
		}
		local_irq_enable();
	}
	DEBUG_PRINT("%s: exit. i2s_err_lur=%d\n",__FUNCTION__,i2s_err_lur);

	local_irq_enable();

	return len;

debug:
	HAL_I2S_DISABLE_I2S();
	return count;
}

static void __exit i2s_exit_module(void){
	printk("%s:\n",__FUNCTION__);
	remove_proc_entry("str8100/i2s", NULL);
	free_irq(INTC_I2S_BIT_INDEX, NULL);
#ifdef CONFIG_I2S_USE_DMA
	free_irq(INTC_GDMAC_TC_BIT_INDEX, NULL);
	free_irq(INTC_GDMAC_ERROR_BIT_INDEX, NULL);
#endif //CONFIG_I2S_USE_DMA
	if(lbuffer) {
		pci_free_consistent(NULL, BUFSIZE, lbuffer, lbuffer_p);
		lbuffer=lbuffer_p=NULL;
	}
}

static int __init i2s_init_module(void)
{
	
	u32 ret;
	
	printk("%s:\n",__FUNCTION__);
#ifdef CONFIG_I2S_USE_DMA
	printk("%s: DMA Enabled...\n",__FUNCTION__);
#endif //CONFIG_I2S_USE_DMA

	star_i2s_proc_entry = create_proc_entry("str8100/i2s", S_IFREG | S_IRUGO, NULL);
	if(!star_i2s_proc_entry){
		return -EBUSY;
	}
	star_i2s_proc_entry->read_proc=proc_read_i2s;
	star_i2s_proc_entry->write_proc=proc_write_i2s;
	
	lbuffer = pci_alloc_consistent(NULL, BUFSIZE, &lbuffer_p);
	if(!lbuffer){
		printk("%s: alloc lbuffer failed.\n",__FUNCTION__);
		goto exit1;
	}

	str8100_set_interrupt_trigger (INTC_I2S_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if ((ret=request_irq(INTC_I2S_BIT_INDEX, str8100_i2s_irq_handler, 0, "i2s", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_I2S_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}

#ifdef CONFIG_I2S_USE_DMA
	str8100_set_interrupt_trigger (INTC_GDMAC_TC_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_TC_BIT_INDEX, str8100_dma_tc_irq_handler, 0, "dma tc", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_TC_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}
	str8100_set_interrupt_trigger (INTC_GDMAC_ERROR_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GDMAC_ERROR_BIT_INDEX, str8100_dma_err_irq_handler, 0, "dma error", NULL))){
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_GDMAC_ERROR_BIT_INDEX,ret,-EBUSY);
		goto exit1;
	}
#endif //CONFIG_I2S_USE_DMA

	return 0;
exit1:
	i2s_exit_module();
	return -EBUSY;
}

module_init(i2s_init_module);
module_exit(i2s_exit_module);
