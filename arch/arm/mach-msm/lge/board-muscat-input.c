/* arch/arm/mach-msm/board-muscat-input.c
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

#define ADWARDK_DEBUG

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
#include <mach/pmic.h>
#include "proc_comm.h"

#include "board-muscat.h"

/* pp2106 qwerty keyboard device */
static unsigned short pp2106_keycode[PP2106_KEYPAD_ROW][PP2106_KEYPAD_COL] = {
	{KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U },
	{KEY_I, KEY_O, KEY_P, KEY_A, KEY_S, KEY_D, KEY_F },
	{KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_DEL, KEY_LEFTALT },
	{KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M },
	{KEY_COMMA, KEY_ENTER, KEY_LEFTSHIFT, KEY_ATSIGN, KEY_LANG, KEY_SPACE, KEY_DOT }, 
	{KEY_FUNC1, KEY_FUNC2, KEY_MENU, KEY_HOME, KEY_BACK, KEY_SEARCH, KEY_UNKNOWN },
	{KEY_UNKNOWN, KEY_UNKNOWN, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN },
};

static int pp2106_set_power(unsigned char onoff)
{
	int rc = -EINVAL;
	static int is_enabled = 0;
	struct vreg *vreg_gp6;

	if (lge_bd_rev < LGE_REV_C)
		return 0;

	
	vreg_gp6 = vreg_get(NULL, "gp6");
	
	if (IS_ERR(vreg_gp6)) {
		printk(KERN_ERR "%s: vreg_get(%s) failed (%ld)\n",
			__func__, "gp6", PTR_ERR(vreg_gp6));
		return PTR_ERR(vreg_gp6);
	}

	if (onoff) {
		if (is_enabled) {
			return 0;
		}
		
		rc = vreg_set_level(vreg_gp6, 2600);
		if (rc) {
			printk(KERN_ERR "%s: GP6 vreg_set_level failed (%d)\n",
				__func__, rc);
			return -EIO;
		}
		
		rc = vreg_enable(vreg_gp6);
		if (rc) {
			printk(KERN_ERR "%s: GP6 vreg_enable failed (%d)\n",
				__func__, rc);
			return -EIO;
		}

		is_enabled = 1;
	} else {
		if (!is_enabled) {
			return 0;
		}

		rc = vreg_disable(vreg_gp6);
		if (rc) {
			printk(KERN_ERR "%s: GP6 vreg_disable failed (%d)\n",
				__func__, rc);
			return -EIO;
		}

		is_enabled = 0;
	}
	
	return 0;
}

static struct pp2106_platform_data pp2106_pdata = {
	.keypad_row = PP2106_KEYPAD_ROW,
	.keypad_col = PP2106_KEYPAD_COL,
	.keycode = (unsigned char *)pp2106_keycode,
	.reset_pin = GPIO_PP2106_RESET,
	.irq_pin = GPIO_PP2106_IRQ,
	.sda_pin = GPIO_PP2106_SDA,
	.scl_pin = GPIO_PP2106_SCL,
	.power = pp2106_set_power,
};

static struct platform_device qwerty_device = {
	.name = "kbd_pp2106",
	.id = -1,
	.dev = {
		.platform_data = &pp2106_pdata,
	},
};


/* LGE_S [ynj.kim@lge.com] 2010-05-15 : atcmd virtual device */
static unsigned short atcmd_virtual_keycode[ATCMD_VIRTUAL_KEYPAD_ROW][ATCMD_VIRTUAL_KEYPAD_COL] = {
	{KEY_1, 		KEY_8, 				KEY_Q,  	 KEY_I,          KEY_D,      	KEY_HOME,	KEY_B,          KEY_UP},
	{KEY_2, 		KEY_9, 		  		KEY_W,		 KEY_O,       	 KEY_F,		 	KEY_RIGHTSHIFT, 	KEY_N,			KEY_DOWN},
	{KEY_3, 		KEY_0, 		  		KEY_E,		 KEY_P,          KEY_G,      	KEY_Z,        	KEY_M, 			KEY_UNKNOWN},
	{KEY_4, 		KEY_BACK,  			KEY_R,		 KEY_SEARCH,     KEY_H,			KEY_X,    		KEY_LEFTSHIFT,	KEY_UNKNOWN},
	{KEY_5, 		KEY_BACKSPACE, 		KEY_T,		 KEY_LEFTALT,    KEY_J,      	KEY_C,     		KEY_REPLY,    KEY_CAMERA},
	{KEY_6, 		KEY_ENTER,  		KEY_Y,  	 KEY_A,		     KEY_K,			KEY_V,  	    KEY_RIGHT,     	KEY_CAMERAFOCUS},
	{KEY_7, 		KEY_MENU,	KEY_U,  	 KEY_S,    		 KEY_L, 	    KEY_SPACE,      KEY_LEFT,     	KEY_SEND},
	{KEY_UNKNOWN, 	KEY_UNKNOWN,  		KEY_UNKNOWN, KEY_UNKNOWN, 	 KEY_UNKNOWN,	KEY_UNKNOWN,    KEY_FOLDER_MENU,      	KEY_FOLDER_HOME},

};

static struct atcmd_virtual_platform_data atcmd_virtual_pdata = {
	.keypad_row = ATCMD_VIRTUAL_KEYPAD_ROW,
	.keypad_col = ATCMD_VIRTUAL_KEYPAD_COL,
	.keycode = (unsigned char *)atcmd_virtual_keycode,
};

static struct platform_device atcmd_virtual_device = {
	.name = "atcmd_virtual_kbd",
	.id = -1,
	.dev = {
		.platform_data = &atcmd_virtual_pdata,
	},
};
/* LGE_E [ynj.kim@lge.com] 2010-05-15 : atcmd virtual device */

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

/*
 * 2011-02-05, jinkyu.choi@lge.com
 * GPIO key map for muscat Rev.B
 */
static unsigned int keypad_row_gpios[] = {
	37, 38
};

static unsigned int keypad_col_gpios[] = {26, 32, 58};

#define KEYMAP_INDEX(col, row) ((col)*ARRAY_SIZE(keypad_row_gpios) + (row))

static const unsigned short keypad_keymap_muscat[] = {
	[KEYMAP_INDEX(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX(0, 1)] = KEY_BACK,
	[KEYMAP_INDEX(1, 0)] = KEY_HOME,
	[KEYMAP_INDEX(1, 1)] = KEY_SEARCH,
	[KEYMAP_INDEX(2, 0)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(2, 1)] = KEY_VOLUMEUP,
};

int muscat_matrix_info_wrapper(struct gpio_event_input_devs *input_dev,struct gpio_event_info *info, void **data, int func)
{
        int ret ;
		if (func == GPIO_EVENT_FUNC_RESUME) {
			gpio_tlmm_config(GPIO_CFG(keypad_row_gpios[0], 0,
						GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			gpio_tlmm_config(GPIO_CFG(keypad_row_gpios[1], 0,
						GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		}

		ret = gpio_event_matrix_func(input_dev,info, data,func);
        return ret ;
}

static int muscat_gpio_matrix_power(
                const struct gpio_event_platform_data *pdata, bool on)
{
	/* this is dummy function to make gpio_event driver register suspend function
	 * 2010-01-29, cleaneye.kim@lge.com
	 * copy from ALOHA code
	 * 2010-04-22 younchan.kim@lge.com
	 */

	return 0;
}

static struct gpio_event_matrix_info muscat_keypad_matrix_info = {
	.info.func	= muscat_matrix_info_wrapper,
	.keymap		= NULL,
	.output_gpios	= keypad_col_gpios,
	.input_gpios	= keypad_row_gpios,
	.noutputs	= ARRAY_SIZE(keypad_col_gpios),
	.ninputs	= ARRAY_SIZE(keypad_row_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_DRIVE_INACTIVE
};

static void __init muscat_select_keymap(void)
{
	muscat_keypad_matrix_info.keymap = keypad_keymap_muscat;

	return;
}

static struct gpio_event_info *muscat_keypad_info[] = {
	&muscat_keypad_matrix_info.info
};

static struct gpio_event_platform_data muscat_keypad_data = {
	.name		= "muscat_keypad",
	.info		= muscat_keypad_info,
	.info_count	= ARRAY_SIZE(muscat_keypad_info),
	.power          = muscat_gpio_matrix_power,
};

struct platform_device keypad_device_muscat= {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &muscat_keypad_data,
	},
};

/* keyreset platform device */
static int muscat_reset_keys_up[] = {
	KEY_HOME,
	0
};

static struct keyreset_platform_data muscat_reset_keys_pdata = {
	.keys_up = muscat_reset_keys_up,
	.keys_down = {
		//KEY_BACK,
		KEY_END,
		KEY_VOLUMEDOWN,
		0
	},
};

struct platform_device muscat_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &muscat_reset_keys_pdata,
};

/* input platform device */
static struct platform_device *muscat_input_devices[] __initdata = {
	&hs_device,
	//&muscat_reset_keys_device,
	&qwerty_device,
	&atcmd_virtual_device,
};

static struct platform_device *muscat_gpio_input_devices[] __initdata = {
	&keypad_device_muscat,/* the gpio keypad for muscat Rev.B */
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
	.udelay			= 1,
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

		if (lge_bd_rev <= LGE_REV_C)
			gpio_set_value(28, 1);

		rc = vreg_set_level(vreg_touch, 3050);
		if (rc != 0) {
			printk("[Touch] vreg_set_level failed\n");
			return -1;
		}
		vreg_enable(vreg_touch);
	} else {
		if (lge_bd_rev <= LGE_REV_C)
			gpio_set_value(28, 0);
		vreg_disable(vreg_touch);
	}

	return 0;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power 	  = ts_set_vreg,
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

/* this routine should be checked for nessarry */
static int init_gpio_i2c_pin_touch(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin, struct i2c_board_info *i2c_board_info_data)
{
	i2c_adap_pdata->sda_pin = gpio_i2c_pin.sda_pin;
	i2c_adap_pdata->scl_pin = gpio_i2c_pin.scl_pin;

	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.sda_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.scl_pin, 0, GPIO_CFG_OUTPUT,
				GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(gpio_i2c_pin.sda_pin, 1);
	gpio_set_value(gpio_i2c_pin.scl_pin, 1);

	if (gpio_i2c_pin.reset_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.reset_pin, 0, GPIO_CFG_OUTPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(gpio_i2c_pin.reset_pin, 1);
	}

	if (gpio_i2c_pin.irq_pin) {
		gpio_tlmm_config(GPIO_CFG(gpio_i2c_pin.irq_pin, 0, GPIO_CFG_INPUT,
					GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		i2c_board_info_data->irq =
			MSM_GPIO_TO_INT(gpio_i2c_pin.irq_pin);
	}

	if (lge_bd_rev <= LGE_REV_C)
		gpio_tlmm_config(GPIO_CFG(28, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	return 0;
}

static void __init muscat_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin_touch(&ts_i2c_pdata, ts_i2c_pin[0], &ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}

/** accelerometer x**/
static int accel_power(unsigned char onoff)
{
	int ret = 0;
	struct vreg *gp3_vreg = vreg_get(0, "gp3");
/* LGE_CHANGE_S [adwardk.kim@lge.com] 2011-02-07 */
//	struct vreg *msmp_vreg = vreg_get(0, "msmp");
	int rc;
	unsigned on_off=0, id;
	
	if (onoff) {
#ifdef ADWARDK_DEBUG
		printk(KERN_INFO "accel_power_on\n");
#endif
		on_off = 0;
		id = PM_VREG_PDOWN_GP3_ID;
		msm_proc_comm(PCOM_VREG_PULLDOWN, &on_off, &id);
		vreg_disable(gp3_vreg);
		
		rc = vreg_set_level(gp3_vreg, 3000);
		if (rc != 0) {
			printk("[Accel] vreg_set_level failed\n");
			return -1;
		}
		vreg_enable(gp3_vreg);
	} 
	else {
#ifdef ADWARDK_DEBUG
		printk(KERN_INFO "accel_power_off\n");
#endif
		vreg_disable(gp3_vreg);
		on_off = 1;
		id = PM_VREG_PDOWN_GP3_ID;
		msm_proc_comm(PCOM_VREG_PULLDOWN, &on_off, &id);
	}
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-02-07 */
	return ret;
}

struct acceleration_platform_data bma250 = {
	.power = accel_power,
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
	[0] = {
		I2C_BOARD_INFO("bma250", ACCEL_I2C_ADDRESS),
		.type = "bma250",
		.platform_data = &bma250,
	},
};

static void __init muscat_init_i2c_acceleration(int bus_num)
{
	accel_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);

	i2c_register_board_info(bus_num, &accel_i2c_bdinfo[0], 1);

	platform_device_register(&accel_i2c_device);
}


/* ecompass */
static int ecom_power_set(unsigned char onoff)
{
	int ret = 0;
	struct vreg *gp3_vreg = vreg_get(0, "gp3");

	if (onoff) {
#ifdef ADWARDK_DEBUG
		printk(KERN_INFO "ecom_power_on\n");
#endif
		vreg_set_level(gp3_vreg, 3000);
		vreg_enable(gp3_vreg);
	} else {
#ifdef ADWARDK_DEBUG
		printk(KERN_INFO "ecom_power_off\n");
#endif
		vreg_disable(gp3_vreg);
	}
/* LGE_CHANGE_E [adwardk.kim@lge.com] 2011-02-07 */
	return ret;
}

static struct ecom_platform_data ecom_pdata = {
	.pin_int        	= ECOM_GPIO_INT,
	.pin_rst		= 0,
	.power          	= ecom_power_set,
};

static struct i2c_board_info ecom_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("ami304_sensor", ECOM_I2C_ADDRESS),
		.type = "ami304_sensor",
		.platform_data = &ecom_pdata,
	},
};

static struct gpio_i2c_pin ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= ECOM_GPIO_I2C_SDA,
		.scl_pin	= ECOM_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data ecom_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device ecom_i2c_device = {
        .name = "i2c-gpio",
        .dev.platform_data = &ecom_i2c_pdata,
};


static void __init muscat_init_i2c_ecom(int bus_num)
{
	ecom_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&ecom_i2c_pdata, ecom_i2c_pin[0], &ecom_i2c_bdinfo[0]);

	i2c_register_board_info(bus_num, &ecom_i2c_bdinfo[0], 1);
	platform_device_register(&ecom_i2c_device);
}

/* common function */
void __init lge_add_input_devices(void)
{
	muscat_select_keymap();
	platform_add_devices(muscat_input_devices, ARRAY_SIZE(muscat_input_devices));

	if (lge_bd_rev >= LGE_REV_B)
		platform_add_devices(muscat_gpio_input_devices, ARRAY_SIZE(muscat_gpio_input_devices));

	lge_add_gpio_i2c_device(muscat_init_i2c_touch);
	lge_add_gpio_i2c_device(muscat_init_i2c_ecom);
	lge_add_gpio_i2c_device(muscat_init_i2c_acceleration);
}

