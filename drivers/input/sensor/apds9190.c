/*
 *  apds9190.c - Linux kernel modules for proximity sensor
 *
 *  Copyright (C) 2010 Lee Kai Koon <kai-koon.lee@avagotech.com>
 *  Copyright (C) 2010 Avago Technologies
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/input.h>

#include <mach/board_lge.h>

#define APDS9190_DRV_NAME	"apds9190"
#define DRIVER_VERSION		"1.0.4"

#define APDS9190_INT	IRQ_EINT20
#define APDS9190_PS_DETECTION_THRESHOLD		600
#define APDS9190_PS_HSYTERESIS_THRESHOLD	500

/* Change History 
 *
 * 1.0.1	Functions apds9190_show_rev(), apds9190_show_id() and apds9190_show_status()
 *			have missing CMD_BYTE in the i2c_smbus_read_byte_data(). APDS-9190 needs
 *			CMD_BYTE for i2c write/read byte transaction.
 *
 *
 * 1.0.2	Include PS switching threshold level when interrupt occurred
 *
 *
 * 1.0.3	Implemented ISR and delay_work, correct PS threshold storing
 *
 * 1.0.4	Added Input Report Event
 */

/*
 * Defines
 */

#define APDS9190_ENABLE_REG	0x00
#define APDS9190_ATIME_REG	0x01
#define APDS9190_PTIME_REG	0x02
#define APDS9190_WTIME_REG	0x03
#define APDS9190_AILTL_REG	0x04
#define APDS9190_AILTH_REG	0x05
#define APDS9190_AIHTL_REG	0x06
#define APDS9190_AIHTH_REG	0x07
#define APDS9190_PILTL_REG	0x08
#define APDS9190_PILTH_REG	0x09
#define APDS9190_PIHTL_REG	0x0A
#define APDS9190_PIHTH_REG	0x0B
#define APDS9190_PERS_REG	0x0C
#define APDS9190_CONFIG_REG	0x0D
#define APDS9190_PPCOUNT_REG	0x0E
#define APDS9190_CONTROL_REG	0x0F
#define APDS9190_REV_REG	0x11
#define APDS9190_ID_REG		0x12
#define APDS9190_STATUS_REG	0x13
#define APDS9190_CDATAL_REG	0x14
#define APDS9190_CDATAH_REG	0x15
#define APDS9190_IRDATAL_REG	0x16
#define APDS9190_IRDATAH_REG	0x17
#define APDS9190_PDATAL_REG	0x18
#define APDS9190_PDATAH_REG	0x19

#define CMD_BYTE	0x80
#define CMD_WORD	0xA0
#define CMD_SPECIAL	0xE0

#define CMD_CLR_PS_INT	0xE5
#define CMD_CLR_ALS_INT	0xE6
#define CMD_CLR_PS_ALS_INT	0xE7

/*
 * Structs
 */

struct apds9190_data {
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work	dwork;	/* for PS interrupt */
	struct delayed_work    poll_dwork; /* for timer polling */
	struct input_dev *input_dev_ps;

	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

	/* control flag from HAL */
	unsigned int enable_ps_sensor;

	/* PS parameters */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; /* always lower than ps_threshold */
	unsigned int ps_detection;		/* 0 = near-to-far; 1 = far-to-near */
	unsigned int ps_data;			/* to store PS data */

	unsigned int poll_delay;	/* needed for sensor polling : micro-second (us) */
};

/*
 * Global data
 */

/*
 * Management functions
 */

static int apds9190_set_command(struct i2c_client *client, int command)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	int clearInt;

	if (command == 0)
		clearInt = CMD_CLR_PS_INT;
	else if (command == 1)
		clearInt = CMD_CLR_ALS_INT;
	else
		clearInt = CMD_CLR_PS_ALS_INT;
		
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte(client, clearInt);
	mutex_unlock(&data->update_lock);

	return ret;
}

static int apds9190_set_enable(struct i2c_client *client, int enable)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ENABLE_REG, enable);
	mutex_unlock(&data->update_lock);

	data->enable = enable;

	return ret;
}

static int apds9190_set_atime(struct i2c_client *client, int atime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ATIME_REG, atime);
	mutex_unlock(&data->update_lock);

	data->atime = atime;

	return ret;
}

static int apds9190_set_ptime(struct i2c_client *client, int ptime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PTIME_REG, ptime);
	mutex_unlock(&data->update_lock);

	data->ptime = ptime;

	return ret;
}

static int apds9190_set_wtime(struct i2c_client *client, int wtime)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_WTIME_REG, wtime);
	mutex_unlock(&data->update_lock);

	data->wtime = wtime;

	return ret;
}

static int apds9190_set_ailt(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_AILTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	
	data->ailt = threshold;

	return ret;
}

static int apds9190_set_aiht(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_AIHTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	
	data->aiht = threshold;

	return ret;
}

static int apds9190_set_pilt(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PILTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	
	data->pilt = threshold;

	return ret;
}

static int apds9190_set_piht(struct i2c_client *client, int threshold)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PIHTL_REG, threshold);
	mutex_unlock(&data->update_lock);
	
	data->piht = threshold;

	return ret;
}

static int apds9190_set_pers(struct i2c_client *client, int pers)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PERS_REG, pers);
	mutex_unlock(&data->update_lock);

	data->pers = pers;

	return ret;
}

static int apds9190_set_config(struct i2c_client *client, int config)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_CONFIG_REG, config);
	mutex_unlock(&data->update_lock);

	data->config = config;

	return ret;
}

static int apds9190_set_ppcount(struct i2c_client *client, int ppcount)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_PPCOUNT_REG, ppcount);
	mutex_unlock(&data->update_lock);

	data->ppcount = ppcount;

	return ret;
}

static int apds9190_set_control(struct i2c_client *client, int control)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_CONTROL_REG, control);
	mutex_unlock(&data->update_lock);

	data->control = control;

	return ret;
}

static void apds9190_change_ps_threshold(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(client);

	data->ps_data =	i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_PDATAL_REG);

	if ( (data->ps_data > data->pilt) && (data->ps_data >= data->piht) ) {
		/* far-to-near detected */
		data->ps_detection = 1;

		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 1);/* FAR-to-NEAR detection */	
		input_sync(data->input_dev_ps);

		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PILTL_REG, data->ps_hysteresis_threshold);
		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PIHTL_REG, 1023);

		data->pilt = data->ps_hysteresis_threshold;
		data->piht = 1023;

		printk("far-to-near detected\n");
	}
	else if ( (data->ps_data <= data->pilt) && (data->ps_data < data->piht) ) {
		/* near-to-far detected */
		data->ps_detection = 0;

		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 0);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PILTL_REG, 0);
		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PIHTL_REG, data->ps_threshold);

		data->pilt = 0;
		data->piht = data->ps_threshold;

		printk("near-to-far detected\n");
	}
}

static void apds9190_reschedule_work(struct apds9190_data *data,
					  unsigned long delay)
{
	//unsigned long flags;

	//spin_lock_irqsave(&data->update_lock, flags);

	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->dwork);
	schedule_delayed_work(&data->dwork, delay);

	//spin_unlock_irqrestore(&data->update_lock, flags);
}

/* Polling routine */
static void apds9190_polling_work_handler(struct work_struct *work)
{
	struct apds9190_data *data = container_of(work, struct apds9190_data, poll_dwork.work);
	struct i2c_client *client=data->client;
	int cdata, irdata, pdata;
	
	cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_CDATAL_REG);
	irdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_IRDATAL_REG);
	pdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_PDATAL_REG);
		
	printk("%s: cdata = %x  irdata = %x pdata = %x \n", __func__, cdata, irdata, pdata);

	// check PS under sunlight
	if ( (data->ps_detection == 1) && (cdata > (75*(1024*(256-data->atime)))/100))	// PS was previously in far-to-near condition
	{
		// need to inform input event as there will be no interrupt from the PS
		input_report_abs(data->input_dev_ps, ABS_DISTANCE, 0);/* NEAR-to-FAR detection */	
		input_sync(data->input_dev_ps);

		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PILTL_REG, 0);
		i2c_smbus_write_word_data(client, CMD_WORD|APDS9190_PIHTL_REG, data->ps_threshold);

		data->pilt = 0;
		data->piht = data->ps_threshold;

		data->ps_detection = 0;	/* near-to-far detected */

		printk("apds_9190_proximity_handler = FAR\n");	
	}	
	
	schedule_delayed_work(&data->poll_dwork, msecs_to_jiffies(data->poll_delay));	// restart timer
}

/* PS interrupt routine */
static void apds9190_work_handler(struct work_struct *work)
{
	struct apds9190_data *data = container_of(work, struct apds9190_data, dwork.work);
	struct i2c_client *client=data->client;
	int	status;
	int cdata;

	status = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9190_STATUS_REG);

	i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ENABLE_REG, 1);	/* disable 9190's ADC first */

	printk("status = %x\n", status);

	if ((status & data->enable & 0x30) == 0x30) {
		
		cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_CDATAL_REG);
		if (cdata < (75*(1024*(256-data->atime)))/100)
			apds9190_change_ps_threshold(client);
		else {
			printk("Triggered by background ambient noise\n");
		}

		apds9190_set_command(client, 2);	/* 2 = CMD_CLR_PS_ALS_INT */
	}
	else if ((status & data->enable & 0x20) == 0x20) {
		/* only PS is interrupted */
		
		/* check if this is triggered by background ambient noise */
		cdata = i2c_smbus_read_word_data(client, CMD_WORD|APDS9190_CDATAL_REG);
		if (cdata < (75*(1024*(256-data->atime)))/100)
			apds9190_change_ps_threshold(client);
		else {
			printk("Triggered by background ambient noise\n");
		}

		apds9190_set_command(client, 0);	/* 0 = CMD_CLR_PS_INT */
	}
	else if ((status & data->enable & 0x10) == 0x10) {

		apds9190_set_command(client, 1);	/* 1 = CMD_CLR_ALS_INT */
	}

	i2c_smbus_write_byte_data(client, CMD_BYTE|APDS9190_ENABLE_REG, data->enable);	
}

/* assume this is ISR */
static irqreturn_t apds9190_interrupt(int vec, void *info)
{
	struct i2c_client *client=(struct i2c_client *)info;
	struct apds9190_data *data = i2c_get_clientdata(client);

	printk("==> apds9190_interrupt\n");
	apds9190_reschedule_work(data, 0);	

	return IRQ_HANDLED;
}

/*
 * SysFS support
 */

static ssize_t apds9190_show_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", data->enable_ps_sensor);
}

static ssize_t apds9190_store_enable_ps_sensor(struct device *dev,
				struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
 	unsigned long flags;
	
	printk("%s: enable ps senosr ( %ld)\n", __func__, val);
	
	if ((val != 0) && (val != 1)) {
		printk("%s:store unvalid value=%ld\n", __func__, val);
		return count;
	}
	
	if(val == 1) {
		//turn on p sensor
		if (data->enable_ps_sensor==0) {

			data->enable_ps_sensor= 1;
		
			apds9190_set_enable(client,0); /* Power Off */
			apds9190_set_atime(client, 0xf6); /* 27.2ms */
			apds9190_set_ptime(client, 0xff); /* 2.72ms */
		
			apds9190_set_ppcount(client, 8); /* 8-pulse */
			apds9190_set_control(client, 0x20); /* 100mA, IR-diode, 1X PGAIN, 1X AGAIN */
		
			apds9190_set_piht(client, 0);
			apds9190_set_piht(client, APDS9190_PS_DETECTION_THRESHOLD);
		
			data->ps_threshold = APDS9190_PS_DETECTION_THRESHOLD;
		
			apds9190_set_ailt( client, 0);
			apds9190_set_aiht( client, 0xffff);
		
			apds9190_set_pers(client, 0x33); /* 3 persistence */
		
			/* we need this polling timer routine for sunlight canellation */
			spin_lock_irqsave(&data->update_lock.wait_lock, flags); 
			
			/*
			 * If work is already scheduled then subsequent schedules will not
			 * change the scheduled time that's why we have to cancel it first.
			 */
			__cancel_delayed_work(&data->poll_dwork);
			schedule_delayed_work(&data->poll_dwork, msecs_to_jiffies(data->poll_delay));	// 100ms
			
			spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);	

			apds9190_set_enable(client, 0x27);	 /* only enable PS interrupt */
		}
		
	} 
	else {
		//turn off p sensor - kk 25 Apr 2011 we can't turn off the entire sensor, the light sensor may be needed by HAL
		data->enable_ps_sensor = 0;
		apds9190_set_enable(client, 0);

		spin_lock_irqsave(&data->update_lock.wait_lock, flags); 
			
		/*
		 * If work is already scheduled then subsequent schedules will not
		 * change the scheduled time that's why we have to cancel it first.
		 */
		__cancel_delayed_work(&data->poll_dwork);
	
		spin_unlock_irqrestore(&data->update_lock.wait_lock, flags); 
	}
		
	return count;
}

static DEVICE_ATTR(enable_ps_sensor, S_IWUGO | S_IRUGO,
				   apds9190_show_enable_ps_sensor, apds9190_store_enable_ps_sensor);


static ssize_t apds9190_show_poll_delay(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	
	return sprintf(buf, "%d\n", data->poll_delay*1000);	// return in micro-second
}

static ssize_t apds9190_store_poll_delay(struct device *dev,
					struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9190_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
 	unsigned long flags;
	
	if (val<5000)
		val = 5000;	// minimum 5ms
	
	data->poll_delay = val/1000;	// convert us => ms
	
	/* we need this polling timer routine for sunlight canellation */
	spin_lock_irqsave(&data->update_lock.wait_lock, flags); 
		
	/*
	 * If work is already scheduled then subsequent schedules will not
	 * change the scheduled time that's why we have to cancel it first.
	 */
	__cancel_delayed_work(&data->poll_dwork);
	schedule_delayed_work(&data->poll_dwork, msecs_to_jiffies(data->poll_delay));	// 100ms
			
	spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);	
	
	return count;
}

static DEVICE_ATTR(poll_delay, S_IWUSR | S_IRUGO,
				   apds9190_show_poll_delay, apds9190_store_poll_delay);

static struct attribute *apds9190_attributes[] = {
	&dev_attr_enable_ps_sensor.attr,
	&dev_attr_poll_delay.attr,
	NULL
};

static const struct attribute_group apds9190_attr_group = {
	.attrs = apds9190_attributes,
};

/*
 * Initialization function
 */

static int apds9190_init_client(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	int err;
	int id;

	err = apds9190_set_enable(client, 0);

	if (err < 0)
		return err;
	
	id = i2c_smbus_read_byte_data(client, CMD_BYTE|APDS9190_ID_REG);
	if (id == 0x29) {
		printk("APDS-9190\n");
	}
	else {
		printk("NOT APDS-9190\n");
		return -EIO;
	}

	apds9190_set_atime(client, 0xDB);	// 100.64ms ALS integration time
	apds9190_set_ptime(client, 0xFF);	// 2.72ms Prox integration time
	apds9190_set_wtime(client, 0xFF);	// 2.72ms Wait time

	apds9190_set_ppcount(client, 0x08);	// 8-Pulse for proximity
	apds9190_set_config(client, 0);		// no long wait
	apds9190_set_control(client, 0x20);	// 100mA, IR-diode, 1X PGAIN, 1X AGAIN

	apds9190_set_pilt(client, 0);		// init threshold for proximity
	apds9190_set_piht(client, APDS9190_PS_DETECTION_THRESHOLD);

	data->ps_threshold = APDS9190_PS_DETECTION_THRESHOLD;
	data->ps_hysteresis_threshold = APDS9190_PS_HSYTERESIS_THRESHOLD;

	apds9190_set_ailt(client, 0);		// init threshold for als
	apds9190_set_aiht(client, 0xFFFF);

	apds9190_set_pers(client, 0x22);	// 2 consecutive Interrupt persistence

	// sensor is in disabled mode but all the configurations are preset

	return 0;
}

/*
 * I2C init/probing/exit functions
 */

static struct i2c_driver apds9190_driver;
static int __devinit apds9190_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9190_data *data;
	struct proximity_platform_data *pdata;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct apds9190_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

	data->enable = 0;	/* default mode is standard */
	data->ps_threshold = 0;
	data->ps_hysteresis_threshold = 0;
	data->ps_detection = 0;	/* default to no detection */
	data->enable_ps_sensor = 0;	// default to 0
	data->poll_delay = 100;	// default to 100ms
	
	dev_info(&client->dev, "enable = %s\n",
			data->enable ? "1" : "0");

	mutex_init(&data->update_lock);

	if (request_irq(client->irq, apds9190_interrupt, IRQF_DISABLED|IRQ_TYPE_EDGE_FALLING,
		APDS9190_DRV_NAME, (void *)client)) {
		dev_info(&client->dev, "apds9190.c: Could not allocate APDS9190_INT !\n");
	
		goto exit_kfree;
	}

	INIT_DELAYED_WORK(&data->dwork, apds9190_work_handler);
	INIT_DELAYED_WORK(&data->poll_dwork, apds9190_polling_work_handler); 

	dev_info(&client->dev, "interrupt is hooked\n");

	pdata = client->dev.platform_data;
	
	if (pdata->power)
		pdata->power(1);

	msleep(10);

	/* Initialize the APDS9190 chip */
	err = apds9190_init_client(client);
	if (err)
		goto exit_kfree;

	/* Register to Input Device */
	data->input_dev_ps = input_allocate_device();
	if (!data->input_dev_ps) {
		err = -ENOMEM;
		printk("Failed to allocate input device proximity\n");
		goto exit_free_irq;
	}
	
	set_bit(EV_ABS, data->input_dev_ps->evbit);

	input_set_abs_params(data->input_dev_ps, ABS_DISTANCE, 0, 1, 0, 0);

	data->input_dev_ps->name = "Avago proximity sensor";

	err = input_register_device(data->input_dev_ps);
	if (err) {
		err = -ENOMEM;
		printk("Unable to register input device proximity: %s\n",
		       data->input_dev_ps->name);
		goto exit_free_dev_ps;
	}

	/* Register sysfs hooks */
	err = sysfs_create_group(&client->dev.kobj, &apds9190_attr_group);
	if (err)
		goto exit_unregister_dev_ps;

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

	return 0;

exit_unregister_dev_ps:
	input_unregister_device(data->input_dev_ps);	
exit_free_dev_ps:
	input_free_device(data->input_dev_ps);
exit_free_irq:
	free_irq(client->irq, client);	
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit apds9190_remove(struct i2c_client *client)
{
	struct apds9190_data *data = i2c_get_clientdata(client);
	
	input_unregister_device(data->input_dev_ps);
	
	input_free_device(data->input_dev_ps);

	free_irq(client->irq, client);

	sysfs_remove_group(&client->dev.kobj, &apds9190_attr_group);

	/* Power down the device */
	apds9190_set_enable(client, 0);

	kfree(data);

	return 0;
}

#ifdef CONFIG_PM

static int apds9190_suspend(struct i2c_client *client, pm_message_t mesg)
{
	return apds9190_set_enable(client, 0);
}

static int apds9190_resume(struct i2c_client *client)
{
	return apds9190_set_enable(client, 0);
}

#else

#define apds9190_suspend	NULL
#define apds9190_resume		NULL

#endif /* CONFIG_PM */

static const struct i2c_device_id apds9190_id[] = {
	{ "apds9190", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds9190_id);

static struct i2c_driver apds9190_driver = {
	.driver = {
		.name	= APDS9190_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.suspend = apds9190_suspend,
	.resume	= apds9190_resume,
	.probe	= apds9190_probe,
	.remove	= __devexit_p(apds9190_remove),
	.id_table = apds9190_id,
};

static int __init apds9190_init(void)
{
	return i2c_add_driver(&apds9190_driver);
}

static void __exit apds9190_exit(void)
{
	i2c_del_driver(&apds9190_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9190 proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9190_init);
module_exit(apds9190_exit);

