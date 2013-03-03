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
#define	MISC_MEM_MAP_VALUE(reg_offset)			(*((u32 volatile *)(SYSPA_MISC_BASE_ADDR + reg_offset)))
#elif defined(__LINUX__)
#define	MISC_MEM_MAP_VALUE(reg_offset)			(*((u32 volatile *)(SYSVA_MISC_BASE_ADDR + reg_offset)))
#else
#error "NO SYSTEM DEFINED"
#endif


#define	MISC_MEMORY_REMAP_REG				MISC_MEM_MAP_VALUE(0x00)
#define	MISC_USB_CONTROL_REG				MISC_MEM_MAP_VALUE(0x04)
#define	MISC_PCI_DEVICE_ID_VENDOR_ID_CONFIG_REG		MISC_MEM_MAP_VALUE(0x0C)
#define	MISC_PCI_CLASS_CODE_REVISION_ID_CONFIG_REG	MISC_MEM_MAP_VALUE(0x10)
#define	MISC_PCI_SUBSYSTEM_ID_SUBVENDOR_ID_CONFIG_REG	MISC_MEM_MAP_VALUE(0x14)
#define	MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG	MISC_MEM_MAP_VALUE(0x18)
#define	MISC_PCI_ARBITER_INTERRUPT_MASK_REG		MISC_MEM_MAP_VALUE(0x1C)
#define	MISC_PCI_BROKEN_STATUS_REG			MISC_MEM_MAP_VALUE(0x20)
#define	MISC_DEBUG_SIGNAL_SELECT_REG			MISC_MEM_MAP_VALUE(0x24)
#define	MISC_EARLY_TERMINATION_CONTROL_REG		MISC_MEM_MAP_VALUE(0x34)


/*
 * define constants macros
 */
#define	HAL_MISC_ENABLE_MEMORY_REMAP() \
    ((MISC_MEMORY_REMAP) = (1))


#define	HAL_MISC_READ_USB20_PHY_CONTROL(usb20_phy_control) \
    ((usb20_phy_control) = ((MISC_USB_CONTROL) & 0x0003FFFF))


#define	HAL_MISC_WRITE_USB20_PHY_CONTROL(usb20_phy_control) \
{ \
    (MISC_USB_CONTROL) &= (0xFFFC0000);	\
    (MISC_USB_CONTROL) |= (usb20_phy_control); \
}


#define	HAL_MISC_READ_PCI_BRIDGE_DEVICE_ID_VENDOR_ID(device_id_vendor_id) \
    ((device_id_vendor_id) = (MISC_PCI_DEVICE_ID_VENDOR_ID_CONFIG_REG))


#define	HAL_MISC_WRITE_PCI_BRIDGE_DEVICE_ID_VENDOR_ID(device_id_vendor_id) \
    ((MISC_PCI_DEVICE_ID_VENDOR_ID_CONFIG_REG) = (device_id_vendor_id))


#define	HAL_MISC_READ_PCI_BRIDGE_CLASS_CODE_REVISION_ID(class_code_revision_id)	\
    ((class_code_revision_id) =	(MISC_PCI_CLASS_CODE_REVISION_ID_CONFIG_REG))


#define	HAL_MISC_WRITE_PCI_BRIDGE_CLASS_CODE_REVISION_ID(class_code_revision_id) \
    ((MISC_PCI_CLASS_CODE_REVISION_ID_CONFIG_REG) = (class_code_revision_id))


#define	HAL_MISC_READ_PCI_BRIDGE_SUBSYSTEM_ID_SUBSYSTEM_VENDOR_ID(subsystem_id_subvendor_id) \
    ((subsystem_id_subvendor_id) = (MISC_PCI_SUBSYSTEM_ID_SUBVENDOR_ID_CONFIG_REG))


#define	HAL_MISC_WRITE_PCI_BRIDGE_SUBSYSTEM_ID_SUBSYSTEM_VENDOR_ID(subsystem_id_subvendor_id) \
    ((MISC_PCI_SUBSYSTEM_ID_SUBVENDOR_ID_CONFIG_REG) = (subsystem_id_subvendor_id))


#define	HAL_MISC_ENABLE_PCI_BRIDGE_66MHZ_CAPABILITY_CONFIGURATION() \
    ((MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG) |= (1))


#define	HAL_MISC_DISABLE_PCI_BRIDGE_66MHZ_CAPABILITY_CONFIGURATION() \
    ((MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG) &= ~(1))


#define	HAL_MISC_READ_PCI_BRIDGE_INTERRUPT_PIN_CONFIG(interrupt_pin_config) \
{ \
    (interrupt_pin_config) = (MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG); \
    (interrupt_pin_config) = (((interrupt_pin_config) &	0x0000FF00) >> 8); \
}


#define	HAL_MISC_WRITE_PCI_BRIDGE_INTERRUPT_PIN_CONFIG(interrupt_pin_config) \
{ \
    (MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG) &= (0xFFFF00FF); \
    (MISC_PCI_INTERRUPT_PIN_66MHZ_CAP_CONFIG_REG) |= (interrupt_pin_config << 8); \
}


#define	HAL_MISC_ENABLE_PCI_ARBITER_AGENT0_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) |= (1 << 0))


#define	HAL_MISC_ENABLE_PCI_ARBITER_AGENT1_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) |= (1 << 1))


#define	HAL_MISC_ENABLE_PCI_ARBITER_AGENT2_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) |= (1 << 2))


#define	HAL_MISC_DISABLE_PCI_ARBITER_AGENT0_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) &= (~(1 << 0)))


#define	HAL_MISC_DISABLE_PCI_ARBITER_AGENT1_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) &= (~(1 << 1)))


#define	HAL_MISC_DISABLE_PCI_ARBITER_AGENT2_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) &= (~(1 << 2)))


#define	HAL_MISC_ENABLE_PCI_ARBITER_BRIDGE_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) |= (1 << 4))


#define	HAL_MISC_DISABLE_PCI_ARBITER_BRIDGE_BROKEN_MASK() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) &= (~(1 << 4)))


#define	HAL_MISC_ENABLE_PCI_SUPPORT_33MHZ_ONLY() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) &= (~(1 << 8)))


#define	HAL_MISC_ENABLE_PCI_SUPPORT_UPTO_66MHZ() \
    ((MISC_PCI_ARBITER_INTERRUPT_MASK_REG) |= (1 << 8))


#define	HAL_MISC_READ_PCI_ARBITER_BROKEN_STATUS(pci_broken_status) \
    ((pci_broken_status) = ((MISC_PCI_BROKEN_STATUS_REG) & 0x1F))


#define	HAL_MISC_CLEAR_PCI_ARBITER_BROKEN_STATUS(pci_broken_status) \
    ((MISC_PCI_BROKEN_STATUS_REG) = ((pci_broken_status) & 0x1F))


#define	HAL_MISC_READ_DEBUG_SIGNAL_SELECT(debug_signal_select) \
    ((debug_signal_select) = (MISC_DEBUG_SIGNAL_SELECT_REG))


#define	HAL_MISC_WRITE_DEBUG_SIGNAL_SELECT(debug_signal_select)	\
    ((MISC_DEBUG_SIGNAL_SELECT_REG) = (debug_signal_select))



#define HAL_MISC_ORION_ECO_AD(ad)  (ad = ((MISC_DEBUG_SIGNAL_SELECT_REG >> 8)&0x1)) 

#endif	// end of #ifndef _STAR_MISC_H_

