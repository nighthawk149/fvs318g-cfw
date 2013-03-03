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

#ifndef __STR9100_COUNTER_H_
#define __STR9100_COUNTER_H_
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <linux/module.h>



#define STR9100_TIMER_COUNTER_MAX 20

typedef struct str9100_counter {
	u32 count;
	u64 time;
	u32 temp;
} str9100_counter;


void str9100_counter_init( void );
void str9100_counter_index_init( int index );
void str9100_counter_enable ( void );
void str9100_counter_disable ( void );
void str9100_counter_start ( int index );
void str9100_counter_end ( int index );
str9100_counter *str9100_counter_get ( int index );


#endif /* __STR9100_COUNTER_H_ */

