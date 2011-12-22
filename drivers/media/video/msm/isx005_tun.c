/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Sony 3M ISX005 camera sensor driver
 * Auther: Lee Hyung Tae[hyungtae.lee@lge.com], 2010-04-09
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>

#include "isx005_tun.h"
#include "isx005_reg.h"

#define ISX005_INTERVAL_T2		8	/* 8ms */
#define ISX005_INTERVAL_T3		1	/* 0.5ms */
#define ISX005_INTERVAL_T4		2	/* 15ms */
#define ISX005_INTERVAL_T5		25	/* 200ms */

/*
* AF Total steps parameters
*/
#define ISX005_TOTAL_STEPS_NEAR_TO_FAR	30

/*  ISX005 Registers  */
#define REG_ISX005_INTSTS_ID			0x00F8	/* Interrupt status */
#define REG_ISX005_INTCLR_ID			0x00FC	/* Interrupt clear */

#define ISX005_OM_CHANGED				0x0001	/* Operating mode */
#define ISX005_CM_CHANGED				0x0002	/* Camera mode */

/* It is distinguish normal from macro focus */
static int prev_af_mode;
/* It is distinguish scene mode */
static int prev_scene_mode;

static int init_prev_mode;

struct isx005_work {
	struct work_struct work;
};
static struct isx005_work *isx005_sensorw;

static struct i2c_client *isx005_client;

struct isx005_ctrl {
	const struct msm_camera_sensor_info *sensordata;
};

static struct isx005_ctrl *isx005_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(isx005_wait_queue);

DEFINE_MUTEX(isx005_mutex);

struct platform_device *isx005_pdev;

int pclk_rate;
extern int mclk_rate;
static int always_on = 0;

static int32_t isx005_i2c_txdata(unsigned short saddr,
	unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if (i2c_transfer(isx005_client->adapter, msg, 1) < 0) {
		CDBG("isx005_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t isx005_i2c_write(unsigned short saddr,
	unsigned short waddr, unsigned short wdata, enum isx005_width width)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	switch (width) {
		case BYTE_LEN:	
			buf[0] = (waddr & 0xFF00) >> 8;
			buf[1] = (waddr & 0x00FF);
			buf[2] = wdata;
			rc = isx005_i2c_txdata(saddr, buf, 3);
			break;
			
		case WORD_LEN:
			buf[0] = (waddr & 0xFF00) >> 8;
			buf[1] = (waddr & 0x00FF);
			buf[2] = (wdata & 0xFF00) >> 8;
			buf[3] = (wdata & 0x00FF);
			rc = isx005_i2c_txdata(saddr, buf, 4);
			break;

		default:
			break;
	}

	if (rc < 0)
		printk(KERN_ERR "i2c_write failed, addr = 0x%x, val = 0x%x!\n", waddr, wdata);

	return rc;
}

static int32_t isx005_i2c_write_table(
	struct isx005_register_address_value_pair const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	for (i = 0; i < num_of_items_in_table; ++i) {
		rc = isx005_i2c_write(isx005_client->addr,
			reg_conf_tbl->register_address, reg_conf_tbl->register_value,
			reg_conf_tbl->register_length);
		if (rc < 0)
			break;

		reg_conf_tbl++;
	}

	return rc;
}

static int isx005_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	struct i2c_msg msgs[] = {
		{
			.addr   = saddr,
			.flags = 0,
			.len   = 2,
			.buf   = rxdata,
		},
		{
			.addr   = saddr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = rxdata,
		},
	};

	if (i2c_transfer(isx005_client->adapter, msgs, 2) < 0) {
		printk(KERN_ERR "isx005_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t isx005_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum isx005_width width)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	switch (width) {
		case BYTE_LEN:
			buf[0] = (raddr & 0xFF00) >> 8;
			buf[1] = (raddr & 0x00FF);
			rc = isx005_i2c_rxdata(saddr, buf, 2);
			if (rc < 0)
				return rc;
			*rdata = buf[0];
			break;
			
		case WORD_LEN:
			buf[0] = (raddr & 0xFF00) >> 8;
			buf[1] = (raddr & 0x00FF);
			rc = isx005_i2c_rxdata(saddr, buf, 2);
			if (rc < 0)
				return rc;
			*rdata = buf[0] << 8 | buf[1];
			break;

		default:
			break;
	}

	if (rc < 0)
		printk(KERN_ERR "isx005_i2c_read failed!\n");

	return rc;
}

static int isx005_reg_init(void)
{
	int rc = 0;
	int i;

	/* Configure sensor for Initial setting (PLL, Clock, etc) */
	for (i = 0; i < isx005_regs.init_reg_settings_size; ++i) {
		rc = isx005_i2c_write(isx005_client->addr,
			isx005_regs.init_reg_settings[i].register_address,
			isx005_regs.init_reg_settings[i].register_value,
			isx005_regs.init_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}
	
	return rc;
}

static int isx005_reg_tuning(void)
{
	int rc = 0;
	int i;
	
	/* Configure sensor for various tuning */
	for (i = 0; i < isx005_regs.tuning_reg_settings_size; ++i) {
		rc = isx005_i2c_write(isx005_client->addr,
			isx005_regs.tuning_reg_settings[i].register_address,
			isx005_regs.tuning_reg_settings[i].register_value,
			isx005_regs.tuning_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	return rc;
}

static int isx005_reg_preview(void)
{
	int rc = 0;
	int i;

	/* Configure sensor for Preview mode */
	for (i = 0; i < isx005_regs.prev_reg_settings_size; ++i) {
		rc = isx005_i2c_write(isx005_client->addr,
		  isx005_regs.prev_reg_settings[i].register_address,
		  isx005_regs.prev_reg_settings[i].register_value,
		  isx005_regs.prev_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	/* Checking the mode change status */
	/* BUG FIX : msm is changed preview mode but sensor is not change preview mode. */
	/*so there is case that vfe get capture image on preview mode */	
	/* eunyoung.shin@lge.com 2010.07.13*/
  for(i=0; i<300;i++)
  {
      unsigned short cm_changed_status = 0;
      rc = isx005_i2c_read(isx005_client->addr, 0x00F8, &cm_changed_status, BYTE_LEN);

      if(cm_changed_status & 0x0002)
      {
	       printk("[%s]:Sensor Preview Mode check : %d-> success \n", __func__, cm_changed_status);
        break;  
      }
      else 
       msleep(10);
   
  }


	return rc;
}

static int isx005_reg_snapshot(void)
{
	int rc = 0;
	int i;

	/* Configure sensor for Snapshot mode */
	for (i = 0; i < isx005_regs.snap_reg_settings_size; ++i) {
		rc = isx005_i2c_write(isx005_client->addr,
			isx005_regs.snap_reg_settings[i].register_address,
			isx005_regs.snap_reg_settings[i].register_value,
			isx005_regs.snap_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	/* Checking the mode change status */
	/* eunyoung.shin@lge.com 2010.07.13*/	
	 for(i=0; i<300;i++)
   {
   			printk("[%s]:Sensor Snapshot Mode Start\n", __func__);
        unsigned short cm_changed_status= 0;
        rc = isx005_i2c_read(isx005_client->addr, 0x00F8, &cm_changed_status, BYTE_LEN);

        if(cm_changed_status & 0x0002)
        {
  	       printk("[%s]:Sensor Snapshot Mode check : %d-> success \n", __func__, cm_changed_status);
          break;  
        }
        else 
          msleep(10);

        printk("[%s]:Sensor Snapshot Mode checking : %d \n", __func__, cm_changed_status);        
   }

	return rc;
}

static int isx005_set_sensor_mode(int mode)
{
	int rc = 0;
	int retry = 0;

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		for (retry = 0; retry < 3; ++retry) {
			rc = isx005_reg_preview();
			if (rc < 0)
				printk(KERN_ERR "[ERROR]%s:Sensor Preview Mode Fail\n", __func__);
			else
				break;

			mdelay(1);
		}
		
		break;

	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE:	/* Do not support */
		for (retry = 0; retry < 3; ++retry) {
			rc = isx005_reg_snapshot();
			if (rc < 0)
				printk(KERN_ERR "[ERROR]%s:Sensor Snapshot Mode Fail\n", __func__);
			else
				break;

			mdelay(1);
		}
		break;

	default:
		return -EINVAL;
	}

	CDBG("Sensor Mode : %d, rc = %d\n", mode, rc);

	msleep(20);

	return rc;
}

static int isx005_cancel_focus(int mode)
{
	int rc;
	int lense_po_back=0;
	
	switch(mode){
	case 0:
		lense_po_back = 0x3200;
		break;
	
	case 1:
		lense_po_back = 0x0403;
		break;
	}

	rc = isx005_i2c_write(isx005_client->addr,
			0x002E, 0x02, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx005_i2c_write(isx005_client->addr,
			0x4852, lense_po_back, WORD_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx005_i2c_write(isx005_client->addr,
			0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx005_i2c_write(isx005_client->addr,
			0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = isx005_i2c_write(isx005_client->addr,
			0x00FC, 0x1F, BYTE_LEN);
	if (rc < 0)
		return rc;

	return rc;
}

static int isx005_check_af_lock(void)
{
	int rc;
	int i;
	unsigned short af_lock;
	

	for (i = 0; i < 10; ++i) {
		/*INT state read -*/
		rc = isx005_i2c_read(isx005_client->addr,
			0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			CDBG("isx005: reading af_lock fail\n");
			return rc;
		}

		/* af interruption lock state read compelete */
		if((af_lock & 0x10) == 0x10)
			break;

		msleep(10);
	}

	/* INT clear */
	rc = isx005_i2c_write(isx005_client->addr,
		0x00FC, 0x10, BYTE_LEN);
	if (rc < 0)
		return rc;

	for (i = 0; i < 10; ++i) {
		/*INT state read to confirm INT release state*/
		rc = isx005_i2c_read(isx005_client->addr,
				0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			CDBG("isx005: reading af_lock fail\n");
			return rc;
		}

		if ((af_lock & 0x10) == 0x00) {
			CDBG("af_lock is released\n");
			break;
		}
		msleep(10);
	}

	return rc;
}

static int isx005_check_focus(int *lock)
{
	int rc;
	unsigned short af_status;
	unsigned short af_result;

	printk("isx005_check_focus\n");

	/*af status check  0:load, 1: init,  8: af_lock */
	rc = isx005_i2c_read(isx005_client->addr,
		0x6D76, &af_status, BYTE_LEN);

	if (af_status != 0x8)
		return -ETIME;

	
	isx005_check_af_lock();
	
	/* af result read  success/ fail*/
	rc = isx005_i2c_read(isx005_client->addr, 0x6D77, &af_result, BYTE_LEN);
	if (rc < 0) {
		printk("[isx005.c]%s: fai; in reading af_result\n",__func__);
		return rc;
	}

	/* single autofocus off */
	rc = isx005_i2c_write(isx005_client->addr, 0x002E, 0x03, BYTE_LEN);
	if (rc < 0)
		return rc;
		
	/* single autofocus refresh*/
	rc = isx005_i2c_write(isx005_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	if (af_result == 1) {
		*lock = CFG_AF_LOCKED;  // success
		return rc;
	} else {
		*lock = CFG_AF_UNLOCKED; //0: focus fail or 2: during focus
		return rc;
	}

	return -ETIME;
}

static int isx005_set_af_start(int mode)
{
	int rc = 0;

	if(prev_af_mode == mode) {
		rc = isx005_i2c_write_table(isx005_regs.af_start_reg_settings,
			isx005_regs.af_start_reg_settings_size);
	} else {
		switch (mode) {
			case FOCUS_NORMAL:
				rc = isx005_i2c_write_table(isx005_regs.af_normal_reg_settings,
					isx005_regs.af_normal_reg_settings_size);
				break;

			case FOCUS_MACRO:
				rc = isx005_i2c_write_table(isx005_regs.af_macro_reg_settings,
					isx005_regs.af_macro_reg_settings_size);
				break;

			case FOCUS_AUTO:	
				rc = isx005_i2c_write_table(isx005_regs.af_normal_reg_settings,
					isx005_regs.af_normal_reg_settings_size);
				break;

			case FOCUS_MANUAL:	
				rc = isx005_i2c_write_table(isx005_regs.af_manual_reg_settings,
					isx005_regs.af_manual_reg_settings_size);
				break;

			default:
				printk(KERN_ERR "[ERROR]%s: invalid af mode\n", __func__);
				break;
		}
		/*af start*/
		rc = isx005_i2c_write_table(isx005_regs.af_start_reg_settings,
			isx005_regs.af_start_reg_settings_size);
	}	

	prev_af_mode = mode;
		
	return rc;
}

static int isx005_move_focus(int32_t steps)
{
	int32_t rc;
	unsigned short cm_changed_sts, cm_changed_clr, af_pos, manual_pos;
	int i;

	rc = isx005_i2c_write_table(isx005_regs.af_manual_reg_settings,
			isx005_regs.af_manual_reg_settings_size);

	prev_af_mode = FOCUS_MANUAL;

	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in writing for move focus\n",
			__func__);
		return rc;
	}

	/* check cm_changed_sts */
	for(i = 0; i < 24; ++i) {
		rc = isx005_i2c_read(isx005_client->addr,
				0x00F8, &cm_changed_sts, BYTE_LEN);
		if (rc < 0){
			printk(KERN_ERR "[ERROR]%s; fail in reading cm_changed_sts\n",
				__func__);
			return rc;
		}

		if((cm_changed_sts & 0x02) == 0x02)
			break;

		msleep(10);
	}

	/* clear the interrupt register */
	rc = isx005_i2c_write(isx005_client->addr, 0x00FC, 0x02, BYTE_LEN);
	if (rc < 0)
		return rc;

	/* check cm_changed_clr */
	for(i = 0; i < 24; ++i) {
		rc = isx005_i2c_read(isx005_client->addr,
			0x00FC, &cm_changed_clr, BYTE_LEN);
		if (rc < 0) {
			printk(KERN_ERR "[ERROR]%s:fail in reading cm_changed_clr\n",
				__func__);
			return rc;
		}

		if((cm_changed_clr & 0x00) == 0x00)
			break;

		msleep(10);
	}

	if (steps <= 10)
		manual_pos = cpu_to_be16(50 + (50 * steps));
	else
		manual_pos = 50;

	rc = isx005_i2c_write(isx005_client->addr, 0x4852, manual_pos, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = isx005_i2c_write(isx005_client->addr, 0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx005_i2c_write(isx005_client->addr, 0x0015, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	isx005_check_af_lock();
	
	/* check lens position */
	for(i = 0; i < 24; ++i) {
		rc = isx005_i2c_read(isx005_client->addr, 0x6D7A, &af_pos, WORD_LEN);
		if (rc < 0)
			printk(KERN_ERR "[ERROR]%s:fail in reading af_lenspos\n",
				__func__);
	
		if(af_pos == manual_pos)
			break;
		
		msleep(10);
	}

	return rc;
}

static int isx005_set_default_focus(void)
{
	int rc;

	rc = isx005_cancel_focus(prev_af_mode);
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in cancel_focus\n", __func__);
		return rc;
	}

	rc = isx005_i2c_write_table(isx005_regs.af_normal_reg_settings,
		isx005_regs.af_normal_reg_settings_size);

	prev_af_mode = FOCUS_AUTO;

	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in writing for focus\n", __func__);
		return rc;
	}

	msleep(60);
	
	isx005_check_focus(&rc);

	return rc;
}


static int isx005_set_effect(int effect)
{
	int rc = 0;

	switch (effect) {
	case CAMERA_EFFECT_OFF:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_MONO:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x04, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_NEGATIVE:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_SOLARIZE:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x01, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_SEPIA:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x03, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;
		
		break;

	/* This effect is not supported in ISX005 */
	case CAMERA_EFFECT_POSTERIZE:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	/* This effect is not supported in ISX005 */
	case CAMERA_EFFECT_AQUA:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_NEGATIVE_SEPIA:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6D11, WORD_LEN);
		if (rc < 0)
			return rc;
		
		break;

	case CAMERA_EFFECT_BLUE:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x03, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6D11, WORD_LEN);
		if (rc < 0)
			return rc;
		break;

	case CAMERA_EFFECT_PASTEL:
		rc = isx005_i2c_write(isx005_client->addr, 0x005F, 0x05, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx005_i2c_write(isx005_client->addr, 0x038A, 0x6911, WORD_LEN);
		if (rc < 0)
			return rc;
		break;		

	default:
		return -EINVAL;
	}

	CDBG("Effect : %d, rc = %d\n", effect, rc);

	return rc;
}

static int isx005_set_wb(int mode)
{
	int rc;

	switch (mode) {
		case CAMERA_WB_AUTO:
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x20, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;
			
		case CAMERA_WB_CUSTOM:	/* Do not support */
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x20, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;

		case CAMERA_WB_INCANDESCENT:
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x3B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x08, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_WB_FLUORESCENT:
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x3B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x07, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;
			
		case CAMERA_WB_DAYLIGHT:
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x3B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x04, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_WB_CLOUDY_DAYLIGHT:
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x3B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x06, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;

		case CAMERA_WB_TWILIGHT:	/* Do not support */
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x20, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_WB_SHADE:		/* Do not support */
			rc = isx005_i2c_write(isx005_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = isx005_i2c_write(isx005_client->addr, 0x0102, 0x20, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;

		default:
			return -EINVAL;
	}
	return rc;
}

static int isx005_set_antibanding(int mode)
{
	int rc;

	switch (mode) {
		case CAMERA_ANTIBANDING_OFF:
			rc = isx005_i2c_write(isx005_client->addr, 0x4001, 0x00, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_60HZ:
			rc = isx005_i2c_write(isx005_client->addr, 0x4001, 0x04, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_50HZ:
			rc = isx005_i2c_write(isx005_client->addr, 0x4001, 0x03, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_AUTO:
			rc = isx005_i2c_write(isx005_client->addr, 0x4001, 0x00, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_MAX_ANTIBANDING:
			rc = isx005_i2c_write(isx005_client->addr, 0x4001, 0x04, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		default:
			return -EINVAL;
	}

	return rc;	
}

static int isx005_set_iso(int iso)
{
	int32_t rc;
	
	switch (iso) {
		case CAMERA_ISO_AUTO:
			rc = isx005_i2c_write(isx005_client->addr,
					0x01E5, 0x00, BYTE_LEN);
			break;

		case CAMERA_ISO_DEBLUR:	/* Do not support */
		case CAMERA_ISO_100:
			rc = isx005_i2c_write(isx005_client->addr,
					0x01E5, 0x07, BYTE_LEN);
			break;

		case CAMERA_ISO_200:
			rc = isx005_i2c_write(isx005_client->addr,
					0x01E5, 0x0A, BYTE_LEN);
			break;

		case CAMERA_ISO_400:
			rc = isx005_i2c_write(isx005_client->addr,
					0x01E5, 0x0D, BYTE_LEN);
			break;
			
		case CAMERA_ISO_800:
			rc = isx005_i2c_write(isx005_client->addr,
					0x01E5, 0x10, BYTE_LEN);
			break;

		default:
			rc = -EINVAL;
	}
	
	return rc;
}

static int32_t isx005_set_scene_mode(int8_t mode)
{
	int32_t rc = 0;

	if (prev_scene_mode == mode)
		return rc;

	switch (mode) {
		case CAMERA_SCENE_AUTO:
			rc = isx005_i2c_write_table(isx005_regs.scene_auto_reg_settings,
				isx005_regs.scene_auto_reg_settings_size);
			break;

		case CAMERA_SCENE_PORTRAIT:
			rc = isx005_i2c_write_table(isx005_regs.scene_portrait_reg_settings,
				isx005_regs.scene_portrait_reg_settings_size);
			break;

		case CAMERA_SCENE_LANDSCAPE:
			rc = isx005_i2c_write_table(isx005_regs.scene_landscape_reg_settings,
				isx005_regs.scene_landscape_reg_settings_size);
			break;

		case CAMERA_SCENE_SPORTS:
			rc = isx005_i2c_write_table(isx005_regs.scene_sports_reg_settings,
				isx005_regs.scene_sports_reg_settings_size);
			break;

		case CAMERA_SCENE_SUNSET:
			rc = isx005_i2c_write_table(isx005_regs.scene_sunset_reg_settings,
				isx005_regs.scene_sunset_reg_settings_size);
			break;

		case CAMERA_SCENE_NIGHT:
			rc = isx005_i2c_write_table(isx005_regs.scene_night_reg_settings,
				isx005_regs.scene_night_reg_settings_size);
			break;

		default:
			printk(KERN_ERR "[ERROR]%s:Incorrect scene mode value\n", __func__);
	}

	prev_scene_mode = mode;

	return rc;
}

static int32_t isx005_set_brightness(int8_t brightness)
{
	int32_t rc=0;
	
	switch (brightness) {
		case 0:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;
			
			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x50, BYTE_LEN);
			if(rc<0)
				return rc;
			
			break;

		case 1:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x60, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 2:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x70, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 3:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0xCD, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
			return rc;	

			break;

		case 4:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0xEF, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 5:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x00, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 6:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x18, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 7:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x8A, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 8:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0x9C, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 9:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0xAA, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 10:
			rc = isx005_i2c_write(isx005_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = isx005_i2c_write(isx005_client->addr,
					0x0061, 0xC8, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		default:
			printk(KERN_ERR "[ERROR]%s:incoreect brightness value\n",
				__func__);
	}
	
	return rc;
}

static int isx005_init_sensor(const struct msm_camera_sensor_info *data)
{
	int rc;
	int nNum = 0;

	rc = data->pdata->camera_power_on();
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:failed to power on!\n", __func__);
		return rc;
	}

	/*pll register write*/
	rc = isx005_reg_init();
	if (rc < 0) {
		for(nNum = 0; nNum<5; nNum++)
		{
		 msleep(2);
			printk(KERN_ERR "[ERROR]%s:Set initial register error! retry~! \n", __func__);
			rc = isx005_reg_init();
			if(rc < 0)
			{
				nNum++;
				printk(KERN_ERR "[ERROR]%s:Set initial register error!- loop no:%d \n", __func__, nNum);
			}
			else
			{
				printk(KERN_DEBUG"[%s]:Set initial register Success!\n", __func__);
				break;
			}
		}
	}

	mdelay(16);  // T3+T4

	/*tuning register write*/
	rc = isx005_reg_tuning();
	if (rc < 0) {
		for(nNum = 0; nNum<5 ;nNum++)
		{
		  msleep(2);
			printk(KERN_ERR "[ERROR]%s:Set initial register error! retry~! \n", __func__);
			rc = isx005_reg_tuning();
			if(rc < 0)
			{
				nNum++;
				printk(KERN_ERR "[ERROR]%s:Set tuning register error! loop no:%d\n", __func__, nNum);
			}
			else
			{
				printk(KERN_DEBUG"[%s]:Set initial tuning Success!\n", __func__);
				break;
			}
		}
	
	}

	return rc;
}

#define INIT_REG_SIZE		200
#define TUNING_REG_SIZE		4000

static struct isx005_register_address_value_pair
	init_register[INIT_REG_SIZE];
static struct isx005_register_address_value_pair
	tuning_register[TUNING_REG_SIZE];

#define BUF_SIZE	(256 * 1024)

static void parsing_init_register(char* buf, int buf_size)
{
	int i = 0;
	int reg_count = 0;
	
	int addr, value, length;
	int rc;

	char scan_buf[32];
	int scan_buf_len;

	while (i < buf_size && reg_count < INIT_REG_SIZE) {
		if (buf[i] == '{') {
			scan_buf_len = 0;
			while(buf[i] != '}' && scan_buf_len < 30) {
				if (buf[i] < 33 || 126 < buf[i]) {
					++i;
					continue;
				} else
					scan_buf[scan_buf_len++] = buf[i++];
			}
			scan_buf[scan_buf_len++] = buf[i];
			scan_buf[scan_buf_len] = 0;

			rc = sscanf(scan_buf, "{%x,%x,%d}", &addr, &value, &length);
			if (rc != 3) {
				printk(KERN_ERR "file format error. rc = %d\n", rc);
				return;
			}
			printk("init reg[%d] addr = 0x%x, value = 0x%x, length = %d\n", 
				reg_count, addr, value, length);

			init_register[reg_count].register_address = addr;
			init_register[reg_count].register_value = value;
			init_register[reg_count].register_length= length;
			++reg_count;
		}
		++i;
	}

	if (reg_count > 0) {
		isx005_regs.init_reg_settings = init_register;
		isx005_regs.init_reg_settings_size = reg_count;
	}
}

static void isx005_read_init_register_from_file()
{
	int fd;
	mm_segment_t oldfs;
	char* buf;
	int read_size;

	oldfs = get_fs();
	set_fs(get_ds());

	fd = sys_open("/sdcard/init_register", O_RDONLY, 0644);
	if (fd < 0) {
		printk(KERN_ERR "File open fail\n");
		return;
	}

	buf = kmalloc(BUF_SIZE, GFP_KERNEL);
	if (!buf) {
		printk(KERN_ERR "Memory alloc fail\n");
		return;
	}

	read_size = sys_read(fd, buf, BUF_SIZE);
	if (read_size < 0) {
		printk(KERN_ERR "File read fail: read_size = %d\n", read_size);
		return;
	}
	
	parsing_init_register(buf, read_size);

	kfree(buf);

	sys_close(fd);

	set_fs(oldfs);
}

static void parsing_tuning_register(char* buf, int buf_size)
{
	int i = 0;
	int reg_count = 0;
	
	int addr, value, length;
	int rc;

	char scan_buf[32];
	int scan_buf_len;

	while (i < buf_size && reg_count < TUNING_REG_SIZE) {
		if (buf[i] == '{') {
			scan_buf_len = 0;
			while(buf[i] != '}' && scan_buf_len < 30) {
				if (buf[i] < 33 || 126 < buf[i]) {
					++i;
					continue;
				} else
					scan_buf[scan_buf_len++] = buf[i++];
			}
			scan_buf[scan_buf_len++] = buf[i];
			scan_buf[scan_buf_len] = 0;
			
			rc = sscanf(scan_buf, "{%x,%x,%d}", &addr, &value, &length);
			if (rc != 3) {
				printk(KERN_ERR "file format error. rc = %d\n", rc);
				return;
			}
			printk("tuning reg[%d] addr = 0x%x, value = 0x%x, length = %d\n",
				reg_count, addr, value, length);

			tuning_register[reg_count].register_address = addr;
			tuning_register[reg_count].register_value = value;
			tuning_register[reg_count].register_length= length;
			++reg_count;
		}
		++i;
	}

	if (reg_count > 0) {
		isx005_regs.tuning_reg_settings = tuning_register;
		isx005_regs.tuning_reg_settings_size = reg_count;
	}
}

static void isx005_read_tuning_register_from_file()
{
	int fd;
	mm_segment_t oldfs;
	char* buf;
	int read_size;

	oldfs = get_fs();
	set_fs(get_ds());

	fd = sys_open("/sdcard/tuning_register", O_RDONLY, 0644);
	if (fd < 0) {
		printk(KERN_ERR "File open fail\n");
		return;
	}

	buf = kmalloc(BUF_SIZE, GFP_KERNEL);
	if (!buf) {
		printk(KERN_ERR "Memory alloc fail\n");
		return;
	}

	read_size = sys_read(fd, buf, BUF_SIZE);
	if (read_size < 0) {
		printk(KERN_ERR "File read fail: read_size = %d\n", read_size);
		return;
	}
	
	parsing_tuning_register(buf, read_size);

	kfree(buf);

	sys_close(fd);

	set_fs(oldfs);
}

static int isx005_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	CDBG("init entry \n");

	if (data == 0) {
		printk(KERN_ERR "[ERROR]%s: data is null!\n", __func__);
		return -1;
	}

	isx005_read_init_register_from_file();
	isx005_read_tuning_register_from_file();

	rc = isx005_init_sensor(data);
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:failed to initialize sensor!\n", __func__);
		goto init_probe_fail;
	}

	prev_af_mode = -1;
	prev_scene_mode = -1;

	return rc;

init_probe_fail:
	return rc;
}

int isx005_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	isx005_ctrl = kzalloc(sizeof(struct isx005_ctrl), GFP_KERNEL);
	if (!isx005_ctrl) {
		printk(KERN_ERR "[ERROR]%s:isx005_init failed!\n", __func__);
		rc = -ENOMEM;
		goto init_done;
	}

	if (data)
		isx005_ctrl->sensordata = data;

	rc = isx005_sensor_init_probe(data);
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:isx005_sensor_init failed!\n", __func__);
		goto init_fail;
	}

init_done:
	return rc;

init_fail:
	kfree(isx005_ctrl);
	return rc;
}

int isx005_sensor_release(void)
{
	int rc = 0;

	if(always_on) {
		printk("always power-on camera.\n");
		return rc;
	}

	mutex_lock(&isx005_mutex);

	rc = isx005_ctrl->sensordata->pdata->camera_power_off();

	kfree(isx005_ctrl);

	mutex_unlock(&isx005_mutex);

	return rc;
}

int isx005_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	int rc;

	rc = copy_from_user(&cfg_data, (void *)argp,
		sizeof(struct sensor_cfg_data));

	if (rc < 0)
		return -EFAULT;

	CDBG("isx005_ioctl, cfgtype = %d, mode = %d\n",
		cfg_data.cfgtype, cfg_data.mode);

	mutex_lock(&isx005_mutex);

	switch (cfg_data.cfgtype) {
		case CFG_SET_MODE:
			rc = isx005_set_sensor_mode(cfg_data.mode);
			break;

		case CFG_SET_EFFECT:
			//rc = isx005_set_effect(cfg_data.mode);
			break;

		case CFG_MOVE_FOCUS:
			rc = isx005_move_focus(cfg_data.cfg.focus.steps);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc = isx005_set_default_focus();
			break;

		case CFG_GET_AF_MAX_STEPS:
			cfg_data.max_steps = ISX005_TOTAL_STEPS_NEAR_TO_FAR;
			if (copy_to_user((void *)argp,
					&cfg_data,
					sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_START_AF_FOCUS:
			rc = isx005_set_af_start(cfg_data.mode);
			break;

		case CFG_CHECK_AF_DONE:
			rc = isx005_check_focus(&cfg_data.mode);
			if (copy_to_user((void *)argp,
					&cfg_data,
					sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_CHECK_AF_CANCEL:
			rc = isx005_cancel_focus(cfg_data.mode);
			break;

		case CFG_SET_WB:
			//rc = isx005_set_wb(cfg_data.mode);
			break;

		case CFG_SET_ANTIBANDING:
			//rc= isx005_set_antibanding(cfg_data.mode);
			break;

		case CFG_SET_ISO:
			//rc = isx005_set_iso(cfg_data.mode);
			break;

		case CFG_SET_SCENE:
			//rc = isx005_set_scene_mode(cfg_data.mode);
			break;

		case CFG_SET_BRIGHTNESS:
			//rc = isx005_set_brightness(cfg_data.mode);
			break;

		default:
			rc = -EINVAL;
			break;
	}

	mutex_unlock(&isx005_mutex);

	return rc;
}

static const struct i2c_device_id isx005_i2c_id[] = {
	{ "isx005", 0},
	{ },
};

static int isx005_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&isx005_wait_queue);
	return 0;
}

static int isx005_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	isx005_sensorw = kzalloc(sizeof(struct isx005_work), GFP_KERNEL);
	if (!isx005_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, isx005_sensorw);
	isx005_init_client(client);
	isx005_client = client;

	CDBG("isx005_probe succeeded!\n");

	return rc;

probe_failure:
	printk(KERN_ERR "[ERROR]%s:isx005_probe failed!\n", __func__);
	return rc;
}

static struct i2c_driver isx005_i2c_driver = {
	.id_table = isx005_i2c_id,
	.probe  = isx005_i2c_probe,
	.remove = __exit_p(isx005_i2c_remove),
	.driver = {
		.name = "isx005",
	},
};

static ssize_t mclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("mclk_rate = %d\n", mclk_rate);
	return 0;
}

static ssize_t mclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	mclk_rate = value;

	printk("mclk_rate = %d\n", mclk_rate);
	return size;
}

static DEVICE_ATTR(mclk, S_IRWXUGO, mclk_show, mclk_store);

static ssize_t always_on_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("always_on = %d\n", always_on);
	return 0;
}

static ssize_t always_on_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
	always_on = value;

	printk("always_on = %d\n", always_on);
	return size;
}

static DEVICE_ATTR(always_on, S_IRWXUGO, always_on_show, always_on_store);

static int isx005_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&isx005_i2c_driver);
	if (rc < 0 || isx005_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;
	}

	s->s_init = isx005_sensor_init;
	s->s_release = isx005_sensor_release;
	s->s_config  = isx005_sensor_config;

	rc = device_create_file(&isx005_pdev->dev, &dev_attr_mclk);
	if (rc < 0) {
		printk("device_create_file error!\n");
		return rc;
	}

	rc = device_create_file(&isx005_pdev->dev, &dev_attr_always_on);
	if (rc < 0) {
		printk("device_create_file error!\n");
		return rc;
	}

probe_done:
	CDBG("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __isx005_probe(struct platform_device *pdev)
{
	isx005_pdev = pdev;
	return msm_camera_drv_start(pdev, isx005_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __isx005_probe,
	.driver = {
		.name = "msm_camera_isx005",
		.owner = THIS_MODULE,
	},
};

static int __init isx005_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

late_initcall(isx005_init);
