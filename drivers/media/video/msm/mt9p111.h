/*
 * drivers/media/video/msm/mt9p111.h
 *
 * Aptina MT9P111 1/4-Inch 5MP System-On-Chip(SOC) CMOS Digital Image Sensor
 *
 * Copyright (C) 2011 LG Electronics.
 * Author: taiyou.kang@lge.com, 2011-03-10
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 */

#ifndef MT9P111_H
#define MT9P111_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct mt9p111_reg mt9p111_regs;

#define USE_CONTINUOUS_AF
#define USE_I2C_BURST_MODE

#define MT9P111_AF_FAIL 0
#define MT9P111_AF_SUCCESS 1
#define MT9P111_AF_SEARCHING 2

#define MT9P111_TOTAL_STEPS_NEAR_TO_FAR (10)

#define MAX_I2C_BURST_NUM (3000)

enum mt9p111_width
{
	BYTE_LEN = 1,
	WORD_LEN = 2,
	QUAD_LEN = 4,
};

enum mt9p111_type
{
	CMD_WRITE = 1,
	CMD_POLL = 2,
	CMD_DELAY = 3,
#if defined(USE_I2C_BURST_MODE)		
	CMD_WRITE_BURST_S = 4,
	CMD_WRITE_BURST_E = 5,
#endif	
};

struct mt9p111_register_pair
{
	enum mt9p111_type register_type;
	uint16_t register_address;
	uint32_t register_value;
	enum mt9p111_width register_length;
};

struct mt9p111_reg
{
	const struct mt9p111_register_pair *init_settings;
	uint16_t init_settings_size;
	
	const struct mt9p111_register_pair *init2_settings;
	uint16_t init2_settings_size;
	
	const struct mt9p111_register_pair *prev_settings;
	uint16_t prev_settings_size;
	
	const struct mt9p111_register_pair *snap_settings;
	uint16_t snap_settings_size;
	
	const struct mt9p111_register_pair *af_macro_settings;
	uint16_t af_macro_settings_size;
	
	const struct mt9p111_register_pair *af_auto_settings;
	uint16_t af_auto_settings_size;

	const struct mt9p111_register_pair *af_continuous_video_settings;
	uint16_t af_continuous_video_settings_size;

	const struct mt9p111_register_pair *af_continuous_camera_settings;
	uint16_t af_continuous_camera_settings_size;

	const struct mt9p111_register_pair *wb_auto_settings;
	uint16_t wb_auto_settings_size;

	const struct mt9p111_register_pair *wb_cloudy_settings;
	uint16_t wb_cloudy_settings_size;

	const struct mt9p111_register_pair *wb_daylight_settings;
	uint16_t wb_daylight_settings_size;

	const struct mt9p111_register_pair *wb_fluorescent_settings;
	uint16_t wb_fluorescent_settings_size;

	const struct mt9p111_register_pair *wb_incandescent_settings;
	uint16_t wb_incandescent_settings_size;

	const struct mt9p111_register_pair *effect_off_settings;
	uint16_t effect_off_settings_size;

	const struct mt9p111_register_pair *effect_gray_settings;
	uint16_t effect_gray_settings_size;

	const struct mt9p111_register_pair *effect_green_settings;
	uint16_t effect_green_settings_size;

	const struct mt9p111_register_pair *effect_cool_settings;
	uint16_t effect_cool_settings_size;

	const struct mt9p111_register_pair *effect_yellow_settings;
	uint16_t effect_yellow_settings_size;

	const struct mt9p111_register_pair *effect_sepia_settings;
	uint16_t effect_sepia_settings_size;

	const struct mt9p111_register_pair *effect_purple_settings;
	uint16_t effect_purple_settings_size;

	const struct mt9p111_register_pair *effect_red_settings;
	uint16_t effect_red_settings_size;

	const struct mt9p111_register_pair *effect_pink_settings;
	uint16_t effect_pink_settings_size;

	const struct mt9p111_register_pair *effect_aqua_settings;
	uint16_t effect_aqua_settings_size;

	const struct mt9p111_register_pair *effect_negative_settings;
	uint16_t effect_negative_settings_size;

	const struct mt9p111_register_pair *effect_solarize_settings;
	uint16_t effect_solarize_settings_size;

	const struct mt9p111_register_pair *exposure_normal_settings;
	uint16_t exposure_normal_settings_size;

	const struct mt9p111_register_pair *exposure_spot_settings;
	uint16_t exposure_spot_settings_size;

	const struct mt9p111_register_pair *exposure_avg_settings;
	uint16_t exposure_avg_settings_size;

	const struct mt9p111_register_pair *scene_auto_settings;
	uint16_t scene_auto_settings_size;

	const struct mt9p111_register_pair *scene_landscape_settings;
	uint16_t scene_landscape_settings_size;

	const struct mt9p111_register_pair *scene_sunset_settings;
	uint16_t scene_sunset_settings_size;

	const struct mt9p111_register_pair *scene_night_settings;
	uint16_t scene_night_settings_size;

	const struct mt9p111_register_pair *scene_portrait_settings;
	uint16_t scene_portrait_settings_size;

	const struct mt9p111_register_pair *scene_sports_settings;
	uint16_t scene_sports_settings_size;
};

enum mt9p111_af_mode
{
	FOCUS_NORMAL = 0,
	FOCUS_MACRO= 1,
	FOCUS_AUTO = 2,
	FOCUS_MANUAL = 3,
	FOCUS_RECT = 4,
	FOCUS_CONTINUOUS_VIDEO = 5,
	FOCUS_CONTINUOUS_CAMERA = 6
};

enum mt9p111_af_manual
{
	FOCUS_MANUAL_MIN = 0,
	FOCUS_MANUAL_POS_0 = 0,
	FOCUS_MANUAL_POS_1 = 1,
	FOCUS_MANUAL_POS_2 = 2,
	FOCUS_MANUAL_POS_3 = 3,
	FOCUS_MANUAL_POS_4 = 4,
	FOCUS_MANUAL_POS_5 = 5,
	FOCUS_MANUAL_POS_6 = 6,
	FOCUS_MANUAL_POS_7 = 7,
	FOCUS_MANUAL_POS_8 = 8,
	FOCUS_MANUAL_POS_9 = 9,
	FOCUS_MANUAL_MAX = 9
};

enum mt9p111_exposure_mode
{
	EXPOSURE_NORMAL,
	EXPOSURE_SPOT,
	EXPOSURE_AVG
};

enum mt9p111_scene_mode
{
	SCENE_AUTO = 0,
	SCENE_LANDSCAPE = 1,
	SCENE_SUNSET = 4,
	SCENE_NIGHT = 5,
	SCENE_PORTRAIT = 6,
	SCENE_SPORTS = 8,
};

enum mt9p111_frame_mode
{
	FPS_VARIABLE,
	FPS_FIXED
};

enum mt9p111_effect_mode
{
	EFFECT_OFF,
	EFFECT_GRAY,
	EFFECT_NEGATIVE,
	EFFECT_SOLARIZE,
	EFFECT_SEPIA,
	EFFECT_GREEN,
	EFFECT_COOL,
	EFFECT_YELLOW,
	EFFECT_AQUA,
	EFFECT_PURPLE,
	EFFECT_RED,
	EFFECT_PINK,
};

enum mt9p111_wb_mode
{
	WB_AUTO = 1,
	WB_INCANDESCENT = 3,
	WB_FLUORESCENT = 4,
	WB_DAYLIGHT = 5,
	WB_CLOUDY = 6
};

enum mt9p111_brightness_mode
{
	BRIGHTNESS_MIN = 0,
	BRIGHTNESS_M5 = 0,
	BRIGHTNESS_M4 = 1,
	BRIGHTNESS_M3 = 2,
	BRIGHTNESS_M2 = 3,
	BRIGHTNESS_M1 = 4,
	BRIGHTNESS_NORMAL = 5,
	BRIGHTNESS_P1 = 6,
	BRIGHTNESS_P2 = 7,
	BRIGHTNESS_P3 = 8,
	BRIGHTNESS_P4 = 9,
	BRIGHTNESS_P5 = 10,
	BRIGHTNESS_MAX = 10
};

enum mt9p111_zoom_mode
{
	ZOOM_MIN = 0,
	ZOOM_1P0 = 0,
	ZOOM_1P2 = 1,
	ZOOM_1P4 = 2,
	ZOOM_1P6 = 3,
	ZOOM_1P8 = 4,
	ZOOM_2P0 = 5,
	ZOOM_MAX = 5
};

enum mt9p111_iso_mode
{
	ISO_AUTO = 0,
	ISO_50 = 1,
	ISO_100 = 2,
	ISO_200 = 3,
	ISO_400 = 4,
	ISO_800 = 5,
	ISO_MAX
};

//#define CAM_TXT_TUNING // Enable this define for camera sensor tuning via SD card
#if defined(CAM_TXT_TUNING)
#define CMD_BUF_MAX_LENGTH (10)
#define DEC_VAL (1)
#define HEX_VAL (2)

#define INIT_FILE "init.txt"
#define INIT2_FILE "init2.txt"
#define LENS_CHECK_FILE "lens_check.txt"
#define LENS_ZONE0_FILE "lens_zone0.txt"
#define LENS_DEFAULT_FILE "lens_default.txt"
#define PREVIEW_FILE "preview.txt"
#define SNAPSHOT_FILE "snapshot.txt"
#define FOCUS_MACRO_FILE "focus_macro.txt"
#define FOCUS_AUTO_FILE "focus_auto.txt"
#define FOCUS_CONTINUOUS_VIDEO_FILE "focus_continuous_video.txt"
#define FOCUS_CONTINUOUS_CAMERA_FILE "focus_continuous_camera.txt"
#define EXPOSURE_NORMAL_FILE "exposure_normal.txt"
#define EXPOSURE_SPOT_FILE "exposure_spot.txt"
#define EXPOSURE_AVG_FILE "exposure_avg.txt"
#define SCENE_AUTO_FILE "scene_auto.txt"
#define SCENE_LANDSCAPE_FILE "scene_landscape.txt"
#define SCENE_SUNSET_FILE "scene_sunset.txt"
#define SCENE_NIGHT_FILE "scene_night.txt"
#define SCENE_PORTRAIT_FILE "scene_portrait.txt"
#define SCENE_SPORTS_FILE "scene_sports.txt"
#define FPS_VARIABLE_FILE "fps_variable.txt"
#define FPS_FIXED_FILE "fps_fixed.txt"
#define EFFECT_OFF_FILE "effect_off.txt"
#define EFFECT_GRAY_FILE "effect_gray.txt"
#define EFFECT_NEGATIVE_FILE "effect_negative.txt"
#define EFFECT_SOLARIZE_FILE "effect_solarize.txt"
#define EFFECT_SEPIA_FILE "effect_sepia.txt"
#define EFFECT_GREEN_FILE "effect_green.txt"
#define EFFECT_COOL_FILE "effect_cool.txt"
#define EFFECT_YELLOW_FILE "effect_yellow.txt"
#define EFFECT_AQUA_FILE "effect_aqua.txt"
#define EFFECT_PURPLE_FILE "effect_purple.txt"
#define EFFECT_RED_FILE "effect_red.txt"
#define EFFECT_PINK_FILE "effect_pink.txt"
#define WB_AUTO_FILE "wb_auto.txt"
#define WB_INCANDESCENT_FILE "wb_incandescent.txt"
#define WB_FLUORESCENT_FILE "wb_fluorescent.txt"
#define WB_DAYLIGHT_FILE "wb_daylight.txt"
#define WB_CLOUDY_FILE "wb_cloudy.txt"
#define ISO_AUTO_FILE "iso_auto.txt"
#define ISO_50_FILE "iso_50.txt"
#define ISO_100_FILE "iso_100.txt"
#define ISO_200_FILE "iso_200.txt"
#define ISO_400_FILE "iso_400.txt"
#define ISO_800_FILE "iso_800.txt"
#define BRIGHTNESS_M5_FILE "brightness_m5.txt"
#define BRIGHTNESS_M4_FILE "brightness_m4.txt"
#define BRIGHTNESS_M3_FILE "brightness_m3.txt"
#define BRIGHTNESS_M2_FILE "brightness_m2.txt"
#define BRIGHTNESS_M1_FILE "brightness_m1.txt"
#define BRIGHTNESS_NORMAL_FILE "brightness_normal.txt"
#define BRIGHTNESS_P1_FILE "brightness_p1.txt"
#define BRIGHTNESS_P2_FILE "brightness_p2.txt"
#define BRIGHTNESS_P3_FILE "brightness_p3.txt"
#define BRIGHTNESS_P4_FILE "brightness_p4.txt"
#define BRIGHTNESS_P5_FILE "brightness_p5.txt"
#define ZOOM_1P0_FILE "zoom_1p0.txt"
#define ZOOM_1P2_FILE "zoom_1p2.txt"
#define ZOOM_1P4_FILE "zoom_1p4.txt"
#define ZOOM_1P6_FILE "zoom_1p6.txt"
#define ZOOM_1P8_FILE "zoom_1p8.txt"
#define ZOOM_2P0_FILE "zoom_2p0.txt"
#define FOCUS_MANUAL_0_FILE "foucs_manual_0"
#define FOCUS_MANUAL_1_FILE "foucs_manual_1"
#define FOCUS_MANUAL_2_FILE "foucs_manual_2"
#define FOCUS_MANUAL_3_FILE "foucs_manual_3"
#define FOCUS_MANUAL_4_FILE "foucs_manual_4"
#define FOCUS_MANUAL_5_FILE "foucs_manual_5"
#define FOCUS_MANUAL_6_FILE "foucs_manual_6"
#define FOCUS_MANUAL_7_FILE "foucs_manual_7"
#define FOCUS_MANUAL_8_FILE "foucs_manual_8"
#define FOCUS_MANUAL_9_FILE "foucs_manual_9"

#define CAM_TXT_SENSOR "mt9p111"
#define CAM_TXT_DIR "/sdcard/cam_txt"
#define CAM_TXT_FILE(_name) CAM_TXT_DIR"/"CAM_TXT_SENSOR"_"_name

int cam_txt_from_file(char *file_name);
#endif

#endif /* MT9P111_H */

