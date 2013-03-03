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


//#include <linux/config.h>
//#include <asm/io.h>
//#include <asm/hardware.h>

//#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/arch/star_gpio.h>

#define LED_MASK_A 			0xfe3fe07f
#define LED_MASK_B 			0xffe0dfff

#if 0
#define LED_MASK 					LED_MASK_A
#define GPIO_DIRECTION_REG			GPIOA_DIRECTION_REG
#define GPIO_DATA_OUTPUT_REG 		GPIOA_DATA_OUTPUT_REG
#define GPIO_DATA_BIT_SET_REG 		GPIOA_DATA_BIT_SET_REG
#define GPIO_DATA_BIT_CLEAR_REG 	GPIOA_DATA_BIT_CLEAR_REG
#else
#define LED_MASK 					LED_MASK_B
#define GPIO_DIRECTION_REG			GPIOB_DIRECTION_REG
#define GPIO_DATA_OUTPUT_REG 		GPIOB_DATA_OUTPUT_REG
#define GPIO_DATA_BIT_SET_REG 		GPIOB_DATA_BIT_SET_REG
#define GPIO_DATA_BIT_CLEAR_REG 	GPIOB_DATA_BIT_CLEAR_REG
#endif


#define LED_DELAY_MS		50

/*
 * Configure all LEDs on
 */
void str8100_led_all_on(void)
{
    /*
     * perform Write Low to GPIO Pin
     */    
    GPIO_DATA_BIT_CLEAR_REG |= LED_MASK;
}



/*
 * Configure all LEDs off
 */
void str8100_led_all_off(void)
{
    /*
     * perform Write High to GPIO Pin
     */
    GPIO_DATA_BIT_SET_REG |= LED_MASK;
}


/*
 * Configure one LED on
 */
void str8100_led_on(unsigned int led_index)
{
    /*
     * perform Write Low to GPIO Pin
     */
    GPIO_DATA_BIT_CLEAR_REG |= (led_index & LED_MASK);
}


/*
 * Configure one LED off
 */
void str8100_led_off(unsigned int led_index)
{
    /*
     * perform Write High to GPIO Pin
     */
    GPIO_DATA_BIT_SET_REG |= (led_index & LED_MASK);
}



/*
 * Toggle one LED on/off
 */
void str8100_led_toggle(unsigned int led_index)
{     
    volatile unsigned int    data_out_state;


    /*
     * 1. read GPIO Data Out State
     * 2. if GPIO High, turn LED on, otherwise, turn LED off
     */
    data_out_state = GPIO_DATA_OUTPUT_REG;
    
    if (data_out_state & led_index& LED_MASK)
    {
        // GPIO High, i.e., LED is off. Now, turn it on
        str8100_led_on(led_index & LED_MASK);
    }
    else
    {
        // GPIO Low, i.e., LED is on. Now turn it off
        str8100_led_off(led_index & LED_MASK);
    }
}


/*
 * Initilaize LED settings
 */
void str8100_led_init(void)
{
    volatile unsigned int    ii;


    /*
     * Configure all GPIO pins as follows:
     * 1. output pins
     * 2. turn all leds off
     * 3. sequentially turn all leds on and all leds off     
     *    then, we can set GPIO Low to turn LED On, and GPIO High to turn LED Off 
     */
	printk("%s: \n",__FUNCTION__);

	GPIO_DIRECTION_REG |= LED_MASK;
    
    str8100_led_all_off();
    
    for (ii = 0; ii < 32; ii++)
    {
    	if(1<<ii&LED_MASK){
//printk("%s: ii=%d\n",__FUNCTION__,ii);
        	str8100_led_on(1 << ii);
        	msleep(LED_DELAY_MS);
        	str8100_led_off(1 << ii);
//        	msleep(LED_DELAY_MS);

        }
    }
}



