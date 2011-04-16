/* arch/arm/mach-msm/lge/board-jump-misc.c
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

#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/msm_battery.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <asm/io.h>
#include <mach/rpc_server_handset.h>
#include <mach/board_lge.h>
#include "board-jump.h"


static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design 	= 2800,
	.voltage_max_design	= 4300,
	.avail_chg_sources   	= AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
};

static struct platform_device msm_batt_device = {
	.name           = "msm-battery",
	.id         = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};


/* jump Board Vibrator Functions for Android Vibrator Driver */
static uint motor_voltage = 3000;

extern int aat2870bl_ldo_set_level(struct device * dev, unsigned num, unsigned vol);
extern int aat2870bl_ldo_enable(struct device * dev, unsigned num, unsigned enable);

static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
	.dev.platform_data = "button-backlight",
};

/*
 * for muscat model using the DC motor
 * the voltage level should be optimized, later
 * by jinkyu.choi@lge.com
 */
int jump_vibrator_power_set(int enable)
{
	static int is_enabled = 0;
	struct device *dev = jump_backlight_dev();

	if (dev==NULL) {
		printk(KERN_ERR "%s: backlight devive get failed\n", __FUNCTION__);
		return -1;
	}

	if (enable) {
		if (aat28xx_ldo_set_level(dev, 1, motor_voltage) < 0) {
			printk(KERN_ERR "%s: vibrator LDO set failed\n", __FUNCTION__);
			return -EIO;
		}
		
		if (aat28xx_ldo_enable(dev, 1, 1) < 0) {
			printk(KERN_ERR "%s: vibrator LDO enable failed\n", __FUNCTION__);
			return -EIO;
		}

		//pmic_vib_mot_set_volt(motor_voltage);
		printk("%s: vibrator volage %d\n", __FUNCTION__, motor_voltage);

		is_enabled = 1;
	} else {
		if (!is_enabled) {
			//printk(KERN_INFO "vibrator power was disabled, already\n");
			return 0;
		}
		
		//pmic_vib_mot_set_volt(0);

		if (aat28xx_ldo_set_level(dev, 1, 0) < 0) {		
			printk(KERN_ERR "%s: vibrator LDO set failed\n", __FUNCTION__);
			return -EIO;
		}
		
		if (aat28xx_ldo_enable(dev, 1, 0) < 0) {
			printk(KERN_ERR "%s: vibrator LDO disable failed\n", __FUNCTION__);
			return -EIO;
		}

		is_enabled = 0;
	}
	return 0;
}

int jump_vibrator_pwm_set(int enable, int amp)
{
	/* for test, jinkyu.choi@lge.com */
   if (amp >= 100)
	   motor_voltage = 3300;
   else if (amp >= 90)
	   motor_voltage = 3200;
   else if (amp >= 80)
	   motor_voltage = 3100;
   else if (amp >= 70)
	   motor_voltage = 3000;
   else if (amp >= 60)
	   motor_voltage = 2900;
   else if (amp >= 50)
	   motor_voltage = 2800;
   else if (amp >= 40)
	   motor_voltage = 2700;
   else if (amp >= 30)
	   motor_voltage = 2600;
   else if (amp >= 20)
	   motor_voltage = 2500;
   else if (amp >= 10)
	   motor_voltage = 2200;
   else
	   motor_voltage = 0;

	return 0;
}

int jump_vibrator_ic_enable_set(int enable)
{
	/* nothing to do, thunder does not using Motor Enable pin */
	return 0;
}

static struct android_vibrator_platform_data jump_vibrator_data = {
	.enable_status = 0,	
	.power_set = jump_vibrator_power_set,
	.pwm_set = jump_vibrator_pwm_set,
	.ic_enable_set = jump_vibrator_ic_enable_set,
	.amp_value = 92,
};

static struct platform_device android_vibrator_device = {
	.name   = "android-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &jump_vibrator_data,
	},
};

/* ear sense driver */
static char *ear_state_string[] = {
	"0",
	"1",
};

enum {
	EAR_STATE_EJECT = 0,
	EAR_STATE_INJECT = 1, 
};

enum {
	EAR_EJECT = 0,
	EAR_INJECT = 1,
};

static int jump_gpio_earsense_work_func(void)
{
	int state;
	int gpio_value;
	
	gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO"%s: ear sense detected : %s\n", __func__, 
			gpio_value?"injected":"ejected");
	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		pmic_mic_en(0);
	} else {
		state = EAR_STATE_INJECT;
		pmic_mic_set_volt(MIC_VOLT_1_80V);
		pmic_mic_en(1);
	}

	return state;
}

static char *jump_gpio_earsense_print_state(int state)
{
	return ear_state_string[state];
}

static int jump_gpio_earsense_sysfs_store(const char *buf, size_t size)
{
	int state;

	if (!strncmp(buf, "eject", size - 1))
		state = EAR_STATE_EJECT;
	else if (!strncmp(buf, "inject", size - 1))
		state = EAR_STATE_INJECT;
	else
		return -EINVAL;

	return state;
}

static unsigned jump_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

static struct lge_gpio_switch_platform_data jump_earsense_data = {
	.name = "h2w",
	.gpios = jump_earsense_gpios,
	.num_gpios = ARRAY_SIZE(jump_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = jump_gpio_earsense_work_func,
	.print_state = jump_gpio_earsense_print_state,
	.sysfs_store = jump_gpio_earsense_sysfs_store,
};

static struct platform_device jump_earsense_device = {
	.name   = "lge-switch-gpio",
	.id = 1,
	.dev = {
		.platform_data = &jump_earsense_data,
	},
};

/* misc platform devices */
static struct platform_device *jump_misc_devices[] __initdata = {
	&msm_batt_device,
	&android_vibrator_device,
	&jump_earsense_device,
};

/* main interface */
void __init lge_add_misc_devices(void)
{
	motor_voltage = 3000;
	muscat_vibrator_data.amp_value = 70;

	platform_add_devices(jump_misc_devices, ARRAY_SIZE(jump_misc_devices));
	platform_device_register(&msm_device_pmic_leds);
}

