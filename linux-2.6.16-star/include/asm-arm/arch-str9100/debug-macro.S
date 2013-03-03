/*******************************************************************************
 *
 *  Copyright(c) 2006 Star Semiconductor Corporation, All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  this program; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *  The full GNU General Public License is included in this distribution in the
 *  file called LICENSE.
 *
 *  Contact Information:
 *  Technology Support <tech@starsemi.com>
 *  Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ******************************************************************************/

#include <linux/config.h>
#include <linux/linkage.h>
#include <asm/hardware.h>

		.macro	addruart,rx
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1				@ MMU enabled ?
		moveq	\rx, #(SYSPA_UART_BASE_ADDR)	@ physical base address of UART0
		movne	\rx, #(SYSVA_UART_BASE_ADDR & 0xFF000000) @ virtual base address of UART0
		orrne	\rx, \rx, #(SYSVA_UART_BASE_ADDR & 0x00FF0000)
		orrne	\rx, \rx, #(SYSVA_UART_BASE_ADDR & 0x0000FF00)
		orrne	\rx, \rx, #(SYSVA_UART_BASE_ADDR & 0x000000FF)
		.endm

		.macro	senduart,rd,rx
		strb	\rd, [\rx, #0x00]
		.endm

		.macro	waituart,rd,rx
		mov	\rd, #0xf000
1001:		subs	\rd, \rd, #1
		bne	1001b	
		.endm

		.macro	busyuart,rd,rx
		nop
		mov	\rd, #0xf000
1010:		subs	\rd, \rd, #1
		bne	1010b
		.endm

