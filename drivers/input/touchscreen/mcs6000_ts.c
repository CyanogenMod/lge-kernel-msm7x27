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

#include <linux/timer.h>
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
#include "touch_mcs6000_ioctl.h"
#include <linux/i2c-gpio.h>
#include <mach/board_lge.h>






#include <mach/vreg.h>
struct vreg {
	const char *name;
	unsigned id;
	int status;
	unsigned refcnt;
};

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


int fw_rev = 0;

static bool running_debug = 0;

 
/* shoud be checked, what is the difference, TOUCH_SEARCH and KEY_SERACH, TOUCH_BACK  and KEY_BACK */
//#define LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY

#define TS_POLLING_TIME 10 /* msec */

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

#define MCS6000_TS_MAX_HW_VERSION				0x40
#define MCS6000_TS_MAX_FW_VERSION				0x20
#define MCS6000_TS_INTR_CONTROL			 		0x1D
#define MCS6000_TS_CHIP_RESET			 		0x1E

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
	struct workqueue_struct *ts_wq;
};

static struct input_dev *mcs6000_ts_input = NULL;
static struct mcs6000_ts_device mcs6000_ts_dev; 
static int is_downloading = 0;
static int is_touch_suspend = 0;

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

/* LGE_CHANGE_S [kyuhyung.lee@lge.com] 2010.02.23 : to support touch event from UTS*/
/* [FIXME temporary code] copy form VS740 by younchan.kim 2010-06-11 */
void Send_Touch( unsigned int x, unsigned int y)
{
#ifdef LG_FW_MULTI_TOUCH
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ts_dev.input_dev);
	input_sync(mcs6000_ts_dev.input_dev);
	
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ts_dev.input_dev);
	input_sync(mcs6000_ts_dev.input_dev);

#else
	//mcs6000_ts_event_touch( x, y , &mcs6000_ts_dev) ;
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_Y, y);
	input_report_key(mcs6000_ts_dev.input_dev, BTN_TOUCH, 0);
	input_sync(mcs6000_ts_dev.input_dev);
#endif
}
EXPORT_SYMBOL(Send_Touch);
/* LGE_CHANGE_E [kyuhyung.lee@lge.com] 2010.02.23 */
/* copy form VS740 by younchan.kim 2010-06-11 */

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
	if( running_debug )
	{
		if( x2 < 0 && y2<0)
			printk(KERN_INFO "[%d] x1=%d y1=%d\n", value, x1, y1);
		else
			printk(KERN_INFO "[%d] x1=%d y1=%d x2=%d y2=%d", value, x1, y1, x2, y2);
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
		report = 1;
	}

		if ((x >= 0) && (y >= 0) && running_debug )
		{			 
			printk(KERN_INFO "[%d] x=%d y=%d\n", value, x , y );
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
		}

		if(input_type) {
			touch_pressed = 1;

			/* exceptional routine for the touch case moving from key area to touch area of touch screen */
			if(key_pressed) {
				mcs6000_key_event_touch(key_pressed, RELEASED, dev);
				key_pressed = 0;
			}
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
	} 
	else { /* touch released case */
		if(key_pressed) {
			mcs6000_key_event_touch(key_pressed, RELEASED, dev);
			key_pressed = 0;
		}

		if(touch_pressed) {
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

touch_retry:
	if (dev->pendown) {
		//ret = schedule_delayed_work(&dev->work, msecs_to_jiffies(TS_POLLING_TIME));
		queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME));
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
		//schedule_delayed_work(&dev->work, 0);
		queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME));
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

void mcs6000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver)
{
	unsigned char data;
	struct mcs6000_ts_device *dev = NULL;
	int try_cnt = 0;
	dev = &mcs6000_ts_dev;

#if 0
	msleep(200);
	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_FW_VERSION);
	printk(KERN_INFO "MCS6000 F/W Version [0x%x]\n", data);
	dev->input_dev->id.version = data;
	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_HW_REVISION);
	printk(KERN_INFO "MCS6000 H/W Revision [0x%x]\n", data);
	dev->input_dev->id.product= data ;
#else
	/* for avoiding the read fail form mcs6000 IC*/
	do {
		data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_FW_VERSION);
		msleep(10);
		try_cnt ++;
	} while (data > MCS6000_TS_MAX_FW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS6000 F/W Version [0x%x]\n", data);
	*fw_ver = data;

	try_cnt = 0;
	do {
		data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_HW_REVISION);
		msleep(10);
		try_cnt ++;
	} while (data > MCS6000_TS_MAX_HW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS6000 H/W Revision [0x%x]\n", data);
	*hw_ver = data;
#endif
}
void mcs6000_ts_intr_enable(unsigned char enable)
{
	unsigned char data;
	unsigned char pCommand[2];
	struct mcs6000_ts_device *dev = NULL;
	dev = &mcs6000_ts_dev;

	pCommand[0] = MCS6000_TS_INTR_CONTROL;

	if( enable )
		pCommand[1] = 1;
	else
		pCommand[1] = 0;

	
	if(i2c_master_send(dev->client, pCommand, 2) < 0) {
		printk(KERN_ERR "%s touch ic write error\n", __FUNCTION__);
	}

	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_INTR_CONTROL);
	printk(KERN_INFO "MCS6000 Interrupt Control [0x%x]\n", data);

}

void mcs6000_ts_reset(void)
{

	unsigned char pCommand[2];
	struct mcs6000_ts_device *dev = NULL;
	dev = &mcs6000_ts_dev;

	pCommand[0] = MCS6000_TS_CHIP_RESET;
	pCommand[1] = 1;

	mcs6000_ts_intr_enable(0);
	if(i2c_master_send(dev->client, pCommand, 2) < 0) {
		printk(KERN_ERR "%s touch ic write error\n", __FUNCTION__);
	}
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

#if defined(CONFIG_MACH_MSM7X27_MUSCAT) || defined (CONFIG_MACH_MSM7X27_JUMP)
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
			gpio_direction_output(dev->intr_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_LOW:
			gpio_direction_output(dev->intr_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_OUT:
			gpio_direction_output(dev->intr_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_IN:
			gpio_direction_input(dev->intr_gpio);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_HIGH:
			gpio_direction_output(dev->scl_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_LOW:
			gpio_direction_output(dev->scl_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_HIGH:
			gpio_direction_output(dev->sda_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_LOW:
			gpio_direction_output(dev->sda_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_OUT:
			gpio_direction_output(dev->scl_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_OUT:
			gpio_direction_output(dev->sda_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_ENABLE:
			//mcs6000_ts_down_i2c_block_enable(1);
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_DISABLE:
			//mcs6000_ts_down_i2c_block_enable(0);
			break;
#else

	switch (cmd) {
		case MCS6000_TS_DOWN_IOCTL_VDD_HIGH:
			err = dev->power(ON);
			if( err < 0 )
				printk(KERN_INFO"%s: Power On Fail....\n", __FUNCTION__);
			break;

		case MCS6000_TS_DOWN_IOCTL_VDD_LOW:
			err = dev->power(OFF);
			if( err < 0 )
				printk(KERN_INFO"%s: Power Down Fail..\n",	__FUNCTION__);
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

#endif

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
		case MCS6000_TS_IOCTL_MAGIC :
			switch(cmd){
				case MCS6000_TS_IOCTL_FW_VER:
				{
					unsigned char fw_ver, hw_ver;
					mcs6000_firmware_info(&fw_ver, &hw_ver);
					err = fw_ver;
					fw_rev = fw_ver;					
					break;
				}
				case MCS6000_TS_IOCTL_MAIN_ON:
				case MCS6000_TS_IOCTL_MAIN_OFF:
					break;
			}
			break;
		default:
			printk(KERN_ERR "%s unknow ioctl\n", __FUNCTION__);
			err = -EINVAL;
			break;
	}
	return err;
}

static int mcs6000_ioctl_open(struct inode *inode, struct file *flip) {
	if(is_touch_suspend == 0) {
		disable_irq(mcs6000_ts_dev.num_irq);
		printk(KERN_INFO "touch download start : irq disabled by ioctl\n");
	}
	is_downloading = 1;
	return 0;
}

static int mcs6000_ioctl_release(struct inode *inode, struct file *flip) {
	if(is_touch_suspend == 1) {
		mcs6000_ts_dev.power(OFF);
		printk(KERN_INFO "touch download done : power off by ioctl\n");
	} else {
		enable_irq(mcs6000_ts_dev.num_irq);
		printk(KERN_INFO "touch download done : irq enabled by ioctl\n");
	}
	is_downloading = 0;
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

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	unsigned char hw_ver, fw_ver;

	if (is_downloading == 1) {
		r = sprintf(buf,"Now, MCS6000 Firmware Update is going, check it later\n ");
		return r;
	}

	mcs6000_firmware_info(&fw_ver, &hw_ver);
	r = sprintf(buf,"MCS6000 Touch Version HW:%02d FW:%02d ATCMD-FW:%02d\n",hw_ver, fw_ver, fw_rev);

	return r;
}

static ssize_t read_touch_dl_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;

	r = sprintf(buf,"MCS6000 Download Status %d\n",is_downloading);
	return r;
}

static ssize_t read_touch_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	int int_status;
	struct mcs6000_ts_device *dev_tmp ;
	struct vreg *vreg_touch;

	dev_tmp = &mcs6000_ts_dev;
	vreg_touch = vreg_get(0, "synt");
	//printk ("Vreg_touch info : name [%s], id [%d],status[%d], refcnt[%d]\n",vreg_touch->name,vreg_touch->id,vreg_touch->status,vreg_touch->refcnt);
	int_status = gpio_get_value(dev_tmp->intr_gpio);
	r = sprintf(buf,"MCS6000 interrupt Pin [%d] , power Status [%d]\n",int_status,vreg_touch->refcnt);
	return r;
}

static ssize_t write_touch_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd,err;
	struct mcs6000_ts_device *dev_tmp;

	dev_tmp = &mcs6000_ts_dev;
	sscanf(buf, "%d", &cmd);

	switch (cmd){
		case 1:	/* interrupt pin high */
			err = gpio_direction_input(dev_tmp->intr_gpio);
			if (err < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				return err;
			}
			gpio_set_value(dev_tmp->intr_gpio, 1 );
			break;
		case 2:	/* interrupt pin LOW */
			err = gpio_direction_input(dev_tmp->intr_gpio);
			if (err < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				return err;
			}
			gpio_set_value(dev_tmp->intr_gpio, 0 );
			break;
		case 3:	/* touch power on */
			dev_tmp->power(ON);
			break;
		case 4:	/*touch power off */
			dev_tmp->power(OFF);
			break;
		case 5: 
			mcs6000_ts_intr_enable(0);
			break;
		case 6: 
			mcs6000_ts_intr_enable(1);
			break;		
		case 7: 
			mcs6000_ts_reset();
			break;
		default :
			printk(KERN_INFO "usage: echo [1|2|3|4|5|6|7] > control\n");
			printk(KERN_INFO "  - 1: interrupt pin high\n");
			printk(KERN_INFO "  - 2: interrupt pin low\n");
			printk(KERN_INFO "  - 3: power on\n");
			printk(KERN_INFO "  - 4: power off\n");
			printk(KERN_INFO "  - 5: interrupt disable\n");
			printk(KERN_INFO "  - 6: interrupt enable\n");
			printk(KERN_INFO "  - 7: reset\n");
			break;
	}
	return size;
}

static ssize_t write_touch_log(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd;
	//struct mcs6000_ts_device *dev_tmp;

	//dev_tmp = &mcs6000_ts_dev;
	sscanf(buf, "%d", &cmd);

	switch (cmd){
		case 0:	/* print off */
			running_debug= 0;
			printk(KERN_INFO "touch log end...\n");
			break;
		case 1:	/* print coordinate from touch controller for debugging */
			running_debug = 1;
			printk(KERN_INFO "touch log start...\n");
			break;
		default :
			break;
	}
	return size;
}
static DEVICE_ATTR(touch_control, S_IRUGO|S_IWUSR,NULL,write_touch_control);
static DEVICE_ATTR(touch_status, S_IRUGO,read_touch_status, NULL);
static DEVICE_ATTR(version, S_IRUGO /*| S_IWUSR*/,read_touch_version, NULL);
static DEVICE_ATTR(dl_status, S_IRUGO,read_touch_dl_status, NULL);
static DEVICE_ATTR(touch_log, S_IRUGO|S_IWUSR,NULL,write_touch_log);

int mcs6000_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_dl_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_dl_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_dl_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_control);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_control create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_control);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_log);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_log create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_log);
		return ret;
	}
	return ret;
}

int mcs6000_remove_file(struct input_dev *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_version);
	device_remove_file(&pdev->dev, &dev_attr_dl_status);
	device_remove_file(&pdev->dev, &dev_attr_touch_status);
	device_remove_file(&pdev->dev, &dev_attr_touch_control);
	device_remove_file(&pdev->dev, &dev_attr_touch_log);
	return 0;
}
static int mcs6000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct touch_platform_data *ts_pdata;
	struct mcs6000_ts_device *dev;
	//unsigned char fw_ver, hw_ver;

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
	err = gpio_request(dev->intr_gpio, "MCS6000-IRQ");
	if(err < 0) {
		printk(KERN_ERR "%s: gpio_request fail\n", __FUNCTION__);
		return err;
	}
	err = gpio_direction_input(dev->intr_gpio);
	if (err < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return err;
	}
	/* TODO: You have try to change this driver's architecture using request_threaded_irq()
	 * So, I will change this to request_threaded_irq()
	 */
	err = request_threaded_irq(dev->num_irq, NULL, mcs6000_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mcs6000_ts", dev);

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
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1 ;
	register_early_suspend(&ts_early_suspend);
#endif	
	//mcs6000_firmware_info(&fw_ver, &hw_ver);	
	mcs6000_create_file(mcs6000_ts_input);  
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

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		dev->power(OFF);
	}
	is_touch_suspend = 1;

	return 0;
}

static int mcs6000_ts_resume(struct i2c_client *client)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		dev->power(ON);
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs6000_early_suspend(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		dev->power(OFF);		
	}
	is_touch_suspend = 1;
}

static void mcs6000_late_resume(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		mcs6000_ts_on();

		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;
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
	struct mcs6000_ts_device *dev;
	dev = &mcs6000_ts_dev;

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

	dev->ts_wq = create_singlethread_workqueue("ts_wq");
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
	mcs6000_remove_file(mcs6000_ts_input);
	i2c_del_driver(&mcs6000_i2c_ts_driver);
	input_unregister_device(mcs6000_ts_input);
	input_free_device(mcs6000_ts_input);

	printk(KERN_INFO "touchscreen driver was unloaded!\nHave a nice day!\n");
}

module_init(mcs6000_ts_init);
module_exit(mcs6000_ts_exit);

MODULE_DESCRIPTION("MELFAS MCS6000 Touchscreen Driver");
MODULE_LICENSE("GPL");

