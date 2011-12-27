/* drivers/video/backlight/aat2870_bl.c
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
#include <linux/mutex.h>
#include <mach/board_lge.h>

#define MODULE_NAME  "aat2870bl"
#define CONFIG_AAT28XX_LEDS_CLASS
//#define CONFIG_AAT28XX_BACKLIGHT_CLASS

#ifdef CONFIG_AAT28XX_LEDS_CLASS
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

#define LCD_LED_MAX 21 /* 20.32mA */
#define LCD_LED_MIN 0  /* 0.48mA */
#define DEFAULT_BRIGHTNESS 13
#define AAT28XX_LDO_NUM 4

#define AAT2862BL_REG_BLM   0x03  /* Register address for Main BL brightness */
#define AAT2862BL_REG_BLS   0x04  /* Register address for Main BL brightness */
#define AAT2862BL_REG_FADE	0x07  /* Register address for Backlight Fade control */
#define AAT2862BL_REG_LDOAB 0x00  /* Register address for LDO select A/B */
#define AAT2862BL_REG_LDOCD 0x01  /* Register address for LDO select C/D */
#define AAT2862BL_REG_LDOEN 0x02  /* Register address for LDO Enable */

#define AAT2870BL_REG_BLM   0x01  /* Register address for Main BL brightness */
#define AAT2870BL_REG_LDOAB 0x24  /* Register address for LDO select A/B */
#define AAT2870BL_REG_LDOCD 0x25  /* Register address for LDO select C/D */
#define AAT2870BL_REG_LDOEN 0x26  /* Register address for LDO Enable */

#ifdef CONFIG_AAT28XX_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#endif

enum {
	ALC_MODE,
	NORMAL_MODE,
} AAT2870BL_MODE;

enum {
	UNINIT_STATE=-1,
	POWERON_STATE,
	NORMAL_STATE,
	SLEEP_STATE,
	POWEROFF_STATE
} AAT2870BL_STATE;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

struct ldo_vout_struct {
	unsigned char reg;
	unsigned vol;
};

struct aat28xx_ctrl_tbl {
	unsigned char reg;
	unsigned char val;
};

struct aat28xx_reg_addrs {
	unsigned char bl_m;
	unsigned char bl_s;
	unsigned char fade;
	unsigned char ldo_ab;
	unsigned char ldo_cd;
	unsigned char ldo_en;
};

struct aat28xx_cmds {
	struct aat28xx_ctrl_tbl *normal;
	struct aat28xx_ctrl_tbl *alc;
	struct aat28xx_ctrl_tbl *sleep;
};

struct aat28xx_driver_data {
	struct i2c_client *client;
#ifdef CONFIG_AAT28XX_BACKLIGHT_CLASS
	struct backlight_device *bd;
#endif
#ifdef CONFIG_AAT28XX_LEDS_CLASS
	struct led_classdev *led;
#endif
	int gpio;
	int intensity;
	int max_intensity;
	int mode;
	int state;
	int ldo_ref[AAT28XX_LDO_NUM];
	unsigned char reg_ldo_enable;
	unsigned char reg_ldo_vout[2];
	int version;
	int initialized;
	struct aat28xx_cmds cmds;
	struct aat28xx_reg_addrs reg_addrs;
	struct mutex power_lock;
	int refcnt;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

/********************************************
 * Global variable
 ********************************************/
static unsigned int debug = 0;
module_param(debug, uint, 0644);

/* Set to Normal mode */
static struct aat28xx_ctrl_tbl aat2862bl_normal_tbl[] = {
/* LGE_UPDATE_S kideok.kim@lge.com 20101020*/
	 { 0x03, 0xD2 },  /* MEQS(7)=high, DISABLE FADE_MAIN(6)=high(disabled), LCD_ON(5)=high(On),	Brightness=Default (0x12, 13th setp)*/
//	 { 0x03, 0xF2 },  /* MEQS(7)=high, DISABLE FADE_MAIN(6)=high(disabled), LCD_ON(5)=high(On),	Brightness=Default (0x12, 13th setp)*/
	{ 0xFF, 0xFE }	 /* end of command */
};

/* Set to ALC mode */
static struct aat28xx_ctrl_tbl aat2862bl_alc_tbl[] = {
	// AAT2862 has no ALC mode !!
	{ 0xFF, 0xFE }   /* end or command */
};

/* Set to sleep mode */
static struct aat28xx_ctrl_tbl aat2862bl_sleep_tbl[] = {
	{ 0x03, 0xDF }, /* MEQS(7) = high, DISABLE FADE_MAIN(6) = high(disabled), LCD_ON(5) = high(Off),  Brightness = Min(0x1F)*/
	{ 0xFF, 0xFE },  /* end of command */	
};

/* Set to Normal mode */
static struct aat28xx_ctrl_tbl aat2870bl_normal_tbl[] = {
	{ 0x00, 0xFF },  /* Channel Enable=ALL */
	{ 0x0E, 0x26 },  /* SNSR_LIN_LOG=linear, ALSOUT_LIN_LOG=linear, RSET=1k~4k,
	                               * GAIN=low, GM=auto gain, ALS_EN=off */
	{ 0x0F, 0x06 },  /* SBIAS=2.6V, SBIAS=off */
	{ 0xFF, 0xFE }	 /* end of command */
};

/* Set to ALC mode */
static struct aat28xx_ctrl_tbl aat2870bl_alc_tbl[] = {
	{ 0x00, 0xFF },  /* Channel Enable : ALL */	
	{ 0x0E, 0x27 },  /* SNSR_LIN_LOG=linear, ALSOUT_LIN_LOG=linear, RSET=1k~4k,
	                               * GAIN=low, GM=auto gain, ALS_EN=on */
	{ 0x0F, 0x07 },  /* SBIAS=2.6V, SBIAS=on */
	{ 0x10, 0x00 },  /* CABC=0, OFF_TM=auto, PTME=0.5s, G_ADJ=no  */
	{ 0xFF, 0xFE }   /* end or command */
};

/* Set to sleep mode */
static struct aat28xx_ctrl_tbl aat2870bl_sleep_tbl[] = {
//	{ 0x0C, 0x00 },  /* FMT=1s, DISABLE_FADE_MAIN=0, FADE_MAIN=fade out */
	{ 0x0E, 0x26 },  /* SNSR_LIN_LOG=linear, ALSOUT_LIN_LOG=linear, RSET=1k~4k,
	                               * GAIN=low, GM=auto gain, ALS_EN=off */
	{ 0x0F, 0x06 },  /* SBIAS=2.6V, SBIAS=off */
	{ 0x00, 0x00 },  /* Channel Enable=disable */
	{ 0xFF, 0xFE },  /* end of command */	
};

static struct ldo_vout_struct ldo_vout_table[] = {
	{/* 0000 */ 0x00, 1200},
	{/* 0001 */ 0x01, 1300},
	{/* 0010 */ 0x02, 1500},
	{/* 0011 */ 0x03, 1600},
	{/* 0100 */ 0x04, 1800},
	{/* 0101 */ 0x05, 2000},
	{/* 0110 */ 0x06, 2200},
	{/* 0111 */ 0x07, 2500},
	{/* 1000 */ 0x08, 2600},
	{/* 1001 */ 0x09, 2700},
	{/* 1010 */ 0x0A, 2800},
	{/* 1011 */ 0x0B, 2900},
	{/* 1100 */ 0x0C, 3000},
	{/* 1101 */ 0x0D, 3100},
	{/* 1110 */ 0x0E, 3200},
	{/* 1111 */ 0x0F, 3300},
	{/* Invalid */ 0xFF, 0},
};

/********************************************
 * Functions
 ********************************************/
static int aat28xx_send_intensity_no_check(struct aat28xx_driver_data *drvdata, int next);

static int aat28xx_setup_version(struct aat28xx_driver_data *drvdata)
{
	if(!drvdata)
		return -ENODEV;

	if(drvdata->version == 2862) {
		drvdata->cmds.normal = aat2862bl_normal_tbl;
		drvdata->cmds.alc = aat2862bl_alc_tbl;
		drvdata->cmds.sleep = aat2862bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2862BL_REG_BLM;
		drvdata->reg_addrs.bl_s = AAT2862BL_REG_BLS;
		drvdata->reg_addrs.fade = AAT2862BL_REG_FADE;
		drvdata->reg_addrs.ldo_ab = AAT2862BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2862BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2862BL_REG_LDOEN;
	}
	else if(drvdata->version == 2870) {
		drvdata->cmds.normal = aat2870bl_normal_tbl;
		drvdata->cmds.alc = aat2870bl_alc_tbl;
		drvdata->cmds.sleep = aat2870bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2870BL_REG_BLM;
		drvdata->reg_addrs.ldo_ab = AAT2870BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2870BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2870BL_REG_LDOEN;
	}
	else {
		eprintk("Not supported version!!\n");
		return -ENODEV;
	}

	return 0;
}

static int aat28xx_read(struct i2c_client *client, u8 reg, u8 *pval)
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

static int aat28xx_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	int status = 0;

	if (client == NULL) {	/* No global client pointer? */
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		eprintk("fail to write(reg=0x%x,val=0x%x,ret=%d)\n", reg, val, ret);
	}

	return status;
}

static int aat28xx_set_ldos(struct i2c_client *i2c_dev, unsigned num, int enable)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	if (drvdata) {
		if (enable) drvdata->reg_ldo_enable |= 1 << (num-1);
		else drvdata->reg_ldo_enable &= ~(1 << (num-1));
		
		dprintk("enable ldos, reg:0x13 value:0x%x\n", drvdata->reg_ldo_enable);
		
		return aat28xx_write(i2c_dev, drvdata->reg_addrs.ldo_en, drvdata->reg_ldo_enable);
	}
	return -EIO;
}

static unsigned char aat28xx_ldo_get_vout_val(unsigned vol)
{
	int i = 0;
	do {
		if (ldo_vout_table[i].vol == vol)
			return ldo_vout_table[i].reg;
		else
			i++;
	} while (ldo_vout_table[i].vol != 0);

	return ldo_vout_table[i].reg;
}

static int aat28xx_ldo_set_vout(struct i2c_client *i2c_dev, unsigned num, unsigned char val)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	unsigned char *next_val;
	unsigned char reg;

	if (drvdata) {
		if (num <= 2) {
			reg = drvdata->reg_addrs.ldo_ab;
			next_val = &drvdata->reg_ldo_vout[0];
		} else {
			reg = drvdata->reg_addrs.ldo_cd;
			next_val = &drvdata->reg_ldo_vout[1];
		}
		if (num % 2) {
			*next_val &= 0x0F;
			val = val << 4;		
		}
		else {
			*next_val &= 0xF0;		
		}
		*next_val |= val;
		dprintk("target register[0x%x], value[0x%x]\n",	reg, *next_val);
		return aat28xx_write(i2c_dev, reg, *next_val);
	}
	return -EIO;
}

/*******************************************************
 * Function: aat28xx_ldo_set_level
 * Description: Set LDO vout level
 * Parameter
 *         num: ldo number and it is 1-based value
 *         level: voltage level
 *******************************************************/
int aat28xx_ldo_enable(struct device *dev, unsigned num, unsigned enable)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	struct aat28xx_driver_data *drvdata;
	int err = 0;

	dprintk("ldo_no[%d], on/off[%d]\n",num, enable);

	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {
			drvdata = i2c_get_clientdata(client);
			if (enable) {
				if (drvdata->ldo_ref[num-1]++ == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			else {
				if (--drvdata->ldo_ref[num-1] == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			return err;
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_enable);

/*******************************************************
 * Function: aat28xxcp_ldo_set_level
 * Description: Set LDO vout level
 * Parameter
 *         num: ldo number and it is 1-based value
 *         level: voltage level
 *******************************************************/
int aat28xx_ldo_set_level(struct device *dev, unsigned num, unsigned vol)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	unsigned char val;

	dprintk("ldo_no[%d], level[%d]\n", num, vol);
	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {
			val = aat28xx_ldo_get_vout_val(vol);
			dprintk("vout register value 0x%x for level %d\n", val, vol);
			return aat28xx_ldo_set_vout(client, num, val);
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_set_level);

static void aat28xx_power_internal(struct aat28xx_driver_data *drvdata, int on)
{
	if(!drvdata || !drvdata->gpio)
		return;

	if(drvdata->refcnt == 0 && !on) {
		printk(KERN_ERR "%s: already in power off!!\n",  __func__);
		return;
	}

	printk(KERN_INFO "%s: on = %d, refcnt = %d\n",
		   __func__, on, drvdata->refcnt);

	mutex_lock(&drvdata->power_lock);
	if(on) {
		if(drvdata->refcnt == 0) {
			gpio_set_value(drvdata->gpio, 0);
			mdelay(20);
			gpio_set_value(drvdata->gpio, 1);
			mdelay(2);
		}
		drvdata->refcnt++;
	} else {
		drvdata->refcnt--;
		if(drvdata->refcnt ==0) {
			gpio_set_value(drvdata->gpio, 0);
			mdelay(20);
		}
	}
	mutex_unlock(&drvdata->power_lock);
}

void aat28xx_power(struct device *dev, int on)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;

	if((adap = dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap)))
		aat28xx_power_internal(i2c_get_clientdata(client), on);
}
EXPORT_SYMBOL(aat28xx_power);

static int aat28xx_set_table(struct aat28xx_driver_data *drvdata, struct aat28xx_ctrl_tbl *ptbl)
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
			if (aat28xx_write(drvdata->client, ptbl->reg, ptbl->val) != 0)
				dprintk("i2c failed addr:%d, value:%d\n", ptbl->reg, ptbl->val);
		}
		ptbl++;
		i++;
	}
	return 0;
}

static void aat28xx_hw_reset(struct aat28xx_driver_data *drvdata)
{
	aat28xx_power_internal(drvdata, 1);
}

static void aat28xx_go_opmode(struct aat28xx_driver_data *drvdata)
{
	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			aat28xx_set_table(drvdata, drvdata->cmds.normal);
			drvdata->state = NORMAL_STATE;
			break;

		case ALC_MODE:
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_device_init(struct aat28xx_driver_data *drvdata)
{
	if (drvdata->initialized && system_state == SYSTEM_BOOTING) {
		drvdata->refcnt++;
		aat28xx_go_opmode(drvdata);
		return;
	}
	aat28xx_hw_reset(drvdata);
	aat28xx_go_opmode(drvdata);
}

static void aat28xx_poweron(struct aat28xx_driver_data *drvdata)
{
	unsigned int aat28xx_intensity;
	if (!drvdata || drvdata->state != POWEROFF_STATE)
		return;
	
	dprintk("POWER ON \n");

	aat28xx_device_init(drvdata);
	
	if (drvdata->mode == NORMAL_MODE)
	{
		if(drvdata->version == 2862)
		{
			aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	/* Invert BL control bits and Clear upper 3bits */
			aat28xx_intensity |= 0xE0;				/* MEQS(7)=1, Disable Fade(6)=1, LCD_ON(5)=1*/
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
		}
		else
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	}
}

#if 0
static void aat28xx_poweroff(struct aat28xx_driver_data *drvdata)
{
	printk("%s backlight power state = %d, gpio %d\n",
			__func__, drvdata->state, drvdata->gpio);

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
static void aat28xx_sleep(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == SLEEP_STATE)
		return;

	printk(KERN_INFO "%s: cur state=%d to sleep state(2)\n", __func__, drvdata->state);
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			drvdata->state = SLEEP_STATE;
			aat28xx_set_table(drvdata, drvdata->cmds.sleep);
			break;

		case ALC_MODE:
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_wakeup(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == NORMAL_STATE)
		return;

	printk(KERN_INFO "%s: cur state=%d to wakeup state(1)\n", __func__, drvdata->state);
	
	if (drvdata->state == POWEROFF_STATE) {
		aat28xx_poweron(drvdata);
	} else if (drvdata->state == SLEEP_STATE) {
		if (drvdata->mode == NORMAL_MODE) {
			if(drvdata->version == 2862) {
				aat28xx_write(drvdata->client, drvdata->reg_addrs.fade, 0x00);	/* Floor current : 0.48mA */
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, 0x9F);
				aat28xx_send_intensity_no_check(drvdata, drvdata->intensity);
			} else {
				aat28xx_set_table(drvdata, drvdata->cmds.normal);
				aat28xx_send_intensity_no_check(drvdata, drvdata->intensity);
			}
			drvdata->state = NORMAL_STATE;
		} else if (drvdata->mode == ALC_MODE) {
			printk(KERN_ERR "%s: invalide mode!!!\n", __func__);
		}
	}
}

static int aat28xx_send_intensity_no_check(struct aat28xx_driver_data *drvdata, int next)
{
	int aat2862_bl_next;

	if(drvdata->version == 2862) {
		if(next != 0) {
			aat2862_bl_next = (~next & 0x1F);	/* Invert BL control bits and Clear upper 3bits */
			aat2862_bl_next |= 0xE0;		/* MEQS(7)=1, Disable Fade(6)=1, LCD_ON(5)=1*/
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);
		} else {	// Off the backlight if brightness set level is 0.
			aat2862_bl_next = 0xDF;		/* MEQS(7)=1, Disable Fade(6)=1, LCD_ON(5)=0*/
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);
		}
	} else {	/* 2870 */
		aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, next);
	}
	return 0;
}

static int aat28xx_send_intensity(struct aat28xx_driver_data *drvdata, int next)
{
	if (drvdata->mode == NORMAL_MODE) {
		if (next > drvdata->max_intensity)
			next = drvdata->max_intensity;
		if (next < LCD_LED_MIN)
			next = LCD_LED_MIN;
		dprintk("next current is %d\n", next);

		if (drvdata->state == NORMAL_STATE && drvdata->intensity != next)
			aat28xx_send_intensity_no_check(drvdata, next);
		
		drvdata->intensity = next;
	}
	else {
		dprintk("A manual setting for intensity is only permitted in normal mode\n");
	}

	return 0;
}

static int aat28xx_get_intensity(struct aat28xx_driver_data *drvdata)
{
	return drvdata->intensity;
}

static int aat28xx_change_max_current(struct aat28xx_driver_data *drvdata, int max_current)
{
	drvdata->max_intensity = max_current;
	return aat28xx_send_intensity(drvdata, drvdata->max_intensity);
}

#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void aat28xx_early_suspend(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	aat28xx_sleep(drvdata);
	aat28xx_power_internal(drvdata, 0);

	return;
}

static void aat28xx_late_resume(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	aat28xx_power_internal(drvdata, 1);
	aat28xx_wakeup(drvdata);

	return;
}
#else
static int aat28xx_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_sleep(drvdata);
	aat28xx_power_internal(drvdata, 0);
	return 0;
}

static int aat28xx_resume(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_power_internal(drvdata, 1);
	aat28xx_wakeup(drvdata);
	return 0;
}
#endif	/* CONFIG_HAS_EARLYSUSPEND */
#else
#define aat28xx_suspend	NULL
#define aat28xx_resume	NULL
#endif	/* CONFIG_PM */

/* 
 * 2011-03-08, jinkyu.choi@lge.com
 * if the backlight ic is using vbat power,
 * we should turn off the backlight ic when reboot or power down.
 */
static void aat28xx_shutdown(struct i2c_client *i2c_dev) {
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

#if 0
	printk("%s backlight power state = %d, gpio %d\n",
			__func__, drvdata->state, drvdata->gpio);
#endif

	/* change the state to sleep and disable the backlight ic */
	aat28xx_sleep(drvdata);
	aat28xx_power_internal(drvdata, 0);
	//printk("%s is done!\n", __func__);
}

void aat28xx_switch_mode(struct device *dev, int next_mode)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);

	if (!drvdata || drvdata->mode == next_mode)
		return;

	if (next_mode == NORMAL_MODE) {
		aat28xx_set_table(drvdata, drvdata->cmds.alc);
	    aat28xx_send_intensity_no_check(drvdata, drvdata->intensity);
	} else {
		printk(KERN_ERR "%s: invalid mode(%d)!!!\n", __func__, next_mode);
		return;
	}

	drvdata->mode = next_mode;
	return;
}

ssize_t aat28xx_show_alc(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->mode == ALC_MODE) ? "1":"0");
	
	return r;
}

ssize_t aat28xx_store_alc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
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

	aat28xx_switch_mode(dev->parent, next_mode);

	return count;
}

ssize_t aat28xx_show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);
	int len = 0;
	unsigned char val;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Registers is following..\n");
	aat28xx_read(drvdata->client, 0x00, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[CH_EN(0x00)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x01, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[BLM(0x01)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x0E, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS(0x0E)] = 0x%x\n", val);	
	aat28xx_read(drvdata->client, 0x0F, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[SBIAS(0x0F)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x10, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS_GAIN(0x10)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x11, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[AMBIENT_LEVEL(0x11)] = 0x%x\n", val);

	return len;
}

ssize_t aat28xx_show_drvstat(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int len = 0;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Backlight Driver Status is following..\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "mode                   = %3d\n", drvdata->mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "state                  = %3d\n", drvdata->state);
	len += snprintf(buf + len, PAGE_SIZE - len, "current intensity      = %3d\n", drvdata->intensity);

	return len;
}

ssize_t aat28xx_store_max_current(struct device *dev, struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int max_current;

	if (!count)	return -EINVAL;

	sscanf(buf, "%d", &max_current);
	aat28xx_change_max_current(drvdata, max_current);

	return count;
}

ssize_t aat28xx_show_max_current(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	ssize_t len;

	len = sprintf(buf, "%d\n", drvdata->max_intensity);

	return len;
}

DEVICE_ATTR(alc, 0664, aat28xx_show_alc, aat28xx_store_alc);
DEVICE_ATTR(reg, 0444, aat28xx_show_reg, NULL);
DEVICE_ATTR(drvstat, 0444, aat28xx_show_drvstat, NULL);
DEVICE_ATTR(max_current, 0664, aat28xx_show_max_current, aat28xx_store_max_current);

#ifdef CONFIG_AAT28XX_BACKLIGHT_CLASS
static int aat28xx_set_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_send_intensity(drvdata, bd->props.brightness);
}

static int aat28xx_get_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_get_intensity(drvdata);
}

static struct backlight_ops aat28xx_ops = {
	.get_brightness = aat28xx_get_brightness,
	.update_status  = aat28xx_set_brightness,
};
#endif

#ifdef CONFIG_AAT28XX_LEDS_CLASS
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);
	int brightness;
	int next;

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}

	brightness = aat28xx_get_intensity(drvdata);

	next = value * drvdata->max_intensity / led_cdev->max_brightness;
	dprintk("input brightness value=%d]\n", next);

	if (brightness != next) {
		dprintk("brightness[current=%d, next=%d]\n", brightness, next);
		aat28xx_send_intensity(drvdata, next);
	}
}

static struct led_classdev aat28xx_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = leds_brightness_set,
	.max_brightness = LED_FULL,
};
#endif

static int __init aat28xx_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct aat28xx_platform_data *pdata;
	struct aat28xx_driver_data *drvdata;
#ifdef CONFIG_AAT28XX_BACKLIGHT_CLASS
	struct backlight_device *bd;
#endif
	int err;

	dprintk("start, client addr=0x%x\n", i2c_dev->addr);

	pdata = i2c_dev->dev.platform_data;
	if(!pdata)
		return -EINVAL;
		
	drvdata = kzalloc(sizeof(struct aat28xx_driver_data), GFP_KERNEL);
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
	drvdata->initialized = pdata->initialized;

	if(aat28xx_setup_version(drvdata) != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}		

	if (drvdata->gpio && gpio_request(drvdata->gpio, "aat28xx_en") != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}

#ifdef CONFIG_AAT28XX_BACKLIGHT_CLASS
	bd = backlight_device_register("aat28xx-bl", &i2c_dev->dev, NULL, &aat28xx_ops, NULL);
	if (bd == NULL) {
		eprintk("entering aat28xx probe function error \n");
		if (gpio_is_valid(drvdata->gpio))
			gpio_free(drvdata->gpio);
		kfree(drvdata);
		return -1;
	}
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.brightness = drvdata->intensity;
	bd->props.max_brightness = drvdata->max_intensity;
	drvdata->bd = bd;
#endif

#ifdef CONFIG_AAT28XX_LEDS_CLASS
	if (led_classdev_register(&i2c_dev->dev, &aat28xx_led_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &aat28xx_led_dev;
		err = device_create_file(drvdata->led->dev, &dev_attr_alc);
		err = device_create_file(drvdata->led->dev, &dev_attr_reg);
		err = device_create_file(drvdata->led->dev, &dev_attr_drvstat);
		err = device_create_file(drvdata->led->dev, &dev_attr_max_current);
	}
#endif

	mutex_init(&drvdata->power_lock);

	i2c_set_clientdata(i2c_dev, drvdata);
	i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	aat28xx_device_init(drvdata);
	aat28xx_send_intensity(drvdata, DEFAULT_BRIGHTNESS);

#ifdef CONFIG_HAS_EARLYSUSPEND
	drvdata->early_suspend.suspend = aat28xx_early_suspend;
	drvdata->early_suspend.resume = aat28xx_late_resume;
	drvdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&drvdata->early_suspend);
#endif

	eprintk("done\n");
	return 0;
}

static int __devexit aat28xx_remove(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	aat28xx_send_intensity(drvdata, 0);

#ifdef CONFIG_AAT28XX_BACKLIGHT_CLASS
	backlight_device_unregister(drvdata->bd);
#endif
#ifdef CONFIG_AAT28XX_LEDS_CLASS
	led_classdev_unregister(drvdata->led);
#endif
	i2c_set_clientdata(i2c_dev, NULL);
	if (gpio_is_valid(drvdata->gpio))
		gpio_free(drvdata->gpio);
	kfree(drvdata);

	return 0;
}

static struct i2c_device_id aat28xx_idtable[] = {
	{ MODULE_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, aat28xx_idtable);

static struct i2c_driver __refdata aat28xx_driver = {
	.probe 		= aat28xx_probe,
	.remove 	= aat28xx_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= aat28xx_suspend,
	.resume 	= aat28xx_resume,
#endif
	.shutdown	= aat28xx_shutdown,
	.id_table 	= aat28xx_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init aat28xx_init(void)
{
	printk("AAT28XX init start\n");
	return i2c_add_driver(&aat28xx_driver);
}

static void __exit aat28xx_exit(void)
{
	i2c_del_driver(&aat28xx_driver);
}

module_init(aat28xx_init);
module_exit(aat28xx_exit);

MODULE_DESCRIPTION("Backlight driver for ANALOGIC TECH AAT28XX");
MODULE_AUTHOR("Munyoung Hwang <munyoung.hwang@lge.com>");
MODULE_LICENSE("GPL");
