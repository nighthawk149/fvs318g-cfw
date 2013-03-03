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


#ifndef __STAR_GSW_CONFIG_H__
#define __STAR_GSW_CONFIG_H__


// add by descent 2006/07/05
// this debug macro reference linux device drivers, 2e

//#define STR9100_GSW_DEBUG

#undef PDEBUG
  #ifdef STR9100_GSW_DEBUG
    #ifdef __KERNEL__
      #define PDEBUG(fmt, args...) printk (KERN_INFO "*str9100_gsw_debug* " fmt, ## args)
      //#define PDEBUG printk 
    #else
      #define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
    #endif // __KERNEL__
  #else
    #define PDEBUG(fmt, args...)
  #endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)


// modify by descent 2006/07/05
// use for print needed message
#if 1
#define PRINT_INFO printk 
#else
#define PRINT_INFO(arg...)
#endif


#define CONFIG_STAR_GSW_NAPI

// add by descent 2006/07/07
// in star_gsw_get_rfd_buff
// if define CONFIG_STAR_GSW_BRIDGE drop no packets
// else drop invalid packets.


#include <linux/config.h>
#ifdef CONFIG_BRIDGE // reference linux config network bridge
#define CONFIG_STAR_GSW_BRIDGE
#endif

// add by descent 2006/07/05
// 1 mean rate controll on
// 0 mean rate controll off
#define STR9100_GSW_BROADCAST_RATE_CONTROL 0
#define STR9100_GSW_MULTICAST_RATE_CONTROL 0
#define STR9100_GSW_UNKNOW_PACKET_RATE_CONTROL 0

// 1 mean forwarding off
// 0 mean forwarding on
#define STR9100_GSW_DISABLE_FORWARDING_BROADCAST_PACKET 0
#define STR9100_GSW_DISABLE_FORWARDING_MULTICAST_PACKET 0
#define STR9100_GSW_DISABLE_FORWARDING_UNKNOW_PACKET 0

//#define STR9100_GSW_FAST_AGE_OUT

//#define STAR_GSW_TIMER
//#define FREE_TX_SKB_MULTI

#define STAR_GSW_TX_HW_CHECKSUM
#define STAR_GSW_RX_HW_CHECKSUM

#define STAR_GSW_SG
#if defined(STAR_GSW_SG) && !defined(STAR_GSW_TX_HW_CHECKSUM)
#define STAR_GSW_TX_HW_CHECKSUM
#endif

//#define ADJUSTMENT_TX_RX_SKEW
//#define STAR_GSW_STATUS_ISR
#define STAR_GSW_FSQF_ISR


// adjust MAX_PEND_INT_CNT and MAX_PEND_TIME at run time.
//#define CHANGE_DELAY_INT

#define STAR_GSW_DELAYED_INTERRUPT

// 20070130 Richard.Liu : MAX_PEND_INT_CNT = 0x4 and MAX_PEND_TIME = 0x40 will get
//                        better performance
#define MAX_PEND_INT_CNT	0x4
#define MAX_PEND_TIME		0x40

/*
 * This tag is for 9102 + 9109 Case only, and 9102's MAC0 Connect ot 9102's MAC1
 */
#ifdef CONFIG_STAR_GSW_TYPE_EWC
#define CONFIG_STAR_GSW_TYPE_ONEARM "y"
#undef CONFIG_SERCOMM_VSC7385
#endif

#if 0

#define VLAN0_GROUP_ID			(0)
#define VLAN1_GROUP_ID			(1)
#define VLAN2_GROUP_ID			(2)
#define VLAN3_GROUP_ID			(3)
#define VLAN4_GROUP_ID			(4)
#define VLAN5_GROUP_ID			(5)
#define VLAN6_GROUP_ID			(6)
#define VLAN7_GROUP_ID			(7)

#ifdef CONFIG_STAR_GSW_TYPE_ONEARM
#ifdef CONFIG_SERCOMM_VSC7385
#define VLAN0_VID			(0x1)
#define VLAN1_VID			(0x2)
#define VLAN2_VID			(0x3)
#define VLAN3_VID			(0x4)
#define VLAN4_VID			(0x5)
#define VLAN5_VID			(0x6)
#define VLAN6_VID			(0x7)
#define VLAN7_VID			(0x8)
#else
#define VLAN0_VID			(0x2)
#define VLAN1_VID			(0x1)
#define VLAN2_VID			(0x3)
#define VLAN3_VID			(0x4)
#define VLAN4_VID			(0x5)
#define VLAN5_VID			(0x6)
#define VLAN6_VID			(0x7)
#define VLAN7_VID			(0x8)
#endif
#else
#define VLAN0_VID			(0x111)
#define VLAN1_VID			(0x222)
#define VLAN2_VID			(0x333)
#define VLAN3_VID			(0x444)
#define VLAN4_VID			(0x555)
#define VLAN5_VID			(0x666)
#define VLAN6_VID			(0x777)
#define VLAN7_VID			(0x888)
#endif

#define PORT0				(1 << 0)	/* bit map : bit 0 */
#define PORT1				(1 << 1)	/* bit map : bit 1 */
#define CPU_PORT			(1 << 2)	/* bit map : bit 2 */

#ifdef CONFIG_STAR_GSW_TYPE_ONEARM
#define VLAN0_GROUP			(PORT0 | CPU_PORT)
#define VLAN1_GROUP			(PORT0 | CPU_PORT)
#ifdef CONFIG_STAR_GSW_TYPE_EWC
#define VLAN2_GROUP			(PORT1 | CPU_PORT)
#define VLAN3_GROUP			(PORT1 | CPU_PORT)
#else
#define VLAN2_GROUP			(0)
#define VLAN3_GROUP			(0)
#endif
#define VLAN4_GROUP			(0)
#define VLAN5_GROUP			(0)
#define VLAN6_GROUP			(0)
#ifdef CONFIG_STAR_GSW_TYPE_EWC
#define VLAN7_GROUP			(PORT1 | CPU_PORT)
#else
#define VLAN7_GROUP			(0)
#endif

#else
#define VLAN0_GROUP			(PORT0 | PORT1 | CPU_PORT)
#define VLAN1_GROUP			(PORT0 | CPU_PORT)
#define VLAN2_GROUP			(PORT1 | CPU_PORT)
#define VLAN3_GROUP			(0)
#define VLAN4_GROUP			(0)
#define VLAN5_GROUP			(0)
#define VLAN6_GROUP			(0)
#define VLAN7_GROUP			(0)
#endif

#ifdef CONFIG_STAR_GSW_TYPE_ONEARM

#ifdef CONFIG_HAVE_VLAN_TAG

#define VLAN0_VLAN_TAG			(5)	// cpu port and mac 0 port
#define VLAN1_VLAN_TAG			(5)	// cpu port and mac 0 port

#else
#define VLAN0_VLAN_TAG			(1)	// only mac 0 port
#define VLAN1_VLAN_TAG			(1)	// only mac 0 port
#endif

#define VLAN2_VLAN_TAG			(0)
#define VLAN3_VLAN_TAG			(0)
#define VLAN4_VLAN_TAG			(0)
#define VLAN5_VLAN_TAG			(0)
#define VLAN6_VLAN_TAG			(0)
#ifdef CONFIG_STAR_GSW_TYPE_EWC
#define VLAN7_VLAN_TAG			(0)	// MAC1 don't need Tag
#else
#define VLAN7_VLAN_TAG			(0)
#endif
#else
#define VLAN0_VLAN_TAG			(0)
#define VLAN1_VLAN_TAG			(0)
#define VLAN2_VLAN_TAG			(0)
#define VLAN3_VLAN_TAG			(0)
#define VLAN4_VLAN_TAG			(0)
#define VLAN5_VLAN_TAG			(0)
#define VLAN6_VLAN_TAG			(0)
#define VLAN7_VLAN_TAG			(0)
#endif


#define PORT0_PVID			(VLAN1_GROUP_ID)
#define PORT1_PVID			(VLAN2_GROUP_ID)
#define CPU_PORT_PVID			(VLAN0_GROUP_ID)

#endif

#define MAX_PACKET_LEN			(1536)

/* 
 * Maximum Transmit/Receive Frame Descriptors for GSW's MAC frame
 */
#ifdef FREE_TX_SKB_MULTI
#define STAR_GSW_MAX_TFD_NUM		256
#define STAR_GSW_MAX_RFD_NUM		256
#else
#define STAR_GSW_MAX_TFD_NUM		40
#define STAR_GSW_MAX_RFD_NUM		48
#endif

#endif /* __STAR_GSW_CONFIG_H__ */
