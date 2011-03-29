/* arch/arm/mach-msm/board-muscat-panel.c
 * Copyright (C) 2009 LGE, Inc.
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

#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <mach/msm_rpcrouter.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include <linux/fb.h>
#include "devices.h"
#include "board-muscat.h"

#define MSM_FB_LCDC_VREG_OP(name, op, level)	\
	do {														\
		vreg = vreg_get(0, name);								\
		vreg_set_level(vreg, level);							\
		if (vreg_##op(vreg))									\
			printk(KERN_ERR "%s: %s vreg operation failed \n",	\
				   (vreg_##op == vreg_enable) ? "vreg_enable"	\
				   : "vreg_disable", name);						\
	} while (0)

static char *msm_fb_vreg[] = {
	"gp1",
	"gp2",
};

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
};

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("ebi2", 0);
}

/* Use pmic_backlight function as power save function, munyoung.hwang@lge.com */
static int mddi_power_save_on;
static int ebi2_tovis_power_save(int on)
{
	struct vreg *vreg;
	int flag_on = !!on;

	printk(KERN_INFO"%s: on=%d\n", __func__, flag_on);

	if (mddi_power_save_on == flag_on)
		return 0;

	mddi_power_save_on = flag_on;

	if (on) {
		/* MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], enable, 1800); */
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], enable, 2800);
	} else{
		/* LGE_CHANGE, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
		//MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], disable, 0);
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], disable, 0);
	}
	return 0;
}

static struct msm_panel_ilitek_pdata ebi2_tovis_panel_data = {
	.gpio = 102,				/* lcd reset_n */
	.lcd_power_save = ebi2_tovis_power_save,
	.maker_id = PANEL_ID_TOVIS,
	.initialized = 1,
};

static struct platform_device ebi2_tovis_panel_device = {
	.name   = "ebi2_tovis_qvga",
	.id     = 0,
	.dev    = {
		.platform_data = &ebi2_tovis_panel_data,
	}
};
/* backlight device */
static struct gpio_i2c_pin bl_i2c_pin[] = {
	[0] = {
		.sda_pin	= 89,
		.scl_pin	= 88,
		.reset_pin	= 82,
		.irq_pin	= 0,
	},
};

static struct i2c_gpio_platform_data bl_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay				= 2,
};

static struct platform_device bl_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &bl_i2c_pdata,
};

static struct aat28xx_platform_data aat2870bl_data = {
	.gpio = 82,
	.version = 2862,
	.initialized = 1,
};

static struct i2c_board_info bl_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("aat2870bl", 0x60),
		.type = "aat2870bl",
		.platform_data = NULL,
	},
};

struct device* muscat_backlight_dev(void)
{
	return &bl_i2c_device.dev;
}

void __init muscat_init_i2c_backlight(int bus_num)
{
	bl_i2c_device.id = bus_num;
	bl_i2c_bdinfo[0].platform_data = &aat2870bl_data;

	init_gpio_i2c_pin(&bl_i2c_pdata, bl_i2c_pin[0],	&bl_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &bl_i2c_bdinfo[0], 1);
	platform_device_register(&bl_i2c_device);
}

static int muscat_fb_event_notify(struct notifier_block *self,
			      unsigned long action, void *data)
{
	struct fb_event *event = data;
	struct fb_info *info = event->info;
	struct fb_var_screeninfo *var = &info->var;
	if(action == FB_EVENT_FB_REGISTERED) {
		var->width = 43;
		var->height = 58;
	}
	return 0;
}

static struct notifier_block muscat_fb_event_notifier = {
	.notifier_call	= muscat_fb_event_notify,
};

/* common functions */
void __init lge_add_lcd_devices(void)
{
	if(ebi2_tovis_panel_data.initialized)
		ebi2_tovis_power_save(1);

	fb_register_client(&muscat_fb_event_notifier);

	platform_device_register(&ebi2_tovis_panel_device);
	msm_fb_add_devices();
	lge_add_gpio_i2c_device(muscat_init_i2c_backlight);
}
