#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_ipv6/ip6t_NPT.h>

enum {
	O_SRC_PFX	= 1 << 0,
	O_DST_PFX	= 1 << 1,
};

static const struct xt_option_entry SNPT_options[] = {
	{ .name = "src-pfx", .id = O_SRC_PFX, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_MAND },
	{ .name = "dst-pfx", .id = O_DST_PFX, .type = XTTYPE_HOSTMASK,
	  .flags = XTOPT_MAND },
	{ }
};

static void SNPT_help(void)
{
	printf("SNPT target options:"
	       "\n"
	       " --src-pfx prefix/length\n"
	       " --dst-pfx prefix/length\n"
	       "\n");
}

static void SNPT_parse(struct xt_option_call *cb)
{
	struct ip6t_npt_tginfo *npt = cb->data;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_SRC_PFX:
		npt->src_pfx = cb->val.haddr;
		npt->src_pfx_len = cb->val.hlen;
		break;
	case O_DST_PFX:
		npt->dst_pfx = cb->val.haddr;
		npt->dst_pfx_len = cb->val.hlen;
		break;
	}
}

static void SNPT_print(const void *ip, const struct xt_entry_target *target,
		       int numeric)
{
	const struct ip6t_npt_tginfo *npt = (const void *)target->data;

	printf("src-pfx %s/%u ", xtables_ip6addr_to_numeric(&npt->src_pfx.in6),
				 npt->src_pfx_len);
	printf("dst-pfx %s/%u ", xtables_ip6addr_to_numeric(&npt->dst_pfx.in6),
				 npt->dst_pfx_len);
}

static struct xtables_target snpt_tg_reg = {
	.name		= "DNPT",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size		= XT_ALIGN(sizeof(struct ip6t_npt_tginfo)),
	.userspacesize	= offsetof(struct ip6t_npt_tginfo, adjustment),
	.help		= SNPT_help,
	.x6_parse	= SNPT_parse,
	.print		= SNPT_print,
	.x6_options	= SNPT_options,
};

void _init(void)
{
	xtables_register_target(&snpt_tg_reg);
}
