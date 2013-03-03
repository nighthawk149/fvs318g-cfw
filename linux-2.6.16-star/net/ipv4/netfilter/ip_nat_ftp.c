/* FTP extension for TCP NAT alteration. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/moduleparam.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_ftp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

#ifdef CONFIG_STR9100_SHNAT
#include <linux/str9100/str9100_shnat_hook.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rusty Russell <rusty@rustcorp.com.au>");
MODULE_DESCRIPTION("ftp NAT helper");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/* FIXME: Time out? --RR */

static int
mangle_rfc959_packet(struct sk_buff **pskb,
		     u_int32_t newip,
		     u_int16_t port,
		     unsigned int matchoff,
		     unsigned int matchlen,
		     struct ip_conntrack *ct,
		     enum ip_conntrack_info ctinfo,
		     u32 *seq)
{
	char buffer[sizeof("nnn,nnn,nnn,nnn,nnn,nnn")];

	sprintf(buffer, "%u,%u,%u,%u,%u,%u",
		NIPQUAD(newip), port>>8, port&0xFF);

	DEBUGP("calling ip_nat_mangle_tcp_packet\n");

	*seq += strlen(buffer) - matchlen;
	return ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, matchoff, 
					matchlen, buffer, strlen(buffer));
}

/* |1|132.235.1.2|6275| */
static int
mangle_eprt_packet(struct sk_buff **pskb,
		   u_int32_t newip,
		   u_int16_t port,
		   unsigned int matchoff,
		   unsigned int matchlen,
		   struct ip_conntrack *ct,
		   enum ip_conntrack_info ctinfo,
		   u32 *seq)
{
	char buffer[sizeof("|1|255.255.255.255|65535|")];

	sprintf(buffer, "|1|%u.%u.%u.%u|%u|", NIPQUAD(newip), port);

	DEBUGP("calling ip_nat_mangle_tcp_packet\n");

	*seq += strlen(buffer) - matchlen;
	return ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, matchoff, 
					matchlen, buffer, strlen(buffer));
}

/* |1|132.235.1.2|6275| */
static int
mangle_epsv_packet(struct sk_buff **pskb,
		   u_int32_t newip,
		   u_int16_t port,
		   unsigned int matchoff,
		   unsigned int matchlen,
		   struct ip_conntrack *ct,
		   enum ip_conntrack_info ctinfo,
		   u32 *seq)
{
	char buffer[sizeof("|||65535|")];

	sprintf(buffer, "|||%u|", port);

	DEBUGP("calling ip_nat_mangle_tcp_packet\n");

	*seq += strlen(buffer) - matchlen;
	return ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, matchoff, 
					matchlen, buffer, strlen(buffer));
}

static int (*mangle[])(struct sk_buff **, u_int32_t, u_int16_t,
		     unsigned int,
		     unsigned int,
		     struct ip_conntrack *,
		     enum ip_conntrack_info,
		     u32 *seq)
= { [IP_CT_FTP_PORT] = mangle_rfc959_packet,
    [IP_CT_FTP_PASV] = mangle_rfc959_packet,
    [IP_CT_FTP_EPRT] = mangle_eprt_packet,
    [IP_CT_FTP_EPSV] = mangle_epsv_packet
};

/* So, this packet has hit the connection tracking matching code.
   Mangle it, and change the expectation to match the new version. */
static unsigned int ip_nat_ftp(struct sk_buff **pskb,
			       enum ip_conntrack_info ctinfo,
			       enum ip_ct_ftp_type type,
			       unsigned int matchoff,
			       unsigned int matchlen,
			       struct ip_conntrack_expect *exp,
			       u32 *seq)
{
	u_int32_t newip;
	u_int16_t port;
	int dir = CTINFO2DIR(ctinfo);
	struct ip_conntrack *ct = exp->master;

	DEBUGP("FTP_NAT: type %i, off %u len %u\n", type, matchoff, matchlen);

	/* Connection will come from wherever this packet goes, hence !dir */
	newip = ct->tuplehash[!dir].tuple.dst.ip;
	exp->saved_proto.tcp.port = exp->tuple.dst.u.tcp.port;
	exp->dir = !dir;

	/* When you see the packet, we need to NAT it the same as the
	 * this one. */
	exp->expectfn = ip_nat_follow_master;

	// For Debug Purpose
#if 0
#ifdef CONFIG_STR9100_SHNAT
	printk("dir: %d (! %d)\n",dir,!dir);
	printk("newip: %d.%d.%d.%d \n",NIPQUAD(newip));

	printk("tuplehash[%d]: ",dir);
	DUMP_TUPLE2(&ct->tuplehash[dir].tuple);
	printk("tuplehash[%d]: ",!dir);
	DUMP_TUPLE2(&ct->tuplehash[!dir].tuple);
	printk("exp:");
	DUMP_TUPLE2(&exp->tuple);
	printk("exp(mask):");
	DUMP_TUPLE2(&exp->mask);
	if(exp->master){
	printk("exp(master hash[0]):");
	DUMP_TUPLE2(&exp->master->tuplehash[0].tuple);
	printk("exp(master hash[1]):");
	DUMP_TUPLE2(&exp->master->tuplehash[0].tuple);
	}
	printk("exp->saved_proto.tcp.port: %d \n",htons(exp->saved_proto.tcp.port));
#endif
#endif

	{
	    struct iphdr *iph;
	    struct tcphdr *tcph;

	    iph = (*pskb)->nh.iph;
	    tcph = (struct tcphdr *)((u_int32_t *)iph + iph->ihl);
	    #if 0
	    printk(" ip: saddr:%u.%u.%u.%u:%u daddr:%u.%u.%u.%u:%u \n",
		NIPQUAD(iph->saddr), htons(tcph->source),
		NIPQUAD(iph->daddr),htons(tcph->dest));
	    #endif

	}

    
#if 1
//#if 0
#ifdef CONFIG_STR9100_SHNAT
	if (star9100_shnat_hook_ready) {
		DEBUGP("[%s:%s():%d] newip is <%u.%u.%u.%u> dst ip is <%u.%u.%u.%u>\n",
			__FILE__, __FUNCTION__, __LINE__, NIPQUAD(newip), NIPQUAD(exp->tuple.dst.ip));

		// Client to Server dir =  0 == Port Mode
		/* (ct->tuplehash[0].dst.ip == ct->tuplehash[1].src.ip) for 
		 * Server to Private Ip used (FTP Procotol)
		 * dir = 0 == FTP Port Mode
		 */ 
		if (dir==0&&(ct->tuplehash[0].tuple.dst.ip == ct->tuplehash[1].tuple.src.ip)) {
			int shnat_hash = star9100_shnat_nf_nat_preadd_hnatable_hook(ct, dir, htons(exp->saved_proto.tcp.port));
			DEBUGP("LAN=> WAN: PORT Mode port is <%u> shnat_hash is <%u>\n",htons(exp->saved_proto.tcp.port), shnat_hash);

			if (shnat_hash >= 0 ) {
				/* got port from HNAT */
				port = shnat_hash;
				//exp->saved_proto.tcp.port = htons(port);
				exp->tuple.dst.u.tcp.port = htons(port);
				ip_conntrack_expect_related(exp);
			} else {
				goto orig_assign_port;
			}
		}
#if 0 
		/*
		 *   LAN=>WAN , Passive mode.
		 */
		else if (dir==1&&(ct->tuplehash[0].tuple.dst.ip == ct->tuplehash[1].tuple.src.ip)) {
			int shnat_hash = star9100_shnat_nf_nat_preadd_hnatable_hook(ct, dir, htons(exp->saved_proto.tcp.port));
			printk("LAN=> WAN: Passive Mode port is <%u> shnat_hash is <%u>\n",htons(exp->saved_proto.tcp.port), shnat_hash);

			if ( shnat_hash >= 0 ) {
				/* got port from HNAT */
				port = shnat_hash;
				//exp->saved_proto.tcp.port = htons(port);
				exp->tuple.dst.u.tcp.port = htons(port);
				ip_conntrack_expect_related(exp);
			} else {
				goto orig_assign_port;
			}
		}
#endif
		/*
		 *   WAN ==> LAN , Passive mode.
		 */
		else if (dir==1&&(ct->tuplehash[0].tuple.src.ip == ct->tuplehash[1].tuple.dst.ip)) {
			int shnat_hash = star9100_shnat_nf_nat_preadd_hnatable_hook(ct, dir, htons(exp->saved_proto.tcp.port));
			DEBUGP("WAN=>LAN Passive mode port is <%u> shnat_hash is <%u>\n",htons(exp->saved_proto.tcp.port), shnat_hash);

			if ( shnat_hash >= 0 ) {
				/* got port from HNAT */
				port = shnat_hash;
				//exp->saved_proto.tcp.port = htons(port);
				exp->tuple.dst.u.tcp.port = htons(port);
				ip_conntrack_expect_related(exp);
			} else {
				goto orig_assign_port;
			}
		}
		/*
		 * WAN ==> LAN, PORT Mode.
		 */
		else if (dir==0 &&(ct->tuplehash[0].tuple.src.ip == ct->tuplehash[1].tuple.dst.ip)) {
			int shnat_hash = star9100_shnat_nf_nat_preadd_hnatable_hook(ct, !dir, htons(exp->saved_proto.tcp.port));
			DEBUGP("WAN=>LAN PORT MODE: port is <%u> shnat_hash is <%u>\n",htons(exp->saved_proto.tcp.port), shnat_hash);

			if ( shnat_hash >= 0 ) {
				/* got port from HNAT */
				port = shnat_hash;
				//exp->saved_proto.tcp.port = htons(port);
				exp->tuple.dst.u.tcp.port = htons(port);
				ip_conntrack_expect_related(exp);
			} else {
				goto orig_assign_port;
			}

		} else {
			goto orig_assign_port;
		}

		goto shnat_assign_port;
	}

orig_assign_port:
	for (port = ntohs(exp->saved_proto.tcp.port); port != 0; port++) {
		exp->tuple.dst.u.tcp.port = htons(port);
		if (ip_conntrack_expect_related(exp) == 0)
			break;
	}

shnat_assign_port:
#else
	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(exp->saved_proto.tcp.port); port != 0; port++) {
		exp->tuple.dst.u.tcp.port = htons(port);
		if (ip_conntrack_expect_related(exp) == 0)
			break;
	}
#endif
#endif

	if (port == 0)
		return NF_DROP;

	if (!mangle[type](pskb, newip, port, matchoff, matchlen, ct, ctinfo,
			  seq)) {
		ip_conntrack_unexpect_related(exp);
		return NF_DROP;
	}
	return NF_ACCEPT;
}

static void __exit fini(void)
{
	ip_nat_ftp_hook = NULL;
	/* Make sure noone calls it, meanwhile. */
	synchronize_net();
}

static int __init init(void)
{
	BUG_ON(ip_nat_ftp_hook);
	ip_nat_ftp_hook = ip_nat_ftp;
	return 0;
}

/* Prior to 2.6.11, we had a ports param.  No longer, but don't break users. */
static int warn_set(const char *val, struct kernel_param *kp)
{
	printk(KERN_INFO KBUILD_MODNAME
	       ": kernel >= 2.6.10 only uses 'ports' for conntrack modules\n");
	return 0;
}
module_param_call(ports, warn_set, NULL, NULL, 0);

module_init(init);
module_exit(fini);
