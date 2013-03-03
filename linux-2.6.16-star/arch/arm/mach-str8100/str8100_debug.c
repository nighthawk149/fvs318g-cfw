/*******************************************************************************
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


#include <linux/types.h>
#include <asm/hardware.h>

void debug_puts(const char *s)
{
	while (*s) {
		volatile unsigned int status = 0;
		do {
			status = _UART_LSR(0);
		} while (!((status & THR_EMPTY) == THR_EMPTY));

		_UART_THR(0) = *s;

		if (*s == '\n') {
			do {
				status = _UART_LSR(0);
			} while (!((status & THR_EMPTY) == THR_EMPTY));

			_UART_THR(0) = '\r';
		}
		s++;
	}
}

