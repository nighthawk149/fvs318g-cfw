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

#include <asm/arch/star_intc.h>

		.macro	disable_fiq
		.endm

#ifdef CONFIG_VIC_INTERRUPT
		.macro get_fiqnr_and_base, irqnr, irqstat, base, tmp
		ldr		\base, =(SYSVA_VIC_BASE_ADDR + 0x140)
		ldr		\irqnr, [\base]
		.endm

		.macro get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr		\base, =(SYSVA_VIC_BASE_ADDR + 0x140)
		ldr		\irqnr, [\base]
		.endm
#else
		.macro get_fiqnr_and_base, irqnr, irqstat, base, tmp
		ldr		\base, =(SYSVA_VIC_BASE_ADDR + 0x20)
		ldr		\irqstat, [\base]
		mov		\irqnr, #0
9001:
		tst		\irqstat, #1
		bne		9002f
		add		\irqnr, \irqnr, #1
		mov		\irqstat, \irqstat, lsr #1
		cmp		\irqnr, #32
		bcc		9001b
9002:
		.endm

		.macro get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr		\base, =(SYSVA_VIC_BASE_ADDR + 0x1C)
		ldr		\irqstat, [\base]
		mov		\irqnr, #0
9003:
		tst		\irqstat, #1
		bne		9004f
		add		\irqnr, \irqnr, #1
		mov		\irqstat, \irqstat, lsr #1
		cmp		\irqnr, #32
		bcc		9003b
9004:
		.endm
#endif

		.macro irq_prio_table
		.endm

