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


#ifndef __STR9100_GPIO_H__
#define __STR9100_GPIO_H__

#define PIN_INPUT 0
#define PIN_OUTPUT 1

#define GPIO_MASK_0	0x00000001
#define GPIO_MASK_1	0x00000002
#define GPIO_MASK_2	0x00000004
#define GPIO_MASK_3	0x00000008
#define GPIO_MASK_4	0x00000010
#define GPIO_MASK_5	0x00000020
#define GPIO_MASK_6	0x00000040
#define GPIO_MASK_7	0x00000080
#define GPIO_MASK_8	0x00000100
#define GPIO_MASK_9	0x00000200
#define GPIO_MASK_10	0x00000400
#define GPIO_MASK_11	0x00000800
#define GPIO_MASK_12	0x00001000
#define GPIO_MASK_13	0x00002000
#define GPIO_MASK_14	0x00004000
#define GPIO_MASK_15	0x00008000
#define GPIO_MASK_16	0x00010000
#define GPIO_MASK_17	0x00020000
#define GPIO_MASK_18	0x00040000
#define GPIO_MASK_19	0x00080000
#define GPIO_MASK_20	0x00100000
#define GPIO_MASK_21	0x00200000
#define GPIO_MASK_ALL	0x003FFFFF

#define PIN_INPUT 0
#define PIN_OUTPUT 1

#define PIN_TRIG_EDGE 0
#define PIN_TRIG_LEVEL 1

#define PIN_TRIG_SINGLE 0
#define PIN_TRIG_BOTH 1

#define PIN_TRIG_RISING 0
#define PIN_TRIG_FAILING 1

#define PIN_TRIG_HIGH 0
#define PIN_TRIG_LOW 1


#ifndef  INTC_CLEAR_EDGE_TRIGGER_INTERRUPT
#define INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(source_bit_index) \
    (INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG) |= ((1 << source_bit_index))
#endif

extern int str9100_gpio_in( volatile __u32 *data);
extern int str9100_gpio_out( volatile __u32 data);
extern int str9100_gpio_read_direction( volatile __u32 *data);
extern int str9100_gpio_write_direction( volatile __u32 data);
extern int str9100_gpio_dataset( volatile __u32 data);
extern int str9100_gpio_dataclear( volatile __u32 data);

extern int str9100_gpio_in_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_out_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_direction_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_direction_bit(volatile __u32 data,int bit);
extern int str9100_gpio_dataset_bit(volatile __u32 data,int bit);
extern int str9100_gpio_dataclear_bit(volatile __u32 data,int bit);

#ifdef CONFIG_STR9100_GPIO_INTERRUPT
extern int str9100_gpio_read_intrenable(volatile __u32 *data);
extern int str9100_gpio_read_intrenable_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrenable(volatile __u32 data);
extern int str9100_gpio_write_intrenable_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_intrrawstate( volatile __u32 *data);
extern int str9100_gpio_read_intrrawstate_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_read_intrmaskedstatus( volatile __u32 *data);
extern int str9100_gpio_read_intrmaskedstatus_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_read_intrmask( volatile __u32 *data);
extern int str9100_gpio_read_intrmask_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrmask(__u32 data);
extern int str9100_gpio_write_intrmask_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_intrclear(volatile __u32 *data);
extern int str9100_gpio_read_intrclear_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrclear(__u32 data);
extern int str9100_gpio_write_intrclear_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_intrtrigger(volatile __u32 *data);
extern int str9100_gpio_read_intrtrigger_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrtrigger(__u32 data);
extern int str9100_gpio_write_intrtrigger_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_intrboth(volatile __u32 *data);
extern int str9100_gpio_read_intrboth_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrboth(__u32 data);
extern int str9100_gpio_write_intrboth_bit(volatile __u32 data,int bit);
extern int str9100_gpio_read_intrriseneg(volatile __u32 *data);
extern int str9100_gpio_read_intrriseneg_bit(volatile __u32 *data,int bit);
extern int str9100_gpio_write_intrriseneg(__u32 data);
extern int str9100_gpio_write_intrriseneg_bit(volatile __u32 data,int bit);

extern void str9100_gpio_set_edgeintr(void (*funcptr)(int),int trig_both,int trig_rising, int gpio_pin);
extern void str9100_gpio_clear_intr(int gpio_pin);
extern void str9100_gpio_set_levelintr(void (*funcptr)(int),int trig_level, int gpio_pin);
#endif


#endif
