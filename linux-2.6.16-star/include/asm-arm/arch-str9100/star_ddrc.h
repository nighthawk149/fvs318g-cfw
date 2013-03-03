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


#ifndef	_STAR_DDRC_H_
#define	_STAR_DDRC_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	DDRC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSPA_DDR_SDRAM_CONTROLLER_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	DDRC_MEM_MAP_VALUE(reg_offset)		(*((u32	volatile *)(SYSVA_DDR_SDRAM_CONTROLLER_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


#define	DDRC_TIMIMG_PARAMETER0_REG		DDRC_MEM_MAP_VALUE(0x00)
#define	DDRC_TIMIMG_PARAMETER1_REG		DDRC_MEM_MAP_VALUE(0x04)
#define	DDRC_CONFIGURATION_REG			DDRC_MEM_MAP_VALUE(0x08)
#define	DDRC_BANK_CONFIGURATION_REG		DDRC_MEM_MAP_VALUE(0x0C)
#define	DDRC_PAD_POWER_DOWN_REG			DDRC_MEM_MAP_VALUE(0x10)
#define	DDRC_DQS_OUTPUT_DELAY_REG		DDRC_MEM_MAP_VALUE(0x14)
#define	DDRC_DQS_INPUT_DELAY_REG		DDRC_MEM_MAP_VALUE(0x18)
#define	DDRC_PREREAD_ENABLE_REG			DDRC_MEM_MAP_VALUE(0x1C)
#define	DDRC_CHANNEL1_PREREAD_TIMEOUT_REG	DDRC_MEM_MAP_VALUE(0x20)
#define	DDRC_CHANNEL7_PREREAD_TIMEOUT_REG	DDRC_MEM_MAP_VALUE(0x24)
#define	DDRC_CHANNEL4_PREREAD_TIMEOUT_REG	DDRC_MEM_MAP_VALUE(0x28)
#define	DDRC_CHANNEL6_PREREAD_TIMEOUT_REG	DDRC_MEM_MAP_VALUE(0x2C)


/*
 * define constants macros
 */
#define	CPU_PREREAD_BIT_INDEX			0
#define	GDMA_PREREAD_BIT_INDEX			1
#define	GSW_DMA_RX_PREREAD_BIT_INDEX		2
#define	PCI_PREREAD_BIT_INDEX			3
#define	USB20_PREREAD_BIT_INDEX			4
#define	USB11_PREREAD_BIT_INDEX			5
#define	IDP_PREREAD_BIT_INDEX			6
#define	GSW_DMA_TX_PREREAD_BIT_INDEX		7


/*
 * macro declarations
 */
#define	HAL_DDRC_READ_TIMING_PARAMETER0(timing_parameter0) \
    ((timing_parameter0) = (DDRC_TIMIMG_PARAMETER0_REG))


#define	HAL_DDRC_WRITE_TIMING_PARAMETER0(timing_parameter0) \
    ((DDRC_TIMIMG_PARAMETER0_REG) = (timing_parameter0))


#define	HAL_DDRC_ENABLE_DLL() \
    ((DDRC_TIMIMG_PARAMETER0_REG) &= ~(0x1 << 21))


#define	HAL_DDRC_DISABLE_DLL() \
    ((DDRC_TIMIMG_PARAMETER0_REG) |= (0x1 << 21))


#define	HAL_DDRC_READ_AUTO_REFRESH_COMMAND_CONFIG(auto_refresh_config) \
    ((auto_refresh_config) = (DDRC_TIMIMG_PARAMETER1_REG))


#define	HAL_DDRC_WRITE_AUTO_REFRESH_COMMAND_CONFIG(auto_refresh_config)	\
    ((DDRC_TIMIMG_PARAMETER1_REG) = (auto_refresh_config))


#define	HAL_DDRC_READ_PREREAD_ENABLE_CONFIG(preread_enable_config) \
    ((preread_enable_config) = (DDRC_PREREAD_ENABLE_REG))


#define	HAL_DDRC_WRITE_PREREAD_ENABLE_CONFIG(preread_enable_config) \
    ((DDRC_PREREAD_ENABLE_REG) = (preread_enable_config))


#define	HAL_DDRC_ENABLE_DMA_PREREAD() \
    ((DDRC_PREREAD_ENABLE_REG) |= (0x1 << GDMA_PREREAD_BIT_INDEX))


#define	HAL_DDRC_DISABLE_DMA_PREREAD() \
    ((DDRC_PREREAD_ENABLE_REG) &= ~(0x1 << GDMA_PREREAD_BIT_INDEX))


#define	HAL_DDRC_ENABLE_GSW_DMA_PREREAD() \
{ \
    ((DDRC_PREREAD_ENABLE_REG) |= (0x1 << GSW_DMA_RX_PREREAD_BIT_INDEX)); \
    ((DDRC_PREREAD_ENABLE_REG) |= (0x1 << GSW_DMA_TX_PREREAD_BIT_INDEX)); \
}


#define	HAL_DDRC_DISABLE_GSW_DMA_PREREAD() \
{ \
    ((DDRC_PREREAD_ENABLE_REG) &= ~(0x1 << GSW_DMA_RX_PREREAD_BIT_INDEX)); \
    ((DDRC_PREREAD_ENABLE_REG) &= ~(0x1 << GSW_DMA_TX_PREREAD_BIT_INDEX)); \
}


#define	HAL_DDRC_ENABLE_USB20_PREREAD()	\
    ((DDRC_PREREAD_ENABLE_REG) |= (0x1 << USB20_PREREAD_BIT_INDEX))


#define	HAL_DDRC_DISABLE_USB20_PREREAD() \
    ((DDRC_PREREAD_ENABLE_REG) &= ~(0x1 << USB20_PREREAD_BIT_INDEX))


#define	HAL_DDRC_ENABLE_IDP_PREREAD() \
    ((DDRC_PREREAD_ENABLE_REG) |= (0x1 << IDP_PREREAD_BIT_INDEX))


#define	HAL_DDRC_DISABLE_IDP_PREREAD() \
    ((DDRC_PREREAD_ENABLE_REG) &= ~(0x1 << IDP_PREREAD_BIT_INDEX))


#endif	// end of #ifndef _STAR_DDRC_H_

