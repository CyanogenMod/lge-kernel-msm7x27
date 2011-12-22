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

#ifndef MT9T113_H
#define MT9T113_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct mt9t113_reg mt9t113_regs;
extern int mclk_rate;

enum mt9t113_width {
	BYTE_LEN,
	WORD_LEN,
};

struct mt9t113_register_address_value_pair {
	uint16_t register_address;
	uint16_t register_value;
	enum mt9t113_width register_length;
};

struct mt9t113_reg {
	const struct mt9t113_register_address_value_pair *init_reg_settings;
	uint16_t init_reg_settings_size;

	const struct mt9t113_register_address_value_pair *tuning_reg_settings;
	uint16_t tuning_reg_settings_size;
	
	/* framerate mode start */
	const struct mt9t113_register_address_value_pair *auto_framerate_reg_settings;
	uint16_t auto_framerate_reg_settings_size;
	
	const struct mt9t113_register_address_value_pair *fixed_framerate_reg_settings;
	uint16_t fixed_framerate_reg_settings_size;	
	/* framerate mode end */

	const struct mt9t113_register_address_value_pair *prev_reg_settings;
	uint16_t prev_reg_settings_size;
	const struct mt9t113_register_address_value_pair *snap_reg_settings;
	uint16_t snap_reg_settings_size;

 /* effect start */
 const struct mt9t113_register_address_value_pair *effect_off_reg_settings;
 uint16_t effect_off_reg_settings_size;

 const struct mt9t113_register_address_value_pair *effect_mono_reg_settings;
 uint16_t effect_mono_reg_settings_size;

 const struct mt9t113_register_address_value_pair *effect_negative_reg_settings;
 uint16_t effect_negative_reg_settings_size;

 const struct mt9t113_register_address_value_pair *effect_solarize_reg_settings;
 uint16_t effect_solarize_reg_settings_size;

 const struct mt9t113_register_address_value_pair *effect_sepia_reg_settings;
 uint16_t effect_sepia_reg_settings_size;

 const struct mt9t113_register_address_value_pair *effect_aqua_reg_settings;
 uint16_t effect_aqua_reg_settings_size;
 
 /* white balance start */
 const struct mt9t113_register_address_value_pair *wb_auto_reg_settings;
 uint16_t wb_auto_reg_settings_size;

 const struct mt9t113_register_address_value_pair *wb_incandescent_reg_settings;
 uint16_t wb_incandescent_reg_settings_size;

 const struct mt9t113_register_address_value_pair *wb_fluorescent_reg_settings;
 uint16_t wb_fluorescent_reg_settings_size;

 const struct mt9t113_register_address_value_pair *wb_daylight_reg_settings;
 uint16_t wb_daylight_reg_settings_size;

 const struct mt9t113_register_address_value_pair *wb_cloudy_reg_settings;
 uint16_t wb_cloudy_reg_settings_size; 
 
 /* iso start */
 const struct mt9t113_register_address_value_pair *iso_auto_reg_settings;
 uint16_t iso_auto_reg_settings_size;
 
 const struct mt9t113_register_address_value_pair *iso_100_reg_settings;
 uint16_t iso_100_reg_settings_size;
 
 const struct mt9t113_register_address_value_pair *iso_200_reg_settings;
 uint16_t iso_200_reg_settings_size;
 
 const struct mt9t113_register_address_value_pair *iso_400_reg_settings;
 uint16_t iso_400_reg_settings_size;   

 /* brightness start */
	const struct mt9t113_register_address_value_pair *brightness_reg_settings;
	uint16_t brightness_reg_settings_size; 
};

/* this value is defined in Android native camera */
enum mt9t113_wb_type {
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

/* Enum Type for different ISO Mode supported */
enum mt9t113_iso_value {
	CAMERA_ISO_AUTO = 0,
	CAMERA_ISO_DEBLUR,
	CAMERA_ISO_100,
	CAMERA_ISO_200,
	CAMERA_ISO_400,
	CAMERA_ISO_800,
	CAMERA_ISO_MAX
};

// 2010-11-24 change the framerate mode between capture and video
enum mt9t113_fps_mode {
 FRAME_RATE_AUTO = 0,
 FRAME_RATE_FIXED
};

#endif /* MT9T113_H */

