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

#include <linux/err.h>
#include <mach/clk.h>

#include "rpm.h"
#include "clock.h"
#include "clock-rpm.h"

static DEFINE_SPINLOCK(rpm_clock_lock);

#define CLK_RESOURCE(id, name) \
	[(id)] = { \
		.rpm_clk_id = MSM_RPM_ID_##name##_CLK, \
		.rpm_status_id = MSM_RPM_STATUS_ID_##name##_CLK, \
	}
static struct rpm_clk {
	int rpm_clk_id;
	int rpm_status_id;
	int requested_khz;
	int count;
} rpm_clk[] = {
	CLK_RESOURCE(R_AFAB_CLK,  APPS_FABRIC),
	CLK_RESOURCE(R_CFPB_CLK,  CFPB),
	CLK_RESOURCE(R_DFAB_CLK,  DAYTONA_FABRIC),
	CLK_RESOURCE(R_EBI1_CLK,  EBI1),
	CLK_RESOURCE(R_MMFAB_CLK, MM_FABRIC),
	CLK_RESOURCE(R_MMFPB_CLK, MMFPB),
	CLK_RESOURCE(R_SFAB_CLK,  SYSTEM_FABRIC),
	CLK_RESOURCE(R_SFPB_CLK,  SFPB),
	CLK_RESOURCE(R_SMI_CLK,   SMI),
};

static int rpm_set(unsigned id, unsigned khz)
{
	struct msm_rpm_iv_pair iv;
	int rc;

	iv.id = rpm_clk[id].rpm_clk_id;
	iv.value = khz;
	rc = msm_rpm_set_noirq(MSM_RPM_CTX_SET_0, &iv, 1);

	return rc;
}

static int rpm_clk_enable(unsigned id)
{
	unsigned long flags;
	int rc = 0;

	spin_lock_irqsave(&rpm_clock_lock, flags);

	if (!rpm_clk[id].count) {
		rpm_clk[id].requested_khz = max(rpm_clk[id].requested_khz, 1);
		rc = rpm_set(id, rpm_clk[id].requested_khz);
	}
	if (!rc)
		rpm_clk[id].count++;

	spin_unlock_irqrestore(&rpm_clock_lock, flags);

	return rc;
}

static void rpm_clk_disable(unsigned id)
{
	unsigned long flags;

	spin_lock_irqsave(&rpm_clock_lock, flags);

	if (rpm_clk[id].count > 0)
		rpm_clk[id].count--;
	else {
		pr_warning("%s: Reference counts are incorrect for clock %d!\n",
			   __func__, id);
		goto out;
	}

	if (!rpm_clk[id].count && rpm_clk[id].requested_khz)
		rpm_set(id, 0);

out:
	spin_unlock_irqrestore(&rpm_clock_lock, flags);

	return;
}

static int rpm_clk_reset(unsigned id, enum clk_reset_action action)
{
	/* Not supported. */
	return -EPERM;
}

static int rpm_clk_set_rate(unsigned id, unsigned rate)
{
	/* Not supported. */
	return -EPERM;
}

static int rpm_clk_set_min_rate(unsigned id, unsigned rate)
{
	unsigned long flags;
	unsigned rate_khz;
	int rc = 0;

	rate_khz = DIV_ROUND_UP(rate, 1000);

	spin_lock_irqsave(&rpm_clock_lock, flags);

	if (rpm_clk[id].requested_khz == rate_khz)
		goto out;

	rc = rpm_set(id, rate_khz);
	if (!rc)
		rpm_clk[id].requested_khz = rate_khz;

out:
	spin_unlock_irqrestore(&rpm_clock_lock, flags);

	return rc;
}

static int rpm_clk_set_max_rate(unsigned id, unsigned rate)
{
	/* Not supported. */
	return -EPERM;
}

static int rpm_clk_set_flags(unsigned id, unsigned flags)
{
	/* Not supported. */
	return -EPERM;
}

static unsigned rpm_clk_get_rate(unsigned id)
{
	struct msm_rpm_iv_pair iv = { rpm_clk[id].rpm_status_id };
	int rc;

	rc  = msm_rpm_get_status(&iv, 1);
	if (rc < 0)
		return rc;
	return iv.value * 1000;
}

static int rpm_clk_measure_rate(unsigned id)
{
	/* Not supported. */
	return -EPERM;
}

static unsigned rpm_clk_is_enabled(unsigned id)
{
	return !!(rpm_clk_get_rate(id));
}

static long rpm_clk_round_rate(unsigned id, unsigned rate)
{
	/* Not supported. */
	return rate;
}

struct clk_ops clk_ops_remote = {
	.enable = rpm_clk_enable,
	.disable = rpm_clk_disable,
	.output_enable = rpm_clk_enable,
	.output_disable = rpm_clk_disable,
	.reset = rpm_clk_reset,
	.set_rate = rpm_clk_set_rate,
	.set_min_rate = rpm_clk_set_min_rate,
	.set_max_rate = rpm_clk_set_max_rate,
	.set_flags = rpm_clk_set_flags,
	.get_rate = rpm_clk_get_rate,
	.measure_rate = rpm_clk_measure_rate,
	.is_enabled = rpm_clk_is_enabled,
	.round_rate = rpm_clk_round_rate,
};
