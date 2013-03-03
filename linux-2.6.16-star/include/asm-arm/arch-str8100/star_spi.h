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


#ifndef _STAR_SPI_H_
#define _STAR_SPI_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_SPI_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_SPI_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define SPI_CONFIGURATION_REG			SPI_MEM_MAP_VALUE(0x40)
#define SPI_SERVICE_STATUS_REG			SPI_MEM_MAP_VALUE(0x44)
#define SPI_BIT_RATE_CONTROL_REG		SPI_MEM_MAP_VALUE(0x48)
#define SPI_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x4C)
#define SPI_TRANSMIT_BUFFER_REG			SPI_MEM_MAP_VALUE(0x50)
#define SPI_RECEIVE_CONTROL_REG			SPI_MEM_MAP_VALUE(0x54)
#define SPI_RECEIVE_BUFFER_REG			SPI_MEM_MAP_VALUE(0x58)
#define SPI_FIFO_TRANSMIT_CONFIG_REG		SPI_MEM_MAP_VALUE(0x5C)
#define SPI_FIFO_TRANSMIT_CONTROL_REG		SPI_MEM_MAP_VALUE(0x60)
#define SPI_FIFO_RECEIVE_CONFIG_REG		SPI_MEM_MAP_VALUE(0x64)
#define SPI_INTERRUPT_STATUS_REG		SPI_MEM_MAP_VALUE(0x68)
#define SPI_INTERRUPT_ENABLE_REG		SPI_MEM_MAP_VALUE(0x6C)


/*
 * define constants macros
 */
#define SPI_TX_RX_FIFO_DEPTH			(8)

#define SPI_CH0					(0)
#define SPI_CH1					(1)
#define SPI_CH2					(2)
#define SPI_CH3					(3)


#define SPI_RXFIFO_OT_FG			(0x01)
#define SPI_TXFIFO_UT_FG			(0x02)
#define SPI_RXBUF_FULL_FG			(0x04)
#define SPI_TXBUF_EMPTY_FG			(0x08)

#define SPI_RXFIFO_OR_FG			(0x10)
#define SPI_TXFIFO_UR_FG			(0x20)
#define SPI_RXBUF_OR_FG				(0x40)
#define SPI_TXBUF_UR_FG				(0x80)

/*
 * define Character Length Control
 */
#define SPI_LEN_BIT_8				(0)
#define SPI_LEN_BIT_16				(1)
#define SPI_LEN_BIT_24				(2)
#define SPI_LEN_BIT_32				(3)


/*
 * macro declarations
 */
#define HAL_SPI_ENABLE_SPI() \
{ \
    (SPI_CONFIGURATION_REG) |= ((u_int32)0x1 << 31); \
}

#define HAL_SPI_DISABLE_SPI() \
{ \
    (SPI_CONFIGURATION_REG) &= ~((u_int32)0x1 << 31); \
}

#define HAL_SPI_ENABLE_DATA_SWAP() \
{ \
    (SPI_CONFIGURATION_REG) |= (0x1 << 24); \
}

#define HAL_SPI_DISABLE_DATA_SWAP() \
{ \
    (SPI_CONFIGURATION_REG) &= ~(0x1 << 24); \
}

#define HAL_SPI_TRANSMIT_DATA(tx_data) \
{ \
    (SPI_TRANSMIT_BUFFER_REG) = tx_data; \
}

#define HAL_SPI_RECEIVE_DATA(rx_data) \
{ \
    (rx_data) = SPI_RECEIVE_BUFFER_REG; \
}

#define HAL_SPI_GET_TRANSMIT_FIFO_WORDS_NUMBER(tx_fifo_words_num) \
{ \
    (tx_fifo_words_num) = SPI_FIFO_TRANSMIT_CONFIG_REG & 0xF; \
}

#define HAL_SPI_GET_RECEIVE_FIFO_WORDS_NUMBER(rx_fifo_words_num) \
{ \
    (rx_fifo_words_num) = SPI_FIFO_RECEIVE_CONFIG_REG & 0xF; \
}

#define HAL_SPI_DISABLE_ALL_INTERRUPT_SOURCES() \
{ \
    (SPI_INTERRUPT_ENABLE_REG) = 0; \
}

#define HAL_SPI_DISABLE_TX_FIFO_THRESHOLD_INTERRUPT() \
{ \
    (SPI_INTERRUPT_ENABLE_REG) &= ~(0x1 << 1); \
}

#define HAL_SPI_DISABLE_RX_FIFO_THRESHOLD_INTERRUPT() \
{ \
    (SPI_INTERRUPT_ENABLE_REG) &= ~(0x1 << 0); \
}

#define HAL_SPI_READ_INTERRUPT_STATUS(status) \
{ \
    (status) = SPI_INTERRUPT_STATUS_REG; \
}

#define HAL_SPI_CLEAR_INTERRUPT_STATUS(status) \
{ \
    (SPI_INTERRUPT_STATUS_REG) = (status & 0xF0); \
}

#define HAL_SPI_SET_FIFO_TRANSMIT_DELAY(delay) \
{ \
    (SPI_FIFO_TRANSMIT_CONTROL_REG) = (delay & 0x1F); \
}

#define STR8100_SPI_SERIAL_MODE_GENERAL              0x0
#define STR8100_SPI_SERIAL_MODE_MICROPROCESSOR       0x1
 
struct str8100_spi_dev_attr
{ 
	int spi_serial_mode;
};

#endif // end of #ifndef _STAR_SPI_H_

