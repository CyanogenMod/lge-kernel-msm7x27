/* arch/arm/mach-msm/include/mach/board_muscat.h
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
#ifndef __ARCH_MSM_BOARD_MUSCAT_H
#define __ARCH_MSM_BOARD_MUSCAT_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include "pm.h"

/* LGE_S [ynj.kim@lge.com] 2010-05-21 : atcmd - virtual device */
#define KEY_SPEAKERMODE 241 // KEY_VIDEO_NEXT is not used in GED
#define KEY_CAMERAFOCUS 242 // KEY_VIDEO_PREV is not used in GED
#define KEY_FOLDER_HOME 243
#define KEY_FOLDER_MENU 244

/* LGE_S [moon.yongho@lge.com] 2010-12-29 : add qwerty keymap */
/* uzBrainnet */
#define KEY_ATSIGN	215 
#define KEY_DEL		14 
#define KEY_LANG	121 
#define KEY_FUNC1	122 
#define KEY_FUNC2	123 

#define ATCMD_VIRTUAL_KEYPAD_ROW	8
#define ATCMD_VIRTUAL_KEYPAD_COL	8
/* LGE_E [ynj.kim@lge.com] 2010-05-21 : atcmd - virtual device */

/* sdcard related macros */
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
#define GPIO_SD_DETECT_N	49
// LGE_CHANGE_S dangwoo.choi@lge.com - MMC Cover Detect is removed
#define GPIO_MMC_COVER_DETECT 77
// LGE_CHANGE_E dangwoo.choi@lge.com
#define VREG_SD_LEVEL       3000

#define GPIO_SD_DATA_3      51
#define GPIO_SD_DATA_2      52
#define GPIO_SD_DATA_1      53
#define GPIO_SD_DATA_0      54
#define GPIO_SD_CMD         55
#define GPIO_SD_CLK         56
#endif

/* touch-screen macros */
#define TS_X_MIN			0
#define TS_X_MAX			240
#define TS_Y_MIN			0
#define TS_Y_MAX			320 // pecan
#define TS_GPIO_I2C_SDA		91
#define TS_GPIO_I2C_SCL		90
#define TS_GPIO_IRQ			92
#define TS_I2C_SLAVE_ADDR	0x20

/* camera */
#if defined (CONFIG_MT9T113)
#define CAM_I2C_SLAVE_ADDR			0x3C
#elif defined (CONFIG_S5K5CAGA)
#define CAM_I2C_SLAVE_ADDR			0x3C 
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

// LGE_CHANGE_S dangwoo.choi@lge.com - Proximity Sensor is removed
#if 0 
/* proximity sensor */
#define PROXI_GPIO_I2C_SCL	107
#define PROXI_GPIO_I2C_SDA 	108
#define PROXI_GPIO_DOUT		109
#define PROXI_I2C_ADDRESS	0x44 /*slave address 7bit*/
#define PROXI_LDO_NO_VCC	1
#endif
// LGE_CHANGE_E dangwoo.choi@lge.com - Proximity Sensor is removed

/* accelerometer */
#define ACCEL_GPIO_INT	 		39
#define ACCEL_GPIO_I2C_SCL  	2
#define ACCEL_GPIO_I2C_SDA  	3
#define ACCEL_I2C_ADDRESS		0x18

/*Ecompass*/
#define ECOM_GPIO_I2C_SCL		107
#define ECOM_GPIO_I2C_SDA		108
#define ECOM_GPIO_INT			31
#define ECOM_I2C_ADDRESS		0x0E /* slave address 7bit */

/* lcd & backlight */
#define GPIO_LCD_BL_EN		82
#define GPIO_BL_I2C_SCL		88
#define GPIO_BL_I2C_SDA		89
#define GPIO_LCD_VSYNC_O	97
#define GPIO_LCD_MAKER_LOW	101
#define GPIO_LCD_RESET_N	102

#define BL_POWER_SUSPEND 0
#define BL_POWER_RESUME  1

/* for desk dock
 * 2010-07-05, dongjin.ha@lge.com
 */
#define GPIO_CARKIT_DETECT	21
/* ear sense driver macros */
#define GPIO_EAR_SENSE		29

// LGE_CHANGE_S dangwoo.choi@lge.com - removed
//#define GPIO_HS_MIC_BIAS_EN	26
// LGE_CHANGE_E dangwoo.choi@lge.com - removed

/* pp2106 qwerty keyboard */
#define PP2106_KEYPAD_ROW	7
#define PP2106_KEYPAD_COL	7

#define GPIO_PP2106_RESET	33
#define GPIO_PP2106_IRQ		23
#define GPIO_PP2106_SDA		21
#define GPIO_PP2106_SCL		18


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
void muscat_pwrsink_resume(void);

struct device* muscat_backlight_dev(void);
#endif
