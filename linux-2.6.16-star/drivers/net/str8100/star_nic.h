/*******************************************************************************
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

#ifndef STAR_NIC_H
#define STAR_NIC_H

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
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/bitops.h>
#include <asm/irq.h>		// 2006.03.22 richliu add list include file
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

#include <asm/arch/star_nic.h>

#include "star_nic_config.h"

#define LAN_PORT 1

typedef struct
{
	u32		mib_rx_ok_pkt;
	u32		mib_rx_ok_byte;
	u32		mib_rx_runt;
	u32		mib_rx_over_size;
	u32		mib_rx_no_buffer_drop;
	u32		mib_rx_crc_err;
	u32		mib_rx_arl_drop;
	u32		mib_rx_myvid_drop;
	u32		mib_rx_csum_err;
	u32		mib_rx_pause_frame;
	u32		mib_tx_ok_pkt;
	u32		mib_tx_ok_byte;
	u32		mib_tx_pause_frame;
} mib_info_t;

/* store this information for the driver.. */
struct star_nic_private {
	struct net_device_stats stats;
	spinlock_t lock;
	int dev_index;
	u8 phy_addr;
	u16 phy_id;
	mib_info_t mib_info;
};

/*
 * Network Driver, Receive/Send and Initial Buffer Function
 */
typedef struct {
	// 1st 32Bits
	u32 data_ptr;

	// 2nd  32Bits
	u32 length:16;
	u32 reserved0:7;
	u32 tco:1;
	u32 uco:1;
	u32 ico:1;
	u32 insv:1;
	u32 intr:1;
	u32 ls:1;
	u32 fs:1;
	u32 eor:1;
	u32 cown:1;

	// 3rd 32Bits
	u32 vid:12;
	u32 cfi:1;
	u32 pri:3;
	u32 epid:16;

	// 4th 32Bits
	u32 reserved1;
} __attribute__((packed)) STAR_NIC_TXDESC;

typedef struct {
	// 1st 32Bits
	u32 data_ptr;

	// 2nd  32Bits
	u32 length:16;
	u32 l4f:1;
	u32 ipf:1;
	u32 prot:2;
	u32 vted:1;
	u32 mymac:1;
	u32 hhit:1;
	u32 rmc:1;
	u32 crce:1;
	u32 osize:1;
	u32 reserved0:2;
	u32 ls:1;
	u32 fs:1;
	u32 eor:1;
	u32 cown:1;

	// 3rd 32Bits
	u32 vid:12;
	u32 cfi:1;
	u32 pri:3;
	u32 epid:16;

	// 4th 32Bits
	u32 reserved1;
} __attribute__((packed)) STAR_NIC_RXDESC;

/* 
 * Transmit Frame Descriptor Ring for TFDS
 */
typedef struct {
	u32			phy_addr;
	STAR_NIC_TXDESC		*vir_addr;
	u32			cur_index; // TX's current will point to Free Descriptors
#if defined(FREE_TX_SKB_MULTI) || defined(STAR_NIC_TIMER)
	u32			to_free_index;
#endif
	struct sk_buff		*skb_ptr[STAR_NIC_MAX_TFD_NUM]; // TX's sk_buff ptr
} TXRING_INFO;

/* 
 * Receive Frame Descriptor Ring for RFDS
 */
typedef struct {
	u32			phy_addr;
	STAR_NIC_RXDESC		*vir_addr;
	u32			cur_index;
	struct sk_buff		*skb_ptr[STAR_NIC_MAX_RFD_NUM];	// RX's sk_buff ptr
} RXRING_INFO;

#endif

