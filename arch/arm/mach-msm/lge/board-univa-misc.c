/* arch/arm/mach-msm/lge/board-univa-misc.c
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
#include "board-univa.h"
#if defined(UNIVA_EVB)
#include <../drivers/video/backlight/lp8720.h>  
#endif

#ifdef CONFIG_LGE_FUEL_GAUGE
static u32 univa_battery_capacity(u32 current_soc)
{
	if(current_soc > 100)
		current_soc = 100;

	return current_soc;
}
#endif

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design     = 3200,
	.voltage_max_design     = 4200,
	.avail_chg_sources      = AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
#ifdef CONFIG_LGE_FUEL_GAUGE
	.calculate_capacity		= univa_battery_capacity,
#endif
};

static struct platform_device msm_batt_device = {
	.name           = "msm-battery",
	.id         = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

/* univa Board Vibrator Functions for Android Vibrator Driver */
#define VIBE_IC_VOLTAGE			3300
#define GPIO_LIN_MOTOR_PWM		28

#define GP_MN_CLK_MDIV_REG		0x004C
#define GP_MN_CLK_NDIV_REG		0x0050
#define GP_MN_CLK_DUTY_REG		0x0054

/* about 22.93 kHz, should be checked */
#define GPMN_M_DEFAULT			21
#define GPMN_N_DEFAULT			4500
/* default duty cycle = disable motor ic */
#define GPMN_D_DEFAULT			(GPMN_N_DEFAULT >> 1) 
#define PWM_MAX_HALF_DUTY		((GPMN_N_DEFAULT >> 1) - 60) /* minimum operating spec. should be checked */

#define GPMN_M_MASK				0x01FF
#define GPMN_N_MASK				0x1FFF
#define GPMN_D_MASK				0x1FFF

#define REG_WRITEL(value, reg)	writel(value, (MSM_WEB_BASE+reg))

#if defined(UNIVA_EVB)
extern void subpm_output_enable(void);
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
#endif

static char *dock_state_string[] = {
	"0",
	"1",
	"2",
};

enum {
	DOCK_STATE_UNDOCKED = 0,
	DOCK_STATE_DESK = 1, /* multikit */
	DOCK_STATE_CAR = 2, /* carkit */
	DOCK_STATE_UNKNOWN,
};

enum {
	KIT_DOCKED = 0,
	KIT_UNDOCKED = 1,
};

static void univa_desk_dock_detect_callback(int state)
{
	int ret;

	if (state)
		state = DOCK_STATE_DESK;

	ret = lge_gpio_switch_pass_event("dock", state);

	if (ret)
		printk(KERN_INFO "%s: desk dock event report fail\n", __func__);

	return;
}

static int univa_register_callback(void)
{
	rpc_server_hs_register_callback(univa_desk_dock_detect_callback);

	return 0;
}

static int univa_gpio_carkit_work_func(void)
{
	return DOCK_STATE_UNDOCKED;
}

static char *univa_gpio_carkit_print_state(int state)
{
	return dock_state_string[state];
}

static int univa_gpio_carkit_sysfs_store(const char *buf, size_t size)
{
	int state;

	if (!strncmp(buf, "undock", size-1))
		state = DOCK_STATE_UNDOCKED;
	else if (!strncmp(buf, "desk", size-1))
		state = DOCK_STATE_DESK;
	else if (!strncmp(buf, "car", size-1))
		state = DOCK_STATE_CAR;
	else
		return -EINVAL;

	return state;
}

static unsigned univa_carkit_gpios[] = {
};

static struct lge_gpio_switch_platform_data univa_carkit_data = {
	.name = "dock",
	.gpios = univa_carkit_gpios,
	.num_gpios = ARRAY_SIZE(univa_carkit_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = univa_gpio_carkit_work_func,
	.print_state = univa_gpio_carkit_print_state,
	.sysfs_store = univa_gpio_carkit_sysfs_store,
	.additional_init = univa_register_callback,
};

static struct platform_device univa_carkit_device = {
	.name = "lge-switch-gpio",
	.id = 0,
	.dev = {
		.platform_data = &univa_carkit_data,
	},
};

static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
	.dev.platform_data = "button-backlight",
};

int univa_vibrator_power_set(int enable)
{
#if defined(UNIVA_EVB)
	static int is_enabled = 0;
	
	if(enable)
	{
		if(is_enabled)
		{
			//printk(KERN_INFO "vibrator power was enabled, already\n");
			return 0;
		}
		
		/* 3300 mV for Motor IC */
		gpio_direction_output(LP8720_ENABLE, 1);
		subpm_set_output(LDO1, 1);
		subpm_output_enable();
		is_enabled = 1;
	}
	else
	{
		if(!is_enabled)
		{
			//printk(KERN_INFO "vibrator power was disabled, already\n");
			return 0;
		}
		
		subpm_set_output(LDO1, 0);
		subpm_output_enable();
		gpio_direction_output(LP8720_ENABLE, 0);
		is_enabled = 0;
	}
#else
	if(enable)
	{
		if(pmic_vib_mot_set_volt(3100) < 0)
			printk("LGE: motor power on fail\n");
	}
	else
	{
		if(pmic_vib_mot_set_volt(0) < 0)
			printk("LGE: motor power off fail\n");
	}
#endif
	
	return 0;
}

int univa_vibrator_pwm_set(int enable, int amp)
{
#if defined(UNIVA_EVB)
	int gain = ((PWM_MAX_HALF_DUTY*amp) >> 7)+ GPMN_D_DEFAULT;

	REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV_REG);
	REG_WRITEL((~( GPMN_N_DEFAULT - GPMN_M_DEFAULT )&GPMN_N_MASK), GP_MN_CLK_NDIV_REG);
		
	if (enable)
	{		
		// hj74.kim@lge.com 2010/07/09 LAB1_FW : HW 요청 사항.
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		REG_WRITEL((gain & GPMN_D_MASK), GP_MN_CLK_DUTY_REG);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 1);
	}
	else
	{
		REG_WRITEL(GPMN_D_DEFAULT, GP_MN_CLK_DUTY_REG);
		// hj74.kim@lge.com 2010/07/09 LAB1_FW : HW 요청 사항.
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 0);		
	}
#endif	
	
	return 0;
}

int univa_vibrator_ic_enable_set(int enable)
{
	/* nothing to do, thunder does not using Motor Enable pin */
	return 0;
}

static struct android_vibrator_platform_data univa_vibrator_data = {
	.enable_status = 0,	
	.power_set = univa_vibrator_power_set,
	.pwm_set = univa_vibrator_pwm_set,
	.ic_enable_set = univa_vibrator_ic_enable_set,
	.amp_value = 115,
};

static struct platform_device android_vibrator_device = {
	.name   = "android-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &univa_vibrator_data,
	},
};

//LGE_SND_UPDATE_S [
#if 0
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

static int univa_gpio_earsense_work_func(void)
{
	int state;
	int gpio_value;
	
	gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO"%s: ear sense detected : %s\n", __func__, 
			gpio_value?"injected":"ejected");
	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 0);
	} else {
		state = EAR_STATE_INJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 1);
	}

	return state;
}

static char *univa_gpio_earsense_print_state(int state)
{
	return ear_state_string[state];
}

static int univa_gpio_earsense_sysfs_store(const char *buf, size_t size)
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

static unsigned univa_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

static struct lge_gpio_switch_platform_data univa_earsense_data = {
	.name = "h2w",
	.gpios = univa_earsense_gpios,
	.num_gpios = ARRAY_SIZE(univa_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = univa_gpio_earsense_work_func,
	.print_state = univa_gpio_earsense_print_state,
	.sysfs_store = univa_gpio_earsense_sysfs_store,
};

static struct platform_device univa_earsense_device = {
	.name   = "lge-switch-gpio",
	.id = 1,
	.dev = {
		.platform_data = &univa_earsense_data,
	},
};
#endif
//LGE_SND_UPDATE_E ]

/* misc platform devices */
static struct platform_device *univa_misc_devices[] __initdata = {
	&msm_batt_device,
	&android_vibrator_device,
	&univa_carkit_device,
//LGE_SND_UPDATE_S [
//	&univa_earsense_device,
//LGE_SND_UPDATE_E ]
};

/* main interface */
void __init lge_add_misc_devices(void)
{
	platform_add_devices(univa_misc_devices, ARRAY_SIZE(univa_misc_devices));
	platform_device_register(&msm_device_pmic_leds);
}

