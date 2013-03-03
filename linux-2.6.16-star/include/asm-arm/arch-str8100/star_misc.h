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


#ifndef	_STAR_MISC_H_
#define	_STAR_MISC_H_


#include <asm/arch/star_sys_memory_map.h>


#if defined(__UBOOT__)
#define	MISC_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSPA_MISC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	MISC_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(SYSVA_MISC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


/*
 * define access macros
 */
#define	MISC_MEMORY_REMAP_REG				MISC_MEM_MAP_VALUE(0x00)
#define	MISC_CHIP_CONFIG_REG				MISC_MEM_MAP_VALUE(0x04)
#define	MISC_DEBUG_PROBE_DATA_REG			MISC_MEM_MAP_VALUE(0x08)
#define	MISC_DEBUG_PROBE_SELECTION_REG			MISC_MEM_MAP_VALUE(0x0C)
#define	MISC_PCI_CONTROL_BROKEN_MASK_REG		MISC_MEM_MAP_VALUE(0x10)
#define	MISC_PCI_BROKEN_STATUS_REG			MISC_MEM_MAP_VALUE(0x14)
#define	MISC_PCI_DEVICE_VENDOR_ID_REG			MISC_MEM_MAP_VALUE(0x18)
#define	MISC_USB_HOST_PHY_CONTROL_TEST_REG		MISC_MEM_MAP_VALUE(0x1C)
#define	MISC_GPIOA_PIN_ENABLE_REG			MISC_MEM_MAP_VALUE(0x20)
#define	MISC_GPIOB_PIN_ENABLE_REG			MISC_MEM_MAP_VALUE(0x24)
#define	MISC_GPIOA_RESISTOR_CONFIG_REG			MISC_MEM_MAP_VALUE(0x28)
#define	MISC_GPIOA_DRIVE_STRENGTH_CONFIG_REG		MISC_MEM_MAP_VALUE(0x2C)
#define	MISC_FAST_ETHERNET_PHY_CONFIG_REG		MISC_MEM_MAP_VALUE(0x30)
#define	MISC_SOFTWARE_TEST_1_REG			MISC_MEM_MAP_VALUE(0x38)
#define	MISC_SOFTWARE_TEST_2_REG			MISC_MEM_MAP_VALUE(0x3C)

#define	MISC_E_FUSE_0_REG				MISC_MEM_MAP_VALUE(0x60)
#define	MISC_E_FUSE_1_REG				MISC_MEM_MAP_VALUE(0x64)


/*
 * define constants macros
 */
#define	MISC_PARALLEL_FLASH_BOOT		(0)
#define	MISC_SPI_SERIAL_FLASH_BOOT		(1)

#define	MISC_LITTLE_ENDIAN			(0)
#define	MISC_BIG_ENDIAN				(1)

#define	MISC_FARADAY_ICE			(0)
#define	MISC_ARM_ICE				(1)

#define	MISC_EXT_INT29_PINS			((0x1 << 0))
#define	MISC_EXT_INT30_PINS			((0x1 << 1))
#define	MISC_EXT_INT31_PINS			((0x1 << 2))
#define	MISC_I2C_PINS				((0x1 << 13) | (0x1 << 14))
#define	MISC_I2S_PINS				((0x1 << 15) | (0x1 << 16) | (0x1 << 17))
#define	MISC_PCM_PINS				((0x1 << 18) | (0x1 << 19) | (0x1 << 20) | (0x1 << 21))
#define	MISC_LED0_PINS				((0x1 << 22))
#define	MISC_LED1_PINS				((0x1 << 23))
#define	MISC_LED2_PINS				((0x1 << 24))
#define	MISC_LED012_PINS			((0x1 << 22) | (0x1 << 23) | (0x1 << 24))
#define	MISC_WDTIMER_RESET_PINS			((0x1 << 25))
#define	MISC_SPI_PINS				((0x1 << 26) | (0x1 << 27) | (0x1 << 28) | (0x1 << 29) | (0x1 << 30) | (0x1 << 31))
#define	MISC_MDC_MDIO_PINS			((0x1 << 0) | (0x1 << 1))
#define	MISC_NIC_COL_PINS			((0x1 << 2))
#define	MISC_IDE_PINS				((0xFF << 3))
#define	MISC_SRAM_BANK1_PINS			((0x1 << 11) | (0x1 << 14))
#define	MISC_SRAM_BANK2_PINS			((0x1 << 12) | (0x1 << 15))
#define	MISC_SRAM_BANK3_PINS			((0x1 << 13) | (0x1 << 16))
#define	MISC_PCMCIA_PINS			((0x1 << 17) | (0x1 << 18) | (0x1 << 19) | (0x1 << 20))
#define	MISC_UART1_PINS				((0x1 << 21) | (0x1 << 22))
#define	MISC_PCI_PINS				(((u32)0x1FF << 23))

#define	MISC_UART0_ACT0_Pin			(0x1 << 2)
#define	MISC_UART1_ACT1_Pin			(0x1 << 3)

#define	MISC_GPIOA_PIN_0			(0)
#define	MISC_GPIOA_PIN_1			(1)
#define	MISC_GPIOA_PIN_2			(2)
#define	MISC_GPIOA_PIN_3			(3)
#define	MISC_GPIOA_PIN_4			(4)
#define	MISC_GPIOA_PIN_5			(5)
#define	MISC_GPIOA_PIN_6			(6)
#define	MISC_GPIOA_PIN_7			(7)
#define	MISC_GPIOA_PIN_8			(8)
#define	MISC_GPIOA_PIN_9			(9)
#define	MISC_GPIOA_PIN_10			(10)

#define	MISC_GPIOA_75K_RESISTOR_PULL_DOWN	(1)
#define	MISC_GPIOA_75K_RESISTOR_PULL_UP		(2)
#define	MISC_GPIOA_75K_RESISTOR_PULL_KEEPER	(3)

#define	MISC_GPIOA_DRIVE_STRENGTH_4MA		(0)
#define	MISC_GPIOA_DRIVE_STRENGTH_8MA		(1)


/*
 * macro declarations
 */
#define	HAL_MISC_ENABLE_SPI_SERIAL_FLASH_BANK_ACCESS() \
{ \
    (MISC_CHIP_CONFIG_REG) |= (0x1 << 4); \
}

#define	HAL_MISC_DISABLE_SPI_SERIAL_FLASH_BANK_ACCESS()	\
{ \
    (MISC_CHIP_CONFIG_REG) &= ~(0x1 << 4); \
}


/*
 * Macro defines for GPIOA and GPIOB Pin Enable	Register
 */
#define	HAL_MISC_ENABLE_EXT_INT29_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_EXT_INT29_PINS); \
}

#define	HAL_MISC_DISABLE_EXT_INT29_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_EXT_INT29_PINS); \
}

#define	HAL_MISC_ENABLE_EXT_INT30_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_EXT_INT30_PINS); \
}

#define	HAL_MISC_DISABLE_EXT_INT30_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_EXT_INT30_PINS); \
}

#define	HAL_MISC_ENABLE_I2C_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_I2C_PINS); \
}

#define	HAL_MISC_DISABLE_I2C_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_I2C_PINS); \
}

#define	HAL_MISC_ENABLE_I2S_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_I2S_PINS); \
}

#define	HAL_MISC_DISABLE_I2S_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_I2S_PINS); \
}

#define	HAL_MISC_ENABLE_PCM_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_PCM_PINS); \
}

#define	HAL_MISC_DISABLE_PCM_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_PCM_PINS); \
}

#define	HAL_MISC_ENABLE_LED0_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_LED0_PINS); \
}

#define	HAL_MISC_DISABLE_LED0_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_LED0_PINS); \
}

#define	HAL_MISC_ENABLE_LED1_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_LED1_PINS); \
}

#define	HAL_MISC_DISABLE_LED1_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_LED1_PINS); \
}

#define	HAL_MISC_ENABLE_LED2_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_LED2_PINS); \
}

#define	HAL_MISC_DISABLE_LED2_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_LED2_PINS); \
}

#define	HAL_MISC_ENABLE_LED012_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_LED012_PINS); \
}

#define	HAL_MISC_DISABLE_LED012_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_LED012_PINS);	\
}

#define	HAL_MISC_ENABLE_WDTIMER_RESET_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_WDTIMER_RESET_PINS); \
}

#define	HAL_MISC_DISABLE_WDTIMER_RESET_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_WDTIMER_RESET_PINS); \
}

#define	HAL_MISC_ENABLE_SPI_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_SPI_PINS); \
}

#define	HAL_MISC_DISABLE_SPI_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_SPI_PINS); \
}

#define	HAL_MISC_ENABLE_UART0_ACT0_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_UART0_ACT0_Pin); \
}

#define	HAL_MISC_DISABLE_UART0_ACT0_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_UART0_ACT0_Pin); \
}

#define	HAL_MISC_ENABLE_UART1_ACT1_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	|= (MISC_UART1_ACT1_Pin); \
}

#define	HAL_MISC_DISABLE_UART1_ACT1_PIN() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	&= ~(MISC_UART1_ACT1_Pin); \
}

#define	HAL_MISC_ENABLE_MDC_MDIO_PINS()	\
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_MDC_MDIO_PINS); \
}

#define	HAL_MISC_DISABLE_MDC_MDIO_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_MDC_MDIO_PINS); \
}

#define	HAL_MISC_ENABLE_NIC_COL_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_NIC_COL_PINS);	\
}

#define	HAL_MISC_DISABLE_NIC_COL_PINS()	\
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_NIC_COL_PINS); \
}

#define	HAL_MISC_ENABLE_IDE_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_IDE_PINS); \
}

#define	HAL_MISC_DISABLE_IDE_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_IDE_PINS); \
}

#define	HAL_MISC_ENABLE_SRAM_BANK1_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_SRAM_BANK1_PINS); \
}

#define	HAL_MISC_DISABLE_SRAM_BANK1_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_SRAM_BANK1_PINS); \
}

#define	HAL_MISC_ENABLE_SRAM_BANK2_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_SRAM_BANK2_PINS); \
}

#define	HAL_MISC_DISABLE_SRAM_BANK2_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_SRAM_BANK2_PINS); \
}

#define	HAL_MISC_ENABLE_SRAM_BANK3_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_SRAM_BANK3_PINS); \
}

#define	HAL_MISC_DISABLE_SRAM_BANK3_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_SRAM_BANK3_PINS); \
}

#define	HAL_MISC_ENABLE_PCMCIA_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_PCMCIA_PINS); \
}

#define	HAL_MISC_DISABLE_PCMCIA_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_PCMCIA_PINS);	\
}

#define	HAL_MISC_ENABLE_UART1_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_UART1_PINS); \
}

#define	HAL_MISC_DISABLE_UART1_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_UART1_PINS); \
}

#define	HAL_MISC_ENABLE_PCI_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	|= (MISC_PCI_PINS); \
}

#define	HAL_MISC_DISABLE_PCI_PINS() \
{ \
    (MISC_GPIOB_PIN_ENABLE_REG)	&= ~(MISC_PCI_PINS); \
}

#define	HAL_MISC_ENABLE_ALL_SHARED_GPIO_PINS() \
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	= (0x0); \
    (MISC_GPIOB_PIN_ENABLE_REG)	= (0x0); \
}

#define	HAL_MISC_DISABLE_ALL_SHARED_GPIO_PINS()	\
{ \
    (MISC_GPIOA_PIN_ENABLE_REG)	= (0xFFFFFFFF);	\
    (MISC_GPIOB_PIN_ENABLE_REG)	= (0xFFFFFFFF);	\
}

#define	HAL_MISC_CONFIGURE_GPIOA_RESISTOR(pin_index, value) \
{ \
    (MISC_GPIOA_RESISTOR_CONFIG_REG) &=	~(0x3 << (2 * pin_index)); \
    (MISC_GPIOA_RESISTOR_CONFIG_REG) |=	((value	& 0x3) << (2 * pin_index)); \
}

#define	HAL_MISC_CONFIGURE_GPIOA_DRIVE_STRENGTH(pin_index, value) \
{ \
    (MISC_GPIOA_DRIVE_STRENGTH_CONFIG_REG) &= ~(0x1 << pin_index); \
    (MISC_GPIOA_DRIVE_STRENGTH_CONFIG_REG) |= (value <<	pin_index); \
}

#define	HAL_MISC_SELECT_FAST_ETHERNET_PHY_LED_MODE0() \
{ \
    (MISC_FAST_ETHERNET_PHY_CONFIG_REG)	= (0x0); \
}

#define	HAL_MISC_SELECT_FAST_ETHERNET_PHY_LED_MODE1() \
{ \
    (MISC_FAST_ETHERNET_PHY_CONFIG_REG)	= (0x1); \
}

#define	HAL_MISC_SELECT_FAST_ETHERNET_PHY_LED_MODE2() \
{ \
    (MISC_FAST_ETHERNET_PHY_CONFIG_REG)	= (0x2); \
}

#define	HAL_MISC_SELECT_FAST_ETHERNET_PHY_LED_MODE3() \
{ \
    (MISC_FAST_ETHERNET_PHY_CONFIG_REG)	= (0x3); \
}


#endif	// end of #ifndef _STAR_MISC_H_
