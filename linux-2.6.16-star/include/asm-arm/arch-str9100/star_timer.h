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


#ifndef	_STAR_TIMER_H_
#define	_STAR_TIMER_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	TIMER_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_TIMER_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	TIMER_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_TIMER_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


#define	TIMER1_COUNTER_REG			TIMER_MEM_MAP_VALUE(0x00)
#define	TIMER1_AUTO_RELOAD_VALUE_REG		TIMER_MEM_MAP_VALUE(0x04)
#define	TIMER1_MATCH_VALUE1_REG			TIMER_MEM_MAP_VALUE(0x08)
#define	TIMER1_MATCH_VALUE2_REG			TIMER_MEM_MAP_VALUE(0x0C)

#define	TIMER2_COUNTER_REG			TIMER_MEM_MAP_VALUE(0x10)
#define	TIMER2_AUTO_RELOAD_VALUE_REG		TIMER_MEM_MAP_VALUE(0x14)
#define	TIMER2_MATCH_VALUE1_REG			TIMER_MEM_MAP_VALUE(0x18)
#define	TIMER2_MATCH_VALUE2_REG			TIMER_MEM_MAP_VALUE(0x1C)

#define	TIMER_CONTROL_REG			TIMER_MEM_MAP_VALUE(0x30)
#define	TIMER_INTERRUPT_STATUS_REG		TIMER_MEM_MAP_VALUE(0x34)
#define	TIMER_INTERRUPT_MASK_REG		TIMER_MEM_MAP_VALUE(0x38)

#define	TIMER_REVISION_REG			TIMER_MEM_MAP_VALUE(0x3C)


/*
 * define constants macros
 */
#define	TIMER1_ENABLE_BIT_INDEX			0
#define	TIMER1_CLOCK_SOURCE_BIT_INDEX		1
#define	TIMER1_OVERFLOW_ENABLE_BIT_INDEX	2

#define	TIMER2_ENABLE_BIT_INDEX			3
#define	TIMER2_CLOCK_SOURCE_BIT_INDEX		4
#define	TIMER2_OVERFLOW_ENABLE_BIT_INDEX	5

#define	TIMER1_UP_DOWN_COUNT_BIT_INDEX		9
#define	TIMER2_UP_DOWN_COUNT_BIT_INDEX		10

#define	TIMER1_MATCH1_INTERRUPT_BIT_INDEX	0
#define	TIMER1_MATCH2_INTERRUPT_BIT_INDEX	1
#define	TIMER1_OVERFLOW_INTERRUPT_BIT_INDEX	2

#define	TIMER2_MATCH1_INTERRUPT_BIT_INDEX	3
#define	TIMER2_MATCH2_INTERRUPT_BIT_INDEX	4
#define	TIMER2_OVERFLOW_INTERRUPT_BIT_INDEX	5


#define	TIMER_CLOCK_SOURCE_PCLK			0
#define	TIMER_CLOCK_SOURCE_EXT_CLK		1


#define	TIMER_OVERFLOW_MODE_DISABLE		0
#define	TIMER_OVERFLOW_MODE_ENABLE		1


#define	TIMER_COUNTER_MODE_UP			0
#define	TIMER_COUNTER_MODE_DOWN			1


#define	TIMER_1					1
#define	TIMER_2					2


#define	ACTION_READ				0
#define	ACTION_WRITE				1


#define	MATCH1_MASK_ENABLE			(1 << 0)

#define	MATCH2_MASK_ENABLE			(1 << 1)

#define	OVERFLOW_MASK_ENABLE			(1 << 2)


/*
 * macro declarations
 */
#define	HAL_TIMER_ENABLE_TIMER1() \
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER1_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_DISABLE_TIMER1() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER1_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_ENABLE_TIMER2() \
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER2_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_DISABLE_TIMER2() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER2_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_ENABLE_TIMER1_OVERFLOW_MODE()	\
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER1_OVERFLOW_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_DISABLE_TIMER1_OVERFLOW_MODE() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER1_OVERFLOW_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_ENABLE_TIMER2_OVERFLOW_MODE()	\
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER2_OVERFLOW_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_DISABLE_TIMER2_OVERFLOW_MODE() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER2_OVERFLOW_ENABLE_BIT_INDEX)); \
}


#define	HAL_TIMER_SET_TIMER1_DOWNCOUNT() \
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER1_UP_DOWN_COUNT_BIT_INDEX)); \
}


#define	HAL_TIMER_SET_TIMER1_UPCOUNT() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER1_UP_DOWN_COUNT_BIT_INDEX)); \
}


#define	HAL_TIMER_SET_TIMER2_DOWNCOUNT() \
{ \
    ((TIMER_CONTROL_REG) |= (1 << TIMER2_UP_DOWN_COUNT_BIT_INDEX)); \
}


#define	HAL_TIMER_SET_TIMER2_UPCOUNT() \
{ \
    ((TIMER_CONTROL_REG) &= ~(1 << TIMER2_UP_DOWN_COUNT_BIT_INDEX)); \
}


#define	HAL_TIMER_READ_INTERRUPT_STATUS(interrupt_status) \
{ \
    ((interrupt_status)	= (TIMER_INTERRUPT_STATUS_REG)); \
}


#define	HAL_TIMER_WRITE_INTERRUPT_STATUS(interrupt_status) \
{ \
    ((TIMER_INTERRUPT_STATUS_REG) = (interrupt_status)); \
}


#define	HAL_TIMER_READ_INTERRUPT_MASK(interrupt_mask) \
{ \
    ((interrupt_mask) =	(TIMER_INTERRUPT_MASK_REG)); \
}


#define	HAL_TIMER_WRITE_INTERRUPT_MASK(interrupt_mask) \
{ \
    ((TIMER_INTERRUPT_MASK_REG) = (interrupt_mask)); \
}


#define	HAL_TIMER_MASK_TIMER1_TIMER2_ALL_INTERRUPTS() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) = (0x3F));\
}


#define	HAL_TIMER_MASK_TIMER1_MATCH1_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER1_MATCH1_INTERRUPT_BIT_INDEX));	\
}


#define	HAL_TIMER_MASK_TIMER1_MATCH2_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER1_MATCH2_INTERRUPT_BIT_INDEX));	\
}


#define	HAL_TIMER_MASK_TIMER1_OVERFLOW_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER1_OVERFLOW_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER1_MATCH1_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER1_MATCH1_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER1_MATCH2_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER1_MATCH2_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER1_OVERFLOW_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER1_OVERFLOW_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_MASK_TIMER2_MATCH1_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER2_MATCH1_INTERRUPT_BIT_INDEX));	\
}


#define	HAL_TIMER_MASK_TIMER2_MATCH2_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER2_MATCH2_INTERRUPT_BIT_INDEX));	\
}


#define	HAL_TIMER_MASK_TIMER2_OVERFLOW_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) |= (1 << TIMER2_OVERFLOW_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER2_MATCH1_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER2_MATCH1_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER2_MATCH2_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER2_MATCH2_INTERRUPT_BIT_INDEX)); \
}


#define	HAL_TIMER_UNMASK_TIMER2_OVERFLOW_INTERRUPT() \
{ \
    ((TIMER_INTERRUPT_MASK_REG) &= ~(1 << TIMER2_OVERFLOW_INTERRUPT_BIT_INDEX)); \
}

#endif	// end of #ifndef _STAR_TIMER_H_

