/* drivers/input/touchscreen/qt602240.c
 *
 * Quantum TSP driver.
 *
 * Copyright (C) 2009 Samsung Electronics Co. Ltd.
 *
 * 2010 Modified by LG Electronics Co., Ltd. 
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/earlysuspend.h>
#include <linux/jiffies.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>
#include <linux/i2c-gpio.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#include "qt602240.h"

// [LGE PATCH : START] edward1.kim@lge.com 20110216  
#define ON 	1
#define OFF 	0
// [LGE PATCH : END] edward1.kim@lge.com 20110216  

struct i2c_driver qt602240_i2c_driver;

struct workqueue_struct *qt602240_wq = NULL;

struct qt602240_data* qt602240;
struct touch_platform_data *qt602240_pdata;


#if ENABLE_NOISE_TEST_MODE
                                       //botton_right    botton_left            center              top_right          top_left 
unsigned char test_node[TEST_POINT_NUM] = {12,       		   20, 		   104,    		        188,     			   196};        

unsigned int return_refer_0, return_refer_1, return_refer_2, return_refer_3, return_refer_4;
unsigned int return_delta_0, return_delta_1, return_delta_2, return_delta_3, return_delta_4;
uint16_t diagnostic_addr;
#endif

#ifdef _SUPPORT_MULTITOUCH_
static report_finger_info_t fingerInfo[MAX_USING_FINGER_NUM];
static int qt_initial_ok=0;
#endif

#ifdef QT_STYLUS_ENABLE
static int config_mode_val = 0; 	//0: normal 1: stylus
#endif

#ifdef FEATUE_QT_INFOBLOCK_STATIC		//by donghoon.kwak
static info_block_t info_block_data;
static info_block_t *info_block=&info_block_data;

static report_id_map_t report_id_map_data[30];
static report_id_map_t *report_id_map=&(report_id_map_data[0]);

static object_t info_object_table[25];
static object_t *info_object_ptr=&info_object_table[0];

/*! Message buffer holding the latest message received. */
uint8_t quantum_msg[9]={0};
#else
static info_block_t *info_block;

static report_id_map_t *report_id_map;

/*! Message buffer holding the latest message received. */
uint8_t *quantum_msg;
#endif

volatile uint8_t read_pending;

static int max_report_id = 0;

uint8_t max_message_length;

uint16_t message_processor_address;

/*! Command processor address. */
static uint16_t command_processor_address;

/*! Flag indicating if driver setup is OK. */
static enum driver_setup_t driver_setup = DRIVER_SETUP_INCOMPLETE;

/*! \brief The current address pointer. */
static U16 address_pointer;

static uint8_t tsp_version;

//20100217 julia
static uint8_t cal_check_flag = 0u;
//static int CAL_THR = 10;

//20100913 sooo.shin
static uint8_t force_cal_check_flag = 0;

static unsigned char is_inputmethod=0;

struct wake_lock tsp_firmware_wake_lock;
unsigned char qt60224_notfound_flag=0;
//static int firmware_ret_val = -1;

static unsigned int qt602240_init_fail = 1;

//extern unsigned int charging_mode_get(void);
extern int get_boot_charger_info(void);
extern int isVoiceCall;
int touch_isVoiceCall_state = 0;

/* /sys/devices/virtual/sec/ts/trace */
int qt602240_trace_enable = 1;
#define QT602240_ERROR(fmt, args...) {if(qt602240_trace_enable & 0x01) printk("[%s] " fmt, __func__, ##args);}
#define QT602240_TRACE(fmt, args...) {if(qt602240_trace_enable & 0x02) printk("[%s] " fmt, __func__, ##args);}

enum
{
	QT_TOUCHKEY_MENU = 0x01,
	QT_TOUCHKEY_HOME = 0x02,
	QT_TOUCHKEY_BACK = 0x04,
//	QT_TOUCHKEY_SEARCH,
	QT_TOUCHKEY_MAX
};

typedef struct 
{
	int16_t key;		/* linux key type */
	int16_t touchkey;	/* touch keyarray number */
	int16_t id;		/* pressed id */
} touchkey_info_t;

static touchkey_info_t touchkey_array[QT_TOUCHKEY_MAX] = 
{
	[QT_TOUCHKEY_MENU] = { KEY_MENU,	0x1,	0xFF	},	/* QT_TOUCHKEY_MENU */
	[QT_TOUCHKEY_HOME] = { KEY_HOME,	0x2,	0xFF	},	/* QT_TOUCHKEY_HOME */
	[QT_TOUCHKEY_BACK] = { KEY_BACK,	0x4,	0xFF	},	/* QT_TOUCHKEY_BACK */
};

#ifdef LG_FW_TOUCH_SOFT_KEY

typedef struct
{
	int16_t x1;         /* x start        */
	int16_t y1;         /* y start        */
	int16_t x2;         /* x end          */
	int16_t y2;         /* y end          */
	int16_t amp_thrs;	/* amp threshold  */
	int16_t size_thrs;	/* size threshold */
	int16_t key;        /* linux key type */
	int16_t id;         /* pressed id     */

} touchkey_area_info_t;

static touchkey_area_info_t touchkey_area[QT_TOUCHKEY_MAX] = {
/*     x1,   y1,   x2,    y2,  amp, size,        key,      id                          */
    {    9,  550,   49,  595,  500,   50,   KEY_MENU,    0xFF }, /* QT_TOUCHKEY_MENU   */
    {   92,  550,  136,  595,  500,   50,   KEY_HOME,    0xFF }, /* QT_TOUCHKEY_HOME   */
    {  177,  550,  222,  595,  500,   50,   KEY_BACK,    0xFF }, /* QT_TOUCHKEY_BACK   */
//    {  263,  550,  306,  595,  500,   50, KEY_SEARCH,    0xFF }  /* QT_TOUCHKEY_SEARCH */
};
#endif


/*------------------------------ for tunning ATmel - start ----------------------------*/
struct class *touch_class;
EXPORT_SYMBOL(touch_class);

struct device *switch_test;
EXPORT_SYMBOL(switch_test);

#ifdef QT_STYLUS_ENABLE
struct device *qt_stylus;
EXPORT_SYMBOL(qt_stylus);
#endif

/******************************************************************************
* 
*
*       QT602240 Object table init
* 
*                                                             
* *****************************************************************************/
//General Object
gen_commandprocessor_t6_config_t command_config = {0};                 //sooo.shin_100728
gen_powerconfig_t7_config_t power_config = {0};                 //Power config settings.
gen_acquisitionconfig_t8_config_t acquisition_config = {0};     // Acquisition config. 


//Touch Object

touch_multitouchscreen_t9_config_t touchscreen_config = {0};    //Multitouch screen config.
touch_keyarray_t15_config_t keyarray_config = {0};              //Key array config.
touch_proximity_t23_config_t proximity_config = {0};        //Proximity config. 


//Signal Processing Objects

proci_gripfacesuppression_t20_config_t gripfacesuppression_config = {0};    //Grip / face suppression config.
procg_noisesuppression_t22_config_t noise_suppression_config = {0};         //Noise suppression config.
proci_onetouchgestureprocessor_t24_config_t onetouch_gesture_config = {0};  //One-touch gesture config. 
proci_twotouchgestureprocessor_t27_config_t twotouch_gesture_config = {0};  //Two-touch gesture config.


//Support Objects

spt_gpiopwm_t19_config_t  gpiopwm_config = {0};             //GPIO/PWM config
spt_selftest_t25_config_t selftest_config = {0};            //Selftest config.
spt_cteconfig_t28_config_t cte_config = {0};                //Capacitive touch engine config.

spt_comcconfig_t18_config_t   comc_config = {0};            //Communication config settings.

spt_userdata_t38_config_t userdata38_config; 

#if defined(USE_CONFIG_TABLE)
typedef struct 
{
	spt_userdata_t38_config_t t38; // userdata
	gen_powerconfig_t7_config_t t7; // power config
	gen_acquisitionconfig_t8_config_t t8; // acquisition config
	touch_multitouchscreen_t9_config_t t9; // multitouch screen
	touch_keyarray_t15_config_t t15; // key array
	spt_comcconfig_t18_config_t t18; // comc config
	spt_gpiopwm_t19_config_t t19; // gpio pwm
	proci_gripfacesuppression_t20_config_t t20; // grip face suppression
	procg_noisesuppression_t22_config_t t22; // noise suppression
	touch_proximity_t23_config_t t23; // proximity
	proci_onetouchgestureprocessor_t24_config_t t24; // one touch gesture processor
	spt_selftest_t25_config_t t25; // self test
	spt_cteconfig_t28_config_t t28; // cte config
} qt602240_config;

const qt602240_config touch_config =
{
	{ // SPT_USERDATA_T38
		{0, 0, 0, 0, 0, 0, 0, 0} // data[0]~data[7]
	},
	{ // GEN_POWERCONFIG_T7
		64, // IDLEACQINT
		12, // ACTVACQINT
		50 // ACTV2IDLETO
	},
	{ // GEN_ACQUISITIONCONFIG_T8
		10, // CHRGTIME
		0, // ATCHDRIFT
		5, // TCHDRIFT
		5, // DRIFTST
		0, // TCHAUTOCAL
		0, // SYNC
		12, // ATCHCALST
		5, // ATCHCALSTHR
		0, // ATCHFRCCALTHR
		0 // ATCHFRCCALRATIO
	},
	{ // TOUCH_MULTITOUCHSCREEN_T9
		143, // CTRL
		0, // XORIGIN
		2, // YORIGIN
		14, // XSIZE
		9, // YSIZE
		0, // AKSCFG
		0, // BLEN
		30, // TCHTHR
		2, // TCHDI
		1, // ORIENT
		0, // MRGTIMEOUT
		0, // MOVHYSTI
		0, // MOVHYSTN
		79, // MOVFILTER
		8, // NUMTOUCH
		3, // MRGHYST
		26, // MRGTHR
		6, // AMPHYST
		479, // XRANGE
		319, // YRANGE
		10, // XLOCLIP
		10, // XHICLIP
		35, // YLOCLIP
		35, // YHICLIP
		136, // XEDGECTRL
		90, // XEDGEDIST
		136, // YEDGECTRL
		90, // YEDGEDIST
		15, // JUMPLIMIT
		0 // TCHHYST
	},
	{ // TOUCH_KEYARRAY_T15
		131, // CTRL
		11, // XORIGIN
		11, // YORIGIN
		3, // XSIZE
		1, // YSIZE
		0, // AKSCFG
		0, // BLEN
		30, // TCHTHR
		2, // TCHDI
		{0, 0} // RESERVED[0]~RESERVED[1]
	},
	{ // SPT_COMMSCONFIG_T18
		0, // CTRL
		0 // COMMAND
	},
	{ // SPT_GPIOPWM_T19
		0, // CTRL
		0, // REPORTMASK
		0, // DIR
		0, // INTPULLUP
		0, // OUT
		0, // WAKE
		0, // PWM
		0, // PERIOD
		{0, 0, 0, 0}, // DUTY[0]~DUTY[3]
		{0, 0, 0, 0} // TRIGGER[0]~TRIGGER[3]
	},
	{ // PROCI_GRIPFACESUPPRESSION_T20
		0, // CTRL
		0, // XLOGRIP
		0, // XHIGRIP
		0, // YLOGRIP
		0, // YHIGRIP
		0, // MAXTCHS
		0, // RESERVED[0]
		0, // SZTHR1
		0, // SZTHR2
		0, // SHPTHR1
		0, // SHPTHR2
		0 // SUPEXTTO
	},
	{ // PROCG_NOISESUPPRESSION_T22
		5, // CTRL
		0, // VIRTREFRNKG
		0, // RESERVED[0]
		0, // GCAFUL
		0, // GCAFLL
		3, // ACTVGCAFVALID
		20, // NOISETHR
		0, // RESERVED[1]
		0, // FREQHOPSCALE
		{10, 15, 20, 25, 30}, // FREQ[0]~FREQ[4]
		3 // IDLEGCAFVALID
	},
	{ // TOUCH_PROXIMITY_T23
		0, // CTRL
		0, // XORIGIN
		0, // YORIGIN
		0, // XSIZE
		0, // YSIZE
		0, // RESERVED[0]
		0, // BLEN
		0, // FXDDTHR
		0, // FXDDI
		0, // AVERAGE
		0, // MVNULLRATE
		0, // MVDTHR
	},
	{ // PROCI_ONETOUCHGESTUREPROCESSOR_T24
		0, // CTRL
		0, // NUMGEST
		0, // GESTEN
		0, // PROCESS
		0, // TAPTO
		0, // FLICKTO
		0, // DRAGTO
		0, // SPRESSTO
		0, // LPRESSTO
		0, // REPPRESSTO
		0, // FLICKTHR
		0, // DRAGTHR
		0, // TAPTHR
		0 // THROWTHR
	},
	{ // SPT_SELFTEST_T25
		0, // CTRL
		0, // CMD
		{
			{0, 0}, // SIGLIM[0]
			{0, 0}, // SIGLIM[1]
			{0, 0}, // SIGLIM[2]
		}
	},
	{ // SPT_CTECONFIG_T28
		0, // CTRL
		0, // CMD
		1, // MODE
		16, // IDLEGCAFDEPTH
		32, // ACTVGCAFDEPTH
		10 // VOLTAGE
	}
};
#endif

/*------------------------------ for tunning ATmel - end ----------------------------*/

#define __QT_CONFIG__
/*****************************************************************************
*
*
*
*
*
*       QT602240  Configuration Block
*
*
*
*
* ***************************************************************************/
// [LGE PATCH : START] edward1.kim@lge.com 20110216 
static int qt_ts_on(void)
{
	int ret = 0;

	ret = qt602240->power(ON);
	if(ret < 0)	{
		QT602240_ERROR("power on failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}

// [LGE PATCH : END] edward1.kim@lge.com 20110216  

/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Userdata_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	userdata38_config.data[0] = touch_config.t38.data[0];
	userdata38_config.data[1] = touch_config.t38.data[1];
	userdata38_config.data[2] = touch_config.t38.data[2];
	userdata38_config.data[3] = touch_config.t38.data[3];
	userdata38_config.data[4] = touch_config.t38.data[4];
	userdata38_config.data[5] = touch_config.t38.data[5];
	userdata38_config.data[6] = touch_config.t38.data[6];
	userdata38_config.data[7] = touch_config.t38.data[7];
#else
	userdata38_config.data[0] = 0;
	userdata38_config.data[1] = 0;
	userdata38_config.data[2] = 0;
	userdata38_config.data[3] = 0;
	userdata38_config.data[4] = 0;
	userdata38_config.data[5] = 0;
	userdata38_config.data[6] = 0;
	userdata38_config.data[7] = 0;
#endif	

	if(write_userdata_t38_config(userdata38_config) != CFG_WRITE_OK)
	{
		QT602240_ERROR("configuration fail\n");
	}
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Power_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	power_config.idleacqint = touch_config.t7.idleacqint;
	QT602240_TRACE("real board\n");
	if(qt602240->touch_key == 1)
	{
		power_config.actvacqint = touch_config.t7.actvacqint;
	}
	else
	{
		power_config.actvacqint = 12;
	}
	power_config.actv2idleto = touch_config.t7.actv2idleto;
#else
    /* Set Idle Acquisition Interval to 32 ms. */
	power_config.idleacqint = 64; // 255;

    /* Set Active Acquisition Interval to 16 ms. */
	QT602240_TRACE("real board\n");
	if(qt602240->touch_key == 1)
	{
		power_config.actvacqint = 12; // 11;
	}
	else
	{
		power_config.actvacqint = 12;
	}

    /* Set Active to Idle Timeout to 4 s (one unit = 200ms). */
    power_config.actv2idleto = 50; // 0;
#endif

    /* Write power config to chip. */
    if (write_power_config(power_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}




/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Acquisition_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	acquisition_config.chrgtime =  touch_config.t8.chrgtime;
	acquisition_config.reserved = touch_config.t8.reserved;
	acquisition_config.tchdrift = touch_config.t8.tchdrift;
	acquisition_config.driftst = touch_config.t8.driftst;
	acquisition_config.tchautocal = touch_config.t8.tchautocal;
	acquisition_config.sync = touch_config.t8.sync;
	acquisition_config.atchcalst = touch_config.t8.atchcalst;
	acquisition_config.atchcalsthr = touch_config.t8.atchcalsthr;
	if(qt602240->touch_key == 1)
	{
		acquisition_config.atchhfrccalthr = touch_config.t8.atchhfrccalthr;
		acquisition_config.atchfrccalratio = touch_config.t8.atchfrccalratio;
	}
	else
	{
		acquisition_config.atchhfrccalthr = 25;
		acquisition_config.atchfrccalratio = 0;
	}
#else
    acquisition_config.chrgtime =  10;
    acquisition_config.reserved = 0;

    acquisition_config.tchdrift = 5; // 20;
    acquisition_config.driftst = 5; // 20;

    acquisition_config.tchautocal = 0; // infinite
    acquisition_config.sync = 0; // disabled

    acquisition_config.atchcalst = 9;
    acquisition_config.atchcalsthr = 23; // 35;
    if(qt602240->touch_key == 1)
    {
	acquisition_config.atchhfrccalthr = 5;
	acquisition_config.atchfrccalratio = 2;
    }
    else
    {
      acquisition_config.atchhfrccalthr = 25;
      acquisition_config.atchfrccalratio = 0;
    }
#endif	

    if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Multitouchscreen_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	touchscreen_config.ctrl = touch_config.t9.ctrl;
	touchscreen_config.xorigin = touch_config.t9.xorigin;
	touchscreen_config.yorigin = touch_config.t9.yorigin;
	touchscreen_config.xsize = touch_config.t9.xsize;
	if(qt602240->touch_key == 1)
	{
		touchscreen_config.ysize = touch_config.t9.ysize;
	}
	else
	{
		touchscreen_config.ysize = 10;
	}
	touchscreen_config.akscfg = touch_config.t9.akscfg;
	if(qt602240->touch_key == 1)
	{
		touchscreen_config.blen = touch_config.t9.blen;
		touchscreen_config.tchthr = touch_config.t9.tchthr;
	}
	else
	{
		touchscreen_config.blen = 32;
		touchscreen_config.tchthr = 40;
	}
	touchscreen_config.tchdi = touch_config.t9.tchdi;
	touchscreen_config.orient = touch_config.t9.orient;
	touchscreen_config.mrgtimeout = touch_config.t9.mrgtimeout;
	touchscreen_config.movhysti = touch_config.t9.movhysti;
	touchscreen_config.movhystn = touch_config.t9.movhystn;
	touchscreen_config.movfilter = touch_config.t9.movfilter;
	if(qt602240->touch_key == 1)
	{
		touchscreen_config.numtouch = touch_config.t9.numtouch;
		touchscreen_config.mrghyst = touch_config.t9.mrghyst;
		touchscreen_config.mrgthr = touch_config.t9.mrgthr;
		touchscreen_config.amphyst = touch_config.t9.amphyst;
	}
	else
	{
		touchscreen_config.numtouch= MAX_USING_FINGER_NUM;
		touchscreen_config.mrghyst = 3;
		touchscreen_config.mrgthr = 26;
		touchscreen_config.amphyst = 6;
	}
	touchscreen_config.xrange = touch_config.t9.xrange;
	touchscreen_config.yrange = touch_config.t9.yrange;
	if(qt602240->touch_key == 1)
	{
		touchscreen_config.xloclip = touch_config.t9.xloclip;
		touchscreen_config.xhiclip = touch_config.t9.xhiclip;
		touchscreen_config.yloclip = touch_config.t9.yloclip;
		touchscreen_config.yhiclip = touch_config.t9.yhiclip;
		touchscreen_config.xedgectrl = touch_config.t9.xedgectrl;
		touchscreen_config.xedgedist = touch_config.t9.xedgedist;
		touchscreen_config.yedgectrl = touch_config.t9.yedgectrl;
		touchscreen_config.yedgedist = touch_config.t9.yedgedist;
	}
	else
	{
		touchscreen_config.xloclip = 10;
		touchscreen_config.xhiclip = 10;
		touchscreen_config.yloclip = 24;
		touchscreen_config.yhiclip = 24;
		touchscreen_config.xedgectrl = 136;
		touchscreen_config.xedgedist = 90;
		touchscreen_config.yedgectrl = 136;
		touchscreen_config.yedgedist = 90;
	}
	touchscreen_config.jumplimit = touch_config.t9.jumplimit;
	touchscreen_config.tchhyst = touch_config.t9.tchhyst;
#else
#ifdef _SUPPORT_TOUCH_AMPLITUDE_
    touchscreen_config.ctrl = 0x8B; // enable amplitude
#else
    touchscreen_config.ctrl = 143; // enable + message-enable
#endif
    touchscreen_config.xorigin = 0;
    touchscreen_config.yorigin = 2;

    touchscreen_config.xsize = 14;//17;

    if(qt602240->touch_key == 1)
    {
      touchscreen_config.ysize = 9;
    }
    else
    {
      touchscreen_config.ysize = 10;//11;
    }
    

    touchscreen_config.akscfg = 0;
    if(qt602240->touch_key == 1)
    {
	touchscreen_config.blen = 0; // 16;
	touchscreen_config.tchthr = 30; // 40;
	touchscreen_config.numtouch= 8; // 5;

	touchscreen_config.mrghyst = 3;
	touchscreen_config.mrgthr = 26;

	touchscreen_config.amphyst = 6;

	touchscreen_config.xloclip = 10;
	touchscreen_config.xhiclip = 10;
	touchscreen_config.yloclip = 35;
	touchscreen_config.yhiclip = 35;

	touchscreen_config.xedgectrl = 136;
	touchscreen_config.xedgedist = 90;
	touchscreen_config.yedgectrl = 136;
	touchscreen_config.yedgedist = 90;
    }
    else
    {
      touchscreen_config.blen = 32; //0x21
      touchscreen_config.tchthr = 40;//45;

     
#ifdef _SUPPORT_MULTITOUCH_	
		 touchscreen_config.numtouch= MAX_USING_FINGER_NUM;  // it is 5 now and it can be set up to 10
#else
		 touchscreen_config.numtouch= 1;
#endif
    touchscreen_config.mrghyst = 3;//5;
    touchscreen_config.mrgthr = 26;//40; //20;	//5;
    touchscreen_config.amphyst = 6; //10;
    
    touchscreen_config.xloclip = 10;
    touchscreen_config.xhiclip = 10;
    touchscreen_config.yloclip = 24;
    touchscreen_config.yhiclip = 24;

    touchscreen_config.xedgectrl = 136; //143;
    touchscreen_config.xedgedist = 90;
    touchscreen_config.yedgectrl = 136; //143;
    touchscreen_config.yedgedist = 90;
    }

	touchscreen_config.orient = 1;
	touchscreen_config.movfilter = 0;

    touchscreen_config.tchdi = 2;

    touchscreen_config.mrgtimeout = 0;
    touchscreen_config.movhysti = 3;	// 6;

	
    touchscreen_config.movhystn = 1;	// 5; ///1;
  

#ifdef LG_FW_TOUCH_SOFT_KEY
    touchscreen_config.xrange = qt602240_pdata->ts_y_scrn_max+120-1;
#else
    touchscreen_config.xrange = qt602240_pdata->ts_y_scrn_max-1;
#endif
    touchscreen_config.yrange = qt602240_pdata->ts_x_max-1;

	touchscreen_config.jumplimit = 15; // 0; //18;
#endif	
	
    if (write_multitouchscreen_config(0, touchscreen_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}

#ifdef QT_STYLUS_ENABLE
void qt_Multitouchscreen_stylus_Init(void)
{

    touchscreen_config.tchthr = 25;//45;	
    touchscreen_config.movhysti = 1;
	
//    touchscreen_config.movhystn = 5;///1;
    touchscreen_config.movfilter = 79;//0
    touchscreen_config.numtouch= 1;

	
    if (write_multitouchscreen_config(0, touchscreen_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}

void qt_Multitouchscreen_normal_Init(void)
{

    touchscreen_config.tchthr = 40;//45;	
    
    touchscreen_config.movhysti = 3;
//    touchscreen_config.movhystn = 6;///1;
    touchscreen_config.movfilter = 0;//0x2e;//0
    touchscreen_config.numtouch= MAX_USING_FINGER_NUM;

	
    if (write_multitouchscreen_config(0, touchscreen_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}
#endif

/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_KeyArray_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	if(qt602240->touch_key == 1)
	{
		keyarray_config.ctrl = touch_config.t15.ctrl;
		keyarray_config.xorigin = touch_config.t15.xorigin;
		keyarray_config.yorigin = touch_config.t15.yorigin;
		keyarray_config.xsize = touch_config.t15.xsize;
		keyarray_config.ysize = touch_config.t15.ysize;
		keyarray_config.akscfg = touch_config.t15.akscfg;
		keyarray_config.blen = touch_config.t15.blen;
		keyarray_config.tchthr = touch_config.t15.tchthr;
		keyarray_config.tchdi = touch_config.t15.tchdi;
		keyarray_config.reserved[0] = touch_config.t15.reserved[0];
		keyarray_config.reserved[1] = touch_config.t15.reserved[1];
	}
	else
	{
		keyarray_config.ctrl = 0;
		keyarray_config.xorigin = 0;
		keyarray_config.yorigin = 0;
		keyarray_config.xsize = 0;
		keyarray_config.ysize = 0;
		keyarray_config.akscfg = 0;
		keyarray_config.blen = 0;
		keyarray_config.tchthr = 0;
		keyarray_config.tchdi = 0;
		keyarray_config.reserved[0] = 0;
		keyarray_config.reserved[1] = 0;
	}
#else
	if(qt602240->touch_key == 1)
	{
		keyarray_config.ctrl = 131;
		keyarray_config.xorigin = 11;
		keyarray_config.yorigin =11;
		keyarray_config.xsize = 3;
		keyarray_config.ysize = 1;
		keyarray_config.akscfg = 0;
		keyarray_config.blen = 0; // 16;
		keyarray_config.tchthr = 30; // 40;
		keyarray_config.tchdi = 2;
		keyarray_config.reserved[0] = 0;
		keyarray_config.reserved[1] = 0;
	}
	else
	{
		keyarray_config.ctrl = 0;
		keyarray_config.xorigin = 0;
		keyarray_config.yorigin = 0;
		keyarray_config.xsize = 0;
		keyarray_config.ysize = 0;
		keyarray_config.akscfg = 0;
		keyarray_config.blen = 0;
		keyarray_config.tchthr = 0;
		keyarray_config.tchdi = 0;
		keyarray_config.reserved[0] = 0;
		keyarray_config.reserved[1] = 0;
	}
#endif

    if (write_keyarray_config(0, keyarray_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_ComcConfig_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	comc_config.ctrl = touch_config.t18.ctrl;
	comc_config.cmd = touch_config.t18.cmd;
#else
    comc_config.ctrl = 0x00;
    comc_config.cmd = NO_COMMAND;//COMM_MODE1;
#endif

    if (get_object_address(SPT_COMCONFIG_T18, 0) != OBJECT_NOT_FOUND)
    {
        if (write_comc_config(0, comc_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Gpio_Pwm_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	gpiopwm_config.ctrl =touch_config.t19.ctrl;
	gpiopwm_config.reportmask = touch_config.t19.reportmask;
	gpiopwm_config.dir = touch_config.t19.dir;
	gpiopwm_config.intpullup = touch_config.t19.intpullup;
	gpiopwm_config.out = touch_config.t19.out;
	gpiopwm_config.wake = touch_config.t19.wake;
	gpiopwm_config.pwm = touch_config.t19.pwm;
	gpiopwm_config.period = touch_config.t19.period;
	gpiopwm_config.duty[0] = touch_config.t19.duty[0];
	gpiopwm_config.duty[1] = touch_config.t19.duty[1];
	gpiopwm_config.duty[2] = touch_config.t19.duty[2];
	gpiopwm_config.duty[3] = touch_config.t19.duty[3];
	gpiopwm_config.trigger[0] = touch_config.t19.trigger[0];
	gpiopwm_config.trigger[1] = touch_config.t19.trigger[1];
	gpiopwm_config.trigger[2] = touch_config.t19.trigger[2];
	gpiopwm_config.trigger[3] = touch_config.t19.trigger[3];
#else
    gpiopwm_config.ctrl = 0;
    gpiopwm_config.reportmask = 0;
    gpiopwm_config.dir = 0;
    gpiopwm_config.intpullup = 0;
    gpiopwm_config.out = 0;
    gpiopwm_config.wake = 0;
    gpiopwm_config.pwm = 0;
    gpiopwm_config.period = 0;
    gpiopwm_config.duty[0] = 0;
    gpiopwm_config.duty[1] = 0;
    gpiopwm_config.duty[2] = 0;
    gpiopwm_config.duty[3] = 0;
    gpiopwm_config.trigger[0] = 0;
    gpiopwm_config.trigger[1] = 0;
    gpiopwm_config.trigger[2] = 0;
    gpiopwm_config.trigger[3] = 0;
#endif
	
    if (write_gpio_config(0, gpiopwm_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Grip_Face_Suppression_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	gripfacesuppression_config.ctrl = touch_config.t20.ctrl;
	gripfacesuppression_config.xlogrip = touch_config.t20.xlogrip;
	gripfacesuppression_config.xhigrip = touch_config.t20.xhigrip;
	gripfacesuppression_config.ylogrip = touch_config.t20.ylogrip;
	gripfacesuppression_config.yhigrip = touch_config.t20.yhigrip;
	gripfacesuppression_config.maxtchs = touch_config.t20.maxtchs;
	gripfacesuppression_config.reserved = touch_config.t20.reserved;
	gripfacesuppression_config.szthr1 = touch_config.t20.szthr1;
	gripfacesuppression_config.szthr2 = touch_config.t20.szthr2;
	gripfacesuppression_config.shpthr1 = touch_config.t20.shpthr1;
	gripfacesuppression_config.shpthr2 = touch_config.t20.shpthr2;
	gripfacesuppression_config.supextto = touch_config.t20.supextto;
#else
    gripfacesuppression_config.ctrl = 0; //-> disable PALM bit
    gripfacesuppression_config.xlogrip = 0;
    gripfacesuppression_config.xhigrip = 0;
    gripfacesuppression_config.ylogrip = 0;
    gripfacesuppression_config.yhigrip = 0;
    gripfacesuppression_config.maxtchs = 0;
    gripfacesuppression_config.reserved = 0;
    gripfacesuppression_config.szthr1 = 0;
    gripfacesuppression_config.szthr2 = 0;
    gripfacesuppression_config.shpthr1 = 0;
    gripfacesuppression_config.shpthr2 = 0;

    gripfacesuppression_config.supextto = 0;
#endif

    /* Write grip suppression config to chip. */
    if (get_object_address(PROCI_GRIPFACESUPPRESSION_T20, 0) != OBJECT_NOT_FOUND)
    {
        if (write_gripsuppression_config(0, gripfacesuppression_config) !=
            CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Noise_Suppression_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	noise_suppression_config.ctrl = touch_config.t22.ctrl;
	noise_suppression_config.virtrefrnkg = touch_config.t22.virtrefrnkg;
	noise_suppression_config.reserved1 = touch_config.t22.reserved1;
	noise_suppression_config.gcaful = touch_config.t22.gcaful;
	noise_suppression_config.gcafll = touch_config.t22.gcafll;
	noise_suppression_config.actvgcafvalid = touch_config.t22.actvgcafvalid;
	if(qt602240->touch_key == 1)
	{
		noise_suppression_config.noisethr = touch_config.t22.noisethr;
	}
	else
	{
		noise_suppression_config.noisethr = 30; 
	}
	noise_suppression_config.reserved2 = touch_config.t22.reserved2;
	noise_suppression_config.freqhopscale = touch_config.t22.freqhopscale;
	if(qt602240->touch_key == 1)
	{
		noise_suppression_config.freq[0] = touch_config.t22.freq[0];
		noise_suppression_config.freq[1] = touch_config.t22.freq[1];
		noise_suppression_config.freq[2] = touch_config.t22.freq[2];
		noise_suppression_config.freq[3] = touch_config.t22.freq[3];
		noise_suppression_config.freq[4] = touch_config.t22.freq[4];
	}
	else
	{
		noise_suppression_config.freq[0] = 10;
		noise_suppression_config.freq[1] = 15;
		noise_suppression_config.freq[2] = 20;
		noise_suppression_config.freq[3] = 25;
		noise_suppression_config.freq[4] = 30;
	}
	noise_suppression_config.idlegcafvalid = touch_config.t22.idlegcafvalid;
#else
    noise_suppression_config.ctrl = 5; // 7;

    noise_suppression_config.virtrefrnkg = 0;

    noise_suppression_config.reserved1 = 0;
    noise_suppression_config.gcaful = 0;
    noise_suppression_config.gcafll = 0;

    noise_suppression_config.actvgcafvalid = 3; // 0;

   if(qt602240->touch_key == 1)
   {
	noise_suppression_config.noisethr = 20;
	noise_suppression_config.freq[0] = 10; // 5;
	noise_suppression_config.freq[1] = 15; // 10;
	noise_suppression_config.freq[2] = 20; // 15;
	noise_suppression_config.freq[3] = 25; // 20;
	noise_suppression_config.freq[4] = 30; // 25;
   }
   else
   {
   	noise_suppression_config.noisethr = 30; 
	noise_suppression_config.freq[0] = 10;
	noise_suppression_config.freq[1] = 15;
	noise_suppression_config.freq[2] = 20;
	noise_suppression_config.freq[3] = 25;
	noise_suppression_config.freq[4] = 30;
   }
	
	noise_suppression_config.reserved2 = 0;
	noise_suppression_config.freqhopscale = 0;
    
    noise_suppression_config.idlegcafvalid = 3; // 0;
#endif

    /* Write Noise suppression config to chip. */
    if (get_object_address(PROCG_NOISESUPPRESSION_T22, 0) != OBJECT_NOT_FOUND)
    {
        if (write_noisesuppression_config(0,noise_suppression_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Proximity_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	proximity_config.ctrl = touch_config.t23.ctrl;
	proximity_config.xorigin = touch_config.t23.xorigin;
	proximity_config.yorigin = touch_config.t23.yorigin;
	proximity_config.xsize = touch_config.t23.xsize;
	proximity_config.ysize = touch_config.t23.ysize;
	proximity_config.reserved = touch_config.t23.reserved;
	proximity_config.blen = touch_config.t23.blen;
	proximity_config.fxddthr = touch_config.t23.fxddthr;
	proximity_config.fxddi = touch_config.t23.fxddi;
	proximity_config.average = touch_config.t23.average;
	proximity_config.mvnullrate = touch_config.t23.mvnullrate;
	proximity_config.mvdthr = touch_config.t23.mvdthr;
#else
    proximity_config.ctrl = 0;
    proximity_config.xorigin = 0;
    proximity_config.yorigin = 0;	
    proximity_config.xsize = 0;
    proximity_config.ysize = 0;
    proximity_config.reserved_for_future_aks_usage = 0;
    proximity_config.blen = 0;
    proximity_config.tchthr = 0;
    proximity_config.tchdi = 0;
    proximity_config.average = 0;
    proximity_config.rate = 0;
#endif

    if (get_object_address(TOUCH_PROXIMITY_T23, 0) != OBJECT_NOT_FOUND)
    {
        if (write_proximity_config(0, proximity_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_One_Touch_Gesture_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	onetouch_gesture_config.ctrl = touch_config.t24.ctrl;
	onetouch_gesture_config.numgest = touch_config.t24.numgest;
	onetouch_gesture_config.gesten = touch_config.t24.gesten;
	onetouch_gesture_config.pressproc = touch_config.t24.pressproc;
	onetouch_gesture_config.tapto = touch_config.t24.tapto;
	onetouch_gesture_config.flickto = touch_config.t24.flickto;
	onetouch_gesture_config.dragto = touch_config.t24.dragto;
	onetouch_gesture_config.spressto = touch_config.t24.spressto;
	onetouch_gesture_config.lpressto = touch_config.t24.lpressto;
	onetouch_gesture_config.reppressto = touch_config.t24.reppressto;
	onetouch_gesture_config.flickthr = touch_config.t24.flickthr;
	onetouch_gesture_config.dragthr = touch_config.t24.dragthr;
	onetouch_gesture_config.tapthr = touch_config.t24.tapthr;
	onetouch_gesture_config.throwthr = touch_config.t24.throwthr;
#else
    /* Disable one touch gestures. */
    onetouch_gesture_config.ctrl = 0;
    onetouch_gesture_config.numgest = 0;

    onetouch_gesture_config.gesten = 0;
    onetouch_gesture_config.pressproc = 0;
    onetouch_gesture_config.tapto = 0;
    onetouch_gesture_config.flickto = 0;
    onetouch_gesture_config.dragto = 0;
    onetouch_gesture_config.spressto = 0;
    onetouch_gesture_config.lpressto = 0;
    onetouch_gesture_config.reppressto = 0;
    onetouch_gesture_config.flickthr = 0;
    onetouch_gesture_config.dragthr = 0;
    onetouch_gesture_config.tapthr = 0;
    onetouch_gesture_config.throwthr = 0;
#endif

    if (get_object_address(PROCI_ONETOUCHGESTUREPROCESSOR_T24, 0) !=
        OBJECT_NOT_FOUND)
    {
        if (write_onetouchgesture_config(0, onetouch_gesture_config) !=
            CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Selftest_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	 selftest_config.ctrl = touch_config.t25.ctrl;
	selftest_config.cmd = touch_config.t25.cmd;
	selftest_config.siglim[0].upsiglim = touch_config.t25.siglim[0].upsiglim;
	selftest_config.siglim[0].losiglim = touch_config.t25.siglim[0].losiglim;
	selftest_config.siglim[1].upsiglim = touch_config.t25.siglim[1].upsiglim;
	selftest_config.siglim[1].losiglim = touch_config.t25.siglim[1].losiglim;
	selftest_config.siglim[2].upsiglim = touch_config.t25.siglim[2].upsiglim;
	selftest_config.siglim[2].losiglim = touch_config.t25.siglim[2].losiglim;
#else
    selftest_config.ctrl = 0;
    selftest_config.cmd = 0;

#if(NUM_OF_TOUCH_OBJECTS)
    siglim.upsiglim[0] = 0;
    siglim.losiglim[0] = 0;
#endif
#endif

    if (get_object_address(SPT_SELFTEST_T25, 0) != OBJECT_NOT_FOUND)
    {
        if (write_selftest_config(0,selftest_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_Two_touch_Gesture_Config_Init(void)
{
    /* Disable two touch gestures. */
    twotouch_gesture_config.ctrl = 0;
    twotouch_gesture_config.numgest = 0;

    twotouch_gesture_config.reserved2 = 0;
    twotouch_gesture_config.gesten = 0;
    twotouch_gesture_config.rotatethr = 0;
    twotouch_gesture_config.zoomthr = 0;


    if (get_object_address(PROCI_TWOTOUCHGESTUREPROCESSOR_T27, 0) !=
        OBJECT_NOT_FOUND)
    {
        if (write_twotouchgesture_config(0, twotouch_gesture_config) !=
            CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void qt_CTE_Config_Init(void)
{
#if defined(USE_CONFIG_TABLE)
	cte_config.ctrl = touch_config.t28.ctrl;
	cte_config.cmd = touch_config.t28.cmd;
	cte_config.mode = touch_config.t28.mode;
	if(qt602240->touch_key == 1)
	{
		cte_config.idlegcafdepth = touch_config.t28.idlegcafdepth;
		cte_config.actvgcafdepth = touch_config.t28.actvgcafdepth;
	}
	else
	{
		cte_config.idlegcafdepth = 16;
		cte_config.actvgcafdepth = 32;
	}
	cte_config.voltage = touch_config.t28.voltage;
#else
    /* Set CTE config */
    cte_config.ctrl = 0;
    cte_config.cmd = 0;	
    cte_config.mode = 1;
	
	if(qt602240->touch_key == 1)
	{
		cte_config.idlegcafdepth = 16; // 32;
		cte_config.actvgcafdepth = 32; // 63;
	}
	else
	{
		cte_config.idlegcafdepth = 16;
		cte_config.actvgcafdepth = 32;
	};

    cte_config.voltage = 10; // 30;
#endif

    /* Write CTE config to chip. */
    if (get_object_address(SPT_CTECONFIG_T28, 0) != OBJECT_NOT_FOUND)
    {
        if (write_CTE_config(cte_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }
}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

unsigned char Comm_Config_Process(unsigned char change_en)
{
#if 0
    if(change_en == 1)
    {
        change_en = 0;

        if (get_object_address(SPT_COMCONFIG_T18, 0) != OBJECT_NOT_FOUND)
        {
            if((comc_config.cmd == COMM_MODE1))
            {
                if(read_changeline()== CHANGELINE_NEGATED)
                {
                    return (change_en);
                }

            }
            if (write_comc_config(0, comc_config) != CFG_WRITE_OK)
            {
                return (change_en);
            }
        }
    }
#else
	change_en = 0;
#endif
    return (change_en);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t reset_chip(void)
{
    uint8_t data = 1u;

	QT602240_TRACE("start\n");
   cal_check_flag = 1u; //20100309s
    return(write_mem(command_processor_address + RESET_OFFSET, 1, &data));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t calibrate_chip(void)
{
    uint8_t data = 1u;
#if 1
	int ret = WRITE_MEM_OK;
	uint8_t atchcalst, atchcalsthr;
	
	if((tsp_version>=0x16)&&(cal_check_flag == 0))
	{     
		/* change calibration suspend settings to zero until calibration confirmed good */
		/* store normal settings */
		atchcalst = acquisition_config.atchcalst;
		atchcalsthr = acquisition_config.atchcalsthr;

		/* resume calibration must be performed with zero settings */
		acquisition_config.atchcalst = 0;
		acquisition_config.atchcalsthr = 0; 

		  QT602240_TRACE("start\n");
		QT602240_TRACE("reset acq atchcalst: %d, atchcalsthr: %d\n", acquisition_config.atchcalst, acquisition_config.atchcalsthr);

		/* Write temporary acquisition config to chip. */
		if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
		{
		/* "Acquisition config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
		}

		/* restore settings to the local structure so that when we confirm the 
		* cal is good we can correct them in the chip */
		/* this must be done before returning */
		acquisition_config.atchcalst = atchcalst;
		acquisition_config.atchcalsthr = atchcalsthr;
	}        

	/* send calibration command to the chip */
	if(ret == WRITE_MEM_OK)
	{
        /* change calibration suspend settings to zero until calibration confirmed good */
        	ret = write_mem(command_processor_address + CALIBRATE_OFFSET, 1, &data);
        
	        /* set flag for calibration lockup recovery if cal command was successful */
	        if(ret == WRITE_MEM_OK)
	        { 
		        /* set flag to show we must still confirm if calibration was good or bad */
		        cal_check_flag = 1u;
	        }
	}
	return ret;


#else
    cal_check_flag = 1u; //20100217 julia
    
    return(write_mem(command_processor_address + CALIBRATE_OFFSET, 1, &data));
#endif
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t diagnostic_chip(uint8_t mode)
{
    uint8_t status;
    status = write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &mode);
    return(status);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t backup_config(void)
{

    /* Write 0x55 to BACKUPNV register to initiate the backup. */
    uint8_t data = 0x55u;
    return(write_mem(command_processor_address + BACKUP_OFFSET, 1, &data));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_version(uint8_t *version)
{

    if (info_block)
    {
        *version = info_block->info_id.version;
    }
    else
    {
        return(ID_DATA_NOT_AVAILABLE);
    }
    return (ID_DATA_OK);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_family_id(uint8_t *family_id)
{

    if (info_block)
    {
        *family_id = info_block->info_id.family_id;
    }
    else
    {
        return(ID_DATA_NOT_AVAILABLE);
    }
    return (ID_DATA_OK);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_build_number(uint8_t *build)
{

    if (info_block)
    {
        *build = info_block->info_id.build;
    }
    else
    {
        return(ID_DATA_NOT_AVAILABLE);
    }
    return (ID_DATA_OK);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_variant_id(uint8_t *variant)
{

    if (info_block)
    {
        *variant = info_block->info_id.variant_id;
    }
    else
    {
        return(ID_DATA_NOT_AVAILABLE);
    }
    return (ID_DATA_OK);
}

/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_command_config(gen_commandprocessor_t6_config_t cfg)
{
    return(write_simple_config(GEN_COMMANDPROCESSOR_T6, 0, (void *) &cfg));
}

/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_userdata_t38_config(spt_userdata_t38_config_t cfg)
{
    return(write_simple_config(SPARE_T38, 0, (void *) &cfg));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_power_config(gen_powerconfig_t7_config_t cfg)
{
    return(write_simple_config(GEN_POWERCONFIG_T7, 0, (void *) &cfg));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_acquisition_config(gen_acquisitionconfig_t8_config_t cfg)
{
    return(write_simple_config(GEN_ACQUISITIONCONFIG_T8, 0, (void *) &cfg));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_multitouchscreen_config(uint8_t instance, touch_multitouchscreen_t9_config_t cfg)
{
    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(TOUCH_MULTITOUCHSCREEN_T9);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);


    /* 18 elements at beginning are 1 byte. */
    memcpy(tmp, &cfg, 18);

    /* Next two are 2 bytes. */

    *(tmp + 18) = (uint8_t) (cfg.xrange &  0xFF);
    *(tmp + 19) = (uint8_t) (cfg.xrange >> 8);

    *(tmp + 20) = (uint8_t) (cfg.yrange &  0xFF);
    *(tmp + 21) = (uint8_t) (cfg.yrange >> 8);

    /* And the last 4(8) 1 bytes each again. */

    *(tmp + 22) = cfg.xloclip;
    *(tmp + 23) = cfg.xhiclip;
    *(tmp + 24) = cfg.yloclip;
    *(tmp + 25) = cfg.yhiclip;

#if defined(__VER_1_4__)
    *(tmp + 26) = cfg.xedgectrl;
    *(tmp + 27) = cfg.xedgedist;
    *(tmp + 28) = cfg.yedgectrl;
    *(tmp + 29) = cfg.yedgedist;
#endif
    	*(tmp + 30) = cfg.jumplimit;
    *(tmp + 31) = cfg.tchhyst;	
    object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9,
        instance);

    if (object_address == 0)
    {
        kfree(tmp);
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_keyarray_config(uint8_t instance, touch_keyarray_t15_config_t cfg)
{

    return(write_simple_config(TOUCH_KEYARRAY_T15, instance, (void *) &cfg));

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_linearization_config(uint8_t instance, proci_linearizationtable_t17_config_t cfg)
{

    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(PROCI_LINEARIZATIONTABLE_T17);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);

    *(tmp + 0) = cfg.ctrl;
    *(tmp + 1) = (uint8_t) (cfg.xoffset & 0x00FF);
    *(tmp + 2) = (uint8_t) (cfg.xoffset >> 8);

    memcpy((tmp+3), &cfg.xsegment, 16);

    *(tmp + 19) = (uint8_t) (cfg.yoffset & 0x00FF);
    *(tmp + 20) = (uint8_t) (cfg.yoffset >> 8);

    memcpy((tmp+21), &cfg.ysegment, 16);

    object_address = get_object_address(PROCI_LINEARIZATIONTABLE_T17,
        instance);

    if (object_address == 0)
    {
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);

}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_comc_config(uint8_t instance, spt_comcconfig_t18_config_t cfg)
{

    return(write_simple_config(SPT_COMCONFIG_T18, instance, (void *) &cfg));

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_gpio_config(uint8_t instance, spt_gpiopwm_t19_config_t cfg)
{


    return(write_simple_config(SPT_GPIOPWM_T19, instance, (void *) &cfg));

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_gripsuppression_config(uint8_t instance, proci_gripfacesuppression_t20_config_t cfg)
{

    return(write_simple_config(PROCI_GRIPFACESUPPRESSION_T20, instance,
        (void *) &cfg));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_noisesuppression_config(uint8_t instance, procg_noisesuppression_t22_config_t cfg)
{
    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(PROCG_NOISESUPPRESSION_T22);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }
    memset(tmp,0,object_size);


    /* 18 elements at beginning are 1 byte. */
    memcpy(tmp, &cfg, 3);

    /* Next two are 2 bytes. */

    *(tmp + 3) = (uint8_t) (cfg.gcaful&  0xFF);
    *(tmp + 4) = (uint8_t) (cfg.gcaful >> 8);

    *(tmp + 5) = (uint8_t) (cfg.gcafll&  0xFF);
    *(tmp + 6) = (uint8_t) (cfg.gcafll >> 8);

     memcpy((tmp+7), &cfg.actvgcafvalid, 10);
	
    object_address = get_object_address(PROCG_NOISESUPPRESSION_T22, instance);

    if (object_address == 0)
    {
        kfree(tmp);
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);


//    return(write_simple_config(PROCG_NOISESUPPRESSION_T22, instance, (void *) &cfg));

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_proximity_config(uint8_t instance, touch_proximity_t23_config_t cfg)
{
    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(TOUCH_PROXIMITY_T23);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);

    *(tmp + 0) = cfg.ctrl;
    *(tmp + 1) = cfg.xorigin;
    *(tmp + 2) = cfg.yorigin;
    *(tmp + 3) = cfg.xsize;
    *(tmp + 4) = cfg.ysize;
    *(tmp + 5) = cfg.reserved;
    *(tmp + 6) = cfg.blen;

    *(tmp + 7) = (uint8_t) (cfg.fxddthr & 0x00FF);
    *(tmp + 8) = (uint8_t) (cfg.fxddthr >> 8);

    *(tmp + 9) = cfg.fxddi;
    *(tmp + 10) = cfg.average;

    *(tmp + 11) = (uint8_t) (cfg.mvnullrate & 0x00FF);
    *(tmp + 12) = (uint8_t) (cfg.mvnullrate >> 8);

    *(tmp + 13) = (uint8_t) (cfg.mvdthr & 0x00FF);
    *(tmp + 14) = (uint8_t) (cfg.mvdthr >> 8);

    object_address = get_object_address(TOUCH_PROXIMITY_T23,
        instance);

    if (object_address == 0)
    {
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_onetouchgesture_config(uint8_t instance, proci_onetouchgestureprocessor_t24_config_t cfg)
{

    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(PROCI_ONETOUCHGESTUREPROCESSOR_T24);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);
    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);

    *(tmp + 0) = cfg.ctrl;
#if defined(__VER_1_2__)
    *(tmp + 1) = 0;
#else
    *(tmp + 1) = cfg.numgest;
#endif

    *(tmp + 2) = (uint8_t) (cfg.gesten & 0x00FF);
    *(tmp + 3) = (uint8_t) (cfg.gesten >> 8);

    memcpy((tmp+4), &cfg.pressproc, 7);

    *(tmp + 11) = (uint8_t) (cfg.flickthr & 0x00FF);
    *(tmp + 12) = (uint8_t) (cfg.flickthr >> 8);

    *(tmp + 13) = (uint8_t) (cfg.dragthr & 0x00FF);
    *(tmp + 14) = (uint8_t) (cfg.dragthr >> 8);

    *(tmp + 15) = (uint8_t) (cfg.tapthr & 0x00FF);
    *(tmp + 16) = (uint8_t) (cfg.tapthr >> 8);

    *(tmp + 17) = (uint8_t) (cfg.throwthr & 0x00FF);
    *(tmp + 18) = (uint8_t) (cfg.throwthr >> 8);

    object_address = get_object_address(PROCI_ONETOUCHGESTUREPROCESSOR_T24,
        instance);

    if (object_address == 0)
    {
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_selftest_config(uint8_t instance, spt_selftest_t25_config_t cfg)
{

    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(SPT_SELFTEST_T25);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);


    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);

    *(tmp + 0) = cfg.ctrl;
    *(tmp + 1) = cfg.cmd;

    *(tmp + 2) = (uint8_t) (cfg.siglim[0].upsiglim& 0x00FF);
    *(tmp + 3) = (uint8_t) (cfg.siglim[0].upsiglim>> 8);
    *(tmp + 4) = (uint8_t) (cfg.siglim[0].losiglim& 0x00FF);
    *(tmp + 5) = (uint8_t) (cfg.siglim[0].losiglim>> 8);

    *(tmp + 6) = (uint8_t) (cfg.siglim[1].upsiglim& 0x00FF);
    *(tmp + 7) = (uint8_t) (cfg.siglim[1].upsiglim>> 8);
    *(tmp + 8) = (uint8_t) (cfg.siglim[1].losiglim& 0x00FF);
    *(tmp + 9) = (uint8_t) (cfg.siglim[1].losiglim>> 8);	

    *(tmp + 10) = (uint8_t) (cfg.siglim[2].upsiglim& 0x00FF);
    *(tmp + 11) = (uint8_t) (cfg.siglim[2].upsiglim>> 8);
    *(tmp + 12) = (uint8_t) (cfg.siglim[2].losiglim& 0x00FF);
    *(tmp + 13) = (uint8_t) (cfg.siglim[2].losiglim>> 8);

    object_address = get_object_address(SPT_SELFTEST_T25,
        instance);

    if (object_address == 0)
    {
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);
    return(status);
}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_twotouchgesture_config(uint8_t instance, proci_twotouchgestureprocessor_t27_config_t cfg)
{

    uint16_t object_address;
    uint8_t *tmp;
    uint8_t status;
    uint8_t object_size;

    object_size = get_object_size(PROCI_TWOTOUCHGESTUREPROCESSOR_T27);
    if (object_size == 0)
    {
        return(CFG_WRITE_FAILED);
    }
    tmp = (uint8_t *) kmalloc(object_size, GFP_KERNEL | GFP_ATOMIC);

    if (tmp == NULL)
    {
        return(CFG_WRITE_FAILED);
    }

    memset(tmp,0,object_size);

    *(tmp + 0) = cfg.ctrl;

#if defined(__VER_1_2__)
    *(tmp + 1) = 0;
#else
    *(tmp + 1) = cfg.numgest;
#endif

    *(tmp + 2) = 0;

    *(tmp + 3) = cfg.gesten;

    *(tmp + 4) = cfg.rotatethr;

    *(tmp + 5) = (uint8_t) (cfg.zoomthr & 0x00FF);
    *(tmp + 6) = (uint8_t) (cfg.zoomthr >> 8);

    object_address = get_object_address(PROCI_TWOTOUCHGESTUREPROCESSOR_T27,
        instance);

    if (object_address == 0)
    {
        return(CFG_WRITE_FAILED);
    }

    status = write_mem(object_address, object_size, tmp);

    kfree(tmp);

    return(status);

}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_CTE_config(spt_cteconfig_t28_config_t cfg)
{

    return(write_simple_config(SPT_CTECONFIG_T28, 0, (void *) &cfg));
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t write_simple_config(uint8_t object_type, uint8_t instance, void *cfg)
{
    uint16_t object_address;
    uint8_t object_size;

    object_address = get_object_address(object_type, instance);
    object_size = get_object_size(object_type);

    if ((object_size == 0) || (object_address == 0))
    {
        return(CFG_WRITE_FAILED);
    }

    return (write_mem(object_address, object_size, cfg));
}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_object_size(uint8_t object_type)
{
    uint8_t object_table_index = 0;
    uint8_t object_found = 0;
    uint16_t size = OBJECT_NOT_FOUND;

    object_t *object_table;
    object_t obj;
    object_table = info_block->objects;
    while ((object_table_index < info_block->info_id.num_declared_objects) &&
        !object_found)
    {
        obj = object_table[object_table_index];
        /* Does object type match? */
        if (obj.object_type == object_type)
        {
            object_found = 1;
            size = obj.size + 1;
        }
        object_table_index++;
    }

    return(size);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t type_to_report_id(uint8_t object_type, uint8_t instance)
{

    uint8_t report_id = 1;
    int8_t report_id_found = 0;

    while((report_id <= max_report_id) && (report_id_found == 0))
    {
        if((report_id_map[report_id].object_type == object_type) &&
            (report_id_map[report_id].instance == instance))
        {
            report_id_found = 1;
        }
        else
        {
            report_id++;
        }
    }
    if (report_id_found)
    {
        return(report_id);
    }
    else
    {
        return(ID_MAPPING_FAILED);
    }
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t report_id_to_type(uint8_t report_id, uint8_t *instance)
{

    if (report_id <= max_report_id)
    {
        *instance = report_id_map[report_id].instance;
        return(report_id_map[report_id].object_type);
    }
    else
    {
        return(ID_MAPPING_FAILED);
    }

}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t read_id_block(info_id_t *id)
{
    uint8_t status;

    status = read_mem(0, 1, (void *) &id->family_id);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(1, 1, (void *) &id->variant_id);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(2, 1, (void *) &id->version);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(3, 1, (void *) &id->build);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(4, 1, (void *) &id->matrix_x_size);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(5, 1, (void *) &id->matrix_y_size);
    if (status != READ_MEM_OK)
    {
        return(status);
    }

    status = read_mem(6, 1, (void *) &id->num_declared_objects);

    return(status);

}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t get_max_report_id()
{
    return(max_report_id);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint16_t get_object_address(uint8_t object_type, uint8_t instance)
{
    uint8_t object_table_index = 0;
    uint8_t address_found = 0;
    uint16_t address = OBJECT_NOT_FOUND;

    object_t *object_table;
    object_t obj;
    object_table = info_block->objects;
    while ((object_table_index < info_block->info_id.num_declared_objects) &&
        !address_found)
    {
        obj = object_table[object_table_index];
        /* Does object type match? */
        if (obj.object_type == object_type)
        {

            address_found = 1;

            /* Are there enough instances defined in the FW? */
            if (obj.instances >= instance)
            {
                address = obj.i2c_address + (obj.size + 1) * instance;
            }
        }
        object_table_index++;
    }

    return(address);
}



/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint32_t get_stored_infoblock_crc()
{
    uint32_t crc;
    crc = (uint32_t) (((uint32_t) info_block->CRC_hi) << 16);
    crc = crc | info_block->CRC;
    return(crc);
}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint8_t calculate_infoblock_crc(uint32_t *crc_pointer)
{

    uint32_t crc = 0;
    uint16_t crc_area_size;
    uint8_t *mem;

    uint8_t i;

    uint8_t status;

    /* 7 bytes of version data, 6 * NUM_OF_OBJECTS bytes of object table. */
    crc_area_size = 7 + info_block->info_id.num_declared_objects * 6;

    mem = (uint8_t *) kmalloc(crc_area_size, GFP_KERNEL | GFP_ATOMIC);
    if (mem == NULL)
    {
        return(CRC_CALCULATION_FAILED);
    }

    status = read_mem(0, crc_area_size, mem);

    if (status != READ_MEM_OK)
    {
        return(CRC_CALCULATION_FAILED);
    }

    i = 0;
    while (i < (crc_area_size - 1))
    {
        crc = CRC_24(crc, *(mem + i), *(mem + i + 1));
        i += 2;
    }

    crc = CRC_24(crc, *(mem + i), 0);

    kfree(mem);

    /* Return only 24 bit CRC. */
    *crc_pointer = (crc & 0x00FFFFFF);
    return(CRC_CALCULATION_OK);

}


/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

uint32_t CRC_24(uint32_t crc, uint8_t byte1, uint8_t byte2)
{
    static const uint32_t crcpoly = 0x80001B;
    uint32_t result;
    uint16_t data_word;

    data_word = (uint16_t) ((uint16_t) (byte2 << 8u) | byte1);
    result = ((crc << 1u) ^ (uint32_t) data_word);

    if (result & 0x1000000)
    {
        result ^= crcpoly;
    }

    return(result);

}




/*------------------------------ main block -----------------------------------*/
void quantum_touch_probe(void)
{
	U8 report_id;
	U8 object_type, instance;
	U32 stored_crc;
	U8 family_id, variant, build;
	uint32_t crc;

	if (init_touch_driver( QT602240_I2C_ADDR_A, QT602240_I2C_ADDR_B ) == DRIVER_SETUP_OK)
	{
		/* "\nTouch device found\n" */
		QT602240_TRACE("touch device found\n");
	}
	else
	{
		/* "\nTouch device NOT found\n" */
		QT602240_ERROR("touch device NOT found\n");
		return ;
	}

		/* Get and show the version information. */
		get_family_id(&family_id);
		get_variant_id(&variant);
		get_version(&tsp_version);
		get_build_number(&build);


		QT602240_TRACE("version: 0x%x\n", tsp_version);
		QT602240_TRACE("family: 0x%x\n", family_id);
		QT602240_TRACE("variant: 0x%x\n", variant);
		QT602240_TRACE("build number: 0x%x\n", build);

		QT602240_TRACE("matrix X size : %d\n", info_block->info_id.matrix_x_size);
		QT602240_TRACE("matrix Y size : %d\n", info_block->info_id.matrix_y_size);

		if(calculate_infoblock_crc(&crc) != CRC_CALCULATION_OK)
		{
			QT602240_ERROR("calculating CRC failed, skipping check!\n");
		}
		else
		{
			QT602240_TRACE("Calculated CRC\n");
			write_message_to_usart((uint8_t *) &crc, 4);
		}

		stored_crc = get_stored_infoblock_crc();
		QT602240_TRACE("Stored CRC\n");
		write_message_to_usart((uint8_t *) &stored_crc, 4);

		if (stored_crc != crc)
		{
			QT602240_ERROR("warning: info block CRC value doesn't match the calculated\n");
		}
		else
		{
			QT602240_TRACE("info block CRC value OK\n");
		}

		/* Test the report id to object type / instance mapping: get the maximum
		* report id and print the report id map. */

//		QT602240_TRACE("report ID to object type / instance mapping:\n");

		max_report_id = get_max_report_id();
		for (report_id = 0; report_id <= max_report_id; report_id++)
		{
			object_type = report_id_to_type(report_id, &instance);
//			QT602240_TRACE("report ID: %d, object type: T%d, instance: %d\n",report_id, object_type, instance);
		}

#ifdef OPTION_WRITE_CONFIG
        QT602240_TRACE("chip setup sequence is now configuring\n");

		qt_Userdata_Init();
		
		qt_Power_Config_Init();

		qt_Acquisition_Config_Init();

		qt_Multitouchscreen_Init();

		qt_KeyArray_Init();

		qt_ComcConfig_Init();

		qt_Gpio_Pwm_Init();

		qt_Grip_Face_Suppression_Config_Init();

		qt_Noise_Suppression_Config_Init();

		qt_Proximity_Config_Init();

		qt_One_Touch_Gesture_Config_Init();

		qt_Selftest_Init();

		//qt_Two_touch_Gesture_Config_Init();

		qt_CTE_Config_Init();

		/* Backup settings to NVM. */
		if (backup_config() != WRITE_MEM_OK)
		{
			QT602240_ERROR("failed to backup, exiting\n");
			return;
		}
		else
		{
			//QT602240_TRACE("backed up the config to non-volatile memory\n");
		}

#else
		QT602240_TRACE("chip setup sequence was bypassed\n");
#endif /* OPTION_WRITE_CONFIG */

		/* reset the touch IC. */
		if (reset_chip() != WRITE_MEM_OK)
		{
			QT602240_ERROR("failed to reset, exiting\n");
			return;
		}
		else
		{
			msleep(60);
			QT602240_TRACE("chip reset OK\n");
		}

}

/*------------------------------ Sub functions -----------------------------------*/
/*!
  \brief Initializes touch driver.

  This function initializes the touch driver: tries to connect to given 
  address, sets the message handler pointer, reads the info block and object
  table, sets the message processor address etc.

  @param I2C_address is the address where to connect.
  @param (*handler) is a pointer to message handler function.
  @return DRIVER_SETUP_OK if init successful, DRIVER_SETUP_FAILED 
  otherwise.
  */

uint8_t touch_i2c_address_probe(uint8_t I2C_address_A, uint8_t I2C_address_B)
{
    uint8_t status;
    uint8_t buf;

	qt602240->client->addr = I2C_address_A;;

	QT602240_TRACE("0x%02x probe\n", I2C_address_A);
    status = read_mem(0, 1, (void *) &buf);
    if (status != READ_MEM_OK)
    {
		QT602240_ERROR("0x%02x probe fail: %d\n", I2C_address_A, status);
		QT602240_ERROR("0x%02x probe\n", I2C_address_B);
		qt602240->client->addr = I2C_address_B;;
	    status = read_mem(0, 1, (void *) &buf);
	    if (status != READ_MEM_OK)
		{
			QT602240_ERROR("0x%02x probe fail: %d\n",I2C_address_B, status);
		}
		else
		{
			QT602240_TRACE("0x%02x probe success\n",I2C_address_B);
		}
    }
	else
	{
		QT602240_TRACE("0x%02x probe success\n",I2C_address_A);
	}

	return (status);
}

uint8_t init_touch_driver(uint8_t I2C_address_A, uint8_t I2C_address_B)
{
	int i;

	int current_report_id = 0;

	uint8_t tmp;
	uint16_t current_address;
	uint16_t crc_address;
	object_t *object_table;
	info_id_t *id;
	uint8_t status;

	QT602240_TRACE("start\n");
	/* Set the message handler function pointer. */ 
	//application_message_handler = handler;

	touch_i2c_address_probe(I2C_address_A,I2C_address_B);

#ifdef FEATUE_QT_INFOBLOCK_STATIC	// by donghoon.kwak
	/* Read the info block data. */

	id = &info_block->info_id;

	if (read_id_block(id) != 1)
	{
		QT602240_ERROR("error: 2\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}  

	/* Read object table. */

	info_block->objects = info_object_ptr;
	object_table = info_block->objects;


	/* Reading the whole object table block to memory directly doesn't work cause sizeof object_t
	   isn't necessarily the same on every compiler/platform due to alignment issues. Endianness
	   can also cause trouble. */

	current_address = OBJECT_TABLE_START_ADDRESS;


	for (i = 0; i < id->num_declared_objects; i++)
	{
		status = read_mem(current_address, 1, &(object_table[i]).object_type);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 4\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_U16(current_address, &object_table[i].i2c_address);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 5\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address += 2;

		status = read_mem(current_address, 1, (U8*)&object_table[i].size);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 6\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].instances);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 7\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].num_report_ids);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 8\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		max_report_id += object_table[i].num_report_ids;

		/* Find out the maximum message length. */
		if (object_table[i].object_type == GEN_MESSAGEPROCESSOR_T5)
		{
			max_message_length = object_table[i].size + 1;
		}
	}

	/* Check that message processor was found. */
	if (max_message_length == 0)
	{
		QT602240_ERROR("error: 9\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Read CRC. */

	crc_address = OBJECT_TABLE_START_ADDRESS + 
		id->num_declared_objects * OBJECT_TABLE_ELEMENT_SIZE;

	status = read_mem(crc_address, 1u, &tmp);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 11\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC = tmp;

	status = read_mem(crc_address + 1u, 1u, &tmp);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 12\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC |= (tmp << 8u);

	status = read_mem(crc_address + 2, 1, &info_block->CRC_hi);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 13\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Store message processor address, it is needed often on message reads. */
	message_processor_address = get_object_address(GEN_MESSAGEPROCESSOR_T5, 0);
	//QT602240_TRACE("message_processor_address: %d\n", message_processor_address);

	if (message_processor_address == 0)
	{
		QT602240_ERROR("error: 14\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Store command processor address. */
	command_processor_address = get_object_address(GEN_COMMANDPROCESSOR_T6, 0);
	if (command_processor_address == 0)
	{
		QT602240_ERROR("error: 15\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

#else
	/* Read the info block data. */

	id = (info_id_t *) kmalloc(sizeof(info_id_t), GFP_KERNEL | GFP_ATOMIC);
	if (id == NULL)
	{
		return(DRIVER_SETUP_INCOMPLETE);
	}



	if (read_id_block(id) != 1)
	{
		QT602240_ERROR("error: 2\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}  

	/* Read object table. */

	object_table = (object_t *) kmalloc(id->num_declared_objects * sizeof(object_t), GFP_KERNEL | GFP_ATOMIC);
	if (object_table == NULL)
	{
		QT602240_ERROR("error: 3\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}


	/* Reading the whole object table block to memory directly doesn't work cause sizeof object_t
	   isn't necessarily the same on every compiler/platform due to alignment issues. Endianness
	   can also cause trouble. */

	current_address = OBJECT_TABLE_START_ADDRESS;


	for (i = 0; i < id->num_declared_objects; i++)
	{
		status = read_mem(current_address, 1, &(object_table[i]).object_type);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 4\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_U16(current_address, &object_table[i].i2c_address);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 5\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address += 2;

		status = read_mem(current_address, 1, (U8*)&object_table[i].size);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 6\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].instances);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 7\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		status = read_mem(current_address, 1, &object_table[i].num_report_ids);
		if (status != READ_MEM_OK)
		{
			QT602240_ERROR("error: 8\n");
			return(DRIVER_SETUP_INCOMPLETE);
		}
		current_address++;

		max_report_id += object_table[i].num_report_ids;

		/* Find out the maximum message length. */
		if (object_table[i].object_type == GEN_MESSAGEPROCESSOR_T5)
		{
			max_message_length = object_table[i].size + 1;
		}
	}

	/* Check that message processor was found. */
	if (max_message_length == 0)
	{
		QT602240_ERROR("error: 9\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Read CRC. */

	CRC = (uint32_t *) kmalloc(sizeof(info_id_t), GFP_KERNEL | GFP_ATOMIC);
	if (CRC == NULL)
	{
		QT602240_ERROR("error: 10\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}



	info_block = kmalloc(sizeof(info_block_t), GFP_KERNEL | GFP_ATOMIC);
	if (info_block == NULL)
	{
		QT602240_ERROR("info_block alloc fail\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}


	info_block->info_id = *id;

	info_block->objects = object_table;

	crc_address = OBJECT_TABLE_START_ADDRESS + 
		id->num_declared_objects * OBJECT_TABLE_ELEMENT_SIZE;

	status = read_mem(crc_address, 1u, &tmp);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 11\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC = tmp;

	status = read_mem(crc_address + 1u, 1u, &tmp);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 12\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}
	info_block->CRC |= (tmp << 8u);

	status = read_mem(crc_address + 2, 1, &info_block->CRC_hi);
	if (status != READ_MEM_OK)
	{
		QT602240_ERROR("error: 13\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Store message processor address, it is needed often on message reads. */
	message_processor_address = get_object_address(GEN_MESSAGEPROCESSOR_T5, 0);
	//QT602240_TRACE("message_processor_address: %d\n", message_processor_address);

	if (message_processor_address == 0)
	{
		QT602240_ERROR("error: 14\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Store command processor address. */
	command_processor_address = get_object_address(GEN_COMMANDPROCESSOR_T6, 0);
	if (command_processor_address == 0)
	{
		QT602240_ERROR("error: 15\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	quantum_msg = kmalloc(max_message_length, GFP_KERNEL | GFP_ATOMIC);
	if (quantum_msg == NULL)
	{
		QT602240_ERROR("error: 16\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

	/* Allocate memory for report id map now that the number of report id's 
	 * is known. */

	report_id_map = kmalloc(sizeof(report_id_map_t) * max_report_id, GFP_KERNEL | GFP_ATOMIC);

	if (report_id_map == NULL)
	{
		QT602240_ERROR("error: 17\n");
		return(DRIVER_SETUP_INCOMPLETE);
	}

#endif
	/* Report ID 0 is reserved, so start from 1. */

	report_id_map[0].instance = 0;
	report_id_map[0].object_type = 0;
	current_report_id = 1;

	for (i = 0; i < id->num_declared_objects; i++)
	{
		if (object_table[i].num_report_ids != 0)
		{
			int instance;
			for (instance = 0; 
					instance <= object_table[i].instances; 
					instance++)
			{
				int start_report_id = current_report_id;
				for (; 
						current_report_id < 
						(start_report_id + object_table[i].num_report_ids);
						current_report_id++)
				{
					report_id_map[current_report_id].instance = instance;
					report_id_map[current_report_id].object_type = 
						object_table[i].object_type;
				}
			}
		}
	}
	driver_setup = DRIVER_SETUP_OK;

	/* Initialize the pin connected to touch ic pin CHANGELINE to catch the
	 * falling edge of signal on that line. */

	return(DRIVER_SETUP_OK);
}

void read_all_register(void)
{
	U16 addr = 0;
	U8 msg;
	U16 calc_addr = 0;

	for(addr = 0 ; addr < 1273 ; addr++)
	{
		calc_addr = addr;

		if(read_mem(addr, 1, &msg) == READ_MEM_OK)
		{
			QT602240_TRACE("0x%2x\n", msg);
			if( (addr+1) % 10 == 0)
			{
				QT602240_TRACE("%2d:\n", addr+1);
			}

		}else
		{
			QT602240_ERROR("read fail\n");
		}
	}
}

#if DEBUG_PRESS
void print_msg(void)
{
	if ((quantum_msg[1] & 0x80) == 0x80 )
	{
		QT602240_TRACE("1\n");	
	}
	else
	{
		QT602240_TRACE("0\n");
	}
	
	if ((quantum_msg[1] & 0x40) == 0x40 )
	{
		QT602240_TRACE("1\n");	
	}
	else
	{
		QT602240_TRACE("0\n");	
	}
	
	if ((quantum_msg[1] & 0x20) == 0x20 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}
	
	if ((quantum_msg[1] & 0x10) == 0x10 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}

	if ((quantum_msg[1] & 0x08) == 0x08 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}
	
	if ((quantum_msg[1] & 0x04) == 0x04 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}

	if ((quantum_msg[1] & 0x02) == 0x02 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}
	
	if ((quantum_msg[1] & 0x01) == 0x01 )
	{
		QT602240_TRACE("1\n");
	}
	else
	{
		QT602240_TRACE("0\n");
	}
}
#endif

static unsigned int qt_time_point=0;
static unsigned int qt_time_diff=0;
static unsigned int qt_timer_state =0;
static int check_abs_time(void)
{

	if (!qt_time_point)
		return 0;

	qt_time_diff = jiffies_to_msecs(jiffies) - qt_time_point;
	QT602240_TRACE("qt_timer_state:%d, qt_time_point: %d, qt_time_diff: %d\n", qt_timer_state, qt_time_point, qt_time_diff);
	if(qt_time_diff >0)
		return 1;
	else
		return 0;
	
}

//hugh 0312
uint8_t good_check_flag = 0; 
//20100217 julia
//hugh 0312 void check_chip_calibration(void)
void check_chip_calibration(unsigned char one_touch_input_flag)
{
	uint8_t data_buffer[100] = { 0 };
	uint8_t try_ctr = 0;
	uint8_t data_byte = 0xF3; /* dianostic command to get touch flags */
	uint16_t diag_address;
	uint8_t tch_ch = 0, atch_ch = 0;
	uint8_t check_mask;
	uint8_t i;
	uint8_t j;
	uint8_t x_line_limit;

	if(tsp_version >=0x16)
	{
		/* we have had the first touchscreen or face suppression message 
		 * after a calibration - check the sensor state and try to confirm if
		 * cal was good or bad */

		/* get touch flags from the chip using the diagnostic object */
		/* write command to command processor to get touch flags - 0xF3 Command required to do this */
		write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);
		/* get the address of the diagnostic object so we can get the data we need */
		diag_address = get_object_address(DEBUG_DIAGNOSTIC_T37,0);

		msleep(10); 

		/* read touch flags from the diagnostic object - clear buffer so the while loop can run first time */
		memset( data_buffer , 0xFF, sizeof( data_buffer ) );

		/* wait for diagnostic object to update */
		while(!((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00)))
		{
			/* wait for data to be valid  */
			if(try_ctr > 10) //0318 hugh 100-> 10
			{
				/* Failed! */
				QT602240_ERROR("diagnostic data did not update\n");
				qt_timer_state = 0;//0430 hugh
				break;
			}
			msleep(2); //0318 hugh  3-> 2
			try_ctr++; /* timeout counter */
			read_mem(diag_address, 2,data_buffer);
			QT602240_TRACE("waiting for diagnostic data to update, try %d\n", try_ctr);
		}


		/* data is ready - read the detection flags */
		read_mem(diag_address, 82,data_buffer);

		/* data array is 20 x 16 bits for each set of flags, 2 byte header, 40 bytes for touch flags 40 bytes for antitouch flags*/

		/* count up the channels/bits if we recived the data properly */
		if((data_buffer[0] == 0xF3) && (data_buffer[1] == 0x00))
		{

			/* mode 0 : 16 x line, mode 1 : 17 etc etc upto mode 4.*/
			x_line_limit = 16 + cte_config.mode;
			if(x_line_limit > 20)
			{
				/* hard limit at 20 so we don't over-index the array */
				x_line_limit = 20;
			}

			/* double the limit as the array is in bytes not words */
			x_line_limit = x_line_limit << 1;

			/* count the channels and print the flags to the log */
			for(i = 0; i < x_line_limit; i+=2) /* check X lines - data is in words so increment 2 at a time */
			{
				/* print the flags to the log - only really needed for debugging */
				//QT602240_TRACE("detect flags X%d, %x%x, %x%x \n", i>>1,data_buffer[3+i],data_buffer[2+i],data_buffer[43+i],data_buffer[42+i]);

				/* count how many bits set for this row */
				for(j = 0; j < 8; j++)
				{
					/* create a bit mask to check against */
					check_mask = 1 << j;

					/* check detect flags */
					if(data_buffer[2+i] & check_mask)
					{
						tch_ch++;
					}
					if(data_buffer[3+i] & check_mask)
					{
						tch_ch++;
					}

					/* check anti-detect flags */
					if(data_buffer[42+i] & check_mask)
					{
						atch_ch++;
					}
					if(data_buffer[43+i] & check_mask)
					{
						atch_ch++;
					}
				}
			}


			/* print how many channels we counted */
			QT602240_TRACE("flags counted channels: t:%d a:%d\n", tch_ch, atch_ch);

			/* send page up command so we can detect when data updates next time,
			 * page byte will sit at 1 until we next send F3 command */
			data_byte = 0x01;
			write_mem(command_processor_address + DIAGNOSTIC_OFFSET, 1, &data_byte);

			/* process counters and decide if we must re-calibrate or if cal was good */      
//0413			if((tch_ch>=0) && (atch_ch <= 1))  //jwlee change.
			if((tch_ch>0) && (atch_ch == 0))  //jwlee change.
			{
				/* cal was good - don't need to check any more */
				//hugh 0312
#if 1
				if(!check_abs_time())
					qt_time_diff=501;	
//				else
//					QT602240_TRACE("CURRENT time diff: %d, qt_timer_state: %d\n", qt_time_diff, qt_timer_state);
				
				if(qt_timer_state == 1)
				{
#if defined(USE_THREADED_IRQ)
					if(qt_time_diff > 50)
#else
					if(qt_time_diff > 500)
#endif
#else
				if(good_check_flag >=2)
#endif
				{
					QT602240_TRACE("calibration was good\n");
					cal_check_flag = 0;
					good_check_flag = 0;
					qt_timer_state =0;
					qt_time_point = jiffies_to_msecs(jiffies);

					QT602240_TRACE("reset acq atchcalst: %d, atchcalsthr: %d\n", acquisition_config.atchcalst, acquisition_config.atchcalsthr);
					/* Write normal acquisition config back to the chip. */
					if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
					{
						/* "Acquisition config write failed!\n" */
						QT602240_ERROR("configuration fail\n");
						// MUST be fixed
					}

				}
					else 
					{
						cal_check_flag = 1;
					}
				}
#if 1
				else {
					qt_timer_state=1;
					qt_time_point = jiffies_to_msecs(jiffies);
					cal_check_flag=1;
					}
#else
				else if(one_touch_input_flag == 1)
				{
					good_check_flag++;
				       QT602240_TRACE("good_check_flag became %d\n", good_check_flag);
				}        
				else if( one_touch_input_flag != 1 )
				{
				        QT602240_TRACE("do calibrate_chip because of multi touch\n");
				        good_check_flag=0;
				        calibrate_chip();
				        
				}       
#endif
			}
//Org of Dan			else if((tch_ch + CAL_THR /*10*/ ) <= atch_ch)
			else if(atch_ch >= 8)		//jwlee add 0325
			{
#if 1
				QT602240_TRACE("calibration was bad\n");
				/* cal was bad - must recalibrate and check afterwards */
				calibrate_chip();
				qt_timer_state=0;
				qt_time_point = jiffies_to_msecs(jiffies);
#else
                            good_check_flag = 0; //hugh 0312
				QT602240_TRACE("calibration was bad\n");
				/* cal was bad - must recalibrate and check afterwards */
				calibrate_chip();
#endif
			}
#if 1
			else {
				QT602240_TRACE("calibration was not decided yet\n");
				/* we cannot confirm if good or bad - we must wait for next touch  message to confirm */
				cal_check_flag = 1u;
				/* Reset the 100ms timer */
				qt_timer_state=0;//0430 hugh 1 --> 0
				qt_time_point = jiffies_to_msecs(jiffies);
				}
#endif

		}
	}
}

int qt_touch_num_state[MAX_USING_FINGER_NUM]={0};
unsigned int touch_state_val=0;
EXPORT_SYMBOL(touch_state_val);

#if USE_TS_TA_DETECT_CHANGE_REG 
int set_tsp_for_ta_detect(int state)
{
	uint16_t object_address;
    	uint8_t *tmp;
    	uint8_t status;
//    	uint8_t object_size;
//	int ret = WRITE_MEM_OK;			

	if (qt_initial_ok == 0)
		return - 1;

	disable_irq(qt602240->client->irq);
	if(state)
		{
			touchscreen_config.tchthr = 70;
			noise_suppression_config.noisethr = 20;		   

			QT602240_TRACE("TA Detect\n");

			object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9, 0);

			if (object_address == 0)
			{
			    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 object_address\n");
				enable_irq(qt602240->client->irq);
				return -1;
			}
			tmp= &touchscreen_config.tchthr;
			status = write_mem(object_address+7, 1, tmp);	
			
			if (status == WRITE_MEM_FAILED)
			{
			    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
			}

			object_address = get_object_address(PROCG_NOISESUPPRESSION_T22, 0);

			if (object_address == 0)
			{
			    QT602240_ERROR("PROCG_NOISESUPPRESSION_T22 object_address\n");
				enable_irq(qt602240->client->irq);
				return -1;
			}
			tmp= &noise_suppression_config.noisethr ;
			status = write_mem(object_address+8, 1, tmp);	
			
			if (status == WRITE_MEM_FAILED)
			{
			    QT602240_ERROR("PROCG_NOISESUPPRESSION_T22 write_mem\n");
			}	
			
        }
		else 
		{
		    touchscreen_config.tchthr = 40;
		    noise_suppression_config.noisethr = 30;		   

		    QT602240_TRACE("TA NON-Detect\n");

			object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9, 0);

			if (object_address == 0)
			{
			    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 object_address\n");
				enable_irq(qt602240->client->irq);
				return -1;
			}
			tmp= &touchscreen_config.tchthr;
			status = write_mem(object_address+7, 1, tmp);	
			
			if (status == WRITE_MEM_FAILED)
			{
			    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
			}

			object_address = get_object_address(PROCG_NOISESUPPRESSION_T22, 0);

			if (object_address == 0)
			{
			    QT602240_ERROR("PROCG_NOISESUPPRESSION_T22 object_address\n");
				enable_irq(qt602240->client->irq);
				return -1;
			}
			tmp= &noise_suppression_config.noisethr ;
			status = write_mem(object_address+8, 1, tmp);	
			
			if (status == WRITE_MEM_FAILED)
			{
			   QT602240_ERROR("PROCG_NOISESUPPRESSION_T22 write_mem\n");
			}	
        }

	enable_irq(qt602240->client->irq);
	return 1;
}
EXPORT_SYMBOL(set_tsp_for_ta_detect);
#endif

void TSP_forced_release(void)
{
	int i;
	int temp_value=0;

	if (qt_initial_ok == 0)
		return;

	for ( i= 1; i<MAX_USING_FINGER_NUM; i++ )
	{
		if ( fingerInfo[i].pressure == -1 ) continue;

		fingerInfo[i].pressure = 0;

		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
		input_report_abs(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);	// 0이면 Release, 아니면 Press 상태(Down or Move)
		input_report_abs(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].size_id);	// (ID<<8) | Size
		input_mt_sync(qt602240->input_dev);

		if ( fingerInfo[i].pressure == 0 ) fingerInfo[i].pressure= -1;
		temp_value++;
	}
	if(temp_value>0)
	input_sync(qt602240->input_dev);
}
EXPORT_SYMBOL(TSP_forced_release);

void TSP_forced_release_for_call(void)
{
	int i=0;
	int temp_value=0;
//    	uint8_t status;	
	int ret = WRITE_MEM_OK;		

	if (qt_initial_ok == 0)
		return;
	
	for ( i=0; i<MAX_USING_FINGER_NUM; ++i )
	{
		if ( fingerInfo[i].pressure == -1 ) continue;

		fingerInfo[i].pressure = 0;
		command_config.reportall = 1;

		/* Write temporary command config to chip. */
		if (write_command_config(command_config) != CFG_WRITE_OK)
		{
		/* "Acquisition config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
		}		

		QT602240_TRACE("start\n");
		QT602240_TRACE("reset acq reportall: %d\n", command_config.reportall);

		msleep(20);
		calibrate_chip();
		msleep(20);	

		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
		input_report_abs(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);	// 0이면 Release, 아니면 Press 상태(Down or Move)
		input_report_abs(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].size_id);	// (ID<<8) | Size
		input_mt_sync(qt602240->input_dev);

		if ( fingerInfo[i].pressure == 0 ) fingerInfo[i].pressure= -1;
		temp_value++;
	}
	if(temp_value>0)
		input_sync(qt602240->input_dev);

}

void TSP_forced_release_forOKkey(void)
{
	int i;
	int temp_value=0;

	if (qt_initial_ok == 0)
		return;
	
	for ( i=0; i<MAX_USING_FINGER_NUM; i++ )
	{
		if ( fingerInfo[i].pressure == -1 ) continue;

		fingerInfo[i].pressure = 0;

		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
		input_report_abs(qt602240->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
		input_report_abs(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);	// 0이면 Release, 아니면 Press 상태(Down or Move)
		input_report_abs(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].size_id);	// (ID<<8) | Size
		input_mt_sync(qt602240->input_dev);

     		force_cal_check_flag = 1;
		if ( fingerInfo[i].pressure == 0 ) fingerInfo[i].pressure= -1;
		temp_value++;
	}
	if(temp_value>0)
		input_sync(qt602240->input_dev);
}

#define PRESSED 	1
#define RELEASED 	0

#ifdef LG_FW_TOUCH_SOFT_KEY
enum{
	NO_KEY_TOUCHED,
	KEY1_TOUCHED,
	KEY2_TOUCHED,
	KEY3_TOUCHED,
	KEY4_TOUCHED,
	MAX_KEY_TOUCH
};
#endif

static __inline void qt_key_event_touch(int touch_reg,  int value)
{

	input_report_key(qt602240->input_dev, touch_reg, value);
	input_sync(qt602240->input_dev);

	QT602240_TRACE("touch key code: %d, value: %d\n", touch_reg, value);

	return;
}


EXPORT_SYMBOL(TSP_forced_release_forOKkey);

#if defined(MOVE_EVENT_WA)
#define MOV_LIMIT (4)
#define MAX_MOV_CNT (30)
static int mov_limit_cnt = 0;
#endif

#if defined(USE_THREADED_IRQ)
void  get_message(void)
#else
void  get_message(struct work_struct * p)
#endif
{
	unsigned long x, y;
	unsigned int press = 3;
	uint8_t ret_val = MESSAGE_READ_FAILED;
	int size=2, i;	
//	int btn_report=0;	
//	static int check_flag=0;
	uint8_t touch_message_flag = 0;
       uint8_t one_touch_input_flag=0;//hugh 0312

#ifdef _SUPPORT_MULTITOUCH_
	static int nPrevID= -1;
	uint8_t id = 0;
	int bChangeUpDn= 0;
#endif
int ret = WRITE_MEM_OK;
	static long cnt=0;
//	int amplitude=0;	
	cnt++;


	//QT602240_TRACE ("start\n");
	//disable_irq(qt602240->client->irq);

	if (driver_setup == DRIVER_SETUP_OK && qt602240_init_fail == 0)
	{
	#ifdef _SUPPORT_TOUCH_AMPLITUDE_
		if(read_mem(message_processor_address, 7, quantum_msg) == READ_MEM_OK)
	#else
		if(read_mem(message_processor_address, 6, quantum_msg) == READ_MEM_OK)
	#endif
		{
			/* Call the main application to handle the message. */
			//QT602240_TRACE("msg id: %d ", quantum_msg[0]);

			//20102017 julia
			if( quantum_msg[0] == 14 )
			{
				if((quantum_msg[1]&0x01) == 0x00)   
				{ 
					QT602240_TRACE("palm touch! - %d released, ver. %x\n", quantum_msg[1], info_block->info_id.version);	
					for ( i= 0; i<MAX_USING_FINGER_NUM; ++i )
					{
						if ( fingerInfo[i].pressure == -1 ) continue;

						if(i == 0){
							touch_state_val=0;
							}
						fingerInfo[i].pressure= 0;
						input_report_abs(qt602240->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
						input_report_abs(qt602240->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
						input_report_abs(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);	// 0이면 Release, 아니면 Press 상태(Down or Move)
						input_report_abs(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].size_id);	// (ID<<8) | Size
						input_mt_sync(qt602240->input_dev);
			
						qt_touch_num_state[i]--;
						if ( fingerInfo[i].pressure == 0 ) fingerInfo[i].pressure= -1;
						else if( fingerInfo[i].pressure > 0 ) one_touch_input_flag++;//hugh 0312
					}
					input_sync(qt602240->input_dev);
				}
				else
				{
					QT602240_TRACE("palm touch! - %d suppressed\n", quantum_msg[1]);	
					touch_message_flag = 1;
					one_touch_input_flag = 1; //hugh 0312
//					if(cal_check_flag == 2)
					msleep(30);
					command_config.reportall = 1;
					/* Write temporary command config to chip. */
					if (write_command_config(command_config) != CFG_WRITE_OK)
					{
					/* "Acquisition config write failed!\n" */
					QT602240_TRACE("configuration fail\n");
					ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
					}						
				}
			}	
			
				QT602240_TRACE("msg id: %x %x %x %x %x %x %x %x %x\n", quantum_msg[0], quantum_msg[1], quantum_msg[2], quantum_msg[3], quantum_msg[4], quantum_msg[5], quantum_msg[6], quantum_msg[7], quantum_msg[8]);
			if(quantum_msg[0] < 2  || quantum_msg[0] >= 12) {

				if((quantum_msg[0] == 1)&&((quantum_msg[1]&0x10) == 0x10)){
					QT602240_TRACE("quantum_msg[0] = 1 and quantum_msg[1] = 0x10 cal_check_flag=1\n");
//					cal_check_flag = 1u;
				}
				else if((quantum_msg[0] == 1) && ((quantum_msg[1]&0x10)==0)/* && (cal_check_flag==1)*/){
					QT602240_TRACE("quantum_msg[0] = 1 and quantum_msg[1] = 0x00 cal_check_flag=2\n");
//					cal_check_flag = 2u;
					if(touch_message_flag && (cal_check_flag/*==2*/))
					{
						check_chip_calibration(1);
					}
				}

				if ((quantum_msg[0] ==  14) ||(quantum_msg[0] == 0) ||(quantum_msg[0] == 0xFF)) {
					if(touch_message_flag && (cal_check_flag/*==2*/))
					{
						check_chip_calibration(one_touch_input_flag);
					}
				}
#if defined(USE_THREADED_IRQ)
#else
				enable_irq(qt602240->client->irq);
#endif
				return ;
			} 

			if(qt602240->touch_key && quantum_msg[0] == 10) // touch key
			{
				int  touchkey = quantum_msg[2];
				if(quantum_msg[1] == 0x80) // detect
				{
					if(touchkey_array[touchkey].id == 0xFF)
					{
						touchkey_array[touchkey].id = 0x0;
						input_report_key(qt602240->input_dev, touchkey_array[touchkey].key, 1); //pressed
						QT602240_TRACE("key: %d DOWN\n", touchkey_array[touchkey].key);
						input_sync(qt602240->input_dev);
					}
				}
				else if(quantum_msg[1] == 0x00) // release
				{
					if(touchkey_array[QT_TOUCHKEY_MENU].id != 0xFF)
					{
						touchkey_array[QT_TOUCHKEY_MENU].id = 0xFF;
						input_report_key(qt602240->input_dev, touchkey_array[QT_TOUCHKEY_MENU].key, 0);  //released
						QT602240_TRACE("key : MENU UP\n");
						input_sync(qt602240->input_dev);
					}
					if(touchkey_array[QT_TOUCHKEY_HOME].id != 0xFF)
					{
						touchkey_array[QT_TOUCHKEY_HOME].id = 0xFF;
						input_report_key(qt602240->input_dev, touchkey_array[QT_TOUCHKEY_HOME].key, 0);  //released
						QT602240_TRACE("key : HOME UP\n");
						input_sync(qt602240->input_dev);
					}
					if(touchkey_array[QT_TOUCHKEY_BACK].id != 0xFF)
					{
						touchkey_array[QT_TOUCHKEY_BACK].id = 0xFF;
						input_report_key(qt602240->input_dev, touchkey_array[QT_TOUCHKEY_BACK].key, 0);  //released
						QT602240_TRACE("key : BACK UP\n");
						input_sync(qt602240->input_dev);
					}
				}
#if defined(USE_THREADED_IRQ)
#else
				enable_irq(qt602240->client->irq);
#endif
				return;
			}
			
#ifdef _SUPPORT_MULTITOUCH_
			id= quantum_msg[0] - 2;
#endif

			x = quantum_msg[2];
			x = x << 2;
			x = x | quantum_msg[4] >> 6;

			y = quantum_msg[3];
			y = y << 2;
			y = y | ((quantum_msg[4] & 0x0C)  >> 2);

			size = quantum_msg[5];
#if 0
			if( size > 20 ) {
				print_msg();
				QT602240_TRACE("oversize detected, size: %d\n", size);
				s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
				enable_irq(qt602240->client->irq);
				return ;
			}
#endif
			/* 
			 * quantum_msg[1] & 0x80 : 10000000 -> DETECT 
			 * quantum_msg[1] & 0x40 : 01000000 -> PRESS
			 * quantum_msg[1] & 0x20 : 00100000 -> RELEASE
			 * quantum_msg[1] & 0x10 : 00010000 -> MOVE
			 * quantum_msg[1] & 0x08 : 00001000 -> VECTOR
			 * quantum_msg[1] & 0x04 : 00000100 -> AMP
			 * quantum_msg[1] & 0x02 : 00000010 -> SUPPRESS
			 */
#ifdef _SUPPORT_MULTITOUCH_
			if ( (quantum_msg[1] & 0x20 ) )	// Release
			{				
				fingerInfo[id].size_id= (id<<8)|size;

	#ifdef LG_FW_TOUCH_SOFT_KEY
				if (qt602240_pdata->ts_y_scrn_max <= fingerInfo[id].y)
				{
					QT602240_TRACE("finger[%d] Up (%d,%d) - touch num is (%d)  status=0x%02x, over panel max, ignored\n", id, fingerInfo[id].x, fingerInfo[id].y, --qt_touch_num_state[id], quantum_msg[1]);

					for(i=0;i<QT_TOUCHKEY_MAX;i++)
					{
						if(	touchkey_area[i].x1 <= fingerInfo[id].x &&
							touchkey_area[i].x2 >= fingerInfo[id].x &&
							touchkey_area[i].y1 <= fingerInfo[id].y &&
							touchkey_area[i].y2 >= fingerInfo[id].y )
						{
							QT602240_TRACE("find touch key: key(%d), cur_id(%d), prs_id(%d)\n", touchkey_area[i].key, id, touchkey_area[i].id);

	                        if(touchkey_area[i].id == id)
							{
		                        touchkey_area[i].id = 0xFF;
								qt_key_event_touch(touchkey_area[i].key,  RELEASED);
							}
							break;
						}
					}

					// put invalid pressure value;
					fingerInfo[id].size_id = 0;
					fingerInfo[id].pressure = -1;
					fingerInfo[id].x = (int16_t)0;
					fingerInfo[id].y = (int16_t)0;

				}
				else
	#endif			
				{	
#if defined(MOVE_EVENT_WA)
					mov_limit_cnt = 0;
#endif
					fingerInfo[id].pressure= 0;
					bChangeUpDn= 1;
					QT602240_TRACE("finger[%d] Up (%d,%d) - touch num is (%d)  status=0x%02x\n", id, fingerInfo[id].x, fingerInfo[id].y, --qt_touch_num_state[id], quantum_msg[1]);
				}
			}
			else if ( (quantum_msg[1] & 0x80) && (quantum_msg[1] & 0x40) )	// Detect & Press
			{
			touch_message_flag = 1; //20100217 julia

				fingerInfo[id].size_id= (id<<8)|size;
			#ifdef _SUPPORT_TOUCH_AMPLITUDE_
				fingerInfo[id].pressure= quantum_msg[6];
			#else
				fingerInfo[id].pressure= 40;
			#endif

	#ifdef LG_FW_TOUCH_SOFT_KEY
				if (qt602240_pdata->ts_y_scrn_max <= y)
				{
					QT602240_TRACE("finger[%d] Down (%d,%d) - touch num is (%d)   status=0x%02x, over panel max, ignored\n", id, x, y, ++qt_touch_num_state[id], quantum_msg[1]);

					for(i=0;i<QT_TOUCHKEY_MAX;i++)
					{
						if(	touchkey_area[i].x1 <= x &&
							touchkey_area[i].x2 >= x &&
							touchkey_area[i].y1 <= y &&
							touchkey_area[i].y2 >= y )
						{
							QT602240_TRACE("find touch key: key(%d), cur_id(%d), prs_id(%d)\n", touchkey_area[i].key, id, touchkey_area[i].id);

	                        if(touchkey_area[i].id == 0xFF)
							{
		                        touchkey_area[i].id = id;
								qt_key_event_touch(touchkey_area[i].key,  PRESSED);
								fingerInfo[id].x = (int16_t)x;
								fingerInfo[id].y = (int16_t)y;
							}
							break;
						}
					}

					// put invalid pressure value;
					fingerInfo[id].size_id = 0;
					fingerInfo[id].pressure = -1;
				}
				else
	#endif			
				{
#if defined(MOVE_EVENT_WA)
					mov_limit_cnt = 0;
#endif				
					fingerInfo[id].x = (int16_t)x;
					fingerInfo[id].y = (int16_t)y;
					bChangeUpDn= 1;
					QT602240_TRACE("finger[%d] Down (%d,%d) - touch num is (%d) status=0x%02x\n", id, fingerInfo[id].x, fingerInfo[id].y, ++qt_touch_num_state[id], quantum_msg[1]);
				}
			}
			else if ( (quantum_msg[1] & 0x80) && (quantum_msg[1] & 0x10) )	// Detect & Move
			{
				touch_message_flag = 1;
				fingerInfo[id].size_id= (id<<8)|size;
			#ifdef _SUPPORT_TOUCH_AMPLITUDE_
				fingerInfo[id].pressure= quantum_msg[6];
			#endif

	#ifdef LG_FW_TOUCH_SOFT_KEY
				if (qt602240_pdata->ts_y_scrn_max <= y)
				{

					for(i=0;i<QT_TOUCHKEY_MAX;i++)
					{
	                    if(touchkey_area[i].id == id)
						{
							QT602240_TRACE("find pressing touch key: key(%d), cur_id(%d), prs_id(%d)\n", touchkey_area[i].key, id, touchkey_area[i].id);

							if(	touchkey_area[i].x1 > x ||
								touchkey_area[i].x2 < x ||
								touchkey_area[i].y1 > y ||
								touchkey_area[i].y2 < y )
							{
		                        touchkey_area[i].id = 0xFF;
								qt_key_event_touch(touchkey_area[i].key,  RELEASED);
							}
						}
					}

					// if moved from touch screen area
					if(fingerInfo[id].pressure > 0 && qt602240_pdata->ts_y_scrn_max > fingerInfo[id].y )
					{
						QT602240_TRACE("finger[%d] Move (%d,%d), over screen max, release event generated\n", id, x, y);

						// generate release action
						bChangeUpDn= 1;
						fingerInfo[id].size_id = 0;
						fingerInfo[id].pressure = 0;
					}
					else
					{
						QT602240_TRACE("finger[%d] Move (%d,%d), over screen max, ignored\n", id, x, y);

						// put invalid pressure value;
						fingerInfo[id].size_id = 0;
						fingerInfo[id].pressure = -1;
						fingerInfo[id].x = (int16_t)x;
						fingerInfo[id].y = (int16_t)y;
					}
				}
				else
	#endif
				{
	#ifdef LG_FW_TOUCH_SOFT_KEY
					if (qt602240_pdata->ts_y_scrn_max <= fingerInfo[id].y)
					{
						QT602240_TRACE("finger[%d] Move (%d,%d), moved from touch key area, pressed event generated\n", id, x, y);
					#ifndef _SUPPORT_TOUCH_AMPLITUDE_
						fingerInfo[id].pressure = 40;
					#endif
					}
					else
	#endif
					{
						QT602240_TRACE("finger[%d] Move (%d,%d)\n", (int)id, (int)x, (int)y);
					}

#if defined(MOVE_EVENT_WA)
					if(abs(fingerInfo[id].x - x) < MOV_LIMIT && abs(fingerInfo[id].y - y) < MOV_LIMIT)
					{
						mov_limit_cnt++;
						if(mov_limit_cnt > MAX_MOV_CNT)
						{
							QT602240_ERROR("move drop: %d, %d\n", (int16_t)x, (int16_t)y);
#if defined(USE_THREADED_IRQ)
#else
							enable_irq(qt602240->client->irq);
#endif
							return;
						}
						else
						{
							fingerInfo[id].x = (int16_t)x;
							fingerInfo[id].y = (int16_t)y;
						}
					}
					else
					{
						mov_limit_cnt = 0;
					fingerInfo[id].x = (int16_t)x;
					fingerInfo[id].y = (int16_t)y;
				}
#else
					fingerInfo[id].x = (int16_t)x;
					fingerInfo[id].y = (int16_t)y;
#endif					
				}
			}
#else			 
			if( ((quantum_msg[1] & 0x80) == 0x80 ) && ((quantum_msg[1] & 0x40) == 0x40) )    // case 11000000 -> DETECT & PRESS
			{
				press = 1;
				btn_report = 1;
				print_msg();
				//input_report_key(qt602240->input_dev, BTN_TOUCH, 1);
			}
			else if( ((quantum_msg[1] & 0x80) == 0x80 ) && ((quantum_msg[1] & 0x10) == 0x10) )    // case 10010000 -> DETECT & MOVE
			{
				press = 1;
				print_msg();

			}	

			else if( ((quantum_msg[1] & 0x20 ) == 0x20))   // case 00100000 -> RELEASE
			{
				press = 0;
				print_msg();
				input_report_key(qt602240->input_dev, BTN_TOUCH, 0);
			}
#endif			
			else
			{
				press = 3;
				QT602240_TRACE("unknown state: 0x%x, 0x%x\n ", quantum_msg[0], quantum_msg[1]);
				//print_msg();
			}

			if ( ((quantum_msg[1] & 0xe0 ) == 0xe0)) 
				qt_touch_num_state[id]++;

			ret_val = MESSAGE_READ_OK;
		}
	
		else
			QT602240_ERROR("read_mem is failed, check your TSP chip\n");
	}
#ifdef _SUPPORT_MULTITOUCH_
		if ( nPrevID >= id || bChangeUpDn )
		{
	//		amplitude= 0;
			for ( i= 0; i<MAX_USING_FINGER_NUM; ++i )
			{
				if ( fingerInfo[i].pressure == -1 ) continue;
				input_report_abs(qt602240->input_dev, ABS_MT_POSITION_X, fingerInfo[i].x);
				input_report_abs(qt602240->input_dev, ABS_MT_POSITION_Y, fingerInfo[i].y);
				input_report_abs(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, fingerInfo[i].pressure);	// 0이면 Release, 아니면 Press 상태(Down or Move)
				input_report_abs(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, fingerInfo[i].size_id);	// (ID<<8) | Size
				input_mt_sync(qt602240->input_dev);
				
	//			amplitude++;
				QT602240_TRACE("x: %3d, y:%3d\n", fingerInfo[i].x, fingerInfo[i].y);
				QT602240_TRACE("ID[%d]: %s\n", (fingerInfo[i].size_id>>8), (fingerInfo[i].pressure == 0)?"Up ":"");
	
				if ( fingerInfo[i].pressure == 0 ) fingerInfo[i].pressure= -1;
				else if( fingerInfo[i].pressure > 0 ) one_touch_input_flag++;//hugh 0312
			}
			input_sync(qt602240->input_dev);
			//QT602240_TRACE("multi-touch event[%d] Done\n", amplitude);
		}
		nPrevID= id;
#else

	if(press == 1)
	{
		input_report_abs(qt602240->input_dev, ABS_X, x);
		input_report_abs(qt602240->input_dev, ABS_Y, y);
		if( btn_report == 1)
			input_report_key(qt602240->input_dev, BTN_TOUCH, 1);

		input_sync(qt602240->input_dev);
		amplitude = quantum_msg[6];
		QT602240_TRACE("x: %3d, y: %3d, BTN: %d, size: %d, amp: %d\n", x, y, press, size, amplitude);
	}
	else if(press == 0)
	{
		input_sync(qt602240->input_dev);
		amplitude = quantum_msg[6];
		QT602240_TRACE("x: %3d, y: %3d, BTN: %d, size: %d, amp: %d\n", x, y, press, size, amplitude);
	}
#endif

	// 20100217 julia
	if(touch_message_flag && (cal_check_flag/*==2*/))
	{
		check_chip_calibration(one_touch_input_flag);
	}
	//20100913 sooo.shin
	if(force_cal_check_flag)
	{
		force_cal_check_flag = 0;
		calibrate_chip();
		
	}	
	
#if defined(USE_THREADED_IRQ)
#else
	enable_irq(qt602240->client->irq);
#endif
	return ;
}




/*------------------------------ I2C Driver block -----------------------------------*/



#define I2C_M_WR 0 /* for i2c */
#define I2C_MAX_SEND_LENGTH     300
int qt602240_i2c_write(u16 reg, u8 *read_val, unsigned int len)
{
	struct i2c_msg wmsg;
	//unsigned char wbuf[3];
	unsigned char data[I2C_MAX_SEND_LENGTH];
	int ret,i;

	address_pointer = reg;

	if(len+2 > I2C_MAX_SEND_LENGTH)
	{
		QT602240_ERROR("data length error: %d\n", len);
		return -ENODEV;
	}

	wmsg.addr = qt602240->client->addr;
	wmsg.flags = I2C_M_WR;
	wmsg.len = len + 2;
	wmsg.buf = data;

	data[0] = reg & 0x00ff;
	data[1] = reg >> 8;

	for (i = 0; i < len; i++)
	{
		data[i+2] = *(read_val+i);
	}

	//QT602240_TRACE("%s\n",wbuf);
	ret = i2c_transfer(qt602240->client->adapter, &wmsg, 1);
	return ret;
}

int boot_qt602240_i2c_write(u16 reg, u8 *read_val, unsigned int len)
{
	struct i2c_msg wmsg;
	//unsigned char wbuf[3];
	unsigned char data[I2C_MAX_SEND_LENGTH];
	int ret,i;

	if(len+2 > I2C_MAX_SEND_LENGTH)
	{
		QT602240_ERROR("data length error: %d\n", len);
		return -ENODEV;
	}

	wmsg.addr = 0x24;
	wmsg.flags = I2C_M_WR;
	wmsg.len = len;
	wmsg.buf = data;


	for (i = 0; i < len; i++)
	{
		data[i] = *(read_val+i);
	}

	//QT602240_TRACE("%s\n", wbuf);
	ret = i2c_transfer(qt602240->client->adapter, &wmsg, 1);
	return ret;
}


int qt602240_i2c_read(u16 reg,unsigned char *rbuf, int buf_size)
{
	static unsigned char first_read=1;
	struct i2c_msg rmsg;
	int ret;
	unsigned char data[2];

	rmsg.addr = qt602240->client->addr;

    if(first_read == 1)
	{
		first_read = 0;
		address_pointer = reg+1;
	}

	if((address_pointer != reg) || (reg != message_processor_address))
	{
		address_pointer = reg;

		rmsg.flags = I2C_M_WR;
		rmsg.len = 2;
		rmsg.buf = data;
		data[0] = reg & 0x00ff;
		data[1] = reg >> 8;
		ret = i2c_transfer(qt602240->client->adapter, &rmsg, 1);
	}
	//QT602240_TRACE("%d, %d\n",data[0], data[1]);

	//if(ret>=0) {
		rmsg.flags = I2C_M_RD;
		rmsg.len = buf_size;
		rmsg.buf = rbuf;
		ret = i2c_transfer(qt602240->client->adapter, &rmsg, 1);
	//}


	//QT602240_TRACE("%d\n", rbuf);
	return ret;
}


/*! \brief Enables pin change interrupts on falling edge. */
void enable_changeline_int(void)
{
#if 0 /* TODO */
	set_touch_irq_gpio_init();
	enable_irq_handler();

#endif
}

/*! \brief Returns the changeline state. */
U8 read_changeline(void)
{
#if 0 /* TODO */
	return get_touch_irq_gpio_value();
#endif
	return 0;

}

/*! \brief Maxtouch Memory read by I2C bus */
U8 read_mem(U16 start, U8 size, U8 *mem)
{
	int ret;

	//   read_buf = (char *)kmalloc(size, GFP_KERNEL | GFP_ATOMIC);
	memset(mem,0xFF,size);
	ret = qt602240_i2c_read(start,mem,size);
	if(ret < 0) {
		QT602240_ERROR("i2c read failed: ret: %d, start: 0x%x, size: %d\n", ret, start, size);
		return(READ_MEM_FAILED);
	} 

	return(READ_MEM_OK);
}

U8 boot_read_mem(U16 start, U8 size, U8 *mem)
{
	struct i2c_msg rmsg;
	int ret;

	rmsg.addr=0x24;
	rmsg.flags = I2C_M_RD;
	rmsg.len = size;
	rmsg.buf = mem;
	ret = i2c_transfer(qt602240->client->adapter, &rmsg, 1);
	
	return ret;
}

U8 read_U16(U16 start, U16 *mem)
{
	U8 status;

	status = read_mem(start, 2, (U8 *) mem);

	return (status);
}

U8 write_mem(U16 start, U8 size, U8 *mem)
{
	int ret;

	ret = qt602240_i2c_write(start,mem,size);
	if(ret < 0) {
        QT602240_ERROR("write_mem fail: %d\n", ret); 
		return(WRITE_MEM_FAILED);
	}
	else
		return(WRITE_MEM_OK);
}

U8 boot_write_mem(U16 start, U16 size, U8 *mem)
{
	int ret;

	ret = boot_qt602240_i2c_write(start,mem,size);
	if(ret < 0){
		QT602240_ERROR("boot write mem fail: %d \n", ret);
		return(WRITE_MEM_FAILED);
	}
	else
		return(WRITE_MEM_OK);
}




/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*
* ***************************************************************************/

void write_message_to_usart(uint8_t msg[], uint8_t length)
{
    int i;
    for (i=0; i < length; i++)
    {
        QT602240_TRACE("0x%02x\n", msg[i]);
    }
}

/*
 * Message handler that is called by the touch chip driver when messages
 * are received.
 * 
 * This example message handler simply prints the messages as they are
 * received to USART1 port of EVK1011 board.
 */
void message_handler(U8 *msg, U8 length)
{  
	//usart_write_line(&AVR32_USART1, "Touch IC message: ");
	//write_message_to_usart(msg, length);
	//usart_write_line(&AVR32_USART1, "\n");
}


irqreturn_t qt602240_irq_handler(int irq, void *dev_id)
{
#if defined(USE_THREADED_IRQ)
	disable_irq_nosync(qt602240->client->irq);
	get_message();
	enable_irq(qt602240->client->irq);
#else
	disable_irq_nosync(qt602240->client->irq);

	//QT602240_TRACE("start\n");
#if 0
	/* Check touch GPIO_INT irq */
	if (!(readl( gpio_mask_mem + 0x100) & 0x20))
		return IRQ_HANDLED;

	writel( readl( gpio_mask_mem + 0x100) | 0x20, gpio_mask_mem + 0x100);		//	pending	touch GPIO_INT
#endif
	queue_work(qt602240_wq, &qt602240->ts_event_work);
#endif
	return IRQ_HANDLED;
}

int qt602240_power_onoff(char onoff)
{
//	struct mcs6000_ts_device *dev = NULL;
	int ret = 0;

    QT602240_TRACE("start: %d\n", onoff);
	ret = qt602240->power(onoff);
	if(ret < 0)	{
		QT602240_ERROR("failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}
int qt602240_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	int err = 0;
	int ret;	
//	int i;

	QT602240_TRACE("start\n");

	QT602240_TRACE("+-----------------------------------------+\n");
	QT602240_TRACE("|  Quantum Touch Driver Probe!            |\n");
	QT602240_TRACE("+-----------------------------------------+\n");

#if defined(USE_THREADED_IRQ)
#else
	INIT_WORK(&qt602240->ts_event_work, get_message );
#endif

	qt602240_pdata = client->dev.platform_data;

	qt602240->client = client;

	qt602240->power = qt602240_pdata->power;
	qt602240->num_irq = qt602240_pdata->irq;        //(92)
	qt602240->intr_gpio = qt602240_pdata->gpio_int;
	qt602240->hw_i2c =  qt602240_pdata->hw_i2c;
	qt602240->sda_gpio = qt602240_pdata->sda;
	qt602240->scl_gpio  = qt602240_pdata->scl;
    //qt602240->ce_gpio  = qt602240_pdata->ce;
	qt602240->touch_key = qt602240_pdata->touch_key;
	qt602240->client->irq = qt602240_pdata->irq;

	gpio_tlmm_config(GPIO_CFG(qt602240->intr_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

#ifdef CONFIG_MAINTOUCH_CE
	if (gpio_is_valid(qt602240->ce_gpio)) {
		if (gpio_request(qt602240->ce_gpio, "maintouch_ce"))
			QT602240_ERROR("failed to request GPIO_TOUCH_EN\n");
		gpio_direction_output(qt602240->ce_gpio, 1);
	}
	gpio_free(qt602240->ce_gpio);
#endif

	QT602240_TRACE("qt602240 GPIO Status\n");

#ifdef CONFIG_MAINTOUCH_CE
	QT602240_TRACE("TOUCH_EN: %s\n", gpio_get_value(qt602240->ce_gpio)? "High":"Low");
#endif
	QT602240_TRACE("TOUCH_INT: %s\n", gpio_get_value(qt602240->intr_gpio)? "High":"Low");

	msleep(100);

	qt602240->input_dev = input_allocate_device();
	if (qt602240->input_dev == NULL) {
		ret = -ENOMEM;
		QT602240_ERROR("failed to allocate input device\n");
		goto err_input_dev_alloc_failed;
	}
	qt602240->input_dev->name = "qt602240_ts_input";
	set_bit(EV_SYN, qt602240->input_dev->evbit);
	set_bit(EV_KEY, qt602240->input_dev->evbit);
	if(qt602240->touch_key == 1)
	{
		set_bit(KEY_MENU, qt602240->input_dev->keybit);
		set_bit(KEY_HOME, qt602240->input_dev->keybit);
		set_bit(KEY_BACK, qt602240->input_dev->keybit);
		//set_bit(KEY_SEARCH, qt602240->input_dev->keybit);
	}	
	set_bit(EV_ABS, qt602240->input_dev->evbit);

	input_set_abs_params(qt602240->input_dev, ABS_X, qt602240_pdata->ts_x_min, qt602240_pdata->ts_x_max, 0, 0);
	input_set_abs_params(qt602240->input_dev, ABS_Y, qt602240_pdata->ts_y_min, qt602240_pdata->ts_y_max, 0, 0);
#ifdef _SUPPORT_MULTITOUCH_
	input_set_abs_params(qt602240->input_dev, ABS_MT_POSITION_X, qt602240_pdata->ts_x_min, qt602240_pdata->ts_x_max, 0, 0);
	input_set_abs_params(qt602240->input_dev, ABS_MT_POSITION_Y, qt602240_pdata->ts_y_min, qt602240_pdata->ts_y_max, 0, 0);
#endif

	input_set_abs_params(qt602240->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	input_set_abs_params(qt602240->input_dev, ABS_TOOL_WIDTH, 0, 15, 0, 0);
#ifdef _SUPPORT_MULTITOUCH_
	input_set_abs_params(qt602240->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(qt602240->input_dev, ABS_MT_WIDTH_MAJOR, 0, 30, 0, 0);
#endif	

	ret = input_register_device(qt602240->input_dev);
	if (ret) {
		QT602240_ERROR("unable to register %s input device\n", qt602240->input_dev->name);
		goto err_input_register_device_failed;
	}

	i2c_set_clientdata(client, qt602240);

	if (!(err = i2c_check_functionality(client->adapter, I2C_FUNC_I2C))) {
		QT602240_ERROR("fucntionality check failed: %d\n", err);
		return err;
	}

	disable_irq(qt602240->client->irq);
	qt_ts_on();// [LGE PATCH] edward1.kim@lge.com 20110216  
	enable_irq(qt602240->client->irq);

#ifdef QT_FIRMUP_ENABLE
{
	QT_reprogram();
}
#else
	quantum_touch_probe();
#endif

	err = gpio_request(qt602240->intr_gpio, "ts_int");
	if (err < 0) {
		QT602240_ERROR("gpio request fail: %d\n", err);
		return err;
	}
		
	err = gpio_direction_input(qt602240->intr_gpio);
	if (err < 0) {
		QT602240_ERROR("gpio input direction fail: %d\n", err);
		return err;
	}

#if defined(USE_THREADED_IRQ)
	ret = request_threaded_irq(qt602240->client->irq, NULL, qt602240_irq_handler, IRQF_TRIGGER_LOW|IRQF_ONESHOT, "qt602240 irq", qt602240);
#else
	ret = request_irq(qt602240->client->irq, qt602240_irq_handler, IRQF_DISABLED, "qt602240 irq", qt602240);
#endif

	if (ret == 0) {
		QT602240_TRACE("start touchscreen %s\n", qt602240->input_dev->name);
	}
	else {
		QT602240_ERROR("request_irq failed: %d\n", ret);
	}

	//	enable_irq(qt602240->client->irq);
#ifdef USE_TSP_EARLY_SUSPEND
	qt602240->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	qt602240->early_suspend.suspend = qt602240_early_suspend;
	qt602240->early_suspend.resume = qt602240_late_resume;
	register_early_suspend(&qt602240->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	qt_initial_ok = 1;
	return 0;

err_input_register_device_failed:
	input_free_device(qt602240->input_dev);

err_input_dev_alloc_failed:
	kfree(qt602240);
	return ret;
}

//int qt602240_remove(struct i2c_client *client)
static int qt602240_remove(struct i2c_client *client)
{
	struct qt602240_data *data = i2c_get_clientdata(client);
#ifdef USE_TSP_EARLY_SUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif	/* CONFIG_HAS_EARLYSUSPEND */

	free_irq(data->num_irq, data);
	free_irq(qt602240->client->irq, 0);
	input_unregister_device(qt602240->input_dev);
	kfree(qt602240);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifndef USE_TSP_EARLY_SUSPEND
static int qt602240_suspend(struct platform_device * dev,pm_message_t mesg)
{
	gen_powerconfig_t7_config_t power_config_sleep = {0};
	int i=0;

	//disable_irq(qt602240->client->irq);

	/* Set power config. */
	/* Set Idle Acquisition Interval to 32 ms. */
	power_config_sleep.idleacqint = 0;
	/* Set Active Acquisition Interval to 16 ms. */
	power_config_sleep.actvacqint = 0;

	/* Write power config to chip. */
	if (write_power_config(power_config_sleep) != CFG_WRITE_OK)
	{
		/* "Power config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
	}

#ifdef _SUPPORT_MULTITOUCH_
	for (i=0; i<MAX_USING_FINGER_NUM ; i++)
		fingerInfo[i].pressure = -1;
#endif

	return 0;
}

static int qt602240_resume(struct platform_device *dev)
{
	int ret,i;

	QT602240_TRACE("start\n");
	if ( (ret = write_power_config(power_config)) != CFG_WRITE_OK)
	{
		/* "Power config write failed!\n" */
		for(i=0;i<10;i++) {
			QT602240_ERROR("config error: %d, i: %d\n",ret, i);
			msleep(20);
			if ( (ret = write_power_config(power_config)) == CFG_WRITE_OK)
				break;
		}
		if( i == 10)
			return -1;
	}

	//hugh 0312
	good_check_flag=0;

	calibrate_chip();
	//enable_irq(qt602240->client->irq);

	return 0;
}
#endif

#ifdef USE_TSP_EARLY_SUSPEND
static void qt602240_early_suspend(struct early_suspend *h)
{
	gen_powerconfig_t7_config_t power_config_sleep = {0};
	int i=0;

	QT602240_TRACE("start\n");
	//disable_irq(qt602240->client->irq);

	/* Set power config. */
	/* Set Idle Acquisition Interval to 32 ms. */
	power_config_sleep.idleacqint = 0;
	/* Set Active Acquisition Interval to 16 ms. */
	power_config_sleep.actvacqint = 0;

	/* Write power config to chip. */
	if (write_power_config(power_config_sleep) != CFG_WRITE_OK)
	{
		/* "Power config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
	}

#ifdef _SUPPORT_MULTITOUCH_
	for (i=0; i<MAX_USING_FINGER_NUM ; i++){
		fingerInfo[i].pressure = -1;
		qt_touch_num_state[i]=0;
		}
	touch_state_val=0;
#endif
	qt_timer_state=0;

	qt602240 ->power(OFF);// [LGE PATCH] edward1.kim@lge.com 20110216  
}

static void qt602240_late_resume(struct early_suspend *h)
{
	int ret,i;

	qt_ts_on(); // [LGE PATCH] edward1.kim@lge.com 20110216  
	msleep(20);

	QT602240_TRACE("start\n");
	if ( (ret = write_power_config(power_config)) != CFG_WRITE_OK)
	{
		/* "Power config write failed!\n" */
		for(i=0;i<10;i++) {
			QT602240_ERROR("config error: %d, i:%d\n", ret, i);
			msleep(20);
			if ( (ret = write_power_config(power_config)) == CFG_WRITE_OK)
				break;
		}
		if( i >= 10)
			return;
	}
	//hugh 0312
	good_check_flag=0;
	is_inputmethod = 0; // kmj_DC31	
	cal_check_flag = 0;	
	
	calibrate_chip();
	//enable_irq(qt602240->client->irq);
}
#endif	// End of USE_TSP_EARLY_SUSPEND

static struct i2c_device_id qt602240_idtable[] = {
	{ "qt602240_ts", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, qt602240_idtable);

struct i2c_driver __refdata qt602240_i2c_driver = {
	.driver = {
		.owner  = THIS_MODULE,
		.name	= "qt602240_ts",
	},
	.id_table	= qt602240_idtable,
	.probe		= qt602240_probe,
	.remove		= __devexit_p(qt602240_remove),
#ifndef USE_TSP_EARLY_SUSPEND
	.suspend	= qt602240_suspend,
	.resume		= qt602240_resume,
#endif //USE_TSP_EARLY_SUSPEND
};

struct class *sec_class;
struct device *ts_dev;

static ssize_t trace_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("qt602240_trace_enable: %d\n", qt602240_trace_enable);

	return sprintf(buf, "%d\n", qt602240_trace_enable);
}

static ssize_t trace_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	qt602240_trace_enable = value;

	QT602240_TRACE("qt602240_trace_enable: %d\n", qt602240_trace_enable);

	return size;
}

static ssize_t i2c_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	int ret;
	unsigned char read_buf[5];

	ret = qt602240_i2c_read(0,read_buf, 5);
	if (ret < 0) {
		QT602240_ERROR("read failed: %d\n", ret);
	}
	QT602240_TRACE("%x, %x, %x, %x, %x\n", read_buf[0], read_buf[1], read_buf[2], read_buf[3], read_buf[4]);

	return sprintf(buf, "%s\n", buf);
}

static ssize_t i2c_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	return size;
}

static ssize_t gpio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("qt602240 GPIO Status\n");
#ifdef CONFIG_MAINTOUCH_CE
	QT602240_TRACE("TOUCH_EN: %s\n", gpio_get_value(qt602240->ce_gpio)? "High":"Low");
#endif
	QT602240_TRACE("TOUCH_INT: %s\n", gpio_get_value(qt602240->intr_gpio)? "High":"Low");

	return sprintf(buf, "%s\n", buf);
}

static ssize_t gpio_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
#ifdef CONFIG_MAINTOUCH_CE
	if(strncmp(buf, "ENHIGH", 6) == 0 || strncmp(buf, "enhigh", 6) == 0) {
		gpio_set_value(qt602240->ce_gpio, 1);
		QT602240_TRACE("set TOUCH_EN High\n");
		mdelay(100);
	}
	if(strncmp(buf, "ENLOW", 5) == 0 || strncmp(buf, "enlow", 5) == 0) {
		gpio_set_value(qt602240->ce_gpio, 0);
		QT602240_TRACE("set TOUCH_EN Low\n");
		mdelay(100);
	}
#endif
	return size;
}

static ssize_t firmware1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char a, b;
	a = tsp_version & 0xf0;
	a = a >> 4;
	b = tsp_version & 0x0f;
	return sprintf(buf, "%d.%d\n", a, b);
}

static ssize_t firmware1_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	return size;
}

static ssize_t key_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	return sprintf(buf, "%d\n", touchscreen_config.tchthr);
}

static ssize_t key_threshold_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int i;
	if(sscanf(buf,"%d",&i)==1)
	{
    		touchscreen_config.tchthr = i;	//basically,40;
		QT602240_TRACE("threshold is changed to %d\n",i);
	}
	else
		QT602240_ERROR("threshold write error\n");

	return 0;
}


#ifdef QT_FIRMUP_ENABLE

uint8_t boot_unlock(void)
{

	int ret;
	unsigned char data[2];

	//   read_buf = (char *)kmalloc(size, GFP_KERNEL | GFP_ATOMIC);
	data[0] = 0xDC;
	data[1] = 0xAA;
	
	ret = boot_qt602240_i2c_write(0,data,2);
	if(ret < 0) {
		QT602240_ERROR("i2c write failed: %d\n", ret);
		return(WRITE_MEM_FAILED);
	} 

	return(WRITE_MEM_OK);

}

void TSP_Restart(void)
{
	
#ifdef CONFIG_MAINTOUCH_CE
	if (gpio_is_valid(qt602240->ce_gpio)) {
		if (gpio_request(qt602240->ce_gpio, "maintouch_ce"))
			QT602240_ERROR("failed to request GPIO_TOUCH_EN\n");
		gpio_set_value(qt602240->ce_gpio, 0);
	}
	gpio_free(qt602240->ce_gpio);

	mdelay(300);

	if (gpio_is_valid(qt602240->ce_gpio)) {
		if (gpio_request(qt602240->ce_gpio, "maintouch_ce"))
			QT602240_ERROR("failed to request GPIO_TOUCH_EN\n");
		gpio_set_value(qt602240->ce_gpio, 1);
	}
	gpio_free(qt602240->ce_gpio);
#endif
	
    
}

uint8_t QT_Boot(void)
{
		unsigned char boot_status;
		unsigned char boot_ver;
		unsigned char retry_cnt;
		unsigned long int character_position;
		unsigned int frame_size=0;
		unsigned int next_frame;
		unsigned int crc_error_count;
		unsigned int size1,size2;
		unsigned int j,read_status_flag;
		uint8_t data = 0xA5;
	
		uint8_t reset_result = 0;
	
		unsigned char  *firmware_data ;
	
		firmware_data = QT602240_firmware;
	
		crc_error_count = 0;
		character_position = 0;
		next_frame = 0;
	
		reset_result = write_mem(command_processor_address + RESET_OFFSET, 1, &data);
	
	
		if(reset_result != WRITE_MEM_OK)
		{
			for(retry_cnt =0; retry_cnt < 3; retry_cnt++)
			{
				mdelay(100);
				reset_result = write_mem(command_processor_address + RESET_OFFSET, 1, &data);
				if(reset_result == WRITE_MEM_OK)
				{
					break;
				}
			}
	
		}
		if (reset_result == WRITE_MEM_OK)
			QT602240_TRACE("Boot reset OK\n");

		mdelay(100);
	
		for(retry_cnt = 0; retry_cnt < 30; retry_cnt++)
		{
			
			if( (boot_read_mem(0,1,&boot_status) == READ_MEM_OK) && (boot_status & 0xC0) == 0xC0) 
			{
				boot_ver = boot_status & 0x3F;
				crc_error_count = 0;
				character_position = 0;
				next_frame = 0;

				while(1)
				{ 
					for(j = 0; j<5; j++)
					{
						mdelay(60);
						if(boot_read_mem(0,1,&boot_status) == READ_MEM_OK)
						{
							read_status_flag = 1;
							break;
						}
						else 
							read_status_flag = 0;
					}
					
					if(read_status_flag==1)	
		//			if(boot_read_mem(0,1,&boot_status) == READ_MEM_OK)
					{
						retry_cnt  = 0;
						QT602240_TRACE("TSP boot status is %x, stage 2\n", boot_status);
						if((boot_status & QT_WAITING_BOOTLOAD_COMMAND) == QT_WAITING_BOOTLOAD_COMMAND)
						{
							if(boot_unlock() == WRITE_MEM_OK)
							{
								mdelay(10);
			
								QT602240_TRACE("unlock OK\n");
			
							}
							else
							{
								QT602240_ERROR("unlock fail\n");
							}
						}
						else if((boot_status & 0xC0) == QT_WAITING_FRAME_DATA)
						{
							 /* Add 2 to frame size, as the CRC bytes are not included */
							size1 =  *(firmware_data+character_position);
							size2 =  *(firmware_data+character_position+1)+2;
							frame_size = (size1<<8) + size2;
			
							QT602240_TRACE("frame size: %d\n", frame_size);
							QT602240_TRACE("firmware pos: %lu\n", character_position);
							/* Exit if frame data size is zero */
							if( 0 == frame_size )
							{
								QT602240_TRACE("frame_size: 0\n");
								return 1;
							}
							next_frame = 1;
							boot_write_mem(0,frame_size, (firmware_data +character_position));
							mdelay(10);
						}
						else if(boot_status == QT_FRAME_CRC_CHECK)
						{
							QT602240_TRACE("CRC Check\n");
						}
						else if(boot_status == QT_FRAME_CRC_PASS)
						{
							if( next_frame == 1)
							{
								QT602240_TRACE("CRC Ok\n");
								character_position += frame_size;
								next_frame = 0;
							}
							else {
								QT602240_TRACE("next_frame != 1\n");
							}
						}
						else if(boot_status  == QT_FRAME_CRC_FAIL)
						{
							QT602240_ERROR("CRC Fail\n");
							crc_error_count++;
						}
						if(crc_error_count > 10)
						{
							return QT_FRAME_CRC_FAIL;
						}
			
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				QT602240_ERROR("read_boot_state() or (boot_status & 0xC0) == 0xC0) is fail\n");
			}
		}
		
		QT602240_TRACE("end\n");
		return 0;
}

uint8_t QT_Boot_no_reset(void)
{
	unsigned char boot_status;
	unsigned char boot_ver;
	unsigned char retry_cnt;
	unsigned short character_position;
	unsigned short frame_size = 0;
	unsigned short next_frame;
	unsigned short crc_error_count;
	unsigned char size1,size2;
	unsigned short j,read_status_flag;
//	uint8_t data = 0xA5;

	unsigned char  *firmware_data ;

	firmware_data = QT602240_firmware;

	crc_error_count = 0;
	character_position = 0;
	next_frame = 0;

		for(retry_cnt = 0; retry_cnt < 30; retry_cnt++)
		{
			
			if( (boot_read_mem(0,1,&boot_status) == READ_MEM_OK) && (boot_status & 0xC0) == 0xC0) 
			{
				boot_ver = boot_status & 0x3F;
				crc_error_count = 0;
				character_position = 0;
				next_frame = 0;

				while(1)
				{ 
					for(j = 0; j<5; j++)
					{
						mdelay(60);
						if(boot_read_mem(0,1,&boot_status) == READ_MEM_OK)
						{
							read_status_flag = 1;
							break;
						}
						else 
							read_status_flag = 0;
					}
					
					if(read_status_flag==1)	
					{
						retry_cnt  = 0;
						QT602240_TRACE("TSP boot status is %x, stage 2 \n", boot_status);
						if((boot_status & QT_WAITING_BOOTLOAD_COMMAND) == QT_WAITING_BOOTLOAD_COMMAND)
						{
							if(boot_unlock() == WRITE_MEM_OK)
							{
								mdelay(10);
			
								QT602240_TRACE("unlock OK\n");
			
							}
							else
							{
								QT602240_ERROR("unlock fail\n");
							}
						}
						else if((boot_status & 0xC0) == QT_WAITING_FRAME_DATA)
						{
							 /* Add 2 to frame size, as the CRC bytes are not included */
							size1 =  *(firmware_data+character_position);
							size2 =  *(firmware_data+character_position+1)+2;
							frame_size = (size1<<8) + size2;
			
							QT602240_TRACE("frame size: %d\n", frame_size);
							QT602240_TRACE("firmware pos: %d\n", character_position);
							/* Exit if frame data size is zero */
							if( 0 == frame_size )
							{
								QT602240_TRACE("frame_size: 0\n");
								return 1;
							}
							next_frame = 1;
							boot_write_mem(0,frame_size, (firmware_data +character_position));
							mdelay(10);
						}
						else if(boot_status == QT_FRAME_CRC_CHECK)
						{
							QT602240_TRACE("CRC Check\n");
						}
						else if(boot_status == QT_FRAME_CRC_PASS)
						{
							if( next_frame == 1)
							{
								QT602240_TRACE("CRC Ok\n");
								character_position += frame_size;
								next_frame = 0;
							}
							else {
								QT602240_TRACE("next_frame != 1\n");
							}
						}
						else if(boot_status  == QT_FRAME_CRC_FAIL)
						{
							QT602240_ERROR("CRC Fail\n");
							crc_error_count++;
						}
						if(crc_error_count > 10)
						{
							return QT_FRAME_CRC_FAIL;
						}
			
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				QT602240_TRACE("read_boot_state() or (boot_status & 0xC0) == 0xC0) is fail\n");
			}
		}
		
		QT602240_TRACE("end\n");
		return 0;

}



//*****************************************************************************
//
// 
//
//*****************************************************************************

void QT_reprogram(void)
{
	uint8_t version, build;
//	unsigned char rxdata;


	QT602240_TRACE("check\n");

#ifdef QT_BOOTLOADER_CHECK
	if(boot_read_mem(0,1,&rxdata) == READ_MEM_OK)
	{
		QT602240_TRACE("enter to new firmware: boot mode\n");
	        if(QT_Boot_no_reset())
	            TSP_Restart();
	        QT602240_TRACE("reprogram done: boot mode\n");
	}
#endif

	quantum_touch_probe();  /* find and initialise QT device */

	get_version(&version);
	get_build_number(&build);

	if(((version != 0x14)&&(version < 0x16)) ||((version == 0x16)&&(build == 0xAA)))
	{
	        QT602240_TRACE("enter to new firmware: addr = other version\n");
	        if(!QT_Boot())
	        {
	            TSP_Restart();
	            quantum_touch_probe();
	        }
	        QT602240_TRACE("reprogram done: addr = other version\n");
	}

}

#endif

static ssize_t setup_show(struct device *dev, struct device_attribute *attr, char *buf)
{	
	QT602240_TRACE("qt602240 setup status\n");

	return 0;
}

static ssize_t setup_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	return size;
}

static ssize_t firmware_show(struct device *dev, struct device_attribute *attr, char *buf)
{	// v1.2 = 18 , v1.4 = 20 , v1.5 = 21
	uint8_t version, build;
	
	get_version(&version);
	get_build_number(&build);
	
	QT602240_TRACE("QT602240 firmware ver.\n");
	QT602240_TRACE("version = %x\n", version);
	QT602240_TRACE("build = %x\n", build);

    return sprintf(buf, "0X%x", version );
}


static ssize_t firmware_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	char *after;
	int firmware_ret_val = -1;
	uint8_t version, build;
	unsigned long value = simple_strtoul(buf, &after, 10);	


	get_version(&version);
	get_build_number(&build);

	QT602240_TRACE("start\n");

	if ( value == 1 )	// auto update.
	{
		QT602240_TRACE("firmware update start\n" );
		QT602240_TRACE("version 0x%x\n", tsp_version);


		if(((version != 0x14)&&(version <0x16))||((version==0x16)&&(build==0xaa)))
		{			
			wake_lock(&tsp_firmware_wake_lock);
			
		        QT602240_TRACE("enter to new firmware: addr = other version\n");

			firmware_ret_val = QT_Boot();
		        if(!firmware_ret_val)
		        {
		            quantum_touch_probe();
		        }
		        QT602240_TRACE("reprogram done: addr = other version\n");
				
			wake_unlock(&tsp_firmware_wake_lock);			
		}	
		else
		{
			firmware_ret_val = 1; 
		}
		QT602240_TRACE("firmware result: %d\n", firmware_ret_val);

	}
  
  return size;  
}


static DEVICE_ATTR(trace, 0664, trace_show, trace_store);
static DEVICE_ATTR(gpio, 0664, gpio_show, gpio_store);
static DEVICE_ATTR(i2c, 0664, i2c_show, i2c_store);
static DEVICE_ATTR(setup, 0664, setup_show, setup_store);
static DEVICE_ATTR(firmware, 0664, firmware_show, firmware_store);
static DEVICE_ATTR(firmware1, 0664, firmware1_show, firmware1_store);
static DEVICE_ATTR(key_threshold, 0664, key_threshold_show, key_threshold_store);


/*------------------------------ for tunning ATmel - start ----------------------------*/
/*
   power_config.idleacqint = 32;      // 0
   power_config.actvacqint = 16;      // 1
   power_config.actv2idleto = 50;     // 2 
   */
static ssize_t set_power_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_power_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );

	if (cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, power_config.idleacqint: %d\n",config_value);
		power_config.idleacqint = config_value;
	}		
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, power_config.actvacqint: %d\n", config_value);
		power_config.actvacqint = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, power_config.actv2idleto: %d\n", config_value);
		power_config.actv2idleto = config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

	if (write_power_config(power_config) != CFG_WRITE_OK)
    {
        QT602240_TRACE("configuration fail\n");
    }

	return size;
}
static DEVICE_ATTR(set_power, 0664, set_power_show, set_power_store);



/*
   acquisition_config.atchdrift = 5;
   acquisition_config.chrgtime = 8;
   acquisition_config.driftst = 20;
   acquisition_config.sync = 0;
   acquisition_config.tchautocal = 0;
   acquisition_config.tchdrift = 20;
   */
static ssize_t set_acquisition_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_acquisition_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );

	if (cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, acquisition_config.chrgtime: %d\n", config_value);
		acquisition_config.chrgtime = config_value;
	}		
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, acquisition_config.tchdrift: %d\n", config_value);
		acquisition_config.tchdrift = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, acquisition_config.driftst: %d\n", config_value);
		acquisition_config.driftst = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, acquisition_config.tchautocal: %d\n", config_value);
		acquisition_config.tchautocal= config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, acquisition_config.sync: %d\n", config_value);
		acquisition_config.sync = config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, acquisition_config.atchcalst: %d\n", config_value);
		acquisition_config.atchcalst = config_value;
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, acquisition_config.atchcalsthr: %d\n", config_value);
		acquisition_config.atchcalsthr = config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

	if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }

	return size;
}
static DEVICE_ATTR(set_acquisition, 0664, set_acquisition_show, set_acquisition_store);

/* Set touchscreen config. 
   touchscreen_config.ctrl = 131;
   touchscreen_config.xorigin = 0;
   touchscreen_config.yorigin = 0;
   touchscreen_config.xsize = 15;
   touchscreen_config.ysize = 11;
   touchscreen_config.akscfg = 0;
   touchscreen_config.blen = 65;
   touchscreen_config.tchthr = 50;
   touchscreen_config.tchdi = 2;
   touchscreen_config.orientate = 0;
   touchscreen_config.mrgtimeout = 0;
   touchscreen_config.movhysti = 1;
   touchscreen_config.movhystn = 1;
   touchscreen_config.movfilter = 0;
   touchscreen_config.numtouch= 1;
   touchscreen_config.mrghyst = 10;
	touchscreen_config.mrgthr = 5;
   touchscreen_config.tchamphyst = 10;
   touchscreen_config.xres = 0;
   touchscreen_config.yres = 0;
   touchscreen_config.xloclip = 0;
   touchscreen_config.xhiclip = 0;
   touchscreen_config.yloclip = 0;
   touchscreen_config.yhiclip = 0; */
static ssize_t set_touchscreen_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_touchscreen_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int  cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );

	if (cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, touchscreen_config.ctrl: %d\n", config_value);
		touchscreen_config.ctrl = config_value;
	}		
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, touchscreen_config.xorigin: %d\n", config_value);
		touchscreen_config.xorigin = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, touchscreen_config.yorigin: %d\n", config_value);
		touchscreen_config.yorigin  = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, touchscreen_config.xsize: %d\n", config_value);
		touchscreen_config.xsize =  config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, touchscreen_config.ysize: %d\n", config_value);
		touchscreen_config.ysize =  config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, touchscreen_config.akscfg: %d\n", config_value);
		touchscreen_config.akscfg = config_value;
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, touchscreen_config.blen: %d\n", config_value);
		touchscreen_config.blen = config_value;
	}
	else if(cmd_no == 7)
	{
		QT602240_TRACE("CMD 7, touchscreen_config.tchthr: %d\n", config_value);
		touchscreen_config.tchthr = config_value;
	}
	else if(cmd_no == 8)
	{
		QT602240_TRACE("CMD 8, touchscreen_config.tchdi: %d\n", config_value);
		touchscreen_config.tchdi= config_value;
	}
	else if(cmd_no == 9)
	{
		QT602240_TRACE("CMD 9, touchscreen_config.orient: %d\n", config_value);
		touchscreen_config.orient = config_value;
	}
	else if(cmd_no == 10)
	{
		QT602240_TRACE("CMD 10, touchscreen_config.mrgtimeout: %d\n", config_value);
		touchscreen_config.mrgtimeout = config_value;
	}
	else if(cmd_no == 11)
	{
		QT602240_TRACE("CMD 11, touchscreen_config.movhysti: %d\n", config_value);
		touchscreen_config.movhysti = config_value;
	}
	else if(cmd_no == 12)
	{
		QT602240_TRACE("CMD 12, touchscreen_config.movhystn: %d\n", config_value);
		touchscreen_config.movhystn = config_value;
	}
	else if(cmd_no == 13)
	{
		QT602240_TRACE("CMD 13, touchscreen_config.movfilter: %d\n", config_value);
		touchscreen_config.movfilter = config_value;
	}
	else if(cmd_no == 14)
	{
		QT602240_TRACE("CMD 14, touchscreen_config.numtouch: %d\n", config_value);
		touchscreen_config.numtouch = config_value;
	}
	else if(cmd_no == 15)
	{
		QT602240_TRACE("CMD 15, touchscreen_config.mrghyst: %d\n", config_value);
		touchscreen_config.mrghyst = config_value;
	}
	else if(cmd_no == 16)
	{
		QT602240_TRACE("CMD 16, touchscreen_config.mrgthr: %d\n", config_value);
		touchscreen_config.mrgthr = config_value;
	}
	else if(cmd_no == 17)
	{
		QT602240_TRACE("CMD 17, touchscreen_config.tchamphyst: %d\n", config_value);
		touchscreen_config.amphyst = config_value;
	}
	else if(cmd_no == 18)
	{
		QT602240_TRACE("CMD 18, touchscreen_config.xrange: %d\n", config_value);
		touchscreen_config.xrange= config_value;
	}
	else if(cmd_no == 19)
	{
		QT602240_TRACE("CMD 19, touchscreen_config.yrange: %d\n", config_value);
		touchscreen_config.yrange= config_value;
	}
	else if(cmd_no == 20)
	{
		QT602240_TRACE("CMD 20, touchscreen_config.xloclip: %d\n", config_value);
		touchscreen_config.xloclip = config_value;
	}
	else if(cmd_no == 21)
	{
		QT602240_TRACE("CMD 21, touchscreen_config.xhiclip: %d\n", config_value);
		touchscreen_config.xhiclip = config_value;
	}
	else if(cmd_no == 22)
	{
		QT602240_TRACE("CMD 22, touchscreen_config.yloclip: %d\n", config_value);
		touchscreen_config.yloclip = config_value;
	}
	else if(cmd_no == 23)
	{
		QT602240_TRACE("CMD 23, touchscreen_config.yhiclip: %d\n", config_value);
		touchscreen_config.yhiclip = config_value;
	}
	else if(cmd_no == 24)
	{
		QT602240_TRACE("CMD 24, touchscreen_config.xedgectrl: %d\n", config_value);
		touchscreen_config.xedgectrl  = config_value;
	}
	else if(cmd_no == 25)
	{
		QT602240_TRACE("CMD 25, touchscreen_config.xedgedist: %d\n", config_value);
		touchscreen_config.xedgedist   = config_value;
	}
	else if(cmd_no == 26)
	{
		QT602240_TRACE("CMD 26, touchscreen_config.yedgectrl: %d\n", config_value);
		touchscreen_config.yedgectrl    = config_value;
	}
	else if(cmd_no == 27)
	{
		QT602240_TRACE("CMD 27, touchscreen_config.yedgedist: %d\n", config_value);
		touchscreen_config.yedgedist     = config_value;
	}
	else if(cmd_no == 28)
	{
		if(tsp_version >= 0x16){
			QT602240_TRACE("CMD 28, touchscreen_config.jumplimit: %d\n", config_value);
			touchscreen_config.jumplimit      = config_value;
			}
		else
			QT602240_TRACE("CMD 28, touchscreen_config.jumplimit  is not supported in this version.\n");
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

	if (write_multitouchscreen_config(0, touchscreen_config) != CFG_WRITE_OK)
    {
        QT602240_ERROR("configuration fail\n");
    }

	return size;
}
static DEVICE_ATTR(set_touchscreen, 0664, set_touchscreen_show, set_touchscreen_store);

/* Set key array config. 
   keyarray_config.ctrl = 0;
   keyarray_config.xorigin = 0;
   keyarray_config.xsize = 0;
   keyarray_config.yorigin = 0;
   keyarray_config.ysize = 0;
   keyarray_config.akscfg = 0;
   keyarray_config.blen = 0;
   keyarray_config.tchthr = 0;
   keyarray_config.tchdi = 0; */
static ssize_t set_keyarray_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_keyarray_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );

	if (cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, keyarray_config.ctrl: %d\n", config_value);
		keyarray_config.ctrl = config_value;
	}		
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, keyarray_config.xorigin: %d\n", config_value);
		keyarray_config.xorigin = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, keyarray_config.xsize: %d\n", config_value);
		keyarray_config.xsize = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, keyarray_config.yorigin: %d\n", config_value);
		keyarray_config.yorigin = config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, keyarray_config.ysize: %d\n", config_value);
		keyarray_config.ysize  = config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, keyarray_config.akscfg: %d\n", config_value);
		keyarray_config.akscfg  = config_value;
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, keyarray_config.blen: %d\n", config_value);
		keyarray_config.blen = config_value;
	}
	else if(cmd_no == 7)
	{
		QT602240_TRACE("CMD 7, keyarray_config.tchthr: %d\n", config_value);
		keyarray_config.tchthr = config_value;
	}
	else if(cmd_no == 8)
	{
		QT602240_TRACE("CMD 8, keyarray_config.tchdi: %d\n", config_value);
		keyarray_config.tchdi = config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

	return size;
}
static DEVICE_ATTR(set_keyarray, 0664, set_keyarray_show, set_keyarray_store);

static ssize_t set_qt_init_state_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
	unsigned int i2c_state;
	
	QT602240_TRACE("operate nothing\n");

	i2c_state = qt602240_init_fail;	

	return sprintf(buf,"%u\n",i2c_state);
}
static DEVICE_ATTR(set_qt_init_state, 0664, set_qt_init_state_show, NULL);


static ssize_t set_noise_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_noise_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");

	if(value < 10000){
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );}
	else{
	cmd_no = (int) (value / 100000);
	config_value = ( int ) (value % 100000 );
	}

	if(cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, noise_suppression_config.ctrl: %d\n", config_value);
		noise_suppression_config.ctrl = config_value;
	}
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, noise_suppression_config.reserved: %d\n", config_value);
	#if defined(__VER_1_2__)
    	noise_suppression_config.outflen = config_value;
	#elif defined(__VER_1_4__)
    	noise_suppression_config.virtrefrnkg = config_value;
	#endif
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, noise_suppression_config.reserved1: %d\n", config_value);
		noise_suppression_config.reserved1  = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, noise_suppression_config.gcaful: %d\n", config_value);
		noise_suppression_config.gcaful = config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, noise_suppression_config.gcafll: %d\n", config_value);
		noise_suppression_config.gcafll  = config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, noise_suppression_config.actvgcafvalid: %d\n", config_value);
	#if defined(__VER_1_2__)
    	noise_suppression_config.gcaflcount = config_value;
	#elif defined(__VER_1_4__)
    	noise_suppression_config.actvgcafvalid = config_value;
	#endif
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, noise_suppression_config.noisethr: %d\n", config_value);
		noise_suppression_config.noisethr  = config_value;
	}
	else if(cmd_no == 7)
	{
		QT602240_TRACE("CMD 7, noise_suppression_config.freqhopscale: %d\n", config_value);
		noise_suppression_config.freqhopscale  = config_value;
	}
	else if(cmd_no == 8)
	{
		QT602240_TRACE("CMD 8, noise_suppression_config.freq[0]: %d\n", config_value);
		noise_suppression_config.freq[0]  = config_value;
	}
	else if(cmd_no == 9)
	{
		QT602240_TRACE("CMD 9, noise_suppression_config.freq[1]: %d\n", config_value);
		noise_suppression_config.freq[1]  = config_value;
	}
	else if(cmd_no == 10)
	{
		QT602240_TRACE("CMD 10, noise_suppression_config.freq[2]: %d\n", config_value);
		noise_suppression_config.freq[2]  = config_value;
	}
	else if(cmd_no == 11)
	{
		QT602240_TRACE("CMD 11, noise_suppression_config.freq[3]: %d\n", config_value);
		noise_suppression_config.freq[3]  = config_value;
	}
	else if(cmd_no == 12)
	{
		QT602240_TRACE("CMD 12, noise_suppression_config.freq[4]: %d\n", config_value);
		noise_suppression_config.freq[4]  = config_value;
	}
	else if(cmd_no == 13)
	{
		QT602240_TRACE("CMD 13, noise_suppression_config.idlegcafvalid: %d\n", config_value);
		noise_suppression_config.idlegcafvalid  = config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}
	
    /* Write Noise suppression config to chip. */
    if (get_object_address(PROCG_NOISESUPPRESSION_T22, 0) != OBJECT_NOT_FOUND)
    {
        if (write_noisesuppression_config(0,noise_suppression_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }

	
    /* Write grip suppression config to chip. */
    if (get_object_address(PROCI_GRIPFACESUPPRESSION_T20, 0) != OBJECT_NOT_FOUND)
    {
        if (write_gripsuppression_config(0, gripfacesuppression_config) !=
            CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }

	return size;
}
static DEVICE_ATTR(set_noise, 0664, set_noise_show, set_noise_store);

static ssize_t set_grip_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_grip_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );
	
	if(cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, gripfacesuppression_config.ctrl: %d\n", config_value);
		gripfacesuppression_config.ctrl  = config_value;
	}
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, gripfacesuppression_config.xlogrip: %d\n", config_value);
		gripfacesuppression_config.xlogrip  = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, gripfacesuppression_config.xhigrip is not set: %d\n", config_value);
		gripfacesuppression_config.xhigrip  = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, gripfacesuppression_config.ylogrip: %d\n", config_value);
		gripfacesuppression_config.ylogrip  = config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, gripfacesuppression_config.yhigrip: %d\n", config_value);
		gripfacesuppression_config.yhigrip  =  config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, gripfacesuppression_config.maxtchs: %d\n", config_value);
		gripfacesuppression_config.maxtchs  = config_value;
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, gripfacesuppression_config.reserved: %d\n", config_value);
		gripfacesuppression_config.reserved   = config_value;
	}
	else if(cmd_no == 7)
	{
		QT602240_TRACE("CMD 7, gripfacesuppression_config.szthr1: %d\n", config_value);
		gripfacesuppression_config.szthr1   = config_value;
	}
	else if(cmd_no == 8)
	{
		QT602240_TRACE("CMD 8, gripfacesuppression_config.szthr2: %d\n", config_value);
		gripfacesuppression_config.szthr2   = config_value;
	}
	else if(cmd_no == 9)
	{
		QT602240_TRACE("CMD 9, gripfacesuppression_config.shpthr1: %d\n", config_value);
		gripfacesuppression_config.shpthr1   = config_value;
	}
	else if(cmd_no == 10)
	{
		QT602240_TRACE("CMD 10, gripfacesuppression_config.shpthr2: %d\n", config_value);
		gripfacesuppression_config.shpthr2   = config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

    /* Write grip suppression config to chip. */
    if (get_object_address(PROCI_GRIPFACESUPPRESSION_T20, 0) != OBJECT_NOT_FOUND)
    {
        if (write_gripsuppression_config(0, gripfacesuppression_config) !=
            CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }

	return size;
}
static DEVICE_ATTR(set_grip, 0664, set_grip_show, set_grip_store);



/* total 
   linearization_config.ctrl = 0;
   twotouch_gesture_config.ctrl = 0;
   onetouch_gesture_config.ctrl = 0;
   noise_suppression_config.ctrl = 0;
   selftest_config.ctrl = 0;
   gripfacesuppression_config.ctrl = 0;
   cte_config.ctrl = 0;    */
static ssize_t set_total_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("operate nothing\n");

	return 0;
}
static ssize_t set_total_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd_no,config_value = 0;
	char *after;

	unsigned long value = simple_strtoul(buf, &after, 10);	
	QT602240_TRACE("start\n");
	cmd_no = (int) (value / 1000);
	config_value = ( int ) (value % 1000 );

	//if (cmd_no == 0)
	//{
	//	QT602240_TRACE("[%s] CMD 0 , linearization_config.ctrl: %d\n", config_value);
	//	linearization_config.ctrl = config_value;
	//}		
	if(cmd_no == 0)
	{
		QT602240_TRACE("CMD 0, twotouch_gesture_config.ctrl: %d\n", config_value);
		twotouch_gesture_config.ctrl = config_value;
	}
	else if(cmd_no == 1)
	{
		QT602240_TRACE("CMD 1, onetouch_gesture_config.ctrl: %d\n", config_value);
		onetouch_gesture_config.ctrl = config_value;
	}
	else if(cmd_no == 2)
	{
		QT602240_TRACE("CMD 2, noise_suppression_config.ctrl is not set: %d\n", config_value);
		noise_suppression_config.ctrl = config_value;
	}
	else if(cmd_no == 3)
	{
		QT602240_TRACE("CMD 3, selftest_config.ctrl: %d\n", config_value);
		selftest_config.ctrl = config_value;
	}
	else if(cmd_no == 4)
	{
		QT602240_TRACE("CMD 4, gripfacesuppression_config.ctrl: %d\n", config_value);
		gripfacesuppression_config.ctrl =  config_value;
	}
	else if(cmd_no == 5)
	{
		QT602240_TRACE("CMD 5, cte_config.ctrl: %d\n", config_value);
		cte_config.ctrl = config_value;
	}
	else if(cmd_no == 6)
	{
		QT602240_TRACE("CMD 6, cte_config.idlegcafdepth: %d\n", config_value);
		cte_config.idlegcafdepth=config_value;
	}
	else if(cmd_no == 7)
	{
		QT602240_TRACE("CMD 7, cte_config.actvgcafdepth: %d\n", config_value);
		cte_config.actvgcafdepth= config_value;
	}
	else 
	{
		QT602240_TRACE("unknown CMD\n");
	}

	return size;
}
static DEVICE_ATTR(set_total, 0664, set_total_show, set_total_store);

static ssize_t set_write_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	QT602240_TRACE("power_config.idleacqint: %d\n", power_config.idleacqint); 
	QT602240_TRACE("power_config.actvacqint: %d\n", power_config.actvacqint); 
	QT602240_TRACE("power_config.actv2idleto: %d\n", power_config.actv2idleto); 

	if (write_power_config(power_config) != CFG_WRITE_OK)
	{
		/* "Power config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		return CFG_WRITE_FAILED;
	}

	QT602240_TRACE("acquisition_config.chrgtime: %d\n", acquisition_config.chrgtime); 
	QT602240_TRACE("acquisition_config.tchdrift: %d\n", acquisition_config.tchdrift); 
	QT602240_TRACE("acquisition_config.driftst: %d\n", acquisition_config.driftst); 
	QT602240_TRACE("acquisition_config.tchautocal: %d\n", acquisition_config.tchautocal); 
	QT602240_TRACE("acquisition_config.sync: %d\n", acquisition_config.sync); 
	QT602240_TRACE("acquisition_config.atchcalst: %d\n", acquisition_config.atchcalst); 
	QT602240_TRACE("acquisition_config.atchcalsthr: %d\n", acquisition_config.atchcalsthr); 

	/* Write acquisition config to chip. */
	if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
	{
		/* "Acquisition config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		return CFG_WRITE_FAILED;
	}

	QT602240_TRACE("0, touchscreen_config.ctrl: %d\n",  touchscreen_config.ctrl);
	QT602240_TRACE("1, touchscreen_config.xorigin: %d\n", touchscreen_config.xorigin);
	QT602240_TRACE("2, touchscreen_config.yorigin: %d\n",  touchscreen_config.yorigin);
	QT602240_TRACE("3, touchscreen_config.xsize: %d\n",touchscreen_config.xsize);
	QT602240_TRACE("4, touchscreen_config.ysize: %d\n", touchscreen_config.ysize);
	QT602240_TRACE("5, touchscreen_config.akscfg: %d\n", touchscreen_config.akscfg);
	QT602240_TRACE("6, touchscreen_config.blen: %d\n", touchscreen_config.blen);
	QT602240_TRACE("7, touchscreen_config.tchthr: %d\n",  touchscreen_config.tchthr);
	QT602240_TRACE("8, touchscreen_config.tchdi: %d\n",touchscreen_config.tchdi);
	QT602240_TRACE("9, touchscreen_config.orient: %d\n", touchscreen_config.orient);
	QT602240_TRACE("10, touchscreen_config.mrgtimeout: %d\n",touchscreen_config.mrgtimeout);
	QT602240_TRACE("11, touchscreen_config.movhysti: %d\n",touchscreen_config.movhysti);
	QT602240_TRACE("12, touchscreen_config.movhystn: %d\n",touchscreen_config.movhystn);
	QT602240_TRACE("13, touchscreen_config.movfilter: %d\n",touchscreen_config.movfilter);
	QT602240_TRACE("14, touchscreen_config.numtouch: %d\n",touchscreen_config.numtouch);
	QT602240_TRACE("15, touchscreen_config.mrghyst: %d\n",touchscreen_config.mrghyst);
	QT602240_TRACE("16, touchscreen_config.mrgthr: %d\n",touchscreen_config.mrgthr);
	QT602240_TRACE("17, touchscreen_config.amphyst: %d\n",touchscreen_config.amphyst);
	QT602240_TRACE("18, touchscreen_config.xrange: %d\n",touchscreen_config.xrange);
	QT602240_TRACE("19, touchscreen_config.yrange: %d\n",touchscreen_config.yrange);
	QT602240_TRACE("20, touchscreen_config.xloclip: %d\n",touchscreen_config.xloclip);
	QT602240_TRACE("21, touchscreen_config.xhiclip: %d\n",touchscreen_config.xhiclip);
	QT602240_TRACE("22, touchscreen_config.yloclip: %d\n",touchscreen_config.yloclip);
	QT602240_TRACE("23, touchscreen_config.yhiclip: %d\n",touchscreen_config.yhiclip);
	QT602240_TRACE("24, touchscreen_config.xedgectrl: %d\n",touchscreen_config.xedgectrl);
	QT602240_TRACE("25, touchscreen_config.xedgedist: %d\n",touchscreen_config.xedgedist);
	QT602240_TRACE("26, touchscreen_config.yedgectrl: %d\n",touchscreen_config.yedgectrl);
	QT602240_TRACE("27, touchscreen_config.yedgedist: %d\n",touchscreen_config.yedgedist);
	if(tsp_version >= 0x16)
	{
		QT602240_TRACE("28, touchscreen_config.jumplimit: %d\n",touchscreen_config.jumplimit);
	}
	else
	{
		QT602240_TRACE("28, touchscreen_config.jumplimit is not supported in this version.\n");
	}

	/* Write touchscreen (1st instance) config to chip. */
	if (write_multitouchscreen_config(0, touchscreen_config) != CFG_WRITE_OK)
	{
		/* "Multitouch screen config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		return CFG_WRITE_FAILED;
	}

	QT602240_TRACE("0, keyarray_config.ctrl: %d\n", keyarray_config.ctrl);
	QT602240_TRACE("1, keyarray_config.xorigin: %d\n", keyarray_config.xorigin);
	QT602240_TRACE("2, keyarray_config.xsize: %d\n",keyarray_config.xsize);
	QT602240_TRACE("3, keyarray_config.yorigin: %d\n", keyarray_config.yorigin); 
	QT602240_TRACE("4,	keyarray_config.ysize: %d\n", keyarray_config.ysize);
	QT602240_TRACE("5,	keyarray_config.akscfg: %d\n", keyarray_config.akscfg);
	QT602240_TRACE("6,	keyarray_config.blen: %d\n", keyarray_config.blen);
	QT602240_TRACE("7,	keyarray_config.tchthr: %d\n", keyarray_config.tchthr);
	QT602240_TRACE("8,	keyarray_config.tchdi: %d\n", keyarray_config.tchdi);

	/* Write key array (1st instance) config to chip. */
	if (write_keyarray_config(0, keyarray_config) != CFG_WRITE_OK)
	{
		/* "Key array config write failed!\n" */
		QT602240_ERROR("configuration fail\n");
		return CFG_WRITE_FAILED;
	}

	//QT602240_TRACE("0, linearization_config.ctrl: %d\n", linearization_config.ctrl);
	QT602240_TRACE("0, twotouch_gesture_config.ctrl: %d\n", twotouch_gesture_config.ctrl);
	QT602240_TRACE("1, onetouch_gesture_config.ctrl: %d\n",onetouch_gesture_config.ctrl);
	QT602240_TRACE("2, noise_suppression_config.ctrl is not set: %d\n", noise_suppression_config.ctrl); 
	QT602240_TRACE("3,	selftest_config.ctrl is not set: %d\n", selftest_config.ctrl);
	QT602240_TRACE("4,	gripfacesuppression_config.ctrl: %d\n", gripfacesuppression_config.ctrl);
	QT602240_TRACE("5,	cte_config.ctrl: %d\n", cte_config.ctrl);
	QT602240_TRACE("6,	cte_config.idlegcafdepth: %d\n", cte_config.idlegcafdepth);
	QT602240_TRACE("7,	cte_config.actvgcafdepth: %d\n", cte_config.actvgcafdepth);

	/* Write linearization table config to chip. */
	//if (get_object_address(PROCI_LINEARIZATIONTABLE_T17, 0) != OBJECT_NOT_FOUND)
	//{
	//	if (write_linearization_config(0, linearization_config) != CFG_WRITE_OK)
	//	{
	//		/* "Linearization config write failed!\n" */
	//		QT602240_ERROR("configuration fail\n");
	//		return CFG_WRITE_FAILED;
	//	}
	//}
	/* Write twotouch gesture config to chip. */
	if (get_object_address(PROCI_TWOTOUCHGESTUREPROCESSOR_T27, 0) != OBJECT_NOT_FOUND)
	{
		if (write_twotouchgesture_config(0, twotouch_gesture_config) != CFG_WRITE_OK)
		{
			/* "Two touch gesture config write failed!\n" */
			QT602240_ERROR("configuration fail\n");
			return CFG_WRITE_FAILED;
		}
	}

	/* Write one touch gesture config to chip. */
	if (get_object_address(PROCI_ONETOUCHGESTUREPROCESSOR_T24, 0) != OBJECT_NOT_FOUND)
	{
		if (write_onetouchgesture_config(0, onetouch_gesture_config) != CFG_WRITE_OK)
		{
			/* "One touch gesture config write failed!\n" */
			QT602240_ERROR("configuration fail\n");
			return CFG_WRITE_FAILED;
		}
	}

	QT602240_TRACE("gripfacesuppression_config.ctrl: %d\n", gripfacesuppression_config.ctrl); 
	QT602240_TRACE("gripfacesuppression_config.xlogrip: %d\n", gripfacesuppression_config.xlogrip); 
	QT602240_TRACE("gripfacesuppression_config.xhigrip: %d\n", gripfacesuppression_config.xhigrip); 
	QT602240_TRACE("gripfacesuppression_config.ylogrip: %d\n", gripfacesuppression_config.ylogrip); 
	QT602240_TRACE("gripfacesuppression_config.yhigrip: %d\n", gripfacesuppression_config.yhigrip); 
	QT602240_TRACE("gripfacesuppression_config.maxtchs: %d\n", gripfacesuppression_config.maxtchs); 
	QT602240_TRACE("gripfacesuppression_config.reserved: %d\n", gripfacesuppression_config.reserved); 
	QT602240_TRACE("gripfacesuppression_config.szthr1: %d\n", gripfacesuppression_config.szthr1); 
	QT602240_TRACE("gripfacesuppression_config.szthr2: %d\n", gripfacesuppression_config.szthr2); 
	QT602240_TRACE("gripfacesuppression_config.shpthr1: %d\n", gripfacesuppression_config.shpthr1); 
	QT602240_TRACE("gripfacesuppression_config.shpthr2: %d\n", gripfacesuppression_config.shpthr2); 

	/* Write grip suppression config to chip. */
	if (get_object_address(PROCI_GRIPFACESUPPRESSION_T20, 0) != OBJECT_NOT_FOUND)
	{
		if (write_gripsuppression_config(0, gripfacesuppression_config) != CFG_WRITE_OK)
		{
			/* "Grip suppression config write failed!\n" */
			QT602240_ERROR("configuration fail\n");
			return CFG_WRITE_FAILED;
		}
	}

	QT602240_TRACE("noise_suppression_config.ctrl: %d\n", noise_suppression_config.ctrl); 
	QT602240_TRACE("noise_suppression_config.virtrefrnkg: %d\n", noise_suppression_config.virtrefrnkg); 
	QT602240_TRACE("noise_suppression_config.reserved1: %d\n", noise_suppression_config.reserved1); 
	QT602240_TRACE("noise_suppression_config.gcaful: %d\n", noise_suppression_config.gcaful); 
	QT602240_TRACE("noise_suppression_config.gcafll: %d\n", noise_suppression_config.gcafll); 
	QT602240_TRACE("noise_suppression_config.actvgcafvalid: %d\n", noise_suppression_config.actvgcafvalid); 
	QT602240_TRACE("noise_suppression_config.noisethr: %d\n", noise_suppression_config.noisethr); 
	QT602240_TRACE("noise_suppression_config.freqhopscale: %d\n", noise_suppression_config.freqhopscale); 
	QT602240_TRACE("noise_suppression_config.freq[0]: %d\n", noise_suppression_config.freq[0]); 
	QT602240_TRACE("noise_suppression_config.freq[1]: %d\n", noise_suppression_config.freq[1]); 
	QT602240_TRACE("noise_suppression_config.freq[2]: %d\n", noise_suppression_config.freq[2]); 
	QT602240_TRACE("noise_suppression_config.freq[3]: %d\n", noise_suppression_config.freq[3]); 
	QT602240_TRACE("noise_suppression_config.freq[4]: %d\n", noise_suppression_config.freq[4]); 
	QT602240_TRACE("noise_suppression_config.idlegcafvalid: %d\n", noise_suppression_config.idlegcafvalid); 	

    /* Write Noise suppression config to chip. */
    if (get_object_address(PROCG_NOISESUPPRESSION_T22, 0) != OBJECT_NOT_FOUND)
    {
        if (write_noisesuppression_config(0,noise_suppression_config) != CFG_WRITE_OK)
        {
            QT602240_ERROR("configuration fail\n");
        }
    }


	/* Write CTE config to chip. */
	if (get_object_address(SPT_CTECONFIG_T28, 0) != OBJECT_NOT_FOUND)
	{
		if (write_CTE_config(cte_config) != CFG_WRITE_OK)
		{
			/* "CTE config write failed!\n" */
			QT602240_ERROR("configuration fail\n");
			return CFG_WRITE_FAILED;
		}
	}

	/* Backup settings to NVM. */
	if (backup_config() != WRITE_MEM_OK)
	{
		/* "Failed to backup, exiting...\n" */
		return WRITE_MEM_FAILED;
	}
	
	/* Calibrate the touch IC. */
	if (calibrate_chip() != WRITE_MEM_OK)
	{
		QT602240_ERROR("Failed to calibrate, exiting\n");
		return WRITE_MEM_FAILED;
	}

	QT602240_TRACE("\n[TSP] configs saved : %d\n", __LINE__);

	return 0;
}
static ssize_t set_write_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{

	QT602240_TRACE("operate nothing\n");

	return size;
}
static DEVICE_ATTR(set_write, 0664, set_write_show, set_write_store);

/*****************************************************************************
*
*  FUNCTION
*  PURPOSE
*  INPUT
*  OUTPUT
*                                   
* ***************************************************************************/
#if ENABLE_NOISE_TEST_MODE
struct device *qt602240_noise_test;

uint8_t read_uint16_t( uint16_t Address, uint16_t *Data )
{
    uint8_t status;
    uint8_t temp[2];

   status = read_mem(Address, 2, temp);
//   status = read_mem(0, 2, temp);
    *Data= ((uint16_t)temp[1]<<8)+ (uint16_t)temp[0];

    return (status);
}

uint8_t read_dbg_data(uint8_t dbg_mode , uint8_t node, uint16_t * dbg_data)
{
    uint8_t status;
    uint8_t mode,page,i;
    uint8_t read_page,read_point;
  
    diagnostic_addr= get_object_address(DEBUG_DIAGNOSTIC_T37, 0);
    
    read_page = node / 64;
    node %= 64;
    read_point = (node *2) + 2;
    
    //Page Num Clear
    status = diagnostic_chip(QT_CTE_MODE);
    msleep(20);
	 
    status = diagnostic_chip(dbg_mode);
     msleep(20); 
	
    for(i = 0; i < 5; i++)
    {
        msleep(20);    
        status = read_mem(diagnostic_addr,1, &mode);
        if(status == READ_MEM_OK)
        {
            if(mode == dbg_mode)
            {
                break;
            }
        }
	else
	{
	 QT602240_TRACE("READ_MEM_FAILED: %d\n", status);			
	        return READ_MEM_FAILED;
	}
    }


    
    for(page = 0; page < read_page; page ++)
    {
        status = diagnostic_chip(QT_PAGE_UP); 
        msleep(10); 
    }
  
    status = read_uint16_t(diagnostic_addr + read_point,dbg_data);  

    msleep(10); 
    
    return (status);
}



static ssize_t set_refer0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_refrence=0;

	status = read_dbg_data(QT_REFERENCE_MODE, test_node[0],&qt_refrence);
	return sprintf(buf, "%u\n", qt_refrence);	
}

static ssize_t set_refer1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_refrence=0;

	status = read_dbg_data(QT_REFERENCE_MODE, test_node[1],&qt_refrence);
	return sprintf(buf, "%u\n", qt_refrence);	
}

static ssize_t set_refer2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_refrence=0;

	status = read_dbg_data(QT_REFERENCE_MODE, test_node[2],&qt_refrence);
	return sprintf(buf, "%u\n", qt_refrence);	
}


static ssize_t set_refer3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_refrence=0;

	status = read_dbg_data(QT_REFERENCE_MODE, test_node[3],&qt_refrence);
	return sprintf(buf, "%u\n", qt_refrence);	
}


static ssize_t set_refer4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_refrence=0;

	status = read_dbg_data(QT_REFERENCE_MODE, test_node[4],&qt_refrence);
	return sprintf(buf, "%u\n", qt_refrence);	
}

static ssize_t set_delta0_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_delta=0;

	status = read_dbg_data(QT_DELTA_MODE, test_node[0],&qt_delta);
	if(qt_delta < 32767){
		return sprintf(buf, "%u\n", qt_delta);	
	   }
	else	{
			qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);	
	}
}

static ssize_t set_delta1_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_delta=0;

	status = read_dbg_data(QT_DELTA_MODE, test_node[1],&qt_delta);
	if(qt_delta < 32767){
		return sprintf(buf, "%u\n", qt_delta);	
	   }
	else	{
			qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);	
	}	
}

static ssize_t set_delta2_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_delta=0;

	status = read_dbg_data(QT_DELTA_MODE, test_node[2],&qt_delta);
	if(qt_delta < 32767){
		return sprintf(buf, "%u\n", qt_delta);	
	   }
	else	{
			qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);	
	}
}

static ssize_t set_delta3_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_delta=0;

    status = read_dbg_data(QT_DELTA_MODE, test_node[3],&qt_delta);
	if(qt_delta < 32767){
		return sprintf(buf, "%u\n", qt_delta);	
	   }
	else	{
			qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);	
	}
}

static ssize_t set_delta4_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char status;
    uint16_t qt_delta=0;

    status = read_dbg_data(QT_DELTA_MODE, test_node[4],&qt_delta);
	if(qt_delta < 32767){
		return sprintf(buf, "%u\n", qt_delta);	
	   }
	else	{
			qt_delta = 65535 - qt_delta;
		return sprintf(buf, "-%u\n", qt_delta);	
	}
}

static ssize_t set_threshold_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", touchscreen_config.tchthr);	
}


static DEVICE_ATTR(set_refer0, 0664, set_refer0_mode_show, NULL);
static DEVICE_ATTR(set_delta0, 0664, set_delta0_mode_show, NULL);
static DEVICE_ATTR(set_refer1, 0664, set_refer1_mode_show, NULL);
static DEVICE_ATTR(set_delta1, 0664, set_delta1_mode_show, NULL);
static DEVICE_ATTR(set_refer2, 0664, set_refer2_mode_show, NULL);
static DEVICE_ATTR(set_delta2, 0664, set_delta2_mode_show, NULL);
static DEVICE_ATTR(set_refer3, 0664, set_refer3_mode_show, NULL);
static DEVICE_ATTR(set_delta3, 0664, set_delta3_mode_show, NULL);
static DEVICE_ATTR(set_refer4, 0664, set_refer4_mode_show, NULL);
static DEVICE_ATTR(set_delta4, 0664, set_delta4_mode_show, NULL);
static DEVICE_ATTR(set_threshould, 0664, set_threshold_mode_show, NULL);
#endif /* ENABLE_NOISE_TEST_MODE */

#ifdef QT_ATCOM_TEST
struct device *qt602240_atcom_test;
struct work_struct qt_touch_update_work;

unsigned int qt_firm_status_data=0;
void set_qt_update_exe(struct work_struct * p)
{

	disable_irq(qt602240->client->irq);
//	quantum_touch_probe();  /* find and initialise QT device */

        QT602240_TRACE("enter to firmware download by AT command\n");
        if(!QT_Boot())
        {            	
		qt_firm_status_data=2;		// firmware update success
        	QT602240_TRACE("reprogram done: firmware update success\n");
        }
	else	{
		qt_firm_status_data=3;		// firmware update Fail
		 QT602240_ERROR("reprogram done: firmware update fail\n");
	}
	TSP_Restart();
        quantum_touch_probe();
//	msleep(100);
	enable_irq(qt602240->client->irq);

}

static ssize_t set_qt_update_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count;
		
	QT602240_TRACE("touch firmware update\n");
	qt_firm_status_data=1;	//start firmware updating
	INIT_WORK(&qt_touch_update_work, set_qt_update_exe);
	queue_work(qt602240_wq, &qt_touch_update_work);
	
	if(qt_firm_status_data == 3)
	{
		count = sprintf(buf,"FAIL\n");
	}
	else
		count = sprintf(buf,"OK\n");
	return count;

}
static ssize_t set_qt_firm_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d\n", tsp_version);	

}

static ssize_t set_qt_firm_version_read_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	
	return sprintf(buf, "%d\n", info_block->info_id.version);	

}

static ssize_t set_qt_firm_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	int count;

	QT602240_TRACE("enter set_qt_firm_status_show by AT command\n");
	
	if(qt_firm_status_data == 1)
	{
		count = sprintf(buf,"Downloading\n");
	}
	else if(qt_firm_status_data == 2)
	{
		count = sprintf(buf,"PASS\n");
	}
	else if(qt_firm_status_data == 3)
	{
		count = sprintf(buf,"FAIL\n");
	}
	else
		count = sprintf(buf,"PASS\n");

	return count;

}

static DEVICE_ATTR(set_qt_update, 0664, set_qt_update_show, NULL);
static DEVICE_ATTR(set_qt_firm_version, 0664, set_qt_firm_version_show, NULL);
static DEVICE_ATTR(set_qt_firm_status, 0664, set_qt_firm_status_show, NULL);
static DEVICE_ATTR(set_qt_firm_version_read, 0664, set_qt_firm_version_read_show, NULL);
#endif

ssize_t set_tsp_for_inputmethod_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        QT602240_TRACE("is_inputmethod: %d\n", is_inputmethod);
        if( is_inputmethod )
        {
                *buf = '1';
        }
        else
        {
                *buf = '0';
        }

	return 0;
}
ssize_t set_tsp_for_inputmethod_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	uint16_t object_address;
    	uint8_t *tmp;
    	uint8_t status;
//    	uint8_t object_size;

	disable_irq(qt602240->client->irq);
        if( *buf == '1' && (!is_inputmethod ) )
        {
                is_inputmethod = 1;        
		 touchscreen_config.jumplimit = 10;
		QT602240_TRACE("set TSP inputmethod IN\n");		 

		object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9, 0);

		if (object_address == 0)
		{
		    	QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 object_address\n");
			enable_irq(qt602240->client->irq);
			return -1;
		}
		tmp= &touchscreen_config.jumplimit;
		status = write_mem(object_address+30, 1, tmp);	
		
		if (status == WRITE_MEM_FAILED)
		{
		    QT602240_TRACE("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
		}

        }
        else if ( *buf == '0' && (is_inputmethod ))
        {
                is_inputmethod = 0;
		   touchscreen_config.jumplimit  = 18;

		QT602240_TRACE("set TSP inputmethod OUT\n");	

		object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9, 0);

		if (object_address == 0)
		{
		    	QT602240_TRACE("TOUCH_MULTITOUCHSCREEN_T9 object_address\n");
			enable_irq(qt602240->client->irq);
			return -1;
		}
		tmp= &touchscreen_config.jumplimit;
		status = write_mem(object_address+30, 1, tmp);	
		
		if (status == WRITE_MEM_FAILED)
		{
		    	QT602240_TRACE("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
		}

        }        

	enable_irq(qt602240->client->irq);
	return 1;             
        
}

static DEVICE_ATTR(set_tsp_for_inputmethod, 0664, set_tsp_for_inputmethod_show, set_tsp_for_inputmethod_store);


#ifdef QT_STYLUS_ENABLE
static ssize_t qt602240_config_mode_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	if(config_mode_val)
	{
		QT602240_TRACE("config: stylus mode\n");
	}
	else
	{
		QT602240_TRACE("config: normal mode\n");
	}
	
	return snprintf(buf, PAGE_SIZE, "%d\n", config_mode_val);
}

static ssize_t qt602240_config_mode_store(struct device *dev,	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int mode;

	if(*buf == 0x00)
		mode = 0;
	else if(*buf == 0x01)
		mode = 1;
	else
		if (sscanf(buf, "%u", &mode) != 1) {
			dev_err(dev, "Invalid values(0x%x)\n", *buf);
			return -EINVAL;
		}
//	QT602240_TRACE("disable IRQ\n");
	disable_irq(qt602240->client->irq);
	switch(mode)
		{
		case 1:
			{
			if(mode != config_mode_val)
				{
				QT602240_TRACE("set stylus mode\n");
 				qt_Multitouchscreen_stylus_Init();
				calibrate_chip();
				}
			else
				{
				QT602240_TRACE("already stylus mode\n");
				}
		
			config_mode_val = mode;
			break;
			}
		case 0:
			{
			if(mode != config_mode_val)
				{
				QT602240_TRACE("set normal mode\n");
				qt_Multitouchscreen_normal_Init();
				calibrate_chip();
				}
			else
				{
				QT602240_TRACE("already normal mode\n");
				}
		
			config_mode_val = mode;
			break;
			}
		default:
			QT602240_TRACE("invalid mode\n");
			break;
		}

//	QT602240_TRACE("enable IRQ\n");
	enable_irq(qt602240->client->irq);
	return count;
}

static DEVICE_ATTR(config_mode, 0664, qt602240_config_mode_show, qt602240_config_mode_store);
#endif


//100707 sooo.shin
static ssize_t qt602240_call_release_touch(struct device *dev, struct device_attribute *attr, char *buf)
{
	QT602240_TRACE("start\n");
	TSP_forced_release_for_call();

#if 0
	disable_irq(qt602240->client->irq);	

	uint16_t object_address;
    	uint8_t *tmp;
    	uint8_t status;	
	int ret = WRITE_MEM_OK;			
		
	if(isVoiceCall){
		touch_isVoiceCall_state = 1;
		
		acquisition_config.atchcalsthr = 39; // 69 ->59
		acquisition_config.tchdrift = 1; // 4s
		acquisition_config.driftst = 1; // 4	
		touchscreen_config.tchthr = 40; // 70 ->60
		touchscreen_config.blen = 16; //

		object_address = get_object_address(TOUCH_MULTITOUCHSCREEN_T9, 0);
		if (object_address == 0)
		{
		    	QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 object_address\n");
			enable_irq(qt602240->client->irq);
			return -1;
		}
		
		tmp= &touchscreen_config.blen;
		status = write_mem(object_address+6, 1, tmp);			

		if (status == WRITE_MEM_FAILED)
		{
		    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
		}
		
		tmp= &touchscreen_config.tchthr;
		status = write_mem(object_address+7, 1, tmp);	
		
		if (status == WRITE_MEM_FAILED)
		{
		    QT602240_ERROR("TOUCH_MULTITOUCHSCREEN_T9 write_mem\n");
		}
		
		/* Write temporary acquisition config to chip. */
		if (write_acquisition_config(acquisition_config) != CFG_WRITE_OK)
		{
		/* "Acquisition config write failed!\n" */
		QT602240_TRACE("configuration fail\n");
		ret = WRITE_MEM_FAILED; /* calling function should retry calibration call */
		}		

		check_chip_calibration(1);
		
		QT602240_TRACE("reset acq atchcalsthr: %d, tchdrift: %d, driftst: %d\n", acquisition_config.atchcalsthr, acquisition_config.tchdrift ,acquisition_config.driftst);
		QT602240_TRACE("reset acq tchthr: %d, blen: %d\n", touchscreen_config.tchthr, touchscreen_config.blen);		
	}
	
	enable_irq(qt602240->client->irq);
#endif	

	return sprintf(buf,"0\n");
}
static DEVICE_ATTR(call_release_touch, 0664, qt602240_call_release_touch, NULL);

/*------------------------------ for tunning ATmel - end ----------------------------*/
int __init qt602240_init(void)
{
	int ret;
	int i=0;

	qt602240_wq = create_singlethread_workqueue("qt602240_wq");
	if (!qt602240_wq)
		return -ENOMEM;

	qt602240 = kzalloc(sizeof(struct qt602240_data), GFP_KERNEL);
	if (qt602240 == NULL) {
		return -ENOMEM;
	}

	qt_time_point = jiffies_to_msecs(jiffies);

	ret = i2c_add_driver(&qt602240_i2c_driver);
	if(ret) QT602240_ERROR("i2c_add_driver failed(%d)\n", ret);

	QT602240_TRACE("ret: %d, qt602240->client name: %s\n", ret, qt602240->client->name);

	if(!(qt602240->client)){
		QT602240_TRACE("slave address changed try to firmware reprogram\n");
		i2c_del_driver(&qt602240_i2c_driver);

		ret = i2c_add_driver(&qt602240_i2c_driver);
		if(ret) QT602240_ERROR("i2c_add_driver failed(%d)\n", ret);
		QT602240_TRACE("ret: %d, qt602240->client name: %s\n",ret, qt602240->client->name);

		if(qt602240->client){
			QT_reprogram();
			i2c_del_driver(&qt602240_i2c_driver);

			ret = i2c_add_driver(&qt602240_i2c_driver);
			if(ret) QT602240_ERROR("i2c_add_driver failed(%d)\n", ret);
			QT602240_TRACE("ret : %d, qt602240->client name: %s\n",ret, qt602240->client->name);
		}
	}


#ifdef _SUPPORT_MULTITOUCH_
	for (i=0; i<MAX_USING_FINGER_NUM ; i++)		// touchscreen_config.numtouch is 5
		fingerInfo[i].pressure = -1;
#endif

	wake_lock_init(&tsp_firmware_wake_lock, WAKE_LOCK_SUSPEND, "TSP");


/* 
 *  Test file creating
 */
//	ts_dev = device_create_drvdata(sec_class, NULL, 0, NULL, "ts");
	sec_class = class_create(THIS_MODULE, "sec");
    if (IS_ERR(sec_class))
	{
		QT602240_ERROR("failed to create class(sec)\n");
	}

	ts_dev = device_create(sec_class, NULL, 0, NULL, "ts");
	if (IS_ERR(ts_dev))
		QT602240_ERROR("failed to create device(ts)\n");	
	if (device_create_file(ts_dev, &dev_attr_set_qt_init_state) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_qt_init_state.attr.name);

	if(!(qt602240->client)){
		QT602240_TRACE("###################################################\n");
		QT602240_TRACE("##                                               ##\n");
		QT602240_TRACE("##    WARNING! TOUCHSCREEN DRIVER CAN'T WORK.    ##\n");
		QT602240_TRACE("##    PLEASE CHECK YOUR TOUCHSCREEN CONNECTOR!   ##\n");
		QT602240_TRACE("##                                               ##\n");
		QT602240_TRACE("###################################################\n");
		qt602240_init_fail = 1;
		i2c_del_driver(&qt602240_i2c_driver);
		return 0;
	}
	
	QT602240_TRACE("%d\n", __LINE__);
//	ts_dev = device_create_drvdata(sec_class, NULL, 0, NULL, "ts");

//	ts_dev = device_create_drvdata(sec_class, NULL, 0, NULL, "ts");
	if (device_create_file(ts_dev, &dev_attr_trace) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_trace.attr.name);
	if (device_create_file(ts_dev, &dev_attr_gpio) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_gpio.attr.name);
	if (device_create_file(ts_dev, &dev_attr_i2c) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_i2c.attr.name);
	if (device_create_file(ts_dev, &dev_attr_setup) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_setup.attr.name);
	if (device_create_file(ts_dev, &dev_attr_firmware) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_firmware.attr.name);
	if (device_create_file(ts_dev, &dev_attr_firmware1) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_firmware1.attr.name);
	if (device_create_file(ts_dev, &dev_attr_key_threshold) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_key_threshold.attr.name);
	if (device_create_file(ts_dev, &dev_attr_set_tsp_for_inputmethod) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_tsp_for_inputmethod.attr.name);
	if (device_create_file(ts_dev, &dev_attr_call_release_touch) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_call_release_touch.attr.name);	

	QT602240_TRACE("platform_driver_register\n");

	/*------------------------------ for tunning ATmel - start ----------------------------*/
	touch_class = class_create(THIS_MODULE, "touch");
	if (IS_ERR(touch_class))
		QT602240_ERROR("failed to create class(touch)\n");

//	switch_test = device_create_drvdata(touch_class, NULL, 0, NULL, "switch");
	switch_test = device_create(touch_class, NULL, 0, NULL, "switch");
	if (IS_ERR(switch_test))
		QT602240_ERROR("failed to create device(switch)\n");

	if (device_create_file(switch_test, &dev_attr_set_power) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_power.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_acquisition) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_acquisition.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_touchscreen) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_touchscreen.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_keyarray) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_keyarray.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_total ) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_total.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_write ) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_write.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_noise ) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_noise.attr.name);
	if (device_create_file(switch_test, &dev_attr_set_grip ) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_grip.attr.name);	
	/*------------------------------ for tunning ATmel - end ----------------------------*/
	/*------------------------------ for Noise APP - start ----------------------------*/
#if ENABLE_NOISE_TEST_MODE
	qt602240_noise_test = device_create(sec_class, NULL, 0, NULL, "qt602240_noise_test");
	if (IS_ERR(qt602240_noise_test))
		QT602240_ERROR("failed to create device(qt602240_noise_test)\n");

#if 0
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta.attr.name);
#else
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer0)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer0.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta0) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta0.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer1)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer1.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta1) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta1.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer2)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer2.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta2) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta2.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer3)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer3.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta3) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta3.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_refer4)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_refer4.attr.name);
	if (device_create_file(qt602240_noise_test, &dev_attr_set_delta4) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_delta4.attr.name);	
	if (device_create_file(qt602240_noise_test, &dev_attr_set_threshould) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_threshould.attr.name);
#endif
#endif

	/*------------------------------ for Noise APP - end ----------------------------*/

	/*------------------------------	 AT COMMAND TEST 		---------------------*/
#ifdef QT_ATCOM_TEST
	qt602240_atcom_test = device_create(sec_class, NULL, 0, NULL, "qt602240_atcom_test");
	if (IS_ERR(qt602240_atcom_test))
		QT602240_ERROR("failed to create device(qt602240_atcom_test)\n");

	if (device_create_file(qt602240_atcom_test, &dev_attr_set_qt_update)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_qt_update.attr.name);
	if (device_create_file(qt602240_atcom_test, &dev_attr_set_qt_firm_version)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_qt_firm_version.attr.name);
	if (device_create_file(qt602240_atcom_test, &dev_attr_set_qt_firm_status)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_qt_firm_status.attr.name);
	if (device_create_file(qt602240_atcom_test, &dev_attr_set_qt_firm_version_read)< 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_set_qt_firm_version_read.attr.name);
#endif
	/*------------------------------	 AT COMMAND TEST 		---------------------*/

#ifdef QT_STYLUS_ENABLE
	qt_stylus = device_create(touch_class, NULL, 0, NULL, "qt_stylus");
	if (IS_ERR(qt_stylus))
		QT602240_ERROR("failed to create device(qt_stylus)\n");

	if (device_create_file(qt_stylus, &dev_attr_config_mode) < 0)
		QT602240_ERROR("failed to create device file(%s)\n", dev_attr_config_mode.attr.name);
#endif

	qt602240_init_fail = 0;

	return 0;
}

void __exit qt602240_exit(void)
{
	wake_lock_destroy(&tsp_firmware_wake_lock);

	i2c_del_driver(&qt602240_i2c_driver);
	if (qt602240_wq)
		destroy_workqueue(qt602240_wq);
}
late_initcall(qt602240_init);
module_exit(qt602240_exit);

MODULE_DESCRIPTION("Quantum Touchscreen Driver");
MODULE_LICENSE("GPL");
