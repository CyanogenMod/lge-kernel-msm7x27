/* arch/arm/mach-msm/lge/board-thunderc-misc.c
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
#include "board-thunderc.h"

#if defined(CONFIG_LGE_FUEL_GAUGE)
static u32 thunderc_battery_capacity(u32 current_soc)
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
#if defined(CONFIG_LGE_FUEL_GAUGE)
	.calculate_capacity     = thunderc_battery_capacity,
#endif
};

static struct platform_device msm_batt_device = {
	.name           = "msm-battery",
	.id         = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};

/* Vibrator Functions for Android Vibrator Driver */
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
#define PWM_MAX_HALF_DUTY		((GPMN_N_DEFAULT >> 1) - 80) /* minimum operating spec. should be checked */

#define GPMN_M_MASK				0x01FF
#define GPMN_N_MASK				0x1FFF
#define GPMN_D_MASK				0x1FFF

#define REG_WRITEL(value, reg)	writel(value, (MSM_WEB_BASE+reg))

extern int aat2870bl_ldo_set_level(struct device * dev, unsigned num, unsigned vol);
extern int aat2870bl_ldo_enable(struct device * dev, unsigned num, unsigned enable);

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

static void thunderc_desk_dock_detect_callback(int state)
{
	int ret;

	if (state)
		state = DOCK_STATE_DESK;

	ret = lge_gpio_switch_pass_event("dock", state);

	if (ret)
		printk(KERN_INFO "%s: desk dock event report fail\n", __func__);

	return;
}

static int thunderc_register_callback(void)
{
	rpc_server_hs_register_callback(thunderc_desk_dock_detect_callback);

	return 0;
}

static int thunderc_gpio_carkit_work_func(void)
{
	return DOCK_STATE_UNDOCKED;
}

static char *thunderc_gpio_carkit_print_state(int state)
{
	return dock_state_string[state];
}

static int thunderc_gpio_carkit_sysfs_store(const char *buf, size_t size)
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

static unsigned thunderc_carkit_gpios[] = {
};

static struct lge_gpio_switch_platform_data thunderc_carkit_data = {
	.name = "dock",
	.gpios = thunderc_carkit_gpios,
	.num_gpios = ARRAY_SIZE(thunderc_carkit_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = thunderc_gpio_carkit_work_func,
	.print_state = thunderc_gpio_carkit_print_state,
	.sysfs_store = thunderc_gpio_carkit_sysfs_store,
	.additional_init = thunderc_register_callback,
};

static struct platform_device thunderc_carkit_device = {
	.name = "lge-switch-gpio",
	.id = 0,
	.dev = {
		.platform_data = &thunderc_carkit_data,
	},
};
int thunderc_vibrator_power_set(int enable)
{
	static int is_enabled = 0;
	struct device *dev = thunderc_backlight_dev();

	if (dev==NULL) {
		printk(KERN_ERR "%s: backlight devive get failed\n", __FUNCTION__);
		return -1;
	}

	if (enable) {
		if (is_enabled) {
			//printk(KERN_INFO "vibrator power was enabled, already\n");
			return 0;
		}
		/* 3300 mV for Motor IC */				
		if (aat28xx_ldo_set_level(dev, 1, VIBE_IC_VOLTAGE) < 0) {
			printk(KERN_ERR "%s: vibrator LDO set failed\n", __FUNCTION__);
			return -EIO;
		}
		if (aat28xx_ldo_enable(dev, 1, 1) < 0) {
			printk(KERN_ERR "%s: vibrator LDO enable failed\n", __FUNCTION__);
			return -EIO;
		}
		is_enabled = 1;
	} else {
		if (!is_enabled) {
			//printk(KERN_INFO "vibrator power was disabled, already\n");
			return 0;
		}
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

int thunderc_vibrator_pwm_set(int enable, int amp)
{
	int gain = ((PWM_MAX_HALF_DUTY*amp) >> 7)+ GPMN_D_DEFAULT;

	REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV_REG);
	REG_WRITEL((~( GPMN_N_DEFAULT - GPMN_M_DEFAULT )&GPMN_N_MASK), GP_MN_CLK_NDIV_REG);

	if (enable) {
		REG_WRITEL((gain & GPMN_D_MASK), GP_MN_CLK_DUTY_REG);
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 1);
	} else {
		REG_WRITEL(GPMN_D_DEFAULT, GP_MN_CLK_DUTY_REG);
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 0);
	}

	return 0;
}

int thunderc_vibrator_ic_enable_set(int enable)
{
	/* nothing to do, thunder does not using Motor Enable pin */
	return 0;
}

static struct android_vibrator_platform_data thunderc_vibrator_data = {
	.enable_status = 0,
	.power_set = thunderc_vibrator_power_set,
	.pwm_set = thunderc_vibrator_pwm_set,
	.ic_enable_set = thunderc_vibrator_ic_enable_set,
	.amp_value = 105,
};

static struct platform_device android_vibrator_device = {
	.name   = "android-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &thunderc_vibrator_data,
	},
};

/* add led device for VS660 Rev.D by  younchan.kim 2010-05-27  */
static void pmic_mpp_isink_set(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	int mpp_number;
	int on_off;
	int i;
	int brightness;

	if (!strcmp(led_cdev->name ,"button-backlight"))
		mpp_number = (int)PM_MPP_19;
	else
		return;
	printk (" LED DEBUG: Value is %d \n",value);
	if(value == 0)
		on_off = (int)PM_MPP__I_SINK__SWITCH_DIS;
	else
		on_off = (int)PM_MPP__I_SINK__SWITCH_ENA;

	switch(value){
		case 5:
			brightness = PM_MPP__I_SINK__LEVEL_5mA;
			break;
		case 10 :
			brightness = PM_MPP__I_SINK__LEVEL_10mA;
			break;
		case 15 :
			brightness = PM_MPP__I_SINK__LEVEL_15mA;
			break;
		case 20 :
			brightness = PM_MPP__I_SINK__LEVEL_20mA;
			break;
		case 25 :
			brightness = PM_MPP__I_SINK__LEVEL_25mA;
			break;
		case 30 :
			brightness = PM_MPP__I_SINK__LEVEL_30mA;
			break;
		case 35 :
			brightness = PM_MPP__I_SINK__LEVEL_35mA;
			break;
		case 40 :
			brightness = PM_MPP__I_SINK__LEVEL_40mA;
			break;
		default :
			brightness = PM_MPP__I_SINK__LEVEL_15mA;
			break;
	}

	for(i=0; i<4; i++){
		pmic_secure_mpp_config_i_sink((enum mpp_which)mpp_number,
			brightness, (enum mpp_i_sink_switch)on_off);
		mpp_number++;
	}
}

struct led_classdev thunderc_custom_leds[] = {
	{
		.name = "button-backlight",
		.brightness_set = pmic_mpp_isink_set,
		.brightness = LED_OFF,
	},
};

static int register_leds(struct platform_device *pdev)
{
	int rc;
	int i;

	for(i = 0 ; i < ARRAY_SIZE(thunderc_custom_leds) ; i++) {
		rc = led_classdev_register(&pdev->dev, &thunderc_custom_leds[i]);
		if (rc) {
			dev_err(&pdev->dev, "unable to register led class driver : thunderc_custom_leds \n");
			return rc;
		}
		pmic_mpp_isink_set(&thunderc_custom_leds[i], LED_OFF);
	}

	return rc;
}

static void unregister_leds (void)
{
	int i;
	for (i = 0; i< ARRAY_SIZE(thunderc_custom_leds); ++i)
		led_classdev_unregister(&thunderc_custom_leds[i]);
}

static void suspend_leds (void)
{
	int i;
	for (i = 0; i< ARRAY_SIZE(thunderc_custom_leds); ++i)
		led_classdev_suspend(&thunderc_custom_leds[i]);
}

static void resume_leds (void)
{
	int i;
	for (i = 0; i< ARRAY_SIZE(thunderc_custom_leds); ++i)
		led_classdev_resume(&thunderc_custom_leds[i]);
}

int keypad_led_set(unsigned char value)
{
	int ret;

	ret = pmic_set_led_intensity(LED_KEYPAD, value);

	return ret;
}

static struct msm_pmic_leds_pdata leds_pdata = {
	.custom_leds		= thunderc_custom_leds,
	.register_custom_leds	= register_leds,
	.unregister_custom_leds	= unregister_leds,
	.suspend_custom_leds	= suspend_leds,
	.resume_custom_leds	= resume_leds,
	.msm_keypad_led_set	= keypad_led_set,
};

static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
	.dev.platform_data = &leds_pdata,
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

static int thunderc_hs_mic_bias_power(int enable)
{
	struct vreg *hs_bias_vreg;
	static int is_enabled = 0;

	hs_bias_vreg = vreg_get(NULL, "ruim");

	if (IS_ERR(hs_bias_vreg)) {
		printk(KERN_ERR "%s: vreg_get failed\n", __FUNCTION__);
		return PTR_ERR(hs_bias_vreg);
	}

	if (enable) {
		if (is_enabled) {
			//printk(KERN_INFO "HS Mic. Bias power was enabled, already\n");
			return 0;
		}

		if (vreg_set_level(hs_bias_vreg, 2600) <0) {
			printk(KERN_ERR "%s: vreg_set_level failed\n", __FUNCTION__);
			return -EIO;
		}

		if (vreg_enable(hs_bias_vreg) < 0 ) {
			printk(KERN_ERR "%s: vreg_enable failed\n", __FUNCTION__);
			return -EIO;
		}
		is_enabled = 1;
	} else {
		if (!is_enabled) {
			//printk(KERN_INFO "HS Mic. Bias power was disabled, already\n");
			return 0;
		}

		if (vreg_set_level(hs_bias_vreg, 0) <0) {
			printk(KERN_ERR "%s: vreg_set_level failed\n", __FUNCTION__);
			return -EIO;
		}

		if (vreg_disable(hs_bias_vreg) < 0) {
			printk(KERN_ERR "%s: vreg_disable failed\n", __FUNCTION__);
			return -EIO;
		}
		is_enabled = 0;
	}
	return 0;
}

static int thunderc_gpio_earsense_work_func(void)
{
	int state;
	int gpio_value;

	gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO"%s: ear sense detected : %s\n", __func__, 
			gpio_value?"injected":"ejected");
	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		/* LGE_CHANGE_S, [junyoub.an] , 2010-05-28, comment out to control at ARM9 part*/
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 0);
		/* LGE_CHANGE_E, [junyoub.an] , 2010-05-28, comment out to control at ARM9 part*/
	} else {
		state = EAR_STATE_INJECT;
		/* LGE_CHANGE_S, [junyoub.an] , 2010-05-28, comment out to control at ARM9 part*/
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 1);
		/* LGE_CHANGE_E, [junyoub.an] , 2010-05-28, comment out to control at ARM9 part*/
	}

	return state;
}

static char *thunderc_gpio_earsense_print_state(int state)
{
	return ear_state_string[state];
}

static int thunderc_gpio_earsense_sysfs_store(const char *buf, size_t size)
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

static unsigned thunderc_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

static struct lge_gpio_switch_platform_data thunderc_earsense_data = {
	.name = "h2w",
	.gpios = thunderc_earsense_gpios,
	.num_gpios = ARRAY_SIZE(thunderc_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = thunderc_gpio_earsense_work_func,
	.print_state = thunderc_gpio_earsense_print_state,
	.sysfs_store = thunderc_gpio_earsense_sysfs_store,
};

static struct platform_device thunderc_earsense_device = {
	.name   = "lge-switch-gpio",
	.id = 1,
	.dev = {
		.platform_data = &thunderc_earsense_data,
	},
};

static struct platform_device *thunderc_misc_devices[] __initdata = {
	/* LGE_CHANGE
	 * ADD VS740 BATT DRIVER IN THUNDERC
	 * 2010-05-13, taehung.kim@lge.com
	 */
	&msm_batt_device, 
	&android_vibrator_device,
	&thunderc_carkit_device,
	&thunderc_earsense_device,
};

void __init lge_add_misc_devices(void)
{
	platform_add_devices(thunderc_misc_devices, ARRAY_SIZE(thunderc_misc_devices));
	if(lge_bd_rev >= LGE_REV_D)
		platform_device_register(&msm_device_pmic_leds);
}

