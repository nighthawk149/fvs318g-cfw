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

#ifndef _STAR_I2S_H_
#define _STAR_I2S_H_

/******************************************************************************
 * MODULE NAME:    star_i2s.h
 * PROJECT CODE:   Orion
 * DESCRIPTION:    
 * MAINTAINER:     MJLIU
 * DATE:           15 September 2005
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2005 Star Semi Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *     15 September 2005  -  MJLIU	- Initial Version v1.0
 *
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/

#include <asm/arch/star_sys_memory_map.h>

#define I2S_MEM_MAP_ADDR(reg_offset)          (SYSVA_I2S_BASE_ADDR + reg_offset) 
#define I2S_MEM_MAP_VALUE(reg_offset)         (*((unsigned int volatile *)I2S_MEM_MAP_ADDR(reg_offset)))

//#define I2S_BASE_ADDR                         (SYS_I2S_BASE_ADDR)


/*
 * define access macros
 */
#define I2S_CONFIGURATION_REG_ADDR            I2S_MEM_MAP_ADDR(0xC0)
#define I2S_RIGHT_TRANSMIT_DATA_REG_ADDR      I2S_MEM_MAP_ADDR(0xC4)
#define I2S_LEFT_TRANSMIT_DATA_REG_ADDR       I2S_MEM_MAP_ADDR(0xC8)
#define I2S_RIGHT_RECEIVE_DATA_REG_ADDR       I2S_MEM_MAP_ADDR(0xCC)
#define I2S_LEFT_RECEIVE_DATA_REG_ADDR        I2S_MEM_MAP_ADDR(0xD0)
#define I2S_INTERRUPT_STATUS_REG_ADDR         I2S_MEM_MAP_ADDR(0xD4)
#define I2S_INTERRUPT_ENABLE_REG_ADDR         I2S_MEM_MAP_ADDR(0xD8)

#define I2S_CONFIGURATION_REG                 I2S_MEM_MAP_VALUE(0xC0)
#define I2S_RIGHT_TRANSMIT_DATA_REG           I2S_MEM_MAP_VALUE(0xC4)
#define I2S_LEFT_TRANSMIT_DATA_REG            I2S_MEM_MAP_VALUE(0xC8)
#define I2S_RIGHT_RECEIVE_DATA_REG            I2S_MEM_MAP_VALUE(0xCC)
#define I2S_LEFT_RECEIVE_DATA_REG             I2S_MEM_MAP_VALUE(0xD0)
#define I2S_INTERRUPT_STATUS_REG              I2S_MEM_MAP_VALUE(0xD4)
#define I2S_INTERRUPT_ENABLE_REG              I2S_MEM_MAP_VALUE(0xD8)


/*
 * define constants macros
 */
#define I2S_DATA_16_BIT             (0)
#define I2S_DATA_32_BIT             (1)

#define I2S_RXBF_R_FULL_FLAG        (0x01)
#define I2S_TXBF_R_EMPTY_FLAG       (0x02)
#define I2S_RXBF_L_FULL_FLAG        (0x04)
#define I2S_TXBF_L_EMPTY_FLAG       (0x08)

#define I2S_RXBF_R_OR_FLAG          (0x10)
#define I2S_TXBF_R_UR_FLAG          (0x20)
#define I2S_RXBF_L_OR_FLAG          (0x40)
#define I2S_TXBF_L_UR_FLAG          (0x80)


#define I2S_MASTER_MODE             (1)
#define I2S_SLAVE_MODE              (0)

#define I2S_I2S_MODE                (1)
#define I2S_RJF_MODE                (2)
#define I2S_LJF_MODE                (3)

#define I2S_CLOCK_CONTINUOUS_MODE   (0)
#define I2S_CLOCK_256S_MODE         (1)


#define I2S_WS_RATE_32KHZ           (1)    /* 8.192 MHz */
#define I2S_WS_RATE_44_1KHZ         (2)    /* 11.2896 MHz */
#define I2S_WS_RATE_48KHZ           (3)    /* 12.288 MHz */


/*
 * define data structure
 */
#if 0
typedef struct _I2S_OBJECT_    I2S_OBJECT_T;

struct _I2S_OBJECT_
{
    u_int32          config;
    u_int32          interrupt_config;


    /* 
     * For interrupt setting
     */
    INTC_OBJECT_T    intc_obj;
};


/*
 * function declarations
 */
void    Hal_I2s_Initialize(I2S_OBJECT_T *);
#endif


/*
 * macro declarations
 */
#define HAL_I2S_ENABLE_I2S() \
{ \
    (I2S_CONFIGURATION_REG) |= ((u32)0x1 << 31); \
}

#define HAL_I2S_DISABLE_I2S() \
{ \
    (I2S_CONFIGURATION_REG) &= ~((u32)0x1 << 31); \
}

#define HAL_I2S_ENABLE_DATA_SWAP() \
{ \
    (I2S_CONFIGURATION_REG) |= (0x1 << 24); \
}

#define HAL_I2S_DISABLE_DATA_SWAP() \
{ \
    (I2S_CONFIGURATION_REG) &= ~(0x1 << 24); \
}

#define HAL_I2S_DISABLE_LEFT_CHANNEL_TRANSMIT_BUFFER_UNDERRUN_INTERRUPT() \
{ \
    (I2S_INTERRUPT_ENABLE_REG) &= ~(I2S_TXBF_L_UR_FLAG); \
}

#define HAL_I2S_DISABLE_RIGHT_CHANNEL_TRANSMIT_BUFFER_UNDERRUN_INTERRUPT() \
{ \
    (I2S_INTERRUPT_ENABLE_REG) &= ~(I2S_TXBF_R_UR_FLAG); \
}

#define HAL_I2S_DISABLE_LEFT_CHANNEL_RECEIVE_BUFFER_OVERRUN_INTERRUPT() \
{ \
    (I2S_INTERRUPT_ENABLE_REG) &= ~(I2S_RXBF_L_OR_FLAG); \
}

#define HAL_I2S_DISABLE_RIGHT_CHANNEL_RECEIVE_BUFFER_OVERRUN_INTERRUPT() \
{ \
    (I2S_INTERRUPT_ENABLE_REG) &= ~(I2S_RXBF_R_OR_FLAG); \
}


#endif  // end of #ifndef _STAR_I2S_H_
