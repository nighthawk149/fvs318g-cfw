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
 
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/sysctl.h>
#include <linux/rtc.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include <asm/arch/star_rtc.h>
#include <asm/arch/star_intc.h>

#define STR9100_RTC_DATE        "20060628"
#define STR9100_RTC_VERSION     "2.0.0"
#define DEVICE_NAME             "rtc"
#define SECS_PER_HOUR           (60 * 60)
#define SECS_PER_DAY            (SECS_PER_HOUR * 24)
#define TM_YEAR_BASE            1900
#define EPOCH_YEAR              1970
#define RTC_INTR_ALARM		0x20

extern spinlock_t rtc_lock;

static int rtc_busy = 0;
static unsigned long epoch = 1900;
static unsigned int  rtc_interrupt_flag = 0;
static time_t local_rtc_offset, set_rtc_offset, current_rtc_time;
static DECLARE_WAIT_QUEUE_HEAD(str9100_rtc_wait);

# define __isleap(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

static const unsigned char days_in_mo[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                                                                                
static const unsigned short int __mon_yday[2][13] =
{
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

static int offtime (const time_t *t, long int offset, struct rtc_time *tp)
{
    long int days, rem, y;
    const unsigned short int *ip;
				                                                                                
    days = *t / SECS_PER_DAY;
    rem = *t % SECS_PER_DAY;
    rem += offset;
    while (rem < 0)
    {
         rem += SECS_PER_DAY;
   	 --days;
    }
    while (rem >= SECS_PER_DAY)
    {
         rem -= SECS_PER_DAY;
  	 ++days;
    }
    tp->tm_hour = rem / SECS_PER_HOUR;
    rem %= SECS_PER_HOUR;
    tp->tm_min = rem / 60;
    tp->tm_sec = rem % 60;
    tp->tm_wday = (4 + days) % 7;
    if (tp->tm_wday < 0)
      tp->tm_wday += 7;
    y = 1970;
#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))
    while (days < 0 || days >= (__isleap (y) ? 366 : 365))
    {
   	 long int yg = y + days / 365 - (days % 365 < 0);  
   	 days -= ((yg - y) * 365 + LEAPS_THRU_END_OF (yg - 1)- LEAPS_THRU_END_OF (y - 1));
   	 y = yg;
    }
    tp->tm_year = y - 1900;
    if (tp->tm_year != y - 1900)
      return 0;
    tp->tm_yday = days;
    ip = __mon_yday[__isleap(y)];
    for (y = 11; days < (long int) ip[y]; --y)
       continue;
    days -= ip[y];
    tp->tm_mon = y;
    tp->tm_mday = days + 1;
    return 1;
}

static time_t ydhms_tm_diff (int year, int yday, int hour, int min, int sec, const struct rtc_time *tp)
{
    if (!tp) return 1;
    else {
      int a4 = (year >> 2) + (TM_YEAR_BASE >> 2) - ! (year & 3);
      int b4 = (tp->tm_year >> 2) + (TM_YEAR_BASE >> 2) - ! (tp->tm_year & 3);
      int a100 = a4 / 25 - (a4 % 25 < 0);
      int b100 = b4 / 25 - (b4 % 25 < 0);
      int a400 = a100 >> 2;
      int b400 = b100 >> 2;
      int intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);
      time_t years = year - (time_t) tp->tm_year;
      time_t days = (365 * years + intervening_leap_days + (yday - tp->tm_yday));
      return (60 * (60 * (24 * days+(hour - tp->tm_hour))+(min - tp->tm_min))+(sec - tp->tm_sec));
    }
}

static time_t _mktime(struct rtc_time *tp, time_t *offset)
{
    time_t t;
    struct rtc_time tm;
    int sec = tp->tm_sec;
    int min = tp->tm_min;
    int hour = tp->tm_hour;
    int mday = tp->tm_mday;
    int mon = tp->tm_mon;
    int year_requested = tp->tm_year;

    int mon_remainder = mon % 12;
    int negative_mon_remainder = mon_remainder < 0;
    int mon_years = mon / 12 - negative_mon_remainder;
    int year = year_requested + mon_years;
    int yday = ((__mon_yday[__isleap (year + TM_YEAR_BASE)]
  	       [mon_remainder + 12 * negative_mon_remainder])
  	       + mday - 1);
    if (year < 69) return -1;
    tm.tm_year = EPOCH_YEAR - TM_YEAR_BASE;
    tm.tm_yday = tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    t = ydhms_tm_diff (year, yday, hour, min, sec, &tm);
    if (year == 69)
    {
      if (t < 0 || t > 2 * 24 * 60 * 60) return -1;
    }
    *tp = tm;
    return t;
}

void get_rtc_time (struct rtc_time *rtc_tm)
{
    time_t update_time;
    static time_t old_time = 0;	
    
    spin_lock (&rtc_lock);
    update_time =  RTC_SECOND_REG + RTC_MINUTE_REG * 60 + RTC_HOUR_REG * 60 * 60 + RTC_DAY_REG * 60 * 60 * 24 - local_rtc_offset + set_rtc_offset;
    if (old_time > 0)
    {
      printk ("\n\n Old time is %d, last time is %d, diff is %d\n",old_time,update_time,update_time-old_time);
      printk (" Throughput is %d B/s, %d KB/s\n\n",106096775/(update_time-old_time),(106096775/1024)/(update_time-old_time));
    }
    old_time = update_time;
    offtime(&update_time, 0, rtc_tm);	
    if ((rtc_tm->tm_year += (epoch - 1900)) <= 69)
      rtc_tm->tm_year += 100; 
    spin_unlock (&rtc_lock);
}

int set_rtc_time (struct rtc_time *rtc_tm)
{
    unsigned char mon, day, hrs, min, sec, leap_yr;
    unsigned int yrs;
    
    spin_lock (&rtc_lock);
    yrs = rtc_tm->tm_year + 1900;
    mon = rtc_tm->tm_mon + 1;  
    day = rtc_tm->tm_mday;
    hrs = rtc_tm->tm_hour;
    min = rtc_tm->tm_min;
    sec = rtc_tm->tm_sec; 
    if (yrs < 1970) return -EINVAL;
    leap_yr = ((!(yrs % 4) && (yrs % 100)) || !(yrs % 400));
    if ((mon > 12) || (day == 0)) return -EINVAL;
    if (day > (days_in_mo[mon] + ((mon == 2) && leap_yr))) return -EINVAL;
    if ((hrs >= 24) || (min >= 60) || (sec >= 60)) return -EINVAL;
    if ((yrs -= epoch) > 255) return -EINVAL;   
    local_rtc_offset = RTC_SECOND_REG + RTC_MINUTE_REG * 60 + RTC_HOUR_REG * 60 * 60 + RTC_DAY_REG * 60 * 60 * 24;
    set_rtc_offset = _mktime(rtc_tm, 0);
    spin_unlock (&rtc_lock);
    return 0;
}

int set_rtc_alm_time (struct rtc_time *alm_tm)
{
    unsigned char hrs, min, sec;
    unsigned long alm_sec;
    unsigned int volatile rtc_int_status;
    
    spin_lock_irq(&rtc_lock);
    alm_sec = alm_tm->tm_hour * 3600 + alm_tm->tm_min * 60 + alm_tm->tm_sec;
    hrs = alm_sec / 3600;
    min = (alm_sec % 3600) / 60;
    sec = (alm_sec % 3600) % 60;
    RTC_ALARM_HOUR_REG = hrs;
    RTC_ALARM_MINUTE_REG = min;
    RTC_ALARM_SECOND_REG = sec;
    RTC_CONTROL_REG = RTC_MATCH_ALARM_ENABLE_BIT;
    rtc_int_status = RTC_INTERRUPT_STATE_REG;
    RTC_INTERRUPT_STATE_REG = rtc_int_status;                      
    spin_unlock_irq(&rtc_lock);    
    return 0;
}

static loff_t rtc_lseek(struct file *file, loff_t offset, int origin)
{
    return -ESPIPE;
}

static void mask_rtc_irq_bit(unsigned char bit)
{
    unsigned char val;
    unsigned int volatile rtc_int_status;

    spin_lock_irq(&rtc_lock);
    val = RTC_CONTROL_REG;
    val &=  ~bit;
    RTC_CONTROL_REG = val;
    rtc_int_status = RTC_INTERRUPT_STATE_REG;
    spin_unlock_irq(&rtc_lock);
}

static void set_rtc_irq_bit(unsigned char bit)
{
    unsigned char val;
    unsigned int volatile rtc_int_status;	
        
    spin_lock_irq(&rtc_lock);
    val = RTC_CONTROL_REG;
    val |= bit;
    RTC_CONTROL_REG = val;
    rtc_int_status = RTC_INTERRUPT_STATE_REG;
    spin_unlock_irq(&rtc_lock);
}

static void get_rtc_alm_time(struct rtc_time *alm_tm)
{
    spin_lock_irq(&rtc_lock);
    alm_tm->tm_sec = RTC_ALARM_SECOND_REG;
    alm_tm->tm_min = RTC_ALARM_MINUTE_REG;
    alm_tm->tm_hour = RTC_ALARM_HOUR_REG;
    spin_unlock_irq(&rtc_lock);
}

static int rtc_ioctl(struct inode *inode, struct file *file, unsigned int cmd,unsigned long arg)
{
    struct rtc_time wtime; 

    switch (cmd)
    {
	case RTC_RD_TIME:
	        memset(&wtime, 0, sizeof(struct rtc_time));
		get_rtc_time(&wtime);
		break;
	case RTC_SET_TIME:
	{         
		struct rtc_time rtc_tm;
		if (!capable(CAP_SYS_TIME))
		  return -EPERM;
		if (copy_from_user(&rtc_tm, (struct rtc_time*)arg, sizeof(struct rtc_time)))
		  return -EFAULT;
		set_rtc_time(&rtc_tm);
		return 0;
	}	
        case RTC_ALM_SET:
        {
        	struct rtc_time alm_tm; 
		if (copy_from_user(&alm_tm, (struct rtc_time*)arg, sizeof(struct rtc_time)))
		  return -EFAULT;
		memset(&wtime, 0, sizeof(struct rtc_time));  
		set_rtc_alm_time(&alm_tm);
		return 0;
	}	
	case RTC_ALM_READ:
	        memset(&wtime, 0, sizeof(struct rtc_time));
		get_rtc_alm_time(&wtime);
		break;
	case RTC_AIE_OFF:
	        mask_rtc_irq_bit(RTC_INTR_ALARM);	
		return 0;
	case RTC_AIE_ON:
	        set_rtc_irq_bit(RTC_INTR_ALARM);
		return 0;	
	default:
		return -EINVAL;
    }
    return copy_to_user((void *)arg, &wtime, sizeof wtime) ? -EFAULT : 0;
}

static void rtc_fire(int irq, void *dev_id, struct pt_regs *regs)
{
    unsigned int volatile    rtc_int_status;	
    
    HAL_INTC_DISABLE_INTERRUPT_SOURCE(INTC_RTC_BIT_INDEX);
    HAL_RTC_READ_INTERRUPT_STATUS(rtc_int_status);
    HAL_RTC_WRITE_INTERRUPT_STATUS(rtc_int_status);
    if (rtc_int_status & RTC_AUTO_SECOND_ALARM_INTERRUPT_BIT) rtc_interrupt_flag |= RTC_AUTO_SECOND_ALARM_INTERRUPT_BIT;
    if (rtc_int_status & RTC_AUTO_MINUTE_ALARM_INTERRUPT_BIT) rtc_interrupt_flag |= RTC_AUTO_MINUTE_ALARM_INTERRUPT_BIT;
    if (rtc_int_status & RTC_AUTO_HOUR_ALARM_INTERRUPT_BIT) rtc_interrupt_flag |= RTC_AUTO_HOUR_ALARM_INTERRUPT_BIT;
    if (rtc_int_status & RTC_AUTO_DAY_ALARM_INTERRUPT_BIT) rtc_interrupt_flag |= RTC_AUTO_DAY_ALARM_INTERRUPT_BIT;
    if (rtc_int_status & RTC_MATCH_ALARM_INTERRUPT_BIT) {rtc_interrupt_flag |= RTC_MATCH_ALARM_INTERRUPT_BIT;
      wake_up_interruptible(&str9100_rtc_wait);
    }
    if (rtc_int_status & RTC_BATTERY_LOW_VOLTAGE_INTR_BIT)
    {
      rtc_interrupt_flag |= RTC_BATTERY_LOW_VOLTAGE_INTR_BIT;
      printk("str9100 rtc: Low Battery Voltage!!\n");
    }
    if (!(rtc_interrupt_flag & RTC_MATCH_ALARM_INTERRUPT_BIT)) HAL_INTC_ENABLE_INTERRUPT_SOURCE(INTC_RTC_BIT_INDEX);
}

static void rtc_str9100_hwinit(int ctrl, int hour, int min, int sec)
{    	
    RTC_CONTROL_REG &= ~(RTC_ENABLE_BIT);	
    if (ctrl & RTC_MATCH_ALARM_ENABLE_BIT)
    {
      RTC_ALARM_SECOND_REG = sec;
      RTC_ALARM_MINUTE_REG = min;
      RTC_ALARM_HOUR_REG = hour;    	
    }else{
      RTC_ALARM_SECOND_REG = 0;
      RTC_ALARM_MINUTE_REG = 0;
      RTC_ALARM_HOUR_REG = 0;
    } 
    RTC_CONTROL_REG = ctrl;
} 

static int rtc_open(struct inode *inode, struct file *file)
{
    if (rtc_busy) return -EBUSY;

    rtc_busy = 1;
    return 0;
}

static ssize_t rtc_read(struct file * file, char *buf, size_t count, loff_t * ppos)
{
    DECLARE_WAITQUEUE(wait, current);
    unsigned long data;
    ssize_t retval;

    if (count < sizeof(unsigned long))
      return -EINVAL;

    add_wait_queue(&str9100_rtc_wait, &wait);
    set_current_state(TASK_INTERRUPTIBLE);
    for (;;) {
	spin_lock(&rtc_lock);
	data = rtc_interrupt_flag;
	if (data != 0) {
	  rtc_interrupt_flag = 0;
	  break;
	}
        spin_unlock(&rtc_lock);
        if (file->f_flags & O_NONBLOCK) {
	  retval = -EAGAIN;
	  goto out;
	}
	if (signal_pending(current)) {
	  retval = -ERESTARTSYS;
	  goto out;
	}
        schedule();
    }
    spin_unlock(&rtc_lock);
    retval = put_user(data, (unsigned long *) buf);
    if (!retval) retval = sizeof(unsigned long);
out:
    mask_rtc_irq_bit(RTC_INTR_ALARM);
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&str9100_rtc_wait, &wait);
    return retval;
}

static int rtc_release(struct inode *inode, struct file *file)
{
    rtc_busy = 0;
    return 0;
}

static struct file_operations rtc_fops = {
    owner:	THIS_MODULE,
    llseek:	rtc_lseek,
    ioctl:	rtc_ioctl,
    open:	rtc_open,
    read:       rtc_read,
    release:	rtc_release
};

static struct miscdevice rtc_dev = { RTC_MINOR, "rtc", &rtc_fops };

static int rtc_proc_output (char *buf)
{
    char *p;
    struct rtc_time tm;
    
    p = buf;
    get_rtc_time(&tm);
    p += sprintf(p,"rtc_time\t: %02d:%02d:%02d\n"
		   "rtc_date\t: %04d-%02d-%02d\n"
	 	   "rtc_epoch\t: %04lu\n",
		   tm.tm_hour, tm.tm_min, tm.tm_sec,
		   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, epoch);
    get_rtc_alm_time(&tm);
    p += sprintf(p, "alarm\t\t: ");
    if (tm.tm_hour <= 24)
      p += sprintf(p, "%02d:", tm.tm_hour);
    else
      p += sprintf(p, "**:");
    if (tm.tm_min <= 59)
      p += sprintf(p, "%02d:", tm.tm_min);
    else
      p += sprintf(p, "**:");
    if (tm.tm_sec <= 59)
      p += sprintf(p, "%02d\n", tm.tm_sec);
    else
      p += sprintf(p, "**\n");	
    return  p - buf;
}

static int rtc_read_proc(char *page, char **start, off_t off,int count, int *eof, void *data)
{
    int len = rtc_proc_output (page);
    
    if (len <= off+count) *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count) len = count;
    if (len<0) len = 0;
    return len;
}

static int __init str9100_rtc_init(void)
{
    int error;

    error = misc_register(&rtc_dev);
    if (error) {
      printk(KERN_ERR "rtc: unable to get misc minor\n");
      return error;
    }	
    printk(KERN_INFO "STR9100 Real Time Clock Driver v" STR9100_RTC_VERSION "\n");
    rtc_str9100_hwinit(0,0,0,0);
    HAL_RTC_ENABLE();
    request_irq(INTC_RTC_BIT_INDEX, rtc_fire, 0, DEVICE_NAME, NULL);
    create_proc_read_entry ("driver/rtc", 0, 0, rtc_read_proc, NULL);
    local_rtc_offset = 0;
    set_rtc_offset = current_rtc_time = 0;        
    return 0;
}

static void __exit str9100_rtc_exit (void)
{
    char buf[64];
    	
    HAL_RTC_DISABLE();		
    cleanup_sysctl();
    sprintf (buf,"driver/%s",DEVICE_NAME);
    remove_proc_entry (buf, NULL);
    misc_deregister(&rtc_dev);
    free_irq (INTC_RTC_BIT_INDEX, NULL);
}

module_init(str9100_rtc_init);
module_exit(str9100_rtc_exit);

MODULE_LICENSE("GPL");
