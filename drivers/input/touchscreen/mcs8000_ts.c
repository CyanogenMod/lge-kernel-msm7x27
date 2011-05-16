/*
 * MELFAS mcs8000 touchscreen driver
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
/*
#include "touch_mcs8000_down_ioctl.h"
#include "touch_mcs8000_ioctl.h"
*/
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
static void mcs8000_early_suspend(struct early_suspend *h);
static void mcs8000_late_resume(struct early_suspend *h);
#endif

#if defined (CONFIG_MACH_MSM7X27_JUMP)
#define SUPPORT_TOUCH_KEY 1
#else
#define SUPPORT_TOUCH_KEY 0
#endif

#if SUPPORT_TOUCH_KEY
#define LG_FW_HARDKEY_BLOCK

#define TOUCH_SEARCH    247
#define TOUCH_BACK      248
#endif

#if defined (CONFIG_MACH_MSM7X27_JUMP)
#define TS_POLLING_TIME 2 /* msec */
#else
#define TS_POLLING_TIME 0 /* msec */
#endif

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
#define MCS7000_TS_MAX_HW_VERSION				0x40
#define MCS7000_TS_MAX_FW_VERSION				0x20


/* melfas data */
#define TS_MAX_Z_TOUCH			255
#define TS_MAX_W_TOUCH		    30
#define MTSI_VERSION		    0x07    //0x05

#define TS_MAX_X_COORD 		240
#define TS_MAX_Y_COORD 		320
#define FW_VERSION		0x00

#define TS_READ_START_ADDR 	0x10
#define TS_READ_VERSION_ADDR	0x31
#define TS_READ_REGS_LEN 	6
#define MELFAS_MAX_TOUCH 	5	

#define I2C_RETRY_CNT			10

#define PRESS_KEY				1
#define RELEASE_KEY				0
#define DEBUG_PRINT 			0

#define	SET_DOWNLOAD_BY_GPIO	1


enum
{
	None = 0,
	TOUCH_SCREEN,
	TOUCH_KEY
};

struct muti_touch_info
{
	int strength;
	int width;	
	int posX;
	int posY;
};


struct mcs8000_ts_device {
	struct i2c_client *client;
	struct input_dev *input_dev;
	//struct delayed_work work;
	struct work_struct  work;
#ifdef LG_FW_HARDKEY_BLOCK
	struct hrtimer touch_timer;
	bool hardkey_block;
#endif
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	struct workqueue_struct *ts_wq;
};

static struct input_dev *mcs8000_ts_input = NULL;
static struct mcs8000_ts_device mcs8000_ts_dev; 
static int is_downloading = 0;
static int is_touch_suspend = 0;
int fw_rev = 0;

#define READ_NUM 8 /* now, just using two finger data */

void Send_Touch( unsigned int x, unsigned int y)
{
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
	
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs8000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs8000_ts_dev.input_dev);
	input_sync(mcs8000_ts_dev.input_dev);
}
EXPORT_SYMBOL(Send_Touch);

//#define to_delayed_work(_work)  container_of(_work, struct delayed_work, work)

#ifdef LG_FW_HARDKEY_BLOCK
static enum hrtimer_restart timed_touch_timer_func(struct hrtimer *timer)
{
	mcs8000_ts_dev.hardkey_block = 0;	
	return HRTIMER_NORESTART;
}
#endif


static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];
/*
static int melfas_init_panel(struct mcs8000_ts_device *ts)
{
	int ret ;
	int buf = 0;
	ret = i2c_master_send(ts->client, &buf, 1);

	ret = i2c_master_send(ts->client, &buf, 1);

	if(ret <0)
	{
		printk(KERN_ERR "melfas_ts_probe: i2c_master_send() failed\n [%d]", ret);	
		return 0;
	}


	return true;
}
*/
static void mcs8000_work(struct work_struct *work)
{
	struct mcs8000_ts_device *ts = container_of(work, struct mcs8000_ts_device,work);
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
	int touchType=0, touchState =0, touchID=0, posX=0, posY=0, width = 0, strength=0, keyID = 0, reportID = 0;



#if DEBUG_PRINT
	printk(KERN_ERR "melfas_ts_work_func\n");

	if(ts ==NULL)
			printk(KERN_ERR "melfas_ts_work_func : TS NULL\n");
#endif 


#if 0
	/** 
	SMBus Block Read:
		S Addr Wr [A] Comm [A]
				S Addr Rd [A] [Data] A [Data] A ... A [Data] NA P
	*/
	ret = i2c_smbus_read_i2c_block_data(ts->client, TS_READ_START_ADDR, TS_READ_REGS_LEN, buf);
	if (ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func: i2c_smbus_read_i2c_block_data(), failed\n");
	}
#else
	/**
	Simple send transaction:
		S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
	Simple recv transaction:
		S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
	*/

	buf[0] = TS_READ_START_ADDR;

	for(i=0; i<I2C_RETRY_CNT; i++)
	{
		ret = i2c_master_send(ts->client, buf, 1);
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif 
		if(ret >=0)
		{
			ret = i2c_master_recv(ts->client, buf, TS_READ_REGS_LEN);
#if DEBUG_PRINT			
			printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);			
#endif


			if(ret >=0)
			{
				break; // i2c success
			}
		}
	}
#endif


	if (ret < 0)
	{
		printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		return ;	
	}
	else 
	{
		touchType  = (buf[0]>>5)&0x03;
		touchState = (buf[0]>>4)&0x01;
		
		reportID = (buf[0]&0x0f);
		posX = (  ( buf[1]& 0x0F) << (8)  ) +  buf[2];
		posY = (( (buf[1]& 0xF0 ) >> 4 ) << (8)) +  buf[3];
		
		if (touchType == TOUCH_KEY){
			keyID = (buf[0]&0x0f);
		}
		strength = buf[5]; 			

		touchID = reportID-1;

		if(touchID > MELFAS_MAX_TOUCH-1)
		{
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d\n",  touchID);
#endif			
			return ;
		}
		
		if(touchType == TOUCH_SCREEN)
		{
			g_Mtouch_info[touchID].posX= posX;
			g_Mtouch_info[touchID].posY= posY;
			g_Mtouch_info[touchID].width= width;			

			if(touchState)
				g_Mtouch_info[touchID].strength= strength;
			else
				g_Mtouch_info[touchID].strength = 0;


			for(i=0; i<MELFAS_MAX_TOUCH; i++)
			{
				if(g_Mtouch_info[i].strength== -1)
					continue;
				
				input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[i].posX);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[i].posY);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength );
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);      				
				input_mt_sync(ts->input_dev);          
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func: Touch ID: %d, State : %d, x: %d, y: %d, z: %d w: %d\n", 
			i, touchState, g_Mtouch_info[i].posX, g_Mtouch_info[i].posY, g_Mtouch_info[i].strength, g_Mtouch_info[i].width);
#endif					
				if(g_Mtouch_info[i].strength == 0)
					g_Mtouch_info[i].strength = -1;
				
			}
			
				
		}
		else if(touchType == TOUCH_KEY)
		{
			if (keyID == 0x1)
				input_report_key(ts->input_dev, KEY_MENU, touchState ? PRESS_KEY : RELEASE_KEY);		
			if (keyID == 0x2)
				input_report_key(ts->input_dev, KEY_HOME, touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x3)
				input_report_key(ts->input_dev, KEY_BACK, touchState ? PRESS_KEY : RELEASE_KEY);
			if (keyID == 0x4)
				input_report_key(ts->input_dev, KEY_SEARCH, touchState ? PRESS_KEY : RELEASE_KEY);			
#if DEBUG_PRINT
		printk(KERN_ERR "melfas_ts_work_func: keyID : %d, touchState: %d\n", keyID, touchState);
#endif				
		}

		input_sync(ts->input_dev);
	}

	msleep(1);
	enable_irq(ts->client->irq);
}

static irqreturn_t mcs8000_ts_irq_handler(int irq, void *handle)
{
	struct mcs8000_ts_device *dev = (struct mcs8000_ts_device *)handle;

		disable_irq_nosync(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		//schedule_delayed_work(&dev->work, 0);
		schedule_work(&dev->work);
	//	queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME));

	return IRQ_HANDLED;
}

static int mcs8000_ts_off(void)
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

	ret = dev->power(OFF);
	if(ret < 0)	{
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}

static int mcs8000_ts_on(void)
{
	struct mcs8000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs8000_ts_dev;

	ret = dev->power(ON);
	if(ret < 0)	{
		printk(KERN_ERR "mcs8000_ts_on power on failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}

void mcs8000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver)
{
	unsigned char data;
	struct mcs8000_ts_device *dev = NULL;
	int try_cnt = 0;
	dev = &mcs8000_ts_dev;
#if 0 
	msleep(200);
	/* mcs8000 need STOP signal after writing addres by bongkyu.kim */
	i2c_smbus_write_byte(dev->client, MCS7000_TS_FW_VERSION);
	data = i2c_smbus_read_byte(dev->client);
	printk(KERN_INFO "MCS7000 F/W Version [0x%x]\n", data);
	dev->input_dev->id.version = data;

	i2c_smbus_write_byte(dev->client, MCS7000_TS_HW_REVISION);
	data = i2c_smbus_read_byte(dev->client);
	printk(KERN_INFO "MCS7000 H/W Revision [0x%x]\n", data);
	dev->input_dev->id.product= data ;
#else
	msleep(200); /* mcs8000 need STOP signal after writing addres by bongkyu.kim */

	/* for avoiding the read fail form mcs8000 IC*/
	do {
		i2c_smbus_write_byte(dev->client, TS_READ_VERSION_ADDR);
		data = i2c_smbus_read_byte(dev->client);

		msleep(10);
		try_cnt ++;
	} while (data > MCS7000_TS_MAX_FW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS7000 F/W Version [0x%x]\n", data);
	*fw_ver = data;

	try_cnt = 0;
	do {
		i2c_smbus_write_byte(dev->client, TS_READ_VERSION_ADDR);
		data = i2c_smbus_read_byte(dev->client);

		msleep(10);
		try_cnt ++;
	} while (data > MCS7000_TS_MAX_HW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS7000 H/W Revision [0x%x]\n", data);
	*hw_ver = data;
/* working test for touch by younchan.kim */

	for ( try_cnt = 0 ; try_cnt < 0x35; try_cnt ++){
		i2c_smbus_write_byte(dev->client, try_cnt);
		data = i2c_smbus_read_byte(dev->client);
		
	//	data = i2c_smbus_read_byte_data(dev->client, try_cnt);
	//	data = i2c_smbus_read_byte_data(dev->client, MCS7000_TS_FW_VERSION);
		msleep(100);
		printk(KERN_INFO "MCS7000 register data add[0x%x],[0x%x]\n",try_cnt,data);
	}
#endif
}
/*
static struct miscdevice mcs8000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcs8000-touch",
	.fops = &mcs8000_ts_ioctl_fops,
};
*/

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	unsigned char hw_ver, fw_ver;

	mcs8000_firmware_info(&fw_ver, &hw_ver);
	
	r = sprintf(buf,"MCS7000 Touch Version HW:%02x FW:%02x\n",hw_ver, fw_ver);

	return r;
}
/*
static ssize_t read_touch_dl_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;

	r = sprintf(buf,"MCS7000 Download Status %d\n",is_downloading);
	return r;
}

static ssize_t read_touch_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	int int_status;
	struct mcs8000_ts_device *dev_tmp ;
	struct vreg *vreg_touch;

	dev_tmp = &mcs8000_ts_dev;
	vreg_touch = vreg_get(0, "synt");
	//printk ("Vreg_touch info : name [%s], id [%d],status[%d], refcnt[%d]\n",vreg_touch->name,vreg_touch->id,vreg_touch->status,vreg_touch->refcnt);
	int_status = gpio_get_value(dev_tmp->intr_gpio);
	r = sprintf(buf,"MCS7000 interrupt Pin [%d] , power Status [%d]\n",int_status,vreg_touch->refcnt);
	return r;
}
*/
#if 0
static ssize_t write_touch_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd,err;
	struct mcs8000_ts_device *dev_tmp;

	dev_tmp = &mcs8000_ts_dev;
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
		default :
			break;
	}
	return size;
}
#endif
//static DEVICE_ATTR(touch_control, S_IRUGO|S_IWUSR,NULL,write_touch_control);
//static DEVICE_ATTR(touch_status, S_IRUGO,read_touch_status, NULL);
static DEVICE_ATTR(version, S_IRUGO /*| S_IWUSR*/,read_touch_version, NULL);
//static DEVICE_ATTR(dl_status, S_IRUGO,read_touch_dl_status, NULL);
 

int mcs8000_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}
/*
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
*/
	return ret;
}

int mcs8000_remove_file(struct input_dev *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_version);
//	device_remove_file(&pdev->dev, &dev_attr_dl_status);
//	device_remove_file(&pdev->dev, &dev_attr_touch_status);
//	device_remove_file(&pdev->dev, &dev_attr_touch_control);
	return 0;
}

static int mcs8000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct touch_platform_data *ts_pdata;
	struct mcs8000_ts_device *dev;
	unsigned char fw_ver, hw_ver;
	//unsigned char tmp_val, tmp_reg ;

	DMSG("%s: start...\n", __FUNCTION__);

	ts_pdata = client->dev.platform_data;

	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_X, ts_pdata->ts_x_min, ts_pdata->ts_x_max, 0, 0);
	input_set_abs_params(mcs8000_ts_input, ABS_MT_POSITION_Y, ts_pdata->ts_y_min, ts_pdata->ts_y_max, 0, 0);

	dev = &mcs8000_ts_dev;

	//INIT_DELAYED_WORK(&dev->work, mcs8000_work);
	INIT_WORK(&dev->work, mcs8000_work);

	dev->power = ts_pdata->power;	
	dev->num_irq = client->irq;
	dev->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	dev->sda_gpio = ts_pdata->sda;
	dev->scl_gpio  = ts_pdata->scl;

#ifdef LG_FW_HARDKEY_BLOCK
	hrtimer_init(&dev->touch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dev->touch_timer.function = timed_touch_timer_func;
#endif
	dev->input_dev = mcs8000_ts_input;
	DMSG("mcs8000 dev->num_irq is %d , dev->intr_gpio is %d\n", dev->num_irq,dev->intr_gpio);

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

	/* TODO: You have try to change this driver's architecture using request_threaded_irq()
	 * So, I will change this to request_threaded_irq()
	 */
	err = request_threaded_irq(dev->num_irq, NULL, mcs8000_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mcs8000_ts", dev);

	if (err < 0) {
		printk(KERN_ERR "%s: request_irq failed\n", __FUNCTION__);
		return err;
	}

	disable_irq(dev->num_irq);
	mcs8000_ts_off();
	mdelay(10);
	mcs8000_ts_on();
	enable_irq(dev->num_irq);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts_early_suspend.suspend = mcs8000_early_suspend;
	ts_early_suspend.resume = mcs8000_late_resume;
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1 ;
	register_early_suspend(&ts_early_suspend);
#endif
	mcs8000_firmware_info(&fw_ver, &hw_ver);
	mcs8000_create_file(mcs8000_ts_input);  
	DMSG(KERN_INFO "%s: ts driver probed\n", __FUNCTION__);

	return 0;
}

static int mcs8000_ts_remove(struct i2c_client *client)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

	free_irq(dev->num_irq, dev);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
static int mcs8000_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

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

static int mcs8000_ts_resume(struct i2c_client *client)
{
	struct mcs8000_ts_device *dev = i2c_get_clientdata(client);

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
static void mcs8000_early_suspend(struct early_suspend * h)
{	
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);

		/* touch disable */
    gpio_set_value(28, 0);

		dev->power(OFF);
	}
	is_touch_suspend = 1;
}

static void mcs8000_late_resume(struct early_suspend * h)
{	
	struct mcs8000_ts_device *dev = &mcs8000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		mcs8000_ts_on();

		/* touch enable */
    gpio_set_value(28, 1);
		
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;
}
#endif

static const struct i2c_device_id mcs8000_ts_id[] = {
	{ "touch_mcs8000", 1 },	
	{ }
};


static struct i2c_driver mcs8000_i2c_ts_driver = {
	.probe = mcs8000_ts_probe,
	.remove = mcs8000_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mcs8000_ts_suspend,
	.resume  = mcs8000_ts_resume,
#endif
	.id_table = mcs8000_ts_id,
	.driver = {
		.name = "touch_mcs8000",
		.owner = THIS_MODULE,
	},
};

static int __devinit mcs8000_ts_init(void)
{
	int err = 0;
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;

	/* LGE_CHANGE [james.jang@lge.com] 2010-12-21, set gpio 28 config for touch enable */
	gpio_tlmm_config(GPIO_CFG(28, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), 
	  GPIO_CFG_ENABLE);
	/* touch enable */
  gpio_set_value(28, 1);

	memset(&mcs8000_ts_dev, 0, sizeof(struct mcs8000_ts_device));

	mcs8000_ts_input = input_allocate_device();
	if (mcs8000_ts_input == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		err = -ENOMEM;
		goto err_input_allocate;
	}

	mcs8000_ts_input->name = "touch_mcs8000";

	set_bit(EV_SYN, 	 mcs8000_ts_input->evbit);
	set_bit(EV_KEY, 	 mcs8000_ts_input->evbit);
	set_bit(EV_ABS, 	 mcs8000_ts_input->evbit);
	set_bit(ABS_MT_TOUCH_MAJOR, mcs8000_ts_input->absbit);
#if SUPPORT_TOUCH_KEY
	set_bit(KEY_BACK, mcs8000_ts_input->keybit);
	set_bit(KEY_MENU, mcs8000_ts_input->keybit);
	set_bit(KEY_HOME, mcs8000_ts_input->keybit);
	set_bit(KEY_SEARCH, mcs8000_ts_input->keybit);
#endif

	err = input_register_device(mcs8000_ts_input);
	if (err < 0) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto err_input_register;
	}

	err = i2c_add_driver(&mcs8000_i2c_ts_driver);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to probe i2c \n", __FUNCTION__);
		goto err_i2c_add_driver;
	}
/*
	err = misc_register(&mcs8000_ts_misc_dev);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to misc register\n", __FUNCTION__);
		goto err_misc_register;
	}
*/
	dev->ts_wq = create_singlethread_workqueue("ts_wq");
	if (!dev->ts_wq) {
		printk(KERN_ERR "%s: failed to create wp\n", __FUNCTION__);
		err = -1;
	}
	return err;

//err_misc_register:
//	misc_deregister(&mcs8000_ts_misc_dev);
err_i2c_add_driver:
	i2c_del_driver(&mcs8000_i2c_ts_driver);
err_input_register:
	input_unregister_device(mcs8000_ts_input);
err_input_allocate:
	input_free_device(mcs8000_ts_input);
	mcs8000_ts_input = NULL;
	return err;
}

static void __exit mcs8000_ts_exit(void)
{
	struct mcs8000_ts_device *dev;
	dev = &mcs8000_ts_dev;
	
//	mcs8000_remove_file(mcs8000_ts_input);
	i2c_del_driver(&mcs8000_i2c_ts_driver);
	input_unregister_device(mcs8000_ts_input);
	input_free_device(mcs8000_ts_input);

	if (dev->ts_wq)
		destroy_workqueue(dev->ts_wq);
	printk(KERN_INFO "touchscreen driver was unloaded!\nHave a nice day!\n");
}

module_init(mcs8000_ts_init);
module_exit(mcs8000_ts_exit);

MODULE_DESCRIPTION("MELFAS MCS7000 Touchscreen Driver");
MODULE_LICENSE("GPL");

