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

#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/mach-types.h>

#include <asm/arch/star_wdtimer.h>
#include <asm/arch/star_powermgt.h>
#include <asm/arch/star_intc.h>
#include <asm/arch/star_misc.h>

#define STR8100_WATCHDOG_DATE       "20070521"
#define STR8100_WATCHDOG_VERSION    "1.0.1"
#define DEVICE_NAME                 "watchdog"
#define CONFIG_HW_RST

extern void str8100_set_interrupt_trigger (unsigned int, unsigned int, unsigned int, unsigned int);


#define TIMER_MARGIN	60	/* about 60 second */

static int soft_margin = TIMER_MARGIN;	

#ifndef CONFIG_HW_RST
static void watchdog_fire(int irq, void *dev_id, struct pt_regs *regs)
{
    printk(KERN_CRIT "Watchdog(%s): Reboot System.\n",__FUNCTION__);
    HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();
}
#endif /* Disable watchdog_fire */

static void watchdog_ping(void)
{	
    HAL_WDTIMER_ENABLE_RESTART_RELOAD();
}

static void watchdog_str8100_hwinit(void)
{ 
    HAL_MISC_ENABLE_WDTIMER_RESET_PINS();
    HAL_PWRMGT_ENABLE_WDTIMER_CLOCK();
    PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_WDTIMER_SOFTWARE_RESET_BIT_INDEX);
    PWRMGT_SOFTWARE_RESET_CONTROL_REG &= ~(0x1 << PWRMGT_WDTIMER_SOFTWARE_RESET_BIT_INDEX);
    PWRMGT_SOFTWARE_RESET_CONTROL_REG |= (0x1 << PWRMGT_WDTIMER_SOFTWARE_RESET_BIT_INDEX);	    
    WDTIMER_CONTROL_REG &= ~(WDTIMER_ENABLE_BIT);	
    WDTIMER_AUTO_RELOAD_REG = 10 * soft_margin;  
    WDTIMER_CONTROL_REG = 0;
    WDTIMER_CONTROL_REG = (WDTIMER_SYSTEM_INTERRUPT_ENABLE_BIT | WDTIMER_EXTERNAL_CLOCK_ENABLE_BIT) & 0x1E;   
    WDTIMER_INTERRUPT_LENGTH_REG = 0x0F;
    WDTIMER_COUNTER_RESTART_REG = 0x0;	
    HAL_WDTIMER_ENABLE();
#ifdef CONFIG_HW_RST
    HAL_WDTIMER_ENABLE_SYSTEM_RESET();
#else
    HAL_WDTIMER_DISABLE_SYSTEM_RESET();
#endif

}
 
static int watchdog_open(struct inode *inode, struct file *file)
{
#ifndef CONFIG_HW_RST
    request_irq(INTC_WATCHDOG_TIMER_BIT_INDEX, watchdog_fire, 0, "watchdog", NULL);
#endif
    watchdog_str8100_hwinit();
    watchdog_ping();
    return nonseekable_open(inode, file);
}

static int watchdog_release(struct inode *inode, struct file *file)
{
    lock_kernel();
#ifndef CONFIG_HW_RST
    free_irq(INTC_WATCHDOG_TIMER_BIT_INDEX, NULL);
#endif
    HAL_WDTIMER_DISABLE_SYSTEM_INTERRUPT();
    HAL_WDTIMER_DISABLE();
    unlock_kernel();
    return 0;
}

static ssize_t watchdog_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
//    if (ppos != &file->f_pos) return -ESPIPE;
    if (len)
    {
      watchdog_ping();
      return 1;
    }
    return 0;
}

static int watchdog_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int new_margin;
    
    static struct watchdog_info ident=
    {
       WDIOF_SETTIMEOUT,
       0,
       "Str8100 Watchdog"
    };
    switch(cmd)
    {
		case WDIOC_GETSUPPORT:
			if (copy_to_user((struct watchdog_info *)arg, &ident, sizeof(ident)))
			  return -EFAULT;
			return 0;
		case WDIOC_GETSTATUS:
		case WDIOC_GETBOOTSTATUS:
			return put_user(0,(int *)arg);
		case WDIOC_KEEPALIVE:
			watchdog_ping();
			return 0;
		case WDIOC_SETTIMEOUT:
			if (get_user(new_margin, (int *)arg))
			  return -EFAULT;
			if ((new_margin < 0) || (new_margin > 60))
			  return -EINVAL;
			soft_margin = new_margin;
			watchdog_str8100_hwinit();
			watchdog_ping();
		case WDIOC_GETTIMEOUT:
			return put_user(soft_margin, (int *)arg);
		default:
			return -ENOTTY;			
    }
}

static struct file_operations watchdog_fops=
{
	owner:		THIS_MODULE,
	write:		watchdog_write,
	ioctl:		watchdog_ioctl,
	open:		watchdog_open,
	release:	watchdog_release,
};

static struct miscdevice watchdog_miscdev=
{
    WATCHDOG_MINOR,
    "watchdog",
    &watchdog_fops
};

static int __init str8100_watchdog_init(void)
{
    int retval;	
    
    if (machine_is_netwinder())
		return -ENODEV;
    str8100_set_interrupt_trigger (INTC_WATCHDOG_TIMER_BIT_INDEX,INTC_IRQ_INTERRUPT,INTC_EDGE_TRIGGER,INTC_RISING_EDGE);
    retval = misc_register(&watchdog_miscdev);
    if (retval < 0) return retval;
    printk(KERN_INFO "str8100_wdt.o: watchdog module version %s\n", STR8100_WATCHDOG_VERSION);
    return 0;
}

static void __exit str8100_watchdog_exit(void) 
{ 
    misc_deregister(&watchdog_miscdev);	
}

module_init(str8100_watchdog_init);
module_exit(str8100_watchdog_exit);

MODULE_AUTHOR("Rober Hsu");
MODULE_DESCRIPTION("Watchdog driver for Str8100");
MODULE_LICENSE("GPL");


