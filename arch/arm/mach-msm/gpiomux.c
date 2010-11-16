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
	gpiomux_config_t *cfgs[GPIOMUX_CFG_MAX];
	int              ref;
};
static DEFINE_SPINLOCK(gpiomux_lock);
static struct msm_gpiomux_rec *msm_gpiomux_recs;
static gpiomux_config_t *msm_gpiomux_cfgs;
static unsigned msm_gpiomux_ngpio;

int msm_gpiomux_write(unsigned gpio, enum msm_gpiomux_cfg_type which,
	gpiomux_config_t *cfg)
{
	struct msm_gpiomux_rec *rec = msm_gpiomux_recs + gpio;
	unsigned cfg_slot = gpio * GPIOMUX_CFG_MAX + which;
	unsigned long irq_flags;
	gpiomux_config_t *new_config;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);

	if (cfg) {
		msm_gpiomux_cfgs[cfg_slot] = *cfg;
		rec->cfgs[which] = &msm_gpiomux_cfgs[cfg_slot];
	} else {
		rec->cfgs[which] = NULL;
	}

	new_config = rec->ref ? rec->cfgs[GPIOMUX_CFG_ACTIVE] :
		rec->cfgs[GPIOMUX_CFG_SUSPENDED];
	if (new_config)
		__msm_gpiomux_write(gpio, *new_config);

	spin_unlock_irqrestore(&gpiomux_lock, irq_flags);
	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_write);

int msm_gpiomux_get(unsigned gpio)
{
	struct msm_gpiomux_rec *rec = msm_gpiomux_recs + gpio;
	unsigned long irq_flags;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);
	if (rec->ref++ == 0 && rec->cfgs[GPIOMUX_CFG_ACTIVE])
		__msm_gpiomux_write(gpio, *rec->cfgs[GPIOMUX_CFG_ACTIVE]);
	spin_unlock_irqrestore(&gpiomux_lock, irq_flags);
	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_get);

int msm_gpiomux_put(unsigned gpio)
{
	struct msm_gpiomux_rec *rec = msm_gpiomux_recs + gpio;
	unsigned long irq_flags;

	if (!msm_gpiomux_recs)
		return -EFAULT;

	if (gpio >= msm_gpiomux_ngpio)
		return -EINVAL;

	spin_lock_irqsave(&gpiomux_lock, irq_flags);
	BUG_ON(rec->ref == 0);
	if (--rec->ref == 0 && rec->cfgs[GPIOMUX_CFG_SUSPENDED])
		__msm_gpiomux_write(gpio, *rec->cfgs[GPIOMUX_CFG_SUSPENDED]);
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

	/* There is no need to zero this memory, as clients will be blindly
	 * installing configs on top of it.
	 */
	msm_gpiomux_cfgs = kmalloc(sizeof(gpiomux_config_t) * ngpio *
		GPIOMUX_CFG_MAX, GFP_KERNEL);
	if (!msm_gpiomux_cfgs) {
		kfree(msm_gpiomux_recs);
		msm_gpiomux_recs = NULL;
		return -ENOMEM;
	}

	msm_gpiomux_ngpio = ngpio;

	return 0;
}
EXPORT_SYMBOL(msm_gpiomux_init);

void msm_gpiomux_install(struct msm_gpiomux_config *configs, unsigned nconfigs)
{
	unsigned n, cfg;
	int rc;

	for (n = 0; n < nconfigs; ++n) {
		for (cfg = 0; cfg < GPIOMUX_CFG_MAX; ++cfg) {
			rc = msm_gpiomux_write(configs[n].gpio, cfg,
				&configs[n].configs[cfg]);
			if (rc)
				pr_err("%s: write failure: %d\n", __func__, rc);
		}
	}
}
EXPORT_SYMBOL(msm_gpiomux_install);
