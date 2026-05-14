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
	{   0,       0 },
	{ 40, 1171200 },
	{ 80, 1248000 },
	{ 90, 1344000 },
	{ 120, 1420800 },
	{ 160, 1516800 },
	{ 180, 1612800 },
	{ 250, 1708800 },
	{ 299, 1804800 },
};

/* ── Gold (cpu4-6) ─── cap=899, max=2419200 ───────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_gold[] = {
	{   0,   0 },
	{ 500,  825600 },
	{ 521,  940800 },
	{ 553,  1056000 },
	{ 593,  1171200 },
	{ 639,  1286400 },
	{ 672,  1382400 },
	{ 700,  1478400 },
	{ 720,  1574400 },
	{ 740,  1670400 },
	{ 762,  1766400 },
	{ 780,  1862400 },
	{ 790,  2054400 },
    { 806,  2246400 },
    { 830,  2419200 },
};

/* ── Prime (cpu7) ──── cap=1024, max=2956800 ──────────────────────────────── */
static const struct sugov_lut_entry sugov_lut_prime[] = {
	{    0,       0 },
	{  293,  844800 },
	{  332,  960000 },
	{  372, 1075200 },
	{  412, 1190400 },
	{  452, 1305600 },
	{  485, 1401600 },
	{  525, 1516800 },
	{  565, 1632000 },
	{  605, 1747200 },
	{  645, 1862400 },
	{  685, 1977600 },
	{  718, 2073600 },
	{  751, 2169600 },
	{  785, 2265600 },
	{  818, 2361600 },
	{  851, 2457600 },
	{  884, 2553600 },
	{  918, 2649600 },
	{  951, 2745600 },
	{  984, 2841600 },
	{ 1024, 2956800 },
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