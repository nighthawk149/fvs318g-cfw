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


#ifndef	_STAR_PCI_DRIDGE_H_
#define	_STAR_PCI_DRIDGE_H_


#include <asm/arch/star_sys_memory_map.h>


#define	PCI_IO_SPACE_BASE_ADDR			(SYSPA_PCI_IO_SPACE_BASE_ADDR)
#define PCI_IO_SPACE_SIZE			0x08000000 /* 64MB */
#define PCI_IO_SPACE_START			PCI_IO_SPACE_BASE_ADDR
#define PCI_IO_SPACE_END			(PCI_IO_SPACE_BASE_ADDR + PCI_IO_SPACE_SIZE - 1)
#define	PCI_MEMORY_SPACE_BASE_ADDR		(SYSPA_PCI_MEMORY_SPACE_BASE_ADDR)
#define PCI_MEMORY_SPACE_SIZE			0x10000000 /* 256MB */
#define PCI_NPREFETCH_MEMORY_SPACE_START	PCI_MEMORY_SPACE_BASE_ADDR
#define PCI_NPREFETCH_MEMORY_SPACE_SIZE		0x00800000 /* 8MB */
#define PCI_NPREFETCH_MEMORY_SPACE_END		(PCI_NPREFETCH_MEMORY_SPACE_START + PCI_NPREFETCH_MEMORY_SPACE_SIZE - 1)
#define PCI_PREFETCH_MEMORY_SPACE_START		(PCI_NPREFETCH_MEMORY_SPACE_START + PCI_NPREFETCH_MEMORY_SPACE_SIZE)
#define PCI_PREFETCH_MEMORY_SPACE_SIZE		0x00800000 /* 8MB */
#define PCI_PREFETCH_MEMORY_SPACE_END		(PCI_PREFETCH_MEMORY_SPACE_START + PCI_PREFETCH_MEMORY_SPACE_SIZE - 1)


#if defined(__UBOOT__)
#define	PCIB_MEM_MAP_VALUE(base, reg_offset)	(*((u32 volatile *)(SYSPA_PCI_##base##_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	PCIB_MEM_MAP_VALUE(base, reg_offset)	(*((u32 volatile *)(SYSVA_PCI_##base##_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define	PCI_BRIDGE_CONFIG_DATA			PCIB_MEM_MAP_VALUE(CONFIG_DATA_BASE, 0x2C)
#define	PCI_BRIDGE_CONFIG_ADDR			PCIB_MEM_MAP_VALUE(CONFIG_ADDR_BASE, 0x28)

#define PCI_BRIDGE_CONFIG_DATA_REG_OFFSET	0x2C
#define PCI_BRIDGE_CONFIG_ADDR_REG_OFFSET	0x28

/*
 * define constants macros
 */
#define	PCIB_BUS_CLOCK_33M			1

#define	PCIB_BUS_CLOCK_66M			2

#define	PCIB_DEVICE_ID				0x8100

#define	PCIB_VENDOR_ID				0xEEEE

#define	PCIB_CLASS_CODE				0xFF0000

#define	PCIB_REVISION_ID			0x00

#define	PCIB_BAR0_MEMORY_SPACE_BASE		0x20000000

#define	PCIB_BAR1_IO_SPACE_BASE			0x20000000


#define	PCI_MEMORY_SPACE_BASE			0xB0000000

#define	PCI_IO_SPACE_BASE			0xA8000000


#define	PCI_MAX_BUS_NUM				0x01
#define	PCI_MAX_DEVICE_NUM			0x14
#define	PCI_MAX_FUNCTION_NUM			0x01
#define	PCI_MAX_REG_NUM				0x3C


#define	PCI_MAX_DEVICE_TYPE_NUM			0x13
#define	PCI_MAX_BAR_NUM				0x06


#define	PCI_CSH_VENDOR_ID_REG_ADDR		0x00
#define	PCI_CSH_DEVICE_ID_REG_ADDR		0x02
#define	PCI_CSH_COMMAND_REG_ADDR		0x04
#define	PCI_CSH_STATUS_REG_ADDR			0x06
#define	PCI_CSH_REVISION_CLASS_REG_ADDR		0x08
#define	PCI_CSH_CACHE_LINE_SIZE_REG_ADDR	0x0C
#define	PCI_CSH_LATENCY_TIMER_REG_ADDR		0x0D
#define	PCI_CSH_HEADER_TYPE_REG_ADDR		0x0E
#define	PCI_CSH_BIST_REG_ADDR			0x0F
#define	PCI_CSH_BAR_REG_ADDR			0x10


#define	PCI_IO_SPACE_SIZE_1M			0x00
#define	PCI_IO_SPACE_SIZE_2M			0x01
#define	PCI_IO_SPACE_SIZE_4M			0x02
#define	PCI_IO_SPACE_SIZE_8M			0x03
#define	PCI_IO_SPACE_SIZE_16M			0x04
#define	PCI_IO_SPACE_SIZE_32M			0x05
#define	PCI_IO_SPACE_SIZE_64M			0x06
#define	PCI_IO_SPACE_SIZE_128M			0x07
#define	PCI_IO_SPACE_SIZE_256M			0x08
#define	PCI_IO_SPACE_SIZE_512M			0x09
#define	PCI_IO_SPACE_SIZE_1G			0x0A
#define	PCI_IO_SPACE_SIZE_2G			0x0B


#define	PCI_MEMORY_SPACE_TYPE			0
#define	PCI_IO_SPACE_TYPE			1

#define	PCI_BROKEN_FLAG				1
#define	PCI_AHB2PCIB_FLAG			2


#endif	// end of #ifndef _STAR_PCI_DRIDGE_H_

