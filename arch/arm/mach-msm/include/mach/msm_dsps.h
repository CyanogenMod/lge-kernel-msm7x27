/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _MSM_DSPS_H_
#define _MSM_DSPS_H_

#include <linux/types.h>
#include <linux/clk.h>

#define DSPS_SIGNATURE	0x12345678

/**
 * DSPS Clocks Platform data.
 *
 * @name - clock name.
 * @rate - rate to set. zero if not relevant.
 * @clock - clock handle, reserved for the driver.
 */
struct dsps_clk_info {
	const char *name;
	u32 rate;
	struct clk *clock;
};

/**
 * DSPS GPIOs Platform data.
 *
 * @name - clock name.
 * @num - GPIO number.
 * @on_val - value to ouptput for ON (depends on polarity).
 * @off_val - value to ouptput for OFF (depends on polarity).
 * @is_owner - reserved for the driver.
 */
struct dsps_gpio_info {
	const char *name;
	int num;
	int on_val;
	int off_val;
	int is_owner;
};

/**
 * DSPS Platform data.
 *
 * @clks - array of clocks.
 * @clks_num - number of clocks in array.
 * @gpios - array of gpios.
 * @gpios_num - number of gpios.
 * @signature - signature for validity check.
 */
struct msm_dsps_platform_data {
	struct dsps_clk_info *clks;
	int clks_num;
	struct dsps_gpio_info *gpios;
	int gpios_num;
	u32 signature;
};

#endif /* _MSM_DSPS_H_ */
