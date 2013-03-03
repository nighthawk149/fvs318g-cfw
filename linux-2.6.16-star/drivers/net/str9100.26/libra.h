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

#ifndef  LIBRA_H
#define  LIBRA_H


// this configure is for star LIBRA

// add by descent 2006/07/10
#define LIBRA
#ifdef LIBRA
// init phy or switch chip
#define INIT_PORT0_PHY configure_icplus_175c_phy();
#define INIT_PORT1_PHY 
//#define INIT_PORT1_PHY 

// configure mac0/mac1 register
#define INIT_PORT0_MAC init_packet_forward(0);
#define INIT_PORT1_MAC 
//#define INIT_PORT1_MAC 

#define PORT0_LINK_DOWN icp_175c_all_phy_power_down(1);
#define PORT0_LINK_UP icp_175c_all_phy_power_down(0);

#define PORT1_LINK_DOWN 
#define PORT1_LINK_UP 

#define CREATE_NET_DEV0 star_gsw_probe(LAN_PORT);
#define CREATE_NET_DEV1 star_gsw_probe(WAN_PORT);
#define CREATE_NET_DEV2 
//#define CREATE_NET_DEV2 

#undef CONFIG_STR9100_PORT_BASE
#define CONFIG_STR9100_VLAN_BASE
#undef CONFIG_HAVE_VLAN_TAG
#define CONFIG_HAVE_VLAN_TAG


// for star_gsw_send_packet
// port base and vlan base packet flow
#define PORT_BASE_PMAP_LAN_PORT INVALID_PORT_BASE_PMAP_PORT
#define PORT_BASE_PMAP_WAN_PORT INVALID_PORT_BASE_PMAP_PORT
#define PORT_BASE_PMAP_EWC_PORT INVALID_PORT_BASE_PMAP_PORT

#define MODEL "LIBRA"

// OPEN_PORT0 include 2 actions
// 1. enable mac port
// 2. link up port
#define OPEN_PORT(dev) \
{ \
        u32 mac_port_config; \
	if (dev == STAR_GSW_LAN_DEV || dev == STAR_GSW_WAN_DEV) { \
		if (dev == STAR_GSW_LAN_DEV) \
			memcpy(dev->dev_addr, star_gsw_info.vlan[LAN_GID].vlan_mac, 6); \
		if (dev == STAR_GSW_WAN_DEV) \
			memcpy(dev->dev_addr, star_gsw_info.vlan[WAN_GID].vlan_mac, 6); \
		/* rc_port is a reference count variable. */ \
	        if (rc_port == 0) { \
        		PRINT_INFO("open mac port 0\n"); \
		        mac_port_config = GSW_MAC_PORT_0_CONFIG_REG; \
               		/* enable port 0 */ \
		        mac_port_config &= (~(0x1 << 18)); \
               		GSW_MAC_PORT_0_CONFIG_REG = mac_port_config; \
			PORT0_LINK_UP \
	        } \
	        else{ \
      			PRINT_INFO("port 0 already open\n"); \
	        } \
		++rc_port; \
	} \
}

// CLOSE_PORT include 2 actions
// 1. disable mac port
// 2. link down port
#define CLOSE_PORT(dev) \
{ \
        u32 mac_port_config; \
	if (dev == STAR_GSW_LAN_DEV || dev == STAR_GSW_WAN_DEV) { \
		/* rc_port is a reference count variable. */ \
		--rc_port; \
	        if (rc_port == 0) { \
        		PRINT_INFO("close mac port 0\n"); \
			PORT0_LINK_DOWN \
		        mac_port_config = GSW_MAC_PORT_0_CONFIG_REG; \
               		/* disable port 0 */ \
		        mac_port_config |= ((0x1 << 18)); \
               		 GSW_MAC_PORT_0_CONFIG_REG = mac_port_config; \
	        } \
	        else { \
      			PRINT_INFO("a live net device\n"); \
	        } \
	} \
}



// the vlan past waht vlan tag value
#define VLAN0_VID			(0x2) // wan
#define VLAN1_VID			(0x1) // lan
#define VLAN2_VID			(0x3)
#define VLAN3_VID			(0x4)
#define VLAN4_VID			(0x5)
#define VLAN5_VID			(0x6)
#define VLAN6_VID			(0x7)
#define VLAN7_VID			(0x8)

// the vlan include ports
#define VLAN0_GROUP			(PORT0 | CPU_PORT)
#define VLAN1_GROUP			(PORT0 | CPU_PORT)
#define VLAN2_GROUP			(0)
#define VLAN3_GROUP			(0)
#define VLAN4_GROUP			(0)
#define VLAN5_GROUP			(0)
#define VLAN6_GROUP			(0)
#define VLAN7_GROUP			(0)


#ifdef CONFIG_HAVE_VLAN_TAG

// the vlan which ports will past vlan tags.
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
static u8 my_vlan0_mac[6] = {0x00, 0x11, 0xbb, 0xcc, 0xdd, 0x50};

/* lan eth 0*/
static u8 my_vlan1_mac[6] = {0x00, 0x11, 0xbb, 0xcc, 0xdd, 0x60};

/* cpu */
static u8 my_vlan2_mac[6] = {0x00, 0x11, 0xbb, 0xcc, 0xdd, 0x22};

/* ewc  */
static u8 my_vlan3_mac[6] = {0x00, 0x11, 0xbb, 0xcc, 0xdd, 0x23};

// this value is for hnat
// GID is vlan group id
#define LAN_GID 1
#define WAN_GID 0


#endif //LIBRA




#endif
