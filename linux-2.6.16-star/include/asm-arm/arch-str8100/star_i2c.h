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


#ifndef _STAR_I2C_H_
#define _STAR_I2C_H_

#include <asm/arch/star_sys_memory_map.h>

#define I2C_MEM_MAP_ADDR(reg_offset)          (SYSVA_I2C_BASE_ADDR + reg_offset) 
#define I2C_MEM_MAP_VALUE(reg_offset)         (*((unsigned int volatile *)I2C_MEM_MAP_ADDR(reg_offset)))

#define I2C_CONTROLLER_REG_ADDR               I2C_MEM_MAP_ADDR(0x20)
#define I2C_TIME_OUT_REG_ADDR                 I2C_MEM_MAP_ADDR(0x24)
#define I2C_SLAVE_ADDRESS_REG_ADDR            I2C_MEM_MAP_ADDR(0x28)
#define I2C_WRITE_DATA_REG_ADDR               I2C_MEM_MAP_ADDR(0x2C)
#define I2C_READ_DATA_REG_ADDR                I2C_MEM_MAP_ADDR(0x30)
#define I2C_INTERRUPT_STATUS_REG_ADDR         I2C_MEM_MAP_ADDR(0x34)
#define I2C_INTERRUPT_ENABLE_REG_ADDR         I2C_MEM_MAP_ADDR(0x38)

#define I2C_CONTROLLER_REG                    I2C_MEM_MAP_VALUE(0x20)
#define I2C_TIME_OUT_REG                      I2C_MEM_MAP_VALUE(0x24)
#define I2C_SLAVE_ADDRESS_REG                 I2C_MEM_MAP_VALUE(0x28)
#define I2C_WRITE_DATA_REG                    I2C_MEM_MAP_VALUE(0x2C)
#define I2C_READ_DATA_REG                     I2C_MEM_MAP_VALUE(0x30)
#define I2C_INTERRUPT_STATUS_REG              I2C_MEM_MAP_VALUE(0x34)
#define I2C_INTERRUPT_ENABLE_REG              I2C_MEM_MAP_VALUE(0x38)

#define I2C_READ_ONLY_CMD      (0)
#define I2C_WRITE_ONLY_CMD     (1)
#define I2C_WRITE_READ_CMD     (2)
#define I2C_READ_WRITE_CMD     (3)

#define I2C_DATA_LEN_1_BYTE    (0)
#define I2C_DATA_LEN_2_BYTE    (1)
#define I2C_DATA_LEN_3_BYTE    (2)
#define I2C_DATA_LEN_4_BYTE    (3)

#define I2C_BUS_ERROR_FLAG     (0x1)
#define I2C_ACTION_DONE_FLAG   (0x2)

#define HAL_I2C_ENABLE_I2C()          (I2C_CONTROLLER_REG) |= ((unsigned int)0x1 << 31); 
#define HAL_I2C_DISABLE_I2C()         (I2C_CONTROLLER_REG) &= ~((unsigned int)0x1 << 31);
#define HAL_I2C_ENABLE_DATA_SWAP()    (I2C_CONTROLLER_REG) |= (0x1 << 24); 
#define HAL_I2C_DISABLE_DATA_SWAP()   (I2C_CONTROLLER_REG) &= ~(0x1 << 24); 
#define HAL_I2C_START_TRANSFER()      (I2C_CONTROLLER_REG) |= (0x1 << 6); 
#define HAL_I2C_STOP_TRANSFER()       (I2C_CONTROLLER_REG) &= ~(0x1 << 6); 

#endif

