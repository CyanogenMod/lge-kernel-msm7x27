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
#include <linux/init.h>
#include <linux/types.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

#include "rpm.h"
#include "rpm_resources.h"

/******************************************************************************
 * Debug Definitions
 *****************************************************************************/

enum {
	MSM_RPMRS_DEBUG_FLUSH = BIT(0),
	MSM_RPMRS_DEBUG_BUFFER = BIT(1),
};

static int msm_rpmrs_debug_mask;
module_param_named(
	debug_mask, msm_rpmrs_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP
);

/******************************************************************************
 * Resource Definitions
 *****************************************************************************/

enum {
	MSM_RPMRS_PXO_OFF = 0,
	MSM_RPMRS_PXO_ON = 1,
};

enum {
	MSM_RPMRS_L2_CACHE_HSFS_OPEN = 0,
	MSM_RPMRS_L2_CACHE_ACTIVE = 3,
};

enum {
	MSM_RPMRS_VDD_MEM_MIN_500 = 500,
	MSM_RPMRS_VDD_MEM_MIN_750 = 750,
	MSM_RPMRS_VDD_MEM_ACTIVE = 1100,
};

enum {
	MSM_RPMRS_VDD_DIG_MIN_500 = 500,
	MSM_RPMRS_VDD_DIG_MIN_750 = 750,
	MSM_RPMRS_VDD_DIG_ACTIVE = 1100,
};

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
	bool enable_low_power;

	int (*append_req)(struct msm_rpm_iv_pair *req, int count,
			struct msm_rpmrs_limits *limits);
};

static struct msm_rpmrs_resource msm_rpmrs_pxo = {
	.rs[0].id = MSM_RPM_ID_PXO_CLK,
	.size = 1,
	.name = "pxo",
	.enable_low_power = 1,
	.append_req = msm_rpmrs_append_pxo,
};

static struct msm_rpmrs_resource msm_rpmrs_l2_cache = {
	.rs[0].id = MSM_RPM_ID_APPS_L2_CACHE_CTL,
	.rs[0].value = 0,
	.size = 1,
	.name = "L2_cache",
	.is_set = true,
	.append_req = msm_rpmrs_append_l2_cache,
};

static struct msm_rpmrs_resource msm_rpmrs_vdd_mem = {
	.rs[0].id = MSM_RPM_ID_SMPS0_0,
	.rs[1].id = MSM_RPM_ID_SMPS0_1,
	.size = 2,
	.name = "vdd_mem",
	.append_req = msm_rpmrs_append_vdd_mem,
};

static struct msm_rpmrs_resource msm_rpmrs_vdd_dig = {
	.rs[0].id = MSM_RPM_ID_SMPS1_0,
	.rs[1].id = MSM_RPM_ID_SMPS1_1,
	.size = 2,
	.name = "vdd_dig",
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
 * Attribute Definitions
 *****************************************************************************/

struct msm_rpmrs_kboj_attribute {
	struct msm_rpmrs_resource *rs;
	struct kobj_attribute ka;
};

#define GET_RS_FROM_ATTR(attr) \
	(container_of(attr, struct msm_rpmrs_kboj_attribute, ka)->rs)

struct msm_rpmrs_resource_sysfs {
	struct attribute_group attr_group;
	struct attribute *attrs[2];
	struct msm_rpmrs_kboj_attribute kas;
};

/******************************************************************************
 * Power Level Definitions
 *****************************************************************************/

#define MSM_RPMRS_LIMITS(_pxo, _l2, _mem, _dig) { \
	MSM_RPMRS_PXO_##_pxo, \
	MSM_RPMRS_L2_CACHE_##_l2, \
	MSM_RPMRS_VDD_MEM_##_mem, \
	MSM_RPMRS_VDD_DIG_##_dig, \
	{0}, {0}, \
}

struct msm_rpmrs_level {
	enum msm_pm_sleep_mode sleep_mode;
	struct msm_rpmrs_limits rs_limits;
	bool available;

	uint32_t latency_us;
	uint32_t steady_state_power;
	uint32_t energy_overhead;
	uint32_t time_overhead_us;
};

static struct msm_rpmrs_level msm_rpmrs_levels[] = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, ACTIVE, ACTIVE),
		true,
		1, 20000, 100000, 1
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, ACTIVE, ACTIVE),
		true,
		1500, 5000, 60100000, 3000
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, ACTIVE, ACTIVE),
		true,
		1800, 5000, 60350000, 3500
	},
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, ACTIVE, ACTIVE, ACTIVE),
		true,
		3800, 4500, 65350000, 5500
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, ACTIVE),
		false,
		2800, 2500, 66850000, 4800
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, ACTIVE),
		false,
		4800, 2000, 71850000, 6800
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MIN_750, MIN_750),
		false,
		6800, 500, 75850000, 8800
	},

	/*
	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MIN_500, MIN_500),
		false,
		7800, 0, 76350000, 9800
	},
	*/
};


/******************************************************************************
 * Resource Specific Functions
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

static bool msm_rpmrs_pxo_beyond_limits(struct msm_rpmrs_limits *limits)
{
	struct msm_rpmrs_resource *rs = &msm_rpmrs_pxo;
	uint32_t pxo;

	if (rs->enable_low_power && rs->is_set)
		pxo = rs->rs[0].value;
	else
		pxo = MSM_RPMRS_PXO_ON;

	return pxo > limits->pxo;
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

static bool msm_rpmrs_l2_cache_beyond_limits(struct msm_rpmrs_limits *limits)
{
	struct msm_rpmrs_resource *rs = &msm_rpmrs_l2_cache;
	uint32_t l2_cache;

	if (rs->enable_low_power && rs->is_set)
		l2_cache = rs->rs[0].value;
	else
		l2_cache = MSM_RPMRS_L2_CACHE_ACTIVE;

	return l2_cache > limits->l2_cache;
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

static bool msm_rpmrs_vdd_mem_beyond_limits(struct msm_rpmrs_limits *limits)
{
	struct msm_rpmrs_resource *rs = &msm_rpmrs_vdd_mem;
	uint32_t vdd_mem;

	if (rs->enable_low_power && rs->is_set)
		vdd_mem = rs->rs[0].value;
	else
		vdd_mem = MSM_RPMRS_VDD_MEM_ACTIVE;

	return MSM_RPMRS_VDD(vdd_mem) > MSM_RPMRS_VDD(limits->vdd_mem);
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

static bool msm_rpmrs_vdd_dig_beyond_limits(struct msm_rpmrs_limits *limits)
{
	struct msm_rpmrs_resource *rs = &msm_rpmrs_vdd_dig;
	uint32_t vdd_dig;

	if (rs->enable_low_power && rs->is_set)
		vdd_dig = rs->rs[0].value;
	else
		vdd_dig = MSM_RPMRS_VDD_DIG_ACTIVE;

	return MSM_RPMRS_VDD(vdd_dig) > MSM_RPMRS_VDD(limits->vdd_dig);
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

/******************************************************************************
 * Buffering Functions
 *****************************************************************************/

static void msm_rpmrs_update_levels(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_levels); i++) {
		struct msm_rpmrs_level *level = &msm_rpmrs_levels[i];

		if (level->sleep_mode != MSM_PM_SLEEP_MODE_POWER_COLLAPSE)
			continue;

		if (msm_rpmrs_pxo_beyond_limits(&level->rs_limits)) {
			level->available = false;
			continue;
		}

		if (msm_rpmrs_l2_cache_beyond_limits(&level->rs_limits)) {
			level->available = false;
			continue;
		}

		if (msm_rpmrs_vdd_mem_beyond_limits(&level->rs_limits)) {
			level->available = false;
			continue;
		}

		if (msm_rpmrs_vdd_dig_beyond_limits(&level->rs_limits)) {
			level->available = false;
			continue;
		}

		level->available = true;
	}
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
 *   >=0: number of entries in <req> cleared
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
		rc = msm_rpmrs_index_resource(rs, req, count);

		/*
		 * The number of entries in <req> for the resource
		 * does not match what it should be.
		 */
		if (rc > 0 && rc != rs->size)
			return -EINVAL;

		nr_indexed += rc;
	}

	if (!nr_indexed)
		return nr_indexed;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
		rs = msm_rpmrs_resources[i];
		if (rs->req_index[0] < count) {
			rs->is_set = false;
			if (MSM_RPMRS_DEBUG_BUFFER & msm_rpmrs_debug_mask)
				pr_info("%s: %s\n", __func__, rs->name);
		}
	}

	return nr_indexed;
}

static int msm_rpmrs_set_common(
	int ctx, struct msm_rpm_iv_pair *req, int count, bool noirq)
{
	if (ctx == MSM_RPM_CTX_SET_SLEEP) {
		int rc;

		spin_lock(&msm_rpmrs_lock);
		rc = msm_rpmrs_buffer_request(req, count);
		if (rc > 0)
			msm_rpmrs_update_levels();
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
		if (rc > 0)
			msm_rpmrs_update_levels();
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
 * Attribute Functions
 *****************************************************************************/

static ssize_t msm_rpmrs_resource_attr_show(
	struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct kernel_param kp;
	unsigned int temp;
	int rc;

	spin_lock(&msm_rpmrs_lock);
	temp = GET_RS_FROM_ATTR(attr)->enable_low_power;
	spin_unlock(&msm_rpmrs_lock);

	kp.arg = &temp;
	rc = param_get_uint(buf, &kp);

	if (rc > 0) {
		strcat(buf, "\n");
		rc++;
	}

	return rc;
}

static ssize_t msm_rpmrs_resource_attr_store(struct kobject *kobj,
	struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct kernel_param kp;
	unsigned int temp;
	int rc;

	kp.arg = &temp;
	rc = param_set_uint(buf, &kp);
	if (rc)
		return rc;

	spin_lock(&msm_rpmrs_lock);
	GET_RS_FROM_ATTR(attr)->enable_low_power = temp ? true : false;
	msm_rpmrs_update_levels();
	spin_unlock(&msm_rpmrs_lock);

	return count;
}

static int __init msm_rpmrs_resource_sysfs_add(void)
{
	struct kobject *module_kobj;
	struct kobject *low_power_kboj;
	struct msm_rpmrs_resource_sysfs *rs;
	int i;
	int rc;

	module_kobj = kset_find_obj(module_kset, KBUILD_MODNAME);
	if (!module_kobj) {
		pr_err("%s: cannot find kobject for module %s\n",
			__func__, KBUILD_MODNAME);
		rc = -ENOENT;
		goto resource_sysfs_add_exit;
	}

	low_power_kboj = kobject_create_and_add(
				"enable_low_power", module_kobj);
	if (!low_power_kboj) {
		pr_err("%s: cannot create kobject\n", __func__);
		rc = -ENOMEM;
		goto resource_sysfs_add_exit;
	}

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_resources); i++) {
		rs = kzalloc(sizeof(*rs), GFP_KERNEL);
		if (!rs) {
			pr_err("%s: cannot allocate memory for attributes\n",
				__func__);
			rc = -ENOMEM;
			goto resource_sysfs_add_exit;
		}

		rs->kas.rs = msm_rpmrs_resources[i];
		rs->kas.ka.attr.name = msm_rpmrs_resources[i]->name;
		rs->kas.ka.attr.mode = 0644;
		rs->kas.ka.show = msm_rpmrs_resource_attr_show;
		rs->kas.ka.store = msm_rpmrs_resource_attr_store;

		rs->attrs[0] = &rs->kas.ka.attr;
		rs->attrs[1] = NULL;
		rs->attr_group.attrs = rs->attrs;

		rc = sysfs_create_group(low_power_kboj, &rs->attr_group);
		if (rc) {
			pr_err("%s: cannot create kobject attribute group\n",
				__func__);
			goto resource_sysfs_add_exit;
		}
	}

	rc = 0;

resource_sysfs_add_exit:
	return rc;
}

/******************************************************************************
 * Public Functions
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

int msm_rpmrs_flush_buffer(
	uint32_t sclk_count, struct msm_rpmrs_limits *limits)
{
	struct msm_rpm_iv_pair *req = msm_rpmrs_req;
	int count = ARRAY_SIZE(msm_rpmrs_req);
	int rc;
	int i;

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
	return rc;
}

struct msm_rpmrs_limits *msm_rpmrs_lowest_limits(
	enum msm_pm_sleep_mode sleep_mode, uint32_t latency_us,
	uint32_t sleep_us)
{
	unsigned int cpu = smp_processor_id();
	struct msm_rpmrs_level *best_level = NULL;
	int i;

	for (i = 0; i < ARRAY_SIZE(msm_rpmrs_levels); i++) {
		struct msm_rpmrs_level *level = &msm_rpmrs_levels[i];
		uint32_t power;

		if (!level->available)
			continue;

		if (sleep_mode != level->sleep_mode)
			continue;

		if (latency_us < level->latency_us)
			continue;

		if (sleep_us <= level->time_overhead_us) {
			power = level->energy_overhead / sleep_us;
		} else if ((sleep_us >> 10) > level->time_overhead_us) {
			power = level->steady_state_power;
		} else {
			power = (sleep_us - level->time_overhead_us);
			power *= level->steady_state_power;
			power /= sleep_us;
			power += level->energy_overhead / sleep_us;
		}

		if (!best_level ||
				best_level->rs_limits.power[cpu] >= power) {
			level->rs_limits.latency_us[cpu] = level->latency_us;
			level->rs_limits.power[cpu] = power;
			best_level = level;
		}
	}

	return best_level ? &best_level->rs_limits : NULL;
}

device_initcall(msm_rpmrs_resource_sysfs_add);
