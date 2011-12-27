/*  drivers/input/keyboard/synaptics_i2c_rmi.c *  * Copyright  (C) 2007  Google,
Inc. * * This software is licensed  under the terms of the GNU General  Public *
License version 2, as  published by the Free  Software Foundation, and *  may be
copied,  distributed,  and modified  under  those terms.  *  * This  program  is
distributed in  the hope  that it  will be  useful, *  but WITHOUT ANY WARRANTY;
without  even  the  implied  warranty of  *  MERCHANTABILITY  or  FITNESS FOR  A
PARTICULAR PURPOSE.  See the * GNU General Public License for more details. * */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/synaptics_i2c_rmi.h>
#include <mach/gpio.h>

#ifdef SYNAPTICS_FW_REFLASH
#include "synaptics_reflash.h"
#endif

#ifdef TOUCH_TEST
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/module.h>
#include <linux/input.h>
#include <linux/syscalls.h>
#include <linux/delay.h>

#endif

#ifdef SYNAPTICS_FW_REFLASH
#define FW_REFLASH_SUCCEED 0
#endif

#ifdef SYNAPTICS_TOUCH_LOG
#define SHOW_MSG(args...)	printk(KERN_INFO args)
#else
#define SHOW_MSG(args...)
#endif

 #ifdef SYNAPTICS_TOUCH_DEBUG
 #define DEBUG_MSG(args...)	printk(KERN_INFO args)
 #else
 #define DEBUG_MSG(args...)
 #endif

 #ifdef SYNAPTICS_TOUCH_ERR
#define ERR_MSG(args...)	printk(KERN_ERR args)
 #else
#define ERR_MSG(args...)
 #endif

#define SYNAPTICS_TS_POLLING_TIME 	1 /* polling time(msec) when touch was pressed */

#define INT_STATUS_REG				0x14

#define SYNAPTICS_INT_REG			0x26
#define SYNAPTICS_CONTROL_REG		0x25
#define REPORT_MODE_2D				0x27
#define MAX_X_POS_LOW_REG			0x2D
#define MAX_X_POS_HIGH_REG			0x2E
#define MAX_Y_POS_LOW_REG			0x2F
#define MAX_Y_POS_HIGH_REG			0x30

#define QUERY_BASE_REG				0xE3

#ifdef SYNAPTICS_MELTINGMODE
#define SYNAPTICS_MELTING_REG	0xF0
#endif

#define SYNAPTICS_INT_FLASH		1<<0
#define SYNAPTICS_INT_STATUS 	1<<1
#define SYNAPTICS_INT_ABS0 		1<<2

#define SYNAPTICS_CONTROL_SLEEP 	1<<0
#define SYNAPTICS_CONTROL_NOSLEEP	1<<2

#ifdef SYNAPTICS_MELTINGMODE
#define SYNAPTICS_MELTING_NO		0
#define SYNAPTICS_MELTING_MELT		1<<0
#define SYNAPTICS_MELTING_AUTO		1<<1
#endif

#define FINGER_MAX 2
#define START_ADDR      0x13
#define PRODUCT_ID_STRING_NUM	11
#define CMD_REG_BLOCK_NUM		41
/************************************/

int fw_rev = 0;
EXPORT_SYMBOL(fw_rev);
module_param(fw_rev, int, S_IWUSR | S_IRUGO);

/******* enum ***********************/
enum {
	SYNAPTICS_2000 = 0,
	SYNAPTICS_2100,
	SYNAPTICS_3000,
};
/************************************/

/***********************************************************/
/**************** structure ***********************************/
struct synaptics_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	bool has_relative_report;
	struct hrtimer timer;
	struct work_struct  work;
#ifdef SYNAPTICS_FW_REFLASH
	struct work_struct  work_for_reflash;
#endif
	uint16_t max[2];
	int snap_state[2][2];
	int snap_down_on[2];
	int snap_down_off[2];
	int snap_up_on[2];
	int snap_up_off[2];
	int snap_down[2];
	int snap_up[2];
	uint32_t flags;
	int reported_finger_count;
	int8_t sensitivity_adjust;
	int (*power)(unsigned char on);
	struct early_suspend early_suspend;
	int fw_revision;
};

typedef struct									// synaptics 2000	// synaptics 2100
{
	unsigned char device_status_reg;            //0x13
	unsigned char interrupt_status_reg;			//0x14
	unsigned char finger_state_reg;				//0x15

	// Finger 0
	unsigned char X_high_position_finger0_reg;  //0x16
	unsigned char Y_high_position_finger0_reg;	//0x17
	unsigned char XY_low_position_finger0_reg;	//0x18
	unsigned char XY_width_finger0_reg;			//0x19
	unsigned char Z_finger0_reg;				//0x1A
	// Finger 1
	unsigned char X_high_position_finger1_reg;  //0x1B
	unsigned char Y_high_position_finger1_reg;	//0x1C
	unsigned char XY_low_position_finger1_reg;	//0x1D
	unsigned char XY_width_finger1_reg;			//0x1E
	unsigned char Z_finger1_reg;				//0x1F
 } ts_sensor_data;

typedef struct 									// synaptics 2000 // 2100 //3000
{
	unsigned char device_command;				//0x58			  //0x5C  //0x70
	unsigned char command_2d;					//0x59			  //0x5D  //0x71
	unsigned char bootloader_id0;				//0x5A			  //0x5E  //0x72
	unsigned char bootloader_id1;				//0x5B			  //0x5F  //0x73
	unsigned char flash_properties;				//0x5C			  //0x60  //0x74
	unsigned char block_size0;					//0x5D			  //0x61  //0x75
	unsigned char block_size1;					//0x5E			  //0x62  //0x76
	unsigned char firmware_block_cnt0;			//0x5F			  //0x63  //0x77
	unsigned char firmware_block_cnt1;			//0x60			  //0x64  //0x78
	unsigned char config_block_cnt0;			//0x61			  //0x65  //0x79
	unsigned char config_block_cnt1;			//0x62			  //0x66  //0x7A
	unsigned char manufact_id_query;			//0x63			  //0x67  //0x7B
	unsigned char product_properties_query;		//0x64			  //0x68  //0x7C
	unsigned char customer_family_query;		//0x65			  //0x69  //0x7D
	unsigned char firmware_revision_query;		//0x66			  //0x6A  //0x7E
	unsigned char device_serialization_query0;	//0x67			  //0x6B  //0x7F
	unsigned char device_serialization_query1;	//0x68			  //0x6C  //0x80
	unsigned char device_serialization_query2;	//0x69			  //0x6D  //0x81
	unsigned char device_serialization_query3;	//0x6A			  //0x6E  //0x82
	unsigned char device_serialization_query4;	//0x6B			  //0x6F  //0x83
	unsigned char device_serialization_query5;	//0x6C			  //0x70  //0x84
	unsigned char device_serialization_query6;	//0x6D			  //0x71  //0x85
	unsigned char product_id_query0;			//0x6E			  //0x72  //0x86
	unsigned char product_id_query1;			//0x6F			  //0x73  //0x87
	unsigned char product_id_query2;			//0x70			  //0x74  //0x88
	unsigned char product_id_query3;			//0x71			  //0x75  //0x89
	unsigned char product_id_query4;			//0x72			  //0x76  //0x8A
	unsigned char product_id_query5;			//0x73			  //0x77  //0x8B
	unsigned char product_id_query6;			//0x74			  //0x78  //0x8C
	unsigned char product_id_query7;			//0x75			  //0x79  //0x8D
	unsigned char product_id_query8;			//0x76			  //0x7A  //0x8E
	unsigned char product_id_query9;			//0x77			  //0x7B  //0x8F
	unsigned char per_device_query;				//0x78			  //0x7C  //0x90
	unsigned char reporting_mode_2d;			//0x79			  //0x7D  //0x91
	unsigned char number_x_electrode_2d;		//0x7A			  //0x7E  //0x92
	unsigned char number_y_electrode_2d;		//0x7B			  //0x7F  //0x93
	unsigned char maximum_electorde_2d;			//0x7C			  //0x80  //0x94
	unsigned char absolute_query_2d;			//0x7D			  //0x81  //0x95
}ts_sensor_command;

typedef struct {
	unsigned char finger_count;
	int X_position[FINGER_MAX];
	int Y_position[FINGER_MAX];
} ts_finger_data;
/***********************************************************/

/***********************************************************************************/
/*********** MACROS ****************************************************************/
// 0x00 - not present, 0x01 - present & accurate, 0x10 - present but not accurate, 0x11 - Reserved
#define TS_SNTS_GET_FINGER_STATE_0(finger_status_reg) \
		(finger_status_reg&0x03)
#define TS_SNTS_GET_FINGER_STATE_1(finger_status_reg) \
		((finger_status_reg&0x0C)>>2)
#define TS_SNTS_GET_FINGER_STATE_2(finger_status_reg) \
		((finger_status_reg&0x30)>>4)
#define TS_SNTS_GET_FINGER_STATE_3(finger_status_reg) \
      ((finger_status_reg&0xC0)>>6)
#define TS_SNTS_GET_FINGER_STATE_4(finger_status_reg) \
      (finger_status_reg&0x03)

#define TS_SNTS_GET_X_POSITION(high_reg, low_reg) \
		((int)(high_reg*0x10) + (int)(low_reg&0x0F))
#define TS_SNTS_GET_Y_POSITION(high_reg, low_reg) \
		((int)(high_reg*0x10) + (int)((low_reg&0xF0)/0x10))

#define TS_SNTS_HAS_PINCH(gesture_reg) \
		((gesture_reg&0x40)>>6)
#define TS_SNTS_HAS_FLICK(gesture_reg) \
		((gesture_reg&0x10)>>4)
#define TS_SNTS_HAS_DOUBLE_TAP(gesture_reg) \
		((gesture_reg&0x04)>>2)

#define TS_SNTS_GET_REPORT_RATE(device_control_reg) \
		((device_control_reg&0x40)>>6)
// 1st bit : '0' - Allow sleep mode, '1' - Full power without sleeping
// 2nd and 3rd bit : 0x00 - Normal Operation, 0x01 - Sensor Sleep
#define TS_SNTS_GET_SLEEP_MODE(device_control_reg) \
		(device_control_reg&0x07)
/***********************************************************************************/

/****************************************************************/
/**************** STATIC VARIABLE *********************************/
static struct workqueue_struct *synaptics_wq;
#ifdef SYNAPTICS_FW_REFLASH
static struct workqueue_struct *synaptics_fwdl_wq;
int is_need_forced_update = 0;
EXPORT_SYMBOL(is_need_forced_update);
int is_fw_reflash = 0;
EXPORT_SYMBOL(is_fw_reflash);
static int power_status = 0;
#endif
static ts_sensor_data ts_reg_data={0};
static ts_finger_data curr_ts_data;
static ts_sensor_command ts_cmd_reg_data={0};
static int ts_pre_state = 0; /* for checking the touch state */
//static int longpress_pre = 0;
//static int flicking = 0;
//static uint16_t max_x, max_y;
uint16_t max_x, max_y;
static int kind_of_product = SYNAPTICS_2000;
#ifdef SYNAPTICS_MELTINGMODE
static int is_first_release_event = 1;
#endif

EXPORT_SYMBOL(max_x);
EXPORT_SYMBOL(max_y);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h);
static void synaptics_ts_late_resume(struct early_suspend *h);
#endif
/****************************************************************/

#ifdef SYNAPTICS_FW_REFLASH
static void synaptics_ts_fw_reflash_work_func(struct work_struct *work)
{
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work_for_reflash);
	int ret;

	DEBUG_MSG("start F/W reflash for synaptics touch IC!!\n");

	if(kind_of_product != SYNAPTICS_3000)
	{
		DEBUG_MSG("synaptics_ts_fw_reflash_work_func : F/W update is not supported!\n");
		return;
	}

	if (ts->use_irq)
		disable_irq_nosync(ts->client->irq);

	if(SynaDoReflash(ts->client, ts->fw_revision) == FW_REFLASH_SUCCEED)
	{
		SHOW_MSG("synaptics_ts_fw_reflash_work_func : SynaDoReflash succeed!\n");

		ret = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.firmware_revision_query);
		if (ret < 0) {
			ERR_MSG("i2c_smbus_read_byte_data failed\n");
		}
		DEBUG_MSG("synaptics_ts_fw_reflash_work_func: Firmware Revision 0x%x\n", ret);

		ret = i2c_smbus_read_word_data(ts->client, MAX_X_POS_LOW_REG);
		if (ret < 0) {
			ERR_MSG("i2c_smbus_read_word_data failed\n");
		}
		max_x = (ret & 0xFF);

		ret = i2c_smbus_read_word_data(ts->client, MAX_X_POS_HIGH_REG);
		if (ret < 0) {
			ERR_MSG("i2c_smbus_read_word_data failed\n");
		}
		max_x |= (((ret & 0xFF) << 8) & 0xff00);
		ts->max[0] = max_x;

		ret = i2c_smbus_read_word_data(ts->client, MAX_Y_POS_LOW_REG);
		if (ret < 0) {
			ERR_MSG("i2c_smbus_read_word_data failed\n");
		}
		max_y = (ret & 0xFF);

		ret = i2c_smbus_read_word_data(ts->client, MAX_Y_POS_HIGH_REG);
		if (ret < 0) {
			ERR_MSG("i2c_smbus_read_word_data failed\n");
		}
		max_y |= (((ret & 0xFF) << 8) & 0xff00);
		ts->max[1] = max_y;

		SHOW_MSG("synaptics_ts_fw_reflash_work_func : max_x = 0x%x\n",max_x);
		SHOW_MSG("synaptics_ts_fw_reflash_work_func : max_y = 0x%x\n",max_y);

		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 15, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
		ret = input_register_device(ts->input_dev);
		if (ret) {
			ERR_MSG("synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
			input_free_device(ts->input_dev);
		}
	}

	is_fw_reflash = 0;

	/* enable irq */
	if (ts->use_irq)
		enable_irq(ts->client->irq);

	return;
}
#endif

static void synaptics_ts_new_work_func(struct work_struct *work)
{
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work);

	int int_mode;
	int width0, width1;
	int touch2_prestate = 0;
	int touch1_prestate = 0;

	int tmp_x=0, tmp_y=0;
	int finger0_status=0, finger1_status=0;
#ifdef SYNAPTICS_ESD_RECOVERY
	int ret;
#endif
#ifdef TOUCH_TEST
	unsigned char reg_data1[11];
	unsigned char reg_data2[24];
	char tmp_buf[700];
	int fd_check, fd;
#endif

	int_mode = i2c_smbus_read_byte_data(ts->client, INT_STATUS_REG);

	DEBUG_MSG("synaptics_ts_new_work_func : int mode = 0x%x\n", int_mode);

#ifdef TOUCH_TEST
	if((fd_check = sys_open((const char __user *) "/sdcard/start_touch_logging", O_RDONLY, 0) ) >= 0)
	{
		sys_close(fd_check);

		memset(reg_data1, 0x00, sizeof(reg_data1));
		memset(reg_data2, 0x00, sizeof(reg_data2));
		memset(tmp_buf, 0x00, sizeof(tmp_buf));

		i2c_smbus_read_i2c_block_data(ts->client, 0x15, 11, reg_data1);
		i2c_smbus_read_i2c_block_data(ts->client, 0x2c, 24, reg_data2);

		if((fd = sys_open((const char __user *) "/sdcard/touch_reg.txt", O_CREAT | O_APPEND | O_WRONLY, 0)) >= 0)
		{
			sprintf(tmp_buf,"[0x15]= 0x%x,\t[0x16]= 0x%x,\t[0x17]= 0x%x,\t[0x18]= 0x%x,\t[0x19]= 0x%x\n\
[0x1a]= 0x%x,\t[0x1b]= 0x%x,\t[0x1c]= 0x%x,\t[0x1d]= 0x%x,\t[0x1e]= 0x%x\n\
[0x1f]= 0x%x,\t[0x2c]= 0x%x,\t[0x2d]= 0x%x,\t[0x2e]= 0x%x,\t[0x2f]= 0x%x\n\
[0x30]= 0x%x,\t[0x31]= 0x%x,\t[0x32]= 0x%x,\t[0x33]= 0x%x,\t[0x34]= 0x%x\n\
[0x35]= 0x%x,\t[0x36]= 0x%x,\t[0x37]= 0x%x,\t[0x38]= 0x%x,\t[0x39]= 0x%x\n\
[0x3a]= 0x%x,\t[0x3b]= 0x%x,\t[0x3c]= 0x%x,\t[0x3d]= 0x%x,\t[0x3e]= 0x%x\n\
[0x3f]= 0x%x,\t[0x40]= 0x%x,\t[0x41]= 0x%x,\t[0x42]= 0x%x,\t[0x43]= 0x%x\n\n",
				reg_data1[0], reg_data1[1], reg_data1[2], reg_data1[3], reg_data1[4],
				reg_data1[5], reg_data1[6], reg_data1[7], reg_data1[8], reg_data1[9],
				reg_data1[10], reg_data2[0], reg_data2[1], reg_data2[2], reg_data2[3],
				reg_data2[4], reg_data2[5], reg_data2[6], reg_data2[7], reg_data2[8],
				reg_data2[9], reg_data2[10], reg_data2[11], reg_data2[12], reg_data2[13],
				reg_data2[14], reg_data2[15], reg_data2[16], reg_data2[17], reg_data2[18],
				reg_data2[19], reg_data2[20], reg_data2[21], reg_data2[22], reg_data2[23]);

			sys_write(fd, (const char __user *) tmp_buf, strlen(tmp_buf)+1);
			sys_close(fd);
		}
	}
#endif

#ifdef SYNAPTICS_ESD_RECOVERY
	if((int_mode < 0) && (is_first_release_event != 1))
	{
		SHOW_MSG("synaptics_ts_new_work_func : death from ESD attack\n");

		if (ts->use_irq)
			disable_irq_nosync(ts->client->irq);
		else
			hrtimer_cancel(&ts->timer);

		msleep(100);

		ret = i2c_smbus_write_byte_data(ts->client, SYNAPTICS_CONTROL_REG, SYNAPTICS_CONTROL_SLEEP); /* sleep */
		if (ret < 0)
			ERR_MSG("synaptics_ts_suspend: i2c_smbus_write_byte_data failed\n");

		if (ts->power) {
			ret = ts->power(0);
			if (ret < 0)
				ERR_MSG("synaptics_ts_resume power off failed\n");
		}

		msleep(100);

		if (ts->power) {
			ret = ts->power(1);
			if (ret < 0)
				ERR_MSG("synaptics_ts_resume power on failed\n");
#ifdef SYNAPTICS_MELTINGMODE
			is_first_release_event = 1;
#endif
		}

		i2c_smbus_write_byte_data(ts->client, SYNAPTICS_CONTROL_REG, SYNAPTICS_CONTROL_NOSLEEP); /* wake up */

		if (ts->use_irq)
			enable_irq(ts->client->irq);
		else
			hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

		SHOW_MSG("synaptics_ts_new_work_func : death from ESD attack --> recovery action completed\n");

		goto SYNAPTICS_TS_IDLE;
	}
#endif

	if(int_mode & SYNAPTICS_INT_ABS0)
	{
		while (1)
		{
			i2c_smbus_read_i2c_block_data(ts->client, START_ADDR, sizeof(ts_reg_data), (u8 *)&ts_reg_data);

			finger0_status = TS_SNTS_GET_FINGER_STATE_0(ts_reg_data.finger_state_reg);
			finger1_status = TS_SNTS_GET_FINGER_STATE_1(ts_reg_data.finger_state_reg);

			DEBUG_MSG("synaptics_ts_new_work_func : finger0_status = 0x%x, finger1_status = 0x%x\n",finger0_status,finger1_status);

			if((finger0_status == 0) && (ts_pre_state == 0))
			{
				DEBUG_MSG("synaptics_ts_new_work_func: Synaptics Touch is is the idle state\n");
				goto SYNAPTICS_TS_IDLE;
			}

			if((finger0_status == 1))
			{
				ts_pre_state = 1;
			}
			else
			{
				ts_pre_state = 0;
			}

			if(finger0_status == 1)
			{
				touch1_prestate = 1;

				tmp_x = (int)TS_SNTS_GET_X_POSITION(ts_reg_data.X_high_position_finger0_reg, ts_reg_data.XY_low_position_finger0_reg);
				tmp_y = (int)TS_SNTS_GET_Y_POSITION(ts_reg_data.Y_high_position_finger0_reg, ts_reg_data.XY_low_position_finger0_reg);

				curr_ts_data.X_position[0] = tmp_x;
		  		curr_ts_data.Y_position[0] = tmp_y;

				if ((((ts_reg_data.XY_width_finger0_reg & 240) >> 4) - (ts_reg_data.XY_width_finger0_reg & 15)) > 0)
					width0 = (ts_reg_data.XY_width_finger0_reg & 240) >> 4;
				else
					width0 = ts_reg_data.XY_width_finger0_reg & 15;

	        	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, width0);
	       		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[0]);
        		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[0]);
				input_mt_sync(ts->input_dev);

				DEBUG_MSG("push : first_x= %d, first_y = %d, width = %d\n", curr_ts_data.X_position[0], curr_ts_data.Y_position[0], width0);
			}
			else if((finger0_status == 0) && (touch1_prestate == 1))
			{
				touch1_prestate = 0;

				tmp_x = (int)TS_SNTS_GET_X_POSITION(ts_reg_data.X_high_position_finger0_reg, ts_reg_data.XY_low_position_finger0_reg);
				tmp_y = (int)TS_SNTS_GET_Y_POSITION(ts_reg_data.Y_high_position_finger0_reg, ts_reg_data.XY_low_position_finger0_reg);

				curr_ts_data.X_position[0] = tmp_x;
		  		curr_ts_data.Y_position[0] = tmp_y;

				if ((((ts_reg_data.XY_width_finger0_reg & 240) >> 4) - (ts_reg_data.XY_width_finger0_reg & 15)) > 0)
					width0 = (ts_reg_data.XY_width_finger0_reg & 240) >> 4;
				else
					width0 = ts_reg_data.XY_width_finger0_reg & 15;

	        	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, width0);
	       		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[0]);
        		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[0]);
				input_mt_sync(ts->input_dev);

#ifdef SYNAPTICS_MELTINGMODE
				if(is_first_release_event == 1)
				{
					i2c_smbus_write_byte_data(ts->client, SYNAPTICS_MELTING_REG, SYNAPTICS_MELTING_AUTO);
					is_first_release_event = 0;
					DEBUG_MSG("chaning melting mode : melting --> auto melting\n");
				}
#endif

				DEBUG_MSG("release : first_x= %d, first_y = %d, width = %d\n", curr_ts_data.X_position[0], curr_ts_data.Y_position[0], width0);
			}
			else if(finger0_status == 0)
			{
#ifdef SYNAPTICS_MELTINGMODE
				if(is_first_release_event == 1)
				{
					i2c_smbus_write_byte_data(ts->client, SYNAPTICS_MELTING_REG, SYNAPTICS_MELTING_AUTO);
					is_first_release_event = 0;
					DEBUG_MSG("chaning melting mode : melting --> auto melting\n");
				}
#endif

				touch1_prestate = 0;
			}

			if(finger1_status == 1)
			{
				ts_pre_state = 1;
				touch2_prestate = 1;

				tmp_x = (int)TS_SNTS_GET_X_POSITION(ts_reg_data.X_high_position_finger1_reg, ts_reg_data.XY_low_position_finger1_reg);
				tmp_y = (int)TS_SNTS_GET_Y_POSITION(ts_reg_data.Y_high_position_finger1_reg, ts_reg_data.XY_low_position_finger1_reg);

				if ((((ts_reg_data.XY_width_finger1_reg & 240) >> 4) - (ts_reg_data.XY_width_finger1_reg & 15)) > 0)
					width1 = (ts_reg_data.XY_width_finger1_reg & 240) >> 4;
				else
					width1 = ts_reg_data.XY_width_finger1_reg & 15;

				curr_ts_data.X_position[1] = tmp_x;
			  	curr_ts_data.Y_position[1] = tmp_y;

				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, width1);
			    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[1]);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[1]);
				input_mt_sync(ts->input_dev);

				DEBUG_MSG("push : second_x= %d, second_y = %d, width = %d\n", curr_ts_data.X_position[1], curr_ts_data.Y_position[1], width1);
			}
			else if((finger1_status == 0) && (touch2_prestate == 1))
			{
				touch2_prestate = 0;

				tmp_x = (int)TS_SNTS_GET_X_POSITION(ts_reg_data.X_high_position_finger1_reg, ts_reg_data.XY_low_position_finger1_reg);
				tmp_y = (int)TS_SNTS_GET_Y_POSITION(ts_reg_data.Y_high_position_finger1_reg, ts_reg_data.XY_low_position_finger1_reg);

				if ((((ts_reg_data.XY_width_finger1_reg & 240) >> 4) - (ts_reg_data.XY_width_finger1_reg & 15)) > 0)
					width1 = (ts_reg_data.XY_width_finger1_reg & 240) >> 4;
				else
					width1 = ts_reg_data.XY_width_finger1_reg & 15;

				curr_ts_data.X_position[1] = tmp_x;
			  	curr_ts_data.Y_position[1] = tmp_y;

				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, width1);
			    input_report_abs(ts->input_dev, ABS_MT_POSITION_X, curr_ts_data.X_position[1]);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, curr_ts_data.Y_position[1]);
				input_mt_sync(ts->input_dev);

				DEBUG_MSG("release : second_x= %d, second_y = %d, width = %d\n", curr_ts_data.X_position[1], curr_ts_data.Y_position[1], width1);
			}
			else if(finger1_status == 0)
			{
				touch2_prestate = 0;
			}
			input_sync(ts->input_dev);

			if ((ts_pre_state == 0) || (power_status == 0))
			{
				break;
			}

			msleep(SYNAPTICS_TS_POLLING_TIME);
		}/* End of While(1) */
	}

SYNAPTICS_TS_IDLE:
	if (ts->use_irq)
	{
		enable_irq(ts->client->irq);
	}
}

static enum hrtimer_restart synaptics_ts_timer_func(struct hrtimer *timer)
{
	struct synaptics_ts_data *ts = container_of(timer, struct synaptics_ts_data, timer);

	queue_work(synaptics_wq, &ts->work);
		hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL); /* 12.5 msec */

    return HRTIMER_NORESTART;
}

static irqreturn_t synaptics_ts_irq_handler(int irq, void *dev_id)
{
	struct synaptics_ts_data *ts = dev_id;

	disable_irq_nosync(ts->client->irq);
	queue_work(synaptics_wq, &ts->work);

	return IRQ_HANDLED;
}

static void synaptics_ts_get_device_inform(int product_num)
{
	int i;
	unsigned char reg_block_num[CMD_REG_BLOCK_NUM]={0x00};

	switch(product_num)
	{
		case SYNAPTICS_2000:
			for(i=0;i<CMD_REG_BLOCK_NUM;i++)
				reg_block_num[i] = i+0x58;
			break;
		case SYNAPTICS_2100:
			for(i=0;i<CMD_REG_BLOCK_NUM;i++)
				reg_block_num[i]=i+0x5C;
			break;
		case SYNAPTICS_3000:
			for(i=0;i<CMD_REG_BLOCK_NUM;i++)
				reg_block_num[i]= i+0x70;
			break;
		default:
			for(i=0;i<CMD_REG_BLOCK_NUM;i++)
				reg_block_num[i]=0x00;
			ERR_MSG("synaptics_ts_get_device_inform : Not supported deivce!!\n");
			break;
	}
	memcpy(&ts_cmd_reg_data, &reg_block_num[0], CMD_REG_BLOCK_NUM);

	return;
}

/*************************************************************************************************
 * 1. Set interrupt configuration
 * 2. Disable interrupt
 * 3. Power up
 * 4. Read RMI Version
 * 5. Read Firmware version & Upgrade firmware automatically
 * 6. Read Data To Initialization Touch IC
 * 7. Set some register
 * 8. Enable interrupt
*************************************************************************************************/
static int synaptics_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct synaptics_ts_data *ts;
	int i;
	int ret = 0;
	int fuzz_x, fuzz_y, fuzz_p, fuzz_w;
	struct synaptics_i2c_rmi_platform_data *pdata;
	unsigned long irqflags;
	int inactive_area_left;
	int inactive_area_right;
	int inactive_area_top;
	int inactive_area_bottom;
	int snap_left_on;
	int snap_left_off;
	int snap_right_on;
	int snap_right_off;
	int snap_top_on;
	int snap_top_off;
	int snap_bottom_on;
	int snap_bottom_off;
#if 0
	uint32_t panel_version;
#endif
	int product_id_quwery_reg;
	char product_name[PRODUCT_ID_STRING_NUM];

	DEBUG_MSG("%s() -- start\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		ERR_MSG("synaptics_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;

		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}
#ifdef SYNAPTICS_FW_REFLASH
	INIT_WORK(&ts->work_for_reflash, synaptics_ts_fw_reflash_work_func);
#endif
	ts->client = client;
	i2c_set_clientdata(client, ts);
	pdata = client->dev.platform_data;

	if (pdata)
		ts->power = pdata->power;

	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0) {
			ERR_MSG("synaptics_ts_probe power on failed\n");
			goto err_power_failed;
		}
		msleep(500);
#ifdef SYNAPTICS_MELTINGMODE
		is_first_release_event = 1;
#endif
		power_status = 1;
	}

	for(i=0;i <PRODUCT_ID_STRING_NUM;i++)
		product_name[i] = '\0';

	ret = i2c_smbus_read_byte_data(ts->client, QUERY_BASE_REG);
	msleep(500);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_byte_data failed\n");
		if (ts->power) {
			ERR_MSG("[Touch] POWER_DOWN\n");
			ret = ts->power(0);
			return ret;
		}
	}
	product_id_quwery_reg= ret + 11;

	ret = i2c_smbus_read_i2c_block_data(ts->client, product_id_quwery_reg, sizeof(product_name)-1, product_name);
	if (ret < 0)
	{
		ERR_MSG("synaptics_ts_probe : i2c_smbus_read_i2c_block_data failed: product_id_quwery_reg\n");
	}
	SHOW_MSG("synaptics_ts_probe : product_name = %s\n",product_name);

	if(strcmp(product_name, "TM1590-001")==0)
	{
		kind_of_product = SYNAPTICS_2000;
	}
	else if(strcmp(product_name, "TM1610-001")==0)
	{
		kind_of_product = SYNAPTICS_2100;
	}
	else
	{
		kind_of_product = SYNAPTICS_3000;
	}

	synaptics_ts_get_device_inform(kind_of_product);
	if(kind_of_product != SYNAPTICS_2000)
	{
		DEBUG_MSG("synaptics_ts_probe : work function changed : synaptics_ts_new_work_func\n");
		INIT_WORK(&ts->work, synaptics_ts_new_work_func);
	}

	ret = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.customer_family_query);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_byte_data failed\n");
	}
	SHOW_MSG("synaptics_ts_probe: Customer family 0x%x\n", ret);

	ts->fw_revision = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.firmware_revision_query);
	if (ts->fw_revision < 0) {
		ERR_MSG("i2c_smbus_read_byte_data failed\n");
	}
	SHOW_MSG("synaptics_ts_probe: Firmware Revision 0x%x\n", ts->fw_revision);
	fw_rev = ts->fw_revision;
	if (pdata) {
#if 0
		while (pdata->version > panel_version)
			pdata++;
#endif
		ts->flags = pdata->flags;
		ts->sensitivity_adjust = pdata->sensitivity_adjust;
		irqflags = pdata->irqflags;
		inactive_area_left = pdata->inactive_left;
		inactive_area_right = pdata->inactive_right;
		inactive_area_top = pdata->inactive_top;
		inactive_area_bottom = pdata->inactive_bottom;
		snap_left_on = pdata->snap_left_on;
		snap_left_off = pdata->snap_left_off;
		snap_right_on = pdata->snap_right_on;
		snap_right_off = pdata->snap_right_off;
		snap_top_on = pdata->snap_top_on;
		snap_top_off = pdata->snap_top_off;
		snap_bottom_on = pdata->snap_bottom_on;
		snap_bottom_off = pdata->snap_bottom_off;
		fuzz_x = pdata->fuzz_x;
		fuzz_y = pdata->fuzz_y;
		fuzz_p = pdata->fuzz_p;
		fuzz_w = pdata->fuzz_w;
	} else {
		irqflags = 0;
		inactive_area_left = 0;
		inactive_area_right = 0;
		inactive_area_top = 0;
		inactive_area_bottom = 0;
		snap_left_on = 0;
		snap_left_off = 0;
		snap_right_on = 0;
		snap_right_off = 0;
		snap_top_on = 0;
		snap_top_off = 0;
		snap_bottom_on = 0;
		snap_bottom_off = 0;
		fuzz_x = 0;
		fuzz_y = 0;
		fuzz_p = 0;
		fuzz_w = 0;
	}

  	memset(&ts_reg_data, 0x0, sizeof(ts_sensor_data));
  	memset(&curr_ts_data, 0x0, sizeof(ts_finger_data));

	ret = i2c_smbus_read_word_data(ts->client, MAX_X_POS_LOW_REG);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_word_data failed\n");
	}
	max_x = (ret & 0xFF);

	ret = i2c_smbus_read_word_data(ts->client, MAX_X_POS_HIGH_REG);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_word_data failed\n");
	}
	max_x |= (((ret & 0xFF) << 8) & 0xff00);
	ts->max[0] = max_x;

	ret = i2c_smbus_read_word_data(ts->client, MAX_Y_POS_LOW_REG);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_word_data failed\n");
	}
	max_y = (ret & 0xFF);

	ret = i2c_smbus_read_word_data(ts->client, MAX_Y_POS_HIGH_REG);
	if (ret < 0) {
		ERR_MSG("i2c_smbus_read_word_data failed\n");
	}
	max_y |= (((ret & 0xFF) << 8) & 0xff00);
	ts->max[1] = max_y;

	SHOW_MSG("synaptics_ts_probe : max_x = 0x%x\n",max_x);
	SHOW_MSG("synaptics_ts_probe : max_y = 0x%x\n",max_y);

#ifdef SYNAPTICS_FW_REFLASH
	if(kind_of_product == SYNAPTICS_3000)
	{
		if((max_x == 0x00) || (max_y == 0x00))
		{
			ERR_MSG("F/W image is not normal status : need upgrade.\n");
			is_need_forced_update = 1;
		}
	}
#endif

	ret = i2c_smbus_read_i2c_block_data(ts->client, START_ADDR, sizeof(ts_reg_data), (u8 *)&ts_reg_data);
	if (ret < 0)
	{
		ERR_MSG("synaptics_ts_probe : i2c_smbus_read_i2c_block_data failed: START_ADDR\n");
	}
	DEBUG_MSG("synaptics_ts_probe : status_reg(%d), interrupt_status_reg(%d,)\n", ts_reg_data.device_status_reg, ts_reg_data.interrupt_status_reg);

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		ERR_MSG("synaptics_ts_probe: Failed to allocate input device\n");

		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "synaptics-rmi-ts";
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);

	if(is_need_forced_update != 1)
	{
#if 0 /* this routine should be checked, later */
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x, fuzz_x, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y, fuzz_y, 0);
#else
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x+1, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y+1, 0, 0);
#endif
		input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 15, fuzz_p, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, fuzz_w, 0);

		ret = input_register_device(ts->input_dev);
		if (ret) {
			ERR_MSG("synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
			goto err_input_register_device_failed;
		}
	}
	DEBUG_MSG("########## irq [%d], irqflags[0x%x]\n", client->irq, irqflags);

	if (client->irq)
	{
		ret = request_irq(client->irq, synaptics_ts_irq_handler, irqflags, client->name, ts);

		if (ret == 0)
		{
			ts->use_irq = 1;
			DEBUG_MSG("request_irq\n");
		}
		else
		{
			dev_err(&client->dev, "request_irq failed\n");
		}
	}
	if (!ts->use_irq)
	{
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN -40;
	ts->early_suspend.suspend = synaptics_ts_early_suspend;
	ts->early_suspend.resume = synaptics_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

#ifdef SYNAPTICS_FW_REFLASH
	if(synaptics_fwdl_wq)
		queue_work(synaptics_fwdl_wq, &ts->work_for_reflash);
#endif

	SHOW_MSG("synaptics_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");
	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
err_power_failed:
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int synaptics_ts_remove(struct i2c_client *client)
{
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	if(is_fw_reflash == 1)
	{
		SHOW_MSG("synaptics_ts_suspend: during f/w image update - block suspend\n");
		return 0;
	}

	DEBUG_MSG("synaptics_ts_suspend : enter!!\n");

	if (ts->use_irq)
		disable_irq_nosync(client->irq);
	else
		hrtimer_cancel(&ts->timer);

	i2c_smbus_write_byte_data(ts->client, SYNAPTICS_CONTROL_REG, SYNAPTICS_CONTROL_SLEEP); /* sleep */

	if (ts->power) {
		ret = ts->power(0);
		if (ret < 0)
			ERR_MSG("synaptics_ts_resume power off failed\n");
	}

#ifdef SYNAPTICS_MELTINGMODE
	is_first_release_event = 0;
#endif
	power_status = 0;

	return 0;
}

static int synaptics_ts_resume(struct i2c_client *client)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	DEBUG_MSG("synaptics_ts_resume : enter!!\n");

	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0)
			ERR_MSG("synaptics_ts_resume power on failed\n");
	}

    i2c_smbus_write_byte_data(ts->client, SYNAPTICS_CONTROL_REG, SYNAPTICS_CONTROL_NOSLEEP); /* wake up */

	if (ts->use_irq)
		enable_irq(client->irq);
	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

#ifdef SYNAPTICS_MELTINGMODE
	is_first_release_event = 1;
#endif
	power_status = 1;

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void synaptics_ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void synaptics_ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id synaptics_ts_id[] = {
	{ "synaptics-rmi-ts", 0 },
	{ }
};

static struct i2c_driver synaptics_ts_driver = {
	.probe		= synaptics_ts_probe,
	.remove		= synaptics_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= synaptics_ts_suspend,
	.resume		= synaptics_ts_resume,
#endif
	.id_table	= synaptics_ts_id,
	.driver = {
		.name	= "synaptics-rmi-ts",
        .owner = THIS_MODULE,
	},
};

static int __devinit synaptics_ts_init(void)
{
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
#ifdef SYNAPTICS_FW_REFLASH
	synaptics_fwdl_wq = create_singlethread_workqueue("synaptics_fwdl_wq");
#endif

	DEBUG_MSG ("Synaptics ts_init\n");

#ifdef SYNAPTICS_FW_REFLASH
	if ((!synaptics_wq)||(!synaptics_fwdl_wq))
		return -ENOMEM;
#else
	if (!synaptics_wq)
		return -ENOMEM;
#endif

	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
	i2c_del_driver(&synaptics_ts_driver);

	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);

#ifdef SYNAPTICS_FW_REFLASH
	if (synaptics_fwdl_wq)
		destroy_workqueue(synaptics_fwdl_wq);
#endif
}

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");


