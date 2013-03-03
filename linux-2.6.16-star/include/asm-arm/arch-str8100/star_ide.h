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


#ifndef	_STAR_IDE_H_
#define	_STAR_IDE_H_


#include "star_sys_memory_map.h"


#if defined(__UBOOT__)
#define	IDE_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_IDE_CONTROLLER_BASE_ADDR + reg_offset)))
#define	IDE_BUS_MEM_MAP_VALUE(reg_offset)	(*((u8 volatile *)(SYSPA_IDE_DEVICE_BASE_ADDR + reg_offset)))
#define IDE_DATA_MEM_MAP_VALUE(reg_offset)	(*((u16 volatile *)(SYSPA_IDE_DEVICE_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	IDE_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_IDE_CONTROLLER_BASE_ADDR + reg_offset)))
#define	IDE_BUS_MEM_MAP_VALUE(reg_offset)	(*((u8 volatile *)(SYSVA_IDE_DEVICE_BASE_ADDR + reg_offset)))
#define IDE_DATA_MEM_MAP_VALUE(reg_offset)	(*((u16 volatile *)(SYSVA_IDE_DEVICE_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * IDE Controller Registers
 */
#define IDE_PIO_CONTROL_REG			IDE_MEM_MAP_VALUE(0x00)
#define IDE_DRIVE0_PIO_TIMING_CONFIG_REG	IDE_MEM_MAP_VALUE(0x04)
#define IDE_DRIVE1_PIO_TIMING_CONFIG_REG	IDE_MEM_MAP_VALUE(0x08)
#define IDE_DRIVE0_DMA_TIMING_CONFIG_REG	IDE_MEM_MAP_VALUE(0x0C)
#define IDE_DRIVE1_DMA_TIMING_CONFIG_REG	IDE_MEM_MAP_VALUE(0x10)
#define IDE_UDMA_TIMING_CONFIG_REG		IDE_MEM_MAP_VALUE(0x14)
#define IDE_DMA_UDMA_CONTROL_REG		IDE_MEM_MAP_VALUE(0x18)
#define IDE_STATUS_CONTROL_REG			IDE_MEM_MAP_VALUE(0x1C)
#define IDE_BUS_MASTER_DTP_REG			IDE_MEM_MAP_VALUE(0x20)
#define IDE_FAST_PATH_ACCESS_WINDOW_REG		IDE_MEM_MAP_VALUE(0x24)
#define IDE_FAST_PATH_DMA_BURST_SIZE_REG	IDE_MEM_MAP_VALUE(0x28)


/*
 * IDE Command Block Registers
 */
#define _IDE_DATA_REG				IDE_DATA_MEM_MAP_VALUE(0x20)
#define _IDE_ERROR_REG				IDE_BUS_MEM_MAP_VALUE(0x24)
#define _IDE_FEATURES_REG			IDE_BUS_MEM_MAP_VALUE(0x24)
#define _IDE_SECTOR_COUNT_REG			IDE_BUS_MEM_MAP_VALUE(0x28)
#define _IDE_LBA_LOW_REG			IDE_BUS_MEM_MAP_VALUE(0x2C)
#define _IDE_LBA_MID_REG			IDE_BUS_MEM_MAP_VALUE(0x30)
#define _IDE_LBA_HIGH_REG			IDE_BUS_MEM_MAP_VALUE(0x34)
#define _IDE_DEVICE_REG				IDE_BUS_MEM_MAP_VALUE(0x38)
#define _IDE_COMMAND_REG			IDE_BUS_MEM_MAP_VALUE(0x3C)
#define _IDE_STATUS_REG				IDE_BUS_MEM_MAP_VALUE(0x3C)


/*
 * IDE Control Block Registers
 */
#define IDE_DEVICE_CONTROL_REG			IDE_BUS_MEM_MAP_VALUE(0x40)
#define IDE_ALTERNATE_STATUS_REG		IDE_BUS_MEM_MAP_VALUE(0x40)


#define IDE_CD					(0x01)
#define IDE_IO					(0x02)
#define IDE_REL					(0x04)
#define IDE_OVL					(0x02)
#define IDE_BSY					(0x80)
#define IDE_DRQ					(0x08)
#define IDE_SERV				(0x10)
#define IDE_DMRD				(0x20)
#define IDE_ERR					(0x01)
#define IDE_SRST				(0x04)

/*
 * macro declarations for IDE Controller
 */
#define HAL_IDE_DRIVE0_IORDY_SAMPLE_ENABLE() \
{ \
    (IDE_PIO_CONTROL_REG) |= (0x1 << 0); \
}

#define HAL_IDE_DRIVE0_IORDY_SAMPLE_DISABLE() \
{ \
    (IDE_PIO_CONTROL_REG) &= ~(0x1 << 0); \
}

#define HAL_IDE_DRIVE1_IORDY_SAMPLE_ENABLE() \
{ \
    (IDE_PIO_CONTROL_REG) |= (0x1 << 1); \
}

#define HAL_IDE_DRIVE1_IORDY_SAMPLE_DISABLE() \
{ \
    (IDE_PIO_CONTROL_REG) &= ~(0x1 << 1); \
}

#define HAL_IDE_DRIVE0_UDMA_ENABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) |= (0x1 << 0); \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 2); \
}

#define HAL_IDE_DRIVE0_UDMA_DISABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 0); \
}

#define HAL_IDE_DRIVE1_UDMA_ENABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) |= (0x1 << 1); \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 3); \
}

#define HAL_IDE_DRIVE1_UDMA_DISABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 1); \
}

#define HAL_IDE_DRIVE0_DMA_ENABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) |= (0x1 << 2); \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 0); \
}

#define HAL_IDE_DRIVE0_DMA_DISABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 2); \
}

#define HAL_IDE_DRIVE1_DMA_ENABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) |= (0x1 << 3); \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 1); \
}

#define HAL_IDE_TO_USB_FAST_PATH_ENABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) |= (0x1 << 4); \
}

#define HAL_IDE_TO_USB_FAST_PATH_DISABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 4); \
}

#define HAL_IDE_DRIVE1_DMA_DISABLE() \
{ \
    (IDE_DMA_UDMA_CONTROL_REG) &= ~(0x1 << 3); \
}

#define HAL_IDE_DMA_UDMA_START() \
{ \
    (IDE_STATUS_CONTROL_REG) |= (0x1); \
}

#define HAL_IDE_DMA_UDMA_STOP() \
{ \
    (IDE_STATUS_CONTROL_REG) &= ~(0x1); \
}

#define HAL_IDE_CLEAR_PRD_INTERRUPT_STATUS() \
{ \
    (IDE_STATUS_CONTROL_REG) |= (0x1 << 2); \
}

#define HAL_IDE_CLEAR_INTRQ_INTERRUPT_STATUS() \
{ \
    (IDE_STATUS_CONTROL_REG) |= (0x1 << 1); \
}

#define HAL_IDE_HOST_TRANSFER_WRITE_OUT() \
{ \
    (IDE_STATUS_CONTROL_REG) |= (0x1 << 3); \
}

#define HAL_IDE_HOST_TRANSFER_READ_IN() \
{ \
    (IDE_STATUS_CONTROL_REG) &= ~(0x1 << 3); \
}

#define HAL_IDE_MASK_PRD_INTERRUPT() \
{ \
    (IDE_STATUS_CONTROL_REG) |= (0x1 << 6); \
}

#define HAL_IDE_UNMASK_PRD_INTERRUPT() \
{ \
    (IDE_STATUS_CONTROL_REG) &= ~(0x1 << 6); \
}

#define HAL_IDE_SET_DESCRIPTOR_TABLE_POINTER(dtp) \
{ \
    (IDE_BUS_MASTER_DTP_REG) = (dtp); \
}

#define HAL_IDE_SET_FAST_PATH_ACCESS_WINDOW(fp_access_window) \
{ \
    (IDE_FAST_PATH_ACCESS_WINDOW_REG) = (fp_access_window); \
}

/*
 * macro declarations for IDE Device
 */
#define HAL_IDE_SELECT_DEVICE_0() \
{ \
    (_IDE_DEVICE_REG) = 0; \
}

#define HAL_IDE_SELECT_DEVICE_1() \
{ \
    (_IDE_DEVICE_REG) = (0x1 << 4); \
}

#define HAL_IDE_ENABLE_DEVICE_INTRQ() \
{ \
    (IDE_DEVICE_CONTROL_REG) = (0); \
}

#define HAL_IDE_DISABLE_DEVICE_INTRQ() \
{ \
    (IDE_DEVICE_CONTROL_REG) = (0x2); \
}


#endif  // end of #ifndef _STAR_IDE_H_

