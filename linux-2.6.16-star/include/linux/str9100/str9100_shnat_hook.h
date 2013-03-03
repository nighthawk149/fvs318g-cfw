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


#ifndef __STR9100_SHNAT_HOOK_H__
#define __STR9100_SHNAT_HOOK_H__

#include <linux/config.h>

#ifdef CONFIG_NETFILTER

#if defined(CONFIG_STR9100_SHNAT) || defined(CONFIG_STR9100_SHNAT_MODULE)
#include <linux/str9100/star9100_shnat.h>

#define MAX_FP_PCIDEV 8
extern struct net_device *pci_netdev[MAX_FP_PCIDEV];
extern int (*star9100_shnat_pci_fp_forward_skb_ptr)(struct sk_buff *skb);

#define HNAT_SOURCE_PORT_BASE 50000

#define DUMP_TUPLE2(tp) \
	printk("tuple %p: %u %u.%u.%u.%u:%hu -> %u.%u.%u.%u:%hu\n", \
		(tp), (tp)->dst.protonum, \
		NIPQUAD((tp)->src.ip), ntohs((tp)->src.u.all), \
		NIPQUAD((tp)->dst.ip), ntohs((tp)->dst.u.all))

extern int star9100_shnat_hook_ready;
extern int (*star9100_shnat_preadd_hnatable_hook)(u32 sip,u16 sport,u16 dport, u32 proto);
extern int (*star9100_shnat_check_shnat_enable_hook)(void);
extern int (*star9100_shnat_nf_nat_preadd_hnatable_hook)(const struct ip_conntrack *ct, int dir, const u16 port);
extern int (*star9100_shnat_nf_remove_hnatable_hook)(struct ip_conntrack *);
extern int (*star9100_shnat_nf_add_hnatable_hook)(const struct ip_conntrack *ct,const struct iphdr *iph, u16 proto);
extern int (*star9100_shnat_add_arptable_hook)(u32 myip, u32 targetip);
extern int (*star9100_shnat_fix_arptable_hook)(u32 myip, u32 targetip);
extern int (*star9100_shnat_nf_preadd_hnatable_hook)(const struct sk_buff **pskb);
extern int (*star9100_shnat_check_ftponly_enable_hook)(void);
extern int (*star9100_shnat_pci_fp_getdev_hook)(struct sk_buff *skb_ptr);
extern star9100_arp_table *(*star9100_shnat_getarptable_hook)( u32 ip_addr);
extern void (*shnat_api_hook)(shnat_input *input, shnat_return *ret);
#endif /* defined(CONFIG_STR9100_SHNAT) || defined(CONFIG_STR9100_SHNAT_MODULE) */


#endif /* CONFIG_NETFILTER */

#endif /* __STR9100_SHNAT_HOOK_H__ */
