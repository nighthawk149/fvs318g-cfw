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

#if !defined(_STAR_HNAT_H_) && !defined(_STR9100_SHNAT_H_)
#define _STAR_HNAT_H_

/******************************************************************************
 *        
 * MODULE NAME:    star9100_shnat.h
 * PROJECT CODE:   Orion Smart Hardware NAT
 * DESCRIPTION:    
 * MAINTAINER:     Richard Liu
 * DATE:           11 October 2005
 *
 * SOURCE CONTROL: 
 *
 * LICENSE:
 *     This source code is copyright (c) 2005 Star Semiconduction .
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    11 October 2005  -  Richard Liu	- Initial Version v1.0
 *
 ******************************************************************************/

#include <linux/netdevice.h>
#include <linux/etherdevice.h> 
#include <linux/spinlock.h>
#include <linux/types.h>
                                                                                                                             
#include <asm/arch/star_gsw.h>

#if 1
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#endif

#if defined(CONFIG_STAR9100_GSW1000)
#include "star9100_gsw.h"
#elif defined(CONFIG_STAR9100_GSW1000_NEW)
#include "star9100_gsw_new.h"
#endif
                                                                                                                             
#ifndef u32
#define u32 u32
#endif

#ifndef u8
#define u8 u8
#endif

#ifndef u16
#define u16 u16
#endif

#ifndef LAN_PORT
#define LAN_PORT 1
#endif

#ifndef WAN_PORT
#define WAN_PORT 0
#endif


#if 0
typedef struct _dst_mac_struct_    dst_mac_struct_t;

struct _dst_mac_struct_
{
    unsigned int    dmac_high;    
    unsigned int    dmac_low;    
};

typedef struct _vlan_smac_struct_    vlan_smac_struct_t;

struct _vlan_smac_struct_
{
    unsigned char    vlan_src_mac[6];
    unsigned char    pad[2];
};

typedef struct _session_id_struct_    session_id_struct_t;

struct _session_id_struct_
{
    unsigned short    session_id;
    unsigned short    pad;
};

#endif


typedef struct _sip_table_
{
    u32    start_index;
    u32    end_index; /*sip table does not have a end index as data sheet said. david @Sep 9th*/
    u32    current_index;
    u32    entry_num;
} sip_table_t;

typedef struct _alg_table_
{
    u32    start_index;
    u32    end_index;
    u32    current_index;
    u32    entry_num;
} alg_table_t;

typedef struct star9100_shnat_pppoe{
	u32		remote_ip;				// PPPoE's Remote IP
	u32		local_ip;					// PPPoE's Local IP
	u8		remote_mac[6];	// Remote Mac's MAC 
	u16		session_id;				//	PPPoE Session ID
	u8		sid;						//	HNAT sid table index
}star9100_shnat_pppoe;

typedef struct star9100_shnat_vserver{
	u32 wanip;
	u32	lanip;
	u16 port;			// Virtual Server 's PORT
							// If port ==0, it will forward while port 
	u16 eport;			// Virtual Server's EndPort.
	u8	proto; 		// 1=TCP, 2=UDP 0=ALL
	u8	sip_ptr;			// SIPid
}star9100_shnat_vserver;
 
typedef struct star9100_shnat_wanip{
	u32 ip;
	u8	sip_ptr;
}star9100_shnat_wanip;

#define MAX_VSERVER 20

#define MAX_WANIP	10

typedef struct star9100_hnat_entity
{
    unsigned int           	hnat_config;
    unsigned int           	private_ip_base;
    //dst_mac_struct_t  		dst_mac_entry[4];
    //vlan_smac_struct_t		vlan_src_mac_entry[8];
    //session_id_struct_t   	session_id_entry[8];
    //fw_rule_table_t        	fw_rule_table;
    //fl_rule_table_t        	fl_rule_table;
    alg_table_t            		alg_table;
    sip_table_t            		*sip_table;
	u16						sip_table_index;	// Index for sip_table

	star9100_shnat_pppoe		pppoe[8];		// Max 8 Sessions

	star9100_shnat_vserver	vserver[MAX_VSERVER];	 
	star9100_shnat_wanip	wanip[MAX_WANIP];		

	u32					arptable_hash_num;		// NAPT_TABLE Number,1=1K,2=2K,4=4K
	u32					naptable_hash_num;		// ARP Table Number, 1=1K,2=2K,4=4K

	u32					siptable_max_value;
	u32					algtable_max_value;

	unsigned int          	napt_table_base_addr;
	unsigned int		   	napt_table_size;		
	unsigned int          	napt_src_port_base;  
    unsigned int          	arp_table_base_addr;
    unsigned int          	tcp_mss_limit;
    unsigned int          	aging_config;

	unsigned int			napt_table_vir_addr; // System Alloc Memory
	unsigned int			napt_table_phy_addr; // Physical Memory Address
	unsigned int			arp_hash_vir_addr;   
	unsigned int			arp_hash_phy_addr; 
	int						enable;						// HNAT Enable		, 1=enable, 0=disable
	int						delayactive_enable;		// delayacrive_enable , 1=enable, 0=disable
	int						ftponly_enable;			//
	int						udp_disable;			// Disable UDP HNAT 1=disable 0=enable

	int						ao_unit;				// AgingOut Unit 0-16, 
													// 0=no aging out, but we don't use this value.
													// Default 4, will define in module init
	int						ao_tcp;					// TCP Aging Out Timer 0-7, 0 not use.
													// Default 7
	int						ao_udp;					// UDP Aging Out Timer 0-7, 0 not use.
													// Default 1
	int 					ao_arp;					// ARP Aging Out Timer 0-7, 0 not use.
													// Default 7
	
//	struct net_device *landev;
//	struct net_device *wandev;
}star9100_hnat_entity;

#if 0
typedef struct _hnat_dummy_rule_entry_
{
    u32    rule_field;
    u32    action_field;
    
} hnat_dummy_rule_entry_t;

typedef struct _hnat_hnat_arp_entry_
{
    u32		ip_addr;
    u32		info1;
    u16		info2;
    u16		mac_0;
    u32		mac_1;    
} hnat_hnat_arp_entry_t;

typedef struct _hnat_hnat_napt_entry_
{
    u32		info0;
    u16		sip_int;
    u16		sp_int;  
} hnat_hnat_napt_entry_t;
#endif

#define ARP_HASH_XOR		1
#define ARP_HASH_LINEAR		0

#define WVMAP_VLAN0	(0x1 << 16) 
#define WVMAP_VLAN1	(0x1 << 17) 
#define WVMAP_VLAN2	(0x1 << 18) 
#define WVMAP_VLAN3	(0x1 << 19) 
#define WVMAP_VLAN4	(0x1 << 20) 
#define WVMAP_VLAN5	(0x1 << 21) 
#define WVMAP_VLAN6	(0x1 << 22) 
#define WVMAP_VLAN7	(0x1 << 23) 

#define WPMAP_PORT0	(0x1 << 8)
#define WPMAP_PORT1	(0x1 << 9)
#define WPMAP_CPU	(0x1 << 10)

#define TABLE_1K	0x1
#define TABLE_2K	0x2
#define TABLE_4K	0x4

#define AGING_TIME_UNIT	0
#define AGING_TCP_FIN	1
#define AGING_TCP		2
#define AGING_UDP		3
#define AGING_ARP		4

#define AGING_ENABLE	1
#define AGING_DISABLE	0

#define FROMLAN_RULE	0
#define FROMWAN_RULE	1

#define SHOW_HNAT_REGISTER		0x0001
#define SHOW_FL_TABLE			0x0002
#define SHOW_FW_TABLE			0x0004
#define SHOW_ALG_TABLE			0x0008
#define SHOW_SIP_TABLE			0x0010

#define HNAT_FW_RULE_TABLE_START_ENTRY_INDEX (0)
#define HNAT_FW_RULE_TABLE_STOP_ENTRY_INDEX  (40)   // this entry in exclusive
#define HNAT_FL_RULE_TABLE_START_ENTRY_INDEX (60)
#define HNAT_FL_RULE_TABLE_STOP_ENTRY_INDEX  (100)  // this entry in exclusive
#define HNAT_ALG_TABLE_START_ENTRY_INDEX     (110)
#define HNAT_ALG_TABLE_STOP_ENTRY_INDEX      (130)  // this entry in exclusive
#define HNAT_SIP_TABLE_START_ENTRY_INDEX     (140)
#define HNAT_SIP_TABLE_STOP_ENTRY_INDEX      (HNAT_SIP_TABLE_START_ENTRY_INDEX + 256)  // this entry in exclusive


#define HNAT_SOURCE_PORT_BASE	50000


#if 0
struct star9100_hnat_table {
	u32	valid:1;
	u32	ac:3;
	u32	statics:1;
	u32	rst:1;
	u32	lf:1;
	u32	wf:1;
	u32	unused1:8;
	u32	unused2:5;
	u32	tu:1;
	u32	drop:1;
	u32	cpu:1;
	u32	sip_ptr:8;
	u32	unused3:4;
	u32	sip_int:12;
	u32	sp_int:16;
}star9100_hnat_table;
#else
typedef struct star9100_hnat_table {
			// First 32Bits
            u32  sip_ptr:8;
            u32  cpu:1;
            u32  drop:1;
            u32  tu:1;
            u32  unused2:5;
            u32  unused1:8;
            u32  wf:1;
            u32  lf:1;
            u32  rst:1;
            u32  statics:1;
            u32  ac:3;
            u32  valid:1;
			// Second 32Bits
            u32  sp_int:16;
            u32  sip_int:12;
            u32  unused3:4;
}star9100_hnat_table;

typedef struct star9100_alg_table{
			u16	dst_end;
			u16	dst_start;
			u16	src_end;
			u16	src_start;
}star9100_alg_table;

typedef struct _star9100_arp_table{
			// 1st
			u32	ip_addr;
			// 2nd
			u32	unused:27;
			u32	s:1;
			u32	ac:3;
			u32  valid:1;
			// 3rd
			u32	mac4732:16;
			u32	gid:3;
			u32	unused2:1;
			u32	pm:3;
			u32	unused3:2;
			u32	lp:1;
			u32	fr:1;
			u32	r:1;
			u32	unused4:4;
			// 4nd
			u32	mac3100;

}star9100_arp_table;


typedef struct star9100_shnat_ruletable{
	// First 32bits
	u32		fmask:4;
	u32		sarf:4;
	u32		tcp:1;
	u32		udp:1;
	u32		pn:2;
	u32		unused:2;
	u32		ip2mask:5;
	u32		ip1mask:5;
	u32		op:4;
	u32		rt:2;
	u32		ifm:1;
	u32		valid:1;

	// Second 32bits
	u32		dscp:6;
	u32		rd:1;
	u32		p:2;
	u32		fp:1;
	u32		unused3:1;
	u32		mss:1;
	u32		dmac:2;
	u32		pr:1;
	u32		pd:1;
	u32		larp:1;
	u32		alg:1;
#if 0
	union {
		u32 asfield:12;
		struct {
		u32		dnapt:1;
		u32		lsnapt:1;
		u32		dudnapt:1;
		u32 	unused2:1;
		u32 	sip_ptr:8;
		}as0;
		struct{
			u32	dnapt:1;
			u32 lsnapt:1;
			u32 dudnapt:1;
			u32 unused6:1;
			u32 sipptr:8;
		}as1;
		struct{
		u32 sip_ptr:8;
		u32 unused5:4;
		}as2;
		struct{
		u32		dip_int:12;
		}as3;
	}asall;
#else
		u32		asall:12;
#endif
		u32 	as:2;

	//
	u32		sip;
	u32		sport_end:16;
	u32		sport_start:16;

	u32		dip;
	u32		dport_end:16;
	u32		dport_start:16;


}star9100_shnat_ruletable;

#endif
#if 0
struct star9100_sip_table {
	u32 unused1:15;
	u32 pi:1;
	u32 unused2:13;
	u32 pppoe_sid:3;
	u32 sipaddr;
}star9100_sip_table;
#else
typedef struct star9100_sip_table {
	// First 32Bits
	u32 pppoe_sid:3;
	u32 unused2:13;
	u32 pi:1;
	u32 unused1:15;
	// Second 32Bits
	u32 sipaddr;
}star9100_sip_table;

typedef struct star9100_shnat_forcetocpu {
	u32 dip;        // Destination IP
	u32 dip_mask;   // Destination IP MASK
	u32 sip;        // Source IP
	u32 sip_mask;   // Source IP MASK
	u16 ssport;     // Source Start Port
	u16 seport;     // Source End Port
	u16 dsport;     // Destination Start Port
	u16 deport;     // Destination End Port
	u8 hr;
	u8 tcp;
	u8 udp;
	u8 sarf;
	u8 fmask;
	u8 rd;
	u8 dscp;
	u8 used; // determine the rule if used
	u8 wan;  // if wan ==1 means this rule should in WAN side
				// otherwise in LAN side. Default LAN side.
} star9100_shnat_forcetocpu;
#endif

void star9100_Hnat_Embedded_RAM_Init(void);
u16 star9100_hnat_get_hnattable_hash(u32 sip,u16 sport,unsigned int proto);
void star9100_hnat_write_hnat_table(u32 index,star9100_hnat_table *hnat_table);
void star9100_hnat_read_hnat_table(u32 index,star9100_hnat_table **hnat_table);

//int star9100_hnat_add_siptable(star9100_sip_table *sip_entry);
int star9100_hnat_add_siptable(u32 addr,u16 pi,u16 pppoe_sid);

//int star9100_hnat_user_add(struct iphdr *iph,struct tcphdr *tcph);

/*
int star9100_shnat_search_algtable(unsigned int port);
int star9100_shnat_add_algtable(unsigned int startport,unsigned int endport);
int star9100_shnat_remove_algtable(unsigned int startport,unsigned int endport);
*/
u16 star9100_shnat_get_hash_hnattable(u32 sip,u16 sport,u16 proto);

// Read hnat_table ptr, 
star9100_hnat_table *star9100_shnat_read_hnattable(u32 index);

// preadd will preserve hnattable's port .
int star9100_shnat_preadd_hnatable(u32 sip,u32 sport,u16 dip,u16 dport,u8 proto);
int star9100_shnat_add_hnatable(u32 sipip,u16 hnat_hash, u32 sip,u32 sport,u16 proto);
void star9100_shnat_remove_hnatable(u32 sip,u16 sport,u16 proto);

/*
	return -1: Setting Fail
	other return port (with PORT_BASE)
 */
int star9100_shnat_nf_preadd_hnatable(const struct sk_buff **pskb);
int star9100_shnat_nf_add_hnatable(const struct ip_conntrack *conntrack,const struct iphdr *iph,u16 proto);
int star9100_shnat_nf_remove_hnatable(const struct ip_conntrack *ct);
/* Preadd hnat_table , But it will not active now */
int star9100_shnat_nf_nat_preadd_hnatable(const struct ip_conntrack *ct,int dir,const u16 port);
/* Get port from Preadd hnat_table , But it will not active */
int star9100_shnat_nf_nat_preget_hnatable(const struct ip_conntrack *ct,u16 port);
/**
 *		@param skb	input packet buffer, and enable valid bit when --dport 80 and 
 *								stringHOST issue;
 */
void star9100_shnat_nf_stringhost_valid(const struct sk_buff *skb,int enable);

/* New SIP Table, if Sip available, it will overwrite
*/
int 	star9100_shnat_add_siptable(u32 addr,u16 pi,u16 pppoe_sid);
int 	star9100_shnat_search_siptable(u32 addr);
int 	star9100_shnat_clean_siptable(u32 addr);
void 	star9100_shnat_edit_siptable(int index,u32 addr,u16 pi,u16 pppoe_sid);
star9100_sip_table	*star9100_shnat_read_siptable(int index);

int 	star9100_shnat_add_algtable(u16 src_start,u16 src_end,u16 dst_start,u16 dst_end);
int 	star9100_shnat_search_algtable(u16 port);
int 	star9100_shnat_clean_algtable(u16 src_start,u16 src_end,u16 dst_start,u16 dst_end);


/*	ALG and DelayActive Table Support 
	DelayActive is when a connect active, we won't active the HNAT, until user enable it.
*/
#define SHNAT_SOFTTABLE_MAX	100
typedef struct shnat_softport{
	unsigned short int sport;
	unsigned short int eport;
}shnat_softport;

int star9100_shnat_clean_softtable(shnat_softport *table);
int star9100_shnat_add_softtable(shnat_softport *table,unsigned int sport,unsigned int eport);
int star9100_shnat_del_softtable(shnat_softport *table,unsigned int sport,unsigned int eport);
int star9100_shnat_search_softtable(shnat_softport *table,unsigned int port);

int star9100_shnat_proc_table_parser(const char *read_buff);
/**
  * PPPoE Session Function
  */
int star9100_shnat_addsession(u16 pppoe_sid);
void star9100_shnat_delsession(int index, u16 pppoe_sid);
void star9100_shnat_setsession(int index,u16 session_id);
int star9100_shnat_getfreesession(void);
u16 star9100_shnat_getsession(int index);

void star9100shnat_set_hnatentity(star9100_hnat_entity *my_hnat_entity);
//static void Gsw_Hnat_Write_Destination_MAC(u32 index, hnat_dmac_t *hnat_dmac);


int star9100_shnat_set_privateip(u32 addr);
void star9100_shnat_enable_hnat(void);
void star9100_shnat_disable_hnat(void);
void star9100_shnat_setlandev(struct net_device *landev);
void star9100_shnat_setwandev(struct net_device *landev);

/// ARP Tables relation 
u32 star9100_shnat_getarpindex( u32 ip_addr);
//star9100_shnat_arptable *star9100_shnat_read_arptable(u32 index);
int star9100_shnat_add_arptable(u32 myip,u32 targetip);
int star9100_shnat_fix_arptable(u32 myip,u32 targetip);
/**
 *		@param pip pip(private ip): network order,  if private ip == NULL, it will set last Private IP Again, 
 *   
 *     @return	none
*/
void star9100_shnat_set_privateipbase(u8 *pip);
int star9100_shnat_check_privatelan(u32 addr);


void star9100_shnat_enable_da_table(u32 saddr, u16 sport, int enable);

void star9100_shnat_setaging(void);

/**
 *
 *  Rule Table Management .
 *
 *  Note: Only Add and clean , don't implement delete and insert 
 */
int star9100_shnat_init_ruletable(void);
int star9100_shnat_add_ruletable(star9100_shnat_ruletable *ruletable,int iport);
int star9100_shnat_clean_ruletable(void);

int star9100_shnat_check_shnat_enable(void);
int star9100_shnat_check_ftponly_enable(void);
/** 
  *		Support Function
  */ 
char *mac2asc(unsigned char *sha, unsigned char addr_len);
void getParam(char *buf,char *string);

#define STR9100_SHNAT_API

#ifdef STR9100_SHNAT_API

#define MAX_DEVNAME_LEN 15

typedef enum _shnat_boolean {
	shnat_bool_false = 0,
	shnat_bool_true = 1
} shnat_boolean;


typedef enum _shnat_cmd {
	shant_cmd_string = 0,
	shnat_cmd_enable,
	shnat_cmd_lanip,
	shnat_cmd_wanip,
	shnat_cmd_wancfg,
	shnat_cmd_pppoe,
	shnat_cmd_forcetocpu,
	shnat_cmd_vserver,
	shnat_cmd_interswitch,
	shnat_cmd_arp_table,
	shnat_cmd_napt_table,
	shnat_cmd_alg_table,
	shnat_cmd_da_table,
	shnat_cmd_agingout,
	shnat_cmd_sna_th,       /* SNA threshold*/
	shnat_cmd_pcifptowan,
	shnat_cmd_pcifptolan,
	shnat_cmd_ftponly,
	shnat_cmd_da_enable,
	shnat_cmd_chgdscp,
	shnat_cmd_udp,
	shnat_cmd_smartbit
} shnat_cmd;

typedef enum _shnat_operation {
	shnat_op_get = 0,
	shnat_op_set,
	shnat_op_delete,
	shnat_op_clean
} shnat_op;

typedef enum _shnat_proto {
	shnat_proto_none  = 0,
	shnat_proto_udp  = 1,
	shnat_proto_tcp  = 2
} shnat_proto;

#define SHNAT_INPUT_BUFF_MAX 200

typedef struct shnat_input_t {
    shnat_cmd cmd;
	shnat_op op;

	union {
		struct {
			char buff[SHNAT_INPUT_BUFF_MAX];
		} shnat_string;

		struct {
			shnat_boolean enable;
		} shnat_enable;

		struct {
			u32 ip;
		} shnat_lanip;

		struct {
			u32 index;
			u32 ip;
		} shnat_wanip;

		struct {
			u32 gvid;
			u32 pvid;
			u32 lanpn;
			u32 wanpn;
			u32 wanpm;
			u32 wangid;
			u32 nettype_ot;
			u32 otpn;
		} shnat_wancfg;

		struct {
			u32 index;
			star9100_shnat_pppoe pppoe;
		} shnat_pppoe;

		struct {
			int     index;
			star9100_shnat_forcetocpu forcetocpu;
		} shnat_forcetocpu;

		struct {
			int     index;
			star9100_shnat_vserver vserver;
		} shnat_vserver;

		struct {
			int     index;
			star9100_shnat_forcetocpu interswitch;
		} shnat_interswitch;

		struct {
			u32 static_clean;
			star9100_arp_table arp;
		} shnat_arp_table;

		struct {
		} shnat_napt_table;

		struct {
			u16 sport;
			u16 eport;
		} shnat_alg_table;

		struct {
			u16 sport;
			u16 eport;
		} shnat_da_table;

		struct {
			u8  unit;
			u8  arp;
			u8  tcp;
			u8  udp;
		} shnat_agingout;

		struct {
			u32 threshold;
		} shnat_sna_th;

		struct {
			u8      gvid;
			char    devname[MAX_DEVNAME_LEN];
		} shnat_pcifptowan;

		struct {
			shnat_boolean enable;
		} shnat_pcifptolan;

		struct {
			shnat_boolean enable;
		} shnat_ftponly;

		struct {
			shnat_boolean enable;
		} shnat_da_enable;

		struct {
			shnat_boolean enable;
			u8 lan_dscp;
			u8 wan_dscp;
		} shnat_chgdscp;

		struct {
			shnat_boolean enable;
		} shnat_udp;

		struct {
		} shnat_smartbit;
	} data;
} shnat_input;

typedef enum _shnat_status {
	shnat_status_success = 0,
	shnat_status_cmderror,
	shnat_status_tablefull,
	shnat_status_exceedtablesize,
	shnat_status_devicenotfound,
	shnat_status_ipnotfound,
	shnat_status_exceedpcifpnum,
	shnat_status_pcifpdevnotfound,
	shnat_status_agingoutvalueerror,
	shnat_status_exceedindex,
	shnat_status_wanipnotfound,
	shnat_status_argumenterror,
	shnat_status_insertsessionfail,
	shnat_status_removesessionfail
} shnat_status;

typedef struct _shnat_return_t {
    shnat_status       status; /* Return status */
	union {
		struct {
			shnat_boolean enable;
		} shnat_enable;

		struct {
			u32 ip;
		} shnat_lanip;

		struct {
			shnat_boolean enable;
		} shnat_pcifptolan;

		struct {
			u32 threshold;
		} shnat_sna_th;

		struct {
			shnat_boolean enable;
		} shnat_ftponly;

		struct {
			shnat_boolean enable;
		} shnat_da_enable;

		struct {
			shnat_boolean enable;
			u8 landscp;
			u8 wandscp;
		} shnat_chgdscp;

		struct {
			shnat_boolean enable;
		} shnat_udp;

		struct {
			shnat_boolean enable;
		} shnat_smartbit;
	} data;
} shnat_return;

void shnat_api(shnat_input *input, shnat_return *ret);

typedef void (*shnat_api_callback) (shnat_input *input, shnat_return *ret);

typedef struct {
	shnat_cmd cmd;
	shnat_api_callback callback;
} shnat_api_callback_t;

#endif /* STR9100_SHNAT_API */

#endif /* _STAR_HNAT_H_ */
