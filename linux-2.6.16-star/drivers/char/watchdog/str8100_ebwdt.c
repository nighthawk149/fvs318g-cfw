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
#include <linux/interrupt.h>
#include <asm/hardware.h>

#define STR8100_WATCHDOG_DATE       "20070118"
#define STR8100_WATCHDOG_VERSION    "2.0.0"
#define DEVICE_NAME                 "embeded watchdog"

extern void str8100_set_interrupt_trigger (unsigned int, unsigned int, unsigned int, unsigned int);

static void update_watchdog(unsigned long data);
static struct timer_list watchdog_update_timer =
		TIMER_INITIALIZER(update_watchdog, 0, 0);

#define TIMER_MARGIN	60	/* about 60 second */

static int soft_margin = TIMER_MARGIN;
static int update_margin = TIMER_MARGIN-10;

module_param(soft_margin, int, 0);
MODULE_PARM_DESC(soft_margin, "Watchdog soft_margin in seconds. (0<soft_margin<65536, default=" __MODULE_STRING(TIMER_MARGIN) ")");
module_param(update_margin, int, 0);
MODULE_PARM_DESC(update_margin, "Watchdog update_margin in seconds. (0<update_margin<65536, default=" __MODULE_STRING(TIMER_MARGIN) "-10)");

#define DEBUG
#ifdef DEBUG
static int wdt_debug = 0;
module_param(wdt_debug, int, 0);
MODULE_PARM_DESC(wdt_debug, "Watchdog debug flag. bit0: debug msg, bit1: no check param, bit2: no reboot");
#define DEBUG_MSG 		0
#define DEBUG_CHKPARM 	1
#define DEBUG_NOREBOOT	2
#endif

static irqreturn_t watchdog_fire(int irq, void *dev_id, struct pt_regs *regs)
{
    printk(KERN_CRIT "Watchdog(%s): Reboot System.(%lx)\n",__FUNCTION__,jiffies);
#ifdef DEBUG
	if(!(wdt_debug&(1<<DEBUG_NOREBOOT)))
#endif
		HAL_PWRMGT_GLOBAL_SOFTWARE_RESET();

    return IRQ_HANDLED;
}

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
}
 

static void update_watchdog(unsigned long data){
#ifdef DEBUG
	if(wdt_debug&(1<<DEBUG_MSG))
		printk("%s: jiffies=%lx, jiffies+(update_margin*HZ)=%lx\n",__FUNCTION__,jiffies,jiffies+(update_margin*HZ));
#endif
	watchdog_ping();
	mod_timer(&watchdog_update_timer, jiffies+(update_margin*HZ));
      
}

static int __init str8100_watchdog_init(void)
{
	printk(KERN_INFO "str8100_wdt.o: watchdog module version %s\n", STR8100_WATCHDOG_VERSION);
#ifdef DEBUG
	if(!(wdt_debug&(1<<DEBUG_CHKPARM)))
#endif
	{
		if(soft_margin<=0||soft_margin>65535){
			soft_margin = TIMER_MARGIN;
			update_margin = TIMER_MARGIN-10;
		}
		if(update_margin>soft_margin){
			update_margin = soft_margin-10;
			if(update_margin<=0){
				update_margin = 1;
			}
		}
	}
#ifdef DEBUG
	if(wdt_debug&(1<<DEBUG_MSG))
		printk("%s: soft_margin=%d, update_margin=%d\n",__FUNCTION__, soft_margin, update_margin);
#endif
    str8100_set_interrupt_trigger (INTC_WATCHDOG_TIMER_BIT_INDEX,INTC_IRQ_INTERRUPT,INTC_EDGE_TRIGGER,INTC_RISING_EDGE);
    request_irq(INTC_WATCHDOG_TIMER_BIT_INDEX, watchdog_fire, 0, "watchdog", NULL);
    watchdog_str8100_hwinit();

    watchdog_ping();
	mod_timer(&watchdog_update_timer, jiffies+(update_margin*HZ));

    return 0;
}

static void __exit str8100_watchdog_exit(void) 
{ 
//    misc_deregister(&watchdog_miscdev);	
#ifdef DEBUG
	if(wdt_debug&(1<<DEBUG_MSG))
		printk("%s: \n",__FUNCTION__);
#endif
    lock_kernel();
    free_irq(INTC_WATCHDOG_TIMER_BIT_INDEX, NULL);
	del_timer(&watchdog_update_timer);
    HAL_WDTIMER_DISABLE_SYSTEM_INTERRUPT();
    HAL_WDTIMER_DISABLE();
    unlock_kernel();
}

module_init(str8100_watchdog_init);
module_exit(str8100_watchdog_exit);

MODULE_AUTHOR("Mac Lin");
MODULE_DESCRIPTION("Watchdog driver for Str8100");
