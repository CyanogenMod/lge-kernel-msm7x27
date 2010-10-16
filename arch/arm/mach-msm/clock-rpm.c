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

#define CLK_RESOURCE(id, name) \
	[(id)] = { MSM_RPM_ID_##name##_CLK, MSM_RPM_STATUS_ID_##name##_CLK }

static struct rpm_resource {
	int rpm_clk_id;
	int rpm_status_id;
} resource[] = {
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

int rpm_clk_enable(unsigned id)
{
	/* Not yet supported. */
	return -EPERM;
}

void rpm_clk_disable(unsigned id)
{
	/* Not yet supported. */
}

int rpm_clk_reset(unsigned id, enum clk_reset_action action)
{
	/* Not yet supported. */
	return -EPERM;
}

int rpm_clk_set_rate(unsigned id, unsigned rate)
{
	/* Not yet supported. */
	return -EPERM;
}

int rpm_clk_set_min_rate(unsigned id, unsigned rate)
{
	struct msm_rpm_iv_pair iv = { resource[id].rpm_clk_id, (rate/1000) };

	return msm_rpm_set_noirq(MSM_RPM_CTX_SET_0, &iv, 1);
}

int rpm_clk_set_max_rate(unsigned id, unsigned rate)
{
	/* Not yet supported. */
	return -EPERM;
}

int rpm_clk_set_flags(unsigned id, unsigned flags)
{
	/* Not yet supported. */
	return -EPERM;
}

unsigned rpm_clk_get_rate(unsigned id)
{
	struct msm_rpm_iv_pair iv = { resource[id].rpm_status_id };
	int rc;

	rc  = msm_rpm_get_status(&iv, 1);
	if (rc < 0)
		return rc;
	return iv.value * 1000;
}

signed rpm_clk_measure_rate(unsigned id)
{
	/* Not supported. */
	return -EPERM;
}

unsigned rpm_clk_is_enabled(unsigned id)
{
	/* Not yet supported. */
	return 0;
}

long rpm_clk_round_rate(unsigned id, unsigned rate)
{
	/* Not yet supported. */
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
