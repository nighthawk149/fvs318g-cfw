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


#ifndef	_STAR_SMC_H_
#define	_STAR_SMC_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	SMC_MEM_MAP_VALUE(reg_offset)	(*((u32 volatile *)(SYSPA_SMC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	SMC_MEM_MAP_VALUE(reg_offset)	(*((u32 volatile *)(SYSVA_SMC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif



/*
 * Static Memory Controller Registers
 */
#define	SMC_MEM_BANK0_CONFIG_REG	SMC_MEM_MAP_VALUE(0x00)
#define	SMC_MEM_BANK0_TIMING_REG	SMC_MEM_MAP_VALUE(0x04)
#define	SMC_MEM_BANK1_CONFIG_REG	SMC_MEM_MAP_VALUE(0x08)
#define	SMC_MEM_BANK1_TIMING_REG	SMC_MEM_MAP_VALUE(0x0C)
#define	SMC_MEM_BANK2_CONFIG_REG	SMC_MEM_MAP_VALUE(0x10)
#define	SMC_MEM_BANK2_TIMING_REG	SMC_MEM_MAP_VALUE(0x14)
#define	SMC_MEM_BANK3_CONFIG_REG	SMC_MEM_MAP_VALUE(0x18)
#define	SMC_MEM_BANK3_TIMING_REG	SMC_MEM_MAP_VALUE(0x1C)

/*
 * macros declarations
 */

#endif	// end of #ifndef _STAR_SMC_H_
