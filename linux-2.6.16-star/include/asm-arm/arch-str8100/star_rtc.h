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


#ifndef	_STAR_RTC_H_
#define	_STAR_RTC_H_

#include <asm/arch/star_sys_memory_map.h>

#define	RTC_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_RTC_BASE_ADDR + reg_offset)))

#define	RTC_SECOND_REG				RTC_MEM_MAP_VALUE(0x00)
#define	RTC_MINUTE_REG				RTC_MEM_MAP_VALUE(0x04)
#define	RTC_HOUR_REG				RTC_MEM_MAP_VALUE(0x08)
#define	RTC_DAY_REG				RTC_MEM_MAP_VALUE(0x0C)
#define	RTC_SECOND_ALARM_REG			RTC_MEM_MAP_VALUE(0x10)
#define	RTC_MINUTE_ALARM_REG			RTC_MEM_MAP_VALUE(0x14)
#define	RTC_HOUR_ALARM_REG			RTC_MEM_MAP_VALUE(0x18)
#define	RTC_RECORD_REG				RTC_MEM_MAP_VALUE(0x1C)
#define	RTC_CONTROL_REG				RTC_MEM_MAP_VALUE(0x20)
#define	RTC_INTERRUPT_STATUS_REG		RTC_MEM_MAP_VALUE(0x34)

#define	RTC_ENABLE_BIT				(1 << 0)
#define	RTC_AUTO_SECOND_ALARM_ENABLE_BIT	(1 << 1)
#define	RTC_AUTO_MINUTE_ALARM_ENABLE_BIT	(1 << 2)
#define	RTC_AUTO_HOUR_ALARM_ENABLE_BIT		(1 << 3)
#define	RTC_AUTO_DAY_ALARM_ENABLE_BIT		(1 << 4)
#define	RTC_MATCH_ALARM_ENABLE_BIT		(1 << 5)
#define	RTC_BATTERY_LOW_VOLTAGE_ENABLE_BIT	(1 << 6)

#define RTC_AUTO_SECOND_ALARM_INTR_BIT      (1 << 0)
#define RTC_AUTO_MINUTE_ALARM_INTR_BIT      (1 << 1)
#define RTC_AUTO_HOUR_ALARM_INTR_BIT        (1 << 2)
#define RTC_AUTO_DAY_ALARM_INTR_BIT         (1 << 3)
#define RTC_MATCH_ALARM_INTR_BIT            (1 << 4)
#define RTC_BATTERY_LOW_VOLTAGE_INTR_BIT    (1 << 5)

#define	HAL_RTC_READ_SECOND(second)         ((second) = (RTC_SECOND_REG) & 0x3F);
#define	HAL_RTC_READ_MINUTE(minute)         ((minute) = (RTC_MINUTE_REG) & 0x3F);
#define	HAL_RTC_READ_HOUR(hour)	            ((hour) = (RTC_HOUR_REG) & 0x1F);
#define	HAL_RTC_READ_DAY(day)               ((day) = (RTC_DAY_REG) & 0xFFFF);
#define	HAL_RTC_ENABLE()                    ((RTC_CONTROL_REG) |= (RTC_ENABLE_BIT));
#define	HAL_RTC_DISABLE()                   ((RTC_CONTROL_REG) &= ~(RTC_ENABLE_BIT));
#define	HAL_RTC_AUTO_SECOND_ALARM_ENABLE()  ((RTC_CONTROL_REG) |= (RTC_AUTO_SECOND_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_SECOND_ALARM_DISABLE() ((RTC_CONTROL_REG) &= ~(RTC_AUTO_SECOND_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_MINUTE_ALARM_ENABLE()  ((RTC_CONTROL_REG) |= (RTC_AUTO_MINUTE_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_MINUTE_ALARM_DISABLE() ((RTC_CONTROL_REG) &= ~(RTC_AUTO_MINUTE_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_HOUR_ALARM_ENABLE()    ((RTC_CONTROL_REG) |= (RTC_AUTO_HOUR_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_HOUR_ALARM_DISABLE()   ((RTC_CONTROL_REG) &= ~(RTC_AUTO_HOUR_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_DAY_ALARM_ENABLE()	    ((RTC_CONTROL_REG) |= (RTC_AUTO_DAY_ALARM_ENABLE_BIT));
#define	HAL_RTC_AUTO_DAY_ALARM_DISABLE()    ((RTC_CONTROL_REG) &= ~(RTC_AUTO_DAY_ALARM_ENABLE_BIT));
#define	HAL_RTC_MATCH_ALARM_ENABLE()        ((RTC_CONTROL_REG) |= (RTC_MATCH_ALARM_ENABLE_BIT));
#define	HAL_RTC_MATCH_ALARM_DISABLE()       ((RTC_CONTROL_REG) &= ~(RTC_MATCH_ALARM_ENABLE_BIT));
#define	HAL_RTC_BATTERY_LOW_VOLTAGE_INTERRUPT_ENABLE()   ((RTC_CONTROL_REG) |= (RTC_BATTERY_LOW_VOLTAGE_ENABLE_BIT));
#define	HAL_RTC_BATTERY_LOW_VOLTAGE_INTERRUPT_DISABLE()	 ((RTC_CONTROL_REG) &= ~(RTC_BATTERY_LOW_VOLTAGE_ENABLE_BIT));
#define	HAL_RTC_WRITE_RECORD(record)        ((RTC_RECORD_REG) =	(record));
#define	HAL_RTC_READ_RECORD(record)         ((record) =	(RTC_RECORD_REG)); 
#define	HAL_RTC_WRITE_MATCHED_ALARM_SECOND(second)  ((RTC_SECOND_ALARM_REG) = (second &	0x3F));
#define	HAL_RTC_READ_MATCHED_ALARM_SECOND(second)   ((second) =	(RTC_SECOND_ALARM_REG) & 0x3F);
#define	HAL_RTC_WRITE_MATCHED_ALARM_MINUTE(minute)  ((RTC_MINUTE_ALARM_REG) = (minute &	0x3F));
#define	HAL_RTC_READ_MATCHED_ALARM_MINUTE(minute)   ((minute) =	(RTC_MINUTE_ALARM_REG) & 0x3F);
#define	HAL_RTC_WRITE_MATCHED_ALARM_HOUR(hour)      ((RTC_HOUR_ALARM_REG) = (hour & 0x1F));
#define	HAL_RTC_READ_MATCHED_ALARM_HOUR(hour)       ((hour) = (RTC_HOUR_ALARM_REG) & 0x1F);
#define	HAL_RTC_READ_INTERRUPT_STATUS(status)       ((status) =	(RTC_INTERRUPT_STATUS_REG) & 0x3F);
#define	HAL_RTC_WRITE_INTERRUPT_STATUS(status)      ((RTC_INTERRUPT_STATUS_REG)	= (status) & 0x3F);

#endif

