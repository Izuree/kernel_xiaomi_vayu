/*
 * nf_tables_trace_dummy.c - Stub implementation for nftables tracing
 *
 * License: GPL-2.0
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jump_label.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_tables.h>

/* Define and export the static key used to gate tracing */
DEFINE_STATIC_KEY_FALSE(nft_trace_enabled);
EXPORT_SYMBOL_GPL(nft_trace_enabled);

static int trace_fill_id(struct sk_buff *nlskb, struct sk_buff *skb)
{
	(void)nlskb;
	(void)skb;
	return 0;
}

static int trace_fill_header(struct sk_buff *nlskb, u16 type,
			     const struct sk_buff *skb,
			     int off, unsigned int len)
{
	(void)nlskb;
	(void)type;
	(void)skb;
	(void)off;
	(void)len;
	return 0;
}

static int nf_trace_fill_ll_header(struct sk_buff *nlskb,
				   const struct sk_buff *skb)
{
	(void)nlskb;
	(void)skb;
	return 0;
}

static int nf_trace_fill_dev_info(struct sk_buff *nlskb,
				 const struct net_device *indev,
				 const struct net_device *outdev)
{
	(void)nlskb;
	(void)indev;
	(void)outdev;
	return 0;
}

static int nf_trace_fill_pkt_info(struct sk_buff *nlskb,
				 const struct nft_pktinfo *pkt)
{
	(void)nlskb;
	(void)pkt;
	return 0;
}

static int nf_trace_fill_rule_info(struct sk_buff *nlskb,
				  const struct nft_traceinfo *info)
{
	(void)nlskb;
	(void)info;
	return 0;
}

static bool nft_trace_have_verdict_chain(struct nft_traceinfo *info)
{
	(void)info;
	return false;
}

void nft_trace_notify(struct nft_traceinfo *info)
{
	(void)info;
	/* Stub - does nothing */
}
EXPORT_SYMBOL_GPL(nft_trace_notify);

void nft_trace_init(struct nft_traceinfo *info, const struct nft_pktinfo *pkt,
		    const struct nft_verdict *verdict,
		    const struct nft_chain *chain)
{
	(void)info;
	(void)pkt;
	(void)verdict;
	(void)chain;
	/* Stub - does nothing */
}
EXPORT_SYMBOL_GPL(nft_trace_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dummy Tracer");
MODULE_DESCRIPTION("Stub for netfilter nftables tracing");