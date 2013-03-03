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

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/timex.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>

#if 0
// for timer clock < 1MHz
#define uSECS_PER_TICK	(1000000 / APB_clock)
#define TICKS2USECS(x)  ((x) * uSECS_PER_TICK)
#else
// for timer clock >= 1MHz
#define TICKS_PER_uSEC	(APB_clock / 1000000)
#define TICKS2USECS(x)  ((x) / TICKS_PER_uSEC)
#endif

extern u32 APB_clock;
static u32 timer_counter_value;

static inline unsigned int str9100_read_timer_counter(void)
{
	return TIMER1_COUNTER_REG;
}

static inline unsigned int str9100_read_timer_interrupt_status(void)
{
	return TIMER_INTERRUPT_STATUS_REG;
}

static inline void str9100_clear_timer_interrupt_status(unsigned int irq)
{
	TIMER_INTERRUPT_STATUS_REG &= ~(1 << TIMER1_OVERFLOW_INTERRUPT_BIT_INDEX);
}

static void str9100_setup_timer(unsigned int counter_value)
{
	unsigned long control_value;
	unsigned long mask_value;    

	control_value = TIMER_CONTROL_REG;
	mask_value = TIMER_INTERRUPT_MASK_REG;

	TIMER1_COUNTER_REG = counter_value;
	TIMER1_AUTO_RELOAD_VALUE_REG = counter_value;
	TIMER1_MATCH_VALUE1_REG = 0;
	TIMER1_MATCH_VALUE2_REG = 0;

	// Clock Source: PCLK
	control_value &= ~(1 << TIMER1_CLOCK_SOURCE_BIT_INDEX);

	// Down Count Mode
	control_value |= (1 << TIMER1_UP_DOWN_COUNT_BIT_INDEX);

	// un-mask overflow, match2 and match1 interrupt sources
	mask_value &= ~(0x7);

	// mask match2 and match1 interrupt sources
	mask_value |= 0x03;

	TIMER_CONTROL_REG = control_value;
	TIMER_INTERRUPT_MASK_REG = mask_value;
}

static void str9100_timer_enable(void)
{
	unsigned long control_value;

	control_value = TIMER_CONTROL_REG;

	// enable overflow mode
	control_value |= (1 << TIMER1_OVERFLOW_ENABLE_BIT_INDEX);

	// enable the timer
	control_value |= (1 << TIMER1_ENABLE_BIT_INDEX);

	TIMER_CONTROL_REG = control_value;
}

#if 0
static void str9100_timer_disable(void)
{
	unsigned long control_value;

	control_value = TIMER_CONTROL_REG;

	// disable overflow mode
	control_value &= ~(1 << TIMER1_OVERFLOW_ENABLE_BIT_INDEX);

	// disable the timer
	control_value &= ~(1 << TIMER1_ENABLE_BIT_INDEX);

	TIMER_CONTROL_REG = control_value;
}
#endif /* str9100_timer_disable */

/*
 * Returns number of us since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 */
static unsigned long str9100_gettimeoffset(void)
{
	unsigned long ticks1, ticks2;
	unsigned long interrupt_status;

	/*
	 * Get the current number of ticks.  Note that there is a race
	 * condition between us reading the timer and checking for
	 * an interrupt.  We get around this by ensuring that the
	 * counter has not reloaded between our two reads.
	 */
	ticks2 = str9100_read_timer_counter();
	do {
		ticks1 = ticks2;
		interrupt_status = str9100_read_timer_interrupt_status();
		ticks2 = str9100_read_timer_counter();
	} while (ticks2 > ticks1);

	/*
	 * Number of ticks since last interrupt
	 */
	ticks1 = timer_counter_value - ticks2;

	/*
	 * Interrupt pending?  If so, we've reloaded once already.
	 */
	if (interrupt_status) {
		ticks1 += timer_counter_value;
	}

	/*
	 * Convert the ticks to usecs
	 */
	return TICKS2USECS(ticks1);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t
str9100_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	write_seqlock(&xtime_lock);

	str9100_clear_timer_interrupt_status((unsigned int)irq);
	timer_tick(regs);

	write_sequnlock(&xtime_lock);

	return IRQ_HANDLED;
}

static struct irqaction str9100_timer_irq = {
	.name		= "STR9100 Timer Tick",
	.flags		= SA_INTERRUPT | SA_TIMER,
	.handler	= str9100_timer_interrupt,
};

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
static void __init str9100_timer_init(void)
{
	/*
	 * prepare timer-related values
	 */
	timer_counter_value = APB_clock / HZ;

	/*
	 * setup timer-related values
	 */
	str9100_setup_timer(timer_counter_value);

	/*
	 * Make irqs happen for the system timer
	 */
	setup_irq(INTC_TIMER1_BIT_INDEX, &str9100_timer_irq);

	str9100_timer_enable();
}

struct sys_timer str9100_timer = {
	.init		= str9100_timer_init,
	.offset		= str9100_gettimeoffset,
};

