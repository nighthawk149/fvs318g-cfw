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

#include <linux/stddef.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <linux/proc_fs.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/capability.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/pkt_sched.h>
#include <linux/list.h>
#include <linux/reboot.h>
#ifdef NETIF_F_TSO
#include <net/checksum.h>
#endif
#ifdef SIOCGMIIPHY
#include <linux/mii.h>
#endif
#ifdef SIOCETHTOOL
#include <linux/ethtool.h>
#endif
#ifdef NETIF_F_HW_VLAN_TX
#include <linux/if_vlan.h>
#endif

static struct net_device *fast_bridge_dev1;
static struct net_device *fast_bridge_dev2;
static int fast_bridge_dev1_ready;
static int fast_bridge_dev2_ready;

static struct proc_dir_entry *fast_bridge_proc_entry;
int fast_bridge_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data);
int fast_bridge_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);

#ifdef CONFIG_CPU_ISPAD_ENABLE
__attribute__((section(".ispad"))) \
int fast_bridge_forward_skb(struct sk_buff *skb)
#else
int fast_bridge_forward_skb(struct sk_buff *skb)
#endif
{
#if 0
	skb->h.raw = skb->nh.raw = skb->data;
	skb->mac_len = skb->nh.raw - skb->mac.raw;
#endif

	if (!fast_bridge_dev1_ready || !fast_bridge_dev2_ready) {
#if 0
		kfree_skb(skb);
#endif
		return -1;
	}

	if (skb->dev != fast_bridge_dev1 && skb->dev != fast_bridge_dev2) {
		return -1;
	}

	if (skb->dev == fast_bridge_dev1) {
		skb->dev = fast_bridge_dev2;
	} else if (skb->dev == fast_bridge_dev2) {
		skb->dev = fast_bridge_dev1;
	}
	skb->ip_summed = CHECKSUM_NONE;
	skb_push(skb, ETH_HLEN);
#if 0
	dev_queue_xmit(skb);
#else
	skb->dev->hard_start_xmit(skb, skb->dev);
#endif

	return 0;
}

int fast_bridge_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	return 0;
}

int fast_bridge_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char *str;
	char *dev1;
	char *dev2;

	if (count > 0) {
		str = (char *)buffer;
		dev1 = strsep(&str, "\t \n");
		if (!dev1) goto out;
		dev2 = strsep(&str, "\t \n");
		if (!dev2) goto out;
		fast_bridge_dev1 = dev_get_by_name(dev1);
		if (!fast_bridge_dev1) goto out;

		fast_bridge_dev2 = dev_get_by_name(dev2);
		if (!fast_bridge_dev2) {
			dev_put(fast_bridge_dev1);
			goto out;
		}
		rtnl_shlock();
		if (!(fast_bridge_dev1->flags & IFF_UP)) {
			dev_open(fast_bridge_dev1);
		}
		if (!(fast_bridge_dev2->flags & IFF_UP)) {
			dev_open(fast_bridge_dev2);
		}
		if (!(fast_bridge_dev1->flags & IFF_PROMISC)) {
			dev_set_promiscuity(fast_bridge_dev1, 1);
		}
		if (!(fast_bridge_dev2->flags & IFF_PROMISC)) {
			dev_set_promiscuity(fast_bridge_dev2, 1);
		}
		fast_bridge_dev1_ready = 1;
		fast_bridge_dev2_ready = 1;
		rtnl_shunlock();
	}

	return count;

out:
	return -EFAULT;
}

static void fast_bridge_proc_init(void)
{
	fast_bridge_proc_entry = create_proc_entry("fast_bridge", S_IFREG | S_IRUGO | S_IWUSR, NULL);
	if (fast_bridge_proc_entry) {
		fast_bridge_proc_entry->read_proc = fast_bridge_read_proc;
		fast_bridge_proc_entry->write_proc = fast_bridge_write_proc;
		fast_bridge_proc_entry->data = NULL;
	}
}

static int __init fast_bridge_init_module(void)
{
	fast_bridge_proc_init();
	return 0;
}

static void __exit fast_bridge_exit_module(void)
{

}

module_init(fast_bridge_init_module);
module_exit(fast_bridge_exit_module);

MODULE_AUTHOR("KC Huang");
MODULE_DESCRIPTION("FAST SIMPLE BRIDGE");
MODULE_LICENSE("GPL");

