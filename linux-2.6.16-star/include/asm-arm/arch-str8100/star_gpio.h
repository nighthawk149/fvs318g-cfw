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


#ifndef	_STAR_GPIO_H_
#define	_STAR_GPIO_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	GPIOA_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSPA_GPIOA_BASE_ADDR + reg_offset)))
#define	GPIOB_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSPA_GPIOB_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	GPIOA_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSVA_GPIOA_BASE_ADDR + reg_offset)))
#define	GPIOB_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSVA_GPIOB_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * For GPIO Set	A
 */
#define	GPIOA_DATA_OUTPUT_REG			GPIOA_MEM_MAP_VALUE(0x00)
#define	GPIOA_DATA_INPUT_REG			GPIOA_MEM_MAP_VALUE(0x04)
#define	GPIOA_DIRECTION_REG			GPIOA_MEM_MAP_VALUE(0x08)

#define	GPIOA_DATA_BIT_SET_REG			GPIOA_MEM_MAP_VALUE(0x10)
#define	GPIOA_DATA_BIT_CLEAR_REG		GPIOA_MEM_MAP_VALUE(0x14)

#define	GPIOA_INTERRUPT_ENABLE_REG		GPIOA_MEM_MAP_VALUE(0x20)
#define	GPIOA_INTERRUPT_RAW_STATUS_REG		GPIOA_MEM_MAP_VALUE(0x24)
#define	GPIOA_INTERRUPT_MASKED_STATUS_REG	GPIOA_MEM_MAP_VALUE(0x28)
#define	GPIOA_INTERRUPT_MASK_REG		GPIOA_MEM_MAP_VALUE(0x2C)
#define	GPIOA_INTERRUPT_CLEAR_REG		GPIOA_MEM_MAP_VALUE(0x30)
#define	GPIOA_INTERRUPT_TRIGGER_METHOD_REG	GPIOA_MEM_MAP_VALUE(0x34)
#define	GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG	GPIOA_MEM_MAP_VALUE(0x38)
#define	GPIOA_INTERRUPT_TRIGGER_TYPE_REG	GPIOA_MEM_MAP_VALUE(0x3C)

#define	GPIOA_BOUNCE_ENABLE_REG			GPIOA_MEM_MAP_VALUE(0x40)
#define	GPIOA_BOUNCE_CLOCK_PRESCALE_REG		GPIOA_MEM_MAP_VALUE(0x44)


/*
 * For GPIO Set	B
 */
#define	GPIOB_DATA_OUTPUT_REG			GPIOB_MEM_MAP_VALUE(0x00)
#define	GPIOB_DATA_INPUT_REG			GPIOB_MEM_MAP_VALUE(0x04)
#define	GPIOB_DIRECTION_REG			GPIOB_MEM_MAP_VALUE(0x08)

#define	GPIOB_DATA_BIT_SET_REG			GPIOB_MEM_MAP_VALUE(0x10)
#define	GPIOB_DATA_BIT_CLEAR_REG		GPIOB_MEM_MAP_VALUE(0x14)

#define	GPIOB_INTERRUPT_ENABLE_REG		GPIOB_MEM_MAP_VALUE(0x20)
#define	GPIOB_INTERRUPT_RAW_STATUS_REG		GPIOB_MEM_MAP_VALUE(0x24)
#define	GPIOB_INTERRUPT_MASKED_STATUS_REG	GPIOB_MEM_MAP_VALUE(0x28)
#define	GPIOB_INTERRUPT_MASK_REG		GPIOB_MEM_MAP_VALUE(0x2C)
#define	GPIOB_INTERRUPT_CLEAR_REG		GPIOB_MEM_MAP_VALUE(0x30)
#define	GPIOB_INTERRUPT_TRIGGER_METHOD_REG	GPIOB_MEM_MAP_VALUE(0x34)
#define	GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG	GPIOB_MEM_MAP_VALUE(0x38)
#define	GPIOB_INTERRUPT_TRIGGER_TYPE_REG	GPIOB_MEM_MAP_VALUE(0x3C)

#define	GPIOB_BOUNCE_ENABLE_REG			GPIOB_MEM_MAP_VALUE(0x40)
#define	GPIOB_BOUNCE_CLOCK_PRESCALE_REG		GPIOB_MEM_MAP_VALUE(0x44)


/*
 * define constant macros
 */

#define	MAX_GPIO_PINS		(32)

#define	GPIO_0_MASK		(1 << 0)
#define	GPIO_1_MASK		(1 << 1)
#define	GPIO_2_MASK		(1 << 2)
#define	GPIO_3_MASK		(1 << 3)
#define	GPIO_4_MASK		(1 << 4)
#define	GPIO_5_MASK		(1 << 5)
#define	GPIO_6_MASK		(1 << 6)
#define	GPIO_7_MASK		(1 << 7)
#define	GPIO_8_MASK		(1 << 8)
#define	GPIO_9_MASK		(1 << 9)
#define	GPIO_10_MASK		(1 << 10)
#define	GPIO_11_MASK		(1 << 11)
#define	GPIO_12_MASK		(1 << 12)
#define	GPIO_13_MASK		(1 << 13)
#define	GPIO_14_MASK		(1 << 14)
#define	GPIO_15_MASK		(1 << 15)
#define	GPIO_16_MASK		(1 << 16)
#define	GPIO_17_MASK		(1 << 17)
#define	GPIO_18_MASK		(1 << 18)
#define	GPIO_19_MASK		(1 << 19)
#define	GPIO_20_MASK		(1 << 20)
#define	GPIO_21_MASK		(1 << 21)
#define	GPIO_22_MASK		(1 << 22)
#define	GPIO_23_MASK		(1 << 23)
#define	GPIO_24_MASK		(1 << 24)
#define	GPIO_25_MASK		(1 << 25)
#define	GPIO_26_MASK		(1 << 26)
#define	GPIO_27_MASK		(1 << 27)
#define	GPIO_28_MASK		(1 << 28)
#define	GPIO_29_MASK		(1 << 29)
#define	GPIO_30_MASK		(1 << 30)
#define	GPIO_31_MASK		(1 << 31)


/*
 * macro declarations for GPIO Set A
 */
#define	HAL_GPIOA_READ_DATA_OUT_STATUS(data_out_state) \
    ((data_out_state) =	(GPIOA_DATA_OUTPUT_REG))

#define	HAL_GPIOA_READ_DATA_IN_STATUS(data_in_state) \
    ((data_in_state) = (GPIOA_DATA_INPUT_REG))

#define	HAL_GPIOA_SET_DIRECTION_OUTPUT(gpio_index) \
    ((GPIOA_DIRECTION_REG) |= (gpio_index))

#define	HAL_GPIOA_SET_DIRECTION_INPUT(gpio_index) \
    ((GPIOA_DIRECTION_REG) &= ~(gpio_index))

#define	HAL_GPIOA_SET_DATA_OUT_HIGH(gpio_index)	\
    ((GPIOA_DATA_BIT_SET_REG) =	(gpio_index))

#define	HAL_GPIOA_SET_DATA_OUT_LOW(gpio_index) \
    ((GPIOA_DATA_BIT_CLEAR_REG)	= (gpio_index))

#define	HAL_GPIOA_ENABLE_INTERRUPT(gpio_index) \
    ((GPIOA_INTERRUPT_ENABLE_REG) |= (gpio_index))

#define	HAL_GPIOA_DISABLE_INTERRUPT(gpio_index)	\
    ((GPIOA_INTERRUPT_ENABLE_REG) &= ~(gpio_index))

#define	HAL_GPIOA_READ_INTERRUPT_RAW_STATUS(raw_state) \
    ((raw_state) = (GPIOA_INTERRUPT_RAW_STATUS_REG))

#define	HAL_GPIOA_READ_INTERRUPT_MASKED_STATUS(masked_raw_state) \
    ((masked_raw_state)	= (GPIOA_INTERRUPT_MASKED_STATUS_REG))

#define	HAL_GPIOA_DISABLE_INTERRUPT_MASK(gpio_index) \
    ((GPIOA_INTERRUPT_MASK_REG)	&= ~(gpio_index))

#define	HAL_GPIOA_ENABLE_INTERRUPT_MASK(gpio_index) \
    ((GPIOA_INTERRUPT_MASK_REG)	|= (gpio_index))

#define	HAL_GPIOA_CLEAR_INTERRUPT(gpio_index) \
    ((GPIOA_INTERRUPT_CLEAR_REG) = (gpio_index))

#define	HAL_GPIOA_SET_INTERRUPT_EDGE_TRIGGER_MODE(gpio_index) \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index))

#define	HAL_GPIOA_SET_INTERRUPT_LEVEL_TRIGGER_MODE(gpio_index) \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index))

#define	HAL_GPIOA_SET_INTERRUPT_SINGLE_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
}

#define	HAL_GPIOA_SET_INTERRUPT_BOTH_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG) |= (gpio_index));	\
}

#define	HAL_GPIOA_SET_INTERRUPT_SINGLE_RISING_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_TYPE_REG)	&= ~(gpio_index)); \
}

#define	HAL_GPIOA_SET_INTERRUPT_SINGLE_FALLING_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_TYPE_REG)	|= (gpio_index)); \
}

#define	HAL_GPIOA_SET_INTERRUPT_HIGH_LEVEL_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_TYPE_REG)	&= ~(gpio_index)); \
}

#define	HAL_GPIOA_SET_INTERRUPT_LOW_LEVEL_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOA_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index)); \
    ((GPIOA_INTERRUPT_TRIGGER_TYPE_REG)	|= (gpio_index)); \
}

#define	HAL_GPIOA_ENABLE_BOUNCE(gpio_index) \
    ((GPIOA_BOUNCE_ENABLE_REG) |= (gpio_index))

#define	HAL_GPIOA_DISABLE_BOUNCE(gpio_index) \
    ((GPIOA_BOUNCE_ENABLE_REG) &= ~(gpio_index))

#define	HAL_GPIOA_READ_BOUNCE_PRESCALE_RATIO(prescale_ratio) \
    ((prescale_ratio) =	((GPIOA_BOUNCE_CLOCK_PRESCALE_REG) & 0x0000FFFF))

#define	HAL_GPIOA_WRITE_BOUNCE_PRESCALE_RATIO(prescale_ratio) \
    ((GPIOA_BOUNCE_CLOCK_PRESCALE_REG) = (prescale_ratio & 0x0000FFFF))



/*
 * macro declarations for GPIO Set B
 */
#define	HAL_GPIOB_READ_DATA_OUT_STATUS(data_out_state) \
    ((data_out_state) =	(GPIOB_DATA_OUTPUT_REG))

#define	HAL_GPIOB_READ_DATA_IN_STATUS(data_in_state) \
    ((data_in_state) = (GPIOB_DATA_INPUT_REG))

#define	HAL_GPIOB_SET_DIRECTION_OUTPUT(gpio_index) \
    ((GPIOB_DIRECTION_REG) |= (gpio_index))

#define	HAL_GPIOB_SET_DIRECTION_INPUT(gpio_index) \
    ((GPIOB_DIRECTION_REG) &= ~(gpio_index))

#define	HAL_GPIOB_SET_DATA_OUT_HIGH(gpio_index)	\
    ((GPIOB_DATA_BIT_SET_REG) =	(gpio_index))

#define	HAL_GPIOB_SET_DATA_OUT_LOW(gpio_index) \
    ((GPIOB_DATA_BIT_CLEAR_REG)	= (gpio_index))

#define	HAL_GPIOB_ENABLE_INTERRUPT(gpio_index) \
    ((GPIOB_INTERRUPT_ENABLE_REG) |= (gpio_index))

#define	HAL_GPIOB_DISABLE_INTERRUPT(gpio_index)	\
    ((GPIOB_INTERRUPT_ENABLE_REG) &= ~(gpio_index))

#define	HAL_GPIOB_READ_INTERRUPT_RAW_STATUS(raw_state) \
    ((raw_state) = (GPIOB_INTERRUPT_RAW_STATUS_REG))

#define	HAL_GPIOB_READ_INTERRUPT_MASKED_STATUS(masked_raw_state) \
    ((masked_raw_state)	= (GPIOB_INTERRUPT_MASKED_STATUS_REG))

#define	HAL_GPIOB_DISABLE_INTERRUPT_MASK(gpio_index) \
    ((GPIOB_INTERRUPT_MASK_REG)	&= ~(gpio_index))

#define	HAL_GPIOB_ENABLE_INTERRUPT_MASK(gpio_index) \
    ((GPIOB_INTERRUPT_MASK_REG)	|= (gpio_index))

#define	HAL_GPIOB_CLEAR_INTERRUPT(gpio_index) \
    ((GPIOB_INTERRUPT_CLEAR_REG) = (gpio_index))

#define	HAL_GPIOB_SET_INTERRUPT_EDGE_TRIGGER_MODE(gpio_index) \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index))

#define	HAL_GPIOB_SET_INTERRUPT_LEVEL_TRIGGER_MODE(gpio_index) \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index))

#define	HAL_GPIOB_SET_INTERRUPT_SINGLE_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
}

#define	HAL_GPIOB_SET_INTERRUPT_BOTH_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG) |= (gpio_index));	\
}

#define	HAL_GPIOB_SET_INTERRUPT_SINGLE_RISING_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_TYPE_REG)	&= ~(gpio_index)); \
}

#define	HAL_GPIOB_SET_INTERRUPT_SINGLE_FALLING_EDGE_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG) &= ~(gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_TYPE_REG)	|= (gpio_index)); \
}

#define	HAL_GPIOB_SET_INTERRUPT_HIGH_LEVEL_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_TYPE_REG)	&= ~(gpio_index)); \
}

#define	HAL_GPIOB_SET_INTERRUPT_LOW_LEVEL_TRIGGER_MODE(gpio_index) \
{ \
    ((GPIOB_INTERRUPT_TRIGGER_METHOD_REG) |= (gpio_index)); \
    ((GPIOB_INTERRUPT_TRIGGER_TYPE_REG)	|= (gpio_index)); \
}

#define	HAL_GPIOB_ENABLE_BOUNCE(gpio_index) \
    ((GPIOB_BOUNCE_ENABLE_REG) |= (gpio_index))

#define	HAL_GPIOB_DISABLE_BOUNCE(gpio_index) \
    ((GPIOB_BOUNCE_ENABLE_REG) &= ~(gpio_index))

#define	HAL_GPIOB_READ_BOUNCE_PRESCALE_RATIO(prescale_ratio) \
    ((prescale_ratio) =	((GPIOB_BOUNCE_CLOCK_PRESCALE_REG) & 0x0000FFFF))

#define	HAL_GPIOB_WRITE_BOUNCE_PRESCALE_RATIO(prescale_ratio) \
    ((GPIOB_BOUNCE_CLOCK_PRESCALE_REG) = (prescale_ratio & 0x0000FFFF))


#endif	// end of _STAR_GPIO_H_
