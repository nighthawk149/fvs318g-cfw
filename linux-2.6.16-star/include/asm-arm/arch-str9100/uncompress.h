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

#ifndef __ASM_ARCH_UNCOMPRESS_H__
#define __ASM_ARCH_UNCOMPRESS_H__

#include <asm/arch/star_uart.h>

static void putstr(const char *s)
{
	while (*s) {
		volatile unsigned int status = 0;

		do {
			status = __UART_LSR;
		} while (!((status & THR_EMPTY) == THR_EMPTY));

		__UART_THR = *s;

		if (*s == '\n') {
			do {
				status = __UART_LSR;
			} while (!((status & THR_EMPTY) == THR_EMPTY));
			__UART_THR = '\r';
		}
		s++;
	}
}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif
