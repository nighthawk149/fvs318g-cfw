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


#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#if 0
#define TASK_SIZE		(0xC0000000UL)	
#define TASK_SIZE_26		(0x04000000UL)
#define TASK_UNMAPPED_BASE	(TASK_SIZE / 3)
#endif

#define CONFIG_SYSTEM_DRAM_BASE	0x00000000
#if defined(CONFIG_STR8100_DRAM_64M)
#define CONFIG_SYSTEM_DRAM_SIZE 64
#elif defined(CONFIG_STR8100_DRAM_32M)
#define CONFIG_SYSTEM_DRAM_SIZE 32
#elif defined(CONFIG_STR8100_DRAM_16M)
#define CONFIG_SYSTEM_DRAM_SIZE 16
#endif
#define MEM_SIZE		(CONFIG_SYSTEM_DRAM_SIZE)
#define END_MEM			(CONFIG_SYSTEM_DRAM_BASE + CONFIG_SYSTEM_DRAM_SIZE)
#define DMA_SIZE		0xffffffff
#define PHYS_OFFSET		(CONFIG_SYSTEM_DRAM_BASE) // physical start address of memory
#if 0
#define PAGE_OFFSET 		(0xC0000000UL)
#endif

#define __virt_to_bus(x)	((x) - PAGE_OFFSET)
#define __bus_to_virt(x)	((x) + PAGE_OFFSET)

#if 0
#define __virt_to_phys__is_a_macro
#define __virt_to_phys(vpage)	((vpage) - PAGE_OFFSET)

#define __phys_to_virt__is_a_macro
#define __phys_to_virt(ppage)	((ppage) + PAGE_OFFSET)

#define __virt_to_bus__is_a_macro
#define __virt_to_bus(x)	((x) - PAGE_OFFSET)

#define __bus_to_virt__is_a_macro
#define __bus_to_virt(x)	((x) + PAGE_OFFSET)
#endif

#endif
