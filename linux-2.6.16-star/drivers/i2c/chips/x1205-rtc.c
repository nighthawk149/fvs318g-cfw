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
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/mc146818rtc.h>
#include <linux/init.h>
#include <linux/device.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/rtc.h>

#include <asm/mach/time.h>

#include <linux/x1205.h>





static int i2c_x1205_rtc_read_time(struct rtc_time *tm)
{
        unsigned long time;

	tm->tm_year+=100; // only 0-99
	x1205_do_command(X1205_CMD_GETDATETIME, tm);
	//printk("X1205_CMD_GETDATETIME: sec: %d\n", tm->tm_sec);
        return 0;
}

static inline int i2c_x1205_rtc_set_time(struct rtc_time *tm)
{
	//printk("x1205_do_command(X1205_CMD_SETTIME, tm);\n");
	
	//tm->tm_year-=100; // only 0-99
#if 0
	printk("in i2c_x1205_rtc_set_time %s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__FUNCTION__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);
#endif
	//x1205_do_command(X1205_CMD_SETTIME, tm);
	x1205_do_command(X1205_CMD_SETDATETIME, tm);

        return 0;
}

static inline int i2c_x1205_rtc_set_alarm(struct rtc_wkalrm *alrm)
{
        return 0;
}

static inline int i2c_x1205_rtc_read_alarm(struct rtc_wkalrm *alrm)
{
        return 0;
}

static int i2c_x1205_set_rtc(void)
{
        unsigned long record;

        return 1;
}




static struct rtc_ops i2c_x1205_rtc_ops = {
        .owner          = THIS_MODULE,
        .read_time      = i2c_x1205_rtc_read_time,
        .set_time       = i2c_x1205_rtc_set_time,
        .read_alarm     = i2c_x1205_rtc_read_alarm,
        .set_alarm      = i2c_x1205_rtc_set_alarm,
};

extern int (*set_rtc)(void);


int i2c_x1205_rtc_init(void)
{
        int ret;

        ret = register_rtc(&i2c_x1205_rtc_ops);
	if (ret) return ret;

        //set_rtc = i2c_x1205_set_rtc;

#if 0
        i2c_x1205_rtc_hw_init();


        // set RTC clock from system time
        i2c_x1205_set_rtc();

        ret = register_rtc(&i2c_x1205_rtc_ops);
        if (ret) return ret;

        set_rtc = i2c_x1205_set_rtc;

        RTC_SECOND_ALARM_REG    = 0;
        RTC_MINUTE_ALARM_REG    = 0;
        RTC_HOUR_ALARM_REG      = 0;

        // enable RTC
        HAL_RTC_ENABLE();
#endif
        return 0;
}

//module_init(i2c_x1205_rtc_init);

