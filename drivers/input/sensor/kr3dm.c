/*
 *  drivers/input/sensor/kr3dm.c
 *
 *  Copyright (c) 2010 LGE.
 *
 *  All source code in this file is licensed under the following license
 *  except where indicated.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, you can find it at http://www.fsf.org
 */
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <asm/gpio.h>
#include <linux/input-polldev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include <mach/board_lge.h> // platform data
#include <linux/akm8973.h>	// akm daemon ioctl set

#undef CONFIG_HAS_EARLYSUSPEND
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>

struct early_suspend kr3dm_sensor_early_suspend;

static void kr3dm_early_suspend(struct early_suspend *h);
static void kr3dm_late_resume(struct early_suspend *h);
#endif

#define KR3DM_DEBUG_PRINT	(1)
#define KR3DM_ERROR_PRINT	(1)

/* KR3DM Debug mask value
 * usage: echo [mask_value] > /sys/module/kr3dm/parameters/debug_mask
 * All		: 3
 * No msg	: 0
 * default	: 0
 */
enum {
	KR3DM_DEBUG_FUNC_TRACE		= 1U << 0,
	KR3DM_DEBUG_DEV_LOW_DATA		= 1U << 1,
};

static unsigned int kr3dm_debug_mask = 0;

module_param_named(debug_mask, kr3dm_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#if defined(KR3DM_DEBUG_PRINT)
#define KR3DMD(fmt, args...) \
			printk(KERN_INFO "D[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define KR3DMD(fmt, args...)	{};
#endif

#if defined(KR3DM_ERROR_PRINT)
#define KR3DME(fmt, args...) \
			printk(KERN_ERR "E[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define KR3DME(fmt, args...)	{};
#endif

#define USE_WORK_QUEUE        0

/** Maximum polled-device-reported g value */
#define G_MAX			8000

#define SHIFT_ADJ_2G		4
#define SHIFT_ADJ_4G		3
#define SHIFT_ADJ_8G		2

#define AXISDATA_REG		0x28

#define CTRL_REG1		0x20	/* ctrl 1: pm2 pm1 pm0 dr1 dr0 zenable yenable zenable */
#define CTRL_REG2		0x21	/* filter setting */
#define CTRL_REG3		0x22	/* interrupt control reg */
#define CTRL_REG4		0x23
#define CTRL_REG5		0x23	/* scale selection */

#define PM_OFF          	0x00
#define PM_NORMAL       	0x20
#define ENABLE_ALL_AXES 	0x07

#define ODRHALF         	0x40	/* 0.5Hz output data rate */
#define ODR1            	0x60	/* 1Hz output data rate */
#define ODR2            	0x80	/* 2Hz output data rate */
#define ODR5            	0xA0	/* 5Hz output data rate */
#define ODR10           	0xC0	/* 10Hz output data rate */
#define ODR50           	0x00	/* 50Hz output data rate */
#define ODR100          	0x08	/* 100Hz output data rate */
#define ODR400          	0x10	/* 400Hz output data rate */
#define ODR1000         	0x18	/* 1000Hz output data rate */

#define FUZZ			32
#define FLAT			32
#define I2C_RETRY_DELAY		5
#define I2C_RETRIES		5
#define AUTO_INCREMENT		0x80

/** The following define the IOCTL command values via the ioctl macros */
#define KR3DM_IOCTL_BASE 'a'
#define KR3DM_IOCTL_SET_DELAY       _IOW(KR3DM_IOCTL_BASE, 0, int)
#define KR3DM_IOCTL_GET_DELAY       _IOR(KR3DM_IOCTL_BASE, 1, int)
#define KR3DM_IOCTL_SET_ENABLE      _IOW(KR3DM_IOCTL_BASE, 2, int)
#define KR3DM_IOCTL_GET_ENABLE      _IOR(KR3DM_IOCTL_BASE, 3, int)
#define KR3DM_IOCTL_SET_G_RANGE     _IOW(KR3DM_IOCTL_BASE, 4, int)
#define KR3DM_IOCTL_READ_ACCEL_XYZ  _IOW(KR3DM_IOCTL_BASE, 8, int)

#define OUT_X          0x29
#define OUT_Y          0x2B
#define OUT_Z          0x2D

#define KR3DM_G_2G 0x00
#define KR3DM_G_4G 0x10
#define KR3DM_G_8G 0x30

#define WHO_AM_I		0x0f
#define KR3DM_DEVICE_ID	0x12

struct {
	unsigned int cutoff;
	unsigned int mask;
} odr_table_m[] = {
	{
	3,	PM_NORMAL | ODR1000}, {
	10,	PM_NORMAL | ODR400}, {
	20,	PM_NORMAL | ODR100}, {
	100,	PM_NORMAL | ODR50}, {
	200,	ODR1000	| ODR10}, {
	500,	ODR1000 | ODR5}, {
	1000,	ODR1000 | ODR2}, {
	2000,	ODR1000 | ODR1}, {
	0,	ODR1000 | ODRHALF},};

struct kr3dm_data {
	struct i2c_client *client;
	struct kr3dh_platform_data *pdata;

	struct mutex lock;

	struct delayed_work input_work;
	struct input_dev *input_dev;

	int hw_initialized;
	atomic_t enabled;
	int on_before_suspend;

	u8 shift_adj;
	u8 resume_state[5];
};

/*
 * Because misc devices can not carry a pointer from driver register to
 * open, we keep this global.  This limits the driver to a single instance.
 */
struct kr3dm_data *kr3dm_misc_data;

static int kr3dm_i2c_read(struct kr3dm_data *kr, u8 * buf, int len)
{
	int err;
	int tries = 0;
	struct i2c_msg msgs[] = {
		{
		 .addr = kr->client->addr,
		 .flags = kr->client->flags & I2C_M_TEN,
		 .len = 1,
		 .buf = buf,
		 },
		{
		 .addr = kr->client->addr,
		 .flags = (kr->client->flags & I2C_M_TEN) | I2C_M_RD,
		 .len = len,
		 .buf = buf,
		 },
	};

	do {
		err = i2c_transfer(kr->client->adapter, msgs, 2);
		if (err != 2)
			msleep_interruptible(I2C_RETRY_DELAY);
	} while ((err != 2) && (++tries < I2C_RETRIES));

	if (err != 2) {
		dev_err(&kr->client->dev, "read transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int kr3dm_i2c_write(struct kr3dm_data *kr, u8 * buf, int len)
{
	int err;
	int tries = 0;
	struct i2c_msg msgs[] = {
		{
		 .addr = kr->client->addr,
		 .flags = kr->client->flags & I2C_M_TEN,
		 .len = len + 1,
		 .buf = buf,
		 },
	};

	do {
		err = i2c_transfer(kr->client->adapter, msgs, 1);
		if (err != 1)
			msleep_interruptible(I2C_RETRY_DELAY);
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&kr->client->dev, "write transfer error\n");
		err = -EIO;
	} else {
		err = 0;
	}

	return err;
}

static int kr3dm_hw_init(struct kr3dm_data *kr)
{
	int err = -1;
	u8 buf[6];

	buf[0] = (AUTO_INCREMENT | CTRL_REG1);
	buf[1] = kr->resume_state[0];
	buf[2] = kr->resume_state[1];
	buf[3] = kr->resume_state[2];
	buf[4] = kr->resume_state[3];
	buf[5] = kr->resume_state[4];
	err = kr3dm_i2c_write(kr, buf, 5);
	if (err < 0)
		return err;

	kr->hw_initialized = 1;

	return 0;
}

static void kr3dm_device_power_off(struct kr3dm_data *kr)
{
	int err;
	u8 buf[2] = {CTRL_REG1, PM_OFF};

	err = kr3dm_i2c_write(kr, buf, 1);
	if (err < 0)
		dev_err(&kr->client->dev, "soft power off failed\n");

	if (kr->pdata->power_off) {
		kr->pdata->power_off();
		kr->hw_initialized = 0;
	}
}

static int kr3dm_device_power_on(struct kr3dm_data *kr)
{

	int err;

	if (kr->pdata->power_on) {
		err = kr->pdata->power_on();
		if (err < 0)
			return err;

		mdelay(5);	// KR3DM boot up time
	}

	if (!kr->hw_initialized) {
		err = kr3dm_hw_init(kr);
		if (err < 0) {
			kr3dm_device_power_off(kr);
			return err;
		}
	}

	return 0;
}

int kr3dm_update_g_range(struct kr3dm_data *kr, u8 new_g_range)
{
	int err;
	u8 shift;
	u8 buf[2];

	switch (new_g_range) {
		case KR3DM_G_2G:
			shift = SHIFT_ADJ_2G;
			break;
		case KR3DM_G_4G:
			shift = SHIFT_ADJ_4G;
			break;
		case KR3DM_G_8G:
			shift = SHIFT_ADJ_8G;
			break;
		default:
			return -EINVAL;
	}

	if (atomic_read(&kr->enabled)) {
		/* Set configuration register 4, which contains g range setting
		 *  NOTE: this is a straight overwrite because this driver does
		 *  not use any of the other configuration bits in this
		 *  register.  Should this become untrue, we will have to read
		 *  out the value and only change the relevant bits --XX----
		 *  (marked by X) */
		buf[0] = CTRL_REG4;
		buf[1] = new_g_range;
		err = kr3dm_i2c_write(kr, buf, 1);
		if (err < 0)
			return err;
	}

	kr->resume_state[3] = new_g_range;
	kr->shift_adj = shift;

	return 0;
}

int kr3dm_update_odr(struct kr3dm_data *kr, int poll_interval)
{
	int err = -1;
	int i;
	u8 config[2];

	/* Convert the poll interval into an output data rate configuration
	 *  that is as low as possible.  The ordering of these checks must be
	 *  maintained due to the cascading cut off values - poll intervals are
	 *  checked from shortest to longest.  At each check, if the next lower
	 *  ODR cannot support the current poll interval, we stop searching */
	for (i = 0; i < ARRAY_SIZE(odr_table_m); i++) {
		config[1] = odr_table_m[i].mask;
		if (poll_interval < odr_table_m[i].cutoff)
			break;
	}

	config[1] |= ENABLE_ALL_AXES;

	/* If device is currently enabled, we need to write new
	 *  configuration out to it */
	if (atomic_read(&kr->enabled)) {
		config[0] = CTRL_REG1;
		err = kr3dm_i2c_write(kr, config, 1);
		if (err < 0)
			return err;
	}

	kr->resume_state[0] = config[1];

	return 0;
}

static int kr3dm_get_acceleration_data(struct kr3dm_data *kr, int *xyz)
{
	int err = -1;
	/* Data bytes from hardware xL, xH, yL, yH, zL, zH */
	u8 acc_data[6];

#if 1
	acc_data[0] = OUT_X;
	acc_data[1] = OUT_Y;
	acc_data[2] = OUT_Z;

	err = kr3dm_i2c_read(kr, &acc_data[0], 1);
	xyz[0] = (int)((signed char)acc_data[0]);

	err = kr3dm_i2c_read(kr,&acc_data[1], 1);
	xyz[1] = (int)((signed char)acc_data[1]);

	err = kr3dm_i2c_read(kr,&acc_data[2], 1);
	xyz[2] = (int)((signed char)acc_data[2]);
#else
	/* x,y,z hardware data */
	int hw_d[3] = { 0 };

	acc_data[0] = (AUTO_INCREMENT | AXISDATA_REG);
	err = kr3dm_i2c_read(kr, acc_data, 6);
	if (err < 0)
		return err;

	hw_d[0] = (int) (((acc_data[1]) << 8) | acc_data[0]);
	hw_d[1] = (int) (((acc_data[3]) << 8) | acc_data[2]);
	hw_d[2] = (int) (((acc_data[5]) << 8) | acc_data[4]);

	hw_d[0] = (hw_d[0] & 0x8000) ? (hw_d[0] | 0xFFFF0000) : (hw_d[0]);
	hw_d[1] = (hw_d[1] & 0x8000) ? (hw_d[1] | 0xFFFF0000) : (hw_d[1]);
	hw_d[2] = (hw_d[2] & 0x8000) ? (hw_d[2] | 0xFFFF0000) : (hw_d[2]);

	hw_d[0] >>= kr->shift_adj;
	hw_d[1] >>= kr->shift_adj;
	hw_d[2] >>= kr->shift_adj;

	xyz[0] = ((kr->pdata->negate_x) ? (-hw_d[kr->pdata->axis_map_x])
		  : (hw_d[kr->pdata->axis_map_x]));
	xyz[1] = ((kr->pdata->negate_y) ? (-hw_d[kr->pdata->axis_map_y])
		  : (hw_d[kr->pdata->axis_map_y]));
	xyz[2] = ((kr->pdata->negate_z) ? (-hw_d[kr->pdata->axis_map_z])
		  : (hw_d[kr->pdata->axis_map_z]));
#endif

	if (KR3DM_DEBUG_DEV_LOW_DATA & kr3dm_debug_mask)
		KR3DMD("x=%10d, y=%10d, z=%10d\n", xyz[0],xyz[1], xyz[2]);

	return err;
}

#if USE_WORK_QUEUE
static void kr3dm_report_values(struct kr3dm_data *kr, int *xyz)
{
	input_report_abs(kr->input_dev, ABS_X, xyz[0]);
	input_report_abs(kr->input_dev, ABS_Y, xyz[1]);
	input_report_abs(kr->input_dev, ABS_Z, xyz[2]);
	input_sync(kr->input_dev);

	//dev_info(&kr->client->dev, "(x, y ,z) = (%d, %d, %d)\n", xyz[0],xyz[1], xyz[2]);
}
#endif

static int kr3dm_enable(struct kr3dm_data *kr)
{
	int err;

	if (!atomic_cmpxchg(&kr->enabled, 0, 1)) {

		err = kr3dm_device_power_on(kr);
		if (err < 0) {
			atomic_set(&kr->enabled, 0);
			return err;
		}
#if USE_WORK_QUEUE
		schedule_delayed_work(&kr->input_work,
				      msecs_to_jiffies(kr->
						       pdata->poll_interval));
#endif

	}

	return 0;
}

static int kr3dm_disable(struct kr3dm_data *kr)
{
	if (atomic_cmpxchg(&kr->enabled, 1, 0)) {
#if USE_WORK_QUEUE
		cancel_delayed_work_sync(&kr->input_work);
#endif
		kr3dm_device_power_off(kr);
	}

	return 0;
}

static int kr3dm_misc_open(struct inode *inode, struct file *file)
{
	int err;
	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	file->private_data = kr3dm_misc_data;

	return 0;
}

static int kr3dm_misc_ioctl(struct inode *inode, struct file *file,
				unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int buf[3];
	int err;
	int interval;
	struct kr3dm_data *kr = file->private_data;

	switch (cmd) {
	case KR3DM_IOCTL_GET_DELAY:
		interval = kr->pdata->poll_interval;
		if (copy_to_user(argp, &interval, sizeof(interval)))
			return -EFAULT;
		break;

	case KR3DM_IOCTL_SET_DELAY:
		if (copy_from_user(&interval, argp, sizeof(interval)))
			return -EFAULT;
		if (interval < 0 || interval > 200)
			return -EINVAL;

		kr->pdata->poll_interval =
		    max(interval, kr->pdata->min_interval);
		err = kr3dm_update_odr(kr, kr->pdata->poll_interval);
		/* TODO: if update fails poll is still set */
		if (err < 0)
			return err;

		break;

	case KR3DM_IOCTL_SET_ENABLE:
		if (copy_from_user(&interval, argp, sizeof(interval)))
			return -EFAULT;
		if (interval > 1)
			return -EINVAL;

		if (interval)
			kr3dm_enable(kr);
		else
			kr3dm_disable(kr);

		break;

	case KR3DM_IOCTL_GET_ENABLE:
		interval = atomic_read(&kr->enabled);
		if (copy_to_user(argp, &interval, sizeof(interval)))
			return -EINVAL;

		break;

	case KR3DM_IOCTL_SET_G_RANGE:
		if (copy_from_user(&buf, argp, 1))
			return -EFAULT;
		err = kr3dm_update_g_range(kr, arg);
		if (err < 0)
			return err;

		break;

	case KR3DM_IOCTL_READ_ACCEL_XYZ:
		err=kr3dm_get_acceleration_data(kr, buf);
		if (err < 0)
				return err;

		if (copy_to_user(argp, buf, sizeof(int)*3))
			return -EINVAL;

		return err;

		break;

	case AKMD2_TO_ACCEL_IOCTL_READ_XYZ:	/* LGE_CHANGE [hyesung.shin@lge.com] on 2010-1-23, for <Sensor driver structure> */
		err=kr3dm_get_acceleration_data(kr, buf);
		if (err < 0)
				return err;

		if (copy_to_user(argp, buf, sizeof(int)*3))
			return -EINVAL;

		return err;

		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations kr3dm_misc_fops = {
	.owner = THIS_MODULE,
	.open = kr3dm_misc_open,
	.ioctl = kr3dm_misc_ioctl,
};

static struct miscdevice kr3dm_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "KR3DM",
	.fops = &kr3dm_misc_fops,
};

#if USE_WORK_QUEUE
static void kr3dm_input_work_func(struct work_struct *work)
{
	struct kr3dm_data *kr = container_of((struct delayed_work *)work,
						  struct kr3dm_data,
						  input_work);
	int xyz[3] = { 0 };
	int err;

	mutex_lock(&kr->lock);
	err = kr3dm_get_acceleration_data(kr, xyz);
	if (err < 0)
		dev_err(&kr->client->dev, "get_acceleration_data failed\n");
	else
		kr3dm_report_values(kr, xyz);

	schedule_delayed_work(&kr->input_work,
			      msecs_to_jiffies(kr->pdata->poll_interval));
	mutex_unlock(&kr->lock);
}
#endif // USE_WORK_QUEUE

#ifdef KR3DM_OPEN_ENABLE
int kr3dm_input_open(struct input_dev *input)
{
	struct kr3dm_data *kr = input_get_drvdata(input);

	return kr3dm_enable(kr);
}

void kr3dm_input_close(struct input_dev *dev)
{
	struct kr3dm_data *kr = input_get_drvdata(dev);

	kr3dm_disable(kr);
}
#endif

static int kr3dm_validate_pdata(struct kr3dm_data *kr)
{
	kr->pdata->poll_interval = max(kr->pdata->poll_interval,
					kr->pdata->min_interval);

	if (kr->pdata->axis_map_x > 2 ||
	    kr->pdata->axis_map_y > 2 || kr->pdata->axis_map_z > 2) {
		dev_err(&kr->client->dev,
			"invalid axis_map value x:%u y:%u z%u\n",
			kr->pdata->axis_map_x, kr->pdata->axis_map_y,
			kr->pdata->axis_map_z);
		return -EINVAL;
	}

	/* Only allow 0 and 1 for negation boolean flag */
	if (kr->pdata->negate_x > 1 || kr->pdata->negate_y > 1 ||
	    kr->pdata->negate_z > 1) {
		dev_err(&kr->client->dev,
			"invalid negate value x:%u y:%u z:%u\n",
			kr->pdata->negate_x, kr->pdata->negate_y,
			kr->pdata->negate_z);
		return -EINVAL;
	}

	/* Enforce minimum polling interval */
	if (kr->pdata->poll_interval < kr->pdata->min_interval) {
		dev_err(&kr->client->dev, "minimum poll interval violated\n");
		return -EINVAL;
	}

	return 0;
}

static int kr3dm_input_init(struct kr3dm_data *kr)
{
	int err;

#if USE_WORK_QUEUE
	INIT_DELAYED_WORK(&kr->input_work, kr3dm_input_work_func);

#endif
	kr->input_dev = input_allocate_device();
	if (!kr->input_dev) {
		err = -ENOMEM;
		dev_err(&kr->client->dev, "input device allocate failed\n");
		goto err0;
	}

#ifdef KR3DM_OPEN_ENABLE
	kr->input_dev->open = kr3dm_input_open;
	kr->input_dev->close = kr3dm_input_close;
#endif

	input_set_drvdata(kr->input_dev, kr);

	set_bit(EV_ABS, kr->input_dev->evbit);

	input_set_abs_params(kr->input_dev, ABS_X, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(kr->input_dev, ABS_Y, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(kr->input_dev, ABS_Z, -G_MAX, G_MAX, FUZZ, FLAT);

	kr->input_dev->name = "accelerometer";

	err = input_register_device(kr->input_dev);
	if (err) {
		dev_err(&kr->client->dev,
			"unable to register input polled device %s\n",
			kr->input_dev->name);
		goto err1;
	}


	return 0;

err1:
	input_free_device(kr->input_dev);
err0:
	return err;
}

static void kr3dm_input_cleanup(struct kr3dm_data *kr)
{
	input_unregister_device(kr->input_dev);
	input_free_device(kr->input_dev);
}

static int kr3dm_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct kr3dm_data *kr;
	int err = -1;
	u8 id_check = WHO_AM_I;

	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		err = -ENODEV;
		goto err0;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "client not i2c capable\n");
		err = -ENODEV;
		goto err0;
	}

	kr = kzalloc(sizeof(*kr), GFP_KERNEL);
	if (kr == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto err0;
	}

	mutex_init(&kr->lock);
	mutex_lock(&kr->lock);
	kr->client = client;

	kr->pdata = kmalloc(sizeof(*kr->pdata), GFP_KERNEL);
	if (kr->pdata == NULL)
		goto err1;

	memcpy(kr->pdata, client->dev.platform_data, sizeof(*kr->pdata));

	err = kr3dm_validate_pdata(kr);
	if (err < 0) {
		dev_err(&client->dev, "failed to validate platform data\n");
		goto err1_1;
	}

	i2c_set_clientdata(client, kr);

	if (kr->pdata->kr_init) {
		err = kr->pdata->kr_init();
		if (err < 0)
			goto err1_1;
	}

	memset(kr->resume_state, 0, ARRAY_SIZE(kr->resume_state));

	/* control register setting */
	kr->resume_state[0] = PM_NORMAL | ENABLE_ALL_AXES;
	kr->resume_state[1] = 0;
	kr->resume_state[2] = 0;
	kr->resume_state[3] = 0;
	kr->resume_state[4] = 0;

	err = kr3dm_device_power_on(kr);
	if (err < 0)
		goto err2;

	err = kr3dm_i2c_read(kr, &id_check, 1);
	if(id_check != KR3DM_DEVICE_ID)
	{
		dev_err(&client->dev, "Device ID not matched\n");
		err = -ENODEV;
		goto err2;
	}

	atomic_set(&kr->enabled, 1);

	err = kr3dm_update_g_range(kr, kr->pdata->g_range);
	if (err < 0) {
		dev_err(&client->dev, "update_g_range failed\n");
		goto err2;
	}

	err = kr3dm_update_odr(kr, kr->pdata->poll_interval);
	if (err < 0) {
		dev_err(&client->dev, "update_odr failed\n");
		goto err2;
	}

	err = kr3dm_input_init(kr);
	if (err < 0)
		goto err3;

	kr3dm_misc_data = kr;

	err = misc_register(&kr3dm_misc_device);
	if (err < 0) {
		dev_err(&client->dev, "krd_device register failed\n");
		goto err4;
	}

#if defined(CONFIG_HAS_EARLYSUSPEND)
	kr3dm_sensor_early_suspend.suspend = kr3dm_early_suspend;
	kr3dm_sensor_early_suspend.resume = kr3dm_late_resume;
	register_early_suspend(&kr3dm_sensor_early_suspend);
#endif

#if 0
	kr3dm_device_power_off(kr);

	/* As default, do not report information */
	atomic_set(&kr->enabled, 0);
#endif

	mutex_unlock(&kr->lock);

	dev_info(&client->dev, "%s kr3dm: Accelerometer chip found\n", client->name);

	return 0;

err4:
	kr3dm_input_cleanup(kr);
err3:
	kr3dm_device_power_off(kr);
err2:
	if (kr->pdata->kr_exit)
		kr->pdata->kr_exit();
err1_1:
	mutex_unlock(&kr->lock);
	kfree(kr->pdata);
err1:
	kfree(kr);
err0:
	return err;
}

static int __devexit kr3dm_remove(struct i2c_client *client)
{
	/* TODO: revisit ordering here once _probe order is finalized */
	struct kr3dm_data *kr = i2c_get_clientdata(client);

	misc_deregister(&kr3dm_misc_device);
	kr3dm_input_cleanup(kr);
	kr3dm_device_power_off(kr);
	if (kr->pdata->kr_exit)
		kr->pdata->kr_exit();
	kfree(kr->pdata);
	kfree(kr);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&kr3dm_sensor_early_suspend);
#endif

	return 0;
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void kr3dm_early_suspend(struct early_suspend *h)
{
	kr3dm_disable(kr3dm_misc_data);
}

static void kr3dm_late_resume(struct early_suspend *h)
{
	kr3dm_enable(kr3dm_misc_data);
}
#endif

#if defined(CONFIG_PM)
static int kr3dm_resume(struct device *device)
{
	struct i2c_client *client = i2c_verify_client(device);
	struct kr3dm_data *kr = i2c_get_clientdata(client);

#if 0
	int err = 0;

	if (kr->on_before_suspend){

		kr->pdata->gpio_config(1);

		return kr3dm_enable(kr);
	}

	err =  kr3dm_hw_init(kr);
	if (err < 0)
		printk("%s i2c failed\n", __FUNCTION__);

	return 0;
#endif

	return kr3dm_enable(kr);
}

static int kr3dm_suspend(struct device *device)
{
	struct i2c_client *client = i2c_verify_client(device);
	struct kr3dm_data *kr = i2c_get_clientdata(client);

#if 0
	kr->on_before_suspend = atomic_read(&kr->enabled);

	if (kr->on_before_suspend){
		kr->pdata->gpio_config(0);
	}
#endif

	return kr3dm_disable(kr);
}
#endif

static const struct i2c_device_id kr3dm_id[] = {
	{"KR3DM", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, kr3dm_id);

#if defined(CONFIG_PM)
static struct dev_pm_ops kr3dm_pm_ops = {
       .suspend = kr3dm_suspend,
       .resume = kr3dm_resume,
};
#endif

static struct i2c_driver kr3dm_driver = {
	.driver = {
		   .name = "KR3DM",
#if defined(CONFIG_PM)
		   .pm = &kr3dm_pm_ops,
#endif
		   },
	.probe = kr3dm_probe,
	.remove = __devexit_p(kr3dm_remove),
	.id_table = kr3dm_id,
};

static int __init kr3dm_init(void)
{
	pr_info("KR3DM accelerometer driver\n");

	return i2c_add_driver(&kr3dm_driver);
}

static void __exit kr3dm_exit(void)
{
	i2c_del_driver(&kr3dm_driver);
	return;
}

module_init(kr3dm_init);
module_exit(kr3dm_exit);

MODULE_DESCRIPTION("kr3dm accelerometer driver");
MODULE_AUTHOR("STMicroelectronics");
