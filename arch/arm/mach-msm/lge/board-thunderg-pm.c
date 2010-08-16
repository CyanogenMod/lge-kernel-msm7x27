/* arch/arm/mach-msm/lge/board-thunderg-pm.c
 * Copyright (C) 2010 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/platform_device.h>
#include <linux/pm.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include "board-thunderg.h"

int thunderg_pwrsink_suspend_noirq(struct device *dev)
{
	printk(KERN_INFO"%s: configure gpio for suspend\n", __func__);

	return 0;
}

int thunderg_pwrsink_resume_noirq(struct device *dev)
{
	printk(KERN_INFO"%s: configure gpio for resume\n", __func__);

	return 0;
}

static struct dev_pm_ops thunderg_pwrsink_data = {
	.suspend_noirq = thunderg_pwrsink_suspend_noirq,
	.resume_noirq = thunderg_pwrsink_resume_noirq,
};

static struct platform_device thunderg_pwrsink_device = {
	.name = "lge-pwrsink",
	.id = -1,
	.dev = {
		.platform_data = &thunderg_pwrsink_data,
	},
};

void __init lge_add_pm_devices(void)
{
	platform_device_register(&thunderg_pwrsink_device);
}

