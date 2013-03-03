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


#ifndef	_STAR_INTC_H_
#define	_STAR_INTC_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	INTC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSPA_INTC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	INTC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSVA_INTC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


#define	INTC_INTERRUPT_SOURCE_REG		INTC_MEM_MAP_VALUE(0x00)
#define	INTC_INTERRUPT_MASK_REG			INTC_MEM_MAP_VALUE(0x04)
#define	INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG	INTC_MEM_MAP_VALUE(0x08)
#define	INTC_INTERRUPT_TRIGGER_MODE_REG		INTC_MEM_MAP_VALUE(0x0C)
#define	INTC_INTERRUPT_TRIGGER_LEVEL_REG	INTC_MEM_MAP_VALUE(0x10)
#define	INTC_INTERRUPT_STATUS_REG		INTC_MEM_MAP_VALUE(0x14)
#define	INTC_FIQ_MODE_SELECT_REG		INTC_MEM_MAP_VALUE(0x18)
#define	INTC_SOFTWARE_INTERRUPT_REG		INTC_MEM_MAP_VALUE(0x1C)


/*
 * define constants macros
 */
#define	INTC_TIMER1_BIT_INDEX			0
#define	INTC_TIMER2_BIT_INDEX			1
                                        	
#define	INTC_CLOCK_SCALE_BIT_INDEX		2
                                        	
#define	INTC_WATCHDOG_TIMER_BIT_INDEX		3
                                        	
#define	INTC_GPIO_EXTERNAL_INT_BIT_INDEX	4
                                        	
#define	INTC_PCI_INTA_BIT_INDEX			5
#define	INTC_PCI_INTB_BIT_INDEX			6
#define	INTC_PCI_INTC_BIT_INDEX			7
#define	INTC_PCI_AHB2BRIDGE_BIT_INDEX		8
#define	INTC_PCI_ARBITOR_BIT_INDEX		9
                                        	
#define	INTC_UART_BIT_INDEX			10
                                        	
#define	INTC_GDMAC_TC_BIT_INDEX			11
#define	INTC_GDMAC_ERROR_BIT_INDEX		12
                                        	
#define	INTC_PCMCIA_BRIDGE_BIT_INDEX		13
                                        	
#define	INTC_RTC_BIT_INDEX			14
                                        	
#define	INTC_EXTERNAL_INT_BIT_INDEX		15
                                        	
#define	INTC_IDP_FINISH_INT_BIT_INDEX		16
#define	INTC_IDP_QEMPTY_INT_BIT_INDEX		17
                                        	
#define	INTC_GSW_STATUS_BIT_INDEX		18
#define	INTC_GSW_TSTC_BIT_INDEX			19
#define	INTC_GSW_FSRC_BIT_INDEX			20
#define	INTC_GSW_TSQE_BIT_INDEX			21
#define	INTC_GSW_FSQF_BIT_INDEX			22
                                        	
#define	INTC_USB11_BIT_INDEX			23
#define	INTC_USB20_BIT_INDEX			24



/*
 * define interrupt type
 */
#define	INTC_IRQ_INTERRUPT			0
#define	INTC_FIQ_INTERRUPT			1

/*
 * define interrupt trigger mode
 */
#define	INTC_LEVEL_TRIGGER			0
#define	INTC_EDGE_TRIGGER			1

/*
 * define rising/falling edge for edge trigger mode
 */
#define	INTC_RISING_EDGE			0
#define	INTC_FALLING_EDGE			1

/*
 * define active High/Low for level trigger mode
 */
#define	INTC_ACTIVE_HIGH			0
#define	INTC_ACTIVE_LOW				1


#define	INTC_MAX_IRQ_SOURCES			25
#define	INTC_MAX_FIQ_SOURCES			25


/*
 * macro declarations
 */
#define	HAL_INTC_READ_INTERRUPT_SOURCE(int_source) \
{ \
    (int_source) = (INTC_INTERRUPT_SOURCE_REG); \
}


#define	HAL_INTC_READ_INTERRUPT_MASK(int_mask) \
{ \
    (int_mask) = (INTC_INTERRUPT_MASK_REG); \
}


#define	HAL_INTC_WRITE_INTERRUPT_MASK(int_mask)	\
{ \
    (INTC_INTERRUPT_MASK_REG) = (int_mask); \
}


#define	HAL_INTC_ENABLE_INTERRUPT_SOURCE(source_bit_index) \
{ \
    (INTC_INTERRUPT_MASK_REG) &= (~(1 << source_bit_index)); \
}


#define	HAL_INTC_DISABLE_INTERRUPT_SOURCE(source_bit_index) \
{ \
    (INTC_INTERRUPT_MASK_REG) |= (1 << source_bit_index); \
}


#define	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(source_bit_index)	\
{ \
    (INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG) |= (1 << source_bit_index); \
}


#define	HAL_INTC_SET_EDGE_TRIGGER_MODE(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_MODE_REG) |= (1 << source_bit_index);\
}


#define	HAL_INTC_SET_LEVEL_TRIGGER_MODE(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_MODE_REG) &= (~(1 << source_bit_index)); \
}


#define	HAL_INTC_SET_RISING_EDGE_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) &= (~(1 << source_bit_index)); \
}


#define	HAL_INTC_SET_FALLING_EDGE_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) |= (1 << source_bit_index); \
}


#define	HAL_INTC_SET_ACTIVE_HIGH_TRIGGER_LEVEL(source_bit_index) \
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) &= (~(1 << source_bit_index));\
}


#define	HAL_INTC_SET_ACTIVE_LOW_TRIGGER_LEVEL(source_bit_index)	\
{ \
    (INTC_INTERRUPT_TRIGGER_LEVEL_REG) |= ((1 << source_bit_index)); \
}


#define	HAL_INTC_ASSIGN_INTERRUPT_TO_IRQ(source_bit_index) \
{ \
    (INTC_FIQ_MODE_SELECT_REG) &= (~(1 << source_bit_index)); \
}


#define	HAL_INTC_ASSIGN_INTERRUPT_TO_FIQ(source_bit_index) \
{ \
    (INTC_FIQ_MODE_SELECT_REG) |= (1 << source_bit_index); \
}


#endif	// end of #ifndef _STAR_INTC_H_

