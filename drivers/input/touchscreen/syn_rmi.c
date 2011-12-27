/* drivers/input/keyboard/synaptics_i2c_rmi.c
 *
 * Copyright  (C) 2007  Google,Inc.
 *
 * This software is licensed  under the terms of the GNU General  Public
 * License version 2, as  published by the Free  Software Foundation, and
 * may becopied,  distributed,  and modified  under  those terms.
 *
 * This  program  is distributed in  the hope  that it  will be  useful,
 *  but WITHOUT ANY WARRANTY; without  even  the  implied  warranty of
 *  MERCHANTABILITY  or  FITNESS FOR  A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

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
#include <mach/vreg.h>
#include "syn_reflash.h"

struct vreg {
	const char *name;
	unsigned id;
	int status;
	unsigned refcnt;
};


#define DEVICE_STATUS       0x13
#define INT_STATUS			0x14

#define SYN_INT_REG			0x26
#define SYN_CONTROL	    	0x25
#define REPO_MODE_2D		0x27
#define MAX_X_LOW			0x2D
#define MAX_X_HIGH			0x2E
#define MAX_Y_LOW			0x2F
#define MAX_Y_HIGH			0x30

#define QUERY_BASE			0xE3
#define SYNAPTICS_MELTING	0xF0

#define SYN_INT_FLASH		1<<0
#define SYN_INT_STATUS 	    1<<1
#define SYN_INT_ABS0 		1<<2

#define SYN_CONT_SLEEP 	    1<<0
#define SYN_CONT_NOSLEEP	1<<2

#define MELTING_NO	    	0
#define MELTING_MELT		1<<0
#define MELTING_AUTO		1<<1

#define FINGER_MAX          2
#define START_ADDR          0x13
#define PID_STR_NUM	        11
#define CMD_BLOCK_NUM		41
/************************************/
int reflash_flag = 0;
int fw_rev = 0;
EXPORT_SYMBOL(fw_rev);
module_param(fw_rev, int, S_IWUSR | S_IRUGO);

uint16_t max_x, max_y;
extern int is_need_update;
extern int is_fw_reflash;

int product_type;

/**************** structure ***********************************/
struct synaptics_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int use_irq;
	bool has_relative_report;
	struct hrtimer timer;
	struct work_struct  work;
	struct work_struct  work_for_reflash;
	struct work_struct  work_for_on;
	uint16_t max[2];
	uint32_t flags;
	int reported_finger_count;
	int8_t sensitivity_adjust;
	int (*power)(unsigned char on);
	struct early_suspend early_suspend;
	int fw_revision;
};

typedef struct
{
	unsigned char dev_status;                   /* 0x13 */
	unsigned char int_status;			        /* 0x14 */
	unsigned char finger_state;				    /* 0x15 */

	/* Finger 0 */
	unsigned char X_high0;                      /* 0x16 */
	unsigned char Y_high0;                      /* 0x17 */
	unsigned char XY_low0;	                    /* 0x18 */
	unsigned char XY_width0;	        		/* 0x19 */
	unsigned char Z_finger0;    				/* 0x1A */
	/* Finger 1 */
	unsigned char X_high1;                      /* 0x1B */
	unsigned char Y_high1;                    	/* 0x1C */
	unsigned char XY_low1;	                    /* 0x1D */
	unsigned char XY_width1;			        /* 0x1E */
	unsigned char Z_finger1;			    	/* 0x1F */
 } ts_sensor_data;

typedef struct 									/* synaptics 3000 */
{
	unsigned char device_command;				/* 0x70 */
	unsigned char command_2d;					/* 0x71 */
	unsigned char bootloader_id0;				/* 0x72 */
	unsigned char bootloader_id1;				/* 0x73 */
	unsigned char flash_properties;				/* 0x74 */
	unsigned char block_size0;					/* 0x75 */
	unsigned char block_size1;					/* 0x76 */
	unsigned char firmware_block_cnt0;			/* 0x77 */
	unsigned char firmware_block_cnt1;			/* 0x78 */
	unsigned char config_block_cnt0;			/* 0x79 */
	unsigned char config_block_cnt1;			/* 0x7A */
	unsigned char manufact_id_query;			/* 0x7B */
	unsigned char product_properties_query;		/* 0x7C */
	unsigned char customer_family_query;		/* 0x7D */
	unsigned char firmware_revision_query;		/* 0x7E */
	unsigned char device_serialization_query0;	/* 0x7F */
	unsigned char device_serialization_query1;	/* 0x80 */
	unsigned char device_serialization_query2;	/* 0x81 */
	unsigned char device_serialization_query3;	/* 0x82 */
	unsigned char device_serialization_query4;	/* 0x83 */
	unsigned char device_serialization_query5;	/* 0x84 */
	unsigned char device_serialization_query6;	/* 0x85 */
	unsigned char product_id_query0;			/* 0x86 */
	unsigned char product_id_query1;			/* 0x87 */
	unsigned char product_id_query2;			/* 0x88 */
	unsigned char product_id_query3;			/* 0x89 */
	unsigned char product_id_query4;			/* 0x8A */
	unsigned char product_id_query5;			/* 0x8B */
	unsigned char product_id_query6;			/* 0x8C */
	unsigned char product_id_query7;			/* 0x8D */
	unsigned char product_id_query8;			/* 0x8E */
	unsigned char product_id_query9;			/* 0x8F */
	unsigned char per_device_query;				/* 0x90 */
	unsigned char reporting_mode_2d;			/* 0x91 */
	unsigned char number_x_electrode_2d;		/* 0x92 */
	unsigned char number_y_electrode_2d;		/* 0x93 */
	unsigned char maximum_electorde_2d;			/* 0x94 */
	unsigned char absolute_query_2d;			/* 0x95 */
}ts_sensor_command;

typedef struct {
	unsigned char finger_count;
	int X_position[FINGER_MAX];
	int Y_position[FINGER_MAX];
} ts_finger_data;
/***********************************************************/

/**************** STATIC VARIABLE *********************************/
static struct workqueue_struct *synaptics_wq;
static struct workqueue_struct *synaptics_fwdl_wq;
static struct workqueue_struct *synaptics_on_wq;
static ts_sensor_data ts_reg_data={0};
static ts_finger_data curr_ts_data;
static ts_sensor_command ts_cmd_reg_data={0};
static int touch_pressed = 0;
static int multi = 0;
static int irq_flag = 1, melt_mode = 1, melt_count =0, tmp_oldx = 0, tmp_oldy = 0;
/* static int tapcount = 0; */
static int first_touch = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ts_early_suspend(struct early_suspend *h);
static void ts_late_resume(struct early_suspend *h);
#endif
static int synaptics_ts_resume(struct i2c_client *client);

/****************************************************************/

static __inline void ts_event_touch(int pressed, int width, int x, int y, struct synaptics_ts_data *ts)
{
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, pressed);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, width);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(ts->input_dev);
}

static __inline void esd_recovery(struct synaptics_ts_data *ts)
{
	int ret;

    if (!ts->use_irq)
		hrtimer_cancel(&ts->timer);

	msleep(20);
    ret = i2c_smbus_write_byte_data(ts->client, SYN_CONTROL, SYN_CONT_SLEEP); /* sleep */
	if (ret < 0) {
		printk("[TOUCH] i2c_smbus_write_byte_data failed\n");
	}

	if (ts->power) {
		ret = ts->power(0);
		if (ret < 0)
			printk("[TOUCH] power off failed\n");

		ret = ts->power(1);
		if (ret < 0)
			printk("[TOUCH] power on failed\n");
	}
}

static void ts_work_func(struct work_struct *work)
{
	int int_mode, device_status;
	int width0, width1;
	int tmp0_x=0, tmp0_y=0, tmp1_x=0, tmp1_y=0;
	int finger0_status=0, finger1_status=0;
	int finger_status;
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work);

	int_mode = i2c_smbus_read_byte_data(ts->client, INT_STATUS);
	device_status = i2c_smbus_read_byte_data(ts->client, DEVICE_STATUS) & 0x03;

	if (unlikely(int_mode < 0 || device_status == 0x03)) {
		printk("[TOUCH] i2c failed ==> recovery touch!!");
		esd_recovery(ts);
		if (ts->use_irq && irq_flag == 0) {
			irq_flag++;
			enable_irq(ts->client->irq);
		}
		i2c_smbus_read_byte_data(ts->client, INT_STATUS);
		melt_mode = 1;
		return;
	}

	i2c_smbus_read_i2c_block_data(ts->client, START_ADDR, sizeof(ts_reg_data), (unsigned char*)&ts_reg_data);
	finger_status = ts_reg_data.finger_state;
	finger0_status = finger_status & 0x03 ;
	finger1_status = (finger_status & 0x0C) >> 2;

	tmp0_x = (int)((ts_reg_data.X_high0 << 4) + (ts_reg_data.XY_low0 & 0xF));
	tmp0_y = (int)((ts_reg_data.Y_high0 << 4) + (ts_reg_data.XY_low0 >> 4));

	curr_ts_data.X_position[0] = tmp0_x;
	curr_ts_data.Y_position[0] = tmp0_y;

	width0 = ts_reg_data.XY_width0;
	if (((width0 & 0xF0) >> 4)>(width0 & 0xF))
		width0 = (width0 & 0xF0) >> 4;
	else
		width0 = (width0 & 0xF);

	tmp1_x = (int)((ts_reg_data.X_high1 << 4) + (ts_reg_data.XY_low1 & 0xF));
	tmp1_y = (int)((ts_reg_data.Y_high1 << 4) + (ts_reg_data.XY_low1 >> 4));

	curr_ts_data.X_position[1] = tmp1_x;
	curr_ts_data.Y_position[1] = tmp1_y;

	width1 = ts_reg_data.XY_width1;
	if (((width1 & 0xF0) >> 4)>(width1 & 0xF))
		width1 = (width1 & 0xF0) >> 4;
	else
		width1 = (width1 & 0xF);
	if (finger0_status && finger1_status) {    /* 1 & 2 */
		touch_pressed = 1;
		multi = 1, melt_count = 2;
		/* tapcount = 0; */
		ts_event_touch(1, width0, curr_ts_data.X_position[0], curr_ts_data.Y_position[0], ts);
		ts_event_touch(1, width1, curr_ts_data.X_position[1], curr_ts_data.Y_position[1], ts);
	} else if (finger0_status) {    /* 1 only */
		first_touch++;
		touch_pressed = 1;
		if (melt_count == 0)
			melt_count = 1;
		if (first_touch == 1) {
			tmp_oldx = curr_ts_data.X_position[0];
			tmp_oldy = curr_ts_data.Y_position[0];
		}
		ts_event_touch(1, width0, curr_ts_data.X_position[0], curr_ts_data.Y_position[0], ts);
		if (multi == 1) {    /* 1 & 2 ==> release 2 */
			multi = 0;
			ts_event_touch(0, width1, curr_ts_data.X_position[1], curr_ts_data.Y_position[1], ts);
		}
	} else if (finger1_status) {    /* 2 only */
		ts_event_touch(1, width1, curr_ts_data.X_position[1], curr_ts_data.Y_position[1], ts);
		if (multi == 1) {    /* 1 & 2 ==> release 1 */
			multi = 0;
			ts_event_touch(0, width0, curr_ts_data.X_position[0], curr_ts_data.Y_position[0], ts);
		}
	} else if (multi == 1 && touch_pressed == 1) {    /* release 1 & 2 */
		multi = 0;
		touch_pressed =0;
		first_touch = 0;
		/* tapcount = 0; */
		ts_event_touch(0, width0, curr_ts_data.X_position[0], curr_ts_data.Y_position[0], ts);
		ts_event_touch(0, width1, curr_ts_data.X_position[1], curr_ts_data.Y_position[1], ts);
	} else if (touch_pressed == 1) {    /* 1 only ==> release 1 */
		touch_pressed = 0;
		first_touch = 0;
		ts_event_touch(0, width0, curr_ts_data.X_position[0], curr_ts_data.Y_position[0], ts);
		if (melt_mode == 1 && melt_count == 1) {    /* melt_mode */
			if ((abs(tmp0_x - tmp_oldx) > 140) || (abs(tmp0_y - tmp_oldy) > 140)){  /* long drag */
				i2c_smbus_write_byte_data(ts->client, SYNAPTICS_MELTING, MELTING_NO);
				printk("[TOUCH] Long Drag detected!!==> set no melt\n");
				melt_mode = 0;
			}
			/*  else if (tapcount++ > 2) {    /\* tapping!!!! *\/ */
			/* 	i2c_smbus_write_byte_data(ts->client, SYNAPTICS_MELTING, MELTING_NO); */
			/* 	printk("[TOUCH] TapCount detected!!==> set no melt\n"); */
			/* 	tapcount = 0; */
			/* 	melt_mode = 0; */
			/* } */
		}
		melt_count = 0;
	}
	input_sync(ts->input_dev);

	if (ts->use_irq && irq_flag == 0) {
		irq_flag++;
		enable_irq(ts->client->irq);
	}
}

static void ts_reflash_work_func(struct work_struct *work)
{
	int ret, int_mode;
	u8 buf[4];
	enum firmware_type type;
	struct synaptics_ts_data *ts = NULL;
	ts = container_of(work, struct synaptics_ts_data, work_for_reflash);

	printk("[TOUCH] start F/W reflash for synaptics touch IC!!\n");

	type = product_type;

	if (ts->use_irq && irq_flag == 1) {
		irq_flag--;
		disable_irq_nosync(ts->client->irq);
	}
	if(firmware_reflash(ts->client, ts->fw_revision, type) == 0) {
		printk("[TOUCH] %s: DoReflash succeed!\n", __func__);

		ret = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.firmware_revision_query);
		if (ret < 0) {
			printk("[TOUCH] i2c_smbus_read_byte_data failed\n");
		} else {
			fw_rev = ret;
			printk("[TOUCH] %s: new firmware version 0x%x\n", __func__, ret);
		}

		ret = i2c_smbus_read_i2c_block_data(ts->client, MAX_X_LOW, 4, buf);
		if (ret < 0) {
			printk("[TOUCH] %s max x/y read failed\n", __func__);
		}

		max_x = buf[0] | (buf[1] << 8);
		max_y = buf[2] | (buf[3] << 8);

		printk("[TOUCH] %s: max_x = 0x%x, max_y = 0x%x\n",__func__, max_x, max_y);

		if ( ts->max[0] != max_x || ts->max[1] != max_y) {
			printk("[TOUCH] Reinitialize Touch input device\n");
			input_unregister_device(ts->input_dev);
			ts->input_dev = input_allocate_device();
			if (ts->input_dev == NULL) {
				ret = -ENOMEM;
				printk("[TOUCH] synaptics_ts_probe: Failed to allocate input device\n");
				kfree(ts);
				return;
			}
			ts->input_dev->name = "synaptics-rmi-ts";
			set_bit(EV_SYN, ts->input_dev->evbit);
			set_bit(EV_ABS, ts->input_dev->evbit);
			set_bit(EV_KEY, ts->input_dev->evbit);
			ts->max[0] = max_x;
			ts->max[1] = max_y;
			input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x+1, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y+1, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 15, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
			ret = input_register_device(ts->input_dev);
			if (ret) {
				printk("[TOUCH] synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
				input_free_device(ts->input_dev);
			}
		}
	}

	/* enable irq */
	if (ts->use_irq && irq_flag == 0) {
		irq_flag++;
		enable_irq(ts->client->irq);
	}

	int_mode = i2c_smbus_read_byte_data(ts->client, INT_STATUS);
	if (unlikely(int_mode < 0)) {
		printk("[TOUCH] i2c failed ==> recovery touch!!");
		esd_recovery(ts);
		i2c_smbus_read_byte_data(ts->client, INT_STATUS);
	}

	melt_mode = 1;

	ts->fw_revision = fw_rev;
	is_fw_reflash = 0;
	is_need_update = 0;

	return;
}

static void ts_on_work_func(struct work_struct *work)
{
	struct synaptics_ts_data *ts = container_of(work, struct synaptics_ts_data, work_for_on);
	synaptics_ts_resume(ts->client);
	return;
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

	if (ts->use_irq && irq_flag == 1) {
		irq_flag--;
		disable_irq_nosync(ts->client->irq);
	}
	queue_work(synaptics_wq, &ts->work);

	return IRQ_HANDLED;
}

static void get_device_inform(void)
{
	int i;
	unsigned char reg_block_num[CMD_BLOCK_NUM]={0x00};

	for(i=0;i<CMD_BLOCK_NUM;i++)
		reg_block_num[i]= i+0x70;

	memcpy(&ts_cmd_reg_data, &reg_block_num[0], CMD_BLOCK_NUM);
	return;
}

static ssize_t syn_reflash_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int len;

	len = snprintf(buf, PAGE_SIZE, "\n Synaptics Touch Status \n");
	len += snprintf(buf + len, PAGE_SIZE - len, "====================================\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "firmware revision is %d\n", fw_rev);
	return len;
}

static ssize_t syn_reflash_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	struct vreg *vreg_touch = vreg_get(0, "mmc");
	int cmd, ret, version;

	if (sscanf(buf, "%d, %d", &cmd, &version) != 2)
		return -EINVAL;
	switch (cmd) {
	case 1:
		ts->fw_revision = version;

		if (ts->power) {
			if(!vreg_touch->refcnt) {
				ret = ts->power(1);
				if (ret < 0)
					printk("[TOUCH] power on failed\n");
			}
		}

		is_need_update = 1;
		queue_work(synaptics_fwdl_wq, &ts->work_for_reflash);
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static ssize_t syn_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct vreg *vreg_touch = vreg_get(0, "mmc");
	int len;

	len = snprintf(buf, PAGE_SIZE, "\n Synaptics Touch Melt Mode \n");
	len += snprintf(buf + len, PAGE_SIZE - len, "Melt Mode is %d\n", melt_mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "Ref Count is %d\n", vreg_touch->refcnt);
	len += snprintf(buf + len, PAGE_SIZE - len, "irq flag  is %d\n", irq_flag);
	if (product_type == syn_1818)
		len += snprintf(buf + len, PAGE_SIZE - len, "product type  is TM1818\n");
	else
		len += snprintf(buf + len, PAGE_SIZE - len, "product type  is TM1912\n");
	return len;
}

static ssize_t syn_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct synaptics_ts_data *ts = dev_get_drvdata(dev);
	struct vreg *vreg_touch = vreg_get(0, "mmc");
	int cmd, ret;

	if (sscanf(buf, "%d", &cmd) != 1)
		return -EINVAL;
	switch (cmd) {
	case 1:
		if (ts->power) {
			if(!vreg_touch->refcnt) {
				ret = ts->power(1);
				if (ret < 0)
					printk("[TOUCH] power on failed\n");
			}
		}
		break;
	case 2:
		if (ts->power) {
			if(vreg_touch->refcnt) {
				ret = ts->power(0);
				if (ret < 0)
					printk("[TOUCH] Power off failed\n");
			}
		}
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static struct device_attribute syn_device_attrs[] = {
	__ATTR(reflash, S_IRUGO |S_IWUSR, syn_reflash_show, syn_reflash_store),
	__ATTR(status, S_IRUGO |S_IWUSR, syn_status_show, syn_status_store),
};

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
	struct synaptics_i2c_rmi_platform_data *pdata;
	unsigned long irqflags = 0;
	int product_id_query;
	char product_name[PID_STR_NUM];
	u8 buf[4];

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk("[TOUCH] synaptics_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;

		goto err_check_functionality_failed;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) {
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts->client = client;
	i2c_set_clientdata(client, ts);
	pdata = client->dev.platform_data;

	if (pdata) {
		ts->power = pdata->power;
		irqflags = pdata->irqflags;
	}

	if (ts->power) {
		ret = ts->power(1);
		if (ret < 0) {
			printk("[TOUCH] synaptics_ts_probe power on failed\n");
			goto err_power_failed;
		}
	}

	for(i=0;i <PID_STR_NUM;i++)
		product_name[i] = '\0';

	ret = i2c_smbus_read_byte_data(ts->client, QUERY_BASE);
	msleep(500);
	if (ret < 0) {
		printk("[TOUCH] i2c_smbus_read_byte_data failed\n");
		if (ts->power) {
			printk("[TOUCH] [Touch] POWER_DOWN\n");
			ret = ts->power(0);
			return ret;
		}
	}
	product_id_query= ret + 11;

	ret = i2c_smbus_read_i2c_block_data(ts->client, product_id_query, sizeof(product_name)-1, product_name);
	if (ret < 0) {
		printk("[TOUCH] synaptics_ts_probe : i2c_smbus_read_i2c_block_data failed: product_id_query\n");
	}

	if (strcmp(product_name, "TM1818-001") == 0) {
			product_type = syn_1818;
			printk("[TOUCH] Touch Product name is TM1818\n");
	} else if (strcmp(product_name, "TM1912-001") == 0) {
			product_type = syn_1912;
			printk("[TOUCH] Touch Product name is TM1912\n");
	}

	get_device_inform();

	INIT_WORK(&ts->work, ts_work_func);
	INIT_WORK(&ts->work_for_reflash, ts_reflash_work_func);
	INIT_WORK(&ts->work_for_on, ts_on_work_func);

	ret = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.customer_family_query);
	if (ret < 0) {
		printk("[TOUCH] i2c_smbus_read_byte_data failed\n");
	}

	ts->fw_revision = i2c_smbus_read_byte_data(ts->client, ts_cmd_reg_data.firmware_revision_query);
	if (ts->fw_revision < 0) {
		printk("[TOUCH] i2c_smbus_read_byte_data failed\n");
	}

	fw_rev = ts->fw_revision;
	printk("[TOUCH] Synaptics Touch Firmware Version : %x\n", fw_rev);

  	memset(&ts_reg_data, 0x0, sizeof(ts_sensor_data));
  	memset(&curr_ts_data, 0x0, sizeof(ts_finger_data));

	ret = i2c_smbus_read_i2c_block_data(ts->client, MAX_X_LOW, 4, buf);
	if (ret < 0) {
		printk("[TOUCH] %s max x/y read failed\n", __func__);
	}

	max_x = buf[0] | (buf[1] << 8);
	max_y = buf[2] | (buf[3] << 8);
	ts->max[0] = max_x;
	ts->max[1] = max_y;
	printk("%s touch max_x = 0x%x, max_y = 0x%x\n", __func__, max_x, max_y);

	if((max_x == 0x00) || (max_y == 0x00)) {
		printk("[TOUCH] F/W image is not normal status : need upgrade.\n");
		is_need_update = 1;
	}

	ret = i2c_smbus_read_i2c_block_data(ts->client, START_ADDR, sizeof(ts_reg_data), (unsigned char*)&ts_reg_data);
	if (ret < 0) {
		printk("[TOUCH] synaptics_ts_probe : i2c_smbus_read_i2c_block_data failed: START_ADDR\n");
	}

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		printk("[TOUCH] synaptics_ts_probe: Failed to allocate input device\n");

		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "synaptics-rmi-ts";
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, max_x + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, max_y + 1, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 15, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);

	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk("[TOUCH] synaptics_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}

	if (client->irq) {
		ret = request_irq(client->irq, synaptics_ts_irq_handler, irqflags, client->name, ts);

		if (ret == 0) {
			ts->use_irq = 1;
		} else {
			dev_err(&client->dev, "request_irq failed\n");
		}
	}
	if (!ts->use_irq) {
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = synaptics_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = ts_early_suspend;
	ts->early_suspend.resume = ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	if(synaptics_fwdl_wq)
		queue_work(synaptics_fwdl_wq, &ts->work_for_reflash);

	for (i = 0; i < ARRAY_SIZE(syn_device_attrs); i++) {
		if (device_create_file(&client->dev, &syn_device_attrs[i])) {
			printk("[TOUCH] Fail to make sysfs\n");
			goto err_input_register_device_failed;
		}
	}

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
	int i;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);

	for (i = 0; i < ARRAY_SIZE(syn_device_attrs); i++)
		device_remove_file(&client->dev, &syn_device_attrs[i]);

	return 0;
}

static int synaptics_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);

	cancel_work_sync(&ts->work_for_on);

	if(is_fw_reflash == 1) {
		printk("[TOUCH] synaptics_ts_suspend: during f/w image update - block suspend\n");
		reflash_flag = 1;
		return 0;
	}

	if (ts->use_irq) {
		if (irq_flag == 1) {
			irq_flag--;
			disable_irq_nosync(client->irq);
		}
	} else
		hrtimer_cancel(&ts->timer);

	i2c_smbus_write_byte_data(ts->client, SYN_CONTROL, SYN_CONT_SLEEP); /* sleep */

	if (ts->power) {
		ret = ts->power(0);
		if (ret < 0)
			printk("[TOUCH] synaptics_ts_resume power off failed\n");
	}

	return 0;
}

static int synaptics_ts_resume(struct i2c_client *client)
{
	int ret, int_mode = 0;
	struct synaptics_ts_data *ts = i2c_get_clientdata(client);
	struct vreg *vreg_touch = vreg_get(0, "mmc");
	/* tapcount = 0; */
	melt_count = 0;

	if(is_fw_reflash == 1 || reflash_flag ==1) {
		printk("[TOUCH] synaptics_ts_resume: during f/w image update - block resume\n");
		reflash_flag = 0;
		return 0;
	}

	if (ts->power) {
		if(!vreg_touch->refcnt) {
			ret = ts->power(1);
			if (ret < 0)
				printk("[TOUCH] power on failed\n");
		}
	}

    i2c_smbus_write_byte_data(ts->client, SYN_CONTROL, SYN_CONT_NOSLEEP); /* wake up */

	/*
	 * 2011-05-09, jinkyu.choi@lge.com
	 * update the touch f/w version for Folder JIG Test
	 */
	ret = i2c_smbus_read_byte_data(ts->client,
			ts_cmd_reg_data.firmware_revision_query);

	if (ret > 0) {
		fw_rev = ret;
		printk("%s touch f/w version is updated %d\n", __func__, fw_rev);
	} else {
		printk("%s touch f/w version is invalid %d\n", __func__, ret);
	}

	if (ts->use_irq) {
		if (irq_flag == 0) {
			irq_flag++;
			enable_irq(client->irq);
		}
	} else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);

	int_mode = i2c_smbus_read_byte_data(ts->client, INT_STATUS);
	if (unlikely(int_mode < 0)) {
		printk("[TOUCH] i2c failed ==> recovery touch!!");
		esd_recovery(ts);
		i2c_smbus_read_byte_data(ts->client, INT_STATUS);
	}
	melt_mode = 1;
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ts_early_suspend(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	synaptics_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void ts_late_resume(struct early_suspend *h)
{
	struct synaptics_ts_data *ts;
	ts = container_of(h, struct synaptics_ts_data, early_suspend);
	if (synaptics_on_wq)
		queue_work(synaptics_on_wq, &ts->work_for_on);
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
	synaptics_fwdl_wq = create_singlethread_workqueue("synaptics_fwdl_wq");
	synaptics_on_wq = create_singlethread_workqueue("synaptics_on_wq");
	if ((!synaptics_wq)||(!synaptics_fwdl_wq))
		return -ENOMEM;

	return i2c_add_driver(&synaptics_ts_driver);
}

static void __exit synaptics_ts_exit(void)
{
	i2c_del_driver(&synaptics_ts_driver);

	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
	if (synaptics_fwdl_wq)
		destroy_workqueue(synaptics_fwdl_wq);
	if (synaptics_on_wq)
		destroy_workqueue(synaptics_on_wq);
}

module_init(synaptics_ts_init);
module_exit(synaptics_ts_exit);

MODULE_DESCRIPTION("Synaptics Touchscreen Driver");
MODULE_LICENSE("GPL");
