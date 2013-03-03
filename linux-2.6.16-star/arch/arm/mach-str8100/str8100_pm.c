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

#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>

#include <asm/mach-types.h>

#ifdef CONFIG_PM
/******************************************************************************
 *
 * FUNCTION:  Hal_Cpu_Enter_Sleep_Mode
 * PURPOSE:   
 *
 ******************************************************************************/
#ifdef CONFIG_PM_DEBUG
#define PM_DEBUG printk
#else
#define PM_DEBUG(_parm,...) 
#endif
void inline Hal_Cpu_Enter_Sleep_Mode(void)
{
	__asm__ volatile (
				"nop\n"
				"mcr	p15, 0, r0, c7, c0, 5\n");
}

static int str8100_pm_prepare(suspend_state_t state)
{
	int error = 0;
	PM_DEBUG("%s: state=%d\n",__FUNCTION__,state);
	switch (state)
	{
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		break;

	case PM_SUSPEND_DISK:
		return -ENOTSUPP;

	default:
		return -EINVAL;
	}

	return error;
}


static int str8100_pm_enter(suspend_state_t state)
{
	PM_DEBUG("%s: state=%d\n",__FUNCTION__,state);
	switch (state)
	{
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:

#ifdef DEBUG
		/*
		* Configure system Xtal clock to be output to CLKOUT pin
		*/
		HAL_PWRMGT_CONFIGURE_CLOCK_OUT_PIN(0, 0);
#endif

		/*
		* 1. Disable DRAM Controller's clock
		* 2. Power-down sytem 25MHz XTAL pad
		* 3. Force CPU into sleep mode, and wait until wake-up interrupt happens!!
		*    When in sleep mode, CPU internal clock and system PLLs and/or 25MHZ XTAL 
		*    pad will be power-down!!
		*/

//int 16, 18, 28-30
//#define WAKEUP_INT 0x70050000
//int 16, 18, 29-30
//#define WAKEUP_INT 0x60050000
//int 18, 29-30
#define WAKEUP_INT 0x60040000
//int 29-30
//#define WAKEUP_INT 0x60000000
//#define WAKEUP_INT 0xffffffff
		PM_DEBUG("%s: int that can wake cpu up, WAKEUP_INT=%.8x\n",__FUNCTION__,WAKEUP_INT);
//		HAL_INTC_SELECT_INTERRUPT_SOURCE_FOR_SLEEP_WAKEUP(30);
		INTC_POWER_MANAGEMENT_INTERRUPT_REG=WAKEUP_INT;
		HAL_PWRMGT_DISABLE_DRAMC_CLOCK();
#ifndef CONFIG_STAR_NIC_PHY_INTERNAL_PHY
		HAL_PWRMGT_POWER_DOWN_SYSTEM_XTAL_PAD();
#endif

		PM_DEBUG("%s: bye...\n",__FUNCTION__);
		str8100_nic_suspend(state);
		local_irq_enable();
		Hal_Cpu_Enter_Sleep_Mode();
		local_irq_disable();
		str8100_nic_resume();
		PM_DEBUG("%s: awake from sleep\n",__FUNCTION__);

		break;

	case PM_SUSPEND_DISK:
		return -ENOTSUPP;

	default:
		return -EINVAL;
	}

	return 0;
}
/*
static int str8100_pm_finish(suspend_state_t state)
{
printk("%s: \n",__FUNCTION__);
	return 0;
}
*/

static struct pm_ops str8100_pm_ops ={
	.pm_disk_mode = 0,
        .prepare        = str8100_pm_prepare,
        .enter          = str8100_pm_enter,
//        .finish         = str8100_pm_finish,
};

static int __init str8100_pm_init(void)
{
	int ret;
printk("%s: \n",__FUNCTION__);

    /*
     * Configure system Xtal clock to be output to CLKOUT pin
     */
	HAL_PWRMGT_CONFIGURE_CLOCK_OUT_PIN(0, 0);

	pm_set_ops(&str8100_pm_ops);
	return 0;
}
__initcall(str8100_pm_init);


#endif//CONFIG_PM
