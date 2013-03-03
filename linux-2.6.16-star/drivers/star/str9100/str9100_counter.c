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

#include <linux/str9100/str9100_counter.h>
#ifdef CONFIG_CPU_DSPAD_ENABLE
__attribute__((section(".dspad")))
#endif
str9100_counter COUNTER[ STR9100_TIMER_COUNTER_MAX ] ;
#ifdef CONFIG_CPU_DSPAD_ENABLE
__attribute__((section(".dspad")))
#endif
u32 test[8192];

void str9100_counter_init( void ) {
		u32 control_value,mask_value;
		control_value = TIMER_CONTROL_REG;
		control_value &= ~(1 << TIMER2_CLOCK_SOURCE_BIT_INDEX);
		control_value &= ~(1 << TIMER2_UP_DOWN_COUNT_BIT_INDEX);
		TIMER_CONTROL_REG = control_value;

		mask_value = TIMER_INTERRUPT_MASK_REG;
		mask_value |= (0x7 << 3);
		TIMER_INTERRUPT_MASK_REG = mask_value;

		memset ( &COUNTER[0] , 0,
			sizeof(str9100_counter)*STR9100_TIMER_COUNTER_MAX );
}

void str9100_counter_index_init( int index ){
	if (index >= STR9100_TIMER_COUNTER_MAX ) index = 0;
	COUNTER[index].temp = 0;
	COUNTER[index].count = 0;
	COUNTER[index].time = 0;	
}

str9100_counter *str9100_counter_get ( int index ) {

	return &COUNTER[index];
}

inline void str9100_counter_enable ( void ) {
	u32 volatile control_value;
	TIMER2_COUNTER_REG=0;
	control_value = TIMER_CONTROL_REG;
	control_value |= (1 << TIMER2_ENABLE_BIT_INDEX);
	TIMER_CONTROL_REG = control_value;
}
inline void str9100_counter_disable ( void ) {
	u32 volatile control_value;
	control_value = TIMER_CONTROL_REG;
	control_value &= ~(1 << TIMER2_ENABLE_BIT_INDEX);
	TIMER_CONTROL_REG = control_value;

}
inline void str9100_counter_start ( int index ){
	if (index >= STR9100_TIMER_COUNTER_MAX ) index = 0;

	COUNTER[index].temp = TIMER2_COUNTER_REG;
}

inline void str9100_counter_end ( int index ){
	if (index >= STR9100_TIMER_COUNTER_MAX ) index = 0;

	COUNTER[index].count++;
	COUNTER[index].time += (TIMER2_COUNTER_REG - COUNTER[index].temp);
}

EXPORT_SYMBOL(str9100_counter_init);
EXPORT_SYMBOL(str9100_counter_index_init);
EXPORT_SYMBOL(str9100_counter_enable);
EXPORT_SYMBOL(str9100_counter_disable);
EXPORT_SYMBOL(str9100_counter_start);
EXPORT_SYMBOL(str9100_counter_end);
EXPORT_SYMBOL(str9100_counter_get);
