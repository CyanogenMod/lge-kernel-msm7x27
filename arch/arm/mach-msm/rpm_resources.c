/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#include "rpm_resources.h"

/******************************************************************************
 * Data type and structure definitions
 *****************************************************************************/

enum {
	MSM_RPMRS_DEBUG_FLUSH = 1U << 0,
	MSM_RPMRS_DEBUG_BUFFER = 1U << 1,
};

static int msm_rpmrs_debug_mask;
module_param_named(
	debug_mask, msm_rpmrs_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP
);

static int msm_rpmrs_append_pxo(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits);
static int msm_rpmrs_append_l2_cache(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits);
static int msm_rpmrs_append_vdd_mem(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits);
static int msm_rpmrs_append_vdd_dig(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits);

#define MSM_RPMRS_MAX_RS_REGISTER_COUNT 2

struct msm_rpmrs_resource {
	struct msm_rpm_iv_pair rs[MSM_RPMRS_MAX_RS_REGISTER_COUNT];
	uint32_t size;
	char *name;

	uint32_t req_index[MSM_RPMRS_MAX_RS_REGISTER_COUNT];
	bool is_set;

	int (*append_req)(struct msm_rpm_iv_pair *req, int count,
			struct msm_rpmrs_limits *limits);
};

static struct msm_rpmrs_resource msm_rpmrs_pxo = {
	.rs[0].id = MSM_RPM_ID_PXO_CLK,
	.size = 1,
	.name = "PXO",
	.append_req = msm_rpmrs_append_pxo,
};

static struct msm_rpmrs_resource msm_rpmrs_l2_cache = {
	.rs[0].id = MSM_RPM_ID_APPS_L2_CACHE_CTL,
	.size = 1,
	.name = "L2 Cache",
	.append_req = msm_rpmrs_append_l2_cache,
};

static struct msm_rpmrs_resource msm_rpmrs_vdd_mem = {
	.rs[0].id = MSM_RPM_ID_SMPS0_0,
	.rs[1].id = MSM_RPM_ID_SMPS0_1,
	.size = 2,
	.name = "Vdd Mem",
	.append_req = msm_rpmrs_append_vdd_mem,
};

static struct msm_rpmrs_resource msm_rpmrs_vdd_dig = {
	.rs[0].id = MSM_RPM_ID_SMPS1_0,
	.rs[1].id = MSM_RPM_ID_SMPS1_1,
	.size = 2,
	.name = "Vdd Dig",
	.append_req = msm_rpmrs_append_vdd_dig,
};

static struct msm_rpmrs_resource *msm_rpmrs_resources[] = {
	&msm_rpmrs_pxo,
	&msm_rpmrs_l2_cache,
	&msm_rpmrs_vdd_mem,
	&msm_rpmrs_vdd_dig,
};

static DEFINE_SPINLOCK(msm_rpmrs_lock);
static struct msm_rpm_iv_pair msm_rpmrs_req[8];

#define MSM_RPMRS_VDD_MASK  0xfff
#define MSM_RPMRS_VDD(v)  (v & (MSM_RPMRS_VDD_MASK))

/******************************************************************************
 * Internal helper functions
 *****************************************************************************/

static int msm_rpmrs_append_sclk(struct msm_rpm_iv_pair *req, int count,
	uint32_t sclk_count)
{
	if (count < 2) {
		pr_err("%s: invalid array size (%d < %d)\n",
			__func__, count, 2);
		return -EINVAL;
	}

	req[0].id = MSM_RPM_ID_TRIGGER_TIMED_TO;
	req[0].value = 0;
	req[1].id = MSM_RPM_ID_TRIGGER_TIMED_SCLK_COUNT;
	req[1].value = sclk_count;

	return 2;
}

static int msm_rpmrs_append_resource(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_resource *rs)
{
	int i;

	if (!rs->is_set)
		return 0;

	if (count < rs->size) {
		pr_err("%s: invalid array size (%d < %d) for %s\n",
			__func__, count, rs->size, rs->name);
		return -EINVAL;
	}

	for (i = 0; i < rs->size; i++) {
		req[i].id = rs->rs[i].id;
		req[i].value = rs->rs[i].value;
	}

	return rs->size;
}

static int msm_rpmrs_append_pxo(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits)
{
	int rc;

	rc = msm_rpmrs_append_resource(req, count, &msm_rpmrs_pxo);
	if (rc <= 0)
		return rc;

	if (limits->pxo > req[0].value)
		req[0].value = limits->pxo;

	return rc;
}

static int msm_rpmrs_append_l2_cache(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits)
{
	int rc;

	rc = msm_rpmrs_append_resource(req, count, &msm_rpmrs_l2_cache);
	if (rc <= 0)
		return rc;

	if (limits->l2_cache > req[0].value)
		req[0].value = limits->l2_cache;

	return rc;
}

static int msm_rpmrs_append_vdd_mem(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits)
{
	int rc;

	rc = msm_rpmrs_append_resource(req, count, &msm_rpmrs_vdd_mem);
	if (rc <= 0)
		return rc;

	if (MSM_RPMRS_VDD(limits->vdd_mem) > MSM_RPMRS_VDD(req[0].value)) {
		req[0].value &= ~MSM_RPMRS_VDD_MASK;
		req[0].value |= MSM_RPMRS_VDD(limits->vdd_mem);
	}

	return rc;
}

static int msm_rpmrs_append_vdd_dig(struct msm_rpm_iv_pair *req, int count,
	struct msm_rpmrs_limits *limits)
{
	int rc;

	rc = msm_rpmrs_append_resource(req, count, &msm_rpmrs_vdd_dig);
	if (rc <= 0)
		return rc;

	if (MSM_RPMRS_VDD(limits->vdd_dig) > MSM_RPMRS_VDD(req[0].value)) {
		req[0].value &= ~MSM_RPMRS_VDD_MASK;
		req[0].value |= MSM_RPMRS_VDD(limits->vdd_dig);
	}

	return rc;
}

static int msm_rpmrs_index_resource(struct msm_rpmrs_resource *rs,
	struct msm_rpm_iv_pair *req, int count)
{
	int nr_indexed = 0;
	int i, k;

	for (i = 0; i < rs->size; i++) {
		rs->req_index[i] = -1;
		for (k = 0; k < count; k++)
			if (req[k].id == rs->rs[i].id) {
				rs->req_index[i] = k;
				nr_indexed++;
				break;
			}
	}

	return nr_indexed;
}

/*
 * Return value:
 *   >=0: number of entries in <req> buffered
 *   -EINVAL: invalid id in <req> array
 */
static int msm_rpmrs_buffer_request(struct msm_rpm_iv_pair *req, int count)
{
	struct msm_rpmrs_resource *rs;
	int nr_indexed = 0;
	int nr_buffered = 0;
	int rc;
	int i, k;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
		rs = msm_rpmrs_resources[i];
		rc = msm_rpmrs_index_resource(rs, req, count);

		/*
		 * The number of entries in <req> for the resource
		 * does not match what it should be.
		 */
		if (rc > 0 && rc != rs->size)
			return -EINVAL;

		nr_indexed += rc;
	}

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources) &&
				nr_buffered < nr_indexed; i++) {
		rs = msm_rpmrs_resources[i];
		for (k = 0; k < rs->size; k++)
			if (rs->req_index[k] < count) {
				uint32_t value = req[rs->req_index[k]].value;
				rs->rs[k].value = value;
				rs->is_set = true;
				nr_buffered++;

				if (MSM_RPMRS_DEBUG_BUFFER &
						msm_rpmrs_debug_mask)
					pr_info("%s: %s[%d] = 0x%x\n",
						 __func__, rs->name, k, value);
			}
	}

	return nr_buffered;
}

/*
 * Return value:
 *   0: success
 *   -EINVAL: invalid id in <req> array
 */
static int msm_rpmrs_clear_buffer(struct msm_rpm_iv_pair *req, int count)
{
	struct msm_rpmrs_resource *rs;
	int nr_indexed = 0;
	int rc;
	int i;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
		rs = msm_rpmrs_resources[i];
		rc = msm_rpmrs_index_resource(
			msm_rpmrs_resources[i],	req, count);

		/*
		 * The number of entries in <req> for the resource
		 * does not match what it should be.
		 */
		if (rc > 0 && rc != rs->size)
			return -EINVAL;

		nr_indexed += rc;
	}

	if (nr_indexed > 0)
		for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
			rs = msm_rpmrs_resources[i];
			if (rs->req_index[0] < count) {
				rs->is_set = false;
				if (MSM_RPMRS_DEBUG_BUFFER &
						msm_rpmrs_debug_mask)
					pr_info("%s: %s\n", __func__, rs->name);
			}
		}

	return 0;
}

static int msm_rpmrs_set_common(
	int ctx, struct msm_rpm_iv_pair *req, int count, bool noirq)
{
	if (ctx == MSM_RPM_CTX_SET_SLEEP) {
		int rc;

		spin_lock(&msm_rpmrs_lock);
		rc = msm_rpmrs_buffer_request(req, count);
		spin_unlock(&msm_rpmrs_lock);

		if (rc < 0)
			return rc;

		if (rc == count)
			return 0;
	}

	if (noirq)
		return msm_rpm_set_noirq(ctx, req, count);
	else
		return msm_rpm_set(ctx, req, count);
}

static int msm_rpmrs_clear_common(
	int ctx, struct msm_rpm_iv_pair *req, int count, bool noirq)
{
	if (ctx == MSM_RPM_CTX_SET_SLEEP) {
		int rc;

		spin_lock(&msm_rpmrs_lock);
		rc = msm_rpmrs_clear_buffer(req, count);
		spin_unlock(&msm_rpmrs_lock);

		if (rc < 0)
			return rc;
	}

	if (noirq)
		return msm_rpm_clear_noirq(ctx, req, count);
	else
		return msm_rpm_clear(ctx, req, count);
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

int msm_rpmrs_set(int ctx, struct msm_rpm_iv_pair *req, int count)
{
	return msm_rpmrs_set_common(ctx, req, count, false);
}

int msm_rpmrs_set_noirq(int ctx, struct msm_rpm_iv_pair *req, int count)
{
	return msm_rpmrs_set_common(ctx, req, count, true);
}

int msm_rpmrs_clear(int ctx, struct msm_rpm_iv_pair *req, int count)
{
	return msm_rpmrs_clear_common(ctx, req, count, false);
}

int msm_rpmrs_clear_noirq(int ctx, struct msm_rpm_iv_pair *req, int count)
{
	return msm_rpmrs_clear_common(ctx, req, count, true);
}

int msm_rpmrs_flush_buffer_noirq(
	uint32_t sclk_count, struct msm_rpmrs_limits *limits)
{
	struct msm_rpm_iv_pair *req = msm_rpmrs_req;
	int count = ARRAY_SIZE(msm_rpmrs_req);
	int rc;
	int i;

	spin_lock(&msm_rpmrs_lock);

	rc = msm_rpmrs_append_sclk(req, count, sclk_count);
	if (rc < 0)
		goto flush_buffer_exit;

	req += rc;
	count -= rc;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
		rc = msm_rpmrs_resources[i]->append_req(req, count, limits);
		if (rc < 0)
			goto flush_buffer_exit;

		req += rc;
		count -= rc;
	}

	count = ARRAY_SIZE(msm_rpmrs_req) - count;

	if (MSM_RPMRS_DEBUG_FLUSH & msm_rpmrs_debug_mask)
		for (i = 0; i < count; i++)
			pr_info("%s: flush reg %d: 0x%x\n", __func__,
				msm_rpmrs_req[i].id, msm_rpmrs_req[i].value);

	rc = msm_rpm_set_noirq(MSM_RPM_CTX_SET_SLEEP, msm_rpmrs_req, count);

flush_buffer_exit:
	spin_unlock(&msm_rpmrs_lock);
	return rc;
}
