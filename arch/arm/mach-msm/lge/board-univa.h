/* arch/arm/mach-msm/include/mach/board_univa.h
 * Copyright (C) 2009 LGE, Inc.
 * Author: SungEun Kim <cleaneye@lge.com>
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
#ifndef __ARCH_MSM_BOARD_UNIVA_H_
#define __ARCH_MSM_BOARD_UNIVA_H_

#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include "pm.h"

//LGE_DEV_PORTING UNIVA_S jonghyuck.shin 20110328 remove the EVB Define because of REV.A
//#define UNIVA_EVB
//LGE_DEV_PORTING UNIVA_E

/* LGE_S [ynj.kim@lge.com] 2010-05-21 : atcmd - virtual device */
#define KEY_SPEAKERMODE 241 // KEY_VIDEO_NEXT is not used in GED
#define KEY_CAMERAFOCUS 242 // KEY_VIDEO_PREV is not used in GED
#define KEY_FOLDER_HOME 243
#define KEY_FOLDER_MENU 244

#define ATCMD_VIRTUAL_KEYPAD_ROW	8
#define ATCMD_VIRTUAL_KEYPAD_COL	8
/* LGE_E [ynj.kim@lge.com] 2010-05-21 : atcmd - virtual device */

//LGE_DEV_PORTING UNIVA_S jonghyuck.shin uSD patch 20110323
/* sdcard related macros *//* FEATURE_LGE_FS */
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
#if defined(UNIVA_EVB)
#define GPIO_SD_DETECT_N	49
#else
#define GPIO_SD_DETECT_N	41
#endif
//LGE_DEV_PORTING UNIVA_E

#define VREG_SD_LEVEL       3000

#define GPIO_SD_DATA_3      51
#define GPIO_SD_DATA_2      52
#define GPIO_SD_DATA_1      53
#define GPIO_SD_DATA_0      54
#define GPIO_SD_CMD         55
#define GPIO_SD_CLK         56
#endif

/* touch */
#define GPIOF_IRQF_MASK         0x0000ffff /* use to specify edge detection
											* without */
#define GPIOF_IRQF_TRIGGER_NONE 0x00010000 /* IRQF_TRIGGER_NONE is 0 which
											* also means "as already
											* configured" */
#define GPIOF_INPUT             0x00020000
#define GPIOF_DRIVE_OUTPUT      0x00040000
#define GPIOF_OUTPUT_LOW        0x00080000
#define GPIOF_OUTPUT_HIGH       0x00100000

#define GPIOIRQF_SHARED         0x00000001 /* the irq line is shared with
											* other inputs */
/* touch-screen macros */
#define TS_X_MIN			0
#define TS_X_MAX			320
#define TS_Y_MIN			0
#define TS_Y_MAX			480

//LGE_DEV_PORTING UNIVA_S [edward1.kim@lge.com] 20110214  
#define TS_Y_START			0
#define TS_Y_SCRN_MAX		480
//LGE_DEV_PORTING UNIVA_E
#define TS_GPIO_I2C_SDA		91
#define TS_GPIO_I2C_SCL		90
//LGE_DEV_PORTING UNIVA [edward1.kim@lge.com] 20110214  
#if 1 
#define TS_GPIO_INT         92
#define TS_GPIO_IRQ			MSM_GPIO_TO_INT(TS_GPIO_INT)
#define TS_I2C_SLAVE_ADDR	0x4A    //0x4A slave address of ATMEL mXT140 touch IC
#else
#define TS_GPIO_IRQ			92
#define TS_I2C_SLAVE_ADDR	0x20
#endif
//LGE_DEV_PORTING UNIVA_E
/* camera */
#if defined (CONFIG_MT9T113)
#define CAM_I2C_SLAVE_ADDR			0x3C
#elif defined (CONFIG_S5K5CAGA)
#define CAM_I2C_SLAVE_ADDR			0x3C 
//LGE_DEV_PORTING UNIVA_S : Camera (MT9P111)
#elif defined(CONFIG_MT9P111)
#define CAM_I2C_SLAVE_ADDR			0x3C 
//LGE_DEV_PORTING UNIVA_E
#endif
#define GPIO_CAM_RESET		 		0		/* GPIO_0 */
#define GPIO_CAM_PWDN		 		1		/* GPIO_1 */
#define GPIO_CAM_MCLK				15		/* GPIO_15 */
#define ISX005_DEFAULT_CLOCK_RATE	24000000

#define CAM_POWER_OFF		0
#define CAM_POWER_ON		1

#define LDO_CAM_AF_NO		1	/* 2.8V */
#define LDO_CAM_AVDD_NO		2	/* 2.7V */
#define LDO_CAM_DVDD_NO		3	/* 1.2V */
#define LDO_CAM_IOVDD_NO	4	/* 2.6V */

/* proximity sensor */
#define PROXI_GPIO_I2C_SCL	19   /*107 in p500*/
#define PROXI_GPIO_I2C_SDA 	20   /*108 in p500*/
#define PROXI_GPIO_DOUT		109
#define PROXI_I2C_ADDRESS	0x44 /*slave address 7bit*/
#define PROXI_LDO_NO_VCC	1

/* accelerometer */
#define ACCEL_GPIO_INT	 		39
#define ACCEL_GPIO_I2C_SCL  	2
#define ACCEL_GPIO_I2C_SDA  	3
#define ACCEL_I2C_ADDRESS		0x08 /*KR3DM slave address 7bit*/
#define ACCEL_I2C_ADDRESS_H		0x18 /*KR3DH slave address 7bit*/

/*Ecompass*/
//LGE_DEV_PORTING UNIVA
#if defined(UNIVA_EVB)
#define ECOM_GPIO_I2C_SCL	19  /*107 in p500*/
#define ECOM_GPIO_I2C_SDA	20	/*108 in p500*/
#else
#define ECOM_GPIO_I2C_SCL	107  /*107 in p500*/
#define ECOM_GPIO_I2C_SDA	108	/*108 in p500*/
#endif
#define ECOM_GPIO_RST
#define ECOM_GPIO_INT		31
#define ECOM_I2C_ADDRESS		0x0E /* slave address 7bit */
//LGE_DEV_PORTING UNIVA

/* lcd & backlight */
#define GPIO_LCD_BL_EN		82
#define GPIO_BL_I2C_SCL		88
#define GPIO_BL_I2C_SDA		89
#define GPIO_LCD_VSYNC_O	97
#define GPIO_LCD_MAKER_LOW	101
#define GPIO_LCD_RESET_N	102

#define BL_POWER_SUSPEND 0
#define BL_POWER_RESUME  1
/* bluetooth gpio pin */
enum {
	BT_WAKE         = 42,
	BT_RFR          = 43,
	BT_CTS          = 44,
	BT_RX           = 45,
	BT_TX           = 46,
	BT_PCM_DOUT     = 68,
	BT_PCM_DIN      = 69,
	BT_PCM_SYNC     = 70,
	BT_PCM_CLK      = 71,
	BT_HOST_WAKE    = 83,
	BT_RESET_N			= 123,
};

/* for desk dock
 * 2010-07-05, dongjin.ha@lge.com
 */
/* #define GPIO_CARKIT_DETECT	21 */
/* ear sense driver macros */
#define GPIO_EAR_SENSE		29
#define GPIO_HS_MIC_BIAS_EN	26

/* interface variable */
extern struct platform_device msm_device_snd;
extern struct platform_device msm_device_adspdec;
extern struct i2c_board_info i2c_devices[1];

extern int camera_power_state;
extern int lcd_bl_power_state;

/* interface functions */
int config_camera_on_gpios(void);
void config_camera_off_gpios(void);
void camera_power_mutex_lock(void);
void camera_power_mutex_unlock(void);
void univa_pwrsink_resume(void);

struct device* univa_backlight_dev(void);
#endif
