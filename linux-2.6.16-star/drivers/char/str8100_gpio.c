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
#include <asm/hardware.h>
#include <asm/mach-types.h>

#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_gpio.h>


#ifdef CONFIG_STR8100_GPIO_INTERRUPT

#define MAX_GPIOA_LINE		32
#define MAX_GPIOB_LINE		32
#define PIN_INPUT 0
#define PIN_OUTPUT 1

#define PIN_TRIG_EDGE 0
#define PIN_TRIG_LEVEL 1

#define PIN_TRIG_SINGLE 0
#define PIN_TRIG_BOTH 1

#define PIN_TRIG_RISING 0
#define PIN_TRIG_FAILING 1

#define PIN_TRIG_HIGH 0
#define PIN_TRIG_LOW 1
void (*gpio_a_isr[MAX_GPIOA_LINE])(int i);
void (*gpio_b_isr[MAX_GPIOB_LINE])(int i);

#endif


/* 
 * str8100_gpio_a_in - read Data Input Register(RO) of GPIO A
 * @data: the target to store content of register
 */
int str8100_gpio_a_datain(volatile __u32 *data)
{
	HAL_GPIOA_READ_DATA_IN_STATUS(*data);
	return 0;
}

/* 
 * str8100_gpio_b_in - read Data Input Register(RO) of GPIO B
 * @data: the target to store content of register
 */
int str8100_gpio_b_datain(volatile __u32 *data)
{
	HAL_GPIOB_READ_DATA_IN_STATUS(*data);
	return 0;
}

/* str8100_gpio_a_out - write Data Output Register(RW) of GPIO A
 * @data:
 */
int str8100_gpio_a_dataout(__u32 data)
{
	GPIOA_DATA_OUTPUT_REG = data;
	return 0;
}

/* 
 * str8100_gpio_b_out - write Data Output Register(RW) of GPIO B
 * @data:
 */
int str8100_gpio_b_out(__u32 data)
{
	GPIOB_DATA_OUTPUT_REG = data;
	return 0;
}

/* 
 * str8100_gpio_a_read_direction - read Direction Register(RW) GPIO A
 * @data:
 */
int str8100_gpio_a_read_direction(volatile __u32 *data)
{
	*data = GPIOA_DIRECTION_REG;
	return 0;
}

/* 
 * str8100_gpio_b_read_direction - read Direction Register(RW) of GPIO B
 * @data:
 */
int str8100_gpio_b_read_direction(volatile __u32 *data)
{
	*data = GPIOB_DIRECTION_REG;
	return 0;
}

/* 
 * str8100_gpio_a_write_direction - write Direction Register(RW) of GPIO A
 * @data:
 */
int str8100_gpio_a_write_direction(__u32 data)
{
	GPIOA_DIRECTION_REG = data;
	return 0;
}

/* 
 * str8100_gpio_b_write_direction - write Direction Register(RW) of GPIO B
 * @data:
 */
int str8100_gpio_b_write_direction(__u32 data)
{
	GPIOB_DIRECTION_REG = data;
	return 0;
}

/* 
 * str8100_gpio_a_dataset - write Data Bit Set Register (W) of GPIO A
 * @data:
 *
 * When write to this register and if some bits of GpioDataSet are 1,
 * the corresponding bits in GpioDataOut register will be set to 1, 
 * and the others will not be changed.
 */
int str8100_gpio_a_dataset(__u32 data)
{
	GPIOA_DATA_BIT_SET_REG = data;
	return 0;
}

/* 
 * str8100_gpio_b_dataset - write Data Bit Set Register (W) of GPIO B
 * @data:
 */
int str8100_gpio_b_dataset(__u32 data)
{
	GPIOB_DATA_BIT_SET_REG = data;
	return 0;
}

/* 
 * str8100_gpio_a_dataclear - write Data Bit Clear Register (W) of GPIO A
 * @data:
 *
 * When write to this register and if some bits of GpioDataClear are 1,
 * the corresponding bits in GpioDataOut register will be cleard, 
 * and the others will not be changed.
 */
int str8100_gpio_a_dataclear(__u32 data)
{
	GPIOA_DATA_BIT_CLEAR_REG = data;
	return 0;
}

/* 
 * str8100_gpio_b_dataclear - write Data Bit Clear Register (W) of GPIO B
 * @data:
 */
int str8100_gpio_b_dataclear(__u32 data)
{
	GPIOB_DATA_BIT_CLEAR_REG = data;
	return 0;
}

static int str8100_gpio_proc(char *page, char **start,  off_t off, int count, int *eof, void *data)
{
	int num = 0;

	num += sprintf(page+num, "********** GPIO Group A **********\n");
	
	num += sprintf(page+num, "GPIO IN                : %08x \n", GPIOA_DATA_INPUT_REG);

	num += sprintf(page+num, "GPIO Direction         : %08x \n", GPIOA_DIRECTION_REG);

#ifdef CONFIG_STR8100_GPIO_INTERRUPT
	num += sprintf(page+num, "GPIO Interrupt Enable  : %08x \n", GPIOA_INTERRUPT_ENABLE_REG);

	num += sprintf(page+num, "GPIO Interrupt Raw     : %08x \n", GPIOA_INTERRUPT_RAW_STATUS_REG);

	num += sprintf(page+num, "GPIO Interrupt Trigger : %08x \n", GPIOA_INTERRUPT_TRIGGER_METHOD_REG);

	num += sprintf(page+num, "GPIO Interrupt Both    : %08x \n", GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG);

	num += sprintf(page+num, "GPIO Interrupt RiseNeg : %08x \n", GPIOA_INTERRUPT_TRIGGER_TYPE_REG);

	num += sprintf(page+num, "GPIO Interrupt MASKED  : %08x \n", GPIOA_INTERRUPT_MASK_REG);

	num += sprintf(page+num, "GPIO Interrupt MASKEDST: %08x \n", GPIOA_INTERRUPT_MASKED_STATUS_REG);
#endif	

	num+= sprintf(page+num, "********** GPIO Group B **********\n");
	
	num += sprintf(page+num, "GPIO IN                : %08x \n", GPIOB_DATA_INPUT_REG);

	num += sprintf(page+num, "GPIO Direction         : %08x \n", GPIOB_DIRECTION_REG);

#ifdef CONFIG_STR8100_GPIO_INTERRUPT
	num += sprintf(page+num, "GPIO Interrupt Enable  : %08x \n", GPIOB_INTERRUPT_ENABLE_REG);

	num += sprintf(page+num, "GPIO Interrupt Raw     : %08x \n", GPIOB_INTERRUPT_RAW_STATUS_REG);

	num += sprintf(page+num, "GPIO Interrupt Trigger : %08x \n", GPIOB_INTERRUPT_TRIGGER_METHOD_REG);

	num += sprintf(page+num, "GPIO Interrupt Both    : %08x \n", GPIOB_INTERRUPT_TRIGGER_BOTH_EDGES_REG);

	num += sprintf(page+num, "GPIO Interrupt RiseNeg : %08x \n", GPIOB_INTERRUPT_TRIGGER_TYPE_REG);

	num += sprintf(page+num, "GPIO Interrupt MASKED  : %08x \n", GPIOB_INTERRUPT_MASK_REG);

	num += sprintf(page+num, "GPIO Interrupt MASKEDST: %08x \n", GPIOB_INTERRUPT_MASKED_STATUS_REG);
#endif	

	return num;
}

#ifdef CONFIG_STR8100_GPIO_INTERRUPT
static irqreturn_t str8100_gpio_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int volatile    status;
	int i;

	// Clean System irq status
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);

	HAL_GPIOA_READ_INTERRUPT_MASKED_STATUS(status);
	for (i = 0; i < MAX_GPIOA_LINE; i++) {   
		if (status & (1 << i))  {   
			if (gpio_a_isr[i] != NULL) {
				gpio_a_isr[i](i);
			}
		}    
	}  
    HAL_GPIOA_CLEAR_INTERRUPT(status);

	HAL_GPIOB_READ_INTERRUPT_MASKED_STATUS(status);
	for (i = 0; i < MAX_GPIOB_LINE; i++) {   
		if (status & (1 << i)) {   
			if (gpio_b_isr[i] != NULL) {
				gpio_b_isr[i](i);
			}
		}    
	}   
    HAL_GPIOB_CLEAR_INTERRUPT(status);

	/* Unmask Intc Interrupt Status */
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
	return IRQ_HANDLED;
}

/*  
 * Setup GPIO for Edge Triggle Interrupt mode 
 */
void str8100_gpio_a_set_edgeintr(void (*funcptr)(int), int trig_both, int trig_rising, int gpio_pin)
{
	u32 gpio_index = (0x1 << gpio_pin);
	
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOA_LINE) {
		HAL_GPIOA_SET_DIRECTION_INPUT(gpio_index);

		/* Set Triggle Level */
		HAL_GPIOA_SET_INTERRUPT_EDGE_TRIGGER_MODE(gpio_index);

		/* Set Triggle Both */
		HAL_GPIOA_SET_INTERRUPT_SINGLE_EDGE_TRIGGER_MODE(gpio_index);
		if (trig_both) {
			HAL_GPIOA_SET_INTERRUPT_BOTH_EDGE_TRIGGER_MODE(gpio_index);	
		}

		/* Set Triggle Rising/Falling */
		HAL_GPIOA_SET_INTERRUPT_SINGLE_FALLING_EDGE_TRIGGER_MODE(gpio_index);
		if (trig_rising) {
			HAL_GPIOA_SET_INTERRUPT_SINGLE_RISING_EDGE_TRIGGER_MODE(gpio_index);
		}

		gpio_a_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		HAL_GPIOA_ENABLE_INTERRUPT(gpio_index);
	}
}

/*  
 * Setup GPIO for Edge Triggle Interrupt mode 
 */
void str8100_gpio_b_set_edgeintr(void (*funcptr)(int),int trig_both,int trig_rising, int gpio_pin)
{
	u32 gpio_index = (0x1 << gpio_pin);
	
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOA_LINE) {
		HAL_GPIOB_SET_DIRECTION_INPUT(gpio_index);

		/* Set Triggle Level */
		HAL_GPIOB_SET_INTERRUPT_EDGE_TRIGGER_MODE(gpio_index);

		/* Set Triggle Both */
		HAL_GPIOB_SET_INTERRUPT_SINGLE_EDGE_TRIGGER_MODE(gpio_index);
		if (trig_both) {
			HAL_GPIOB_SET_INTERRUPT_BOTH_EDGE_TRIGGER_MODE(gpio_index);	
		}

		/* Set Triggle Rising/Falling */
		HAL_GPIOB_SET_INTERRUPT_SINGLE_FALLING_EDGE_TRIGGER_MODE(gpio_index);
		if (PIN_TRIG_RISING == trig_rising) {
			HAL_GPIOB_SET_INTERRUPT_SINGLE_RISING_EDGE_TRIGGER_MODE(gpio_index);
		}

		gpio_a_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		HAL_GPIOB_ENABLE_INTERRUPT(gpio_index);
	}

}

/*  
 * Clear GPIO Triggle Interrupt
 */
void str8100_gpio_a_clear_intr(int gpio_pin)
{
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOA_LINE) {
		gpio_a_isr[gpio_pin] = NULL;
		// Disable Interrupt
		HAL_GPIOA_DISABLE_INTERRUPT(0x1 << gpio_pin);
	}
}

/*  
 * Clear GPIO Triggle Interrupt
 */
void str8100_gpio_b_clear_intr(int gpio_pin)
{
	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOB_LINE) {
		gpio_a_isr[gpio_pin] = NULL;
		// Disable Interrupt
		HAL_GPIOB_DISABLE_INTERRUPT(0x1 << gpio_pin);
	}
}

/*  
 * Setup GPIO for LEVEL Triggle Interrupt mode 
 */
void str8100_gpio_a_set_levelintr(void (*funcptr)(int),int trig_level, int gpio_pin)
{
	u32 gpio_index = (0x1 << gpio_pin);

	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOA_LINE) {
		HAL_GPIOA_SET_DIRECTION_INPUT(gpio_index);
		/* Set Triggle Level */
		HAL_GPIOA_SET_INTERRUPT_LEVEL_TRIGGER_MODE(gpio_index);

		/* Set Triggle High/Low */
		HAL_GPIOA_SET_INTERRUPT_LOW_LEVEL_TRIGGER_MODE(gpio_index);
		if (PIN_TRIG_HIGH == trig_level) {
			HAL_GPIOA_SET_INTERRUPT_HIGH_LEVEL_TRIGGER_MODE(gpio_index);
		}

		gpio_a_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		HAL_GPIOA_ENABLE_INTERRUPT(gpio_index);
	}
}

/*  
 * Setup GPIO for LEVEL Triggle Interrupt mode 
 */
void str8100_gpio_b_set_levelintr(void (*funcptr)(int),int trig_level, int gpio_pin)
{
	u32 gpio_index = (0x1 << gpio_pin);

	if (gpio_pin >= 0 && gpio_pin < MAX_GPIOB_LINE) {
		HAL_GPIOB_SET_DIRECTION_INPUT(gpio_index);
		/* Set Triggle Level */
		HAL_GPIOB_SET_INTERRUPT_LEVEL_TRIGGER_MODE(gpio_index);

		/* Set Triggle High/Low */
		HAL_GPIOB_SET_INTERRUPT_LOW_LEVEL_TRIGGER_MODE(gpio_index);
		if (PIN_TRIG_HIGH == trig_level) {
			HAL_GPIOB_SET_INTERRUPT_HIGH_LEVEL_TRIGGER_MODE(gpio_index);
		}

		gpio_b_isr[gpio_pin] = funcptr;

		// Enable Interrupt
		HAL_GPIOB_ENABLE_INTERRUPT(gpio_index);
	}
}

EXPORT_SYMBOL(str8100_gpio_a_set_edgeintr);
EXPORT_SYMBOL(str8100_gpio_a_clear_intr);
EXPORT_SYMBOL(str8100_gpio_a_set_levelintr);
EXPORT_SYMBOL(str8100_gpio_b_set_edgeintr);
EXPORT_SYMBOL(str8100_gpio_b_clear_intr);
EXPORT_SYMBOL(str8100_gpio_b_set_levelintr);

/*  
 * Display GPIO information at /proc/str9100/gpio
 */

#ifdef STR8100_GPIO_INTERRUPT_TEST
void str8100_gpio_intr_test(int i)
{
	printk("GPIO Interrupt Service Single Active : %d \n",i);
}
#endif

#endif

static struct proc_dir_entry *proc_str8100_gpio;
int __init str8100_gpio_init(void)
{
#ifdef CONFIG_STR8100_GPIO_INTERRUPT
	u32 i, ret;
#endif

	proc_str8100_gpio = create_proc_read_entry("str8100/gpio", 0, NULL, str8100_gpio_proc, NULL) ;

#ifdef CONFIG_STR8100_GPIO_INTERRUPT
	for (i = 0; i < MAX_GPIOA_LINE; i++) {
		gpio_a_isr[i] = NULL;
	}
	for (i = 0; i < MAX_GPIOB_LINE; i++) {
		gpio_b_isr[i] = NULL;
	}

	/* Clear All Interrupt Status */
	HAL_GPIOA_CLEAR_INTERRUPT(0xFFFFFFFF);
	HAL_GPIOB_CLEAR_INTERRUPT(0xFFFFFFFF);
	str8100_set_interrupt_trigger(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, INTC_EDGE_TRIGGER, INTC_RISING_EDGE);
	ret = request_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, str8100_gpio_irq_handler, 0, "str8100_gpio", 0);
	if (ret < 0) {
		printk("request_irq fail : %d \n", ret);
		return 0;
	} else {
		printk("GPIO interrupt handler install ok. \n");
	}
#endif
#ifdef STR8100_GPIO_INTERRUPT_TEST
	str8100_gpio_a_set_edgeintr(&str8100_gpio_intr_test, PIN_TRIG_SINGLE, PIN_TRIG_RISING, 0);
	str8100_gpio_a_set_levelintr(&str8100_gpio_intr_test, PIN_TRIG_HIGH, 1);
#endif

	return 0;
}	

void __exit str8100_gpio_exit(void)
{
	free_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, 0);
}

module_init(str8100_gpio_init);
module_exit(str8100_gpio_exit);

MODULE_LICENSE("GPL");
