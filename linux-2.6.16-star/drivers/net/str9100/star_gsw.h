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


#ifndef STAR_GSW_H
#define STAR_GSW_H

#include "star_gsw_phy.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/reboot.h>
#include <asm/bitops.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_arp.h>
#include <net/arp.h>

#if defined(LINUX24)
#include <asm/arch/str9100/star_gsw.h>
#include <asm/arch/str9100/star_gpio.h>
#include <asm/arch/str9100/star_powermgt.h>
#endif

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#endif




#include "star_gsw_config.h"

#define BIT(x)		((1 << (x)))

#define LAN_PORT	1
#define WAN_PORT	2
#define EWC_PORT	3
#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH

#define TUN_PORT	4
#define PMAP_TUN_PORT	8
#define CREATE_NET_DEV_AD star_gsw_probe_tun();
#define PORT_BASE_PMAP_TUN_PORT	PMAP_TUN_PORT
#else
#define CREATE_NET_DEV_AD
#define PORT_BASE_PMAP_TUN_PORT
#endif


// add by descent 2006/07/10
#define PMAP_PORT0	1
#define PMAP_PORT1	2
#define PMAP_CPU_PORT	4


/*
 * macro declarations
 */
#define GSW_SET_PORT0_PVID(port0_pvid) \
{ \
	((GSW_VLAN_PORT_PVID) &= (~(0x7 << 0))); \
	((GSW_VLAN_PORT_PVID) |= ((port0_pvid) & 0x07)); \
}

#define GSW_SET_PORT1_PVID(port1_pvid) \
{ \
	((GSW_VLAN_PORT_PVID) &= (~(0x7 << 4))); \
	((GSW_VLAN_PORT_PVID) |= (((port1_pvid) & 0x07) << 4)); \
}

#define GSW_SET_CPU_PORT_PVID(cpu_port_pvid) \
{ \
	((GSW_VLAN_PORT_PVID) &= (~(0x7 << 8))); \
	((GSW_VLAN_PORT_PVID) |= (((cpu_port_pvid) & 0x07) << 8)); \
}

#define GSW_SET_VLAN_0_VID(vid) \
{ \
	((GSW_VLAN_VID_0_1) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_0_1) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_1_VID(vid) \
{ \
	((GSW_VLAN_VID_0_1) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_0_1) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_2_VID(vid) \
{ \
	((GSW_VLAN_VID_2_3) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_2_3) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_3_VID(vid) \
{ \
	((GSW_VLAN_VID_2_3) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_2_3) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_4_VID(vid) \
{ \
	((GSW_VLAN_VID_4_5) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_4_5) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_5_VID(vid) \
{ \
	((GSW_VLAN_VID_4_5) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_4_5) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_6_VID(vid) \
{ \
	((GSW_VLAN_VID_6_7) &= (~(0xFFF << 0))); \
	((GSW_VLAN_VID_6_7) |= (((vid) & 0xFFF) << 0)); \
}

#define GSW_SET_VLAN_7_VID(vid) \
{ \
	((GSW_VLAN_VID_6_7) &= (~(0xFFF << 12))); \
	((GSW_VLAN_VID_6_7) |= (((vid) & 0xFFF) << 12)); \
}

#define GSW_SET_VLAN_0_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 0))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 0)); \
}

#define GSW_SET_VLAN_1_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 3))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 3)); \
}

#define GSW_SET_VLAN_2_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 6))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 6)); \
}

#define GSW_SET_VLAN_3_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 9))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 9)); \
}

#define GSW_SET_VLAN_4_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 12))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 12)); \
}

#define GSW_SET_VLAN_5_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 15))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 15)); \
}

#define GSW_SET_VLAN_6_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 18))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 18)); \
}

#define GSW_SET_VLAN_7_MEMBER(vlan_member) \
{ \
	((GSW_VLAN_MEMBER_PORT_MAP) &= (~(0x7 << 21))); \
	((GSW_VLAN_MEMBER_PORT_MAP) |= (((vlan_member) & 0x7) << 21)); \
}

#define GSW_SET_VLAN_0_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 0))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 0)); \
}

#define GSW_SET_VLAN_1_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 3))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 3)); \
}

#define GSW_SET_VLAN_2_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 6))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 6)); \
}

#define GSW_SET_VLAN_3_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 9))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 9)); \
}

#define GSW_SET_VLAN_4_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 12))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 12)); \
}

#define GSW_SET_VLAN_5_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 15))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 15)); \
}

#define GSW_SET_VLAN_6_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 18))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 18)); \
}

#define GSW_SET_VLAN_7_TAG(vlan_tag) \
{ \
	((GSW_VLAN_TAG_PORT_MAP) &= (~(0x7 << 21))); \
	((GSW_VLAN_TAG_PORT_MAP) |= (((vlan_tag) & 0x7) << 21)); \
}

#define GSW_SET_PPPOE_SESSION_0_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_0_1) &= (~(0xFFFF << 0))); \
	((GSW_PPPOE_SESSION_ID_0_1) |= (((session_id) & 0xFFFF) << 0)); \
}

#define GSW_SET_PPPOE_SESSION_1_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_0_1) &= (~(0xFFFF << 16))); \
	((GSW_PPPOE_SESSION_ID_0_1) |= (((session_id) & 0xFFFF) << 16)); \
}

#define GSW_SET_PPPOE_SESSION_2_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_2_3) &= (~(0xFFFF << 0))); \
	((GSW_PPPOE_SESSION_ID_2_3) |= (((session_id) & 0xFFFF) << 0)); \
}

#define GSW_SET_PPPOE_SESSION_3_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_2_3) &= (~(0xFFFF << 16))); \
	((GSW_PPPOE_SESSION_ID_2_3) |= (((session_id) & 0xFFFF) << 16)); \
}

#define GSW_SET_PPPOE_SESSION_4_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_4_5) &= (~(0xFFFF << 0))); \
	((GSW_PPPOE_SESSION_ID_4_5) |= (((session_id) & 0xFFFF) << 0)); \
}

#define GSW_SET_PPPOE_SESSION_5_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_4_5) &= (~(0xFFFF << 16))); \
	((GSW_PPPOE_SESSION_ID_4_5) |= (((session_id) & 0xFFFF) << 16)); \
}

#define GSW_SET_PPPOE_SESSION_6_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_6_7) &= (~(0xFFFF << 0))); \
	((GSW_PPPOE_SESSION_ID_6_7) |= (((session_id) & 0xFFFF) << 0)); \
}

#define GSW_SET_PPPOE_SESSION_7_ID(session_id) \
{ \
	((GSW_PPPOE_SESSION_ID_6_7) &= (~(0xFFFF << 16))); \
	((GSW_PPPOE_SESSION_ID_6_7) |= (((session_id) & 0xFFFF) << 16)); \
}

#define GSW_READ_INTERRUPT_STATUS(int_status) \
	((int_status) = (GSW_INTERRUPT_STATUS))

#define GSW_CLEAR_ALL_INTERRUPT_STATUS_SOURCES()\
	((GSW_INTERRUPT_STATUS) = (0x00001FFF))

#define GSW_CLEAR_INTERRUPT_STATUS_SOURCES(source) \
	((GSW_INTERRUPT_STATUS) |= (source))

#define GSW_CLEAR_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
	((GSW_INTERRUPT_STATUS) |= (1 << (source_bit_index)))

#define GSW_DISABLE_ALL_INTERRUPT_STATUS_SOURCES() \
	((GSW_INTERRUPT_MASK) = (0x00001FFF))

#define GSW_ENABLE_ALL_INTERRUPT_STATUS_SOURCES() \
	((GSW_INTERRUPT_MASK) = (0x00000000))

#define GSW_DISABLE_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
	((GSW_INTERRUPT_MASK) |= (1 << (source_bit_index)))

#define GSW_ENABLE_INTERRUPT_STATUS_SOURCE_BIT(source_bit_index) \
	((GSW_INTERRUPT_MASK) &= ~(1 << (source_bit_index)))

#define GSW_TS_DMA_START() \
	((GSW_TS_DMA_CONTROL) = (1))

#define GSW_TS_DMA_STOP() \
	((GSW_TS_DMA_CONTROL) = (0))

#define GSW_READ_TS_DMA_STATE(state) \
	((state) = (GSW_TS_DMA_CONTROL))

#define GSW_FS_DMA_START() \
	((GSW_FS_DMA_CONTROL) = (1))

#define GSW_FS_DMA_STOP() \
	((GSW_FS_DMA_CONTROL) = (0))

#define GSW_WRITE_TSSD(tssd_value) \
	((GSW_TS_DESCRIPTOR_POINTER) = (tssd_value))

#define GSW_READ_TSSD(tssd_value) \
	((tssd_value) = (GSW_TS_DESCRIPTOR_POINTER))

#define GSW_WRITE_FSSD(fssd_value) \
	((GSW_FS_DESCRIPTOR_POINTER) = (fssd_value))

#define GSW_READ_FSSD(fssd_value) \
	((fssd_value) = (GSW_FS_DESCRIPTOR_POINTER))

#define GSW_WRITE_TS_BASE(ts_base_value) \
	((GSW_TS_DESCRIPTOR_BASE_ADDR) = (ts_base_value))

#define GSW_READ_TS_BASE(ts_base_value) \
	((ts_base_value) = (GSW_TS_DESCRIPTOR_BASE_ADDR))

#define GSW_WRITE_FS_BASE(fs_base_value) \
	((GSW_FS_DESCRIPTOR_BASE_ADDR) = (fs_base_value))

#define GSW_READ_FS_BASE(fs_base_value) \
	((fs_base_value) = (GSW_FS_DESCRIPTOR_BASE_ADDR))

/*
 * HNAT macros defines
 */
#define GSW_WRITE_HNAT_CONFIGURATION(hnat_config) \
	((GSW_HNAT_CONFIG) = (hnat_config))

#define GSW_READ_HNAT_CONFIGURATION(hnat_config) \
	((hnat_config) = (GSW_HNAT_CONFIG))

#define GSW_WRITE_PRIVATE_IP_BASE(ip_base) \
	((GSW_HNAT_PRIVATE_IP_BASE) = (ip_base & 0x000FFFFF))

#define GSW_WRITE_HNAT_FW_RULE_START_INDEX(rule_start_index) \
	((GSW_HNAT_FW_RULE_START_ADDR) = (rule_start_index & 0x1FF))

#define GSW_WRITE_HNAT_FW_RULE_END_INDEX(rule_end_index) \
	((GSW_HNAT_FW_RULE_END_ADDR) = (rule_end_index & 0x1FF))

#define GSW_READ_HNAT_FW_RULE_START_INDEX(rule_start_index) \
	((rule_start_index) = ((GSW_HNAT_FW_RULE_START_ADDR) & 0x1FF))

#define GSW_READ_HNAT_FW_RULE_END_INDEX(rule_end_index) \
	((rule_end_index) = ((GSW_HNAT_FW_RULE_END_ADDR) & 0x1FF))

#define GSW_WRITE_HNAT_FL_RULE_START_INDEX(rule_start_index) \
	((GSW_HNAT_FL_RULE_START_ADDR) = (rule_start_index & 0x1FF))

#define GSW_WRITE_HNAT_FL_RULE_END_INDEX(rule_end_index) \
	((GSW_HNAT_FL_RULE_END_ADDR) = (rule_end_index & 0x1FF))

#define GSW_READ_HNAT_FL_RULE_START_INDEX(rule_start_index) \
	((rule_start_index) = ((GSW_HNAT_FL_RULE_START_ADDR) & 0x1FF))

#define GSW_READ_HNAT_FL_RULE_END_INDEX(rule_end_index) \
	((rule_end_index) = ((GSW_HNAT_FL_RULE_END_ADDR) & 0x1FF))

#define GSW_WRITE_HNAT_ALG_START_INDEX(rule_start_index) \
	((GSW_HNAT_ALG_START_ADDR) = (rule_start_index & 0x1FF))

#define GSW_WRITE_HNAT_ALG_END_INDEX(rule_end_index) \
	((GSW_HNAT_ALG_END_ADDR) = (rule_end_index & 0x1FF))

#define GSW_READ_HNAT_ALG_START_INDEX(rule_start_index) \
	((rule_start_index) = ((GSW_HNAT_ALG_START_ADDR) & 0x1FF))

#define GSW_READ_HNAT_ALG_END_INDEX(rule_end_index) \
	((rule_end_index) = ((GSW_HNAT_ALG_END_ADDR) & 0x1FF))

#define GSW_WRITE_HNAT_SIP_START_INDEX(rule_start_index) \
	((GSW_HNAT_SIP_BASE_ADDR) = (rule_start_index & 0x1FF))

#define GSW_READ_HNAT_SIP_START_INDEX(rule_start_index) \
	((rule_start_index) = ((GSW_HNAT_SIP_BASE_ADDR) & 0x1FF))

#define GSW_WRITE_HNAT_NAPT_BASE_ADDR(base_addr) \
	((GSW_HNAT_NAPT_BASE_ADDR) = (base_addr))

#define GSW_READ_HNAT_NAPT_BASE_ADDR(base_addr) \
	((base_addr) = (GSW_HNAT_NAPT_BASE_ADDR))

#define GSW_WRITE_HNAT_NAPT_PORT_BASE(port_base) \
	((GSW_HNAT_NAPT_PORT_BASE) = (port_base & 0xFFFF))

#define GSW_WRITE_HNAT_ARP_BASE_ADDR(base_addr) \
	((GSW_HNAT_ARP_BASE_ADDR) = (base_addr))

#define GSW_READ_HNAT_ARP_BASE_ADDR(base_addr) \
	((base_addr) = (GSW_HNAT_ARP_BASE_ADDR))

//---------------------------------------------------
//      STAR9100   INTC  macro define
//---------------------------------------------------
/*
 * macro declarations
 */
#define INTC_ENABLE_INTERRUPT_SOURCE(source_bit_index) \
	(INTC_INTERRUPT_MASK) &= (~(1 << source_bit_index))

#define INTC_DISABLE_INTERRUPT_SOURCE(source_bit_index) \
	(INTC_INTERRUPT_MASK) |= ((1 << source_bit_index))

#define INTC_CLEAR_EDGE_TRIGGER_INTERRUPT(source_bit_index) \
	(INTC_INTERRUPT_CLEAR_EDGE_TRIGGER) |= ((1 << source_bit_index))

#define INTC_SET_INTERRUPT_EDGE_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_MODE) |= ((1 << source_bit_index))

#define INTC_SET_INTERRUPT_LEVEL_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_MODE) &= (~(1 << source_bit_index))

#define INTC_SET_INTERRUPT_RISING_EDGE_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_LEVEL) &= (~(1 << source_bit_index))

#define INTC_SET_INTERRUPT_FALLING_EDGE_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_LEVEL) |= ((1 << source_bit_index))

#define INTC_SET_INTERRUPT_ACTIVE_HIGH_LEVEL_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_LEVEL) &= (~(1 << source_bit_index))

#define INTC_SET_INTERRUPT_ACTIVE_LOW_LEVEL_TRIGGER(source_bit_index) \
	(INTC_INTERRUPT_TRIGGER_LEVEL) |= ((1 << source_bit_index))

#define INTC_ASSIGN_INTERRUPT_TO_IRQ(source_bit_index) \
	(INTC_FIQ_MODE_SELECT) &= (~(1 << source_bit_index))

#define INTC_ASSIGN_INTERRUPT_TO_FIQ(source_bit_index) \
	(INTC_FIQ_MODE_SELECT) |= ((1 << source_bit_index))

/*-------------------------------------------------*/
//                       PHY define
/*-------------------------------------------------*/
#define PHY_CONTROL_REG_ADDR		0x00
#define PHY_STATUA_REG_ADDR		0x01
#define PHY_ID1_REG_ADDR		0x02
#define PHY_ID2_REG_ADDR		0x03
#define PHY_AN_ADVERTISEMENT_REG_ADDR	0x04
#define PHY_AN_REAMOTE_CAP_REG_ADDR	0x05
#define PHY_RESERVED1_REG_ADDR		0x10
#define PHY_RESERVED2_REG_ADDR		0x11
#define PHY_CH_STATUS_OUTPUT_REG_ADDR	0x12
#define PHY_RESERVED3_REG_ADDR		0x13
#define PHY_RESERVED4_REG_ADDR		0x14

#define PHY_LSI_L84225_ID1		0x0016
#define PHY_LSI_L84225_ID2		0xF840	// 0xF870????

//--------------------------------------------------------
//         STAR9100 Hnat related define
//--------------------------------------------------------
/*Define for status print for Inerrrupt Status Register */
#define INT_PORT0_Q_FULL		BIT(0)
#define INT_PORT1_Q_FULL		BIT(1)
#define INT_CPU_Q_FULL			BIT(2)
#define INT_HNAT_Q_FULL			BIT(3)
#define INT_GLOBAL_Q_FULL		BIT(4)
#define INT_BUFFER_FULL			BIT(5)
#define INT_PORT_STATUS_CHG		BIT(6)
#define INT_INTRUDER0			BIT(7)
#define INT_INTRUDER1			BIT(8)
#define INT_CPU_HOLD			BIT(9)
#define INT_PORT0_UNKNOWN_VLAN		BIT(10)
#define INT_PORT1_UNKNOWN_VLAN		BIT(11)
#define INT_CPU_UNKNOWN_VLAN		BIT(12)
#define INT_PORT0_NO_LINK_DROP		BIT(16)
#define INT_PORT0_BCS_DROP		BIT(17)
#define INT_PORT0_RX_CRC_DROP		BIT(18)
#define INT_PORT0_JAMED_DROP		BIT(19)
#define INT_PORT0_QUEUE_DROP		BIT(20)
#define INT_PORT0_RMC_DROP		BIT(21)
#define INT_PORT0_LOCAL_DROP		BIT(22)
#define INT_PORT0_INGRESS_DROP		BIT(23)
#define INT_PORT1_NO_LINK_DROP		BIT(24)
#define INT_PORT1_BCS_DROP		BIT(25)
#define INT_PORT1_RX_CRC_DROP		BIT(26)
#define INT_PORT1_JAMED_DROP		BIT(27)
#define INT_PORT1_QUEUE_DROP		BIT(28)
#define INT_PORT1_RMC_DROP		BIT(29)
#define PORT1_LOCAL_DROP		BIT(30)
#define PORT1_INGRESS_DROP		0x80000000

#define MAX_VLAN_NUM			(8)
#define MAX_PORT_NUM			(3)	/* including port 0, port 1, and CPU port */

typedef struct _vlan_config_ {
	u32	vlan_gid;	/* 3-bit VLAN group ID */
	u32	vlan_vid;	/* 12-bit VLAN ID */
	u32	vlan_group;	/* 3-bit VLAN group port map */
	u32	vlan_tag_flag;	/* 3-bit VLAN tag port map */
	u8	vlan_mac[6];
	u8	pad[2];
} vlan_config_t;

typedef struct _port_config_ {
	u32	pvid;	/* 3-bit Port PVID */
	u32	config_flag;
	u32	status_flag;
} port_config_t;

typedef struct _gsw_info_ {
	vlan_config_t vlan[MAX_VLAN_NUM];
	port_config_t port[MAX_PORT_NUM];
} gsw_info_t;

/* store this information for the driver.. */
struct star_gsw_private {
	struct net_device_stats stats;
	spinlock_t lock;
	/* Note:
	 * device entry pmap = 0 means local loopback network interface
	 * device entry pmap = 1 means GSW port 0 for LAN port network interface
	 * device entry pmap = 2 means GSW port 1 for WAN port network interface
	 * device entry pmap = 4 means GSW cpu port
	 */
	int pmap;
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
        struct vlan_group               *vlgrp;
#endif

};

#define __REG(reg)	(*(u32 volatile *)(reg))

/*
 * Network Driver, Receive/Send and Initial Buffer Function
 */
typedef struct {
	// 1st 32Bits
	u32 data_ptr;

	// 2nd  32Bits
	u32 length:16;
	u32 tco:1;
	u32 uco:1;
	u32 ico:1;
	u32 pmap:3;
	u32 fr:1;
	u32 pri:3;
	u32 fp:1;
	u32 interrupt:1;
	u32 ls:1;
	u32 fs:1;
	u32 eor:1;
	u32 cown:1;

	// 3rd 32Bits
	u32 vid:3;
	u32 insv:1;
	u32 sid:3;
	u32 inss:1;
	u32 unused:24;

	// 4th 32Bits
	u32 unused2;

} __attribute__((packed)) STAR_GSW_TXDESC;

typedef struct {
	// 1st 32Bits
	u32 data_ptr;

	// 2nd  32Bits
	u32 length:16;
	u32 l4f:1;
	u32 ipf:1;
	u32 prot:2;
	u32 hr:6;
	u32 sp:2;
	u32 ls:1;
	u32 fs:1;
	u32 eor:1;
	u32 cown:1;

	// 3rd 32Bits
	u32 unused;

	// 4th 32Bits
	u32 unused2;

} __attribute__((packed)) STAR_GSW_RXDESC;

/* 
 * Transmit Frame Descriptor Ring for TFDS
 */

typedef struct {
	u32			phy_addr;
	STAR_GSW_TXDESC		*vir_addr;
	unsigned int		cur_index; // TX's current will point to Free Descriptors
	struct sk_buff		*skb_ptr[STAR_GSW_MAX_TFD_NUM]; // TX's sk_buff ptr
#if defined(FREE_TX_SKB_MULTI) || defined(STAR_GSW_TIMER)
        u32                     to_free_index;
#endif

} TXRING_INFO;
/* 
 * Receive Frame Descriptor Ring for RFDS
 */

typedef struct {
	u32			phy_addr;
	STAR_GSW_RXDESC		*vir_addr;
	u32			cur_index;
	struct sk_buff		*skb_ptr[STAR_GSW_MAX_RFD_NUM];	// RX's sk_buff ptr
} RXRING_INFO;

extern void str9100_set_interrupt_trigger(unsigned int, unsigned int, unsigned int);

/*
 * data structure defines
 */
typedef struct _gsw_arl_table_entry_
{
    u32    filter;
    u32    vlan_mac;
    u32    vlan_gid;
    u32    age_field;
    u32    port_map;
    u8     mac_addr[6];
    u8     pad[2];

} gsw_arl_table_entry_t;

#endif

