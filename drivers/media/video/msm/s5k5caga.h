/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef S5K5CAGA_H
#define S5K5CAGA_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct lgcam_rear_sensor_reg lgcam_rear_sensor_regs;
extern int mclk_rate;

enum lgcam_rear_sensor_width {
	BURST_LEN,
	DOBULE_LEN,
	WORD_LEN,
	BYTE_LEN,
	ADDRESS_TUNE
};

struct lgcam_rear_sensor_i2c_reg_conf {
	unsigned short waddr;
	uint32_t wdata;
	enum lgcam_rear_sensor_width width;
};
struct lgcam_rear_sensor_register_address_value_pair {
	uint16_t register_address;
	uint16_t register_value;
	enum lgcam_rear_sensor_width register_length;
};

struct lgcam_rear_sensor_reg {
	const struct lgcam_rear_sensor_i2c_reg_conf *pll;
	uint16_t pll_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *init;
	uint16_t init_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *prev_reg_settings;
	uint16_t prev_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *snap_reg_settings;
	uint16_t snap_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *ap001_16bit_settings;
	uint16_t ap001_16bit_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *ap003_16bit_settings;
	uint16_t ap003_16bit_settings_size;
	
	/*register for scene*/
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_normal_reg_settings;
	uint16_t scene_normal_reg_settings_size;	
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_portrait_reg_settings;
	uint16_t scene_portrait_reg_settings_size;	
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_landscape_reg_settings;
	uint16_t scene_landscape_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_sport_reg_settings;
	uint16_t scene_sport_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_sunset_reg_settings;
	uint16_t scene_sunset_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *scene_night_reg_settings;
	uint16_t scene_night_reg_settings_size;
	
	/*register for AF*/
	const struct lgcam_rear_sensor_i2c_reg_conf *AF_reg_settings;
	uint16_t AF_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *AF_nomal_reg_settings;
	uint16_t AF_nomal_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *AF_macro_reg_settings;
	uint16_t AF_macro_reg_settings_size;	
	const struct lgcam_rear_sensor_i2c_reg_conf *manual_focus_reg_settings;
	uint16_t manual_focus_reg_settings_size;	
	const struct lgcam_rear_sensor_i2c_reg_conf *CAF_reg_settings;
	uint16_t CAF_reg_settings_size;

	/*register for ISO*/
	const struct lgcam_rear_sensor_i2c_reg_conf *iso_auto_reg_settings;
	uint16_t iso_auto_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *iso_100_reg_settings;
	uint16_t iso_100_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *iso_200_reg_settings;
	uint16_t iso_200_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *iso_400_reg_settings;
	uint16_t iso_400_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *iso_800_reg_settings;
	uint16_t iso_800_reg_settings_size;	
	
	/*capture zoom°ü·Ã setting*/
	const struct lgcam_rear_sensor_i2c_reg_conf *zoom_mode_capture_127_reg_settings;
	uint16_t zoom_mode_capture_127_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *zoom_mode_capture_162_reg_settings;
	uint16_t zoom_mode_capture_162_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *zoom_mode_capture_203_reg_settings;
	uint16_t zoom_mode_capture_203_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *zoom_mode_capture_405_reg_settings;
	uint16_t zoom_mode_capture_405_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *focus_rect_reg_settings;
	uint16_t focus_rect_reg_settings_size;	

	const struct lgcam_rear_sensor_i2c_reg_conf *auto_frame_reg_settings;
	uint16_t auto_frame_reg_settings_size;
	const struct lgcam_rear_sensor_i2c_reg_conf *fixed_frame_reg_settings;
	uint16_t fixed_frame_reg_settings_size;	
};

/* this value is defined in Android native camera */
enum lgcam_rear_sensor_focus_mode {
	FOCUS_NORMAL,
	FOCUS_MACRO,
	FOCUS_AUTO,
	FOCUS_MANUAL,
	FOCUS_RECT,
	FOCUS_CONTINUOUS_VIDEO,
	FOCUS_CONTINUOUS_CAMERA,	
};

/* this value is defined in Android native camera */
enum lgcam_rear_sensor_wb_type {
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

enum lgcam_rear_sensor_antibanding_type {
	CAMERA_ANTIBANDING_OFF,
	CAMERA_ANTIBANDING_60HZ,
	CAMERA_ANTIBANDING_50HZ,
	CAMERA_ANTIBANDING_AUTO,
	CAMERA_MAX_ANTIBANDING,
};

/* Enum Type for different ISO Mode supported */
enum lgcam_rear_sensor_iso_value {
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
	CAMERA_SCENE_AUTO = 0,
	CAMERA_SCENE_LANDSCAPE = 1,
	CAMERA_SCENE_SUNSET = 4,
	CAMERA_SCENE_NIGHT = 5,
	CAMERA_SCENE_PORTRAIT = 6,
	CAMERA_SCENE_SPORTS = 8,
};

#if defined(CONFIG_MACH_MSM7X27_GELATO)
/* LGE_CHANGE_S. Change code to apply new LUT for display quality.
 * 2010-08-13. minjong.gong@lge.com */
extern void mdp_load_thunder_lut(int lut_type);
#endif

#endif /* lgcam_rear_sensor_H */
