/* drivers/media/video/msm/isx006.c 
*
* This software is for SONY 3M sensor 
*  
* Copyright (C) 2009-2011 LGE Inc.  
* Author: Hyungtae Lee <leehyungtae>  
* (GPL License)  
* This software is licensed under the terms of the GNU General Public  
* License version 2, as published by the Free Software Foundation, and  
* may be copied, distributed, and modified under those terms.  
*  
* This program is distributed in the hope that it will be useful,  
* but WITHOUT ANY WARRANTY; without even the implied warranty of  
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
* GNU General Public License for more details. 
*/


#ifndef ISX006_H
#define ISX006_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct isx006_reg isx006_regs;

enum isx006_width {
	DOBULE_LEN,
	WORD_LEN,
	BYTE_LEN,
	ADDRESS_TUNE
};

struct isx006_i2c_reg_conf {
	unsigned short waddr;
	uint32_t wdata;
	enum isx006_width width;
	unsigned short mdelay_time;
};

struct isx006_register_address_value_pair {
	uint16_t register_address;
	uint32_t register_value;
	enum isx006_width register_length;
};

struct isx006_reg {
	const struct isx006_i2c_reg_conf *pll;
	uint16_t pll_size;
	const struct isx006_i2c_reg_conf *init;
	uint16_t init_size;
	const struct isx006_i2c_reg_conf *prev_reg_settings;
	uint16_t prev_reg_settings_size;
	const struct isx006_i2c_reg_conf *ap001_16bit_settings;
	uint16_t ap001_16bit_settings_size;
	const struct isx006_i2c_reg_conf *ap003_16bit_settings;
	uint16_t ap003_16bit_settings_size;
	
	/*register for scene*/
	const struct isx006_i2c_reg_conf *scene_normal_reg_settings;
	uint16_t scene_normal_reg_settings_size;	
	const struct isx006_i2c_reg_conf *scene_portrait_reg_settings;
	uint16_t scene_portrait_reg_settings_size;	
	const struct isx006_i2c_reg_conf *scene_landscape_reg_settings;
	uint16_t scene_landscape_reg_settings_size;
	const struct isx006_i2c_reg_conf *scene_sport_reg_settings;
	uint16_t scene_sport_reg_settings_size;
	const struct isx006_i2c_reg_conf *scene_sunset_reg_settings;
	uint16_t scene_sunset_reg_settings_size;
	const struct isx006_i2c_reg_conf *scene_night_reg_settings;
	uint16_t scene_night_reg_settings_size;
	
	/*register for AF*/
	const struct isx006_i2c_reg_conf *AF_reg_settings;
	uint16_t AF_reg_settings_size;
	const struct isx006_i2c_reg_conf *AF_nomal_reg_settings;
	uint16_t AF_nomal_reg_settings_size;
	const struct isx006_i2c_reg_conf *AF_macro_reg_settings;
	uint16_t AF_macro_reg_settings_size;	
	const struct isx006_i2c_reg_conf *manual_focus_reg_settings;
	uint16_t manual_focus_reg_settings_size;	

	/*register for ISO*/
	const struct isx006_i2c_reg_conf *iso_auto_reg_settings;
	uint16_t iso_auto_reg_settings_size;
	const struct isx006_i2c_reg_conf *iso_100_reg_settings;
	uint16_t iso_100_reg_settings_size;
	const struct isx006_i2c_reg_conf *iso_200_reg_settings;
	uint16_t iso_200_reg_settings_size;
	const struct isx006_i2c_reg_conf *iso_400_reg_settings;
	uint16_t iso_400_reg_settings_size;
	const struct isx006_i2c_reg_conf *iso_800_reg_settings;
	uint16_t iso_800_reg_settings_size;	
	
	/*capture zoom°ü·Ã setting*/
	const struct isx006_i2c_reg_conf *zoom_mode_capture_127_reg_settings;
	uint16_t zoom_mode_capture_127_reg_settings_size;
	const struct isx006_i2c_reg_conf *zoom_mode_capture_162_reg_settings;
	uint16_t zoom_mode_capture_162_reg_settings_size;
	const struct isx006_i2c_reg_conf *zoom_mode_capture_203_reg_settings;
	uint16_t zoom_mode_capture_203_reg_settings_size;
	const struct isx006_i2c_reg_conf *zoom_mode_capture_405_reg_settings;
	uint16_t zoom_mode_capture_405_reg_settings_size;
};
enum isx006_focus_mode {
	FOCUS_NORMAL,
	FOCUS_MACRO,
	FOCUS_AUTO,
	FOCUS_MANUAL,
};
/* this value is defined in Android native camera */
enum isx006_wb_type {
	CAMERA_WB_MIN_MINUS_1,
	CAMERA_WB_AUTO = 1,  /* This list must match aeecamera.h */
	CAMERA_WB_CUSTOM,
	CAMERA_WB_INCANDESCENT,
	CAMERA_WB_FLUORESCENT,
	CAMERA_WB_DAYLIGHT,
	CAMERA_WB_CLOUDY_DAYLIGHT,
	CAMERA_WB_TWILIGHT,
	CAMERA_WB_SHADE,
	CAMERA_WB_MAX_PLUS_1
};

enum isx006_antibanding_type {
	CAMERA_ANTIBANDING_OFF,
	CAMERA_ANTIBANDING_60HZ,
	CAMERA_ANTIBANDING_50HZ,
	CAMERA_ANTIBANDING_AUTO,
	CAMERA_MAX_ANTIBANDING,
};

/* Enum Type for different ISO Mode supported */
enum isx006_iso_value {
	CAMERA_ISO_AUTO = 0,
	CAMERA_ISO_DEBLUR,
	CAMERA_ISO_100,
	CAMERA_ISO_200,
	CAMERA_ISO_400,
	CAMERA_ISO_800,
	CAMERA_ISO_MAX
};

/* Enum type for scene mode */
enum {
	CAMERA_SCENE_AUTO = 1,
	CAMERA_SCENE_PORTRAIT,
	CAMERA_SCENE_LANDSCAPE,
	CAMERA_SCENE_SPORTS,
	CAMERA_SCENE_NIGHT,
	CAMERA_SCENE_SUNSET,
};
#endif 
