/* arch/arm/mach-msm/lge/board-alessi-misc.c
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
#include <mach/msm_rpcrouter.h>
#include "board-alessi.h"

static u32 thunderg_battery_capacity(u32 current_soc)
{
	if(current_soc > 100)
		current_soc = 100;

	return current_soc;
}

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design     = 3200,
	.voltage_max_design     = 4200,
	.avail_chg_sources      = AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity		= thunderg_battery_capacity,
};

static struct platform_device msm_batt_device = {
	.name           = "msm-battery",
	.id         = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

/* Alessi Board Vibrator Functions for Android Vibrator Driver */
#define GPIO_LIN_MOTOR_PWM		28
#define GPIO_LIN_MOTOR_EN       76

#define GP_MN_CLK_MDIV			0x004C
#define GP_MN_CLK_NDIV			0x0050
#define GP_MN_CLK_DUTY			0x0054

/* about 22.93 kHz, should be checked */
#define GPMN_M_DEFAULT			21
#define GPMN_N_DEFAULT			4500
#define GPMN_D_DEFAULT			(GPMN_N_DEFAULT >> 1) 
#define PWM_MULTIPLIER			((GPMN_N_DEFAULT >> 1) - 60) //(GPMN_N_DEFAULT >> 1)
/* default duty cycle = disable motor ic */

#define GPMN_M_MASK				0x01FF
#define GPMN_N_MASK				0x1FFF
#define GPMN_D_MASK				0x1FFF

#define REG_WRITEL(value, reg)	writel(value, (MSM_WEB_BASE+reg))

static struct platform_device msm_device_pmic_leds = {
	.name                           = "pmic-leds",
	.id                                     = -1,
	.dev.platform_data 		= "button-backlight",
};

int alessi_vibrator_power_set(int enable)
{
	static int is_enabled = 0;
	static struct vreg *s_vreg_vibrator;
	int rc;
	if (enable) {
		if (is_enabled) {
			//printk(KERN_INFO "vibrator power was enabled, already\n");
			return 0;
		}

		s_vreg_vibrator = vreg_get(NULL, "rftx");
		rc = vreg_set_level(s_vreg_vibrator, 3000);
		if(rc != 0) {
			return -1;
		}
		vreg_enable(s_vreg_vibrator);
		is_enabled = 1;
	} else {
		if (!is_enabled) {
			//printk(KERN_INFO "vibrator power was disabled, already\n");
			return 0;
		}

		s_vreg_vibrator = vreg_get(NULL, "rftx");
		rc = vreg_set_level(s_vreg_vibrator, 0);
		vreg_disable(s_vreg_vibrator);
		is_enabled = 0;
	}
	return 0;
}

int alessi_vibrator_pwn_set(int enable, int amp)
{
	int gain = ((PWM_MULTIPLIER * amp) >> 7) + GPMN_D_DEFAULT;
	REG_WRITEL((gain & GPMN_D_MASK), GP_MN_CLK_DUTY);
	return 0;
}

int alessi_vibrator_ic_enable_set(int enable)
{
	if(enable)      {
		REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV);
		REG_WRITEL((~(GPMN_N_DEFAULT - GPMN_M_DEFAULT) & GPMN_N_MASK), GP_MN_CLK_NDIV);
		gpio_direction_output(GPIO_LIN_MOTOR_EN, 1);
	} else {
		gpio_direction_output(GPIO_LIN_MOTOR_EN, 0);
	}
	return 0;
}

int thunderg_vibrator_gpio_request(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_LIN_MOTOR_PWM, "lin_motor_pwm");

	if (rc)
		return rc;

	return 0;
}

static struct android_vibrator_platform_data alessi_vibrator_data = {
	.enable_status = 0,	
	.power_set = alessi_vibrator_power_set,
	.pwm_set = alessi_vibrator_pwn_set,
	.ic_enable_set = alessi_vibrator_ic_enable_set,
	.amp_value = 125,
};

static struct platform_device android_vibrator_device = {
	.name   = "android-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &alessi_vibrator_data,
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

struct rpc_snd_set_hook_mode_args {
	uint32_t mode;
	uint32_t cb_func;
	uint32_t client_data;
};

struct snd_set_hook_mode_msg {
	struct rpc_request_hdr hdr;
	struct rpc_snd_set_hook_mode_args args;
};

struct snd_set_hook_param_rep {
	struct rpc_reply_hdr hdr;
	uint32_t get_mode;
};

#define SND_SET_HOOK_MODE_PROC 75
#define RPC_SND_PROG 0x30000002

#define RPC_SND_VERS 0x00020001

static int thunderg_gpio_earsense_work_func(void)
{
	int state;
	int gpio_value;
	struct snd_set_hook_param_rep hkrep;
	struct snd_set_hook_mode_msg hookmsg;
	int rc;

	struct msm_rpc_endpoint *ept = msm_rpc_connect_compatible(RPC_SND_PROG,
							   RPC_SND_VERS, 0);
	if (IS_ERR(ept)) {
		rc = PTR_ERR(ept);
		ept = NULL;
		printk(KERN_ERR"failed to connect snd svc, error %d\n", rc);
	}
	
	hookmsg.args.cb_func = -1;
	hookmsg.args.client_data = 0;

	gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO"%s: ear sense detected : %s\n", __func__, 
			gpio_value?"injected":"ejected");
	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 0);
		hookmsg.args.mode = cpu_to_be32(0);
	} else {
		state = EAR_STATE_INJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 1);
		hookmsg.args.mode = cpu_to_be32(1);
	}

	if(ept) {
		rc = msm_rpc_call_reply(ept,
				SND_SET_HOOK_MODE_PROC,
				&hookmsg, sizeof(hookmsg),&hkrep, sizeof(hkrep), 5 * HZ);
		if (rc < 0){
			printk(KERN_ERR "%s:rpc err because of %d\n", __func__, rc);
		} else {
		    printk("send success\n");
		}
	 } else {
		printk(KERN_ERR "%s:ext_snd is NULL\n", __func__);
	 }
	 rc = msm_rpc_close(ept);
	 if (rc < 0)
		printk(KERN_ERR"msm_rpc_close failed\n");

	return state;
}

static char *thunderg_gpio_earsense_print_state(int state)
{
	return ear_state_string[state];
}

static int thunderg_gpio_earsense_sysfs_store(const char *buf, size_t size)
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

static unsigned thunderg_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

static struct lge_gpio_switch_platform_data thunderg_earsense_data = {
	.name = "h2w",
	.gpios = thunderg_earsense_gpios,
	.num_gpios = ARRAY_SIZE(thunderg_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = thunderg_gpio_earsense_work_func,
	.print_state = thunderg_gpio_earsense_print_state,
	.sysfs_store = thunderg_gpio_earsense_sysfs_store,
};

static struct platform_device thunderg_earsense_device = {
	.name   = "lge-switch-gpio",
	.id = 1,
	.dev = {
		.platform_data = &thunderg_earsense_data,
	},
};

// 20100831 hyuncheol0.kim <Disable the carkit device for Alessi. [START]
static struct platform_device *alessi_misc_devices[] __initdata = {
	&msm_device_pmic_leds,
	&msm_batt_device,
	&android_vibrator_device,
	&thunderg_earsense_device,
};
// 20100831 hyuncheol0.kim <Disable the carkit device for Alessi. [END]

/* main interface */
void __init lge_add_misc_devices(void)
{
// 20100831 hyuncheol0.kim <Disable the carkit device for Alessi. [START]
	//platform_add_devices(thunderg_misc_devices, ARRAY_SIZE(thunderg_misc_devices));
	platform_add_devices(alessi_misc_devices, ARRAY_SIZE(alessi_misc_devices));
// 20100831 hyuncheol0.kim <Disable the carkit device for Alessi. [END]
}

