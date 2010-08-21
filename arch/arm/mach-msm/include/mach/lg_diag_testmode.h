#ifndef LG_DIAG_TESTMODE_H
#define LG_DIAG_TESTMODE_H
// LG_FW_DIAG_KERNEL_SERVICE

#include "lg_comdef.h"

/*********************** BEGIN PACK() Definition ***************************/
#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif
/********************** END PACK() Definition *****************************/

#define MAX_KEY_BUFF_SIZE    200

typedef enum
{
  VER_SW=0,		//Binary Revision
  VER_DSP,      /* Camera DSP */
  VER_MMS,
  VER_CONTENTS,
  VER_PRL,
  VER_ERI,
  VER_BREW,
  VER_MODEL,  // 250-0-7 Test Mode Version
  VER_HW,
  REV_DSP=9,
  CONTENTS_SIZE,
  JAVA_FILE_CNT=13,
  JAVA_FILE_SIZE,
  VER_JAVA,
  BANK_ON_CNT=16,
  BANK_ON_SIZE,
  MODULE_FILE_CNT,
  MODULE_FILE_SIZE,
  MP3_DSP_OS_VER=21,
  VER_MODULE  ,
  VER_LCD_REVISION=24

} test_mode_req_version_type;

typedef enum
{
  MOTOR_OFF,
  MOTOR_ON
}test_mode_req_motor_type;

typedef enum
{
  ACOUSTIC_OFF=0,
  ACOUSTIC_ON,
  HEADSET_PATH_OPEN,
  HANDSET_PATH_OPEN,
  ACOUSTIC_LOOPBACK_ON,
  ACOUSTIC_LOOPBACK_OFF
}test_mode_req_acoustic_type;

typedef enum
{
  MP3_128KHZ_0DB,
  MP3_128KHZ_0DB_L,
  MP3_128KHZ_0DB_R,
  MP3_MULTISINE_20KHZ,
  MP3_PLAYMODE_OFF,
  MP3_SAMPLE_FILE,
  MP3_NoSignal_LR_128k
}test_mode_req_mp3_test_type;

typedef enum
{
  SPEAKER_PHONE_OFF,
  SPEAKER_PHONE_ON,
  NOMAL_Mic1,
  NC_MODE_ON,
  ONLY_MIC2_ON_NC_ON,
  ONLY_MIC1_ON_NC_ON
}test_mode_req_speaker_phone_type;

typedef enum
{
  VOL_LEV_OFF,
  VOL_LEV_MIN,
  VOL_LEV_MEDIUM,
  VOL_LEV_MAX
}test_mode_req_volume_level_type;

#ifndef LG_BTUI_TEST_MODE
typedef enum
{
  BT_GET_ADDR, //no use anymore
  BT_TEST_MODE_1=1,
  BT_TEST_MODE_CHECK=2,
  BT_TEST_MODE_RELEASE=5,
  BT_TEST_MODE_11=11 // 11~42
}test_mode_req_bt_type;

typedef enum
{
  BT_ADDR_WRITE=0,
  BT_ADDR_READ
}test_mode_req_bt_rw_type;

#define BT_RW_CNT 20

#endif //LG_BTUI_TEST_MODE

typedef enum
{
	CAM_TEST_MODE_OFF = 0,
	CAM_TEST_MODE_ON,
	CAM_TEST_SHOT,
	CAM_TEST_SAVE_IMAGE,
	CAM_TEST_CALL_IMAGE,
	CAM_TEST_ERASE_IMAGE,
	CAM_TEST_FLASH_ON,
	CAM_TEST_FLASH_OFF = 9,
	CAM_TEST_CAMCORDER_MODE_OFF,
	CAM_TEST_CAMCORDER_MODE_ON,
	CAM_TEST_CAMCORDER_SHOT,
	CAM_TEST_CAMCORDER_SAVE_MOVING_FILE,
	CAM_TEST_CAMCORDER_PLAY_MOVING_FILE,
	CAM_TEST_CAMCORDER_ERASE_MOVING_FILE,
	CAM_TEST_CAMCORDER_FLASH_ON,
	CAM_TEST_CAMCORDER_FLASH_OFF,
	CAM_TEST_STROBE_LIGHT_ON,
	CAM_TEST_STROBE_LIGHT_OFF,
	CAM_TEST_CAMERA_SELECT = 22,
}test_mode_req_cam_type;

typedef enum
{
  EXTERNAL_SOCKET_MEMORY_CHECK,
  EXTERNAL_FLASH_MEMORY_SIZE,
  EXTERNAL_SOCKET_ERASE,
  EXTERNAL_FLASH_MEMORY_USED_SIZE = 4,
}test_mode_req_socket_memory;

typedef enum
{
  FIRST_BOOTING_COMPLETE_CHECK,
}test_mode_req_fboot;

typedef enum
{
  MEMORY_TOTAL_CAPA_TEST,
  MEMORY_USED_CAPA_TEST,
  MEMORY_REMAIN_CAPA_TEST
}test_mode_req_memory_capa_type;
#ifndef SKW_TEST
typedef enum
{
  FACTORY_RESET_CHECK,
  FACTORY_RESET_COMPLETE_CHECK,
  FACTORY_RESET_STATUS_CHECK,
  FACTORY_RESET_COLD_BOOT,
  FACTORY_RESET_ERASE_USERDATA = 0x0F, // for NPST dll
}test_mode_req_factory_reset_mode_type;

typedef enum{
  FACTORY_RESET_START = 0,
  FACTORY_RESET_INITIAL = 1,
  FACTORY_RESET_ARM9_END = 2,
  FACTORY_RESET_COLD_BOOT_START = 3,
  FACTORY_RESET_COLD_BOOT_END = 5,
  FACTORY_RESET_NA = 7,
}test_mode_factory_reset_status_type;

#endif
typedef enum
{
  SLEEP_MODE_ON,
  AIR_PLAIN_MODE_ON
}test_mode_sleep_mode_type;

/* LGE_FACTORY_TEST_MODE for Photo Sensor(ALC) */
typedef enum
{
	ALC_TEST_MODE_OFF=0,
	ALC_TEST_MODE_ON,
	ALC_TEST_CHECK_STATUS,
	ALC_TEST_AUTOTEST
} test_mode_req_alc_type;

typedef enum
{
  TEST_SCRIPT_ITEM_SET,
  TEST_SCRIPT_MODE_CHECK,
  CAL_DATA_BACKUP,
  CAL_DATA_RESTORE,
  CAL_DATA_ERASE,
  CAL_DATA_INFO
}test_mode_req_test_script_mode_type;

/* TEST_MODE_PID_TEST */
typedef enum
{
  PID_WRITE,
  PID_READ
}test_mode_req_pid_type;


/* TEST_MODE_SW_VERSION */
typedef enum
{
  SW_VERSION,
  SW_OUTPUT_VERSION,
  SW_COMPLETE_VERSION,
  SW_VERSION_CHECK
} test_mode_req_sw_version_type;

/* TEST_MODE_CAL_CHECK */
typedef enum
{
 CAL_CHECK,
 CAL_DATA_CHECK,
} test_mode_req_cal_check_type;

/* LGE_CHANGES_S, [jaffrhee@lge.com], 2010-08-03, <DB Integrity Check > */
/* TEST_MODE_DB_INTEGRITY_CHECK */
typedef enum
{
  DB_INTEGRITY_CHECK=0,
  DB_CHECK_DUMP_TO_INTERNAL_MEMORY,
  DB_CHECK_COPY_TO_SD_CARD
} test_mode_req_db_check;
/* LGE_CHANGES_E, [jaffrhee@lge.com], 2010-08-03, <DB Integrity Check > */

typedef union
{
  test_mode_req_version_type		version;
#ifndef LG_BTUI_TEST_MODE
  test_mode_req_bt_type	bt;
  byte					bt_rw[BT_RW_CNT];
#endif //LG_BTUI_TEST_MODE
  test_mode_req_socket_memory esm;  // external socket memory
  test_mode_req_fboot fboot;
  test_mode_req_memory_capa_type mem_capa;
  word key_data;
  test_mode_req_motor_type		  	motor;
  test_mode_req_acoustic_type 	  	acoustic;
  test_mode_req_mp3_test_type  		mp3_play;
  test_mode_req_speaker_phone_type	speaker_phone;
  test_mode_req_volume_level_type	volume_level;
  boolean key_test_start;
  test_mode_req_cam_type		 camera;
#ifndef SKW_TEST
  test_mode_req_factory_reset_mode_type  factory_reset;
#endif
  test_mode_sleep_mode_type sleep_mode;
  test_mode_req_test_script_mode_type test_mode_test_scr_mode;
  test_mode_req_pid_type		pid;	// pid Write/Read
  test_mode_req_sw_version_type	sw_version;
  test_mode_req_cal_check_type		cal_check;
  test_mode_req_db_check		db_check;
#if 0
  test_mode_req_lcd_type			lcd;
  test_mode_req_folder_type			folder;
  test_mode_req_motor_type			motor;
  test_mode_req_acoustic_type		acoustic;
  test_mode_req_midi_struct_type	midi;
  test_mode_req_vod_type			vod;
  test_mode_req_cam_type			cam;
  test_mode_req_buzzer_type		buzzer;
  byte							efs_integrity;
  byte							factory_init;
  byte							efs_integrity_detail;
  byte							tx_power;
  byte							m_format;
  test_mode_req_phone_clear_type	phone_clear;
  test_mode_brew_type                      brew;
  test_mode_req_mp3_test_type  mp3_play;
  test_mode_req_bt_type   bt;
// LG_FW 2004.05.10 hieonn created -----------------------------------------
#ifdef LG_FW_FACTORY_MODE_KEY_DETECTION
  boolean if_key_pressed_is_started_or_not; /* to test key_pressed event */
#endif // LG_FW_FACTORY_MODE_KEY_DETECTION
#ifdef LG_FW_FACTORY_MODE  // race 2005.10.28
  test_mode_req_factory_mode_type factory_mode;
#endif /* LG_FW_FACTORY_MODE */

// LG_FW : 2006.04.07 louvethee--------------------
#ifdef LG_FW_TEST_MODE_V6_4
  test_mode_req_batter_bar_type			batt;
  test_mode_req_speaker_phone_type		speaker_phone;
  byte Volume_Level_Test;
#endif  // LG_FW_TEST_MODE_V6_4
// ----------------------------------------------------------

  test_mode_req_memory_capa_type mem_capa;

#ifdef LG_FW_TEST_MODE_V6_7
  test_mode_req_virtual_sim_type		virtual_sim;
  test_mode_req_photo_sensor_type		photo_sensor;
  test_mode_req_vco_self_tunning_type	vco_self_tunning;
  test_mode_req_ext_socket_memory_type	ext_socket_memory;
  test_mode_req_irda_fmrt_finger_uim_type ext_device_cmd;
#endif
#ifdef LG_FW_TEST_MODE_V6_8
  test_mode_req_mrd_usb_type mrd_usb;
#endif

#ifdef LG_FW_BMA020_TESTMODE
  test_mode_req_geomagnetic_sensor_type geomagnetism;
#endif //LG_FW_BMA020_SENSOR

#ifdef LG_FW_PROXI_CAL
  test_mode_req_proximity_type test_mode_test_proxi_mode;
#endif

  test_mode_req_manual_test_mode_type test_manual_mode;

// LG_FW : 2008.07.29 hoonylove004--------------------------------------------
// RF CAL backup
#ifdef LG_FW_TEST_MODE_V7_1
  test_mode_req_test_script_mode_type test_mode_test_scr_mode;
#endif /*LG_FW_TEST_MODE_V7_1*/
//----------------------------------------------------------------------------
#endif
} test_mode_req_type;

typedef struct diagpkt_header
{
  byte opaque_header;
}PACKED diagpkt_header_type;

typedef struct DIAG_TEST_MODE_F_req_tag {
	diagpkt_header_type		xx_header;
	word					sub_cmd_code;
	test_mode_req_type		test_mode_req;
} PACKED DIAG_TEST_MODE_F_req_type;

typedef enum
{
  TEST_OK_S,
  TEST_FAIL_S,
  TEST_NOT_SUPPORTED_S
} PACKED test_mode_ret_stat_type;

typedef struct
{
    byte SVState;
    uint8 SV;
    uint16 MeasuredCNo;
} PACKED CGPSResultType;

typedef union
{
  test_mode_req_version_type		version;
  byte								str_buf[15];
  CGPSResultType TestResult[16];
  test_mode_req_motor_type		    motor;
  test_mode_req_acoustic_type 	    acoustic;
  test_mode_req_mp3_test_type 	    mp3_play;
  test_mode_req_speaker_phone_type  speaker_phone;
  test_mode_req_volume_level_type   volume_level;
  char key_pressed_buf[MAX_KEY_BUFF_SIZE];
  char  memory_check;
  unsigned int    socket_memory_size;
  unsigned int    socket_memory_usedsize;
  int boot_complete;
  test_mode_req_cam_type		 camera;
  unsigned int mem_capa;
  int manual_test;
  test_mode_req_pid_type		pid;
  test_mode_req_sw_version_type	sw_version;
  test_mode_req_cal_check_type		cal_check;
  test_mode_req_db_check                db_check;
#ifndef SKW_TEST
  test_mode_req_factory_reset_mode_type  factory_reset;
#endif
#ifndef LG_BTUI_TEST_MODE
  byte read_bd_addr[BT_RW_CNT];
#endif
#if 0
  test_mode_req_lcd_type			lcd;
  test_mode_req_folder_type			folder;
  test_mode_req_motor_type			motor;
  test_mode_req_acoustic_type		acoustic;
  test_mode_req_midi_struct_type	midi;
  test_mode_req_vod_type			vod;
  test_mode_req_cam_type			cam;
  test_mode_req_buzzer_type			buzzer;
  test_mode_rsp_efs_type   	efs;
  byte								factory_init;
  byte								tx_power;
  test_mode_rsp_efs_integrity_type	efs_integrity_detail;
  test_mode_rsp_phone_clear_type	phone_clear;
  test_mode_brew_type                brew;
  test_mode_rsp_bt_type     bt;
  byte							ext_socket_check;
  unsigned int		brew_cnt;
  unsigned long	brew_size;
  byte       batt_bar_count;

  // LG_FW : 2006.04.07 louvethee--------------------
#ifdef LG_FW_TEST_MODE_V6_4
  char       batt_voltage[5];
  byte		chg_stat;
 test_mode_req_mp3_test_type mp3_play;
#endif  // LG_FW_TEST_MODE_V6_4
// ----------------------------------------------------------
  byte       ant_bar_count;
  unsigned int mem_capa;
#ifdef LG_FW_FACTORY_MODE  // race 2005.10.28
  byte      factory_mode;
#endif

#ifdef LG_FW_TEST_MODE_V6_7
  byte photo_sensor;
  byte	    vco_table[16];
  byte		vco_value;
#endif

#ifdef LG_FW_BMA020_TESTMODE
//  char 	geomagnetic_acceleration[25];
  test_mode_rsp_geomagnetic_rsp_type  geomagnetic_value_rsp;
#endif //LG_FW_BMA020_SENSOR

#ifdef LG_FW_PROXI_CAL
  byte proximity_value;
#endif

  int manual_test;
#endif
} PACKED test_mode_rsp_type;

typedef struct DIAG_TEST_MODE_F_rsp_tag {
	diagpkt_header_type		xx_header;
	word					sub_cmd_code;
	test_mode_ret_stat_type	ret_stat_code;
	test_mode_rsp_type		test_mode_rsp;
} PACKED DIAG_TEST_MODE_F_rsp_type;

typedef enum
{
  TEST_MODE_VERSION=0,
  TEST_MODE_LCD,
  TEST_MODE_MOTOR=3,
  TEST_MODE_ACOUSTIC,
  TEST_MODE_CAM=7,
  TEST_MODE_EFS_INTEGRITY=11,
  TEST_MODE_IRDA_FMRT_FINGER_UIM_TEST=13,
  TEST_MODE_BREW_CNT=20,
  TEST_MODE_BREW_SIZE=21,
  TEST_MODE_KEY_TEST,    //LGF_TM_KEY_PAD_TEST
  TEST_MODE_EXT_SOCKET_TEST,
#ifndef LG_BTUI_TEST_MODE
  TEST_MODE_BLUETOOTH_TEST,
#endif //LG_BTUI_TEST_MODE
  TEST_MODE_BATT_LEVEL_TEST,
  TEST_MODE_MP3_TEST=27,
  TEST_MODE_FM_TRANCEIVER_TEST,
  TEST_MODE_ISP_DOWNLOAD_TEST,
  TEST_MODE_COMPASS_SENSOR_TEST=30,     // Geometric (Compass) Sensor
  TEST_MODE_ACCEL_SENSOR_TEST=31,
  TEST_MODE_ALCOHOL_SENSOR_TEST=32,
  TEST_MODE_TDMB_TEST=33,
  TEST_MODE_WIFI_TEST=33,
  TEST_MODE_TV_OUT_TEST=33,
  TEST_MODE_SDMB_TEST=33,
  TEST_MODE_MANUAL_MODE_TEST=36,   // Manual test
  TEST_MODE_UV_SENSOR_TEST=38,
  TEST_MODE_3D_ACCELERATOR_SENSOR_TEST=39,

  TEST_MODE_KEY_DATA_TEST = 40,  // Key Code Input
  TEST_MODE_MEMORY_CAPA_TEST,  // Memory Volume Check
  TEST_MODE_SLEEP_MODE_TEST,
  TEST_MODE_SPEAKER_PHONE_TEST,	// Speaker Phone test

  TEST_MODE_VIRTUAL_SIM_TEST = 44,
  TEST_MODE_PHOTO_SENSOR_TEST,
  TEST_MODE_VCO_SELF_TUNNING_TEST,

  TEST_MODE_MRD_USB_TEST=47,
  TEST_MODE_TEST_SCRIPT_MODE = 48,

  TEST_MODE_PROXIMITY_SENSOR_TEST = 49,
  TEST_MODE_FACTORY_RESET_CHECK_TEST = 50,
  TEST_MODE_VOLUME_TEST=51,
  TEST_MODE_HANDSET_FREE_ACTIVATION_TEST,
  TEST_MODE_MOBILE_SYSTEM_CHANGE_TEST,
  TEST_MODE_STANDALONE_GPS_TEST,
  TEST_MODE_PRELOAD_INTEGRITY_TEST,
  TEST_MODE_FIRST_BOOT_COMPLETE_TEST = 58,

  TEST_MODE_PID_TEST = 70,		// pid R/W
  TEST_MODE_SW_VERSION = 71,
  TEST_MODE_IME_TEST,
  TEST_MODE_IMPL_TEST,
  TEST_MODE_SIM_LOCK_TYPE_TEST,
  TEST_MODE_UNLOCK_CODE_TEST,
  TEST_MODE_IDDE_TEST,
  TEST_MODE_FULL_SIGNATURE_TEST,
  TEST_MODE_NT_CODE_TEST,
  TEST_MODE_SIM_ID_TEST = 79,

  TEST_MODE_CAL_CHECK= 82,
#ifndef LG_BTUI_TEST_MODE
  TEST_MODE_BLUETOOTH_TEST_RW=83,
#endif //LG_BTUI_TEST_MODE
  TEST_MODE_SKIP_WELCOM_TEST = 87,
  //[START]LGE_DB_CHECK: jaffrhee@lge.com 2010-08-02
  TEST_MODE_DB_INTEGRITY_CHECK=91,
  //[END]LGE_DB_CHECK: jaffrhee@lge.com 2010-08-02
  //
  MAX_TEST_MODE_SUBCMD = 0xFFFF
  //TEST_MODE_CURRENT,
  //TEST_MODE_BREW_FILES,
} PACKED test_mode_sub_cmd_type;

#define TESTMODE_MSTR_TBL_SIZE   128

#define ARM9_PROCESSOR       0
#define ARM11_PROCESSOR     1

typedef void*(* testmode_func_type)(test_mode_req_type * , DIAG_TEST_MODE_F_rsp_type * );

typedef struct
{
  word cmd_code;
	testmode_func_type func_ptr;
  byte  which_procesor;             // to choose which processor will do act.
}testmode_user_table_entry_type;

/* LGE_CHANGES_S, [dongp.kim@lge.com], 2010-01-10, <LGE_FACTORY_TEST_MODE for WLAN RF Test > */
typedef struct
{
	uint16 countresult;
	uint16 wlan_status;
	uint16 g_wlan_status;
	uint16 rx_channel;
	uint16 rx_per;
	uint16 tx_channel;
	uint32 goodFrames;
	uint16 badFrames;
	uint16 rxFrames;
	uint16 wlan_data_rate;
	uint16 wlan_payload;
	uint16 wlan_data_rate_recent;
	unsigned long pktengrxducast_old;
	unsigned long pktengrxducast_new;
	unsigned long rxbadfcs_old;
	unsigned long rxbadfcs_new;
	unsigned long rxbadplcp_old;
	unsigned long rxbadplcp_new;

}wlan_status;
/* LGE_CHANGES_E, [dongp.kim@lge.com], 2010-01-10, <LGE_FACTORY_TEST_MODE for WLAN RF Test > */

typedef struct DIAG_TEST_MODE_KEY_F_rsp_tag {
  diagpkt_header_type		xx_header;
  word					sub_cmd_code;
  test_mode_ret_stat_type	ret_stat_code;
  char key_pressed_buf[MAX_KEY_BUFF_SIZE];
} PACKED DIAG_TEST_MODE_KEY_F_rsp_type;

#endif /* LG_DIAG_TESTMODE_H */
