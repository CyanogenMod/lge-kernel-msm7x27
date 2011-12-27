/* drivers/video/backlight/lm3530_bl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/board_lge.h>

#define MODULE_NAME  "lm3530bl"
#define CONFIG_BACKLIGHT_LEDS_CLASS

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/********************************************
 * Definition
 ********************************************/

/* LED current (0~31, unit = mA) */
/* 0.45, 0.90, 1.80, 2.70, 3.60, 4.50, 5.40, 6.30, 7.20, 8.10, */
/* 9.00, 9.90, 10.8, 11.7, 12.6, 13.5, 14.4, 15.3, 16.2, 17.1, */
/* 18.0, 18.9, 19.8, 20.7, 21.6, 22.5, 23.4, 24.3, 25.2, 26.1, */
/* 27.0, 27.9 */

/* LGE_CHANGE
  * AAT2862 has two parts of LEDs(Main and Sub)
  * Added some definitions and modified I2C write command to control both Main and Sub LEDs.
  * Added 'AAT2862BL_REG_BLS', changed members of structure 'lm3530_reg_addrs'
  * and modified 'lm3530_write' to control two registers(AAT2862BL_REG_BLM and AAT2862BL_REG_BLS)
  * 2010-04-22, minjong.gong@lge.com
  */

/* LGE_CHANGE
  * If MEQS bit in AAT2862BL_REG_BLM is set, we don't need to write command to AAT2862BL_REG_BLS.
  * So modify command array for AAT2862 and related functions. 
  * And change default brightness and maximum brightness.
  * 2010-05-18, minjong.gong@lge.com
  */

#define LCD_LED_MAX 0x7F
#define LCD_LED_MIN 0

#define DEFAULT_BRIGHTNESS 0x4A // linear
#define DEFAULT_BRIGHTNESS_EXPONENTIAL 0x6B // exponential

#define LM3530_MIN_BRIGHTNESS	0x4F
#define LM3530_DEFAULT_BRIGHTNESS	0x6B

#define AAT2862BL_REG_BLS   0x04  /* Register address for Main BL brightness */
#define AAT2862BL_REG_FADE	0x07  /* Register address for Backlight Fade control */

#define LM3530BL_REG_GENERAL_CONFIGURATION		0x10	/* Register address for General Configuration
										1. Simple Interface Enable
										2. PWM Polarity
										3. PWM enable
										4. Full Scale Current Selection
										5. Brightness Mapping Mode Select
										6. I2C Device Enable */
#define LM3530BL_REG_ALS_CONFIGURATION		0x20	/* Register address for ALS Configuration
										1. ALS Current Control Enable
										2. ALS Input Enable
										3. ALS Input Select
										4. ALS Averaging Times */
#define LM3530BL_REG_BRIGHTNESS_RAMP_RATE	0x30	/* Register address for Brightness Ramp Rate
										Programs the rate of rise and fall of the LED current */
#define LM3530BL_REG_ALS_ZONE_INFORMATION	0x40	/* Register address for BALS Zone Information
										1. Zone Boundary Change Flag
										2. Zone Brightness Information */
#define LM3530BL_REG_ALS_REGISTOR_SLECT	0x41	/* Register address for ALS Resistor Select
										Iinternal ALS1 and ALS2 Resistances */
#define LM3530BL_REG_BRIGHTNESS_CONTROL	0xA0	/* Register address for Brightness Control (BRT)
										Holds the 7 bit Brightness Data */
#define LM3530BL_REG_ZB0	0x60	/* Register address for Zone Boundary 0 (ZB0)
										ALS Zone Boundry #0 */
#define LM3530BL_REG_ZB1	0x61	/* Register address for Zone Boundary 1 (ZB1)
										ALS Zone Boundry #1 */
#define LM3530BL_REG_ZB2	0x62	/* Register address for Zone Boundary 2 (ZB2)
										ALS Zone Boundry #2 */
#define LM3530BL_REG_ZB3	0x63	/* Register address for Zone Boundary 3 (ZB3)
										ALS Zone Boundry #3 */
#define LM3530BL_REG_Z0T	0x70	/* Register address for Zone Target 0 (Z0T)
										Zone 0 LED Current Data. The LED Current Source transitions to the
										brightness code in Z0T when the ALS_ input is less than the zone
										boundary programmed in ZB0. */
#define LM3530BL_REG_Z1T	0x71	/* Register address for Zone Target 1 (Z1T)
										Zone 1 LED Current Data. The LED Current Source transitions to the
										brightness code in Z1T when the ALS_ input is less than the zone
										boundary programmed in ZB1 and ZB0. */
#define LM3530BL_REG_Z2T	0x72	/* Register address for Zone Target 2 (Z2T)
										Zone 2 LED Current Data. The LED Current Source transitions to the
										brightness code in Z2T when the ALS_ input is less than the zone
										boundary programmed in ZB2 and ZB1. */
#define LM3530BL_REG_Z3T	0x73	/* Register address for Zone Target 3 (Z3T)
										Zone 3 LED Current Data. The LED Current Source transitions to the
										brightness code in Z3T when the ALS_ input is less than the zone
										boundary programmed in ZB3 and ZB2. */
#define LM3530BL_REG_Z4T	0x74	/* Register address for Zone Target 4 (Z4T)
										Zone 4 LED Current Data. The LED Current Source transitions to the
										brightness code in Z4T when the ALS_ input is less than the zone
										boundary programmed in ZB4 and ZB3. */

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#endif

enum {
	ALC_MODE,
	NORMAL_MODE,
} LM3530BL_MODE;

enum {
	UNINIT_STATE=-1,
	POWERON_STATE,
	NORMAL_STATE,
	SLEEP_STATE,
	POWEROFF_STATE
} LM3530BL_STATE;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

struct lm3530_ctrl_tbl {
	unsigned char reg;
	unsigned char val;
};

struct lm3530_reg_addrs {
	unsigned char bl_m;
	unsigned char bl_s;
	unsigned char fade;
};

struct lm3530_cmds {
	struct lm3530_ctrl_tbl *normal;
	struct lm3530_ctrl_tbl *alc;
	struct lm3530_ctrl_tbl *sleep;
};

struct lm3530_driver_data {
	struct i2c_client *client;
	struct backlight_device *bd;
	struct led_classdev *led;
	int gpio;
	int intensity;
	int max_intensity;
	int mode;
	int state;
	int version;
	struct lm3530_cmds cmds;
	struct lm3530_reg_addrs reg_addrs;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static int exponential_eable;
/********************************************
 * Global variable
 ********************************************/
static unsigned int debug = 1;
module_param(debug, uint, 0644);

/* Set to Normal mode */
static struct lm3530_ctrl_tbl lm3530_normal_tbl[] = {
	{ LM3530BL_REG_GENERAL_CONFIGURATION, 0x17 },   // Full Scale Current Select 22.5mA, linear
	{ 0xFF, 0xFE }	 /* end of command */
};

static struct lm3530_ctrl_tbl lm3530_normal_tbl_exponential[] = {
	{ LM3530BL_REG_GENERAL_CONFIGURATION, 0x15 },   // Full Scale Current Select 22.5mA, exponential
	{ 0xFF, 0xFE }	 /* end of command */
};

/* Set to ALC mode */
static struct lm3530_ctrl_tbl lm3530_alc_tbl[] = {
	// AAT2862 has no ALC mode !!
	{ 0xFF, 0xFE }   /* end or command */
};

/* Set to sleep mode */
static struct lm3530_ctrl_tbl lm3530_sleep_tbl[] = {
	{ LM3530BL_REG_BRIGHTNESS_CONTROL, 0x00 },
	{ LM3530BL_REG_GENERAL_CONFIGURATION, 0x00 },
	{ 0xFF, 0xFE },  /* end of command */	
};

/********************************************
 * Functions
 ********************************************/
static int lm3530_setup_version(struct lm3530_driver_data *drvdata)
{
	if(!drvdata)
		return -ENODEV;

	if(exponential_eable)
		drvdata->cmds.normal = lm3530_normal_tbl_exponential;
	else
		drvdata->cmds.normal = lm3530_normal_tbl;
	drvdata->cmds.alc = lm3530_alc_tbl;
	drvdata->cmds.sleep = lm3530_sleep_tbl;
	drvdata->reg_addrs.bl_m = LM3530BL_REG_BRIGHTNESS_CONTROL;
	drvdata->reg_addrs.bl_s = AAT2862BL_REG_BLS;
	drvdata->reg_addrs.fade = AAT2862BL_REG_FADE;

	return 0;
}

static int lm3530_read(struct i2c_client *client, u8 reg, u8 *pval)
{
	int ret;
	int status = 0;

	if (client == NULL) { 	/* No global client pointer? */
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		status = -EIO;
		eprintk("fail to read(reg=0x%x,val=0x%x)\n", reg,*pval);	
	}

	*pval = ret;
	return status;
}

static int lm3530_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	int status = 0;

    // test dr.ryu
//  	dprintk("---------------> write(reg=0x%x,val=0x%x)\n", reg, val);
	if (client == NULL) {	/* No global client pointer? */
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		eprintk("fail to write(reg=0x%x,val=0x%x)\n", reg, val);
	}

	return status;
}

static int lm3530_set_table(struct lm3530_driver_data *drvdata, struct lm3530_ctrl_tbl *ptbl)
{
	unsigned int i = 0;
	unsigned long delay = 0;

	if (ptbl == NULL) {
		eprintk("input ptr is null\n");
		return -EIO;
	}

	for( ;;) {
		if (ptbl->reg == 0xFF) {
			if (ptbl->val != 0xFE) {
				delay = (unsigned long)ptbl->val;
				udelay(delay);
			}
			else
				break;
		}	
		else {
			if (lm3530_write(drvdata->client, ptbl->reg, ptbl->val) != 0)
				dprintk("i2c failed addr:%d, value:%d\n", ptbl->reg, ptbl->val);
		}
		ptbl++;
		i++;
	}
	return 0;
}

static void lm3530_hw_reset(struct lm3530_driver_data *drvdata)
{
	if (drvdata->client && gpio_is_valid(drvdata->gpio)) {
		gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		/* EN set to LOW(shutdown) -> HIGH(enable) */
		gpio_set_value(drvdata->gpio, 1);
		udelay(10);
		gpio_set_value(drvdata->gpio, 0);
		udelay(10);
		gpio_set_value(drvdata->gpio, 1);
		udelay(10);
	}
}

static void lm3530_go_opmode(struct lm3530_driver_data *drvdata)
{
	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			lm3530_set_table(drvdata, drvdata->cmds.normal);
			drvdata->state = NORMAL_STATE;
			break;
		case ALC_MODE:
			/* LGE_CHANGE
			 * Remove ALC mode
			 * 2010-07-26. minjong.gong@lge.com
			 */
			//lm3530_set_table(drvdata, drvdata->cmds.alc);
			//drvdata->state = NORMAL_STATE;
			//break;
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void lm3530_device_init(struct lm3530_driver_data *drvdata)
{
/* LGE_CHANGE.
  * Do not initialize aat28xx when system booting. The aat28xx is already initialized in oemsbl or LK !!
  * 2010-08-16, minjong.gong@lge.com
  */
	if (system_state == SYSTEM_BOOTING) {
		lm3530_go_opmode(drvdata);
		return;
	}
	lm3530_hw_reset(drvdata);
	lm3530_go_opmode(drvdata);
}

static void lm3530_poweron(struct lm3530_driver_data *drvdata)
{
//	unsigned int lm3530_intensity;
	if (!drvdata || drvdata->state != POWEROFF_STATE)
		return;
	
	dprintk("POWER ON \n");

	lm3530_device_init(drvdata);
	
	if (drvdata->mode == NORMAL_MODE)
	{
		lm3530_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	}
}

#if 0
static void lm3530_poweroff(struct lm3530_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == POWEROFF_STATE)
		return;

	dprintk("POWER OFF \n");

	if (drvdata->state == SLEEP_STATE) {
		gpio_direction_output(drvdata->gpio, 0);
		msleep(6);
		drvdata->state = POWEROFF_STATE;
		return;
	}

	gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_direction_output(drvdata->gpio, 0);
	mdelay(6);
	drvdata->state = POWEROFF_STATE;
}
#endif

/* This function provide sleep enter routine for power management. */
extern void bl_config_gpio(int config);
static void lm3530_sleep(struct lm3530_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == SLEEP_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			drvdata->state = SLEEP_STATE;
			lm3530_set_table(drvdata, drvdata->cmds.sleep);
			break;

		case ALC_MODE:
			/* LGE_CHANGE
			 * Remove ALC mode
			 * 2010-07-26. minjong.gong@lge.com
			 */
			//drvdata->state = SLEEP_STATE;
			//lm3530_set_table(drvdata, drvdata->cmds.sleep);
			//udelay(500);
			//break;

		default:
			eprintk("Invalid Mode\n");
			break;
	}

	if (drvdata->client && gpio_is_valid(drvdata->gpio)) {
		gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		bl_config_gpio(0);
		udelay(10);
		gpio_set_value(drvdata->gpio, 0);
	}	
}

static void lm3530_wakeup(struct lm3530_driver_data *drvdata)
{
//	unsigned int lm3530_intensity;

	if (!drvdata || drvdata->state == NORMAL_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");

	bl_config_gpio(1);
	lm3530_hw_reset(drvdata);

	if (drvdata->state == POWEROFF_STATE) {
		lm3530_poweron(drvdata);
		/* LGE_CHANGE
		 * Because the lm3530_go_opmode is called in the lm3530_poweron above, so I remove below function.
		 * If it is called two times when the previous state of AAT2862 is POWEROFF_STATE, it causes malfucction.
		 * 2010-07-31. minjong.gong@lge.com
		 */
		//lm3530_go_opmode(drvdata);
	} else if (drvdata->state == SLEEP_STATE) {
		if (drvdata->mode == NORMAL_MODE) {
				lm3530_set_table(drvdata, drvdata->cmds.normal);
				lm3530_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
			drvdata->state = NORMAL_STATE;
		} else if (drvdata->mode == ALC_MODE) {
			/* LGE_CHANGE
			 * Remove ALC mode
			 * 2010-07-26. minjong.gong@lge.com
			 */
			//lm3530_set_table(drvdata, drvdata->cmds.alc);
			//drvdata->state = NORMAL_STATE;
		}
	}
}

static int lm3530_send_intensity(struct lm3530_driver_data *drvdata, int next)
{
//	int lm3530_bl_next;

	if (drvdata->mode == NORMAL_MODE) {
		if (next > drvdata->max_intensity)
			next = drvdata->max_intensity;
		if (next < LCD_LED_MIN)
			next = LCD_LED_MIN;
		//dprintk("next current is %d\n", next);

		if (drvdata->state == NORMAL_STATE && drvdata->intensity != next)
			lm3530_write(drvdata->client, drvdata->reg_addrs.bl_m, next);
		
		drvdata->intensity = next;
	}
	else {
		dprintk("A manual setting for intensity is only permitted in normal mode\n");
	}

	return 0;
}

static int lm3530_get_intensity(struct lm3530_driver_data *drvdata)
{
	return drvdata->intensity;
}


#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void lm3530_early_suspend(struct early_suspend * h)
{	
	struct lm3530_driver_data *drvdata = container_of(h, struct lm3530_driver_data,
						    early_suspend);

	dprintk("start\n");
	lm3530_sleep(drvdata);

	return;
}

static void lm3530_late_resume(struct early_suspend * h)
{	
	struct lm3530_driver_data *drvdata = container_of(h, struct lm3530_driver_data,
						    early_suspend);

	dprintk("start\n");
	lm3530_wakeup(drvdata);

	return;
}
#else
static int lm3530_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct lm3530_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	lm3530_sleep(drvdata);
	return 0;
}

static int lm3530_resume(struct i2c_client *i2c_dev)
{
	struct lm3530_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	lm3530_wakeup(drvdata);
	return 0;
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */
#else
#define lm3530_suspend	NULL
#define lm3530_resume	NULL
#endif	/* CONFIG_PM */

void lm3530_switch_mode(struct device *dev, int next_mode)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(dev);
//	unsigned int lm3530_intensity;

	if (!drvdata || drvdata->mode == next_mode)
		return;

	if (next_mode == ALC_MODE) {
		/* LGE_CHANGE
		 * Remove ALC mode
		 * 2010-07-26. minjong.gong@lge.com
		 */
		//lm3530_set_table(drvdata, drvdata->cmds.alc);
	}
	else if (next_mode == NORMAL_MODE) {
		lm3530_set_table(drvdata, drvdata->cmds.alc);

		lm3530_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	} else {
		printk(KERN_ERR "%s: invalid mode(%d)!!!\n", __func__, next_mode);
		return;
	}

	drvdata->mode = next_mode;
	return;
}

ssize_t lm3530_show_alc(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->mode == ALC_MODE) ? "1":"0");
	
	return r;
}

ssize_t lm3530_store_alc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int alc;
	int next_mode;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &alc);

	if (alc)
		next_mode = ALC_MODE;
	else
		next_mode = NORMAL_MODE;

	lm3530_switch_mode(dev->parent, next_mode);

	return count;
}

ssize_t lm3530_show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(dev);
	int len = 0;
	unsigned char val;

	len += snprintf(buf,       PAGE_SIZE,       "\nLM3530 Registers is following..\n");
	lm3530_read(drvdata->client, 0x00, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[CH_EN(0x00)] = 0x%x\n", val);
	lm3530_read(drvdata->client, 0x01, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[BLM(0x01)] = 0x%x\n", val);
	lm3530_read(drvdata->client, 0x0E, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS(0x0E)] = 0x%x\n", val);	
	lm3530_read(drvdata->client, 0x0F, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[SBIAS(0x0F)] = 0x%x\n", val);
	lm3530_read(drvdata->client, 0x10, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS_GAIN(0x10)] = 0x%x\n", val);
	lm3530_read(drvdata->client, 0x11, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[AMBIENT_LEVEL(0x11)] = 0x%x\n", val);

	return len;
}

ssize_t lm3530_show_drvstat(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int len = 0;

	len += snprintf(buf,       PAGE_SIZE,       "\nLM3530 Backlight Driver Status is following..\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "mode                   = %3d\n", drvdata->mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "state                  = %3d\n", drvdata->state);
	len += snprintf(buf + len, PAGE_SIZE - len, "current intensity      = %3d\n", drvdata->intensity);

	return len;
}

DEVICE_ATTR(alc, 0664, lm3530_show_alc, lm3530_store_alc);
DEVICE_ATTR(reg, 0664, lm3530_show_reg, NULL);
DEVICE_ATTR(drvstat, 0664, lm3530_show_drvstat, NULL);

static int lm3530_set_brightness(struct backlight_device *bd)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return lm3530_send_intensity(drvdata, bd->props.brightness);
}

static int lm3530_get_brightness(struct backlight_device *bd)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return lm3530_get_intensity(drvdata);
}

static struct backlight_ops lm3530_ops = {
	.get_brightness = lm3530_get_brightness,
	.update_status  = lm3530_set_brightness,
};


#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct lm3530_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);
	int brightness;
	int next = 0;

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}

	brightness = lm3530_get_intensity(drvdata);

	if(exponential_eable)
	{
		// exponential brightness mapping
		if (value <= 35)
		{
			next = value * LM3530_MIN_BRIGHTNESS / 35;
		}
		else if(value > 35 && value <= 102)
		{
			next = (value - 35) * (LM3530_DEFAULT_BRIGHTNESS - LM3530_MIN_BRIGHTNESS) / (102 - 35) + LM3530_MIN_BRIGHTNESS;
		}
		else if(value >102)
		{
			next = (value - 102) * (drvdata->max_intensity - LM3530_DEFAULT_BRIGHTNESS) / (LED_FULL - 102) + LM3530_DEFAULT_BRIGHTNESS;
		}
	}
	else
	{
		// linear brightness mapping
		next = value * drvdata->max_intensity / LED_FULL;
	}

	if (brightness != next) {
		dprintk("value:%d, current:%d, next:%d\n", value, brightness, next);
		lm3530_send_intensity(drvdata, next);
	}
}

static struct led_classdev lm3530_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = leds_brightness_set,
};
#endif

static int __init lm3530_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct lm3530_platform_data *pdata;
	struct lm3530_driver_data *drvdata;
	struct backlight_device *bd;
	int err;

	dprintk("start, client addr=0x%x\n", i2c_dev->addr);

	pdata = i2c_dev->dev.platform_data;
	if(!pdata)
		return -EINVAL;
		
	drvdata = kzalloc(sizeof(struct lm3530_driver_data), GFP_KERNEL);
	if (!drvdata) {
		dev_err(&i2c_dev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	if (pdata && pdata->platform_init)
		pdata->platform_init();

	drvdata->client = i2c_dev;
	drvdata->gpio = pdata->gpio;
	drvdata->max_intensity = LCD_LED_MAX;
	if (pdata->max_current > 0)
		drvdata->max_intensity = pdata->max_current;
	drvdata->intensity = LCD_LED_MIN;
	drvdata->mode = NORMAL_MODE;
	drvdata->state = UNINIT_STATE;
	drvdata->version = pdata->version;

	if(lge_bd_rev < LGE_REV_C) 
		exponential_eable = 1;
	else
		exponential_eable = 0;

	if(lm3530_setup_version(drvdata) != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}		
	if (drvdata->gpio && gpio_request(drvdata->gpio, "lm3530_en") != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}

	bd = backlight_device_register("lm3530-bl", &i2c_dev->dev, NULL, &lm3530_ops, NULL);
	if (bd == NULL) {
		eprintk("entering lm3530 probe function error \n");
		if (gpio_is_valid(drvdata->gpio))
			gpio_free(drvdata->gpio);
		kfree(drvdata);
		return -1;
	}
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.brightness = drvdata->intensity;
	bd->props.max_brightness = drvdata->max_intensity;
	drvdata->bd = bd;

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
	if (led_classdev_register(&i2c_dev->dev, &lm3530_led_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &lm3530_led_dev;
		err = device_create_file(drvdata->led->dev, &dev_attr_alc);
		err = device_create_file(drvdata->led->dev, &dev_attr_reg);
		err = device_create_file(drvdata->led->dev, &dev_attr_drvstat);
	}
#endif

	i2c_set_clientdata(i2c_dev, drvdata);
	i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	lm3530_device_init(drvdata);
	if(exponential_eable)
		lm3530_send_intensity(drvdata, DEFAULT_BRIGHTNESS_EXPONENTIAL);
	else
		lm3530_send_intensity(drvdata, DEFAULT_BRIGHTNESS);

#ifdef CONFIG_HAS_EARLYSUSPEND
	drvdata->early_suspend.suspend = lm3530_early_suspend;
	drvdata->early_suspend.resume = lm3530_late_resume;
	drvdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&drvdata->early_suspend);
#endif

	eprintk("done\n");
	return 0;
}

static int __devexit lm3530_remove(struct i2c_client *i2c_dev)
{
	struct lm3530_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	lm3530_send_intensity(drvdata, 0);

	backlight_device_unregister(drvdata->bd);
	led_classdev_unregister(drvdata->led);
	i2c_set_clientdata(i2c_dev, NULL);
	if (gpio_is_valid(drvdata->gpio))
		gpio_free(drvdata->gpio);
	kfree(drvdata);

	return 0;
}

static struct i2c_device_id lm3530_idtable[] = {
	{ MODULE_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, lm3530_idtable);

static struct i2c_driver __refdata lm3530_driver = {
	.probe 		= lm3530_probe,
	.remove 	= lm3530_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= lm3530_suspend,
	.resume 	= lm3530_resume,
#endif
	.id_table 	= lm3530_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init lm3530_init(void)
{
	printk("LM3530 init start\n");
	return i2c_add_driver(&lm3530_driver);
}

static void __exit lm3530_exit(void)
{
	i2c_del_driver(&lm3530_driver);
}

module_init(lm3530_init);
module_exit(lm3530_exit);

MODULE_DESCRIPTION("Backlight driver for National Semiconductor LM3530");
MODULE_AUTHOR("<dr.ryu@lge.com>");
MODULE_LICENSE("GPL");
