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


// --------------------------------------------------------------------
//	lmc83:	modified from smc91111.h (2002-11-29)
// --------------------------------------------------------------------

#ifndef STAR9100_GSW1000_H
#define STAR9100_GSW1000_H

//#define CONFIG_STAR9100_ONEARM
#include <linux/module.h>
#include <linux/version.h>
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
//#include <linux/string.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/bitops.h>

#include <asm/io.h>
	//ivan
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

#include <asm/arch/str9100/star_gsw.h>
#include <asm/arch/str9100/star_gpio.h>



typedef unsigned char			byte;
typedef unsigned short			word;
typedef unsigned long int 		dword;



#define BIT(x)              ((1 << x))
#define LAN_PORT 1
#define WAN_PORT 2

#define TRUE 1
#define FALSE 0



#define DADDLEN         6               /* Length of Ethernet header in bytes */

#define IP_ADDR_LEN        4

#define ASIX_GIGA_PHY   1

#define TWO_SINGLE_PHY  2

#define AGERE_GIGA_PHY 3









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
#define PHY_CONTROL_REG_ADDR             0x00
#define PHY_STATUA_REG_ADDR              0x01
#define PHY_ID1_REG_ADDR                 0x02
#define PHY_ID2_REG_ADDR                 0x03
#define PHY_AN_ADVERTISEMENT_REG_ADDR    0x04
#define PHY_AN_REAMOTE_CAP_REG_ADDR      0x05
#define PHY_RESERVED1_REG_ADDR           0x10
#define PHY_RESERVED2_REG_ADDR           0x11
#define PHY_CH_STATUS_OUTPUT_REG_ADDR    0x12
#define PHY_RESERVED3_REG_ADDR           0x13
#define PHY_RESERVED4_REG_ADDR           0x14

#define PHY_LSI_L84225_ID1               0x0016
#define PHY_LSI_L84225_ID2               0xF840    // 0xF870????

//--------------------------------------------------------
//         STAR9100 Hnat related define
//--------------------------------------------------------

#define BIT(x)              ((1 << x))

/*Define for status print for Inerrrupt Status Register */
#define    INT_PORT0_Q_FULL                      BIT(0)
#define    INT_PORT1_Q_FULL                      BIT(1)
#define    INT_CPU_Q_FULL                           BIT(2)
#define    INT_HNAT_Q_FULL                        BIT(3)
#define    INT_GLOBAL_Q_FULL                    BIT(4)
#define    INT_BUFFER_FULL                         BIT(5)
#define    INT_PORT_STATUS_CHG               BIT(6)
#define    INT_INTRUDER0                             BIT(7)
#define    INT_INTRUDER1                             BIT(8)
#define    INT_CPU_HOLD                               BIT(9)
#define    INT_PORT0_UNKNOWN_VLAN       BIT(10)
#define    INT_PORT1_UNKNOWN_VLAN       BIT(11)
#define    INT_CPU_UNKNOWN_VLAN           BIT(12)
#define    INT_PORT0_NO_LINK_DROP        BIT(16)
#define    INT_PORT0_BCS_DROP                 BIT(17)
#define    INT_PORT0_RX_CRC_DROP          BIT(18)
#define    INT_PORT0_JAMED_DROP            BIT(19)
#define    INT_PORT0_QUEUE_DROP            BIT(20)
#define    INT_PORT0_RMC_DROP                BIT(21) 
#define    INT_PORT0_LOCAL_DROP            BIT(22)
#define    INT_PORT0_INGRESS_DROP        BIT(23)
#define    INT_PORT1_NO_LINK_DROP       BIT(24)
#define    INT_PORT1_BCS_DROP                BIT(25)
#define    INT_PORT1_RX_CRC_DROP         BIT(26)
#define    INT_PORT1_JAMED_DROP           BIT(27)
#define    INT_PORT1_QUEUE_DROP           BIT(28)
#define    INT_PORT1_RMC_DROP               BIT(29)
#define    PORT1_LOCAL_DROP                   BIT(30)
#define    PORT1_INGRESS_DROP               0x80000000

 
#define OS_NULL             (0)

#define UINT8               u_int8
#define UINT16              u_int16
#define UINT32              u_int32


#define MAX_VLAN_NUM        (8)

#define VLAN0_GROUP_ID      (0)
#define VLAN1_GROUP_ID      (1)
#define VLAN2_GROUP_ID      (2)
#define VLAN3_GROUP_ID      (3)
#define VLAN4_GROUP_ID      (4)
#define VLAN5_GROUP_ID      (5)
#define VLAN6_GROUP_ID      (6)
#define VLAN7_GROUP_ID      (7)

#ifdef CONFIG_STAR9100_ONEARM
#define VLAN0_VID           (0x2)
#define VLAN1_VID           (0x1)
#define VLAN2_VID           (0x3)
#define VLAN3_VID           (0x4)
#define VLAN4_VID           (0x5)
#define VLAN5_VID           (0x6)
#define VLAN6_VID           (0x7)
#define VLAN7_VID           (0x8)
#else
#define VLAN0_VID           (0x111)
#define VLAN1_VID           (0x222)
#define VLAN2_VID           (0x333)
#define VLAN3_VID           (0x444)
#define VLAN4_VID           (0x555)
#define VLAN5_VID           (0x666)
#define VLAN6_VID           (0x777)
#define VLAN7_VID           (0x888)
#endif

#define MAX_PORT_NUM        (3)         /* including port 0, port 1, and CPU port */


#define PORT0               (1 << 0)    /* bit map : bit 0 */
#define PORT1               (1 << 1)    /* bit map : bit 1 */
#define CPU_PORT            (1 << 2)    /* bit map : bit 2 */

#ifdef CONFIG_STAR9100_ONEARM
#define VLAN0_GROUP         (PORT0 | CPU_PORT)
#define VLAN1_GROUP         (PORT0 | CPU_PORT)
#define VLAN2_GROUP         (PORT0 | PORT1 | CPU_PORT)
#define VLAN3_GROUP         (0)
#define VLAN4_GROUP         (0)
#define VLAN5_GROUP         (0)
#define VLAN6_GROUP         (0)
#define VLAN7_GROUP         (0)

#else
#define VLAN0_GROUP         (PORT0 | PORT1 | CPU_PORT)
#define VLAN1_GROUP         (PORT0 | CPU_PORT)
#define VLAN2_GROUP         (PORT1 | CPU_PORT)
#define VLAN3_GROUP         (0)
#define VLAN4_GROUP         (0)
#define VLAN5_GROUP         (0)
#define VLAN6_GROUP         (0)
#define VLAN7_GROUP         (0)
#endif


#ifdef CONFIG_STAR9100_ONEARM
#define VLAN0_VLAN_TAG      (5)
#define VLAN1_VLAN_TAG      (5)
#define VLAN2_VLAN_TAG      (0)
#define VLAN3_VLAN_TAG      (0)
#define VLAN4_VLAN_TAG      (0)
#define VLAN5_VLAN_TAG      (0)
#define VLAN6_VLAN_TAG      (0)
#define VLAN7_VLAN_TAG      (0)
#else
#define VLAN0_VLAN_TAG      (0)
#define VLAN1_VLAN_TAG      (0)
#define VLAN2_VLAN_TAG      (0)
#define VLAN3_VLAN_TAG      (0)
#define VLAN4_VLAN_TAG      (0)
#define VLAN5_VLAN_TAG      (0)
#define VLAN6_VLAN_TAG      (0)
#define VLAN7_VLAN_TAG      (0)
#endif


#define PORT0_PVID          (VLAN1_GROUP_ID)
#define PORT1_PVID          (VLAN2_GROUP_ID)

#define CPU_PORT_PVID       (VLAN0_GROUP_ID)



#define Last_bit  0x10000000
#define First_bit 0x20000000
#define FirstLast_bit  0x30000000
#define C_bit      0x80000000
#define FirstLastC_bit  0xB0000000
#define FirstLastInt_bit 0x38000000
#define ForceRoute_bit 0x00400000
#define  IpCheckSumOffload    0x00040000
#define  IpUdpCheckSumOffload 0x00060000
#define  IpTcpCheckSumOffload 0x00050000
#define  FirstLast_IpTcpUdpCheckSumOffload 0x30070000
#define  IpTcpUdpCheckSumOffload 0x00070000

#ifdef CONFIG_STAR9100_ONEARM
typedef struct _arphdr
{
	unsigned short	ar_hrd;		/* format of hardware address	*/
	unsigned short	ar_pro;		/* format of protocol address	*/
	unsigned char	ar_hln;		/* length of hardware address	*/
	unsigned char	ar_pln;		/* length of protocol address	*/
	unsigned short	ar_op;		/* ARP opcode (command)		*/


	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/


}arphdr_t;
#endif


typedef struct _vlan_config_
{
    unsigned int    vlan_gid;            /* 3-bit VLAN group ID */
    
    unsigned int    vlan_vid;            /* 12-bit VLAN ID */
    
    unsigned int    vlan_group;          /* 3-bit VLAN group port map */
    
    unsigned int    vlan_tag_flag;       /* 3-bit VLAN tag port map */
    
    unsigned char   vlan_mac[6];
    
    unsigned char   pad[2];
    
} vlan_config_t;


typedef struct _port_config_
{
    unsigned int    pvid;                /* 3-bit Port PVID */
    
    unsigned int    config_flag;
    
    unsigned int    status_flag;
    
} port_config_t;


typedef struct _gsw_info_
{
    vlan_config_t    vlan[MAX_VLAN_NUM];
    
    port_config_t    port[MAX_PORT_NUM];
    
} gsw_info_t;


#define NET_BUFFER_PACKET_SIZE   (1536)

#define NET_BUFFER_SHIFT_BIT_NUM (9)

#define MAX_PACKET_LEN           (1536)

#define IP_CHECKSUM_FAIL 1
#define LAYER4_CHECKSUM_FAIL 2

/*
 * Note the size of struct _NET_BUFFER_ MUST be 16-byte aligned
 */
typedef struct _NET_BUFFER_    NET_BUFFER;

struct _NET_BUFFER_
{
    unsigned char    packet[NET_BUFFER_PACKET_SIZE];// __attribute__((aligned(16)));
    
    unsigned int     total_data_len;

    unsigned char    *data_ptr;
        
    unsigned int     data_len;
    
    unsigned int     flags;
    
    NET_BUFFER       *next_buffer;
    
    NET_BUFFER       *next;
    
    struct net_device     *ingress_dev;
    u_int32                     dma_adr;;
    
};


typedef struct _NET_BUFFER_HEADER_    NET_BUFFER_HEADER;

struct _NET_BUFFER_HEADER_
{
    NET_BUFFER    *head;
    
    NET_BUFFER    *tail;	
};




#define HNAT_FW_RULE_TABLE_START_ENTRY_INDEX (0)

#define HNAT_FW_RULE_TABLE_STOP_ENTRY_INDEX  (40)   // this entry in exclusive

#define HNAT_FL_RULE_TABLE_START_ENTRY_INDEX (60)

#define HNAT_FL_RULE_TABLE_STOP_ENTRY_INDEX  (100)  // this entry in exclusive

#define HNAT_ALG_TABLE_START_ENTRY_INDEX     (110)

#define HNAT_ALG_TABLE_STOP_ENTRY_INDEX      (130)  // this entry in exclusive

#define HNAT_SIP_TABLE_START_ENTRY_INDEX     (140)

#define HNAT_SIP_TABLE_STOP_ENTRY_INDEX      (HNAT_SIP_TABLE_START_ENTRY_INDEX + 256)  // this entry in exclusive

/* 
 * Maximum Transmit/Receive Frame Descriptors for GSW's MAC frame
 */


#define GW_GSW_MAX_TFD_NUM       (48)
#define GW_GSW_MAX_TFD_NUM_HALF  (24)
/*
#define GW_GSW_MAX_RFD_NUM        (40)
#define MAX_BUFFERS              (60)
*/
#define GW_GSW_MAX_RFD_NUM        (48)
#define GW_GSW_MAX_RFD_NUM_HALF   (24)
#define MAX_BUFFERS              (48)


typedef struct GSW_DMA_ADR_REMAP_    GSW_DMA_ADR_REMAP;
struct GSW_DMA_ADR_REMAP_
{
      unsigned int  gsw_dma_virs_adr[MAX_BUFFERS];
      unsigned int  gsw_dma_phy_start_adr;
      int                net_buffer_struct_size;
};

typedef struct GSW_DMA_RFD_ADR_REMAP_    GSW_DMA_RFD_ADR_REMAP;

struct GSW_DMA_RFD_ADR_REMAP_
{
       unsigned int  gsw_rfd_virs_adr[GW_GSW_MAX_RFD_NUM];
      unsigned int  gsw_rfd_phy_start_adr;
      unsigned int  gsw_rfd_virs_start_adr;
      struct sk_buff  * skb_info_point[GW_GSW_MAX_RFD_NUM];
      int                rfd_desc_struct_size;
};

typedef struct GSW_DMA_TFD_ADR_REMAP_    GSW_DMA_TFD_ADR_REMAP;

struct GSW_DMA_TFD_ADR_REMAP_
{
      unsigned int  gsw_tfd_virs_adr[GW_GSW_MAX_TFD_NUM];
      unsigned int  gsw_tfd_phy_start_adr;
      int                tfd_desc_struct_size;
};

typedef struct _GSW_TS_INFO1_    GSW_TS_INFO1_T;

struct _GSW_TS_INFO1_
{
    unsigned int    seg_data_ptr;
};


typedef struct _GSW_TS_INFO2_    GSW_TS_INFO2_T;

struct _GSW_TS_INFO2_
{
    unsigned int    seg_data_len          : 16;
    unsigned int    tcp_checksum_offload  : 1;
    unsigned int    udp_checksum_offload  : 1;
    unsigned int    ip_checksum_offload   : 1;    
    unsigned int    force_route_port_map  : 3;
    unsigned int    force_route           : 1;
    unsigned int    force_priority_value  : 3;
    unsigned int    force_priority        : 1;
    unsigned int    interrupt_bit         : 1;
    unsigned int    last_seg              : 1;
    unsigned int    first_seg             : 1;
    unsigned int    end_bit               : 1;
    unsigned int    c_bit                 : 1;
};


typedef struct _GSW_TS_INFO3_    GSW_TS_INFO3_T;

struct _GSW_TS_INFO3_
{
    unsigned int    vid_index             : 3;
    unsigned int    insert_vid_tag        : 1;
    unsigned int    pppoe_section_index   : 3;
    unsigned int    insert_pppoe_section  : 1;
    unsigned int    wlan_tx_ctrl0         : 24;
};


typedef struct _GSW_TS_INFO4_    GSW_TS_INFO4_T;

struct _GSW_TS_INFO4_
{
    unsigned int    wlan_tx_ctrl1;
};


/* 
 * To Switch Transmit Frame Descriptor(TS_TFD) for GSW MAC frame
 * Note all TS_TFD must be aligned on 8-byte physical address boundaries
 */
typedef struct _GSW_TS_TFD_    GSW_TS_TFD;

struct _GSW_TS_TFD_
{
    GSW_TS_INFO1_T    ts_info1;
    GSW_TS_INFO2_T    ts_info2;
    GSW_TS_INFO3_T    ts_info3;
    GSW_TS_INFO4_T    ts_info4;    
};


/* 
 * Transmit Frame Descriptor Ring for TFDS
 */
typedef struct _GSW_TS_TFD_RING_    GSW_TS_TFD_RING;

struct _GSW_TS_TFD_RING_
{
    GSW_TS_TFD    *head;
    
    GSW_TS_TFD    *tail;
    
    GSW_TS_TFD    *ts_current;
    
    GSW_TS_TFD    *ts_tc_tfd_marker;
    
    NET_BUFFER    *net_buffer_map[GW_GSW_MAX_TFD_NUM];
    
    GSW_TS_TFD    *ts_first_seg_map[GW_GSW_MAX_TFD_NUM];
    
    GSW_TS_TFD    *ts_last_seg_map[GW_GSW_MAX_TFD_NUM];
    
    u_int32       total_packet_num;
    
    u_int32       ts_tc_packet_num;
};


typedef struct _GSW_FS_INFO1_    GSW_FS_INFO1_T;

struct _GSW_FS_INFO1_
{
    unsigned int    seg_data_ptr;
};


typedef struct _GSW_FS_INFO2_    GSW_FS_INFO2_T;

struct _GSW_FS_INFO2_
{
    unsigned int    seg_data_len          : 16;
    unsigned int    layer4_checksum_fail  : 1;
    unsigned int    ip_checksum_fail      : 1;
    unsigned int    protocol_value        : 2;    
    unsigned int    hnat_reason           : 6;
    unsigned int    source_port_value     : 2;    
    unsigned int    last_seg              : 1;
    unsigned int    first_seg             : 1;
    unsigned int    end_bit               : 1;
    unsigned int    c_bit                 : 1;
};


typedef struct _GSW_FS_INFO3_    GSW_FS_INFO3_T;

struct _GSW_FS_INFO3_
{
    unsigned int    wlan_rx_status0;

};


typedef struct _GSW_FS_INFO4_    GSW_FS_INFO4_T;

struct _GSW_FS_INFO4_
{
    unsigned int    wlan_rx_status1;
};


/* 
 * From Switch Receive Frame Descriptor(FS_RFD) for GSW MAC frame
 * Note all FS_RFD must be aligned on 8-byte physical address boundaries
 */
typedef struct _GSW_FS_RFD_    GSW_FS_RFD;

struct _GSW_FS_RFD_
{
    GSW_FS_INFO1_T    fs_info1;
    GSW_FS_INFO2_T    fs_info2;
    GSW_FS_INFO3_T    fs_info3;
    GSW_FS_INFO4_T    fs_info4;  
};


/* 
 * Receive Frame Descriptor Ring for RFDS
 */
typedef struct _GSW_FS_RFD_RING_    GSW_FS_RFD_RING;

struct _GSW_FS_RFD_RING_
{
    GSW_FS_RFD    *head;
    
    GSW_FS_RFD    *tail;
    
    GSW_FS_RFD    *fs_current;
    
    GSW_FS_RFD    *fs_first_seg_map[GW_GSW_MAX_RFD_NUM];
    
    GSW_FS_RFD    *fs_last_seg_map[GW_GSW_MAX_RFD_NUM];
    
    unsigned int  total_packet_num;
    
    unsigned int  fs_rc_packet_num;
    
    unsigned int  forward_flag;
};



/* store this information for the driver.. */
struct star9100_local {

 	// these are things that the kernel wants me to keep, so users
	// can find out semi-useless statistics of how well the card is
	// performing
	struct net_device_stats stats;

	// Set to true during the auto-negotiation sequence

	

	
	
	spinlock_t lock;
	

			
	/* Note:
         * device entry dev_index = 0 means local loopback network interface
         * device entry dev_index = 1 means GSW port 0 for LAN port network interface
         * device entry dev_index = 2 means GSW port 1 for WAN port network interface
        */ 
	int                 dev_index;
	
};

typedef struct _gw_info_
{
    UINT8   PPPOE_Local_Ip[IP_ADDR_LEN];    
    UINT8   PPPOE_Remote_Ip[IP_ADDR_LEN];
    UINT8   PPPOE_AC_MAC_ADDR[DADDLEN];
    UINT16 PPPOE_SESSION_ID;
    UINT8   ETH_Local_Ip[IP_ADDR_LEN];
    UINT8   ETH_Remote_Ip[IP_ADDR_LEN];    
    UINT32            hnat_flag;
} gw_info_t;

#define eth_outl(_addr, _value)        orn_outl(_addr, _value)
#define eth_outw(_addr, _value)        orn_outw(_addr, _value)
#define eth_outb(_addr, _value)        orn_outb(_addr, _value)

#define eth_inl(_addr)                 orn_inl(_addr)
#define eth_inw(_addr)                 orn_inw(_addr)
#define eth_inb(_addr)                 orn_inb(_addr)

#endif
