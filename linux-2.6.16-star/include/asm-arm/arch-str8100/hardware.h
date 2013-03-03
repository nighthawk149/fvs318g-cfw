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


#ifndef __ASM_ARCH_HARDWARE_H__
#define __ASM_ARCH_HARDWARE_H__

#include <linux/config.h>
#include <asm/arch/param.h>

#include <asm/arch/star_sys_memory_map.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_timer.h>
#include <asm/arch/star_uart.h>
#include <asm/arch/star_gpio.h>
#include <asm/arch/star_pci_bridge.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_rtc.h>
#include <asm/arch/star_wdtimer.h>
#include <asm/arch/star_smc.h>
#include <asm/arch/star_nic.h>
#include <asm/arch/star_ide.h>

#if 1 // on ASIC
#define SYS_CLK			(87500000) // 87.5MHz
#define UART_CLK		(24000000) // 24MHz
#define AHB_CLK			(SYS_CLK)
#define APB_CLK			(AHB_CLK >> 1)
#define TIMER_COUNTER		(APB_CLK / HZ)
#else // on FPGA
#define SYS_CLK			(13000000) // 13MHz
#define UART_CLK		(13000000) // 13Mhz
#define AHB_CLK			(SYS_CLK)
#define APB_CLK			(AHB_CLK)
#define TIMER_COUNTER		(APB_CLK / HZ)
#endif

#define DEBUG_CONSOLE		(0)

#define FLASH_BASE_ADDR		SYSPA_FLASH_SRAM_BANK0_BASE_ADDR
#define FLASH_SIZE		0x800000

#define PCIBIOS_MIN_IO		0x00000000
#define PCIBIOS_MIN_MEM		0x00000000
#if 1
#define pcibios_assign_all_busses()	0
#else
#define pcibios_assign_all_busses()	1
#endif

#endif /* __ASM_ARCH_HARDWARE_H__ */
