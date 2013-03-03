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

extern void str8100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);

static int mode=0;
module_param(mode, int, 0);
MODULE_PARM_DESC(mode, "");
static irqreturn_t str8100_ext_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	void (*fnptr)(void)=NULL;
	printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);
	HAL_INTC_DISABLE_INTERRUPT_SOURCE(this_irq);

	if(mode==1) while(1);
	else fnptr();

	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	HAL_INTC_ENABLE_INTERRUPT_SOURCE(this_irq);

    return IRQ_HANDLED;
}

static int __init str8100_inthandler_init(void)
{
	int ret;
printk("%s: \n",__FUNCTION__);
	if(mode==0) panic("%s: panic for testing...\n",__FUNCTION__);

#define register_ext_int(_i){\
			printk("%s: registering int handler for external int%d\n",__FUNCTION__,_i);\
			HAL_MISC_ENABLE_EXT_INT##_i##_PINS();\
			str8100_set_interrupt_trigger (INTC_EXT_INT##_i##_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_LOW);\
			if ((ret=request_irq(INTC_EXT_INT##_i##_BIT_INDEX, str8100_ext_irq_handler, 0, "testing", NULL))){\
				printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,INTC_EXT_INT##_i##_BIT_INDEX,ret,-EBUSY);\
				return -EBUSY;\
			}\
		}
	register_ext_int(29);
	register_ext_int(30);
	return 0;
}

static void __exit str8100_inthandler_exit(void) 
{ 
//#ifdef DEBUG
		printk("%s: \n",__FUNCTION__);
//#endif
    lock_kernel();
    free_irq(INTC_EXT_INT29_BIT_INDEX, NULL);
    free_irq(INTC_EXT_INT30_BIT_INDEX, NULL);
    unlock_kernel();
}

module_init(str8100_inthandler_init);
module_exit(str8100_inthandler_exit);

