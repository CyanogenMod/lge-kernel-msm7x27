/*
 * MELFAS mcs6000 touchscreen driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include "touch_mcs6000_down_ioctl.h"
#include <linux/i2c-gpio.h>
#include <mach/board_lge.h>

#ifdef CONFIG_LGE_DIAGTEST
#include <mach/lge_diag_test.h>
#endif

#if defined (CONFIG_LGE_TOUCH_RECORDING)
/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
#include <linux/lge_alohag_at.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>

static struct early_suspend ts_early_suspend;
static void mcs6000_early_suspend(struct early_suspend *h);
static void mcs6000_late_resume(struct early_suspend *h);
#endif

#define LG_FW_MULTI_TOUCH
#define LG_FW_TOUCH_SOFT_KEY 1
#define TOUCH_SEARCH    247
#define TOUCH_BACK      248

/* shoud be checked, what is the difference, TOUCH_SEARCH and KEY_SERACH, TOUCH_BACK  and KEY_BACK */
//#define LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY

#define TS_POLLING_TIME 5 /* msec */

#define DEBUG_TS 0 /* enable or disable debug message */
#if DEBUG_TS
#define DMSG(fmt, args...) printk(KERN_DEBUG fmt, ##args)
#else
#define DMSG(fmt, args...) do{} while(0)
#endif

#define ON 	1
#define OFF 	0

#define PRESSED 	1
#define RELEASED 	0

#define MCS6000_TS_INPUT_INFO					0x10
#define MCS6000_TS_XY_HIGH						0x11
#define MCS6000_TS_X_LOW						0x12
#define MCS6000_TS_Y_LOW						0x13
#define MCS6000_TS_Z				 			0x14
#define MCS6000_TS_XY2_HIGH				 		0x15
#define MCS6000_TS_X2_LOW					 	0x16
#define MCS6000_TS_Y2_LOW	  					0x17
#define MCS6000_TS_Z2			 				0x18
#define MCS6000_TS_KEY_STRENGTH  				0x19
#define MCS6000_TS_FW_VERSION			 		0x20
#define MCS6000_TS_HW_REVISION					0x21

struct mcs6000_ts_device {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct delayed_work work;
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
};

static struct input_dev *mcs6000_ts_input = NULL;
static struct mcs6000_ts_device mcs6000_ts_dev; 


#define READ_NUM 8 /* now, just using two finger data */

enum{
	NON_TOUCHED_STATE,
	SINGLE_POINT_TOUCH,
	MULTI_POINT_TOUCH,
	MAX_TOUCH_TYPE
};

enum{
	NO_KEY_TOUCHED,
	KEY1_TOUCHED,
	KEY2_TOUCHED,
	KEY3_TOUCHED,
	MAX_KEY_TOUCH
};

#if defined (CONFIG_LGE_TOUCH_RECORDING)
/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
extern unsigned long int ats_mtc_log_mask;

//ACTION filed information
typedef enum{
	ETA_TOUCH_MOVETO = 0, /*Move the pointer to the specified location*/
	ETA_TOUCH_MOVEBY = 1, /*Move the pointer by the specified values*/
	ETA_TOUCH_TAB = 2, /*Tab at the current location*/
	ETA_TOUCH_DOUBLETAB = 3, /*Double tab at the current location*/
	ETA_TOUCH_DOWN = 4, /*Touch down at the current location*/
	ETA_TOUCH_UP = 5, /*Touch up at the current location*/
	ETA_TOUCH_DEFAULT = 0xff,
}eta_touch_event_action_type;

static char eta_prev_action = ETA_TOUCH_DEFAULT;

// LG_FW : 2010.01.22 hoonylove004 -------------------------------------------
//extern struct ats_mtc_key_log_type ats_mtc_key_log;
struct ats_mtc_key_log_type ats_mtc_key_log;
void ats_mtc_send_key_log_to_eta(struct ats_mtc_key_log_type* p_ats_mtc_key_log);
// ---------------------------------------------------------------------------

static char ats_mtc_key_state = 0; // 0 : release, 1 : press

/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
#endif

static __inline void mcs6000_key_event_touch(int touch_reg,  int value,  struct mcs6000_ts_device *dev)
{
	unsigned int keycode;

	if (touch_reg == KEY1_TOUCHED) {
#if defined(LG_FW_TOUCH_SOFT_KEY) || defined(LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY)
		keycode = TOUCH_SEARCH;
#else
		keycode = KEY_SEARCH;
#endif
	} else if (touch_reg == KEY2_TOUCHED) {
		keycode = -1; /* not used, now */
	} else if (touch_reg == KEY3_TOUCHED) {
#if defined(LG_FW_TOUCH_SOFT_KEY) || defined(LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY)
		keycode = TOUCH_BACK;
#else
		keycode = KEY_BACK;
#endif
	} else {
		printk(KERN_INFO "%s Not available touch key reg. %d\n", __FUNCTION__, touch_reg);
		return;
	}
	input_report_key(dev->input_dev, keycode, value);
	input_sync(dev->input_dev);

	DMSG("%s Touch Key Code %d, Value %d\n", __FUNCTION__, keycode, value);

#if defined (CONFIG_LGE_TOUCH_RECORDING)
	/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
	if((ats_mtc_log_mask&0x00000001) != 0)//ETA_LOGITEM_KEY
	{	
		ats_mtc_key_log.log_id = 1; //LOG_ID, 1 key, 2 touch
		ats_mtc_key_log.log_len = 18; //LOG_LEN
		ats_mtc_key_log.x_hold = value; // hold
		if(touch_reg == KEY1_TOUCHED)
			ats_mtc_key_log.y_code = KEY_SEARCH;
		else if(touch_reg == KEY3_TOUCHED)
			ats_mtc_key_log.y_code = KEY_BACK;

		if(ats_mtc_key_state != value)
		{
			ats_mtc_send_key_log_to_eta(&ats_mtc_key_log);
			ats_mtc_key_state = value;
		}

	}
	/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
#endif
	return;
}

#ifdef LG_FW_MULTI_TOUCH
static __inline void mcs6000_multi_ts_event_touch(int x1, int y1, int x2, int y2, int value,
		struct mcs6000_ts_device *dev)
{
	int report = 0;

	if ((x1 >= 0) && (y1 >= 0)) {
		input_report_abs(dev->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_X, x1);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_Y, y1);
		input_mt_sync(dev->input_dev);
		report = 1;
	}

	if ((x2 >= 0) && (y2 >= 0)) {
		input_report_abs(dev->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_X, x2);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_Y, y2);
		input_mt_sync(dev->input_dev);
		report = 1;
	}

	if (report != 0) {
		input_sync(dev->input_dev);
	} else {
		printk(KERN_WARNING "%s: Not Available touch data x1=%d, y1=%d, x2=%d, y2=%d\n", 
				__FUNCTION__,  x1, y1, x2, y2);
	}
	return;
}

#else

static __inline void mcs6000_single_ts_event_touch(unsigned int x, unsigned int y, int value,
				   struct mcs6000_ts_device *dev)
{
	int report = 0;
	if ((x >= 0) && (y >= 0)) {
		input_report_abs(dev->input_dev, ABS_X, x);
		input_report_abs(dev->input_dev, ABS_Y, y);
		reprot = 1;
	}

	if (report != 0) {
		input_report_key(dev->input_dev, BTN_TOUCH, value);
		input_sync(dev->input_dev);
	} else {
		DMSG(KERN_WARNING "%s: Not Available touch data x=%d, y=%d\n", __FUNCTION__, x, y); 
	}

	return;
}

static __inline void mcs6000_single_ts_event_release(struct mcs6000_ts_device *dev)
{
	input_report_key(dev->input_dev, BTN_TOUCH, 0);
	input_sync(dev->input_dev);

	return;
}
#endif /* end of LG_FW_MULTI_TOUCH */

#if defined (CONFIG_LGE_TOUCH_RECORDING)
static __inline void ats_eta_mtc_key_logging (int pendown, int x1, int y1, int x2, int y2)
{
	ats_mtc_key_log.log_id = 2; //LOG_ID, 1 key, 2 touch
	ats_mtc_key_log.log_len = 22; //LOG_LEN

	if (!pendown) // release
	{
		ats_mtc_key_log.action = (unsigned char)ETA_TOUCH_UP;
		eta_prev_action = ETA_TOUCH_UP;
	}
	else // down
	{
		if(eta_prev_action == ETA_TOUCH_DOWN)
		{
			ats_mtc_key_log.action = (unsigned char)ETA_TOUCH_MOVETO;
			/*
			  			if(g_diag_mtc_check == 1)
			 			mdelay(50); // do not need to response all move to events
			 */
		}
		else
			ats_mtc_key_log.action = (unsigned char)ETA_TOUCH_DOWN;
		eta_prev_action = ETA_TOUCH_DOWN;
	}

#ifdef LG_FW_MULTI_TOUCH
	if((x2 > 0) && (y2 >0 )) // multi touch
	{
		ats_mtc_key_log.x_hold = x2;
		ats_mtc_key_log.y_code = y2;
	}
	else
	{
		ats_mtc_key_log.x_hold = x1;
		ats_mtc_key_log.y_code = y1;
	}
#else /*#ifdef LG_FW_MULTI_TOUCH*/
		ats_mtc_key_log.x_hold = x1;
		ats_mtc_key_log.y_code = y1;
#endif /*#ifdef LG_FW_MULTI_TOUCH*/

	printk(KERN_INFO "[ETA] TOUCH X : %d, Y : %d \n", ats_mtc_key_log.x_hold, ats_mtc_key_log.y_code);
	ats_mtc_send_key_log_to_eta(&ats_mtc_key_log);
}
#endif

// LG_FW : 2010.01.06 hoonylove004 -------------------------------------------

#ifdef CONFIG_LGE_DIAGTEST

/*	27 bytes for key log -> will be encoded to ?? byte (base64)
	CMD_CODE		1 (0xF0)
	SUB_CMD 		1 (0x08)
	LOG_ID			1 (1 key, 2 touch)
	LOG_LEN 		2 (data length in bytes)
	LOG_DATA		LOG_LEN = 22
	- TIME		8 (timestamp in milliseconds)
	- SCREEN_ID 1
	- ACTION	1 (Touch-action Type)
	- X 			2 (Absolute X Coordinate)
	- Y 			2 (Absolute Y Coordinate)
	- ACTIVE_UIID 8 (Activated UI ID)
*/
static __inline void ats_eta_mtc_key_logging (int pendown, int x1, int y1, int x2, int y2)
{
	unsigned char *eta_cmd_buf = NULL;
	unsigned char *eta_cmd_buf_encoded = NULL;
	int index =0;
	int lenb64 = 0;
	int exec_result = 0;

	if((ats_mtc_log_mask&0x00000002) != 0) // LOGITEM_TOUCHPAD
	{
		eta_cmd_buf = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
		eta_cmd_buf_encoded = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
		memset(eta_cmd_buf,0x00, 50);
		memset(eta_cmd_buf_encoded,0x00, 50);

		index = 0;
		eta_cmd_buf[index++] = (unsigned char)0xF0; //MTC_CMD_CODE
		eta_cmd_buf[index++] = (unsigned char)0x08; //MTC_LOG_REQ_CMD
		eta_cmd_buf[index++] = (unsigned char)2; //LOG_ID, 1 key, 2 touch
		eta_cmd_buf[index++] = (unsigned char)22; //LOG_LEN
		eta_cmd_buf[index++] = (unsigned char)0; //LOG_LEN
		//LOG_DATA 22bytes
		for(index = 5; index<27; index++)
		{
			eta_cmd_buf[index] = 0;
		}

		index = 13;
		eta_cmd_buf[index++] = (unsigned char)1; // MAIN LCD

/*	ACTION filed information
	MOVETO		0
	MOVEBY		1
	TAB			2
	DOUBLETAB	3
	DOWN		4
	UP			5
*/
		if (!pendown) // release
			eta_cmd_buf[index++] = (unsigned char)5;
		else
			eta_cmd_buf[index++] = (unsigned char)4; // touch

		eta_cmd_buf[index++] = (unsigned char)(x1&0xFF);// index = 15
		eta_cmd_buf[index++] = (unsigned char)((x1>>8)&0xFF);// index = 16

		eta_cmd_buf[index++] = (unsigned char)(y1&0xFF);// index = 17
		eta_cmd_buf[index++] = (unsigned char)((y1>>8)&0xFF);// index = 18

		printk(KERN_INFO "[ETA] TOUCH X : %d, Y : %d \n", x1, y1);

		lenb64 = base64_encode((char *)eta_cmd_buf, index, (char *)eta_cmd_buf_encoded);

		exec_result = eta_execute(eta_cmd_buf_encoded);
		printk(KERN_INFO "[ETA]AT+MTC exec_result %d\n",exec_result);

#ifdef LG_FW_MULTI_TOUCH
		if((x2 >= 0) && (y2 >= 0))
		{
			eta_cmd_buf[index++] = (unsigned char)(x2&0xFF);// index = 15
			eta_cmd_buf[index++] = (unsigned char)((x2>>8)&0xFF);// index = 16

			eta_cmd_buf[index++] = (unsigned char)(y2&0xFF);// index = 17
			eta_cmd_buf[index++] = (unsigned char)((y2>>8)&0xFF);// index = 18

			printk(KERN_INFO "[ETA] DOUAL TOUCH X2 : %d, Y2 : %d \n", x2, y2);

			lenb64 = base64_encode((char *)eta_cmd_buf, index, (char *)eta_cmd_buf_encoded);

			exec_result = eta_execute(eta_cmd_buf_encoded);
			printk(KERN_INFO "[ETA]AT+MTC exec_result %d\n",exec_result);
		}
#endif /*LG_FW_MULTI_TOUCH*/

		kfree(eta_cmd_buf);
		kfree(eta_cmd_buf_encoded);				
	}
}
 
#endif /*CONFIG_LGE_DIAGTEST*/
// ---------------------------------------------------------------------------


#define to_delayed_work(_work)  container_of(_work, struct delayed_work, work)

static void mcs6000_work(struct work_struct *work)
{
	int x1=0, y1 = 0;
#ifdef LG_FW_MULTI_TOUCH
	int x2=0, y2 = 0;
	static int pre_x1, pre_x2, pre_y1, pre_y2;
	static unsigned int s_input_type = NON_TOUCHED_STATE;
#endif
	unsigned int input_type;
	unsigned int key_touch;	
	unsigned char read_buf[READ_NUM];

	static int key_pressed = 0;
	static int touch_pressed = 0;
	
#if defined (CONFIG_LGE_TOUCH_RECORDING)
	int touch_state_eta = -1  ;  /* 0 is touch , 1 is key */
#endif

	struct mcs6000_ts_device *dev 
		= container_of(to_delayed_work(work), struct mcs6000_ts_device, work);

	dev->pendown = !gpio_get_value(dev->intr_gpio);

	/* read the registers of MCS6000 IC */
	if ( i2c_smbus_read_i2c_block_data(dev->client, MCS6000_TS_INPUT_INFO, READ_NUM, read_buf) < 0) {
		printk(KERN_ERR "%s touch ic read error\n", __FUNCTION__);
		goto touch_retry;
	}

	input_type = read_buf[0] & 0x0f;
	key_touch = (read_buf[0] & 0xf0) >> 4;

	x1 = y1 =0;
#ifdef LG_FW_MULTI_TOUCH
	x2 = y2 = 0;
#endif
	x1 = (read_buf[1] & 0xf0) << 4;
	y1 = (read_buf[1] & 0x0f) << 8;

	x1 |= read_buf[2];	
	y1 |= read_buf[3];		

#ifdef LG_FW_MULTI_TOUCH
	if(input_type == MULTI_POINT_TOUCH) {
		s_input_type = input_type;
		x2 = (read_buf[5] & 0xf0) << 4;
		y2 = (read_buf[5] & 0x0f) << 8;
		x2 |= read_buf[6];
		y2 |= read_buf[7];
	}
#endif

	if (dev->pendown) { /* touch pressed case */
		if(key_touch) {
			mcs6000_key_event_touch(key_touch, PRESSED, dev);
			key_pressed = key_touch;
#if defined (CONFIG_LGE_TOUCH_RECORDING)
			touch_state_eta = 1  ;  /* 0 is touch , 1 is key */
#endif
		}

		if(input_type) {
			touch_pressed = 1;

			/* exceptional routine for the touch case moving from key area to touch area of touch screen */
			if(key_pressed) {
				mcs6000_key_event_touch(key_pressed, RELEASED, dev);
				key_pressed = 0;
#if defined (CONFIG_LGE_TOUCH_RECORDING)
				touch_state_eta = 1  ;  /* 0 is touch , 1 is key */
#endif
			}
#if defined (CONFIG_LGE_TOUCH_RECORDING)
			touch_state_eta = 0  ;  /* 0 is touch , 1 is key */
#endif
#ifdef LG_FW_MULTI_TOUCH
			if(input_type == MULTI_POINT_TOUCH) {
				mcs6000_multi_ts_event_touch(x1, y1, x2, y2, PRESSED, dev);
				pre_x1 = x1;
				pre_y1 = y1;
				pre_x2 = x2;
				pre_y2 = y2;
			}
			else if(input_type == SINGLE_POINT_TOUCH) {
				mcs6000_multi_ts_event_touch(x1, y1, -1, -1, PRESSED, dev);
				s_input_type = SINGLE_POINT_TOUCH;				
			}
#else
			if(input_type == SINGLE_POINT_TOUCH) {
				mcs6000_single_ts_event_touch(x1, y1, PRESSED, dev);
			}
#endif				
		}
	} else { /* touch released case */
		if(key_pressed) {
			mcs6000_key_event_touch(key_pressed, RELEASED, dev);
			key_pressed = 0;
#if defined (CONFIG_LGE_TOUCH_RECORDING)
			touch_state_eta = 1  ;  /* 0 is touch , 1 is key */
#endif
		}

		if(touch_pressed) {
#if defined (CONFIG_LGE_TOUCH_RECORDING)
			touch_state_eta = 0  ;  /* 0 is touch , 1 is key */
#endif
#ifdef LG_FW_MULTI_TOUCH
			if(s_input_type == MULTI_POINT_TOUCH) {
				DMSG("%s: multi touch release...(%d, %d), (%d, %d)\n", __FUNCTION__,pre_x1,pre_y1,pre_x2,pre_y2);
				mcs6000_multi_ts_event_touch(pre_x1, pre_y1, pre_x2, pre_y2, RELEASED, dev);
				s_input_type = NON_TOUCHED_STATE; 
				pre_x1 = -1; pre_y1 = -1; pre_x2 = -1; pre_y2 = -1;
			} else {
				DMSG("%s: single touch release... %d, %d\n", __FUNCTION__, x1, y1);
				mcs6000_multi_ts_event_touch(x1, y1, -1, -1, RELEASED, dev);
			}
#else
			DMSG("%s: single release... %d, %d\n", __FUNCTION__, x1, y1);
			mcs6000_single_ts_event_touch (x1, y1, RELEASED, dev);
			touch_pressed = 0;
#endif
		}
	}

#if defined (CONFIG_LGE_DIAGTEST) 
#ifdef LG_FW_MULTI_TOUCH
	ats_eta_mtc_key_logging(dev->pendown, x1, y1, x2, y2); /* Multi Touch */
#else
	ats_eta_mtc_key_logging(dev->pendown, x1, y1, -1, -1); /* Single Touch */
#endif
#endif

#if defined (CONFIG_LGE_TOUCH_RECORDING)
 	if( (touch_state_eta ==0 )&& ((ats_mtc_log_mask&0x00000002) != 0))   {
#ifdef LG_FW_MULTI_TOUCH
	ats_eta_mtc_key_logging(dev->pendown, x1, y1, x2, y2); /* Multi Touch */
#else
	ats_eta_mtc_key_logging(dev->pendown, x1, y1, -1, -1); /* Single Touch */
#endif
	}
#endif

touch_retry:
	if (dev->pendown) {
		schedule_delayed_work(&dev->work, msecs_to_jiffies(TS_POLLING_TIME));	
	} else {
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
}

static irqreturn_t mcs6000_ts_irq_handler(int irq, void *handle)
{
	struct mcs6000_ts_device *dev = handle;

	if (gpio_get_value(dev->intr_gpio) == 0) {
		disable_irq_nosync(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		schedule_delayed_work(&dev->work, 0);
	}

	return IRQ_HANDLED;
}

static int mcs6000_ts_on(void)
{
	struct mcs6000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs6000_ts_dev;

	ret = dev->power(ON);
	if(ret < 0)	{
		printk(KERN_ERR "mcs6000_ts_on power on failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}

void mcs6000_firmware_info(void)
{
	unsigned char data;
	struct mcs6000_ts_device *dev = NULL;
	dev = &mcs6000_ts_dev;

	msleep(200);
	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_FW_VERSION);
	printk(KERN_INFO "MCS6000 F/W Version [0x%x]\n", data);

	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_HW_REVISION);
	printk(KERN_INFO "MCS6000 H/W Revision [0x%x]\n", data);
}

static __inline int mcs6000_ts_ioctl_down_i2c_write(unsigned char addr,
				    unsigned char val)
{
	int err = 0;
	struct i2c_client *client;
	struct i2c_msg msg;

	client = mcs6000_ts_dev.client;
	if (client == NULL) {
		DMSG("\n%s: i2c client error \n", __FUNCTION__);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &val;

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		DMSG("\n%s: i2c write error [%d]\n", __FUNCTION__, err);
	}

	return err;
}

static __inline int mcs6000_ts_ioctl_down_i2c_read(unsigned char addr,
				   unsigned char *ret)
{
	int err = 0;
	struct i2c_client *client;
	struct i2c_msg msg;

	client = mcs6000_ts_dev.client;
	if (client == NULL) {
		DMSG("\n%s: i2c client drror \n", __FUNCTION__);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 1;
	msg.len = 1;
	msg.buf = ret;

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		DMSG("\n%s: i2c read error [%d]\n", __FUNCTION__, err);
	}

	return err;
}

int mcs6000_ts_ioctl_down(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct mcs6000_ts_down_ioctl_i2c_type client_data;
	struct mcs6000_ts_device *dev = NULL;

	dev = &mcs6000_ts_dev;

	//printk(KERN_INFO"%d\n", _IOC_NR(cmd));
	if (_IOC_NR(cmd) >= MCS6000_TS_DOWN_IOCTL_MAXNR)
		return -EINVAL;

	switch (cmd) {
		case MCS6000_TS_DOWN_IOCTL_VDD_HIGH:
			err = dev->power(ON);
			if( err < 0 )
				printk(KERN_INFO"%s: Power On Fail....\n", __FUNCTION__);
			break;

		case MCS6000_TS_DOWN_IOCTL_VDD_LOW:
			err = dev->power(OFF);
			if( err < 0 )
				printk(KERN_INFO"%s: Power Down Fail..\n",  __FUNCTION__);
			break;

		case MCS6000_TS_DOWN_IOCTL_INTR_HIGH:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_LOW:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_OUT:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_IN:
			gpio_configure(dev->intr_gpio, GPIOF_INPUT);
			break;

		case MCS6000_TS_DOWN_IOCTL_SCL_HIGH:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_LOW:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_HIGH:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_LOW:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_OUT:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_OUT:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT);
			break;

		case MCS6000_TS_DOWN_IOCTL_I2C_ENABLE:
			//mcs6000_ts_down_i2c_block_enable(1);
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_DISABLE:
			//mcs6000_ts_down_i2c_block_enable(0);
			break;

		case MCS6000_TS_DOWN_IOCTL_I2C_READ:
			if (copy_from_user(&client_data, (struct mcs6000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copyfromuser error\n", __FUNCTION__);
				return -EFAULT;
			}

			if (0 > mcs6000_ts_ioctl_down_i2c_read( (unsigned char)client_data.addr,
						(unsigned char *)&client_data.data)) {
				err = -EIO;
			}

			if (copy_to_user((void *)arg, (const void *)&client_data,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copytouser error\n",
						__FUNCTION__);
				err = -EFAULT;
			}
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_WRITE:
			if (copy_from_user(&client_data, (struct mcs6000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copyfromuser error\n", __FUNCTION__);
				return -EFAULT;
			}

			if (0 > mcs6000_ts_ioctl_down_i2c_write((unsigned char)client_data.addr,
						(unsigned char)client_data.data)) {
				err = -EIO;
			}
			break;
		case MCS6000_TS_DOWN_IOCTL_SELECT_TS_TYPE:
			break;
		default:
			err = -EINVAL;
			break;
	}

	if (err < 0)
		printk(KERN_ERR "\n==== Touch DONW IOCTL Fail....%d\n",_IOC_NR(cmd));

	return err;
}

static int mcs6000_ts_ioctl(struct inode *inode, struct file *flip,
		     unsigned int cmd, unsigned long arg)
{
	int err = -1;
	/* int size; */

	switch (_IOC_TYPE(cmd)) {
		case MCS6000_TS_DOWN_IOCTL_MAGIC:
			err = mcs6000_ts_ioctl_down(inode, flip, cmd, arg);
			break;
		default:
			printk(KERN_ERR "%s unknow ioctl\n", __FUNCTION__);
			err = -EINVAL;
			break;
	}
	return err;
}

static int mcs6000_ioctl_open(struct inode *inode, struct file *flip) {
	disable_irq(mcs6000_ts_dev.num_irq);
	printk(KERN_INFO "touch irq disabled by ioctl\n");
	return 0;
}

static int mcs6000_ioctl_release(struct inode *inode, struct file *flip) {
	enable_irq(mcs6000_ts_dev.num_irq);
	printk(KERN_INFO "touch irq enabled by ioctl\n");
	return 0;
}

static struct file_operations mcs6000_ts_ioctl_fops = {
	.owner = THIS_MODULE,
	.ioctl = mcs6000_ts_ioctl,
	.open  = mcs6000_ioctl_open,
	.release = mcs6000_ioctl_release,
};

static struct miscdevice mcs6000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcs6000-touch",
	.fops = &mcs6000_ts_ioctl_fops,
};

static int mcs6000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct touch_platform_data *ts_pdata;
	struct mcs6000_ts_device *dev;

	DMSG("%s: start...\n", __FUNCTION__);

	ts_pdata = client->dev.platform_data;

#ifdef LG_FW_MULTI_TOUCH
	input_set_abs_params(mcs6000_ts_input, ABS_MT_POSITION_X, ts_pdata->ts_x_min, ts_pdata->ts_x_max, 0, 0);
	input_set_abs_params(mcs6000_ts_input, ABS_MT_POSITION_Y, ts_pdata->ts_y_min, ts_pdata->ts_y_max, 0, 0);
#else	
	input_set_abs_params(mcs6000_ts_input, ABS_X, ts_pdata->ts_x_min, ts_pdata->ts_x_max, 0, 0);
	input_set_abs_params(mcs6000_ts_input, ABS_Y, ts_pdata->ts_y_min, ts_pdata->ts_y_max, 0, 0);
#endif

	dev = &mcs6000_ts_dev;

	INIT_DELAYED_WORK(&dev->work, mcs6000_work);

	dev->power = ts_pdata->power;	
	dev->num_irq = client->irq;
	dev->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	dev->sda_gpio = ts_pdata->sda;
	dev->scl_gpio  = ts_pdata->scl;

	dev->input_dev = mcs6000_ts_input;
	DMSG("mcs6000 dev->num_irq is %d , dev->intr_gpio is %d\n", dev->num_irq,dev->intr_gpio);

	dev->client = client;
	i2c_set_clientdata(client, dev);

	if (!(err = i2c_check_functionality(client->adapter, I2C_FUNC_I2C))) {
		printk(KERN_ERR "%s: fucntionality check failed\n",
				__FUNCTION__);
		return err;
	}

	err = gpio_direction_input(dev->intr_gpio);
	if (err < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return err;
	}

	err = request_irq(dev->num_irq, mcs6000_ts_irq_handler,
			IRQF_TRIGGER_FALLING, "mcs6000_ts", dev);
	if (err < 0) {
		printk(KERN_ERR "%s: request_irq failed\n", __FUNCTION__);
		return err;
	}

	disable_irq(dev->num_irq);
	mcs6000_ts_on();
	enable_irq(dev->num_irq);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts_early_suspend.suspend = mcs6000_early_suspend;
	ts_early_suspend.resume = mcs6000_late_resume;
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&ts_early_suspend);
#endif
	mcs6000_firmware_info();
	DMSG(KERN_INFO "%s: ts driver probed\n", __FUNCTION__);

	return 0;
}

static int mcs6000_ts_remove(struct i2c_client *client)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	free_irq(dev->num_irq, dev);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
static int mcs6000_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
	disable_irq(dev->num_irq);
	DMSG("%s: irq disable\n", __FUNCTION__);
	dev->power(OFF);

	return 0;
}

static int mcs6000_ts_resume(struct i2c_client *client)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
	dev->power(ON);
	enable_irq(dev->num_irq);
	DMSG("%s: irq enable\n", __FUNCTION__);

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs6000_early_suspend(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
	disable_irq(dev->num_irq);
	DMSG("%s: irq disable\n", __FUNCTION__);
	dev->power(OFF);
}

static void mcs6000_late_resume(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
	mcs6000_ts_on();
	enable_irq(dev->num_irq);
	DMSG("%s: irq enable\n", __FUNCTION__);
}
#endif

static const struct i2c_device_id mcs6000_ts_id[] = {
	{ "touch_mcs6000", 1 },	
	{ }
};


static struct i2c_driver mcs6000_i2c_ts_driver = {
	.probe = mcs6000_ts_probe,
	.remove = mcs6000_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mcs6000_ts_suspend,
	.resume  = mcs6000_ts_resume,
#endif
	.id_table = mcs6000_ts_id,
	.driver = {
		.name = "touch_mcs6000",
		.owner = THIS_MODULE,
	},
};

static int __devinit mcs6000_ts_init(void)
{
	int err = 0;

	memset(&mcs6000_ts_dev, 0, sizeof(struct mcs6000_ts_device));

	mcs6000_ts_input = input_allocate_device();
	if (mcs6000_ts_input == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		err = -ENOMEM;
		goto err_input_allocate;
	}

	mcs6000_ts_input->name = "touch_mcs6000";

	set_bit(EV_SYN, 	 mcs6000_ts_input->evbit);
	set_bit(EV_KEY, 	 mcs6000_ts_input->evbit);
	set_bit(EV_ABS, 	 mcs6000_ts_input->evbit);
#ifdef LG_FW_MULTI_TOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, mcs6000_ts_input->absbit);
#else
	set_bit(BTN_TOUCH, mcs6000_ts_input->keybit);
#endif
#if defined(LG_FW_TOUCH_SOFT_KEY) || defined(LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY)
	set_bit(TOUCH_BACK, mcs6000_ts_input->keybit);
	set_bit(TOUCH_SEARCH, mcs6000_ts_input->keybit);
#else
	set_bit(KEY_BACK, mcs6000_ts_input->keybit);
	set_bit(KEY_SEARCH, mcs6000_ts_input->keybit);
#endif

	err = input_register_device(mcs6000_ts_input);
	if (err < 0) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto err_input_register;
	}

	err = i2c_add_driver(&mcs6000_i2c_ts_driver);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to probe i2c \n", __FUNCTION__);
		goto err_i2c_add_driver;
	}

	err = misc_register(&mcs6000_ts_misc_dev);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to misc register\n", __FUNCTION__);
		goto err_misc_register;
	}

	return err;

err_misc_register:
	misc_deregister(&mcs6000_ts_misc_dev);
err_i2c_add_driver:
	i2c_del_driver(&mcs6000_i2c_ts_driver);
err_input_register:
	input_unregister_device(mcs6000_ts_input);
err_input_allocate:
	input_free_device(mcs6000_ts_input);
	mcs6000_ts_input = NULL;
	return err;
}

static void __exit mcs6000_ts_exit(void)
{
	i2c_del_driver(&mcs6000_i2c_ts_driver);
	input_unregister_device(mcs6000_ts_input);
	input_free_device(mcs6000_ts_input);

	printk(KERN_INFO "touchscreen driver was unloaded!\nHave a nice day!\n");
}

module_init(mcs6000_ts_init);
module_exit(mcs6000_ts_exit);

MODULE_DESCRIPTION("MELFAS MCS6000 Touchscreen Driver");
MODULE_LICENSE("GPL");

