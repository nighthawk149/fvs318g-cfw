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
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <asm/arch/star_spi.h>
#include "str8100_spiflash.h"

static u32 Spi_Flash_Set_Write_Enable(u8 spi_flash_channel);
static u32 Spi_Flash_Is_Flash_Ready(u8 spi_flash_channel);

struct spi_flash_info
{
	u32 sectors;
	u32 sector_size;
	u32 pages;
	u32 page_size;
	struct mtd_info mtd;
#ifdef CONFIG_MTD_PARTITIONS
	struct mtd_partition *parsed_parts;     /* parsed partitions */
#endif
};

static struct spi_flash_info spi_flash_bank[1];

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Initialize
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Initialize(u8 spi_flash_channel)
{
	u32 receive_data;

	// Enable SPI pins
	HAL_MISC_ENABLE_SPI_PINS();

	// Disable SPI serial flash access through 0x30000000 region
	HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS();

	/*
	 * Note SPI is NOT enabled after this function is invoked!!
	 */
	SPI_CONFIGURATION_REG =
		(((0x0 & 0x3) << 0) | /* 8bits shift length */
		 (0x0 << 9) | /* general SPI mode */
		 (0x0 << 10) | /* disable FIFO */
		 (0x1 << 11) | /* SPI master mode */
		 (0x0 << 12) | /* disable SPI loopback mode */
		 (0x0 << 13) |
		 (0x0 << 14) |
		 (0x0 << 24) | /* Disable SPI Data Swap */
		 (0x0 << 30) | /* Disable SPI High Speed Read for BootUp */
		 (0x0 << 31)); /* Disable SPI */

	SPI_BIT_RATE_CONTROL_REG = 0x1 & 0x07; // PCLK/8

	// Configure SPI's Tx channel
	SPI_TRANSMIT_CONTROL_REG &= ~(0x03);
	SPI_TRANSMIT_CONTROL_REG |= spi_flash_channel & 0x03;

	// Configure Tx FIFO Threshold
	SPI_FIFO_TRANSMIT_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_TRANSMIT_CONFIG_REG |= ((0x0 & 0x03) << 4);

	// Configure Rx FIFO Threshold
	SPI_FIFO_RECEIVE_CONFIG_REG &= ~(0x03 << 4);
	SPI_FIFO_RECEIVE_CONFIG_REG |= ((0x1 & 0x03) << 4);

	SPI_INTERRUPT_ENABLE_REG = 0;

	// Clear spurious interrupt sources
	SPI_INTERRUPT_STATUS_REG = (0xF << 4);

	receive_data = SPI_RECEIVE_BUFFER_REG;

	// Enable SPI
	SPI_CONFIGURATION_REG |= (0x1 << 31);

	return;
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Bus_Idle
 * PURPOSE:
 *
 ******************************************************************************/
static inline u32
Spi_Flash_Is_Bus_Idle(void)
{
	/*
	 * Return value :
	 *    1 : Bus Idle
	 *    0 : Bus Busy
	 */
	return ((SPI_SERVICE_STATUS_REG & 0x1) ? 0 : 1);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Tx_Buffer_Empty
 * PURPOSE:
 *
 ******************************************************************************/
static inline u32
Spi_Flash_Is_Tx_Buffer_Empty(void)
{
	/*
	 * Return value :
	 *    1 : SPI Tx Buffer Empty
	 *    0 : SPI Tx Buffer Not Empty
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 3)) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Rx_Buffer_Full
 * PURPOSE:
 *
 ******************************************************************************/
static inline u32
Spi_Flash_Is_Rx_Buffer_Full(void)
{
	/*
	 * Return value :
	 *    1 : SPI Rx Buffer Full
	 *    0 : SPI Rx Buffer Not Full
	 */
	return ((SPI_INTERRUPT_STATUS_REG & (0x1 << 2)) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Buffer_Transmit_Receive
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Buffer_Transmit_Receive(u32 tx_channel, u32 tx_eof_flag, u32 tx_data, u32 *rx_data)
{
	u32 volatile rx_channel;
	u32 volatile rx_eof_flag;

	/*
	 * 1. Wait until SPI Bus is idle, and Tx Buffer is empty
	 * 2. Configure Tx channel and Back-to-Back transmit EOF setting
	 * 3. Write Tx Data 
	 * 4. Wait until Rx Buffer is full
	 * 5. Get Rx channel and Back-to-Back receive EOF setting
	 * 6. Get Rx Data
	 */
	while (!Spi_Flash_Is_Bus_Idle()) ;

	while (!Spi_Flash_Is_Tx_Buffer_Empty()) ;

	SPI_TRANSMIT_CONTROL_REG &= ~(0x7);
	SPI_TRANSMIT_CONTROL_REG |= (tx_channel & 0x3) | ((tx_eof_flag & 0x1) << 2);

	SPI_TRANSMIT_BUFFER_REG = tx_data;

	while (!Spi_Flash_Is_Rx_Buffer_Full()) ;

	rx_channel = (SPI_RECEIVE_CONTROL_REG & 0x3);

	rx_eof_flag = (SPI_RECEIVE_CONTROL_REG & (0x1 << 2)) ? 1 : 0;

	*rx_data = SPI_RECEIVE_BUFFER_REG;

	if ((tx_channel != rx_channel) || (tx_eof_flag != rx_eof_flag)) {
		return 0;	// Failed!!
	} else {
		return 1;	// OK!!
	}
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Read_Status_Register
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Read_Status_Register(u8 spi_flash_channel, u8 * status_reg)
{
	u32 rx_data;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_RDSR_OPCODE, &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, 0xFF, &rx_data);

	*status_reg = (u8) (rx_data & 0xFF);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Write_Status_Register
 * PURPOSE:
 *
 ******************************************************************************/
#if 0
static u32
Spi_Flash_Write_Status_Register(u8 spi_flash_channel, u8 status_reg)
{
	u32 rx_data;

	/*
	 * First, issue "Write Enable" instruction, and then issue "Write Status
	 * Register" instruction 
	 */
	if (Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_WRSR_OPCODE, &rx_data);

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, (u32) status_reg, &rx_data);

		// Wait until this command is complete
		while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

		return 1;
	} else {
		return 0;
	}
}
#endif /* Disable function Spi_Flash_Write_Status_Register */

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Is_Flash_Ready
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Is_Flash_Ready(u8 spi_flash_channel)
{
	u8 status_reg;

	/*
	 * Return value :
	 *    1 : SPI Flash is ready
	 *    0 : SPI Flash is busy
	 */
	Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

	return (status_reg & SPI_FLASH_WIP_BIT) ? 0 : 1;
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Set_Write_Enable
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Set_Write_Enable(u8 spi_flash_channel)
{
	u32 rx_data;
	u8 status_reg;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, SPI_FLASH_WREN_OPCODE, &rx_data);

	Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

	return ((status_reg & SPI_FLASH_WEL_BIT) ? 1 : 0);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Set_Write_Disable
 * PURPOSE:
 *
 ******************************************************************************/
#if 0
static u32
Spi_Flash_Set_Write_Disable(u8 spi_flash_channel)
{
	u32 rx_data;
	u8 status_reg;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, SPI_FLASH_WRDI_OPCODE, &rx_data);

	Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

	return ((status_reg & SPI_FLASH_WEL_BIT) ? 0 : 1);
}
#endif /* Spi_Flash_Set_Write_Disable */

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Read_Identification
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Read_Identification(u8 spi_flash_channel, u8 * manufacture_id, u16 * device_id)
{
	u32 rx_data1, rx_data2, rx_data3;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_RDID_OPCODE, &rx_data1);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, 0xFF, &rx_data1);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, 0xFF, &rx_data2);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, 0xFF, &rx_data3);

	*manufacture_id = (u8) (rx_data1 & 0xFF);

	*device_id = (u16) ((rx_data2 & 0xFF) << 8) | (u16) (rx_data3 & 0xFF);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Read_Data_Bytes
 * PURPOSE:
 *
 ******************************************************************************/
static void
Spi_Flash_Read_Data_Bytes(u8 spi_flash_channel, u32 address, u8 * read_buffer, u32 len)
{
	u32 rx_data;
	u32 ii;

	// Wait until Flash is ready
	while (!Spi_Flash_Is_Flash_Ready(spi_flash_channel)) ;

#if 1
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_FAST_READ_OPCODE, &rx_data);
#else
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_READ_OPCODE, &rx_data);
#endif

	/*
	 * Note the address is 24-Bit.
	 * The first byte addressed can be at any location, and the address is automatically
	 * incremented to the next higher address after each byte of the data is shifted-out.
	 * When the highest address is reached, the address counter rolls over to 000000h,
	 * allowing the read sequence to be continued indefinitely.
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 16) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 8) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);

#if 1
	/*
	 * Dummy Byte - 8bit, only on FAST_READ
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);
#endif

	/*
	 * Read "len" data bytes
	 */
	for (ii = 0; ii < len - 1; ii++) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, 0xFF, &rx_data);

		*read_buffer++ = (u8) (rx_data & 0xFF);
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, 0xFF, &rx_data);

	*read_buffer = (u8) (rx_data & 0xFF);
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Sector_Erase
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Sector_Erase(u8 spi_flash_channel, u32 sector_addr)
{
	struct spi_flash_info *flash_info = &spi_flash_bank[spi_flash_channel];
	u32 rx_data;

	// The specified address is beyond the maximum address range
	if (sector_addr > (flash_info->sectors * flash_info->sector_size))
		return 0;

	/*
	 * First, issue "Write Enable" instruction, and then issue "Sector Erase" instruction
	 * Note any address inside the Sector is a valid address of the Sector Erase instruction
	 */
	if (Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_SE_OPCODE, &rx_data);

		/*
		 * Note the sector address is 24-Bit
		 */
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((sector_addr >> 16) & 0xFF), &rx_data);

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((sector_addr >> 8) & 0xFF), &rx_data);

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, (u32) ((sector_addr >> 0) & 0xFF), &rx_data);

		return 1;
	} else {
		return 0;
	}
}

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Bulk_Erase
 * PURPOSE:
 *
 ******************************************************************************/
#if 0
static u32
Spi_Flash_Bulk_Erase(u8 spi_flash_channel)
{
	u32 rx_data;
	u8 status_reg;

	/*
	 * First, issue "Write Enable" instruction, and then issue "Bulk Erase" instruction
	 * Note the Bulk Erase instruction is executed only if all Block Protect (BP2, BP2, 
	 * BP0) bits are 0. The Bulk Erase instruction is ignored if one or more sectors are
	 * protected.
	 */
	if (Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		Spi_Flash_Read_Status_Register(spi_flash_channel, &status_reg);

		if (status_reg & SPI_FLASH_BP012_BITS) {
			// Failed because one or more sectors are protected!!
			return 0;
		}

		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, SPI_FLASH_BE_OPCODE, &rx_data);

		return 1;
	} else {
		return 0;
	}
}
#endif /* Spi_Flash_Bulk_Erase */

/******************************************************************************
 *
 * FUNCTION:  Spi_Flash_Page_Program_Data_Bytes
 * PURPOSE:
 *
 ******************************************************************************/
static u32
Spi_Flash_Page_Program_Data_Bytes(u8 spi_flash_channel, u32 address, u8 * write_buffer, u32 len)
{
	struct spi_flash_info *flash_info = &spi_flash_bank[spi_flash_channel];
	u32 rx_data;
	u32 ii;

	// This function does not support (len > SPI_FLASH_PAGE_SIZE)
	if (len > flash_info->page_size)
		return 0;

	// The specified address is beyond the maximum address range
	if ((address + len) > (flash_info->pages * flash_info->page_size))
	if ((address + len) > SPI_FLASH_PAGE_BASE_ADDR(SPI_FLASH_MAX_PAGE_NUM))
		return 0;

	// The specified address range will cross the page boundary
	if ((address / flash_info->page_size) != ((address + len - 1) / flash_info->page_size))
		return 0;

	/*
	 * First, issue "Write Enable" instruction, and then issue "Page Program" instruction
	 */
	if (!Spi_Flash_Set_Write_Enable(spi_flash_channel)) {
		return 0;
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, SPI_FLASH_PP_OPCODE, &rx_data);

	/*
	 * Note the address is 24-Bit
	 * If the 8 least significant address bits (A7~A0) are not all zero, all transmitted
	 * data that goes beyond the end of the current page are programmed from the start
	 * address of the same page (from the address whose 8 least significant address bits 
	 * (A7~A0) are all zero.
	 * If more than 256 bytes are sent to the device, previously latched data are discarded
	 * and the last 256 data bytes are guaranteed to be programmed correctly within the
	 * same page.
	 * If less than 256 Data bytes are sent to the device, they are correctly programmed
	 * at the requested addresses without having any effects on the other bytes of the same
	 * page.
	 */
	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 16) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 8) & 0xFF), &rx_data);

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) ((address >> 0) & 0xFF), &rx_data);

	/*
	 * Write "len" data bytes
	 */
	for (ii = 0; ii < len - 1; ii++) {
		Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 0, (u32) * write_buffer++, &rx_data);
	}

	Spi_Flash_Buffer_Transmit_Receive(spi_flash_channel, 1, (u32) * write_buffer, &rx_data);

	return 1;
}

/* #define SPIFLASH_DEBUG */
static char module_name[] = "STR8100 SPI";

#define MIN(a,b)        ((a) < (b) ? (a) : (b))
#define FALSE		0
#define TRUE		1

static struct mtd_erase_region_info erase_regions[] = {
	{
		.offset		= 0x00000000,
		.erasesize	= 0,
		.numblocks	= 0,
	}
};

static struct mtd_partition fixed_parts[] = {
	{
		.name =		"ARMBOOT",
		.offset =	CONFIG_ARMBOOT_OFFSET,
		.size =		CONFIG_KERNEL_OFFSET-CONFIG_ARMBOOT_OFFSET,
	},{
		.name =		"Linux Kernel",
		.offset =	CONFIG_KERNEL_OFFSET,
		.size =		CONFIG_ROOTFS_OFFSET-CONFIG_KERNEL_OFFSET,
	},{
		.name =		"MTD Disk1",
		.offset =	CONFIG_ROOTFS_OFFSET,
		.size =		CONFIG_CFG_OFFSET-CONFIG_ROOTFS_OFFSET,
	},{
		.name =		"MTD Disk2",
		.offset =	CONFIG_CFG_OFFSET,
		.size =		0x800000-CONFIG_CFG_OFFSET,
	}
};

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

static int 
str8100_spiflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct spi_flash_info *flash_info = mtd->priv;
	u32 i;
	u32 start_sector = 0;
	u32 end_sector = 0;

#ifdef DEBUG_DATAFLASH
	printk("dataflash_erase: addr=%i len=%i\n", instr->addr, instr->len);
#endif

	/* Sanity checks */
	if (instr->addr + instr->len > mtd->size)
		return -EINVAL;
	if ((instr->len % mtd->erasesize != 0) || (instr->len % flash_info->sector_size != 0))
		return -EINVAL;
	if ((instr->addr % flash_info->sector_size) != 0)
		return -EINVAL;

	for (i = 0; i < flash_info->sectors; i++) {
		if (instr->addr >= (i * flash_info->sector_size))
			continue;
		start_sector = i - 1;
		break;
	}
	for (i = start_sector; i < flash_info->sectors; i++) {
		if ((instr->addr + instr->len) > (i * flash_info->sector_size))
			continue;
		end_sector = i - 1;
		break;
	}
	for (i = start_sector; i <= end_sector; i++) {
		if (!Spi_Flash_Sector_Erase(0, i * flash_info->sector_size)) {
			printk("SPI flash sector %d erase error!!\n", i);
			goto erase_err;
		} else {
			printk("SPI flash sector %d erase ok!!\n", i);
		}
	}

	/* Inform MTD subsystem that erase is complete */
	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);

	return 0;

erase_err:
	instr->state = MTD_ERASE_FAILED;
	if (instr->callback)
		instr->callback(instr);

	return -EIO;
}

static void
str8100_debug_test(loff_t from, size_t len, size_t len1, size_t len2)
{
	printk("[KC_DEBUG] sizeof(loff_t): %d\n", sizeof(loff_t));
	printk("[KC_DEBUG] str8100_debug_test() from:0x%llx from:0x%x\n", from, (u32)from);
}

static int 
str8100_spiflash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	str8100_debug_test(from, len, len, len);

   	/* sanity checks */
   	if (!len) return (0);
   	if (from + len > mtd->size) return (-EINVAL);

	Spi_Flash_Read_Data_Bytes(0, from, buf, len);

   	/* we always read len bytes */
   	*retlen = len;

	return 0;
}

static int 
str8100_spiflash_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct spi_flash_info *flash_info = mtd->priv;
	u32 prog_size_left;
	u32 prog_size;
	u32 prog_size_total;
	u8 *write_buf = (u8 *)buf;

	/* Sanity checks */
	if (!len)
		return 0;
	if (to + len > mtd->size)
		return -EINVAL;

	*retlen = 0;

	prog_size_left = len;
	prog_size_total = 0;

	while (prog_size_left) {
		if (prog_size_left >= flash_info->page_size) {
			prog_size = flash_info->page_size;
		} else {
			prog_size = prog_size_left;
		}
		if (!Spi_Flash_Page_Program_Data_Bytes(0, to + prog_size_total, write_buf + prog_size_total, prog_size)) {
			goto write_error;
		}
		prog_size_left -= prog_size;
		prog_size_total += prog_size;
		*retlen += prog_size;
	}

	return 0;

write_error:
	return -EIO;
}

static int 
spiflash_probe(void)
{
	u8 manufacturer_id;
	u16 device_id;
	int retval = 0;

	Spi_Flash_Read_Identification(0, &manufacturer_id, &device_id);

	switch ((manufacturer_id << 16) | device_id) {
	case ((ST_MANUFACTURER_ID << 16) | ST_M25P32_DEVICE_ID):
		printk("Found SPI flash ST M25P32\n");
		spi_flash_bank[0].sectors = ST_M25P32_SECTOR_NUM;
		spi_flash_bank[0].sector_size = SPI_FLASH_SECTOR_SIZE;
		spi_flash_bank[0].pages = ST_M25P32_PAGE_NUM;
		spi_flash_bank[0].page_size = SPI_FLASH_PAGE_SIZE;
		break;
	case ((ST_MANUFACTURER_ID << 16) | ST_M25P64_DEVICE_ID):
		printk("Found SPI flash ST M25P64\n");
		spi_flash_bank[0].sectors = ST_M25P64_SECTOR_NUM;
		spi_flash_bank[0].sector_size = SPI_FLASH_SECTOR_SIZE;
		spi_flash_bank[0].pages = ST_M25P64_PAGE_NUM;
		spi_flash_bank[0].page_size = SPI_FLASH_PAGE_SIZE;
		break;
	case ((MX_MANUFACTURER_ID << 16) | MX_25L32_DEVICE_ID):
		printk("Found SPI flash MX 25L32\n");
		spi_flash_bank[0].sectors = MX_25L32_SECTOR_NUM;
		spi_flash_bank[0].sector_size = SPI_FLASH_SECTOR_SIZE;
		spi_flash_bank[0].pages = MX_25L32_PAGE_NUM;
		spi_flash_bank[0].page_size = SPI_FLASH_PAGE_SIZE;
		break;
	case ((MX_MANUFACTURER_ID << 16) | MX_25L64_DEVICE_ID):
		printk("Found SPI flash MX 25L64\n");
		spi_flash_bank[0].sectors = MX_25L64_SECTOR_NUM;
		spi_flash_bank[0].sector_size = SPI_FLASH_SECTOR_SIZE;
		spi_flash_bank[0].pages = MX_25L64_PAGE_NUM;
		spi_flash_bank[0].page_size = SPI_FLASH_PAGE_SIZE;
		break;
	default:
		retval = -1;
		break;
	}

	return retval;
}

/* ......................................................................... */
static int __devinit str8100_spiflash_init(void)
{
	struct mtd_info *mtd;
	struct spi_flash_info *flash_info;

	/* initialize spi channel 0 */
	Spi_Flash_Initialize(0);

   	printk ("STR8100 SPI: Probing for Serial flash ...\n");
	if (spiflash_probe() != 0) {
		return -ENODEV;
	}

	flash_info = &spi_flash_bank[0];
	mtd = &flash_info->mtd;

	erase_regions[0].offset = 0x0;
	erase_regions[0].erasesize = flash_info->sector_size;
	erase_regions[0].numblocks = flash_info->sectors;

	mtd->name = module_name;
	mtd->type = MTD_DATAFLASH;
	mtd->flags = MTD_WRITEABLE;
	mtd->size = flash_info->sectors * flash_info->sector_size;
	mtd->erasesize = flash_info->sector_size;
	mtd->numeraseregions = ARRAY_SIZE(erase_regions);
	mtd->eraseregions = erase_regions;
	mtd->erase = str8100_spiflash_erase;
	mtd->read = str8100_spiflash_read;
	mtd->write = str8100_spiflash_write;
	mtd->priv = flash_info;
	mtd->owner = THIS_MODULE;

	if (mtd_has_partitions()) {
		struct mtd_partition *parts;
		int nr_parts = 0;
#ifdef CONFIG_MTD_CMDLINE_PARTS
		static const char *part_probes[] = { "cmdlinepart", NULL, };
		nr_parts = parse_mtd_partitions(&flash->mtd,
			part_probes, &parts, 0);
#endif

		if (nr_parts > 0) {
			add_mtd_partitions(mtd, parts, nr_parts);
			flash_info->parsed_parts = parts;
		} else {
			add_mtd_partitions(mtd, fixed_parts, ARRAY_SIZE(fixed_parts));
			flash_info->parsed_parts = fixed_parts;
		}
	}

	return add_mtd_device(mtd) == 1 ? -ENODEV : 0;
}

module_init (str8100_spiflash_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Star Semi Corp");
MODULE_DESCRIPTION("STR8100 SPI Flash MTD driver");

