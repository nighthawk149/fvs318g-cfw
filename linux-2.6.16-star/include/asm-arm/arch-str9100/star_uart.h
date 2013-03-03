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


#ifndef	_STAR_UART_H_
#define	_STAR_UART_H_


#include <asm/arch/star_sys_memory_map.h>


#define	UART_MEM_MAP_VALUE_PHY(reg_offset)	(*((u32 volatile *)(SYSPA_UART_BASE_ADDR + reg_offset)))
#define UART_MEM_MAP_VALUE_VIR(reg_offset)	(*((u32 volatile *)(SYSVA_UART_BASE_ADDR + reg_offset)))


#define	__UART_RBR		UART_MEM_MAP_VALUE_PHY(0x00)
#define	__UART_THR		UART_MEM_MAP_VALUE_PHY(0x00)
#define	__UART_DLL		UART_MEM_MAP_VALUE_PHY(0x00)

#define	__UART_IER		UART_MEM_MAP_VALUE_PHY(0x04)
#define	__UART_DLM		UART_MEM_MAP_VALUE_PHY(0x04)

#define	__UART_IIR		UART_MEM_MAP_VALUE_PHY(0x08)
#define	__UART_FCR		UART_MEM_MAP_VALUE_PHY(0x08)
#define	__UART_PSR		UART_MEM_MAP_VALUE_PHY(0x08)

#define	__UART_LCR		UART_MEM_MAP_VALUE_PHY(0x0C)

#define	__UART_MCR		UART_MEM_MAP_VALUE_PHY(0x10)

#define	__UART_LSR		UART_MEM_MAP_VALUE_PHY(0x14)
#define	__UART_TST		UART_MEM_MAP_VALUE_PHY(0x14)

#define	__UART_MSR		UART_MEM_MAP_VALUE_PHY(0x18)

#define	__UART_SPR		UART_MEM_MAP_VALUE_PHY(0x1C)

#define	__UART_FEATURE		UART_MEM_MAP_VALUE_PHY(0x68)

#define	__UART_REVD1		UART_MEM_MAP_VALUE_PHY(0x6C)
#define	__UART_REVD2		UART_MEM_MAP_VALUE_PHY(0x70)
#define	__UART_REVD3		UART_MEM_MAP_VALUE_PHY(0x74)

#if defined(__UBOOT__)
#define	_UART_RBR		UART_MEM_MAP_VALUE_PHY(0x00)
#define	_UART_THR		UART_MEM_MAP_VALUE_PHY(0x00)
#define	_UART_DLL		UART_MEM_MAP_VALUE_PHY(0x00)

#define	_UART_IER		UART_MEM_MAP_VALUE_PHY(0x04)
#define	_UART_DLM		UART_MEM_MAP_VALUE_PHY(0x04)

#define	_UART_IIR		UART_MEM_MAP_VALUE_PHY(0x08)
#define	_UART_FCR		UART_MEM_MAP_VALUE_PHY(0x08)
#define	_UART_PSR		UART_MEM_MAP_VALUE_PHY(0x08)

#define	_UART_LCR		UART_MEM_MAP_VALUE_PHY(0x0C)

#define	_UART_MCR		UART_MEM_MAP_VALUE_PHY(0x10)

#define	_UART_LSR		UART_MEM_MAP_VALUE_PHY(0x14)
#define	_UART_TST		UART_MEM_MAP_VALUE_PHY(0x14)

#define	_UART_MSR		UART_MEM_MAP_VALUE_PHY(0x18)

#define	_UART_SPR		UART_MEM_MAP_VALUE_PHY(0x1C)

#define	_UART_FEATURE		UART_MEM_MAP_VALUE_PHY(0x68)

#define	_UART_REVD1		UART_MEM_MAP_VALUE_PHY(0x6C)
#define	_UART_REVD2		UART_MEM_MAP_VALUE_PHY(0x70)
#define	_UART_REVD3		UART_MEM_MAP_VALUE_PHY(0x74)
#elif defined(__LINUX__)
#define	_UART_RBR		UART_MEM_MAP_VALUE_VIR(0x00)
#define	_UART_THR		UART_MEM_MAP_VALUE_VIR(0x00)
#define	_UART_DLL		UART_MEM_MAP_VALUE_VIR(0x00)

#define	_UART_IER		UART_MEM_MAP_VALUE_VIR(0x04)
#define	_UART_DLM		UART_MEM_MAP_VALUE_VIR(0x04)

#define	_UART_IIR		UART_MEM_MAP_VALUE_VIR(0x08)
#define	_UART_FCR		UART_MEM_MAP_VALUE_VIR(0x08)
#define	_UART_PSR		UART_MEM_MAP_VALUE_VIR(0x08)

#define	_UART_LCR		UART_MEM_MAP_VALUE_VIR(0x0C)

#define	_UART_MCR		UART_MEM_MAP_VALUE_VIR(0x10)

#define	_UART_LSR		UART_MEM_MAP_VALUE_VIR(0x14)
#define	_UART_TST		UART_MEM_MAP_VALUE_VIR(0x14)

#define	_UART_MSR		UART_MEM_MAP_VALUE_VIR(0x18)

#define	_UART_SPR		UART_MEM_MAP_VALUE_VIR(0x1C)

#define	_UART_FEATURE		UART_MEM_MAP_VALUE_VIR(0x68)

#define	_UART_REVD1		UART_MEM_MAP_VALUE_VIR(0x6C)
#define	_UART_REVD2		UART_MEM_MAP_VALUE_VIR(0x70)
#define	_UART_REVD3		UART_MEM_MAP_VALUE_VIR(0x74)
#else
#error "NO SYSTEM DEFINED"
#endif

/*
 * define constants macros
 */
#define	UART_INPUT_CLOCK		(48000000)

#define	UART_FIFO_DEPTH			16

#define	RX_DATA_READY_INT		(1 << 0)
#define	THR_EMPTY_INT			(1 << 1)
#define	RX_LINE_STATUS_INT		(1 << 2)
#define	MODEM_STATUS_INT		(1 << 3)



#define	NO_INT_PENDING_MASK		(0x1)
#define	RX_LINE_STATUS_INT_MASK		(0x6)
#define	RX_DATA_READY_INT_MASK		(0x4)
#define	RX_DATA_TIMEOUT_INT_MASK	(0xC)
#define	THR_EMPTY_INT_MASK		(0x2)
#define	MODEM_STATUS_CHANGE_MASK	(0x0)


/* FCR Register	*/
#define	FIFO_ENABLE			(1 << 0)
#define	RX_FIFO_RESET			(1 << 1)
#define	TX_FIFO_RESET			(1 << 2)
#define	DMA_MODE			(1 << 3)


#define	RX_FIFO_TRIGGER_LEVEL_1		(0 << 6)
#define	RX_FIFO_TRIGGER_LEVEL_4		(1 << 6)
#define	RX_FIFO_TRIGGER_LEVEL_8		(2 << 6)
#define	RX_FIFO_TRIGGER_LEVEL_14	(3 << 6)

/* LCR Register	*/
#define	WORD_LENGTH_5			(0 << 0)
#define	WORD_LENGTH_6			(1 << 0)
#define	WORD_LENGTH_7			(2 << 0)
#define	WORD_LENGTH_8			(3 << 0)

#define	STOP_BIT_1			(0 << 2)
#define	STOP_BIT_1_5			(1 << 2)
#define	STOP_BIT_2			(1 << 2)

#define	PARITY_CHECK_NONE		(0 << 3)
#define	PARITY_CHECK_EVEN		(3 << 3)
#define	PARITY_CHECK_ODD		(1 << 3)
#define	PARITY_CHECK_STICK_ONE		(5 << 3)
#define	PARITY_CHECK_STICK_ZERO		(7 << 3)

#define	SET_BREAK			(1 << 6)

#define	DLAB_ENABLE			(1 << 7)


/* LSR Register	*/
#define	UART_DATA_READY			(1 << 0)
#define	OVERRUN_ERROR			(1 << 1)
#define	PARITY_ERROR			(1 << 2)
#define	FRAMING_ERROR			(1 << 3)
#define	BREAK_INTERRUPT			(1 << 4)
#define	THR_EMPTY			(1 << 5)
#define	TRANSMITTER_EMPTY		(1 << 6)
#define	FIFO_DATA_ERROR			(1 << 7)

#define	TEST_PARITY_ERROR		(1 << 0)
#define	TEST_FRAMING_ERROR		(1 << 1)
#define	TEST_BAUD_GEN			(1 << 2)
#define	TEST_LOOPBACK_ENABLE		(1 << 3)


#define	WORD_FIVE_BITS			5
#define	WORD_SIX_BITS			6
#define	WORD_SEVEN_BITS			7
#define	WORD_EIGHT_BITS			8

#define	NONE_PARITY			1
#define	EVEN_PARITY			2
#define	ODD_PARITY			3
#define	ONE_PARITY			4
#define	ZERO_PARITY			5

#define	ONE_STOP_BIT			1
#define	ONE_HALF_STOP_BIT		2
#define	TWO_STOP_BIT			3

#define	TX_RX_FIFO_DISABLE		0
#define	TX_RX_FIFO_ENABLE		1


/*
 * macros declarations
 */
#define HAL_UART_READ_DATA(data) \
{ \
    ((data) = (_UART_RBR) & 0xFF); \
}


#define HAL_UART_WRITE_DATA(data) \
{ \
    ((_UART_THR) = (data) & 0xFF); \
}


#define HAL_UART_ENABLE_INTERRUPT_TYPE(interrupt_type) \
{ \
    ((_UART_IER) |= (interrupt_type & 0xF)); \
}

    
#define HAL_UART_DISABLE_INTERRUPT_TYPE(interrupt_type) \
{ \
    ((_UART_IER) &= ~(interrupt_type & 0xF)); \
}


#define HAL_UART_READ_INTERRUPT_IDENTIFICATION(uart_IIR) \
{ \
    ((uart_IIR) = (_UART_IIR)); \
}


#define HAL_UART_CHECK_NO_INT_PENDING(uart_IIR) \
{ \
    (((uart_IIR) & 0xF) == (NO_INT_PENDING_MASK)); \
}

    
#define HAL_UART_CHECK_RX_LINE_STATUS_INT(uart_IIR) \
    (((uart_IIR) & 0xF) == (RX_LINE_STATUS_INT_MASK))

    
#define HAL_UART_CHECK_RX_DATA_READY_INT(uart_IIR) \
    (((uart_IIR) & 0xF) == (RX_DATA_READY_INT_MASK))

  
#define HAL_UART_CHECK_RX_DATA_TIMEOUT_INT(uart_IIR) \
    (((uart_IIR) & 0xF) == (RX_DATA_TIMEOUT_INT_MASK))


#define HAL_UART_CHECK_THR_EMPTY_INT(uart_IIR) \
    (((uart_IIR) & 0xF) == (THR_EMPTY_INT_MASK))


#define HAL_UART_CHECK_MODEM_STATUS_CHANGE_INT(uart_IIR) \
    (((uart_IIR) & 0xF) == (MODEM_STATUS_CHANGE_MASK))


#define HAL_UART_FIFO_ENABLE() \
{ \
    ((_UART_FCR) |= (FIFO_ENABLE)); \
}


#define HAL_UART_FIFO_DISABLE() \
{ \
    ((_UART_FCR) &= ~(FIFO_ENABLE)); \
}


#define HAL_UART_RESET_RX_FIFO() \
{ \
   ((_UART_FCR) |= (RX_FIFO_RESET)); \
}


#define HAL_UART_RESET_TX_FIFO() \
{ \
    ((_UART_FCR) |= (TX_FIFO_RESET)); \
}


#define HAL_UART_DLAB_ENABLE() \
{ \
    ((_UART_LCR) |= (DLAB_ENABLE)); \
}


#define HAL_UART_DLAB_DISABLE() \
{ \
    ((_UART_LCR) &= ~(DLAB_ENABLE)); \
}


#define HAL_UART_READ_LINE_STATUS(uart_LSR) \
{ \
    ((uart_LSR) = (_UART_LSR)); \
}


#define HAL_UART_READ_MODEM_STATUS(uart_MSR) \
{ \
    ((uart_MSR) = (_UART_MSR)); \
}


#define HAL_UART_WRITE_DLL(dll_value) \
{ \
    HAL_UART_DLAB_ENABLE(); \
    _UART_DLL = (u32)dll_value; \
    HAL_UART_DLAB_DISABLE(); \
}


#define HAL_UART_WRITE_DLM(dlm_value) \
{ \
    HAL_UART_DLAB_ENABLE(); \
    _UART_DLM = (u32)dlm_value; \
    HAL_UART_DLAB_DISABLE(); \
}


#define HAL_UART_WRITE_PSR(psr_value) \
{ \
    HAL_UART_DLAB_ENABLE(); \
    _UART_PSR = (u32)(psr_value & 0x3); \
    HAL_UART_DLAB_DISABLE(); \
}


#define HAL_UART_READ_PSR(psr_value) \
{ \
    HAL_UART_DLAB_ENABLE(); \
    (psr_value) = (u32)((_UART_PSR) & 0x3); \
    HAL_UART_DLAB_DISABLE(); \
}


#define HAL_UART_CHECK_RX_UART_DATA_READY() \
    (((_UART_LSR) & UART_DATA_READY) == (UART_DATA_READY))

    
#define HAL_UART_CHECK_TX_FIFO_EMPTY() \
    (((_UART_LSR) & THR_EMPTY) == (THR_EMPTY))


#define HAL_UART_CHECK_TRANSMITTER_EMPTY() \
    (((_UART_LSR) & TRANSMITTER_EMPTY) == (TRANSMITTER_EMPTY))    


#endif	// end of #ifndef _STAR_UART_H_

