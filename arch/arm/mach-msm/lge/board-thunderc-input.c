/* arch/arm/mach-msm/board-thunderc-input.c
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
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include <mach/rpc_server_handset.h>

#include "board-thunderc.h"

static int prox_power_set(unsigned char onoff);

/* head set device */
static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};

/* None qwerty keypad device
 * For Thunder CDMA Keypad  [ youngchul.park@lge.com ]
 * gpio key pad device - from keypad-surf-ffa */
static unsigned int keypad_row_gpios[] = {
	32, 33
};

static unsigned int keypad_col_gpios[] = {38, 37,36};

#define KEYMAP_INDEX(row, col) ((row)*ARRAY_SIZE(keypad_col_gpios) + (col))

/* change key map for H/W Rev.B -> Rev.C  2010-06-13 younchan,kim
	[Rev.B key map]
static const unsigned short keypad_keymap_thunder[6] = {
	[KEYMAP_INDEX(0, 0)] = KEY_BACK,
	[KEYMAP_INDEX(0, 1)] = KEY_MENU,
	[KEYMAP_INDEX(0, 2)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(1, 0)] = KEY_SEARCH,
	[KEYMAP_INDEX(1, 1)] = KEY_HOME,
	[KEYMAP_INDEX(1, 2)] = KEY_VOLUMEDOWN,
};
*/
/* add Rev.C key map 2010-05-13 younchan.kim */
static const unsigned short keypad_keymap_thunder[6] = {
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX(0, 1)] = KEY_HOME,
	[KEYMAP_INDEX(0, 2)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(1, 0)] = KEY_SEARCH,
	[KEYMAP_INDEX(1, 1)] = KEY_BACK,
	[KEYMAP_INDEX(1, 2)] = KEY_VOLUMEDOWN,
};
static struct gpio_event_matrix_info thunder_keypad_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keypad_keymap_thunder,
	.output_gpios	= keypad_row_gpios,
	.input_gpios	= keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(keypad_col_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static struct gpio_event_info *thunder_keypad_info[] = {
	&thunder_keypad_matrix_info.info
};

static struct gpio_event_platform_data thunder_keypad_data = {
	.name		= "thunder_keypad",
	.info		= thunder_keypad_info,
	.info_count	= ARRAY_SIZE(thunder_keypad_info)
};
struct platform_device keypad_device_thunder= {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &thunder_keypad_data,
	},
};

/* keyreset platform device */
static int thunderc_reset_keys_up[] = {
	KEY_HOME,
	0
};

static struct keyreset_platform_data thunderc_reset_keys_pdata = {
	.keys_up = thunderc_reset_keys_up,
	.keys_down = {
		KEY_BACK,
		KEY_VOLUMEDOWN,
		KEY_MENU,
		0
	},
};

struct platform_device thunderc_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &thunderc_reset_keys_pdata,
};

/* input platform device */
static struct platform_device *thunder_input_devices[] __initdata = {
	&hs_device,
	&keypad_device_thunder,
	&thunderc_reset_keys_device,
};

/* MCS6000 Touch */
static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= TS_GPIO_I2C_SDA,
		.scl_pin	= TS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= TS_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay				= 2,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};

static int ts_set_vreg(unsigned char onoff)
{
	struct vreg *vreg_touch;
	int rc;

	printk("[Touch] %s() onoff:%d\n",__FUNCTION__, onoff);

	vreg_touch = vreg_get(0, "synt");

	if(IS_ERR(vreg_touch)) {
		printk("[Touch] vreg_get fail : touch\n");
		return -1;
	}

	if (onoff) {
		rc = vreg_set_level(vreg_touch, 3050);
		if (rc != 0) {
			printk("[Touch] vreg_set_level failed\n");
			return -1;
		}
		vreg_enable(vreg_touch);
	} else
		vreg_disable(vreg_touch);

	return 0;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power = ts_set_vreg,
	.irq 	  = TS_GPIO_IRQ,
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs6000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs6000",
		.platform_data = &ts_pdata,
	},
};

static void __init thunderc_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&ts_i2c_pdata, ts_i2c_pin[0],	&ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}

/* accelerometer */
static int kr3dh_config_gpio(int config)
{
	if (config)
	{		/* for wake state */
	}
	else
	{		/* for sleep state */
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_INT, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
	}

	return 0;
}

static int kr_init(void)
{
	return 0;
}

static void kr_exit(void)
{
}

static int accel_power_on(void)
{
	int ret = 0;
	struct vreg *gp3_vreg = vreg_get(0, "gp3");

	printk("[Accelerometer] %s() : Power On\n",__FUNCTION__);

	vreg_set_level(gp3_vreg, 3000);
	vreg_enable(gp3_vreg);

	return ret;
}

static int accel_power_off(void)
{
	int ret = 0;
	struct vreg *gp3_vreg = vreg_get(0, "gp3");

	printk("[Accelerometer] %s() : Power Off\n",__FUNCTION__);

	vreg_disable(gp3_vreg);

	return ret;
}

struct kr3dh_platform_data kr3dh_data = {
	.poll_interval = 100,
	.min_interval = 0,
	.g_range = 0x00,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,

	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,

	.power_on = accel_power_on,
	.power_off = accel_power_off,
	.kr_init = kr_init,
	.kr_exit = kr_exit,
	.gpio_config = kr3dh_config_gpio,
};

static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= ACCEL_GPIO_I2C_SDA,
		.scl_pin	= ACCEL_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data accel_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device accel_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &accel_i2c_pdata,
};

static struct i2c_board_info accel_i2c_bdinfo[] = {
	[1] = {
		I2C_BOARD_INFO("KR3DH", ACCEL_I2C_ADDRESS_H),
		.type = "KR3DH",
		.platform_data = &kr3dh_data,
	},
	[0] = {
		I2C_BOARD_INFO("KR3DM", ACCEL_I2C_ADDRESS),
		.type = "KR3DM",
		.platform_data = &kr3dh_data,
	},
};

static void __init thunderc_init_i2c_acceleration(int bus_num)
{
	accel_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);

	if(lge_bd_rev >= LGE_REV_F)
		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[1], 1);	/* KR3DH */
	else
		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[0], 1);	/* KR3DM */

	platform_device_register(&accel_i2c_device);
}

/* proximity & ecompass */
static int ecom_power_set(unsigned char onoff)
{
	int ret = 0;
	struct vreg *gp3_vreg = vreg_get(0, "gp3");
	struct vreg *gp6_vreg = vreg_get(0, "gp6");

	printk("[Ecompass] %s() : Power %s\n",__FUNCTION__, onoff ? "On" : "Off");

	if (onoff) {
		vreg_set_level(gp3_vreg, 3000);
		vreg_enable(gp3_vreg);

		/* proximity power on , when we turn off I2C line be set to low caues sensor H/W characteristic */
		vreg_set_level(gp6_vreg, 2800);
		vreg_enable(gp6_vreg);
	} else {
		vreg_disable(gp3_vreg);

		/* proximity power off */
		vreg_disable(gp6_vreg);
	}

	return ret;
}

static struct ecom_platform_data ecom_pdata = {
	.pin_int        	= ECOM_GPIO_INT,
	.pin_rst		= 0,
	.power          	= ecom_power_set,
};

static int prox_power_set(unsigned char onoff)
{
	int ret = 0;
	struct vreg *gp6_vreg = vreg_get(0, "gp6");

	printk("[Proximity] %s() : Power %s\n",__FUNCTION__, onoff ? "On" : "Off");

	if (onoff) {
		vreg_set_level(gp6_vreg, 2800);
		vreg_enable(gp6_vreg);
	} else {
		vreg_disable(gp6_vreg);
	}

	return ret;
}

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 0,
	.operation_mode		= 0,
	.debounce	 = 0,
	.cycle = 2,
};

static struct i2c_board_info prox_ecom_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("proximity_gp2ap", PROXI_I2C_ADDRESS),
		.type = "proximity_gp2ap",
		.platform_data = &proxi_pdata,
	},
	[1] = {
		I2C_BOARD_INFO("ami304_sensor", ECOM_I2C_ADDRESS),
		.type = "ami304_sensor",
		.platform_data = &ecom_pdata,
	},
};

static struct gpio_i2c_pin proxi_ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= PROXI_GPIO_I2C_SDA,
		.scl_pin	= PROXI_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= PROXI_GPIO_DOUT,
	},
	[1] = {
		.sda_pin	= ECOM_GPIO_I2C_SDA,
		.scl_pin	= ECOM_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data proxi_ecom_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device proxi_ecom_i2c_device = {
        .name = "i2c-gpio",
        .dev.platform_data = &proxi_ecom_i2c_pdata,
};


static void __init thunderc_init_i2c_prox_ecom(int bus_num)
{
	proxi_ecom_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&proxi_ecom_i2c_pdata, proxi_ecom_i2c_pin[0], &prox_ecom_i2c_bdinfo[0]);
	init_gpio_i2c_pin(&proxi_ecom_i2c_pdata, proxi_ecom_i2c_pin[1], &prox_ecom_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, &prox_ecom_i2c_bdinfo[0], 2);
	platform_device_register(&proxi_ecom_i2c_device);
}

/* common function */
void __init lge_add_input_devices(void)
{

	platform_add_devices(thunder_input_devices, ARRAY_SIZE(thunder_input_devices));

	lge_add_gpio_i2c_device(thunderc_init_i2c_touch);
	lge_add_gpio_i2c_device(thunderc_init_i2c_prox_ecom);
	lge_add_gpio_i2c_device(thunderc_init_i2c_acceleration);
}
