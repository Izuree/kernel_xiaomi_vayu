// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 deu <fawwazzuladhim700@gmail.com>.
 * schedutil_lut.h - Per-cluster util-to-frequency LUT for schedutil governor
 */

#ifndef _SUGOV_DVFS_LUT_H
#define _SUGOV_DVFS_LUT_H

struct sugov_lut_entry {
	unsigned int hroom_util;
	unsigned int freq_khz;
};

/* ── Silver (cpu0-3) ─ cap=299, max=1804800 ───────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_silver[] = {
	{ 0,       0 },
	{ 20,  1171200 },
	{ 40,  1248000 },
	{ 50,  1344000 },
	{ 70,  1420800 },
	{ 80,  1516800 },
	{ 100,  1612800 },
	{ 150,  1708800 },
	{ 180,  1804800 },
};

/* ── Gold (cpu4-6) ─── cap=899, max=2419200 ───────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_gold[] = {
	{ 0,       0 },
	{ 307,   825600 },
	{ 350,   940800 },
	{ 393,  1056000 },
	{ 436,  1171200 },
	{ 514,  1382400 },
	{ 549,  1478400 },
	{ 585,  1574400 },
	{ 621,  1670400 },
	{ 657,  1766400 },
	{ 692,  1862400 },
	{ 728,  1958400 },
	{ 764,  2054400 },
	{ 800,  2150400 },
	{ 836,  2246400 },
	{ 899,  2419200 },
};

/* ── Prime (cpu7) ──── cap=1024, max=2956800 ──────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_prime[] = {
	{ 0,       0 },
	{ 293,   844800 },
	{ 333,   960000 },
	{ 372,  1075200 },
	{ 412,  1190400 },
	{ 452,  1305600 },
	{ 485,  1401600 },
	{ 499,  1516800 },
	{ 521,  1632000 },
	{ 541,  1747200 },
	{ 552,  1862400 },
	{ 560,  1977600 },
	{ 580,  2073600 },
	{ 600,  2169600 },
	{ 630,  2265600 },
	{ 660,  2361600 },
	{ 670,  2457600 },
	{ 690,  2553600 },
	{ 700,  2649600 },
	{ 732,  2745600 },
	{ 750,  2841600 },
	{ 760, 2956800 },
};

#define SUGOV_LUT_SIZE(lut) (ARRAY_SIZE(lut))

/*
 * sugov_lut_lookup - interpolate freq from hroom util using a LUT
 * @lut:  pointer to the cluster's LUT array
 * @size: number of entries (use SUGOV_LUT_SIZE)
 * @util:  util value (0-1024)
 *
 * Returns frequency in kHz.
 */
static inline unsigned int sugov_lut_lookup(const struct sugov_lut_entry *lut,
					    unsigned int size,
					    unsigned long util)
{
	unsigned int i;

	if (util <= lut[0].hroom_util)
		return lut[0].freq_khz;

	for (i = 1; i < size; i++) {
		if (util <= lut[i].hroom_util) {
			unsigned long du = lut[i].hroom_util - lut[i-1].hroom_util;
			unsigned long df = lut[i].freq_khz   - lut[i-1].freq_khz;
			return lut[i-1].freq_khz +
			       (unsigned int)(df * (util - lut[i-1].hroom_util) / du);
		}
	}

	return lut[size - 1].freq_khz;
}

#endif /* _SUGOV_DVFS_LUT_H */