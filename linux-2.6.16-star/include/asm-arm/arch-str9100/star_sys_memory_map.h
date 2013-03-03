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


#ifndef	_STAR_SYS_MEMORY_MAP_H_
#define	_STAR_SYS_MEMORY_MAP_H_


#if 0
#define __UBOOT__
#else
#define __LINUX__
#endif


/*
 * sytem memory	mapping	after reset
 */
#define	SYSPA_FLASH_BASE_ADDR				0x10000000
#define	SYSPA_DDR_SDRAM_BASE_ADDR			0x20000000
#define	SYSPA_SMC_BASE_ADDR				0x30000000
#define	SYSPA_DDR_SDRAM_CONTROLLER_BASE_ADDR		0x40000000
#define	SYSPA_DMAC_BASE_ADDR				0x60000000
#define SYSPA_GSW_BASE_ADDR				0x70000000
#define SYSPA_HNAT_SRAM_BASE_ADDR			0x70001000
#define SYSPA_MISC_BASE_ADDR				0x76000000
#define SYSPA_POWER_MANAGEMENT_BASE_ADDR		0x77000000
#define SYSPA_UART_BASE_ADDR				0x78000000
#define SYSPA_TIMER_BASE_ADDR				0x79000000
#define SYSPA_WATCHDOG_TIMER_BASE_ADDR			0x7A000000
#define SYSPA_RTC_BASE_ADDR				0x7B000000
#define SYSPA_GPIO_BASE_ADDR				0x7C000000
#define SYSPA_INTC_BASE_ADDR				0x7D000000
#define SYSPA_PCMCIA_CONTROLLER_BASE_ADDR		0x80000000
#define SYSPA_PCMCIA_ATTRIBUTE_MEMORY_BASE_ADDR		0x90000000
#define SYSPA_PCMCIA_COMMON_MEMORY_BASE_ADDR		0x94000000
#define	SYSPA_PCI_BRIDGE_CONFIG_DATA_BASE_ADDR		0xA0000000
#define	SYSPA_PCI_BRIDGE_CONFIG_ADDR_BASE_ADDR		0xA4000000
#define	SYSPA_PCI_IO_SPACE_BASE_ADDR			0xA8000000
#define	SYSPA_PCI_MEMORY_SPACE_BASE_ADDR		0xB0000000
#define	SYSPA_USB11_CONFIG_BASE_ADDR			0xC0000000
#define	SYSPA_USB11_OPERATION_BASE_ADDR			0xC4000000
#define	SYSPA_USB20_CONFIG_BASE_ADDR			0xC8000000
#define	SYSPA_USB20_OPERATION_BASE_ADDR			0xCC000000

#if defined(__LINUX__)
#define	SYSVA_SMC_BASE_ADDR				0xFFF00000
#define	SYSVA_DDR_SDRAM_CONTROLLER_BASE_ADDR		0xFFF01000
#define	SYSVA_DMAC_BASE_ADDR				0xFFF02000
#define SYSVA_GSW_BASE_ADDR				0xFFF03000
#define SYSVA_HNAT_SRAM_BASE_ADDR			0xFFF04000
#define SYSVA_MISC_BASE_ADDR				0xFFF05000
#define SYSVA_POWER_MANAGEMENT_BASE_ADDR		0xFFF06000
#define SYSVA_UART_BASE_ADDR				0xFFF07000
#define SYSVA_TIMER_BASE_ADDR				0xFFF08000
#define SYSVA_WATCHDOG_TIMER_BASE_ADDR			0xFFF09000
#define SYSVA_RTC_BASE_ADDR				0xFFF0A000
#define SYSVA_GPIO_BASE_ADDR				0xFFF0B000
#define SYSVA_INTC_BASE_ADDR				0xFFF0C000
#define SYSVA_PCMCIA_CONTROLLER_BASE_ADDR		0xFFF0D000
#define	SYSVA_PCI_BRIDGE_CONFIG_DATA_BASE_ADDR		0xFFF0E000
#define	SYSVA_PCI_BRIDGE_CONFIG_ADDR_BASE_ADDR		0xFFF0F000
#define	SYSVA_USB11_CONFIG_BASE_ADDR			0xFFF10000
#define	SYSVA_USB11_OPERATION_BASE_ADDR			0xFFF11000
#define	SYSVA_USB20_CONFIG_BASE_ADDR			0xFFF12000
#define	SYSVA_USB20_OPERATION_BASE_ADDR			0xFFF13000
#endif


#endif // end of #ifndef _STAR_SYS_MEMORY_MAP_H_

