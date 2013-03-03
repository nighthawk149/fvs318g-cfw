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

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#define NS_MANUFACTURER_ID		0xEF
#define NS_NX25P16_DEVICE_ID		0x2015
#define ST_MANUFACTURER_ID		0x20
#define ST_M25P32_DEVICE_ID		0x2016
#define ST_M25P64_DEVICE_ID		0x2017
#define MX_MANUFACTURER_ID        0xC2
#define MX_25L32_DEVICE_ID        0x2016
#define MX_25L64_DEVICE_ID        0x2017

#define SPI_FLASH_WIP_BIT		(0x01)
#define SPI_FLASH_WEL_BIT		(0x02)
#define SPI_FLASH_BP0_BIT		(0x04)
#define SPI_FLASH_BP1_BIT		(0x08)
#define SPI_FLASH_BP2_BIT		(0x10)
#define SPI_FLASH_SRWD_BIT		(0x80)

#define SPI_FLASH_BP012_BITS		(SPI_FLASH_BP0_BIT | SPI_FLASH_BP1_BIT | SPI_FLASH_BP2_BIT)

#define SPI_FLASH_WREN_OPCODE		(0x06)
#define SPI_FLASH_WRDI_OPCODE		(0x04)
#define SPI_FLASH_RDID_OPCODE		(0x9F)
#define SPI_FLASH_RDSR_OPCODE		(0x05)
#define SPI_FLASH_WRSR_OPCODE		(0x01)
#define SPI_FLASH_READ_OPCODE		(0x03)
#define SPI_FLASH_FAST_READ_OPCODE	(0x0B)
#define SPI_FLASH_PP_OPCODE		(0x02)
#define SPI_FLASH_SE_OPCODE		(0xD8)
#define SPI_FLASH_BE_OPCODE		(0xC7)
#define SPI_FLASH_DP_OPCODE		(0xB9)
#define SPI_FLASH_RES_OPCODE		(0xAB)

#define ST_M25P32_SECTOR_NUM		64
#define ST_M25P32_PAGE_NUM		16384
#define ST_M25P64_SECTOR_NUM		128
#define ST_M25P64_PAGE_NUM		32768
#define NS_NX25P16_SECTOR_NUM		32
#define NS_NX25P16_PAGE_NUM		8192
#define MX_25L32_SECTOR_NUM       64
#define MX_25L32_PAGE_NUM     16384
#define MX_25L64_SECTOR_NUM       128
#define MX_25L64_PAGE_NUM     32768

#define SPI_FLASH_MAX_SECTOR_NUM	128
#define SPI_FLASH_MAX_PAGE_NUM		32768

#define SPI_FLASH_SECTOR0_BASE_ADDR	(0x0)
#define SPI_FLASH_SECTOR_SIZE		(0x10000)
#define SPI_FLASH_SECTOR_BASE_ADDR(x)	(SPI_FLASH_SECTOR0_BASE_ADDR + (x * SPI_FLASH_SECTOR_SIZE))
#define SPI_FLASH_PAGE0_BASE_ADDR	(0x0)
#define SPI_FLASH_PAGE_SIZE		(256)
#define SPI_FLASH_PAGE_BASE_ADDR(x)	(SPI_FLASH_PAGE0_BASE_ADDR + (x * SPI_FLASH_PAGE_SIZE))
#define SPI_FLASH_PAGE_NUM_OF_SECTOR	(SPI_FLASH_SECTOR_SIZE / SPI_FLASH_PAGE_SIZE)

#endif  // _SPI_FLASH_H_

