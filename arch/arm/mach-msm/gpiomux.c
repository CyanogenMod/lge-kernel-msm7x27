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
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "gpiomux.h"

struct msm_gpiomux_rec {
	gpiomux_config_t active;
	gpiomux_config_t suspended;
	int              ref;
};
static DEFINE_SPINLOCK(gpiomux_lock);
static struct msm_gpiomux_rec *msm_gpiomux_recs;
static unsigned msm_gpiomux_ngpio;

int msm_gpiomux_write(unsigned gpio,
		      gpiomux_config_t active,
		      gpiomux_config_t suspended)
{
	struct msm_gpiomux_rec *cfg = msm_gpiomux_recs + gpio;
	unsigned long irq_flags;
	gpiomux_config_t setting;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);

	if (active & GPIOMUX_VALID)
		cfg->active = active;

	if (suspended & GPIOMUX_VALID)
		cfg->suspended = suspended;

	setting = cfg->ref ? active : suspended;
	if (setting & GPIOMUX_VALID)
		__msm_gpiomux_write(gpio, setting);

	spin_unlock_irqrestore(&gpiomux_lock, irq_flags);
	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_write);

int msm_gpiomux_get(unsigned gpio)
{
	struct msm_gpiomux_rec *cfg = msm_gpiomux_recs + gpio;
	unsigned long irq_flags;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);
	if (cfg->ref++ == 0 && cfg->active & GPIOMUX_VALID)
		__msm_gpiomux_write(gpio, cfg->active);
	spin_unlock_irqrestore(&gpiomux_lock, irq_flags);
	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_get);

int msm_gpiomux_put(unsigned gpio)
{
	struct msm_gpiomux_rec *cfg = msm_gpiomux_recs + gpio;
	unsigned long irq_flags;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);
	BUG_ON(cfg->ref == 0);
	if (--cfg->ref == 0 && cfg->suspended & GPIOMUX_VALID)
		__msm_gpiomux_write(gpio, cfg->suspended);
	spin_unlock_irqrestore(&gpiomux_lock, irq_flags);
	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_put);

int msm_gpiomux_init(size_t ngpio)
{
	if (!ngpio)
		return -EINVAL;

	if (msm_gpiomux_recs)
		return -EPERM;

	msm_gpiomux_recs = kzalloc(sizeof(struct msm_gpiomux_rec) * ngpio,
				   GFP_KERNEL);
	if (!msm_gpiomux_recs)
		return -ENOMEM;

	msm_gpiomux_ngpio = ngpio;

	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_init);

void msm_gpiomux_install(struct msm_gpiomux_config *configs, unsigned nconfigs)
{
	unsigned n;
	int rc;

	if (!msm_gpiomux_recs)
		return;

	for (n = 0; n < nconfigs; ++n) {
		rc = msm_gpiomux_write(configs[n].gpio,
				       configs[n].active,
				       configs[n].suspended);
		if (rc)
			pr_err("%s: write failure: %d\n", __func__, rc);
	}
}
EXPORT_SYMBOL(msm_gpiomux_install);
