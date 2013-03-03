/*******************************************************************************
 *
 *
 *   Copyright(c) 2006 Star Semiconductor Corporation, All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 *   Contact Information:
 *   Technology Support <tech@starsemi.com>
 *   Star Semiconductor 4F, No.1, Chin-Shan 8th St, Hsin-Chu,300 Taiwan, R.O.C
 *
 ********************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/arch/star_gpio.h>

#include <linux/str9100/str9100_gpio.h>

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
#define MAX_GPIO_LINE		21
void (*gpio_isr[MAX_GPIO_LINE])(int i);
#endif

/* 
 * Read GPIO input data from register
 */
int str9100_gpio_in(volatile __u32 *data)
{
	*data = GPIO_DATA_IN_REG;
	return 0;
}

/* 
 * Write data to  GPIO output register
 */
int str9100_gpio_out(__u32 data)
{
	GPIO_DATA_OUT_REG = data;
	return 0;
}

/* 
 * Read GPIO direction Register
 */
int str9100_gpio_read_direction(volatile __u32 *data)
{
	*data = GPIO_PIN_DIR_REG;
	return 0;
}

/* 
 * Write GPIO direction Register
 */
int str9100_gpio_write_direction(__u32 data)
{
	GPIO_PIN_DIR_REG = data;
	return 0;
}

/* 
 * Write GPIO set Register
 */
int str9100_gpio_dataset(__u32 data)
{
	GPIO_DATA_SET_REG = data;
	return 0;
}

/* 
 * Write GPIO clear Register
 */
int str9100_gpio_dataclear(__u32 data)
{
	GPIO_DATA_CLEAR_REG = data;
	return 0;
}

/* Read GPIO Data, but only one bit */
int str9100_gpio_in_bit(volatile __u32 *data, int bit)
{
	u32 temp;
	if (bit >= 0 && bit <= 20) {
		str9100_gpio_in(&temp);
		*data = ((temp >> bit)&0x1);
	}
	return 0;
}

/* Write GPIO Data, but only one bit */
int str9100_gpio_out_bit(__u32 data, int bit)
{
	u32 temp;
	if (bit >= 0 && bit <= 20) {
		str9100_gpio_in(&temp);
		temp &= ~(1 <<bit);
		temp |= (data &0x1)<< bit;
		str9100_gpio_out(temp);
	}
	return 0;

}

int str9100_gpio_read_direction_bit(volatile __u32 *data, int bit)
{
	u32 temp;
	if (bit >= 0 && bit <= 20) {
		str9100_gpio_read_direction(&temp);
		*data = ((temp >> bit)&0x1);
	}
	return 0;
}

int str9100_gpio_write_direction_bit(__u32 data, int bit)
{
	u32 temp;
	if (bit >= 0 && bit <= 20) {
		str9100_gpio_read_direction(&temp);
		temp &= ~(1 <<bit);
		temp |= (data &0x1)<< bit;
		str9100_gpio_write_direction(temp);
	}
	return 0;
}

int str9100_gpio_dataset_bit(__u32 data, int bit)
{
	u32 temp=0;
	if (bit >= 0 && bit <= 20) {
		temp |= (data &0x1)<< bit;
		str9100_gpio_dataset(temp);
	}
	return 0;
}

int str9100_gpio_dataclear_bit(__u32 data, int bit)
{
	u32 temp=0;
	if (bit >= 0 && bit <= 20) {
		temp |= (data &0x1)<< bit;
		str9100_gpio_dataclear(temp);
	}
	return 0;
}

#define GEN_GPIO_WRITE_SCRIPT(regname)\
int str9100_gpio_write_##regname##_bit(volatile __u32 data,int bit) \
{ \
	u32 temp=0; \
	if (bit >= 0 && bit <= 20) { \
		temp |= ((data &0x1)<< bit); \
		str9100_gpio_write_##regname(temp); \
	} \
	return 0; \
} 
//EXPORT_SYMBOL(str9100_gpio_write_##regname_bit);

#define GEN_GPIO_READ_SCRIPT(regname) \
int str9100_gpio_read_##regname##_bit(volatile __u32 *data,int bit) \
{ \
	u32 temp; \
	if (bit >= 0 && bit <= 20) { \
		str9100_gpio_read_##regname(&temp); \
		*data = ((temp >> bit)&0x1); \
	} \
	return 0; \
} 
//EXPORT_SYMBOL(str9100_gpio_read_##regname_bit);

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
int str9100_gpio_read_intrenable(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_ENABLE_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrenable);

int str9100_gpio_write_intrenable(__u32 data)
{
	GPIO_INTERRUPT_ENABLE_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrenable);

int str9100_gpio_read_intrrawstate(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_RAW_STATE_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrrawstate);

int str9100_gpio_read_intrmaskedstatus(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_MASKED_STATE_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrmaskedstatus);

int str9100_gpio_read_intrmask(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_MASK_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrmask);

int str9100_gpio_write_intrmask(__u32 data)
{
	GPIO_INTERRUPT_MASK_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrmask);

int str9100_gpio_read_intrclear(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_CLEAR_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrclear);

int str9100_gpio_write_intrclear(__u32 data)
{
	GPIO_INTERRUPT_CLEAR_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrclear);

int str9100_gpio_read_intrtrigger(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_TRIGGER_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrtrigger);

int str9100_gpio_write_intrtrigger(__u32 data)
{
	GPIO_INTERRUPT_TRIGGER_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrtrigger);

int str9100_gpio_read_intrboth(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_BOTH_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrboth);

int str9100_gpio_write_intrboth(__u32 data)
{
	GPIO_INTERRUPT_BOTH_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrboth);

int str9100_gpio_read_intrriseneg(volatile __u32 *data)
{
	*data = GPIO_INTERRUPT_RISE_NEG_REG;
	return 0;
}
GEN_GPIO_READ_SCRIPT(intrriseneg);

int str9100_gpio_write_intrriseneg(__u32 data)
{
	GPIO_INTERRUPT_RISE_NEG_REG = data;
	return 0;
}
GEN_GPIO_WRITE_SCRIPT(intrriseneg);

#endif

#if 0
EXPORT_SYMBOL(str9100_gpio_in);
EXPORT_SYMBOL(str9100_gpio_out);
EXPORT_SYMBOL(str9100_gpio_in_bit);
EXPORT_SYMBOL(str9100_gpio_out_bit);
EXPORT_SYMBOL(str9100_gpio_read_direction);
EXPORT_SYMBOL(str9100_gpio_write_direction);
EXPORT_SYMBOL(str9100_gpio_read_direction_bit);
EXPORT_SYMBOL(str9100_gpio_write_direction_bit);
EXPORT_SYMBOL(str9100_gpio_dataset);
EXPORT_SYMBOL(str9100_gpio_dataclear);
EXPORT_SYMBOL(str9100_gpio_dataset_bit);
EXPORT_SYMBOL(str9100_gpio_dataclear_bit);

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
EXPORT_SYMBOL(str9100_gpio_read_intrenable);
EXPORT_SYMBOL(str9100_gpio_write_intrenable);
EXPORT_SYMBOL(str9100_gpio_read_intrrawstate);
EXPORT_SYMBOL(str9100_gpio_read_intrmaskedstatus);
EXPORT_SYMBOL(str9100_gpio_read_intrmask);
EXPORT_SYMBOL(str9100_gpio_write_intrmask);
EXPORT_SYMBOL(str9100_gpio_read_intrclear);
EXPORT_SYMBOL(str9100_gpio_write_intrclear);
EXPORT_SYMBOL(str9100_gpio_read_intrtrigger);
EXPORT_SYMBOL(str9100_gpio_write_intrtrigger);
EXPORT_SYMBOL(str9100_gpio_read_intrboth);
EXPORT_SYMBOL(str9100_gpio_write_intrboth);
EXPORT_SYMBOL(str9100_gpio_read_intrriseneg);
EXPORT_SYMBOL(str9100_gpio_write_intrriseneg);
#endif
#endif

static int str9100_gpio_proc(char *page, char **start,  off_t off, int count, int *eof, void *data)
{
	u32 temp;
	int num=0;

	str9100_gpio_in(&temp);
	num += sprintf(page+num, "GPIO IN                : %08x \n", temp);

	str9100_gpio_read_direction(&temp);
	num += sprintf(page+num, "GPIO Direction         : %08x \n", temp);

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
	str9100_gpio_read_intrenable(&temp);
	num += sprintf(page+num, "GPIO Interrupt Enable  : %08x \n", temp);

	str9100_gpio_read_intrrawstate(&temp);
	num += sprintf(page+num, "GPIO Interrupt Raw     : %08x \n", temp);

	str9100_gpio_read_intrtrigger(&temp);
	num += sprintf(page+num, "GPIO Interrupt Trigger : %08x \n", temp);

	str9100_gpio_read_intrboth(&temp);
	num += sprintf(page+num, "GPIO Interrupt Both    : %08x \n", temp);

	str9100_gpio_read_intrriseneg(&temp);
	num += sprintf(page+num, "GPIO Interrupt RiseNeg : %08x \n", temp);

	str9100_gpio_read_intrmask(&temp);
	num += sprintf(page+num, "GPIO Interrupt MASKED  : %08x \n", temp);

	str9100_gpio_read_intrmaskedstatus(&temp);
	num += sprintf(page+num, "GPIO Interrupt MASKEDST: %08x \n", temp);
#endif	

	return num;
}

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
static void str9100_gpio_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	u32 gpio_intr;

	// Clean System irq status
	INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
	INTC_INTERRUPT_MASK_REG |= (0x1 << INTC_GPIO_EXTERNAL_INT_BIT_INDEX);

	str9100_gpio_read_intrrawstate(&gpio_intr);
	//printk("GPIO INTERRUPT : %08x \n",gpio_intr);

	for (i = 0; i < MAX_GPIO_LINE; i++) {
		if ((gpio_intr &0x1) == 0x1) {
			if (gpio_isr[i] != NULL) {
				gpio_isr[i](i);
			}
		}
		gpio_intr = gpio_intr >> 1;
	}
	/* Clear All Interrupt Status */
	str9100_gpio_write_intrclear(0x1FFFFF);
	/* Unmask Intc Interrupt Status */
	INTC_INTERRUPT_MASK_REG &= ~(0x1 << INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
}

/*  
 * Setup GPIO for Edge Triggle Interrupt mode 
 */
void str9100_gpio_set_edgeintr(void (*funcptr)(int),int trig_both,int trig_rising, int gpio_pin)
{
	u32 intrtmp;
	
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIO_LINE) {
		str9100_gpio_write_direction_bit(PIN_INPUT,gpio_pin);

		/* Set Triggle Level */
		str9100_gpio_read_intrtrigger(&intrtmp);
		intrtmp &= ~(0x1 << gpio_pin);
		intrtmp |= (PIN_TRIG_EDGE << gpio_pin);
		str9100_gpio_write_intrtrigger(intrtmp);

		/* Set Triggle Both */
		str9100_gpio_read_intrboth(&intrtmp);
		intrtmp &= ~(0x1 << gpio_pin);
		intrtmp |= (trig_both << gpio_pin);
		str9100_gpio_write_intrboth(intrtmp);

		/* Set Triggle Rising/Falling */
		str9100_gpio_read_intrriseneg(&intrtmp);
		intrtmp &= ~(0x1 << gpio_pin);
		intrtmp |= (trig_rising << gpio_pin);
		str9100_gpio_write_intrriseneg(intrtmp);

		gpio_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		str9100_gpio_read_intrenable(&intrtmp);
		intrtmp |= (0x1 << gpio_pin);
		str9100_gpio_write_intrenable(intrtmp);
	}
}
/*  
 * Clear GPIO Triggle Interrupt
 */
void str9100_gpio_clear_intr(int gpio_pin)
{
	u32 intrtmp;
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIO_LINE) {
		gpio_isr[gpio_pin] = NULL;
		// Disable Interrupt
		str9100_gpio_read_intrenable(&intrtmp);
		intrtmp &= ~( 0x1 << gpio_pin);
		str9100_gpio_write_intrenable(intrtmp);
	}
}

/*  
 * Setup GPIO for LEVEL Triggle Interrupt mode 
 */
void str9100_gpio_set_levelintr(void (*funcptr)(int),int trig_level, int gpio_pin)
{
	u32 intrtmp;
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIO_LINE) {
		str9100_gpio_write_direction_bit(PIN_INPUT,gpio_pin);
		/* Set Triggle Level */
		str9100_gpio_read_intrtrigger(&intrtmp);
		intrtmp &= ~(0x1 << gpio_pin);
		intrtmp |= (PIN_TRIG_LEVEL << gpio_pin);
		str9100_gpio_write_intrtrigger(intrtmp);

		/* Set Triggle High/Low */
		str9100_gpio_read_intrriseneg(&intrtmp);
		intrtmp &= ~(0x1 << gpio_pin);
		intrtmp |= (trig_level << gpio_pin);
		str9100_gpio_write_intrriseneg(intrtmp);

		gpio_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		str9100_gpio_read_intrenable(&intrtmp);
		intrtmp |= (0x1 << gpio_pin);
		str9100_gpio_write_intrenable(intrtmp);
	}
}
EXPORT_SYMBOL(str9100_gpio_set_edgeintr);
EXPORT_SYMBOL(str9100_gpio_clear_intr);
EXPORT_SYMBOL(str9100_gpio_set_levelintr);

/*  
 * Display GPIO information at /proc/str9100/gpio
 */

#ifdef STR9100_GPIO_INTERRUPT_TEST
void str9100_gpio_intr_test(int i)
{
	printk("GPIO Interrupt Service Single Active : %d \n",i);
}
#endif

#endif
static struct proc_dir_entry *proc_str9100_gpio;
int __init str9100_gpio_init(void)
{
#ifdef CONFIG_STR9100_GPIO_INTERRUPT
	u32 i, ret;
#endif
#ifdef STR9100_GPIO_INTERRUPT_TEST
	/* test script */
	u32 temp;
	str9100_gpio_read_direction(&temp);
	printk("direction: %08X\n",temp);
	str9100_gpio_write_direction_bit(PIN_OUTPUT,15);
	str9100_gpio_read_direction(&temp);
	printk("direction: %08X\n",temp);
	str9100_gpio_in(&temp);
	printk("data: %08X\n",temp);
#endif

	proc_str9100_gpio = create_proc_read_entry("str9100/gpio", 0, NULL, str9100_gpio_proc, NULL) ;

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
	for (i = 0; i < MAX_GPIO_LINE; i++) {
		gpio_isr[i] = NULL;
	}
	/* Clear All Interrupt Status */
	str9100_gpio_write_intrclear(0x1FFFFF);
	str9100_set_interrupt_trigger(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, INTC_EDGE_TRIGGER, INTC_RISING_EDGE);
	ret = request_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, str9100_gpio_irq_handler, 0, "str9100_gpio", 0);
	if (ret < 0) {
		printk("request_irq fail : %d \n", ret);
		return 0;
	} else {
		printk("GPIO interrupt handler install ok. \n");
	}
#endif
#ifdef STR9100_GPIO_INTERRUPT_TEST
	str9100_gpio_set_edgeintr(&str9100_gpio_intr_test,PIN_TRIG_SINGLE,PIN_TRIG_RISING,12);
#endif

	return 0;
}	

void __exit str9100_gpio_exit(void)
{
	free_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX,0);
}

module_init(str9100_gpio_init);
module_exit(str9100_gpio_exit);

MODULE_LICENSE("GPL");
