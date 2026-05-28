/* Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM ipa
#define TRACE_INCLUDE_FILE ipa_trace

#if !defined(_IPA_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _IPA_TRACE_H

#include <linux/tracepoint.h>

#ifdef CONFIG_IPA_TRACE

/* =====================================================================
 * REAL TRACEPOINTS (Enabled via CONFIG_IPA_TRACE=y)
 * =====================================================================
 */

TRACE_EVENT(
	intr_to_poll3,

	TP_PROTO(unsigned long client),

	TP_ARGS(client),

	TP_STRUCT__entry(
		__field(unsigned long,	client)
	),

	TP_fast_assign(
		__entry->client = client;
	),

	TP_printk("client=%lu", __entry->client)
);

TRACE_EVENT(
	poll_to_intr3,

	TP_PROTO(unsigned long client),

	TP_ARGS(client),

	TP_STRUCT__entry(
		__field(unsigned long,	client)
	),

	TP_fast_assign(
		__entry->client = client;
	),

	TP_printk("client=%lu", __entry->client)
);

TRACE_EVENT(
	idle_sleep_enter3,

	TP_PROTO(unsigned long client),

	TP_ARGS(client),

	TP_STRUCT__entry(
		__field(unsigned long,	client)
	),

	TP_fast_assign(
		__entry->client = client;
	),

	TP_printk("client=%lu", __entry->client)
);

TRACE_EVENT(
	idle_sleep_exit3,

	TP_PROTO(unsigned long client),

	TP_ARGS(client),

	TP_STRUCT__entry(
		__field(unsigned long,	client)
	),

	TP_fast_assign(
		__entry->client = client;
	),

	TP_printk("client=%lu", __entry->client)
);

TRACE_EVENT(
	rmnet_ipa_netifni3,

	TP_PROTO(unsigned long rx_pkt_cnt),

	TP_ARGS(rx_pkt_cnt),

	TP_STRUCT__entry(
		__field(unsigned long,	rx_pkt_cnt)
	),

	TP_fast_assign(
		__entry->rx_pkt_cnt = rx_pkt_cnt;
	),

	TP_printk("rx_pkt_cnt=%lu", __entry->rx_pkt_cnt)
);

TRACE_EVENT(
	rmnet_ipa_netifrx3,

	TP_PROTO(unsigned long rx_pkt_cnt),

	TP_ARGS(rx_pkt_cnt),

	TP_STRUCT__entry(
		__field(unsigned long,	rx_pkt_cnt)
	),

	TP_fast_assign(
		__entry->rx_pkt_cnt = rx_pkt_cnt;
	),

	TP_printk("rx_pkt_cnt=%lu", __entry->rx_pkt_cnt)
);

TRACE_EVENT(
	rmnet_ipa_netif_rcv_skb3,

	TP_PROTO(unsigned long rx_pkt_cnt),

	TP_ARGS(rx_pkt_cnt),

	TP_STRUCT__entry(
		__field(unsigned long,	rx_pkt_cnt)
	),

	TP_fast_assign(
		__entry->rx_pkt_cnt = rx_pkt_cnt;
	),

	TP_printk("rx_pkt_cnt=%lu", __entry->rx_pkt_cnt)
);

TRACE_EVENT(
	ipa3_rx_poll_num,

	TP_PROTO(int poll_num),

	TP_ARGS(poll_num),

	TP_STRUCT__entry(
		__field(int,	poll_num)
	),

	TP_fast_assign(
		__entry->poll_num = poll_num;
	),

	TP_printk("each_poll_aggr_pkt_num=%d", __entry->poll_num)
);

TRACE_EVENT(
	ipa3_rx_poll_cnt,

	TP_PROTO(int poll_num),

	TP_ARGS(poll_num),

	TP_STRUCT__entry(
		__field(int,	poll_num)
	),

	TP_fast_assign(
		__entry->poll_num = poll_num;
	),

	TP_printk("napi_overall_poll_pkt_cnt=%d", __entry->poll_num)
);

#else 

/* =====================================================================
 * STUBBED TRACEPOINTS (Disabled - Compiles down to nothing)
 * =====================================================================
 */

static inline void trace_intr_to_poll3(unsigned long client) {}
static inline bool trace_intr_to_poll3_enabled(void) { return false; }

static inline void trace_poll_to_intr3(unsigned long client) {}
static inline bool trace_poll_to_intr3_enabled(void) { return false; }

static inline void trace_idle_sleep_enter3(unsigned long client) {}
static inline bool trace_idle_sleep_enter3_enabled(void) { return false; }

static inline void trace_idle_sleep_exit3(unsigned long client) {}
static inline bool trace_idle_sleep_exit3_enabled(void) { return false; }

static inline void trace_rmnet_ipa_netifni3(unsigned long rx_pkt_cnt) {}
static inline bool trace_rmnet_ipa_netifni3_enabled(void) { return false; }

static inline void trace_rmnet_ipa_netifrx3(unsigned long rx_pkt_cnt) {}
static inline bool trace_rmnet_ipa_netifrx3_enabled(void) { return false; }

static inline void trace_rmnet_ipa_netif_rcv_skb3(unsigned long rx_pkt_cnt) {}
static inline bool trace_rmnet_ipa_netif_rcv_skb3_enabled(void) { return false; }

static inline void trace_ipa3_rx_poll_num(int poll_num) {}
static inline bool trace_ipa3_rx_poll_num_enabled(void) { return false; }

static inline void trace_ipa3_rx_poll_cnt(int poll_num) {}
static inline bool trace_ipa3_rx_poll_cnt_enabled(void) { return false; }

#endif /* CONFIG_IPA_TRACE */

#endif /* _IPA_TRACE_H */

/* This part must be outside protection and ONLY evaluated when tracing is active */
#ifdef CONFIG_IPA_TRACE
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../drivers/platform/msm/ipa/ipa_v3
#include <trace/define_trace.h>
#endif