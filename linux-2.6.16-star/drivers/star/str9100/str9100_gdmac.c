/******************************************************************************
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
#include <linux/pci.h>
#include <asm/io.h>
//#include <asm/arch/str9100/star_powermgt.h>
#include "str9100_gdmac.h"

void gdmac_channel_init(void)
{
	GDMAC_CHANNEL_DISABLE(0);
	GDMAC_CHANNEL_DISABLE(1);
	GDMAC_CHANNEL_DISABLE(2);
	GDMAC_CHANNEL_DISABLE(3);
}

void gdmac_channel_control_init(void)
{
	u32 channel_control = ((GDMAC_CH_PRIORITY_3 << GDMAC_CH_PRIORITY_BIT) | \
				(GDMAC_CH_SRC_BURST_SIZE_256 << GDMAC_CH_SRC_BURST_SIZE_BIT) | \
				(GDMAC_CH_SRC_WIDTH_8BITS << GDMAC_CH_SRC_WIDTH_BIT) | \
				(GDMAC_CH_DST_WIDTH_8BITS << GDMAC_CH_DST_WIDTH_BIT) | \
				(GDMAC_CH_MODE_NORMAL << GDMAC_CH_MODE_BIT) | \
				(GDMAC_CH_SRC_ADDR_CTL_DEC << GDMAC_CH_SRC_ADDR_CTL_BIT) | \
				(GDMAC_CH_DST_ADDR_CTL_DEC << GDMAC_CH_DST_ADDR_CTL_BIT));


	GDMAC_CHANNEL_WRITE_CSR(0, channel_control);
	//GDMAC_CHANNEL_WRITE_CSR(0, 0);
}
#if 0
void gdmac_debug(void)
{
printk("GDMA_REG(GDMAC_INT_STATUS):		0x%08x\n", GDMAC_REG(0x00000000));
printk("GDMA_REG(GDMAC_TC_INT_STATUS):		0x%08x\n", GDMAC_REG(0x00000004));
printk("GDMA_REG(GDMAC_TC_INT_STATUS_CLR):	0x%08x\n", GDMAC_REG(0x00000008));
printk("GDMA_REG(GDMAC_ERR_INT_STATUS):		0x%08x\n", GDMAC_REG(0x0000000C));
printk("GDMA_REG(GDMAC_ERR_INT_STATUS_CLR):	0x%08x\n", GDMAC_REG(0x00000010));
printk("GDMA_REG(GDMAC_TC_STATUS):		0x%08x\n", GDMAC_REG(0x00000014));
printk("GDMA_REG(GDMAC_ERR_STATUS):		0x%08x\n", GDMAC_REG(0x00000018));
printk("GDMA_REG(GDMAC_CH_ENABLE_STATUS):	0x%08x\n", GDMAC_REG(0x0000001C));
printk("GDMA_REG(GDMAC_CH_BUSY_STATUS):		0x%08x\n", GDMAC_REG(0x00000020));
printk("GDMA_REG(GDMAC_CSR):			0x%08x\n", GDMAC_REG(0x00000024));
printk("GDMA_REG(GDMAC_SYNC):			0x%08x\n", GDMAC_REG(0x00000028));
printk("GDMA_REG(GDMAC_CH0_CSR):		0x%08x\n", GDMAC_REG(0x00000100));
printk("GDMA_REG(GDMAC_CH0_CFG):		0x%08x\n", GDMAC_REG(0x00000104));
printk("GDMA_REG(GDMAC_CH0_SRC_ADDR):		0x%08x\n", GDMAC_REG(0x00000108));
printk("GDMA_REG(GDMAC_CH0_DST_ADDR):		0x%08x\n", GDMAC_REG(0x0000010C));
printk("GDMA_REG(GDMAC_CH0_XFR_SIZE):		0x%08x\n", GDMAC_REG(0x00000114));
}
#endif

void gdmac_init(void)
{
	HAL_PWRMGT_ENABLE_DMA();
	GDMAC_CONFIG_MASTER0_LITTLE_ENDIAN();
	GDMAC_CONFIG_MASTER1_LITTLE_ENDIAN();
	GDMAC_ENABLE();
	gdmac_channel_init();
	gdmac_channel_control_init();
	//gdmac_debug();
}

void memcpy3(void *dst, const void *src, size_t len)
{
	while (GDMAC_CHANNEL_IS_BUSY(0))
		;	/* do nothing */

	consistent_sync((void *)src, len, PCI_DMA_TODEVICE);
	consistent_sync(dst, len, PCI_DMA_FROMDEVICE);
	GDMAC_CHANNEL_WRITE_SRC_ADDR(0, (u32)virt_to_phys((void *)src));
	GDMAC_CHANNEL_WRITE_DST_ADDR(0, (u32)virt_to_phys(dst));
	///printk("LEN:%d LEN:%d\n", len & 0xFFF,len);
	GDMAC_CH0_XFR_SIZE = (len & 0xFFF);
	GDMAC_CHANNEL_WRITE_XFR_SIZE(0, (u32)len);

	//gdmac_debug();

	GDMAC_CHANNEL_ENABLE(0);

	//gdmac_debug();

}

void memcpy3_check(void)
{

	while (!GDMAC_CHANNEL_WITH_TC(0))
		;	/* do nothing */

	GDMAC_CHANNEL_DISABLE(0);
	GDMAC_CHANNEL_CLEAR_TC_INT_STATUS(0);
}

void memcpy2(void *dst, const void *src, size_t len)
{
	while (GDMAC_CHANNEL_IS_BUSY(0))
		;	/* do nothing */

	consistent_sync((void *)src, len, PCI_DMA_TODEVICE);
	consistent_sync(dst, len, PCI_DMA_FROMDEVICE);
	GDMAC_CHANNEL_WRITE_SRC_ADDR(0, (u32)virt_to_phys((void *)src));
	GDMAC_CHANNEL_WRITE_DST_ADDR(0, (u32)virt_to_phys(dst));
	GDMAC_CHANNEL_WRITE_XFR_SIZE(0, (u32)len);

	//gdmac_debug();

	GDMAC_CHANNEL_ENABLE(0);

	//gdmac_debug();

	while (!GDMAC_CHANNEL_WITH_TC(0))
		;	/* do nothing */

	GDMAC_CHANNEL_DISABLE(0);
	GDMAC_CHANNEL_CLEAR_TC_INT_STATUS(0);
}

#ifdef CONFIG_MODULES
EXPORT_SYMBOL(memcpy3_check);
EXPORT_SYMBOL(memcpy3);
EXPORT_SYMBOL(memcpy2);
EXPORT_SYMBOL(gdmac_init);
#endif

