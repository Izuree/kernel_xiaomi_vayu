// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 deu <fawwazzuladhim700@gmail.com>.
 * schedutil_lut.h - Per-cluster util-to-frequency LUT for schedutil governor
 *
 * LUT keys are percentage of cluster capacity (0–100).
 * Lookup converts raw util to percentage via:
 *   util_pct = util * 100 / arch_scale_cpu_capacity(cpu)
 */

#ifndef _SUGOV_DVFS_LUT_H
#define _SUGOV_DVFS_LUT_H

struct sugov_lut_entry {
	unsigned int util_pct;   /* 0–100: percentage of cluster capacity */
	unsigned int freq_khz;
};

/* ── Silver (cpu0-3) ─ cap=299, max=1804800 ───────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_silver[] = {
	{   0,       0 },
	{  5, 1171200 },
	{  8, 1248000 },
	{  21, 1344000 },
	{  32, 1420800 },
	{  42, 1516800 },
	{  52, 1612800 },
	{  62, 1708800 },
	{ 73, 1804800 },
};

/* ── Gold (cpu4-6) ─── cap=899, max=2419200 ───────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_gold[] = {
	{   0,       0 },
	{  33,  825600 },
	{  39,  940800 },
	{  47, 1056000 },
	{  59, 1171200 },
	{  68, 1382400 },
	{  72, 1478400 },
	{  75, 1574400 },
	{  79, 1670400 },
	{  82, 1766400 },
	{  86, 1862400 },
	{  88, 1958400 },
	{  90, 2054400 },
	{  91, 2150400 },
	{  95, 2246400 },
	{ 100, 2419200 },
};

/* ── Prime (cpu7) ──── cap=1024, max=2956800 ──────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_prime[] = {
	{   0,       0 },
	{  29,  844800 },
	{  33,  960000 },
	{  36, 1075200 },
	{  40, 1190400 },
	{  44, 1305600 },
	{  47, 1401600 },
	{  49, 1516800 },
	{  51, 1632000 },
	{  53, 1747200 },
	{  54, 1862400 },
	{  55, 1977600 },
	{  57, 2073600 },
	{  59, 2169600 },
	{  62, 2265600 },
	{  64, 2361600 },
	{  65, 2457600 },
	{  67, 2553600 },
	{  68, 2649600 },
	{  71, 2745600 },
	{  73, 2841600 },
	{ 100, 2956800 },
};

#define SUGOV_LUT_SIZE(lut) (ARRAY_SIZE(lut))

/*
 * sugov_lut_lookup - interpolate freq from util percentage using a LUT
 * @lut:      pointer to the cluster's LUT array
 * @size:     number of entries (use SUGOV_LUT_SIZE)
 * @util_pct: util as percentage of cluster capacity (0–100)
 *
 * Returns frequency in kHz.
 */
static inline unsigned int sugov_lut_lookup(const struct sugov_lut_entry *lut,
					    unsigned int size,
					    unsigned int util_pct)
{
	unsigned int i;

	if (util_pct <= lut[0].util_pct)
		return lut[0].freq_khz;

	for (i = 1; i < size; i++) {
		if (util_pct <= lut[i].util_pct) {
			unsigned long du = lut[i].util_pct  - lut[i-1].util_pct;
			unsigned long df = lut[i].freq_khz  - lut[i-1].freq_khz;
			return lut[i-1].freq_khz +
			       (unsigned int)(df * (util_pct - lut[i-1].util_pct) / du);
		}
	}

	return lut[size - 1].freq_khz;
}

#endif /* _SUGOV_DVFS_LUT_H */