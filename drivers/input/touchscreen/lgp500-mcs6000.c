/*
 * MELFAS mcs6000 touchscreen driver
 *
 * Copyright (C) 2011 LGE, Inc.
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
#include <linux/slab.h>

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/wakelock.h>
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

static void mcs6000_early_suspend(struct early_suspend *h);
static void mcs6000_late_resume(struct early_suspend *h);
#endif

#define TS_SAMPLERATE_HZ 50 /* touchscreen samplerate [Hz]*/
#define	TS_SENSE_CH_CNT	21

#define MCS6000_I2C_TS_NAME		"touch_mcs6000"
#define ON 				1
#define OFF 				0
#define PRESSED 			1
#define RELEASED			0

unsigned char read_buf[8];

int fw_rev = 0;

/* usage: echo [mask_value] > /sys/module/mcs6000_ts_gb/parameters/debug_mask
 * 1 is no debug message print(default)
 * 2 is debug message print
 * 4 is function trace message print
 */
enum {
	MCS6000_DM_TRACE_NO   = 1U << 0,
	MCS6000_DM_TRACE_YES  = 1U << 1,
	MCS6000_DM_TRACE_FUNC = 1U << 2,
	MCS6000_DM_TRACE_REAL = 1U << 3, 
};

static unsigned int mcs6000_debug_mask = MCS6000_DM_TRACE_NO;

module_param_named(debug_mask, mcs6000_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#define DMSG(fmt, args...) \
	printk(KERN_DEBUG "[%-18s:%5d]" \
			fmt, __FUNCTION__, __LINE__, ##args);

#define MCS6000_TS_INPUT_INFO					0x10
#define MCS6000_TS_XY_HIGH					0x11
#define MCS6000_TS_X_LOW					0x12
#define MCS6000_TS_Y_LOW					0x13
#define MCS6000_TS_Z			 			0x14
#define MCS6000_TS_XY2_HIGH			 		0x15
#define MCS6000_TS_X2_LOW				 	0x16
#define MCS6000_TS_Y2_LOW					0x17
#define MCS6000_TS_Z2		 				0x18
#define MCS6000_TS_KEY_STRENGTH  				0x19
#define MCS6000_TS_FW_VERSION			 		0x20
#define MCS6000_TS_HW_REVISION					0x21

#define MCS6000_TS_MAX_HW_VERSION				0x40
#define MCS6000_TS_MAX_FW_VERSION				0x20

static struct workqueue_struct *mcs6000_wq;

struct mcs6000_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	//struct work_struct work;
	struct delayed_work work;
	struct wake_lock wakelock;
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	int irq_sync;
	int fw_version;
	int hw_version;
	int status;
	int tsp_type;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static struct mcs6000_ts_data *mcs6000_ext_ts = (void *)NULL; 
static int misc_opened = 0;

#define READ_NUM 8 /* now, just using two finger data */

enum {
	NON_TOUCHED_STATE,
	SINGLE_POINT_TOUCH,
	MULTI_POINT_TOUCH,
	MAX_TOUCH_TYPE
};

enum {
	NO_KEY_TOUCHED,
	KEY1_TOUCHED,
	KEY2_TOUCHED,
	KEY3_TOUCHED,
	MAX_KEY_TOUCH
};

enum {
	MCS6000_DEV_NORMAL,
	MCS6000_DEV_SUSPEND,
	MCS6000_DEV_DOWNLOAD
};

/* LGE_CHANGE_S [kyuhyung.lee@lge.com] 2010.02.23 : to support touch event from UTS*/
/* [FIXME temporary code] copy form VS740 by younchan.kim 2010-06-11 */
void Send_Touch( unsigned int x, unsigned int y)
{
	if (mcs6000_ext_ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000 not proved\n");
		return;
	}

	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ext_ts->input_dev);
	input_sync(mcs6000_ext_ts->input_dev);

	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ext_ts->input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ext_ts->input_dev);
	input_sync(mcs6000_ext_ts->input_dev);
}
EXPORT_SYMBOL(Send_Touch);
/* LGE_CHANGE_E [kyuhyung.lee@lge.com] 2010.02.23 */
/* copy form VS740 by younchan.kim 2010-06-11 */


static __inline void mcs6000_multi_ts_event_touch(int x1, int y1, int x2, int y2, int value,
		struct mcs6000_ts_data *ts)
{
	int report = 0;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if ((x1 >= 0) && (y1 >= 0)) {
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x1);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y1);
		input_mt_sync(ts->input_dev);
		report = 1;
	}

	if ((x2 >= 0) && (y2 >= 0)) {
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x2);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y2);
		input_mt_sync(ts->input_dev);
		report = 1;
	}

	if (report != 0)
		input_sync(ts->input_dev);
	else {
		if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
			DMSG("Not available touch data x1=%d, y1=%d, x2=%d, y2=%d\n", x1, y1, x2, y2);
	}
	return;
}

#define to_delayed_work(_work)  container_of(_work, struct delayed_work, work)


static void mcs6000_ts_work_func(struct work_struct *work)
{
	  unsigned int input_type;
	  unsigned char read_buf[READ_NUM];

	  static int touch_pressed = 0;

	  //What is done here?
	  struct mcs6000_ts_data *ts = container_of(to_delayed_work(work), struct mcs6000_ts_data, work);
	  //gpio interrupt line is held low if finger
	  //is on screen
	  ts->pendown = !gpio_get_value(ts->intr_gpio);

	  /* read the registers of MCS6000 IC */
	  if ( i2c_smbus_read_i2c_block_data(ts->client, MCS6000_TS_INPUT_INFO, READ_NUM, read_buf) < 0)
	    {
	      printk(KERN_ERR "%s touch ic read error\n", __FUNCTION__);
	      //retry at next sample
	      queue_delayed_work(mcs6000_wq, &ts->work, (HZ / TS_SAMPLERATE_HZ));
	    }


	  input_type = read_buf[0] & 0x0f;

	  if (ts->pendown)   /* touch pressed case */
	    {
	      if(input_type)
	        {
	          touch_pressed = 1;

	          if(input_type == MULTI_POINT_TOUCH || input_type == SINGLE_POINT_TOUCH)
	            {
	              //input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, read_buf[4]);
	              input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
	              input_report_abs(ts->input_dev, ABS_MT_POSITION_X, (((read_buf[1] & 0xf0) << 4) | read_buf[2]));
	              input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, (((read_buf[1] & 0x0f) << 8) | read_buf[3]));
	              input_mt_sync(ts->input_dev);

	              if(input_type == MULTI_POINT_TOUCH)
	                {
	                  input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 1);
	                  input_report_abs(ts->input_dev, ABS_MT_POSITION_X, (((read_buf[5] & 0xf0) << 4) | read_buf[6]));
	                  input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, (((read_buf[5] & 0x0f) << 8) | read_buf[7]));
	                  input_mt_sync(ts->input_dev);
	                }
	              input_sync(ts->input_dev);
	            }
	        }
	      queue_delayed_work(mcs6000_wq, &ts->work, (HZ / TS_SAMPLERATE_HZ));
	    }
	  else   /* touch released case */
	    {
	      if(touch_pressed)
	        {
	          input_mt_sync(ts->input_dev);
	          input_sync(ts->input_dev);
	          touch_pressed = 0;
	        }
	      enable_irq(ts->num_irq);
	      ts->irq_sync++;
	      DMSG("%s: irq enable\n", __FUNCTION__);
	    }
}

static irqreturn_t mcs6000_ts_irq_handler(int irq, void *dev_id)
{
	struct mcs6000_ts_data *ts = dev_id;

	if (gpio_get_value(ts->intr_gpio) == 0) {
		disable_irq_nosync(ts->client->irq);
		ts->irq_sync--;
		//queue_work(mcs6000_wq, &ts->work);
		queue_delayed_work(mcs6000_wq, &ts->work, (HZ / TS_SAMPLERATE_HZ));
	}
	else  {
		printk(KERN_INFO "mcs6000_ts_irq_handler: check int gpio level\n");
	}
	return IRQ_HANDLED;
}

static void mcs6000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver)
{
	//int ret = 0;
	char ret = 0;	
	struct vreg *vreg_touch = vreg_get(0, "synt");

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (mcs6000_ext_ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000 ts data is null\n");
		return;
	}

	if(!vreg_touch->refcnt){
		mcs6000_ext_ts->power(ON);
		msleep(100);
	}

	*fw_ver = 0xfa;
	*hw_ver = 0xfa;
	mcs6000_ext_ts->tsp_type = 0x00;

	{
		int retry = 10;
		while ((retry-- > 0)) {
			//ret = i2c_smbus_read_byte_data(mcs6000_ext_ts->client, MCS6000_TS_FW_VERSION);
			read_buf[0]= MCS6000_TS_FW_VERSION;
			i2c_master_send(mcs6000_ext_ts->client,read_buf ,1);			
			i2c_master_recv(mcs6000_ext_ts->client,&ret ,1);
			
			if (ret >= 0)
				break;
			msleep(100);			
		}
	}
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_firmware_info: fw version read failed\n");
		return;
	}

	//if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
	printk(KERN_INFO "MCS-6000 firmware version(0x%x)\n", ret);

	mcs6000_ext_ts->fw_version = *fw_ver = fw_rev = (unsigned char)ret;

	{
		int retry = 10;
		while ((retry-- > 0)) {
			//ret = i2c_smbus_read_byte_data(mcs6000_ext_ts->client, MCS6000_TS_HW_REVISION);
			read_buf[0]= MCS6000_TS_HW_REVISION;
			i2c_master_send(mcs6000_ext_ts->client,read_buf ,1);
			i2c_master_recv(mcs6000_ext_ts->client,&ret ,1);
			if (ret >= 0)
				break;
			msleep(100);			
		}
	}
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_H/W_info: i2c read failed\n");
		return;
	}

	//if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
		DMSG("MCS-6000 hardware version(0x%x)\n", ret);
	
	mcs6000_ext_ts->hw_version = *hw_ver = (unsigned char)ret;

#if defined(CONFIG_MACH_MSM7X27_MUSCAT)
	{
		int retry = 10;
		while ((retry-- > 0)) {			
			read_buf[0]= 0x28;
			i2c_master_send(mcs6000_ext_ts->client,read_buf ,1);
			i2c_master_recv(mcs6000_ext_ts->client,&ret ,1);
			if (ret >= 0)
				break;
			msleep(100);			
		}
	}
			
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_panel_info : i2c read failed\n");		
		return;
	}
	mcs6000_ext_ts->tsp_type = ret;

	DMSG("MCS-6000 panel pattern type (0x%x)\n", ret);

	{
		int retry = 10;
		while ((retry-- > 0)) {			
			read_buf[0]= 0x22;
			i2c_master_send(mcs6000_ext_ts->client,read_buf ,1);
			i2c_master_recv(mcs6000_ext_ts->client,&ret ,1);
			if (ret >= 0)
				break;
			msleep(100);			
		}
	}
			
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_panel_info : i2c read failed\n");		
		return;
	}


	switch(ret)
	{
		case 0x01:	// tsp vendor : yount fast
			if( mcs6000_ext_ts->tsp_type == 1 )
			{
				mcs6000_ext_ts->tsp_type = 1; //yount fast : diamond
			}
			break;
			
		case 0x02:	// tsp vendor : lg-inotek
			if( mcs6000_ext_ts->tsp_type == 1 )
			{
				mcs6000_ext_ts->tsp_type = 2;	//lg-inotek : diamond
			}
			break;

		default:
			mcs6000_ext_ts->tsp_type = 0;	//young fast : triangle

	}
	
	DMSG("MCS-6000 panel vendor (0x%x)\n", mcs6000_ext_ts->tsp_type);
#endif
}

/* now,Does not support mcs6000 F/W downloads */
static __inline int mcs6000_ioctl_down_i2c_write(struct file *file, unsigned char addr,unsigned char val)
{
	struct mcs6000_ts_data *ts = file->private_data;
	int err = 0;
	struct i2c_msg msg;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000 ts data is null\n");
		return -1;
	}

	if (ts->client == NULL) {
		printk(KERN_ERR "mcs6000_ts_ioctl_down_i2c_write: client is null\n");
		return -1;
	}
	msg.addr = addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &val;

	if ((err = i2c_transfer(ts->client->adapter, &msg, 1)) < 0) {
		printk(KERN_ERR "mcs6000_ts_ioctl_down_i2c_write: transfer failed[%d]\n", err);
	}

	return err;
}

static __inline int mcs6000_ioctl_down_i2c_read(struct file *file, unsigned char addr, unsigned char *ret)
{
	struct mcs6000_ts_data *ts = file->private_data;
	int err = 0;
	struct i2c_msg msg;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000_ts_ioctl_down_i2c_read: client is null\n");
		return -1;
	}

	if (ts->client == NULL) {
		printk(KERN_ERR "mcs6000_ts_ioctl_down_i2c_read: transfer failed[%d]\n", err);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 1;
	msg.len = 1;
	msg.buf = ret;

	if ((err = i2c_transfer(ts->client->adapter, &msg, 1)) < 0) {
		printk(KERN_ERR "mcs6000_ts_ioctl_down_i2c_read: transfer failed[%d]\n", err);
	}

	return err;
}

int mcs6000_ts_ioctl_down(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mcs6000_ts_data *ts = file->private_data;
	int err = 0;
	struct mcs6000_ts_down_ioctl_i2c_type client_data;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (_IOC_NR(cmd) >= MCS6000_TS_DOWN_IOCTL_MAXNR)
		return -EINVAL;

	switch (cmd) {
		case MCS6000_TS_DOWN_IOCTL_VDD_HIGH:
			err = ts->power(1);
			if (err < 0)
				printk(KERN_INFO "mcs6000_ts_ioctl_down: Power up failed\n");
			break;
		case MCS6000_TS_DOWN_IOCTL_VDD_LOW:
			err = ts->power(0);
			if (err < 0)
				printk(KERN_INFO "mcs6000_ts_ioctl_down: Power down failed\n");
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_HIGH:
			gpio_direction_output(ts->intr_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_LOW:
			gpio_direction_output(ts->intr_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_OUT:
			gpio_direction_output(ts->intr_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_IN:
			gpio_direction_input(ts->intr_gpio);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_HIGH:
			gpio_direction_output(ts->scl_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_LOW:
			gpio_direction_output(ts->scl_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_HIGH:
			gpio_direction_output(ts->sda_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_LOW:
			gpio_direction_output(ts->sda_gpio, 0);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_OUT:
			gpio_direction_output(ts->scl_gpio, 1);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_OUT:
			gpio_direction_output(ts->sda_gpio, 1);
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
				printk(KERN_INFO "mcs6000_ts_ioctl_down: copyfromuser-read error\n");
				return -EFAULT;
			}

			if (0 > mcs6000_ioctl_down_i2c_read(file, (unsigned char)client_data.addr,
						(unsigned char *)&client_data.data)) {
				err = -EIO;
			}

			if (copy_to_user((void *)arg, (const void *)&client_data,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "mcs6000_ts_ioctl_down: copytouser-read error\n");
				err = -EFAULT;
			}
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_WRITE:
			if (copy_from_user(&client_data, (struct mcs6000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "mcs6000_ts_ioctl_down: copyfromuser-write error\n");
				return -EFAULT;
			}

			if (0 > mcs6000_ioctl_down_i2c_write(file, (unsigned char)client_data.addr,
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

static int mcs6000_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	int err = -1;
	unsigned char fw_ver = 0, hw_ver = 0;
	

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	switch (_IOC_TYPE(cmd)) {
		case MCS6000_TS_DOWN_IOCTL_MAGIC:
			err = mcs6000_ts_ioctl_down(inode, file, cmd, arg);
			break;
		case MCS6000_TS_IOCTL_MAGIC :
			switch(cmd) {
				case MCS6000_TS_IOCTL_FW_VER:
					{
						msleep(100);
						mcs6000_firmware_info(&fw_ver, &hw_ver);
						err = fw_rev = fw_ver;	
						err |= hw_ver<<8;
						err |= mcs6000_ext_ts->tsp_type<<16;
						printk(KERN_INFO "mcs6000 TSP TYPE: %x\n", mcs6000_ext_ts->tsp_type);
						printk(KERN_INFO "mcs6000 ioctl version info: %x\n", err);
						break;
					}
				case MCS6000_TS_IOCTL_MAIN_ON:
				case MCS6000_TS_IOCTL_MAIN_OFF:
					break;
			}
			break;
		default:
			printk(KERN_ERR "mcs6000_ts_ioctl: unknow ioctl\n");
			err = -EINVAL;
			break;
	}
	return err;
}
static int mcs6000_open(struct inode *inode, struct file *file) 
{
	struct mcs6000_ts_data *ts = mcs6000_ext_ts;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL)
		return -EIO;

	if (misc_opened)
		return -EBUSY;

	if (ts->status == MCS6000_DEV_NORMAL) {
		disable_irq(ts->num_irq);
		ts->irq_sync--;
		//if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
			DMSG("touch download start: irq disabled by ioctl\n");
	}

	misc_opened = 1;

	file->private_data = ts;

	wake_lock(&ts->wakelock);

	ts->status = MCS6000_DEV_DOWNLOAD;

	return 0;
}

static int mcs6000_release(struct inode *inode, struct file *file) 
{
	struct mcs6000_ts_data *ts = file->private_data;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL)
		return -EIO;	

	if (ts->status == MCS6000_DEV_SUSPEND) {
		ts->power(OFF);
		if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
			DMSG("touch download done: power off by ioctl\n");
	} 
	else {
		enable_irq(ts->num_irq);
		ts->irq_sync++;
		//if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
			DMSG("touch download done: irq enabled by ioctl\n");

		ts->status = MCS6000_DEV_NORMAL;
	}

	misc_opened = 0;

	wake_unlock(&ts->wakelock);

	return 0;
}

static struct file_operations mcs6000_ts_ioctl_fops = {
	.owner   = THIS_MODULE,
	.ioctl   = mcs6000_ioctl,
	.open    = mcs6000_open,
	.release = mcs6000_release,
};

static struct miscdevice mcs6000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcs6000-touch",
	.fops = &mcs6000_ts_ioctl_fops,
};

	static ssize_t
mcs6000_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	unsigned char hw_ver, fw_ver;
	

	if (ts->status == MCS6000_DEV_DOWNLOAD)
		return snprintf(buf, PAGE_SIZE, "Now, MCS-6000 FW update is going, check it later\n");
	mcs6000_firmware_info(&fw_ver, &hw_ver);	
	return snprintf(buf, PAGE_SIZE, "fw:0x%x, hw:0x%x, panel:0x%x\n", 
			ts->fw_version, ts->hw_version, ts->tsp_type);
}

	static ssize_t 
mcs6000_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	struct vreg *vreg_touch = vreg_get(0, "synt");
	int len;

	len = snprintf(buf, PAGE_SIZE, "\nMCS-6000 Device Status\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "=============================\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "irq num       is %d\n", ts->num_irq);
	len += snprintf(buf + len, PAGE_SIZE - len, "gpio_irq num  is %d(level=%d)\n", ts->intr_gpio, gpio_get_value(ts->intr_gpio));
	len += snprintf(buf + len, PAGE_SIZE - len, "gpio_scl num  is %d\n", ts->scl_gpio);
	len += snprintf(buf + len, PAGE_SIZE - len, "gpio_sda num  is %d\n", ts->sda_gpio);
	len += snprintf(buf + len, PAGE_SIZE - len, "irq_sync      is %d\n", ts->irq_sync);
	len += snprintf(buf + len, PAGE_SIZE - len, "power line    is %d\n", vreg_touch->refcnt);
	switch(ts->status) {
		case MCS6000_DEV_NORMAL:
			len += snprintf(buf + len, PAGE_SIZE - len, "device status is %s\n", "normal");
			break;
		case MCS6000_DEV_SUSPEND:
			len += snprintf(buf + len, PAGE_SIZE - len, "device status is %s\n", "suspend");
			break;
		case MCS6000_DEV_DOWNLOAD:
			len += snprintf(buf + len, PAGE_SIZE - len, "device status is %s\n", "downloading");
			break;
		default:
			len += snprintf(buf + len, PAGE_SIZE - len, "device status is %s\n", "unknown");
			break;
	}

	return len;
}

	static ssize_t
mcs6000_control_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	int cmd, ret;

	if (sscanf(buf, "%d", &cmd) != 1)
		return -EINVAL;

	switch (cmd){
		case 1:	/* interrupt pin high */
			ret = gpio_direction_input(ts->intr_gpio);
			if (ret < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				break;
			}
			gpio_set_value(ts->intr_gpio, 1);
			printk(KERN_INFO "MCS-6000 INTR GPIO pin high\n");
			break;
		case 2:	/* interrupt pin LOW */
			ret = gpio_direction_input(ts->intr_gpio);
			if (ret < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				break;
			}
			gpio_set_value(ts->intr_gpio, 0);
			printk(KERN_INFO "MCS-6000 INTR GPIO pin low\n");
			break;
		case 3:	/* touch power on */
			ts->power(ON);
			break;
		case 4:	/*touch power off */
			ts->power(OFF);
			break;
		case 5: /* enable irq */
			enable_irq(ts->num_irq);
			ts->irq_sync++;
			break;
		case 6: /* disble irq */
			disable_irq(ts->num_irq);
			ts->irq_sync--;
			break;
		default :
			printk(KERN_INFO "usage: echo [1|2|3|4|5|6] > control\n");
			printk(KERN_INFO "  - 1: interrupt pin high\n");
			printk(KERN_INFO "  - 2: interrupt pin low\n");
			printk(KERN_INFO "  - 3: power on\n");
			printk(KERN_INFO "  - 4: power off\n");
			printk(KERN_INFO "  - 5: interrupt enable\n");
			printk(KERN_INFO "  - 6: interrupt disable\n");
			break;
	}

	return count;
}

	static ssize_t
mcs6000_reset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	int cmd, ret;

	if (sscanf(buf, "%d", &cmd) != 1)
		return -EINVAL;

	if (cmd == 1) {
		printk(KERN_INFO "MCS-6000 IC reset\n");
		ret = i2c_smbus_write_byte_data(ts->client, 0x1e, 0x01);	/* device reset */
		if (ret < 0) {
			printk(KERN_ERR "i2c_smbus_write_byte_data failed\n");
		}
	}
	else {
		printk(KERN_INFO "usage: echo 1 > reset\n");
	}

	return count;
}

	static ssize_t
mcs6000_intr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	int ret;

	ret = i2c_smbus_read_byte_data(ts->client, 0x1d);
	if (ret < 0) {
		printk(KERN_ERR "i2c_smbus_read_byte_data failed\n");
	}

	return snprintf(buf, PAGE_SIZE, "MCS-6000 interrupt status is %d\n", ret);
}

	static ssize_t
mcs6000_intr_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	int cmd, ret;

	if (sscanf(buf, "%d", &cmd) != 1)
		return -EINVAL;

	if (cmd == 0) {
		printk(KERN_INFO "MCS-6000 IC interrupt disable\n");
		ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x00);
		if (ret < 0) {
			printk(KERN_ERR "i2c_smbus_write_byte_data failed\n");
		}
	}
	else if (cmd == 1) {
		printk(KERN_INFO "MCS-6000 IC interrupt enable\n");
		ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x01);
		if (ret < 0) {
			printk(KERN_ERR "i2c_smbus_write_byte_data failed\n");
		}
	}
	else {
		printk(KERN_INFO "usage: echo [0|1] > intr\n");
	}

	return count;
}


static ssize_t
mcs6000_sensitivity_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	
	int len=0;	
	int sense_cnt = 0;
	unsigned char read_buf[TS_SENSE_CH_CNT];	
	
	
	
	struct mcs6000_ts_data *ts = dev_get_drvdata(dev);
	
	if (ts->status == MCS6000_DEV_DOWNLOAD ) {
		len = sprintf(buf,"Now, MCS6000 Firmware Update is going, check it later\n ");
		return len;
	}


	if ( i2c_smbus_read_i2c_block_data(ts->client, 0x30, TS_SENSE_CH_CNT, read_buf) < 0) {
		printk(KERN_ERR "%s touch ic read error\n", __FUNCTION__);
		len = sprintf(buf,"MCS6000 read error\n ");
		return len;
		
	} 
		

	len = snprintf(buf, PAGE_SIZE, "\nMCS-6000 Channel Sensitivity[%x]\n", ts->tsp_type);

	for(sense_cnt=0; sense_cnt<TS_SENSE_CH_CNT; sense_cnt++)
	{
		len += snprintf(buf + len, PAGE_SIZE - len,"%d,",read_buf[sense_cnt]);

	}

	len += snprintf(buf + len, PAGE_SIZE - len, "\nInterrupt :%d\n", gpio_get_value(ts->intr_gpio));
	
	return len;	
}

static struct device_attribute mcs6000_device_attrs[] = {
	__ATTR(status,  S_IRUGO | S_IWUSR, mcs6000_status_show, NULL),
	__ATTR(version, S_IRUGO | S_IWUSR, mcs6000_version_show, NULL),
	__ATTR(control, S_IRUGO | S_IWUSR, NULL, mcs6000_control_store),
	__ATTR(reset,   S_IRUGO | S_IWUSR, NULL, mcs6000_reset_store),
	__ATTR(intr,    S_IRUGO | S_IWUSR, mcs6000_intr_show, mcs6000_intr_store),
	__ATTR(sensitivity,    S_IRUGO | S_IWUSR, mcs6000_sensitivity_show, NULL),
};



static int mcs6000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct mcs6000_ts_data *ts;
	int ret = 0;
	int i = 0;

	struct touch_platform_data *pdata;

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "mcs6000_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENODEV;
		goto err_alloc_data_failed;
	}	

	//INIT_WORK(&ts->work, mcs6000_ts_work_func);
	INIT_DELAYED_WORK(&ts->work, mcs6000_ts_work_func);
	ts->client = client;
	i2c_set_clientdata(client, ts);
	pdata = client->dev.platform_data;

	if (pdata)
		ts->power = pdata->power;
	if (ts->power) {
		ret = ts->power(ON);
		if (ret < 0) {
			printk(KERN_ERR "mcs6000_ts_probe: power on failed\n");
			goto err_power_failed;
		}
		msleep(120);
	}
#if 0
	ret = i2c_smbus_write_byte_data(ts->client, 0x1e, 0x01);	/* device reset */
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_ts_probe: i2c write failed\n");
	}
	{
		int retry = 10;
		while ((retry-- > 0)) {
			ret = i2c_smbus_read_byte_data(ts->client, MCS6000_TS_FW_VERSION);
			if (ret >= 0)
				break;
			msleep(10);
		}
	}
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_ts_probe: fw version read failed\n");
		goto err_detect_failed;
	}

	if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
		printk(KERN_INFO "MCS-6000 firmware version(0x%x)\n", ret);

	ts->fw_version = ret;

	{
		int retry = 10;
		while ((retry-- > 0)) {
			ret = i2c_smbus_read_byte_data(ts->client, MCS6000_TS_HW_REVISION);
			if (ret >= 0)
				break;
			msleep(10);
		}
	}
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_ts_probe: i2c read failed\n");
		goto err_detect_failed;
	}

	if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
		DMSG("MCS-6000 hardware version(0x%x)\n", ret);

	ts->hw_version = ret;
#endif
	if (pdata) {
		ts->num_irq = client->irq;
		ts->intr_gpio = (client->irq) - NR_MSM_IRQS ;
		ts->sda_gpio = pdata->sda;
		ts->scl_gpio = pdata->scl;
		ts->irq_sync = 0;
		ts->status = MCS6000_DEV_NORMAL;
	}

	/* disable int */
//	ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x00);
//	if (ret < 0) {
//		printk(KERN_ERR "mcs6000_ts_probe: i2c write failed\n");
//		goto err_detect_failed;
//	}

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk(KERN_ERR "mcs6000_ts_probe: input device alloc failed\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = MCS6000_I2C_TS_NAME;

	/* touch key function disable by younchan.kim,2010-09-24 */
	set_bit(EV_SYN, ts->input_dev->evbit);
	//set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(ABS_MT_TOUCH_MAJOR, ts->input_dev->absbit);
	/* copy form LS670 touch by younchan.kim
	   LGE_CHANGE [dojip.kim@lge.com] 2010-09-23, android multi touch
	 */
	set_bit(ABS_MT_POSITION_X, ts->input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, ts->input_dev->absbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, pdata->ts_x_min, pdata->ts_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, pdata->ts_y_min, pdata->ts_y_max, 0, 0);

	ret = input_register_device(ts->input_dev);
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	if (MCS6000_DM_TRACE_YES & mcs6000_debug_mask)
		DMSG("MCS-6000 IRQ#=%d, IRQ_GPIO#=%d\n", ts->num_irq, ts->intr_gpio);

	if (client->irq) {

		ret = gpio_request(ts->intr_gpio, "MCS6000-IRQ");
		if(ret < 0) {
			printk(KERN_ERR "%s: gpio_request fail\n", __FUNCTION__);
			return ret;
		}
		
		ret = gpio_direction_input(ts->intr_gpio);
		if (ret < 0) {
			printk(KERN_ERR "mcs6000_probe_ts: gpio_direction_input failed\n");
			goto err_gpio_direction_input_failed;
		}

		ret = request_irq(client->irq, mcs6000_ts_irq_handler, IRQF_TRIGGER_FALLING, "mcs6000_ts", ts);
		if (ret == 0) {
		//	ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x01); /* enable int */
			if (ret)
				free_irq(client->irq, ts);
		}
		if (ret != 0) {
			dev_err(&client->dev, "request_irq fialed\n");
			goto err_request_irq_failed;
		}
	}

	ret = misc_register(&mcs6000_ts_misc_dev);
	if (ret < 0) {
		printk(KERN_ERR "mcs6000_probe_ts: misc register failed\n");
		goto err_misc_register_failed;
	}


	for (i = 0; i < ARRAY_SIZE(mcs6000_device_attrs); i++) {
		ret = device_create_file(&client->dev, &mcs6000_device_attrs[i]);
		if (ret) {
			goto err_device_create_file_failed;
		}
	}

	


#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.suspend	= mcs6000_early_suspend;
	ts->early_suspend.resume	= mcs6000_late_resume;
	ts->early_suspend.level		= EARLY_SUSPEND_LEVEL_DISABLE_FB - 5;
	register_early_suspend(&ts->early_suspend);
#endif

	mcs6000_ext_ts = ts;


	wake_lock_init(&ts->wakelock, WAKE_LOCK_SUSPEND, "mcs6000");

	printk(KERN_INFO "MCS-6000 Device probe complete\n");

	return 0;


err_device_create_file_failed:
	misc_deregister(&mcs6000_ts_misc_dev);


err_misc_register_failed:
err_gpio_direction_input_failed:
err_request_irq_failed:
err_input_register_device_failed:
	input_free_device(ts->input_dev);

err_input_dev_alloc_failed:
//err_detect_failed:
err_power_failed:
	kfree(ts);
err_alloc_data_failed:
err_check_functionality_failed:
	return ret;
}

static int mcs6000_ts_remove(struct i2c_client *client)
{
	struct mcs6000_ts_data *ts = i2c_get_clientdata(client);
	int i = 0;


	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	free_irq(client->irq, ts);
	input_unregister_device(ts->input_dev);
	kfree(ts);


	for (i = 0; i < ARRAY_SIZE(mcs6000_device_attrs); i++)
		device_remove_file(&client->dev, &mcs6000_device_attrs[i]);


	return 0;
}

static int mcs6000_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret = 0;
	struct mcs6000_ts_data *ts = i2c_get_clientdata(client);

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");
	
	if(ts->irq_sync == 0){
		disable_irq(client->irq);
		ts->irq_sync--;
	}

	//ret = cancel_work_sync(&ts->work);
	cancel_delayed_work_sync(&ts->work);
	
//	ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x00); /* disable int */
//	if (ret < 0)
//		printk(KERN_ERR "mcs6000_ts_suspend: i2c write failed\n");

	if (ts->power) {
		ret = ts->power(0);
		if (ret < 0)
			printk(KERN_ERR "mcs6000_ts_suspend: power off failed\n");
	}

	ts->status = MCS6000_DEV_SUSPEND;

	return 0;
}

static int mcs6000_ts_resume(struct i2c_client *client)
{
	int ret = 0;
	struct mcs6000_ts_data *ts = i2c_get_clientdata(client);

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	gpio_request(ts->num_irq, "MCS6000-IRQ");	

	gpio_direction_input(ts->intr_gpio);

	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0)
			printk(KERN_ERR "mcs6000_ts_resume: power on failed\n");
	}
//	msleep(100);

//	ret = i2c_smbus_write_byte_data(ts->client, 0x1d, 0x01); /* enable int */
//	if (ret < 0)
//		printk(KERN_ERR "mcs6000_ts_resume: i2c write failed\n");

	do{
		enable_irq(client->irq);
		ts->irq_sync++;
	}while(ts->irq_sync < 0);

	ts->status = MCS6000_DEV_NORMAL;

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs6000_early_suspend(struct early_suspend * h)
{	
	struct mcs6000_ts_data *ts = container_of(h, struct mcs6000_ts_data, early_suspend);

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000_early_suspend: ts is null\n");
		return;
	}

	if (ts->status == MCS6000_DEV_NORMAL)
		mcs6000_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void mcs6000_late_resume(struct early_suspend * h)
{	
	struct mcs6000_ts_data *ts = container_of(h, struct mcs6000_ts_data, early_suspend);

	if (MCS6000_DM_TRACE_FUNC & mcs6000_debug_mask)
		DMSG("\n");

	if (ts == (void *)NULL) {
		printk(KERN_ERR "mcs6000_early_resume: ts is null\n");
		return;
	}

	if (ts->status == MCS6000_DEV_SUSPEND)
		mcs6000_ts_resume(ts->client);
}
#endif

static const struct i2c_device_id mcs6000_ts_id[] = {
	{ MCS6000_I2C_TS_NAME, 1 },	
	{ }
};

static struct i2c_driver mcs6000_ts_driver = {
	.probe 		= mcs6000_ts_probe,
	.remove 	= mcs6000_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= mcs6000_ts_suspend,
	.resume  	= mcs6000_ts_resume,
#endif
	.id_table 	= mcs6000_ts_id,
	.driver = {
		.name 	= MCS6000_I2C_TS_NAME,
	},
};

static int __devinit mcs6000_ts_init(void)
{
	mcs6000_wq = create_singlethread_workqueue("mcs6000_wq");
	if (!mcs6000_wq)
		return -ENOMEM;

	return i2c_add_driver(&mcs6000_ts_driver);
}

static void __exit mcs6000_ts_exit(void)
{
	
	i2c_del_driver(&mcs6000_ts_driver);
	if (mcs6000_wq)
		destroy_workqueue(mcs6000_wq);
}

module_init(mcs6000_ts_init);
module_exit(mcs6000_ts_exit);

MODULE_AUTHOR("Kenobi Lee");
MODULE_DESCRIPTION("MELFAS MCS6000 Touchscreen Driver");
MODULE_LICENSE("GPL");

