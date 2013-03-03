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

#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/smp_lock.h>
#include <linux/delay.h>

#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>

#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_misc.h>
#include <asm/arch/star_gpio.h>

//#include <linux/str8100_led.h>
extern void str8100_led_all_on(void);
extern void str8100_led_all_off(void);
extern void str8100_led_on(unsigned int led_index);
extern void str8100_led_off(unsigned int led_index);
extern void str8100_led_toggle(unsigned int led_index);
extern void str8100_led_init(void);

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int, unsigned int);


static irqreturn_t str8100_gpio_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
    unsigned int volatile    status;
    int i;

printk("%s: \n",__FUNCTION__);
    HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
    HAL_GPIOA_READ_INTERRUPT_MASKED_STATUS(status);

//printk("%s: %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x \n",__FUNCTION__,GPIOA_DATA_OUTPUT_REG,GPIOA_DATA_INPUT_REG,GPIOA_DIRECTION_REG,GPIOA_INTERRUPT_ENABLE_REG,GPIOA_INTERRUPT_RAW_STATUS_REG,GPIOA_INTERRUPT_MASKED_STATUS_REG,GPIOA_INTERRUPT_MASKED_STATUS_REG,GPIOA_INTERRUPT_MASK_REG,GPIOA_INTERRUPT_TRIGGER_METHOD_REG,GPIOA_INTERRUPT_TRIGGER_BOTH_EDGES_REG,GPIOA_INTERRUPT_TRIGGER_TYPE_REG,GPIOA_BOUNCE_ENABLE_REG,GPIOA_BOUNCE_CLOCK_PRESCALE_REG);
    for (i = 0; i < 32; i++)
    {
       if (status & (1 << i))
       {
       	 printk(" str8100 gpio: Interrupt Happen (status: 0x%x, GPIO %.2d)\n",status,i);
		//GPIOA[0] will toggle GPIOB[2]
		//GPIOA[1] will toggle GPIOB[3]
       	 str8100_led_toggle(1<<(i+2));
       }	 
    }
	HAL_GPIOA_CLEAR_INTERRUPT(status);
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);

    HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_GPIO_EXTERNAL_INT_BIT_INDEX);
    return IRQ_HANDLED;
}

static int __init test_init(void){
	int ret;
	printk("%s: Output led test...\n",__FUNCTION__);
//	__u32 data,cnt=0;
	
    HAL_MISC_ENABLE_ALL_SHARED_GPIO_PINS();	
    HAL_PWRMGT_ENABLE_GPIO_CLOCK();

    PWRMGT_SOFTWARE_RESET_CONTROL_REG |=  (0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);
    PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);
    PWRMGT_SOFTWARE_RESET_CONTROL_REG |=  (0x1 << PWRMGT_GPIO_SOFTWARE_RESET_BIT_INDEX);	

	str8100_led_init();

/*
	printk("%s: Input button test...\n",__FUNCTION__);
	HAL_GPIOA_SET_DIRECTION_INPUT(3);
	while(cnt<20){
		HAL_GPIOA_READ_DATA_IN_STATUS(data);
		printk("%d-%s: read data=0x%x\n",cnt,__FUNCTION__,data);
		msleep(500);
		cnt++;
	}
*/
	printk("%s: IRQ test...\n",__FUNCTION__);
	//Configure GPIOA[0:1] as interrupts
	HAL_GPIOA_SET_DIRECTION_INPUT(3);
	HAL_GPIOA_ENABLE_INTERRUPT(3);
	HAL_GPIOA_DISABLE_INTERRUPT_MASK(3);

	//GPIOA[0] will toggle GPIOB[2] (in interrupt handler)
	//set GPIOA[0] to level trigger
	//==> GPIOB[2] will keep blinking while button pressed
	HAL_GPIOA_SET_INTERRUPT_LEVEL_TRIGGER_MODE(1);
	HAL_GPIOA_SET_INTERRUPT_LOW_LEVEL_TRIGGER_MODE(1);
//	HAL_GPIOA_SET_INTERRUPT_SINGLE_EDGE_TRIGGER_MODE(1);
//	HAL_GPIOA_SET_INTERRUPT_SINGLE_FALLING_EDGE_TRIGGER_MODE(1);
//	HAL_GPIOA_SET_INTERRUPT_SINGLE_RISING_EDGE_TRIGGER_MODE(1);

	//GPIOA[1] will toggle GPIOB[3] (in interrupt handler)
	//set GPIOA[1] to edge trigger
	//==> GPIOB[3] will turn on while pressed
	HAL_GPIOA_SET_INTERRUPT_EDGE_TRIGGER_MODE(2);
	HAL_GPIOA_SET_INTERRUPT_BOTH_EDGE_TRIGGER_MODE(2);

    str8100_set_interrupt_trigger (INTC_GPIO_EXTERNAL_INT_BIT_INDEX,INTC_IRQ_INTERRUPT,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX, str8100_gpio_irq_handler, 0, "testing", NULL))){
		printk("%s: request_irq failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,ret,-EBUSY);
		return -EBUSY;
	}

	return 0;
}

static void __exit test_exit(void){
	printk("%s: \n",__FUNCTION__);
	free_irq(INTC_GPIO_EXTERNAL_INT_BIT_INDEX,NULL);
}

module_init(test_init);
module_exit(test_exit);
