/* drivers/i2c/chips/akm8973.c - akm8973 compass driver
 *
 * Copyright (C) 2007-2008 HTC Corporation.
 * Author: Hou-Kun Chen <houkun.chen@gmail.com>
 *
 * rework by hyesung.shin@lge.com on 2010-1-21, for <Sensor driver structure>
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

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/ioctl.h>
#include <mach/system.h>

#include <mach/board_lge.h>

#include <linux/akm8973.h>

#define MAX_FAILURE_COUNT 10

#define AKMD_DEBUG 0
#if AKMD_DEBUG
#define ADBG(fmt, args...) printk(fmt, ##args)
#else
#define ADBG(fmt, args...) do {} while (0)
#endif /* AKMD_DEBUG */

static unsigned short normal_i2c[] = { I2C_CLIENT_END };

I2C_CLIENT_INSMOD;

static struct i2c_client *this_client;

struct akm8973_data {
	struct input_dev *input_dev;
//	struct work_struct work;
};

/* Addresses to scan -- protected by sense_data_mutex */
//static char sense_data[RBUFF_SIZE + 1];
//static struct mutex sense_data_mutex;

//static DECLARE_WAIT_QUEUE_HEAD(data_ready_wq);
static DECLARE_WAIT_QUEUE_HEAD(open_wq);

//static atomic_t data_ready;
static atomic_t open_count;
static atomic_t open_flag;
static atomic_t reserve_open_flag;

static atomic_t m_flag;
static atomic_t a_flag;
static atomic_t t_flag;
static atomic_t mv_flag;
static atomic_t p_flag;
static int first_start = 0 ;

static int powerDownOrOff=0; //For used current //diyu@lge.com
static int akm8973_set_vreg_check=  0;

//static int failure_count = 0;

static short akmd_delay = 0;

/* LGE_CHANGE_S [ey.cho@lge.com] 2010-02-22, for TestMode */
short pitch, roll;
/* LGE_CHANGE_E [ey.cho@lge.com] 2010-02-22*/
#if defined(CONFIG_PM)
static atomic_t suspend_flag = ATOMIC_INIT(0);
#endif

static char akecs_power_mode; /* LGE_CHANGE [hyesung.shin@lge.com] on 2010-2-22, for <Sensor power consumption> */

//static struct akm8973_platform_data *pdata;

/* following are the sysfs callback functions */

#define config_ctrl_reg(name,address) \
static ssize_t name##_show(struct device *dev, struct device_attribute *attr, \
			   char *buf) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
        return sprintf(buf, "%u\n", i2c_smbus_read_byte_data(client,address)); \
} \
static ssize_t name##_store(struct device *dev, struct device_attribute *attr, \
			    const char *buf,size_t count) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
	unsigned long val = simple_strtoul(buf, NULL, 10); \
	if (val > 0xff) \
		return -EINVAL; \
	i2c_smbus_write_byte_data(client,address, val); \
        return count; \
} \
static DEVICE_ATTR(name, S_IWUSR | S_IRUGO, name##_show, name##_store)

config_ctrl_reg(ms1, AKECS_REG_MS1);
/* LGE_CHANGE_S [ey.cho@lge.com] 2010-02-22, for TestMode */
static ssize_t p_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", pitch);
}


static DEVICE_ATTR(pitch, S_IRUGO | S_IWUSR, p_show, NULL);

static ssize_t r_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", roll);
}


static DEVICE_ATTR(roll, S_IRUGO | S_IWUSR, r_show, NULL);
/* LGE_CHANGE_E [ey.cho@lge.com] 2010-02-22*/

static int AKI2C_RxData(char *rxData, int length)
{
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};

	if (i2c_transfer(this_client->adapter, msgs, 2) < 0) {
		ADBG(KERN_ERR "AKI2C_RxData: transfer error\n");
		return -EIO;
	} else
		return 0;
}

static int AKI2C_TxData(char *txData, int length)
{
	struct i2c_msg msg[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	if (i2c_transfer(this_client->adapter, msg, 1) < 0) {
		ADBG(KERN_ERR "AKI2C_TxData: transfer error\n");
		return -EIO;
	} else
		return 0;
}

static int AKECS_Init(void)
{
	return 0;
}

static void AKECS_Reset(void)
{
    struct ecom_platform_data* ecom_pdata;
    ecom_pdata = this_client->dev.platform_data;

    gpio_set_value(ecom_pdata->pin_rst, 0);
    udelay(200);
    gpio_set_value(ecom_pdata->pin_rst, 1);

	akecs_power_mode = 1;
}

#define RECOVER_EEPROM_COUNT 7

static int AKECS_WriteEEPROM(void)
{
	/* struct akm8973_data *data = i2c_get_clientdata(this_client); */
	char buffer[2];
	char aRecoverData[RECOVER_EEPROM_COUNT+1] = {0x26, 0x77, 0x66, 0xC7, 0x06, 0x06, 0x07};
	/* char i2cData[6]; */
	char data2;
	int ret, i;
    msleep(100);

		/* Set to EEPROM read mode */
		buffer[0] = AKECS_REG_MS1;
		buffer[1] = AK8973_MS1_READ_EEPROM;

		ret = AKI2C_TxData(buffer, 2);
		if (ret < 0)
			return ret;
		msleep(1);

		/* Set to EEPROM read mode */
		buffer[0] = AK8973_EEP_ETST;
		/* Read data */
		ret = AKI2C_RxData(buffer, 1);
		if (ret < 0)
			return ret;
		else
			ADBG("%s\n", buffer);
		msleep(1);

		data2 = buffer[0];
		 if ( data2 == 0xC7)
	  	{
			// Set to PowerDown mode
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AKECS_MODE_POWERDOWN;

			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);
				return ret;
		   	// do nothing...
		}else {

			// Set to PowerDown mode
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AKECS_MODE_POWERDOWN;

			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);

			/* Set to EEPROM read mode */
			buffer[0] = AKECS_REG_MS1;
			buffer[1] = AK8973_MS1_WRITE_EEPROM;

			ret = AKI2C_TxData(buffer, 2);
			if (ret < 0)
				return ret;
			msleep(1);
			for (i=0; i < RECOVER_EEPROM_COUNT; i++)
			{
			  /* Set to EEPROM read mode */
				buffer[0] = AK8973_EEP_ETS+i;
				buffer[1] = aRecoverData[i];

				ret = AKI2C_TxData(buffer, 2);
			    ADBG("[%x]==>  %x \n", AK8973_EEP_ETS+i, aRecoverData[i] );
			  msleep(1);
			}
	 	 }

		// Set to PowerDown mode
 		 buffer[0] = AKECS_REG_MS1;
 		 buffer[1] = AKECS_MODE_POWERDOWN;

 		 ret = AKI2C_TxData(buffer, 2);
 		 if (ret < 0)
 			 return ret;

}

static int AKECS_StartMeasure(void)
{
	char buffer[2];

	struct ecom_platform_data* ecom_pdata;
	ecom_pdata = this_client->dev.platform_data;

	if(powerDownOrOff){
		/* Set measure mode */
		buffer[0] = AKECS_REG_MS1;
		buffer[1] = AKECS_MODE_MEASURE;
	}else{
		if(akm8973_set_vreg_check ==0){
			ecom_pdata->power(1);
			akm8973_set_vreg_check=1;
			AKECS_Reset();
		}

		buffer[0] = AKECS_REG_MS1;
		buffer[1] = AKECS_MODE_MEASURE;
	}

	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_PowerDown(void)
{
	char buffer[2];
	int ret;

	/* Set powerdown mode */
	buffer[0] = AKECS_REG_MS1;
	buffer[1] = AKECS_MODE_POWERDOWN;
	/* Set data */
	ret = AKI2C_TxData(buffer, 2);
	if (ret < 0)
		return ret;

	/* Dummy read for clearing INT pin */
	buffer[0] = AKECS_REG_TMPS;
	/* Read data */
	ret = AKI2C_RxData(buffer, 1);
	if (ret < 0)
		return ret;

	akecs_power_mode = 0;

	return ret;
}

//LGE_CHANGE_S diyu@lge.com.  To add for used current
static int AKECS_PowerOff(void)
{
	//char buffer[2];
	int ret;
	struct ecom_platform_data* ecom_pdata;
	ecom_pdata = this_client->dev.platform_data;

	ret = AKECS_PowerDown();
	ecom_pdata->power(0);
	akm8973_set_vreg_check=0;

	return 0;

}
//LGE_CHANGE_E diyu@lge.com


static int AKECS_StartE2PRead(void)
{
	char buffer[2];

	/* Set E2P mode */
	buffer[0] = AKECS_REG_MS1;
	buffer[1] = AKECS_MODE_E2P_READ;
	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_SetMode(char mode)
{
	int ret;

	switch (mode) {
	case AKECS_MODE_MEASURE:
		ret = AKECS_StartMeasure();
		break;
	case AKECS_MODE_E2P_READ:
		ret = AKECS_StartE2PRead();
		break;
	case AKECS_MODE_POWERDOWN:
		if(powerDownOrOff)
			ret = AKECS_PowerDown();
		else
			ret = AKECS_PowerOff();
		break;
	default:
		return -EINVAL;
	}

	/* wait at least 300us after changing mode */
	msleep(1);
	return ret;
}

static int AKECS_TransRBuff(char *rbuf, int size)
{

	if(size < RBUFF_SIZE + 1)
	  return -EINVAL;

	// read C0 - C4
	rbuf[0] = AKECS_REG_ST;
	return AKI2C_RxData(rbuf, RBUFF_SIZE + 1);

}

static void AKECS_Report_Value(short *rbuf)
{
	struct akm8973_data *data = i2c_get_clientdata(this_client);

	/* Report magnetic sensor information */
	if (atomic_read(&m_flag)) {
		input_report_abs(data->input_dev, ABS_RX, rbuf[0]);
		input_report_abs(data->input_dev, ABS_RY, rbuf[1]);
		input_report_abs(data->input_dev, ABS_RZ, rbuf[2]);
		input_report_abs(data->input_dev, ABS_RUDDER, rbuf[4]);
	}

	/* Report acceleration sensor information */
	if (atomic_read(&a_flag)) {
		input_report_abs(data->input_dev, ABS_X, rbuf[6]);
		input_report_abs(data->input_dev, ABS_Y, rbuf[7]);
		input_report_abs(data->input_dev, ABS_Z, rbuf[8]);
		input_report_abs(data->input_dev, ABS_WHEEL, rbuf[5]);
	}

	/* Report temperature information */
	if (atomic_read(&t_flag)) {
		input_report_abs(data->input_dev, ABS_THROTTLE, rbuf[3]);
	}

	if (atomic_read(&mv_flag)) {
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[9]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[10]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[11]);
	}

	input_sync(data->input_dev);
/* LGE_CHANGE_S [ey.cho@lge.com] 2010-02-22, for TestMode */
	pitch = rbuf[1];
	roll = rbuf[2];
/* LGE_CHANGE_E [ey.cho@lge.com] 2010-02-22*/
}

static int AKECS_GetOpenStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) != 0));
	return atomic_read(&open_flag);
}

static int AKECS_GetCloseStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) <= 0));
	return atomic_read(&open_flag);
}

static void AKECS_CloseDone(void)
{
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);
	atomic_set(&p_flag, 1);
}

static int akm_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int akm_aot_release(struct inode *inode, struct file *file)
{
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
akm_aot_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
	case ECS_IOCTL_APP_SET_AFLAG:
	case ECS_IOCTL_APP_SET_TFLAG:
	case ECS_IOCTL_APP_SET_MVFLAG:
	case ECS_IOCTL_APP_SET_PFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
		atomic_set(&m_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MFLAG:
		flag = atomic_read(&m_flag);
		break;
	case ECS_IOCTL_APP_SET_AFLAG:
		atomic_set(&a_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_AFLAG:
		flag = atomic_read(&a_flag);
		break;
	case ECS_IOCTL_APP_SET_TFLAG:
		atomic_set(&t_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_TFLAG:
		flag = atomic_read(&t_flag);
		break;
	case ECS_IOCTL_APP_SET_PFLAG:
		atomic_set(&p_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_PFLAG:
		flag = atomic_read(&p_flag);
		break;
	case ECS_IOCTL_APP_SET_MVFLAG:
		atomic_set(&mv_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = atomic_read(&mv_flag);
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = akmd_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
		case ECS_IOCTL_APP_SET_MFLAG:
		case ECS_IOCTL_APP_SET_AFLAG:
		case ECS_IOCTL_APP_SET_TFLAG:
		case ECS_IOCTL_APP_SET_MVFLAG:
		case ECS_IOCTL_APP_SET_PFLAG:
			if(!atomic_read(&mv_flag) &&
				!atomic_read(&p_flag) &&
				!atomic_read(&t_flag) &&
				!atomic_read(&a_flag) &&
				!atomic_read(&m_flag) )
			{
				AKECS_SetMode(AKECS_MODE_POWERDOWN);
			}
			else
			{
				if(unlikely(akecs_power_mode==0))
				{
					AKECS_Reset();
				}
			}
			break;
		default:
			break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MFLAG:
	case ECS_IOCTL_APP_GET_AFLAG:
	case ECS_IOCTL_APP_GET_TFLAG:
	case ECS_IOCTL_APP_GET_PFLAG:
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int akmd_open(struct inode *inode, struct file *file)
{
	if(first_start == 0) {
		AKECS_Reset();
		first_start++;
	}

	return nonseekable_open(inode, file);
}

static int akmd_release(struct inode *inode, struct file *file)
{
	AKECS_CloseDone();
	return 0;
}

static int
akmd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	void __user *argp = (void __user *)arg;

	char msg[RBUFF_SIZE + 1], rwbuf[16];
	int ret = -1, status;
	short mode, value[13], delay;
#if 1	/* LGE_CHANGE [hyesung.shin@lge.com] on 2010-1-21, for <Sensor driver structure> */
	char accel_dev_path[30];
	int fdata_sign[7];
	s16 *alayout;
	s16 *hlayout;

	s16 a_layout[18] = {0, };
	s16 h_layout[18] = {0, };
	s16 copycount;
	struct ecom_platform_data* ecom_pdata;

	ecom_pdata = this_client->dev.platform_data;
#endif

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_MODE:
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_INIT:
		ret = AKECS_Init();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_RESET:
		AKECS_Reset();
		break;
	case ECS_IOCTL_READ:
		if (rwbuf[0] < 1)
			return -EINVAL;
		ret = AKI2C_RxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_WRITE:
		if (rwbuf[0] < 2)
			return -EINVAL;
		ret = AKI2C_TxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_MODE:
		ret = AKECS_SetMode((char)mode);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GETDATA:
		ret = AKECS_TransRBuff(msg, RBUFF_SIZE+1);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_YPR:
		AKECS_Report_Value(value);
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
		status = AKECS_GetOpenStatus();
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
		status = AKECS_GetCloseStatus();
		break;
	case ECS_IOCTL_GET_DELAY:
		delay = akmd_delay;
		break;
/* LGE_CHANGE_S [hyesung.shin@lge.com] on 2010-1-21, for <Sensor driver structure> [start] */
	case ECS_IOCTL_GET_ACCEL_PATH:
		break;
	case ECS_IOCTL_ACCEL_INIT:
		break;
	case ECS_IOCTL_GET_ALAYOUT_INIO:
		break;
	case ECS_IOCTL_GET_HLAYOUT_INIO:
		break;
/* LGE_CHANGE_E [hyesung.shin@lge.com] on 2010-1-21, for <Sensor driver structure> [end] */

	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if (copy_to_user(argp, &rwbuf, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		if (copy_to_user(argp, &msg, sizeof(msg)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_DELAY:
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;

/* LGE_CHANGE_S [hyesung.shin@lge.com] on 2010-1-21, for <Sensor driver structure> [start] */
	case ECS_IOCTL_GET_ACCEL_PATH:
		sprintf(accel_dev_path, "/dev/%s", ecom_pdata->accelerator_name);
		if (copy_to_user(argp, accel_dev_path, sizeof(accel_dev_path)))
			return -EFAULT;
		break;
	case ECS_IOCTL_ACCEL_INIT:
		fdata_sign[0] = ecom_pdata->fdata_sign_x;
		fdata_sign[1] = ecom_pdata->fdata_sign_y;
		fdata_sign[2] = ecom_pdata->fdata_sign_z;
		fdata_sign[3] = ecom_pdata->fdata_order0;
		fdata_sign[4] = ecom_pdata->fdata_order1;
		fdata_sign[5] = ecom_pdata->fdata_order2;
		fdata_sign[6] = ecom_pdata->sensitivity1g;

		if (copy_to_user(argp, fdata_sign, sizeof(fdata_sign)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_ALAYOUT_INIO:
		alayout = ecom_pdata->a_layout;
		for(copycount=0; copycount<18;copycount++)
		{
			a_layout[copycount] = alayout[copycount];
		}
		if (copy_to_user(argp, a_layout, sizeof(a_layout)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_HLAYOUT_INIO:
		hlayout = ecom_pdata->h_layout;
		for(copycount=0; copycount<18;copycount++)
		{
			h_layout[copycount] = hlayout[copycount];
		}
		if (copy_to_user(argp, h_layout, sizeof(h_layout)))
			return -EFAULT;
		break;
/* LGE_CHANGE_E [hyesung.shin@lge.com] on 2010-1-21, for <Sensor driver structure> [end] */

	default:
		break;
	}
	return 0;
}


#if defined(CONFIG_PM)
static int akm8973_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct ecom_platform_data* ecom_pdata;
	ecom_pdata = this_client->dev.platform_data;

	ADBG(KERN_INFO "%s\n", __FUNCTION__);

	atomic_set(&suspend_flag, 1);
	if (atomic_read(&open_flag) == 2)
		AKECS_SetMode(AKECS_MODE_POWERDOWN);

	atomic_set(&reserve_open_flag, atomic_read(&open_flag));
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	disable_irq(this_client->irq); //for testing

	ecom_pdata->power(0);
	akm8973_set_vreg_check=0;
	return 0;

}

static int akm8973_resume(struct i2c_client *client)
{
	struct ecom_platform_data* ecom_pdata;
	ecom_pdata = this_client->dev.platform_data;

	ADBG(KERN_INFO "%s\n", __FUNCTION__);

	ecom_pdata->power(1);
	akm8973_set_vreg_check=1;

	AKECS_Reset();

	atomic_set(&suspend_flag, 0);
	atomic_set(&open_flag, atomic_read(&reserve_open_flag));
	wake_up(&open_wq);
	return 0;
}
#endif 

static int akm8973_init_client(struct i2c_client *client)
{
	init_waitqueue_head(&open_wq);

	/* As default, report all information */
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);
	atomic_set(&p_flag, 1);

	return 0;

}

static struct file_operations akmd_fops = {
	.owner = THIS_MODULE,
	.open = akmd_open,
	.release = akmd_release,
	.ioctl = akmd_ioctl,
};

static struct file_operations akm_aot_fops = {
	.owner = THIS_MODULE,
	.open = akm_aot_open,
	.release = akm_aot_release,
	.ioctl = akm_aot_ioctl,
};

static struct miscdevice akm_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8973_aot",
	.fops = &akm_aot_fops,
};

static struct miscdevice akmd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8973_daemon",
	.fops = &akmd_fops,
};

static int check_result_value = -1; /*0: Fail,  1: Pass,  -1 : No result */
static ssize_t akm_checkresult_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ADBG("%s\n", __FUNCTION__);
	return sprintf(buf, "%d\n", check_result_value);
}

static ssize_t akm_checkresult_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int value;
	sscanf(buf, "%d", &value);
	ADBG("%s\n", __FUNCTION__);

	check_result_value = value;
	return size;
}

static DEVICE_ATTR(checkresult, S_IRUGO | S_IWUSR, akm_checkresult_show, akm_checkresult_store);

static int check_opmode_value = 0; /*0: Normal, 1: Factory Mode*/
static ssize_t akm_checkopmode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ADBG("%s\n", __FUNCTION__);
	//check_opmode_value =1;
	return sprintf(buf, "%d\n", check_opmode_value);
}

static ssize_t akm_checkopmode_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int value;
	sscanf(buf, "%d", &value);
	ADBG("%s\n", __FUNCTION__);

	check_opmode_value = value;
	if(check_opmode_value ==1)
		powerDownOrOff = 1;
	else if(check_opmode_value ==0)
		powerDownOrOff = 0;

	return size;
}

static DEVICE_ATTR(checkopmode, S_IRUGO | S_IWUSR, akm_checkopmode_show, akm_checkopmode_store);

int akm8973_probe(struct i2c_client *client, const struct i2c_device_id * devid)
{
	struct akm8973_data *akm;
	int err;

	struct ecom_platform_data* ecom_pdata;

	ADBG(KERN_INFO "%s\n", __FUNCTION__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	akm = kzalloc(sizeof(struct akm8973_data), GFP_KERNEL);
	if (!akm) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

//	INIT_WORK(&akm->work, akm_work_func);
	i2c_set_clientdata(client, akm);
	akm8973_init_client(client);
	this_client = client;

	ecom_pdata = this_client->dev.platform_data;

	ecom_pdata->power(1);
	akm8973_set_vreg_check=1;

	akm->input_dev = input_allocate_device();

	if (!akm->input_dev) {
		err = -ENOMEM;
		ADBG(KERN_ERR
		       "akm8973_probe: Failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, akm->input_dev->evbit);

	/* yaw */
	input_set_abs_params(akm->input_dev, ABS_RX, 0, 23040, 0, 0);
	/* pitch */
	input_set_abs_params(akm->input_dev, ABS_RY, -11520, 11520, 0, 0);
	/* roll */
	input_set_abs_params(akm->input_dev, ABS_RZ, -5760, 5760, 0, 0);
	/* x-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_X, -5760, 5760, 0, 0);
	/* y-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Y, -5760, 5760, 0, 0);
	/* z-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Z, -5760, 5760, 0, 0);
	/* temparature */
	input_set_abs_params(akm->input_dev, ABS_THROTTLE, -30, 85, 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(akm->input_dev, ABS_RUDDER, 0, 3, 0, 0);
	/* status of acceleration sensor */
	input_set_abs_params(akm->input_dev, ABS_WHEEL, 0, 3, 0, 0);
	/* x-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0X, -2048, 2032, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0Y, -2048, 2032, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_BRAKE, -2048, 2032, 0, 0);

	akm->input_dev->name = "compass";

	err = input_register_device(akm->input_dev);

	if (err) {
		ADBG(KERN_ERR
		       "akm8973_probe: Unable to register input device: %s\n",
		       akm->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&akmd_device);
	if (err) {
		ADBG(KERN_ERR "akm8973_probe: akmd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&akm_aot_device);
	if (err) {
		ADBG(KERN_ERR
		       "akm8973_probe: akm_aot_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = device_create_file(&client->dev, &dev_attr_checkopmode);
	if (err) {
		ADBG( "check opmode: Fail\n");
		return err;
	}
	err = device_create_file(&client->dev, &dev_attr_checkresult);
		if (err) {
			ADBG( "check opmode: Fail\n");
			return err;
		}

	err = device_create_file(&client->dev, &dev_attr_ms1);
/* LGE_CHANGE_S [ey.cho@lge.com] 2010-02-22, for TestMode */
	err = device_create_file(&client->dev, &dev_attr_pitch);
	err = device_create_file(&client->dev, &dev_attr_roll);
/* LGE_CHANGE_E [ey.cho@lge.com] 2010-02-22*/

	return 0;

exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(akm->input_dev);
exit_input_dev_alloc_failed:
	kfree(akm);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int akm8973_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info)
{
	strlcpy(info->type, "akm8973", I2C_NAME_SIZE);
	return 0;
}

static int akm8973_remove(struct i2c_client *client)
{
	struct akm8973_data *akm = i2c_get_clientdata(client);
	input_unregister_device(akm->input_dev);
	/* i2c_detach_client(client); */
	kfree(akm);
	return 0;
}

static const struct i2c_device_id akm8973_id[] = {
	{ "akm8973", 0 },
	{ }
};

static struct i2c_driver akm8973_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = akm8973_probe,
	.remove = akm8973_remove,
#if defined(CONFIG_PM)
	.suspend	= akm8973_suspend,
	.resume		= akm8973_resume,
#endif
	.id_table = akm8973_id,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "akm8973",
		   },
	.detect = akm8973_detect,
	.address_data = &addr_data,
};

static int __init akm8973_init(void)
{
	return i2c_add_driver(&akm8973_driver);
}

static void __exit akm8973_exit(void)
{
	i2c_del_driver(&akm8973_driver);
}

module_init(akm8973_init);
module_exit(akm8973_exit);

MODULE_AUTHOR("Hou-Kun Chen <hk_chen@htc.com>");
MODULE_DESCRIPTION("AKM8973 compass driver");
MODULE_LICENSE("GPL");

