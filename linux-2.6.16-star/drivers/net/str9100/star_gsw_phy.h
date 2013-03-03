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


#ifndef __STAR_GSW_PHY_H__
#define __STAR_GSW_PHY_H__




#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,32)
#define LINUX24 1
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define LINUX26 1
#endif

#include <linux/types.h>


//#define CONFIG_VIRGO
//#define CONFIG_DORADO
//#define CONFIG_DORADO2
//#define CONFIG_LEO
//#undef CONFIG_VIRGO
//#undef CONFIG_DORADO
//#undef CONFIG_DORADO2
//#undef CONFIG_LEO

#ifdef CONFIG_LIBRA2
#include "libra2.h"
#endif

#ifdef CONFIG_LIBRA
#include "libra.h"
#endif

#ifdef CONFIG_VELA
#include "vela.h"
#endif

#ifdef CONFIG_DORADO2
#include "dorado2.h"
#endif

#ifdef CONFIG_DORADO
#include "dorado.h"
#endif

#ifdef CONFIG_LEO
#include "leo.h"
#endif

#ifdef CONFIG_VIRGO
#include "virgo.h"
#endif


void star_gsw_config_ASIX(void);
void star_gsw_config_AGERE(void);
int AGERE_phy_power_down(int phy_addr, int y);
int disable_AN_VSC7385(int y);
void star_gsw_config_VSC7385(void);




void star_gsw_config_AGERE(void);

int str9100_gsw_config_mac_port0(void);
int str9100_gsw_config_mac_port1(void);



#define PORT0				(1 << 0)	/* bit map : bit 0 */
#define PORT1				(1 << 1)	/* bit map : bit 1 */
#define CPU_PORT			(1 << 2)	/* bit map : bit 2 */

#define VLAN0_GROUP_ID			(0)
#define VLAN1_GROUP_ID			(1)
#define VLAN2_GROUP_ID			(2)
#define VLAN3_GROUP_ID			(3)
#define VLAN4_GROUP_ID			(4)
#define VLAN5_GROUP_ID			(5)
#define VLAN6_GROUP_ID			(6)
#define VLAN7_GROUP_ID			(7)

#define PORT0_PVID			(VLAN1_GROUP_ID)
#define PORT1_PVID			(VLAN2_GROUP_ID)
#define CPU_PORT_PVID			(VLAN0_GROUP_ID)

#define INVALID_PORT_BASE_PMAP_PORT -1


#ifdef LINUX26
#define GSW_MAC_PORT_0_CONFIG GSW_MAC_PORT_0_CONFIG_REG
#define GSW_MAC_PORT_1_CONFIG GSW_MAC_PORT_1_CONFIG_REG
#define GSW_PORT_MIRROR GSW_PORT_MIRROR_REG
#define GSW_QUEUE_STATUS_TEST_1 GSW_QUEUE_STATUS_TEST_1_REG
#define GSW_PHY_CONTROL GSW_PHY_CONTROL_REG
#define GSW_BIST_RESULT_TEST_0 GSW_BIST_RESULT_TEST_0_REG
#define GSW_PORT0_CFG_REG (SYSVA_GSW_BASE_ADDR + 0x08)
#define GSW_PORT1_CFG_REG (SYSVA_GSW_BASE_ADDR + 0x0C)
#define GSW_SWITCH_CONFIG GSW_SWITCH_CONFIG_REG
#define GSW_ARL_TABLE_ACCESS_CONTROL_0 GSW_ARL_TABLE_ACCESS_CONTROL_0_REG
#define GSW_ARL_TABLE_ACCESS_CONTROL_1 GSW_ARL_TABLE_ACCESS_CONTROL_1_REG
#define GSW_ARL_TABLE_ACCESS_CONTROL_2 GSW_ARL_TABLE_ACCESS_CONTROL_2_REG
#define GSW_VLAN_PORT_PVID GSW_VLAN_PORT_PVID_REG
#define GSW_VLAN_VID_0_1 GSW_VLAN_VID_0_1_REG
#define GSW_VLAN_VID_2_3 GSW_VLAN_VID_2_3_REG
#define GSW_VLAN_VID_4_5 GSW_VLAN_VID_4_5_REG
#define GSW_VLAN_VID_6_7 GSW_VLAN_VID_6_7_REG
#define GSW_VLAN_MEMBER_PORT_MAP GSW_VLAN_MEMBER_PORT_MAP_REG
#define GSW_VLAN_TAG_PORT_MAP GSW_VLAN_TAG_PORT_MAP_REG
#define GSW_INTERRUPT_MASK GSW_INTERRUPT_MASK_REG
#define GSW_INTERRUPT_STATUS GSW_INTERRUPT_STATUS_REG
#define INTC_INTERRUPT_CLEAR_EDGE_TRIGGER INTC_INTERRUPT_CLEAR_EDGE_TRIGGER_REG
#define GSW_TS_DMA_CONTROL GSW_TS_DMA_CONTROL_REG
#define GSW_FS_DMA_CONTROL GSW_FS_DMA_CONTROL_REG
#define GSW_TS_DESCRIPTOR_POINTER GSW_TS_DESCRIPTOR_POINTER_REG
#define GSW_TS_DESCRIPTOR_BASE_ADDR GSW_TS_DESCRIPTOR_BASE_ADDR_REG
#define GSW_FS_DESCRIPTOR_POINTER GSW_FS_DESCRIPTOR_POINTER_REG
#define GSW_FS_DESCRIPTOR_BASE_ADDR GSW_FS_DESCRIPTOR_BASE_ADDR_REG
#define GSW_CPU_PORT_CONFIG GSW_CPU_PORT_CONFIG_REG
#define INTC_INTERRUPT_MASK INTC_INTERRUPT_MASK_REG
#define GSW_DELAYED_INTERRUPT_CONFIG GSW_DELAYED_INTERRUPT_CONFIG_REG
#define GSW_HNAT_SOURCE_MAC_0_HIGH GSW_HNAT_SOURCE_MAC_0_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_0_LOW GSW_HNAT_SOURCE_MAC_0_LOW_REG
#define GSW_HNAT_SOURCE_MAC_1_HIGH GSW_HNAT_SOURCE_MAC_1_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_1_LOW GSW_HNAT_SOURCE_MAC_1_LOW_REG
#define GSW_HNAT_SOURCE_MAC_2_HIGH GSW_HNAT_SOURCE_MAC_2_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_2_LOW GSW_HNAT_SOURCE_MAC_2_LOW_REG
#define GSW_HNAT_SOURCE_MAC_3_HIGH GSW_HNAT_SOURCE_MAC_3_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_3_LOW GSW_HNAT_SOURCE_MAC_3_LOW_REG
#define GSW_HNAT_SOURCE_MAC_4_HIGH GSW_HNAT_SOURCE_MAC_4_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_4_LOW GSW_HNAT_SOURCE_MAC_4_LOW_REG
#define GSW_HNAT_SOURCE_MAC_5_HIGH GSW_HNAT_SOURCE_MAC_5_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_5_LOW GSW_HNAT_SOURCE_MAC_5_LOW_REG
#define GSW_HNAT_SOURCE_MAC_6_HIGH GSW_HNAT_SOURCE_MAC_6_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_6_LOW GSW_HNAT_SOURCE_MAC_6_LOW_REG
#define GSW_HNAT_SOURCE_MAC_7_HIGH GSW_HNAT_SOURCE_MAC_7_HIGH_REG
#define GSW_HNAT_SOURCE_MAC_7_LOW GSW_HNAT_SOURCE_MAC_7_LOW_REG
#define PWRMGT_SOFTWARE_RESET_CONTROL PWRMGT_SOFTWARE_RESET_CONTROL_REG
#define PWRMGT_PAD_DRIVE_STRENGTH_CONTROL PWRMGT_PAD_DRIVE_STRENGTH_CONTROL_REG
#define GSW_ISP_TAGGING_PORT_CONFIG GSW_ISP_TAGGING_PORT_CONFIG_REG

#endif

#endif /* __STAR_GSW_PHY_H__ */
