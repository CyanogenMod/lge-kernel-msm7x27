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
#ifndef __ARCH_ARM_MACH_MSM_GPIOMUX_H
#define __ARCH_ARM_MACH_MSM_GPIOMUX_H

#include <linux/bitops.h>
#include <linux/errno.h>

#if defined(CONFIG_MSM_V2_TLMM)
#include "gpiomux-v2.h"
#else
#include "gpiomux-v1.h"
#endif

enum msm_gpiomux_cfg_type {
	GPIOMUX_CFG_ACTIVE = 0,
	GPIOMUX_CFG_SUSPENDED,
	GPIOMUX_CFG_MAX
};

/**
 * struct msm_gpiomux_config: gpiomux settings for one gpio line.
 *
 * A complete gpiomux config is the bitwise-or of a drive-strength,
 * function, and pull.  For functions other than GPIO, the OE
 * is hard-wired according to the function.  For GPIO mode,
 * OE is controlled by gpiolib.
 *
 * Available settings differ by target; see the gpiomux header
 * specific to your target arch for available configurations.
 *
 * @gpio: The index number of the gpio being described.
 * @configs: The configurations to be installed, specifically:
 *           GPIOMUX_CFG_ACTIVE: The configuration to be installed when the
 *           line is active, or its reference count is > 0.
 *           GPIOMUX_CFG_SUSPENDED: The configuration to be installed when
 *           the line is suspended, or its reference count is 0.
 */
struct msm_gpiomux_config {
	unsigned         gpio;
	gpiomux_config_t configs[GPIOMUX_CFG_MAX];
};

#ifdef CONFIG_MSM_GPIOMUX

/* Before using gpiomux, initialize the subsystem by telling it how many
 * gpios are going to be managed.  Calling any other gpiomux functions before
 * msm_gpiomux_init is unsupported.
 */
int msm_gpiomux_init(size_t ngpio);

/* Install a block of gpiomux configurations in gpiomux.  This is functionally
 * identical to calling msm_gpiomux_write many times.
 */
void msm_gpiomux_install(struct msm_gpiomux_config *configs, unsigned nconfigs);

/* Increment a gpio's reference count, possibly activating the line. */
int __must_check msm_gpiomux_get(unsigned gpio);

/* Decrement a gpio's reference count, possibly suspending the line. */
int msm_gpiomux_put(unsigned gpio);

/* Install a new configuration to the gpio line.  To erase a slot, use NULL.
 */
int msm_gpiomux_write(unsigned gpio, enum msm_gpiomux_cfg_type which,
	gpiomux_config_t *cfg);

/* Architecture-internal function for use by the framework only.
 * This function can assume the following:
 * - the gpio value has passed a bounds-check
 * - the gpiomux spinlock has been obtained
 *
 * This function is not for public consumption.  External users
 * should use msm_gpiomux_write.
 */
void __msm_gpiomux_write(unsigned gpio, gpiomux_config_t val);
#else
static inline int msm_gpiomux_init(size_t ngpio)
{
	return -ENOSYS;
}

static inline void
msm_gpiomux_install(struct msm_gpiomux_config *configs, unsigned nconfigs) {}

static inline int __must_check msm_gpiomux_get(unsigned gpio)
{
	return -ENOSYS;
}

static inline int msm_gpiomux_put(unsigned gpio)
{
	return -ENOSYS;
}

static inline int msm_gpiomux_write(unsigned gpio,
	enum msm_gpiomux_cfg_type which, gpiomux_config_t *cfg)
{
	return -ENOSYS;
}
#endif
#endif
