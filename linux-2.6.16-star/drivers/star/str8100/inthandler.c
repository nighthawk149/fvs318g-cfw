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

 
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

#include <asm/arch/star_gpio.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_powermgt.h>

static int gpio=0;
module_param(gpio, int, 0);
MODULE_PARM_DESC(gpio, "GPIO int switch(0: disable, 1:enable default=0)");

static int ext29=0;
module_param(ext29, int, 0);
MODULE_PARM_DESC(ext29, "Ext int 29 switch(0: disable, 1:enable default=0)");

static int ext30=0;
module_param(ext30, int, 0);
MODULE_PARM_DESC(ext30, "Ext int 30 switch(0: disable, 1:enable default=0)");

static int debug=1;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "debug mode (1=on, 0=off, default=1");

static irqreturn_t (*gpioA_irq_handler)(int, void*, struct pt_regs*)=NULL;
static irqreturn_t (*gpioB_irq_handler)(int, void*, struct pt_regs*)=NULL;
static irqreturn_t (*ext_irq_handler)(int, void*, struct pt_regs*)=NULL;

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);

static irqreturn_t str8100_ext_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	if(debug) printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);
	if(ext_irq_handler){
		ext_irq_handler(this_irq,dev_id,regs);
	}
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

static irqreturn_t str8100_gpio_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	unsigned int volatile    statusA,statusB;
	int i;

	if(debug) printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);

	HAL_GPIOA_READ_INTERRUPT_MASKED_STATUS(statusA);
	HAL_GPIOB_READ_INTERRUPT_MASKED_STATUS(statusB);
//printk("%s: %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x \n",__FUNCTION__,GPIOA_DATA_OUTPUT_REG,GPIOA_DATA_INPUT_REG,GPIOA_DIRECTION_REG,GPIOA_INTERRUPT_ENABLE_REG,GPIOA_INTERRUPT_RAW_STATUS_REG,GPIOA_INTERRUPT_MASKED_STATUS_REG,GPIOA_INTERRUPT_MASKED_STATUS_REG,GPIOA_INTERRUPT_MASK_REG,GPIOA_INTERRUPT_TRIGGER_METHOD_REG,GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG,GPIOA_INTERRUPT_TRIGGER_TYPE_REG,GPIOA_BOUNCE_ENABLE_REG,GPIOA_BOUNCE_CLOCK_PRESCALE_REG);
	for (i = 0; i < 32; i++)
	{
		if (statusA & (1 << i)){
       			if(debug) printk("%s: GPIOA Int %d\n",__FUNCTION__,i);
       			if(gpioA_irq_handler){
       				gpioA_irq_handler(i,dev_id,regs);
       			}
		}	 
		if (statusB & (1 << i)){
       			if(debug) printk("%s: GPIOB Int %d\n",__FUNCTION__,i);
       			if(gpioB_irq_handler){
       				gpioB_irq_handler(i,dev_id,regs);
       			}
		}	 
	}
	HAL_GPIOA_CLEAR_INTERRUPT(statusA);
	HAL_GPIOB_CLEAR_INTERRUPT(statusB);

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);

    return IRQ_HANDLED;
}

static int __init str8100_inthandler_init(void)
{
	int ret;
if(debug) printk("%s: \n",__FUNCTION__);

#if 0
    /*
     * Configure system Xtal clock to be output to CLKOUT pin
     */
    HAL_PWRMGT_CONFIGURE_CLOCK_OUT_PIN(0, 0);
#endif

//	HAL_MISC_ENABLE_ALL_SHARED_GPIO_PINS();	
	if(gpio){
		if(debug) printk("%s: registering int handler for gpio int\n",__FUNCTION__);\
//gpio initialization depend on application
#if 0
		HAL_MISC_DISABLE_EXT_INT29_PINS();
		HAL_MISC_DISABLE_EXT_INT30_PINS();

		HAL_PWRMGT_ENABLE_GPIO_CLOCK();

		PWRMGT_SOFTWARE_RESET_CONTROL_REG |=  (0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);
		PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);
		PWRMGT_SOFTWARE_RESET_CONTROL_REG |=  (0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);	

		HAL_GPIOA_SET_DIRECTION_INPUT(3);
		HAL_GPIOA_ENABLE_INTERRUPT(3);
		HAL_GPIOA_DISABLE_INTERRUPT_MASK(3);
		HAL_GPIOA_SET_INTERRUPT_EDGE_TRIGGER_MODE(3);
#endif

		str8100_set_interrupt_trigger (INTC_GPIO_EXTERNAL_INT_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
		if ((ret=request_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, str8100_gpio_irq_handler, 0, "testing", NULL))){
			if(debug) printk("%s: request_irq failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,ret,-EBUSY);
			return -EBUSY;
		}
	}
#define register_ext_int(_i){\
			if(debug) printk("%s: registering int handler for external int%d\n",__FUNCTION__,_i);\
			HAL_MISC_ENABLE_EXT_INT##_i##_PINS();\
			str8100_set_interrupt_trigger (INTC_EXT_INT##_i##_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);\
			if ((ret=request_irq(INTC_EXT_INT##_i##_BIT_INDEX, str8100_ext_irq_handler, 0, "testing", NULL))){\
				if(debug) printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_EXT_INT##_i##_BIT_INDEX,ret,-EBUSY);\
				return -EBUSY;\
			}\
		}
	if(ext29) register_ext_int(29);
	if(ext30) register_ext_int(30);

/*	HAL_MISC_ENABLE_EXT_INT30_PINS();
	str8100_set_interrupt_trigger (INTC_EXT_INT30_BIT_INDEX,INTC_IRQ_INTERRUPT,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);
	if ((ret=request_irq(INTC_EXT_INT30_BIT_INDEX, str8100_pm_irq_handler, 0, "testing", NULL))){
		printk("%s: request_irq failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,ret,-EBUSY);
		return -EBUSY;
	}
*/
	return 0;
}

static void __exit str8100_inthandler_exit(void) 
{ 
//#ifdef DEBUG
		if(debug) printk("%s: \n",__FUNCTION__);
//#endif
    lock_kernel();
    if(gpio) free_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, NULL);
    if(ext29) free_irq(INTC_EXT_INT29_BIT_INDEX, NULL);
    if(ext30) free_irq(INTC_EXT_INT30_BIT_INDEX, NULL);
    unlock_kernel();
}

module_init(str8100_inthandler_init);
module_exit(str8100_inthandler_exit);

