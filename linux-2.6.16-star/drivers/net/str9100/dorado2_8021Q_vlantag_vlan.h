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

#ifndef  DORADO2_H
#define  DORADO2_H

#include <linux/types.h>

// this configure is for star dorado2

// add by descent 2006/07/10
#define DORADO2

// add by KC 2006/09/07
#ifdef CONFIG_STAR9100_SHNAT_PCI_FASTPATH
// if no VSC8201 on MAC PORT1, we need to define this
// if VSC8201 is present, mark it
//#define DORADO2_PCI_FASTPATH_MAC_PORT1_LOOPBACK
#endif

#ifdef DORADO2
// init phy or switch chip
#define INIT_PORT0_PHY star_gsw_config_VSC7385();
#ifdef DORADO2_PCI_FASTPATH_MAC_PORT1_LOOPBACK
#define INIT_PORT1_PHY star_gsw_config_mac_port1_loopback();
#else
#define INIT_PORT1_PHY star_gsw_config_VSC8201(1,1);
#endif
//#define INIT_PORT1_PHY 

// configure mac0/mac1 register
#define INIT_PORT0_MAC init_packet_forward(0);
#define INIT_PORT1_MAC init_packet_forward(1);
//#define INIT_PORT1_MAC 

#define PORT0_LINK_DOWN disable_AN(0, 0);
#define PORT0_LINK_UP disable_AN(0, 1);

#ifdef DORADO2_PCI_FASTPATH_MAC_PORT1_LOOPBACK
#define PORT1_LINK_DOWN
#define PORT1_LINK_UP
#else
#define PORT1_LINK_DOWN std_phy_power_down(1, 1);
#define PORT1_LINK_UP std_phy_power_down(1, 0);
#endif

#define CREATE_NET_DEV0 star_gsw_probe(LAN_PORT);
#define CREATE_NET_DEV1 star_gsw_probe(WAN_PORT);
#define CREATE_NET_DEV2 star_gsw_probe(EWC_PORT);
//#define CREATE_NET_DEV2 

#define CONFIG_STR9100_PORT_BASE
#define CONFIG_STR9100_VLAN_BASE
//#define CONFIG_HAVE_VLAN_TAG

// for port base, port base max is 2 port.
// use in star_gsw_get_rfd_buff().
// if a port no used, define to "0;"
// NET_DEV0 : rx->sp 0 (port 0)
#define NET_DEV0 0
// NET_DEV1 : rx->sp 1 (port 1)
#define NET_DEV1 STAR_GSW_EWC_DEV

// for star_gsw_send_packet
// port base and vlan base packet flow
#define PORT_BASE_PMAP_LAN_PORT -1
#define PORT_BASE_PMAP_WAN_PORT -1
#define PORT_BASE_PMAP_EWC_PORT 2 // 2 port 1

#define MODEL "DORADO2"

// OPEN_PORT0 include 2 actions
// 1. enable mac port
// 2. link up port
#define OPEN_PORT(dev) \
{ \
        u32 mac_port_config; \
 \
	if (dev == STAR_GSW_EWC_DEV) { \
		memcpy(dev->dev_addr, star_gsw_info.vlan[2].vlan_mac, 6);\
		PRINT_INFO("open mac port1\n"); \
		mac_port_config = GSW_MAC_PORT_1_CONFIG; \
		/* disable port 1 */  \
		mac_port_config &= (~(0x1 << 18)); \
		GSW_MAC_PORT_1_CONFIG = mac_port_config; \
		PORT1_LINK_UP \
	} \
	if (dev == STAR_GSW_LAN_DEV || dev == STAR_GSW_WAN_DEV) { \
		if (dev == STAR_GSW_LAN_DEV) \
			memcpy(dev->dev_addr, star_gsw_info.vlan[1].vlan_mac, 6);\
		if (dev == STAR_GSW_WAN_DEV) \
			memcpy(dev->dev_addr, star_gsw_info.vlan[0].vlan_mac, 6);\
		/* rc_port is a reference count variable. */ \
	        if (rc_port == 0) {\
        		PRINT_INFO("open mac port 0\n");\
		        mac_port_config = GSW_MAC_PORT_0_CONFIG;\
               		/* enable port 0 */ \
		        mac_port_config &= (~(0x1 << 18));\
               		GSW_MAC_PORT_0_CONFIG = mac_port_config;\
			PORT0_LINK_UP\
	        }\
	        else{\
      			PRINT_INFO("port 0 already open\n");\
	        }\
		++rc_port;\
	} \
}

// CLOSE_PORT include 2 actions
// 1. disable mac port
// 2. link down port
#define CLOSE_PORT(dev) \
{ \
        u32 mac_port_config; \
 \
	if (dev == STAR_GSW_EWC_DEV) { \
		PRINT_INFO("close mac port1\n"); \
		PORT1_LINK_DOWN \
		mac_port_config = GSW_MAC_PORT_1_CONFIG; \
		/* disable port 1 */  \
		mac_port_config |= ((0x1 << 18)); \
		GSW_MAC_PORT_1_CONFIG = mac_port_config; \
	} \
	if (dev == STAR_GSW_LAN_DEV || dev == STAR_GSW_WAN_DEV) { \
		--rc_port;\
		/* rc_port is a reference count variable. */ \
	        if (rc_port == 0) {\
        		PRINT_INFO("close mac port 0\n");\
			PORT0_LINK_DOWN\
		        mac_port_config = GSW_MAC_PORT_0_CONFIG;\
               		/* disable port 0 */ \
		        mac_port_config |= ((0x1 << 18));\
               		 GSW_MAC_PORT_0_CONFIG = mac_port_config;\
	        }\
	        else {\
      			PRINT_INFO("a live net device\n");\
	        }\
	} \
}



#define VLAN0_VID			(0x2)
#define VLAN1_VID			(0x1)
#define VLAN2_VID			(0x3)
#define VLAN3_VID			(0x4)
#define VLAN4_VID			(0x5)
#define VLAN5_VID			(0x6)
#define VLAN6_VID			(0x7)
#define VLAN7_VID			(0x8)

#define VLAN0_GROUP			(PORT0 | CPU_PORT)
#define VLAN1_GROUP			(PORT0 | CPU_PORT)
#define VLAN2_GROUP			(PORT1 | CPU_PORT)
#define VLAN3_GROUP			(PORT1 | CPU_PORT)
#define VLAN4_GROUP			(0)
#define VLAN5_GROUP			(0)
#define VLAN6_GROUP			(0)
#define VLAN7_GROUP			(PORT1 | CPU_PORT)


#define CONFIG_VLANTAG_VLAN

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
#define VLAN7_VLAN_TAG			(0)


/* wan eth1 */
static u8 my_vlan0_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x50};

/* lan eth 0*/
static u8 my_vlan1_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x60};

/* cpu */
static u8 my_vlan2_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x22};

/* ewc  */
static u8 my_vlan3_mac[6] = {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0x23};

// this value is for hnat
// GID is vlan group id
#define LAN_GID 1
#define WAN_GID 0


#endif //DORADO2




#endif
