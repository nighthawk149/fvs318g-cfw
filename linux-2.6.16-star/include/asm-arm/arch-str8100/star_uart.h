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


#define	UART_MEM_MAP_VALUE_PHY(reg_offset)	(*((u32	volatile *)(SYSPA_UART0_BASE_ADDR + reg_offset)))
#define	UART_MEM_MAP_VALUE_VIR(reg_offset)	(*((u32	volatile *)(SYSVA_UART0_BASE_ADDR + reg_offset)))


#define	UART1_OFFSET		0x800000  //SYS_UART1_BASE_ADDR	= 0x78800000 = (UART1_OFFSET+ SYS_UART0_BASE_ADDR)

#define	__UART_RBR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)
#define	__UART_THR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)
#define	__UART_DLL(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)

#define	__UART_IER(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x04)
#define	__UART_DLM(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x04)

#define	__UART_IIR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)
#define	__UART_FCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)
#define	__UART_PSR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)

#define	__UART_LCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x0C)
#define	__UART_MCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x10) //UART(n) Control Reg
#define	__UART_LSR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x14)
#define	__UART_SPR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x1C)

#if defined(__UBOOT__)
#define	_UART_RBR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)
#define	_UART_THR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)
#define	_UART_DLL(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x00)

#define	_UART_IER(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x04)
#define	_UART_DLM(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x04)

#define	_UART_IIR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)
#define	_UART_FCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)
#define	_UART_PSR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x08)

#define	_UART_LCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x0C)
#define	_UART_MCR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x10) //UART(n) Control Reg
#define	_UART_LSR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x14)
#define	_UART_SPR(idx)		UART_MEM_MAP_VALUE_PHY((UART1_OFFSET * idx) + 0x1C)
#elif defined(__LINUX__)
#define	_UART_RBR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x00)
#define	_UART_THR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x00)
#define	_UART_DLL(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x00)

#define	_UART_IER(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x04)
#define	_UART_DLM(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x04)

#define	_UART_IIR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x08)
#define	_UART_FCR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x08)
#define	_UART_PSR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x08)

#define	_UART_LCR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x0C)
#define	_UART_MCR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x10) //UART(n) Control Reg
#define	_UART_LSR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x14)
#define	_UART_SPR(idx)		UART_MEM_MAP_VALUE_VIR((UART1_OFFSET * idx) + 0x1C)
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define constants macros
 */
#define	UART_INPUT_CLOCK		(13000000)


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


#define	TX_FIFO_TRIGGER_LEVEL_1		(0 << 4)
#define	TX_FIFO_TRIGGER_LEVEL_3		(1 << 4)
#define	TX_FIFO_TRIGGER_LEVEL_9		(2 << 4)
#define	TX_FIFO_TRIGGER_LEVEL_13	(3 << 4)



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

/* MCR Register	*/
//#define UART_MCR_DTR			0x1		/* Data	Terminal Ready */
//#define UART_MCR_RTS			0x2		/* Request to Send */
//#define UART_MCR_OUT1			0x4		/* output1 */
//#define UART_MCR_OUT2			0x8		/* output2 or global interrupt enable */
#define	UART_MCR_LPBK			0x10		/* loopback mode */


/* LSR Register	*/
#define	_DATA_READY			(1 << 0)
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

#define	HAL_UART_READ_DATA(idx,data) \
{ \
    ((data) = (_UART_RBR(idx)) & 0xFF);	\
}


#define	HAL_UART_WRITE_DATA(idx,data) \
{ \
    ((_UART_THR(idx)) =	(data) & 0xFF);	\
}


#define	HAL_UART_ENABLE_INTERRUPT_TYPE(idx,interrupt_type) \
{ \
    ((_UART_IER(idx)) |= (interrupt_type & 0xF)); \
}


#define	HAL_UART_DISABLE_INTERRUPT_TYPE(idx,interrupt_type) \
{ \
    ((_UART_IER(idx)) &= ~(interrupt_type & 0xF)); \
}


#define	HAL_UART_READ_INTERRUPT_IDENTIFICATION(idx,uart_IIR) \
{ \
    ((uart_IIR)	= (_UART_IIR(idx))); \
}


#define	HAL_UART_CHECK_NO_INT_PENDING(idx,uart_IIR) \
{ \
    (((uart_IIR) & 0xF)	== (NO_INT_PENDING_MASK)); \
}


#define	HAL_UART_CHECK_RX_LINE_STATUS_INT(idx,uart_IIR)	\
    (((uart_IIR) & 0xF)	== (RX_LINE_STATUS_INT_MASK))


#define	HAL_UART_CHECK_RX_DATA_READY_INT(idx,uart_IIR) \
    (((uart_IIR) & 0xF)	== (RX_DATA_READY_INT_MASK))


#define	HAL_UART_CHECK_RX_DATA_TIMEOUT_INT(idx,uart_IIR) \
    (((uart_IIR) & 0xF)	== (RX_DATA_TIMEOUT_INT_MASK))


#define	HAL_UART_CHECK_THR_EMPTY_INT(idx,uart_IIR) \
    (((uart_IIR) & 0xF)	== (THR_EMPTY_INT_MASK))


#define	HAL_UART_FIFO_ENABLE(idx) \
{ \
    ((_UART_FCR(idx)) |= (FIFO_ENABLE)); \
}


#define	HAL_UART_FIFO_DISABLE(idx) \
{ \
    ((_UART_FCR(idx)) &= ~(FIFO_ENABLE)); \
}


#define	HAL_UART_RESET_RX_FIFO(idx) \
{ \
   ((_UART_FCR(idx)) |=	(RX_FIFO_RESET)); \
}


#define	HAL_UART_RESET_TX_FIFO(idx) \
{ \
    ((_UART_FCR(idx)) |= (TX_FIFO_RESET)); \
}


#define	HAL_UART_DLAB_ENABLE(idx) \
{ \
    ((_UART_LCR(idx)) |= (DLAB_ENABLE)); \
}


#define	HAL_UART_DLAB_DISABLE(idx) \
{ \
    ((_UART_LCR(idx)) &= ~(DLAB_ENABLE)); \
}


#define	HAL_UART_ENABLE_LOOPBACK_MODE(idx) \
{ \
    ((_UART_MCR(idx)) |= (UART_MCR_LPBK)); \
}


#define	HAL_UART_DISABLE_LOOPBACK_MODE(idx) \
{ \
    ((_UART_MCR(idx)) &= ~(UART_MCR_LPBK)); \
}


#define	HAL_UART_READ_LINE_STATUS(idx,uart_LSR)	\
{ \
    ((uart_LSR)	= (_UART_LSR(idx))); \
}


#define	HAL_UART_WRITE_DLL(idx,dll_value) \
{ \
    HAL_UART_DLAB_ENABLE(idx); \
    _UART_DLL(idx) = (u32)dll_value; \
    HAL_UART_DLAB_DISABLE(idx);	\
}


#define	HAL_UART_WRITE_DLM(idx,dlm_value) \
{ \
    HAL_UART_DLAB_ENABLE(idx); \
    _UART_DLM(idx) = (u32)dlm_value; \
    HAL_UART_DLAB_DISABLE(idx);	\
}


#define	HAL_UART_WRITE_PSR(idx,psr_value) \
{ \
    HAL_UART_DLAB_ENABLE(idx); \
    _UART_PSR(idx) = (u32)(psr_value & 0x3); \
    HAL_UART_DLAB_DISABLE(idx);	\
}


#define	HAL_UART_READ_PSR(idx,psr_value) \
{ \
    HAL_UART_DLAB_ENABLE(idx); \
    (psr_value)	= (u32)((_UART_PSR(idx)) & 0x3); \
    HAL_UART_DLAB_DISABLE(idx);	\
}


#define	HAL_UART_CHECK_RX_DATA_READY(idx) \
    (((_UART_LSR(idx)) & _DATA_READY) == (_DATA_READY))


#define	HAL_UART_CHECK_TX_FIFO_EMPTY(idx) \
    (((_UART_LSR(idx)) & THR_EMPTY) == (THR_EMPTY))


#define	HAL_UART_CHECK_TRANSMITTER_EMPTY(idx) \
    (((_UART_LSR(idx)) & TRANSMITTER_EMPTY) == (TRANSMITTER_EMPTY))


#endif	// end of #ifndef _STAR_UART_H_
