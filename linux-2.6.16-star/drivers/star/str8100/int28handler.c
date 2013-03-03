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
static struct work_struct reboot_work;
void do_reboot(){
	char* argv[2];
	char* env[1];
	u32 retval;

	argv[0]="/sbin/reboot";
	argv[1]=NULL;
	env[0]=NULL;
	printk("%s: Rebooting...\n",__FUNCTION__);
	//printk("reboot_work data=%.8x, entry=%p, prev=%p, next=%p\n",reboot_work.data, &reboot_work.entry,reboot_work.entry.prev,reboot_work.entry.next);
	if(retval=call_usermodehelper(argv[0],argv,env,0)) kernel_restart(NULL);
	printk("%s: exit...(retval=%d(0x%x)\n",__FUNCTION__,retval,retval);

}
static irqreturn_t str8100_int28vbus_irq_handler(int this_irq, void *dev_id, struct pt_regs *regs)
{
	printk("%s: this_irq=%d\n",__FUNCTION__,this_irq);

	INIT_WORK(&reboot_work,do_reboot,NULL);
	if(PWRMGT_USB_DEVICE_POWERMGT_REG&0x1){
		printk("%s: registering work\n",__FUNCTION__);
		schedule_work(&reboot_work);
	lock_kernel();
	free_irq(INTC_USB_DEVICE_VBUS_BIT_INDEX, NULL);
	unlock_kernel();
	}
	HAL_INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(this_irq);
	return IRQ_HANDLED;
}

static int __init str8100_int28vbus_inthandler_init(void)
{
	int ret;
printk("%s: \n",__FUNCTION__);
	if(PWRMGT_USB_DEVICE_POWERMGT_REG&0x1)
		HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();
	//	schedule_work(&reboot_work);

	//str8100_set_interrupt_trigger (INTC_GPIO_EXTERNAL_INT_BIT_INDEX,INTC_LEVEL_TRIGGER,INTC_ACTIVE_HIGH);
	if ((ret=request_irq(INTC_USB_DEVICE_VBUS_BIT_INDEX, str8100_int28vbus_irq_handler, 0, "vbus", NULL))){
		printk("%s: request_irq failed(ret=0x%x)(-EBUSY=0x%x)\n",__FUNCTION__,ret,-EBUSY);
		return -EBUSY;
	}
	return 0;
}

static void __exit str8100_int28vbus_inthandler_exit(void) 
{ 
	printk("%s: \n",__FUNCTION__);
	lock_kernel();
	free_irq(INTC_USB_DEVICE_VBUS_BIT_INDEX, NULL);
	unlock_kernel();
}

module_init(str8100_int28vbus_inthandler_init);
module_exit(str8100_int28vbus_inthandler_exit);

