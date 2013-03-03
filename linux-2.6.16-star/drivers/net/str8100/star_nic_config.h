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

#ifndef __STAR_NIC_CONFIG_H__
#define __STAR_NIC_CONFIG_H__

#if 1
#define DBG_PRINT printk
#else
#define DBG_PRINT(arg...)
#endif

#if 0
#define DO_PRINT printk
#define STAR_NIC_PRINT_ISR_STATUS
#else
#define DO_PRINT(arg...)
#endif /* __DEBUG_PRINT_OUT */

// VSC8601 and WavePlus Phy are the same
#define STAR_NIC_PHY_ADDR	0

#define CONFIG_STAR_NIC_NAPI
//#define FREE_TX_SKB_MULTI		// FIXME: define this will cause samba fail

#define STAR_NIC_TX_HW_CHECKSUM
#define STAR_NIC_RX_HW_CHECKSUM

#define STAR_NIC_SG

#if defined(STAR_NIC_SG) && !defined(STAR_NIC_TX_HW_CHECKSUM)
#define STAR_NIC_TX_HW_CHECKSUM
#endif

#define STAR_NIC_STATUS_ISR
#define STAR_NIC_RXQF_ISR

//#ifndef CONFIG_STAR_NIC_NAPI
#define STAR_NIC_DELAYED_INTERRUPT
//#endif

#define MAX_PEND_INT_CNT	0x20
#define MAX_PEND_TIME		0x20

//#ifdef CONFIG_STAR_NIC_PHY_VSC8601
//#define CONFIG_STAR_JUMBO
//#endif
//#define CONFIG_STAR_JUMBO
#ifdef CONFIG_STAR_JUMBO
#define MAX_PACKET_LEN		(2038)
//#define MAX_PACKET_LEN		(9038)
#else
#define MAX_PACKET_LEN		(1536)
#endif
/* This constant(PKT_MIN_SIZE) can be replaced with ETH_ZLEN defined in
 * include/linux/if_ether.h */
#define PKT_MIN_SIZE		60

//#define STAR_NIC_TIMER

/* 
 * Maximum Transmit/Receive Frame Descriptors for NIC's MAC frame
 */
#ifdef FREE_TX_SKB_MULTI
#define STAR_NIC_MAX_TFD_NUM	48
#define STAR_NIC_MAX_RFD_NUM	256
#else
#define STAR_NIC_MAX_TFD_NUM	48		// FIXME: original 64 will cause UDP fail
#define STAR_NIC_MAX_RFD_NUM	64
#endif

#endif /* __STAR_NIC_CONFIG_H__ */

