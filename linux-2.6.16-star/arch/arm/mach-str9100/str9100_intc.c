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

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>

#include <asm/hardware.h>
#include <asm/mach/irq.h>
#include <asm/irq.h>

#define INTC_TRIGGER_UNKNOWN -1

typedef struct
{
	int	mode;
	int	level;
} intc_trigger_t;

static intc_trigger_t intc_trigger_table[] =
{
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 0
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 1
	{ INTC_EDGE_TRIGGER,	INTC_FALLING_EDGE	},	// 2
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 3
	{ INTC_TRIGGER_UNKNOWN,	INTC_TRIGGER_UNKNOWN	},	// 4
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_LOW		},	// 5
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_LOW		},	// 6
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 7
	{ INTC_TRIGGER_UNKNOWN,	INTC_TRIGGER_UNKNOWN	},	// 8
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 9
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 10
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 11
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 12
	{ INTC_TRIGGER_UNKNOWN,	INTC_TRIGGER_UNKNOWN	},	// 13
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 14
	{ INTC_EDGE_TRIGGER,	INTC_FALLING_EDGE	},	// 15
	{ INTC_TRIGGER_UNKNOWN,	INTC_TRIGGER_UNKNOWN	},	// 16
	{ INTC_TRIGGER_UNKNOWN,	INTC_TRIGGER_UNKNOWN	},	// 17
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_HIGH	},	// 18
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 19
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 20
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 21
	{ INTC_EDGE_TRIGGER,	INTC_RISING_EDGE	},	// 22
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_LOW		},	// 23
	{ INTC_LEVEL_TRIGGER,	INTC_ACTIVE_LOW		},	// 24
};

/*
 * Configure interrupt trigger mode to be level trigger or edge trigger
 */
static inline void str9100_set_irq_mode(unsigned int irq, unsigned int mode)
{
	unsigned int val;

	if (irq < 0 || irq > NR_IRQS) {
		return;
	}

	if ((mode != INTC_LEVEL_TRIGGER) &&
		(mode != INTC_EDGE_TRIGGER)) {
		return;
	}

	val = INTC_INTERRUPT_TRIGGER_MODE_REG;

	if (mode == INTC_LEVEL_TRIGGER) {
		if (val & (1UL << irq)) {
			val &= ~(1UL << irq);
			INTC_INTERRUPT_TRIGGER_MODE_REG = val;
		}
	} else {
		if (!(val & (1UL << irq))) {
			val |= (1UL << irq);
			INTC_INTERRUPT_TRIGGER_MODE_REG = val;
		}
	}
}	

/*
 * Configure interrupt trigger level to be Active High/Low or Rising/Falling Edge
 */
static inline void str9100_set_irq_level(unsigned int irq, unsigned int level)
{
	unsigned int val;

	if (irq < 0 || irq > NR_IRQS) {
		return;
	}

	if ((level != INTC_ACTIVE_HIGH) &&
		(level != INTC_ACTIVE_LOW) &&
		(level != INTC_RISING_EDGE) &&
		(level != INTC_FALLING_EDGE)) {
		return;
	}

	val = INTC_INTERRUPT_TRIGGER_LEVEL_REG;

	if ((level == INTC_ACTIVE_HIGH) ||
		(level == INTC_RISING_EDGE)) {
		if (val & (1UL << irq)) {
			val &= ~(1UL << irq);
			INTC_INTERRUPT_TRIGGER_LEVEL_REG = val;
		}
	} else {
		if (!(val & (1UL << irq))) {
			val |= (1UL << irq);
			INTC_INTERRUPT_TRIGGER_LEVEL_REG = val;
		}
	}
}

/*
 * Configure interrupt trigger mode and trigger level
 */
void str9100_set_interrupt_trigger(unsigned int irq, unsigned int mode, unsigned int level)
{
	str9100_set_irq_mode(irq, mode);
	str9100_set_irq_level(irq, level);
}
EXPORT_SYMBOL(str9100_set_interrupt_trigger);

/*
 * Mask/Disable this interrupt source
 */
static void str9100_mask_irq(unsigned int irq)
{
	// Mask/Disable this interrupt source
	INTC_INTERRUPT_MASK_REG |= (1UL << irq);
}

/*
 * Un-Mask/Enable this interrupt source
 */
static void str9100_unmask_irq(unsigned int irq)
{
	// Clear interrupt status of the interrupt source which is edge-triggered
	INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG |= (1UL << irq);

	// Mask/Disable this interrupt source
	INTC_INTERRUPT_MASK_REG &= ~(1UL << irq);
}

struct irqchip str9100_irqchip = {
	.ack	= str9100_mask_irq,
	.mask	= str9100_mask_irq,
	.unmask	= str9100_unmask_irq,
};

void __init str9100_init_irq(void)
{
	int i;

	INTC_INTERRUPT_MASK_REG = 0xFFFFFFFF;
	INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG = 0xFFFFFFFF;	
	INTC_FIQ_MODE_SELECT_REG = 0x0;

	for (i = 0; i < NR_IRQS; i++) {
		if (intc_trigger_table[i].mode != INTC_TRIGGER_UNKNOWN) {
			str9100_set_irq_mode(i, intc_trigger_table[i].mode);
			str9100_set_irq_level(i, intc_trigger_table[i].level);
		}
	}
	
	for (i = 0; i < NR_IRQS;  i++) {
		set_irq_chip(i, &str9100_irqchip);
		set_irq_handler(i, do_level_IRQ);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}
}

