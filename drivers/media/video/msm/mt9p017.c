/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Aptina 5M MT9P017 camera sensor driver
 * Auther: Eim junghee[junghee.eim@lge.com], 2010-04-09
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
#include <linux/slab.h>

#include "mt9p017.h"
#include "mt9p017_reg.h"
//#include "register_common_init.h"

typedef enum {
	CAMERA_PREVIEW_MODE_FPS = 0,
	CAMERA_CAMCORD_MODE_FPS,
	CAMERA_CAMCORD_MODE_FPS_FOR_DELIVERING,
	CAMERA_MODE_DEFAULT,
	CAMERA_PREVIEW_MODE_MAX
}camera_preview_mode_t;

#define MT9P017_INTERVAL_T2		8	/* 8ms */
#define MT9P017_INTERVAL_T3		1	/* 0.5ms */
#define MT9P017_INTERVAL_T4		2	/* 15ms */
#define MT9P017_INTERVAL_T5		25	/* 200ms */

/*
* AF Total steps parameters
*/
#define MT9P017_TOTAL_STEPS_NEAR_TO_FAR	30

/*  MT9P017 Registers  */
#define REG_MT9P017_INTSTS_ID			0x00F8	/* Interrupt status */
#define REG_MT9P017_INTCLR_ID			0x00FC	/* Interrupt clear */

#define MT9P017_OM_CHANGED				0x0001	/* Operating mode */
#define MT9P017_CM_CHANGED				0x0002	/* Camera mode */

#define MT9P017_MAX_ZOOM_STEP    16
/* It is distinguish normal from macro focus */
static int prev_af_mode;
/* It is distinguish scene mode */
static int prev_scene_mode;
static int32_t prevew_mode_status;
static int32_t prev_zoom_step;

struct mt9p017_work {
	struct work_struct work;
};
static struct mt9p017_work *mt9p017_sensorw;

static struct i2c_client *mt9p017_client;

struct mt9p017_ctrl {
	const struct msm_camera_sensor_info *sensordata;
};

static struct mt9p017_ctrl *mt9p017_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(mt9p017_wait_queue);

DEFINE_MUTEX(mt9p017_mutex);

struct platform_device *mt9p017_pdev;

static int always_on = 0;

static int is_from_capture;

static int zoom_ratio = 0x105;

static int zoom_offset_x = 0;

static int zoom_offset_y = 0;

static int zoom_table[MT9P017_MAX_ZOOM_STEP] = {0x0100, 0x0104, 0x0108, 0x010C, 0x0111, 0x0115, 0x0119, 0x011D, 0x0122, 0x0126, \
                                               0x012A, 0x012E, 0x0133, 0x0137, 0x013B, 0x0140};

static int32_t mt9p017_i2c_txdata(unsigned short saddr,
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

	if (i2c_transfer(mt9p017_client->adapter, msg, 1) < 0) {
		CDBG("mt9p017_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t mt9p017_i2c_write(unsigned short saddr, unsigned short waddr, unsigned int wdata, enum mt9p017_width width)
{
	int32_t rc = -EIO;
  int int_buf[2];
	unsigned char *pbuf = ((unsigned char*)int_buf) + 2;  // to align multipul 4

  pbuf[0] = (waddr & 0xFF00) >> 8;
  pbuf[1] = (waddr & 0x00FF);
  //*((unsigned int*)&pbuf[2]) = wdata;
  
	switch (width) {
		case BYTE_LEN:	
			pbuf[2] = (unsigned char)wdata;
			break;
		case WORD_LEN:
			pbuf[2] = (unsigned char)((wdata & 0xFF00) >> 8);
			pbuf[3] = (unsigned char)(wdata & 0x00FF);
    case QUAD_LEN:
			//rc = mt9p017_i2c_txdata(saddr, pbuf, width + 2);
			break;

		default:
			break;
	}
  
  rc = mt9p017_i2c_txdata(saddr, pbuf, width + 2);
  
	if (rc < 0)
		printk(KERN_ERR "i2c_write failed, addr = 0x%x, val = 0x%x!\n", waddr, wdata);

	return rc;
}

static int32_t mt9p017_i2c_write_table(
	struct mt9p017_register_address_value_pair const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i;
	int32_t rc = -EIO;

	for (i = 0; i < num_of_items_in_table; ++i) {
		rc = mt9p017_i2c_write(mt9p017_client->addr,
			reg_conf_tbl->register_address, reg_conf_tbl->register_value,
			reg_conf_tbl->register_length);
		if (rc < 0)
			break;

		reg_conf_tbl++;
	}

	return rc;
}

static int mt9p017_i2c_rxdata(unsigned short saddr,
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

	if (i2c_transfer(mt9p017_client->adapter, msgs, 2) < 0) {
		printk(KERN_ERR "mt9p017_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t mt9p017_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned int *rdata, enum mt9p017_width width)
{
	int32_t rc = 0;
  int int_buf;
	unsigned char *pbuf = (unsigned char*)&int_buf;

	if (!rdata)
		return -EIO;

	//memset(buf, 0, sizeof(buf));

  pbuf[0] = (raddr & 0xFF00) >> 8;
  pbuf[1] = (raddr & 0x00FF);

  rc = mt9p017_i2c_rxdata(saddr, pbuf, (int)width);
  if (rc < 0)
    return rc;

	switch (width) {
		case BYTE_LEN:
			*rdata = pbuf[0];
			break;
			
		case WORD_LEN:
			*rdata = ((0xFF00 & pbuf[0]) >> 8) + ((0x00FF & pbuf[1]));
			break;

		case QUAD_LEN:
			break;

		default:
			break;
	}

	if (rc < 0)
		printk(KERN_ERR "mt9p017_i2c_read failed!\n");

	return rc;
}
static int mt9p017_check_seq_cmd(void)
{
  unsigned int read_v = 0x00;
  int i;
  
  for (i=0; i<10; i++)
  {				
		mt9p017_i2c_read(mt9p017_client->addr, 0x8404, &read_v, BYTE_LEN); // SEQ_STATE
    if(read_v == 0x06)
    {
		  printk("5M Aptina Sensor check register complete!\n");
      break;
    }
		printk("5M Aptina Sensor checking register......\n");
		mdelay(10);
  }
	return 1;
}

static int mt9p017_reg_init(void)
{
	int rc = 0;
	int i, j;
	int check_v, read_v;
	printk("[mt9p017_reg_init] start\n");
	/* Configure sensor for Initial setting (PLL, Clock, etc) */
	for (i = 0; i < mt9p017_regs.init_reg_settings_size; ++i) 
	{
		if (mt9p017_regs.init_reg_settings[i].register_address == 0xFFFF)
		{
			msleep(mt9p017_regs.init_reg_settings[i].register_value);
			continue;
		}
		else if (mt9p017_regs.init_reg_settings[i].register_address == 0xFFFE)
		{
		  check_v = mt9p017_regs.init_reg_settings[i].register_length;
			for (j=0; j<10; j++)
			{ 							
				mt9p017_i2c_read(mt9p017_client->addr, mt9p017_regs.init_reg_settings[i].register_value, &read_v, BYTE_LEN); // SEQ_STATE
				if(read_v == check_v)
				{
					printk("[mt9p017_reg_init]5M Aptina Sensor complete!!! register = 0x%x, cur_val = 0x%x, tar_val = 0x%x......\n", mt9p017_regs.init_reg_settings[i].register_value, read_v, check_v);
					break;
				}
				printk("[mt9p017_reg_init]5M Aptina Sensor checking.. register = 0x%x, cur_val = 0x%x\n", mt9p017_regs.init_reg_settings[i].register_value, read_v);
				mdelay(10);
			}
			continue;
		}
		else
		{
  		rc = mt9p017_i2c_write(mt9p017_client->addr,
			mt9p017_regs.init_reg_settings[i].register_address,
			mt9p017_regs.init_reg_settings[i].register_value,
			mt9p017_regs.init_reg_settings[i].register_length);
		}
		if (rc < 0)
			return rc;
	}
	printk("[mt9p017_reg_init] End\n");
	return rc;
}

#if 0
static int mt9p017_reg_preview_addition(void)
{
	int rc = 0;
	int i, CHIPID_L, reg_len;
  const struct mt9p017_register_address_value_pair *preg_settings;

  rc = mt9p017_i2c_read(mt9p017_client->addr,
			0x00F8, &CHIPID_L, BYTE_LEN);

  if ((CHIPID_L & 0xF) == 0x1)
  {
    preg_settings = mt9p017_regs.preview_addition_AP003_reg_settings;
    reg_len = mt9p017_regs.preview_addition_AP003_reg_settings_size;
    printk("OTP_CHIPID_L == 0x01 !\n");
  }
  else
  {
    preg_settings = mt9p017_regs.preview_addition_AP001_reg_settings;
    reg_len = mt9p017_regs.preview_addition_AP001_reg_settings_size;
    printk("OTP_CHIPID_L != 0x01 !\n");
  }
  
	for (i = 0; i < reg_len; ++i) {
		rc = mt9p017_i2c_write(mt9p017_client->addr,
			preg_settings[i].register_address,
			preg_settings[i].register_value,
			preg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	return rc;
}
#endif

static int mt9p017_reg_tuning(void)
{
	int rc = 0;
	int i, j;
	int check_v, read_v;
	/* Configure sensor for various tuning */
	
	printk("[mt9p017_reg_tuning] start\n");
	for (i = 0; i < mt9p017_regs.tuning_reg_settings_size; ++i) 
	{
		if (mt9p017_regs.tuning_reg_settings[i].register_address == 0xFFFF)
		{
			msleep(mt9p017_regs.tuning_reg_settings[i].register_value);
			continue;
		}
		else if (mt9p017_regs.tuning_reg_settings[i].register_address == 0xFFFE)
		{
		  check_v = mt9p017_regs.tuning_reg_settings[i].register_length;
			for (j = 0; j < 50; j++)
			{ 							
				mt9p017_i2c_read(mt9p017_client->addr, mt9p017_regs.tuning_reg_settings[i].register_value, &read_v, BYTE_LEN); // SEQ_STATE
				if(read_v == check_v)
				{
					printk("[mt9p017_reg_tuning]5M Aptina Sensor check complete!! register = 0x%x, cur_val = 0x%x, tar_val = 0x%x......\n", mt9p017_regs.tuning_reg_settings[i].register_value, read_v, check_v);
					break;
				}
				printk("[mt9p017_reg_tuning]5M Aptina Sensor checking register = 0x%x, cur_val = 0x%x\n", mt9p017_regs.tuning_reg_settings[i].register_value, read_v);
				mdelay(10);
			}
			continue;
		}
		else
		{
		  rc = mt9p017_i2c_write(mt9p017_client->addr,
			mt9p017_regs.tuning_reg_settings[i].register_address,
			mt9p017_regs.tuning_reg_settings[i].register_value,
			mt9p017_regs.tuning_reg_settings[i].register_length);
		}
		if (rc < 0)
			return rc;
	}
	printk("[mt9p017_reg_tuning] End\n");
	return rc;
}

static int mt9p017_reg_preview(void)
{
	int rc = 0;
	int i;

    
#if 0
  // Zoom 1.00
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0032, 0x0100, WORD_LEN);  // EZOOM_MAG
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0034, 0x0000, WORD_LEN);  // OFFSET_X
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0036, 0x0000, WORD_LEN);  // OFFSET_Y
  if (rc < 0)
    return rc;

#endif


	/* Configure sensor for Preview mode */
	for (i = 0; i < mt9p017_regs.prev_reg_settings_size; ++i) {
		rc = mt9p017_i2c_write(mt9p017_client->addr,
		  mt9p017_regs.prev_reg_settings[i].register_address,
		  mt9p017_regs.prev_reg_settings[i].register_value,
		  mt9p017_regs.prev_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

  if (is_from_capture)
  {
  }

	return rc;
}

#if 0
static int mt9p017_reg_snapshot(void)
{
	int rc = 0;
	int i;

	/* Configure sensor for Snapshot mode */
	for (i = 0; i < mt9p017_regs.snap_reg_settings_size; ++i) {
		rc = mt9p017_i2c_write(mt9p017_client->addr,
			mt9p017_regs.snap_reg_settings[i].register_address,
			mt9p017_regs.snap_reg_settings[i].register_value,
			mt9p017_regs.snap_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	return rc;
}

static int mt9p017_reg_raw_snapshot(void)
{
	int rc = 0;
	int i;

	/* Configure sensor for Raw-Snapshot mode */
	for (i = 0; i < mt9p017_regs.snap_reg_settings_size; ++i) {
		rc = mt9p017_i2c_write(mt9p017_client->addr,
			mt9p017_regs.snap_reg_settings[i].register_address,
			mt9p017_regs.snap_reg_settings[i].register_value,
			mt9p017_regs.snap_reg_settings[i].register_length);

		if (rc < 0)
			return rc;
	}

	return rc;
}
#endif

static int mt9p017_snapshot_config(int width, int height)
{
  int rc = 0;
//  int read_v;
	return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x098E, 0x48C0, WORD_LEN); // LOGICAL_ADDRESS_ACCESS [CAM_OUTPUT_1_IMAGE_WIDTH]
  if (rc < 0)
    return rc;
  
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0xC8C0, width, WORD_LEN); // CAM_OUTPUT_1_IMAGE_WIDTH
  if (rc < 0)
    return rc;

  rc = mt9p017_i2c_write(mt9p017_client->addr, 0xC8C2, height, WORD_LEN); // CAM_OUTPUT_1_IMAGE_HEIGHT
  if (rc < 0)
    return rc;
  rc = mt9p017_check_seq_cmd();  
  if (rc < 0)
    return rc;
#if 0
  // Zoom 1.02
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0032, zoom_table[prev_zoom_step] + 0x0005, WORD_LEN);  // EZOOM_MAG
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0034, zoom_offset_x, WORD_LEN);  // OFFSET_X
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0036, zoom_offset_y, WORD_LEN);  // OFFSET_Y
  if (rc < 0)
    return rc;
#endif
  is_from_capture = 1;

  return rc;
}

static int mt9p017_raw_snapshot_config(int width, int height)
{
	int rc = 0;
  is_from_capture = 1;

  printk("zoom ration = %x\n", zoom_ratio);

  return rc;
}


static int mt9p017_set_sensor_mode(int mode, int width, int height)
{
	int rc = 0;
  return rc;
  switch (prev_scene_mode) {
    case CAMERA_SCENE_AUTO:
      if (mode == SENSOR_SNAPSHOT_MODE || mode == SENSOR_RAW_SNAPSHOT_MODE)
      {
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x401c, 0x0496, WORD_LEN);
        if (rc < 0)
          return rc;
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4020, 0x03FD, WORD_LEN);
        if (rc < 0)
          return rc;
      }
      else if (mode == SENSOR_PREVIEW_MODE)
      {
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x401c, 0x0496, WORD_LEN);
        if (rc < 0)
          return rc;
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4020, 0x0000, WORD_LEN);
        if (rc < 0)
          return rc;
      }
      break;

    case CAMERA_SCENE_SPORTS:
      if (mode == SENSOR_SNAPSHOT_MODE || mode == SENSOR_RAW_SNAPSHOT_MODE)
      {
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x401c, 0x0364, WORD_LEN);
        if (rc < 0)
          return rc;
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4020, 0x0331, WORD_LEN);
        if (rc < 0)
          return rc;
      }
      else if (mode == SENSOR_PREVIEW_MODE)
      {
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x401c, 0x0298, WORD_LEN);
        if (rc < 0)
          return rc;
        rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4020, 0x0000, WORD_LEN);
        if (rc < 0)
          return rc;
      }
      break;
  }

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = mt9p017_reg_preview();
		if (rc < 0)
			printk(KERN_ERR "[ERROR]%s:Sensor Preview Mode Fail\n", __func__);
		break;

	case SENSOR_SNAPSHOT_MODE:
		rc = mt9p017_snapshot_config(width, height);
		if (rc < 0)
			printk(KERN_ERR "[ERROR]%s:Sensor Snapshot Mode Fail\n", __func__);
		break;
    
  case SENSOR_RAW_SNAPSHOT_MODE:
		rc = mt9p017_raw_snapshot_config(width, height);
		if (rc < 0)
			printk(KERN_ERR "[ERROR]%s:Sensor Raw Snapshot Mode Fail\n", __func__);
		break;

	default:
		return -EINVAL;
	}
  
	printk("%s : %d, rc = %d\n", __func__, mode, rc);

	//msleep(20);

	return rc;
}

static int mt9p017_cancel_focus(int mode)
{
	int rc = 0;
	int lense_po_back = 0;

  printk("%s\n",__func__);
  return rc;  
	switch(mode){
	case 0:
    	rc = mt9p017_i2c_read(mt9p017_client->addr, 0x4800, &lense_po_back, WORD_LEN);
      if (rc < 0)
        return rc;

		//lense_po_back = 0x0032;
		break;
	
	case 1:
    rc = mt9p017_i2c_read(mt9p017_client->addr, 0x4802, &lense_po_back, WORD_LEN);
    if (rc < 0)
      return rc;

		//lense_po_back = 0x0304;
		break;
	}

	rc = mt9p017_i2c_write(mt9p017_client->addr,
			0x002E, 0x02, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = mt9p017_i2c_write(mt9p017_client->addr,
			0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = mt9p017_i2c_write(mt9p017_client->addr,
			0x4852, lense_po_back, WORD_LEN);
	if (rc < 0)
		return rc;
  
	rc = mt9p017_i2c_write(mt9p017_client->addr,
			0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = mt9p017_i2c_write(mt9p017_client->addr,
			0x00FC, 0x1F, BYTE_LEN);
	if (rc < 0)
		return rc;

	return rc;
}

static int mt9p017_check_af_lock(void)
{
	int rc = 0;
	int i;
	unsigned int af_lock = 0;
	
  printk("%s\n",__func__);
  return rc;
	for (i = 0; i < 10; ++i) {
		/*INT state read -*/
		rc = mt9p017_i2c_read(mt9p017_client->addr,
			0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			CDBG("mt9p017: reading af_lock fail\n");
			return rc;
		}

		/* af interruption lock state read compelete */
		if((af_lock & 0x10) == 0x10)
			break;

		msleep(10);
	}

	for (i = 0; i < 10; ++i) {
    /* INT clear */
    rc = mt9p017_i2c_write(mt9p017_client->addr,
      0x00FC, 0x10, BYTE_LEN);
    if (rc < 0)
      return rc;
    
		msleep(10);

		/*INT state read to confirm INT release state*/
		rc = mt9p017_i2c_read(mt9p017_client->addr,
				0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			CDBG("mt9p017: reading af_lock fail\n");
			return rc;
		}

		if ((af_lock & 0x10) == 0x00) {
			CDBG("af_lock is released\n");
			break;
		}
	}

	return rc;
}

static int mt9p017_check_focus(int *lock)
{
	int rc = 0;
	unsigned int af_status;
	unsigned int af_result;

	printk("mt9p017_check_focus\n");
  return rc;
	/*af status check  0:load, 1: init,  8: af_lock */
	rc = mt9p017_i2c_read(mt9p017_client->addr,
		0x6D76, &af_status, BYTE_LEN);

	if (af_status != 0x8)
		return -ETIME;

	mt9p017_check_af_lock();
	
	/* af result read  success/ fail*/
	rc = mt9p017_i2c_read(mt9p017_client->addr, 0x6D77, &af_result, BYTE_LEN);
	if (rc < 0) {
		printk("[mt9p017.c]%s: fai; in reading af_result\n",__func__);
		return rc;
	}

	/* single autofocus off */
	rc = mt9p017_i2c_write(mt9p017_client->addr, 0x002E, 0x03, BYTE_LEN);
	if (rc < 0)
		return rc;
		
	/* single autofocus refresh*/
	rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

  /* change normal fps */
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4014, 0x0F07, WORD_LEN); // AGC_SCL_L : Analog gain
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0103, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0108, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x010D, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0112, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0117, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x011C, 0x01, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
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

static int mt9p017_set_af_start(int mode)
{
	int rc = 0, af_lock = 0, i;

  printk("%s\n",__func__);
  return rc;
	for (i = 0; i < 10; ++i) {
    /* INT clear */
    rc = mt9p017_i2c_write(mt9p017_client->addr,
      0x00FC, 0x10, BYTE_LEN);
    if (rc < 0)
      return rc;
    
		msleep(10);

		/*INT state read to confirm INT release state*/
		rc = mt9p017_i2c_read(mt9p017_client->addr,
				0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			CDBG("mt9p017: reading af_lock fail\n");
			return rc;
		}

		if ((af_lock & 0x10) == 0x00) {
			CDBG("af_lock is released\n");
			break;
		}
	}

  /* change max fps */
  if (mode != FOCUS_MANUAL)
  {
    //rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4014, 0x15BA, WORD_LEN); // AGC_SCL_L : Analog gain
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4014, 0x0F07, WORD_LEN); // AGC_SCL_L : Analog gain
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0103, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0108, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x010D, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0112, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0117, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x011C, 0x00, BYTE_LEN);  // AE_SN1 : Normal AE ==> 고정 fps로 된다.
    if (rc < 0)
      return rc;
  }
  
	if(prev_af_mode == mode) {
		rc = mt9p017_i2c_write_table(mt9p017_regs.af_start_reg_settings,
			mt9p017_regs.af_start_reg_settings_size);
	} else {
		switch (mode) {
			case FOCUS_NORMAL:
				rc = mt9p017_i2c_write_table(mt9p017_regs.af_normal_reg_settings,
					mt9p017_regs.af_normal_reg_settings_size);
				break;

			case FOCUS_MACRO:
				rc = mt9p017_i2c_write_table(mt9p017_regs.af_macro_reg_settings,
					mt9p017_regs.af_macro_reg_settings_size);
				break;

			case FOCUS_AUTO:	
				rc = mt9p017_i2c_write_table(mt9p017_regs.af_normal_reg_settings,
					mt9p017_regs.af_normal_reg_settings_size);
				break;

			case FOCUS_MANUAL:	
				rc = mt9p017_i2c_write_table(mt9p017_regs.af_manual_reg_settings,
					mt9p017_regs.af_manual_reg_settings_size);
				break;

			default:
				printk(KERN_ERR "[ERROR]%s: invalid af mode\n", __func__);
				break;
		}
		/*af start*/
		rc = mt9p017_i2c_write_table(mt9p017_regs.af_start_reg_settings,
			mt9p017_regs.af_start_reg_settings_size);
	}	

	prev_af_mode = mode;
		
	return rc;
}

static int mt9p017_move_focus(int32_t steps)
{
	int32_t rc = 0;
	unsigned int cm_changed_sts, cm_changed_clr, af_pos, manual_pos;
	int i;
  static uint32_t manual_focus_value[] = {514, 468, 408, 375, 342, 309, 276, 243, 210, 150, 130};
  
	rc = mt9p017_i2c_write_table(mt9p017_regs.af_manual_reg_settings,
			mt9p017_regs.af_manual_reg_settings_size);

  prev_af_mode = FOCUS_MANUAL;

  printk(KERN_ERR "mt9p017_move_focus : step=%d\n", steps);
  return rc;  
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in writing for move focus\n",
			__func__);
		return rc;
	}

	/* check cm_changed_sts */
	for(i = 0; i < 24; ++i) {
		rc = mt9p017_i2c_read(mt9p017_client->addr,
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

	/* check cm_changed_clr */
	for(i = 0; i < 24; ++i) {
    /* clear the interrupt register */
    rc = mt9p017_i2c_write(mt9p017_client->addr, 0x00FC, 0x02, BYTE_LEN);
    if (rc < 0)
      return rc;

		rc = mt9p017_i2c_read(mt9p017_client->addr,
			0x00F8, &cm_changed_clr, BYTE_LEN);
		if (rc < 0) {
			printk(KERN_ERR "[ERROR]%s:fail in reading cm_changed_clr\n",
				__func__);
			return rc;
		}

		if((cm_changed_clr & 0x00) == 0x00)
			break;

		msleep(10);
	}

#if 0
	if (steps <= 10)
		manual_pos = 50 + (50 * steps);// cpu_to_be16(50 + (50 * steps));
	else
		manual_pos = 50;
#endif

	if (steps <= 10)
	    manual_pos = manual_focus_value[steps];
	else
		manual_pos = 130;
  
	rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4852, manual_pos, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0015, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	mt9p017_check_af_lock();
	
	/* check lens position */
	for(i = 0; i < 24; ++i) {
		rc = mt9p017_i2c_read(mt9p017_client->addr, 0x6D7A, &af_pos, WORD_LEN);
		if (rc < 0)
			printk(KERN_ERR "[ERROR]%s:fail in reading af_lenspos\n",
				__func__);
	
		if(af_pos == manual_pos)
			break;
		
		msleep(10);
	}

	return rc;
}

static int mt9p017_set_default_focus(void)
{
	int rc = 0;

  printk(KERN_ERR "mt9p017_set_default_focus ..... \n");
  return rc;
	rc = mt9p017_cancel_focus(prev_af_mode);
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in cancel_focus\n", __func__);
		return rc;
	}

	rc = mt9p017_i2c_write_table(mt9p017_regs.af_normal_reg_settings,
		mt9p017_regs.af_normal_reg_settings_size);

	prev_af_mode = FOCUS_AUTO;

	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:fail in writing for focus\n", __func__);
		return rc;
	}

	msleep(60);
	
	mt9p017_check_focus(&rc);

	return rc;
}


static int mt9p017_set_effect(int effect)
{
	int rc = 0;

  printk("%s\n",__func__);
  
	return rc;
	switch (effect) {
	case CAMERA_EFFECT_OFF:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_MONO:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x04, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_NEGATIVE:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_SOLARIZE:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x01, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_SEPIA:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x03, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;
		
		break;

	/* This effect is not supported in MT9P017 */
	case CAMERA_EFFECT_POSTERIZE:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	/* This effect is not supported in MT9P017 */
	case CAMERA_EFFECT_AQUA:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;

		break;

	case CAMERA_EFFECT_NEGATIVE_SEPIA:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;
		
		break;

	case CAMERA_EFFECT_BLUE:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x03, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;
		break;

	case CAMERA_EFFECT_PASTEL:
		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x005F, 0x05, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = mt9p017_i2c_write(mt9p017_client->addr, 0x038A, 0x1169, WORD_LEN);
		if (rc < 0)
			return rc;
		break;		

	default:
		return -EINVAL;
	}

	CDBG("Effect : %d, rc = %d\n", effect, rc);

	return rc;
}

static int mt9p017_set_wb(int mode)
{
	int rc = 0;

  printk(KERN_INFO"mt9p017_set_wb : %d \n", mode);
	return rc;

	switch (mode) {
		case CAMERA_WB_AUTO:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0102, 0x20, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;

		case CAMERA_WB_INCANDESCENT:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0102, 0x28, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_WB_FLUORESCENT:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0102, 0x27, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;
			
		case CAMERA_WB_DAYLIGHT:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4453, 0x7B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0102, 0x24, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_WB_CLOUDY_DAYLIGHT:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4453, 0x3B, BYTE_LEN);
			if (rc < 0)
				return rc;

			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0102, 0x26, BYTE_LEN);
			if (rc < 0)
				return rc;
			
			break;

		case CAMERA_WB_TWILIGHT:	/* Do not support */
    case CAMERA_WB_CUSTOM:  /* Do not support */
		case CAMERA_WB_SHADE:		/* Do not support */
		default:
			return -EINVAL;
	}
	return rc;
}

static int mt9p017_set_antibanding(int mode)
{
	int rc = 0;

  printk(KERN_ERR "mt9p017_set_antibanding : %d\n", mode);
	return rc;

	switch (mode) {
		case CAMERA_ANTIBANDING_OFF:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4001, 0x00, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_60HZ:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4001, 0x04, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_50HZ:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4001, 0x03, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_ANTIBANDING_AUTO:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4001, 0x00, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		case CAMERA_MAX_ANTIBANDING:
			rc = mt9p017_i2c_write(mt9p017_client->addr, 0x4001, 0x04, BYTE_LEN);
			if (rc < 0)
				return rc;

			break;

		default:
			return -EINVAL;
	}

	return rc;	
}

static int mt9p017_set_iso(int iso)
{
	int32_t rc = 0;

  printk("%s\n",__func__);
	return rc;

	switch (iso) {
		case CAMERA_ISO_AUTO:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x01E5, 0x00, BYTE_LEN);
			break;

		case CAMERA_ISO_DEBLUR:	/* Do not support */
		case CAMERA_ISO_100:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x01E5, 0x07, BYTE_LEN);
			break;

		case CAMERA_ISO_200:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x01E5, 0x0A, BYTE_LEN);
			break;

		case CAMERA_ISO_400:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x01E5, 0x0D, BYTE_LEN);
			break;
			
		case CAMERA_ISO_800:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x01E5, 0x10, BYTE_LEN);
			break;

		default:
			rc = -EINVAL;
	}
	
	return rc;
}

static int32_t mt9p017_set_scene_mode(int8_t mode)
{
	int32_t rc = 0;

  printk("%s\n",__func__);
	return rc;

	if (prev_scene_mode == mode)
		return rc;

	switch (mode) {
		case CAMERA_SCENE_AUTO:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_auto_reg_settings,
				mt9p017_regs.scene_auto_reg_settings_size);
			break;

		case CAMERA_SCENE_PORTRAIT:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_portrait_reg_settings,
				mt9p017_regs.scene_portrait_reg_settings_size);
			break;

		case CAMERA_SCENE_LANDSCAPE:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_landscape_reg_settings,
				mt9p017_regs.scene_landscape_reg_settings_size);
			break;

		case CAMERA_SCENE_SPORTS:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_sports_reg_settings,
				mt9p017_regs.scene_sports_reg_settings_size);
			break;

		case CAMERA_SCENE_SUNSET:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_sunset_reg_settings,
				mt9p017_regs.scene_sunset_reg_settings_size);
			break;

		case CAMERA_SCENE_NIGHT:
			rc = mt9p017_i2c_write_table(mt9p017_regs.scene_night_reg_settings,
				mt9p017_regs.scene_night_reg_settings_size);
			break;

		default:
			printk(KERN_ERR "[ERROR]%s:Incorrect scene mode value\n", __func__);
	}

	prev_scene_mode = mode;

	return rc;
}

static int32_t mt9p017_set_brightness(int8_t brightness)
{
	int32_t rc=0;

  printk("%s\n",__func__);
	return rc;

	switch (brightness) {
		case 0:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;
			
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x50, BYTE_LEN);
			if(rc<0)
				return rc;
			
			break;

		case 1:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x60, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 2:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x70, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 3:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0xCD, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
			return rc;	

			break;

		case 4:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0xEF, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 5:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x00, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 6:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x18, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x80, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 7:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x8A, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 8:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0x9C, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 9:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0061, 0xAA, BYTE_LEN);
			if(rc<0)
				return rc;	

			break;

		case 10:
			rc = mt9p017_i2c_write(mt9p017_client->addr,
					0x0060, 0x7F, BYTE_LEN);
			if(rc<0)
				return rc;	

			rc = mt9p017_i2c_write(mt9p017_client->addr,
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

#if 0 // UNIVA_TEMP
static int mt9p017_set_jpeg_quality(int8_t quality)
{
	int32_t rc = 0;
  uint8_t jpeg_qulity;

  if (quality >= 85)
    jpeg_qulity = 0x02;
  else if (quality >= 75)
    jpeg_qulity = 0x01;
  else
    jpeg_qulity = 0x00;
    
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0204, jpeg_qulity, BYTE_LEN);
  
  printk(KERN_INFO "MT9P017: " "jpeg quality %d\n", quality);

  return rc;
}

static int mt9p017_set_jpeg_rotation(int32_t rotation)
{
	int32_t rc = 0;
  uint16_t jpeg_dri;
  
  if (rotation > 0)
    jpeg_dri = 0x0001;
  else
    jpeg_dri = 0x0000;

  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0202, jpeg_dri, WORD_LEN);  //JPEG DRI
  
  printk(KERN_INFO "MT9P017: " "jpeg rotation %d\n", rotation);
   
  return rc;
}

static int32_t mt9p017_set_preview_mode(int32_t mode)
{
	int32_t rc = 0;
	//int retry = 0;  

  printk(KERN_ERR "prevew_mode_status->%d , mode -> %d\n", prevew_mode_status, mode);
	return rc;

  if(prevew_mode_status == mode)
  { 
      return rc; 
  }
    
	switch (mode) {
		case CAMERA_PREVIEW_MODE_FPS:
      mt9p017_i2c_write(mt9p017_client->addr,0x4016,0x2439,2);// 35.00   ms   
      mt9p017_i2c_write(mt9p017_client->addr,0x4018,0x03CA,2);// 5.70  dB   
      mt9p017_i2c_write(mt9p017_client->addr,0x401A,0x029B,2);// 55.00   ms   
      mt9p017_i2c_write(mt9p017_client->addr,0x401C,0x0364,2);// 10.80   dB   
      mt9p017_i2c_write(mt9p017_client->addr,0x401E,0x03BB,2);// 105.00  ms   
      mt9p017_i2c_write(mt9p017_client->addr,0x4020,0x052f,2);// 16.50   dB   
      mt9p017_i2c_write(mt9p017_client->addr,0x02A4,0x0000,2); //VADJ_SENS_1
      mt9p017_i2c_write(mt9p017_client->addr,0x0383,0x02,1); //FPSTYPE_MONI   
      mt9p017_i2c_write(mt9p017_client->addr,0x0012,0x01,1); // MONI_REFRESH_F
			break;

		case CAMERA_CAMCORD_MODE_FPS:
      mt9p017_i2c_write(mt9p017_client->addr,0x4016,0x1CFF,2);	//10.00 	ms
      mt9p017_i2c_write(mt9p017_client->addr,0x4018,0x03FD,2);	//6.00 	dB
      mt9p017_i2c_write(mt9p017_client->addr,0x401A,0x0549,2);	//25.00 	ms
      mt9p017_i2c_write(mt9p017_client->addr,0x401C,0x0331,2);	//10.80 	dB
      mt9p017_i2c_write(mt9p017_client->addr,0x401E,0x02FE,2);	//42.00 	ms
      mt9p017_i2c_write(mt9p017_client->addr,0x4020,0x052F,2);	//16.50 	dB
      mt9p017_i2c_write(mt9p017_client->addr,0x02A4,0x00C0,2);	//VADJ_SENS_1_2
      mt9p017_i2c_write(mt9p017_client->addr,0x0383,0x02,1);	//FPSTYPE_MONI
      mt9p017_i2c_write(mt9p017_client->addr,0x0012,0x01,1);	// MONI_REFRESH_F
			break;

		case CAMERA_CAMCORD_MODE_FPS_FOR_DELIVERING:
      mt9p017_i2c_write(mt9p017_client->addr,0x4016,0x1CFF,2); //10.00   ms
      mt9p017_i2c_write(mt9p017_client->addr,0x4018,0x03FD,2); //6.00  dB
      mt9p017_i2c_write(mt9p017_client->addr,0x401A,0x0657,2); //30.00   ms
      mt9p017_i2c_write(mt9p017_client->addr,0x401C,0x0331,2); //10.80   dB
      mt9p017_i2c_write(mt9p017_client->addr,0x401E,0x048C,2); //66.00   ms
      mt9p017_i2c_write(mt9p017_client->addr,0x4020,0x052F,2); //16.50   dB
      mt9p017_i2c_write(mt9p017_client->addr,0x02A4,0x0000,2); //VADJ_SENS_1_2
      mt9p017_i2c_write(mt9p017_client->addr,0x0383,0x03,1); //FPSTYPE_MONI
      mt9p017_i2c_write(mt9p017_client->addr,0x0012,0x01,1); // MONI_REFRESH_F
			break;
      
		default:
			printk(KERN_ERR "Incorrect preview mode value\n");
	}

  prevew_mode_status = mode;
  
  return rc;
}
#endif

static int32_t mt9p017_set_zoom(int32_t mode)
{
  int rc = 0;
  
	return rc;
  if ((mode / 3) >= MT9P017_MAX_ZOOM_STEP)
  {
    printk(KERN_ERR "mt9p017_set_zoom : invalid >%d %d \n", mode, mode / 3);
    return 0;
  }
  prev_zoom_step = mode / 3;
  
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0032, zoom_table[prev_zoom_step], WORD_LEN);  // EZOOM_MAG
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0034, zoom_offset_x, WORD_LEN);  // OFFSET_X
  if (rc < 0)
    return rc;
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0036, zoom_offset_y, WORD_LEN);  // OFFSET_Y
  if (rc < 0)
    return rc;


  printk(KERN_ERR "mt9p017_set_zoom : >%d %d \n", mode, mode / 3);

  return rc;
}

static int mt9p017_reset(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_reset, "mt9p017");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_reset, value);
	else{
		printk("mt9p017_reset: reset gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_reset);
	return rc;
}

static int mt9p017_pwdn(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_pwd, "mt9p017");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_pwd, value);
	else{
		printk("mt9p017_pwdn: pwdn gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_pwd);
	return rc;
}

static int mt9p017_init_sensor(const struct msm_camera_sensor_info *data)
{
	int rc;
	int nNum = 0;
#ifdef IMAGE_TUNNING
	int check_v = 0x00, read_v = 0x00;
  	common_reg_list_type* pstRegisterList = NULL, *ptr_list;
	int loop;
#endif
  //RESET, PWDN Low
	rc = mt9p017_reset(data, 0);
	if (rc < 0) {
		printk("reset failed!\n");
		goto init_fail;
	}
    rc = mt9p017_pwdn(data, 0);
	if (rc < 0) {
		printk("pwdn failed!\n");
		goto init_fail;
    }
    
	rc = data->pdata->camera_power_on();
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:failed to power on!\n", __func__);
		return rc;
	}

	printk("mt9p017_sensor_power_enable\n");	
	mdelay(5);//typical 500ms
	
	/* Input MCLK = 24MHz */
	msm_camio_clk_rate_set(24000000);
	msm_camio_camif_pad_reg_reset();

	mdelay(1);//70 extclks
	printk("msm_camio_camif_pad_reg_reset\n");	

	rc = mt9p017_reset(data,1);
	if (rc < 0) {
		printk("reset failed!\n");
		goto init_fail;
	}
    mdelay(5);  // 100 EXTCLKs
    

	/*pll register write*/
	rc = mt9p017_reg_init();
	if (rc < 0) {
		for(nNum = 0; nNum<5; nNum++)
		{
		 msleep(2);
			printk(KERN_ERR "[ERROR]%s:Set initial register error! retry~! \n", __func__);
			rc = mt9p017_reg_init();
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
#if 0
  /* preview addition code */
  mt9p017_reg_preview_addition();

  /* move to here */
  rc = mt9p017_i2c_write(mt9p017_client->addr, 0x0009, 0x1F, BYTE_LEN); // INCK_SET          : INCK Free  (26.66667MHz)   
  if (rc < 0)
    return rc;

	mdelay(16);  // T3+T4
#endif
#ifdef IMAGE_TUNNING
  common_register_init(COMMON_REG_MEM_VAR4, &pstRegisterList);
  if (!pstRegisterList)
  {
    rc = mt9p017_reg_tuning();
  }
  else
  {
    ptr_list = pstRegisterList;
  
    for (loop = 0; loop < ptr_list->num_regs; loop++)
    {
      if (ptr_list->list_regs[loop].mem_var4.addr == 0xFFFF)
      {
        mdelay(ptr_list->list_regs[loop].mem_var4.vals.val32);
        continue;
      }
			
			if (ptr_list->list_regs[loop].mem_var4.addr == 0xFFFE)
			{
				check_v = ptr_list->list_regs[loop].mem_var4.len;
				for (i=0; i<10; i++)
				{ 							
					mt9p017_i2c_read(mt9p017_client->addr, ptr_list->list_regs[loop].mem_var4.vals.val32, &read_v, BYTE_LEN); // SEQ_STATE
					if(read_v == check_v)
					{
						printk("5M Aptina Sensor check register [0x%x] complete!\n", ptr_list->list_regs[loop].mem_var4.vals.val32);
						break;
					}
					printk("5M Aptina Sensor checking register = 0x%x, cur_val = 0x%x, tar_val = 0x%x......\n", ptr_list->list_regs[loop].mem_var4.vals.val32, read_v, check_v);
					mdelay(10);
				}
				continue;
			}
      mt9p017_i2c_write(mt9p017_client->addr, ptr_list->list_regs[loop].mem_var4.addr, ptr_list->list_regs[loop].mem_var4.vals.val32, ptr_list->list_regs[loop].mem_var4.len);
    }
  }
  
  if (pstRegisterList)
    kfree(pstRegisterList);
#else
	/*tuning register write*/
	rc = mt9p017_reg_tuning();
	if (rc < 0) {
		for(nNum = 0; nNum<5 ;nNum++)
		{
		  msleep(2);
			printk(KERN_ERR "[ERROR]%s:Set initial register error! retry~! \n", __func__);
			rc = mt9p017_reg_tuning();
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
#endif

  is_from_capture = 1;
  prev_scene_mode = CAMERA_SCENE_AUTO;
  prevew_mode_status = CAMERA_MODE_DEFAULT;
  prev_zoom_step = 0;
  
	return rc;

init_fail:
	printk("mt9p017: mt9p017_sensor_init failed\n");
	kfree(mt9p017_ctrl);
	return rc;  
}

static int mt9p017_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	int rc;

	CDBG("init entry \n");

	rc = mt9p017_init_sensor(data);
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

int mt9p017_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	mt9p017_ctrl = kzalloc(sizeof(struct mt9p017_ctrl), GFP_KERNEL);
	if (!mt9p017_ctrl) {
		printk(KERN_ERR "[ERROR]%s:mt9p017_init failed!\n", __func__);
		rc = -ENOMEM;
		goto init_done;
	}

	if (data)
		mt9p017_ctrl->sensordata = data;

	rc = mt9p017_sensor_init_probe(data);
	if (rc < 0) {
		printk(KERN_ERR "[ERROR]%s:mt9p017_sensor_init failed!\n", __func__);
		goto init_fail;
	}

init_done:
	return rc;

init_fail:
	kfree(mt9p017_ctrl);
	return rc;
}

int mt9p017_sensor_release(void)
{
	int rc = 0;

	if(always_on) {
		printk("always power-on camera.\n");
		return rc;
	}

	mutex_lock(&mt9p017_mutex);

	rc = mt9p017_ctrl->sensordata->pdata->camera_power_off();

	kfree(mt9p017_ctrl);

	mutex_unlock(&mt9p017_mutex);

	return rc;
}

int mt9p017_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	int rc;

	rc = copy_from_user(&cfg_data, (void *)argp,
		sizeof(struct sensor_cfg_data));

	if (rc < 0)
		return -EFAULT;

	CDBG("mt9p017_ioctl, cfgtype = %d, mode = %d\n",
		cfg_data.cfgtype, cfg_data.mode);

	mutex_lock(&mt9p017_mutex);

	switch (cfg_data.cfgtype) {
		case CFG_SET_MODE:
			rc = mt9p017_set_sensor_mode(cfg_data.mode, cfg_data.width, cfg_data.height);
			break;

		case CFG_SET_EFFECT:
			rc = mt9p017_set_effect(cfg_data.mode);
			break;

		case CFG_MOVE_FOCUS:
			rc = mt9p017_move_focus(cfg_data.cfg.focus.steps);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc = mt9p017_set_default_focus();
			break;

		case CFG_GET_AF_MAX_STEPS:
			cfg_data.max_steps = MT9P017_TOTAL_STEPS_NEAR_TO_FAR;
			if (copy_to_user((void *)argp,
					&cfg_data,
					sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_START_AF_FOCUS:
			rc = mt9p017_set_af_start(cfg_data.mode);
			break;

		case CFG_CHECK_AF_DONE:
			rc = mt9p017_check_focus(&cfg_data.mode);
			if (copy_to_user((void *)argp,
					&cfg_data,
					sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_CHECK_AF_CANCEL:
			rc = mt9p017_cancel_focus(cfg_data.mode);
			break;

		case CFG_SET_WB:
			rc = mt9p017_set_wb(cfg_data.mode);
			break;

		case CFG_SET_ANTIBANDING:
			rc= mt9p017_set_antibanding(cfg_data.mode);
			break;

		case CFG_SET_ISO:
			rc = mt9p017_set_iso(cfg_data.mode);
			break;

		case CFG_SET_SCENE:
			rc = mt9p017_set_scene_mode(cfg_data.mode);
			break;

		case CFG_SET_BRIGHTNESS:
			rc = mt9p017_set_brightness(cfg_data.mode);
			break;
#if 0 // UNIVA_TEMP
		case CFG_SET_JPEG_QUALITY:
			rc = mt9p017_set_jpeg_quality(cfg_data.cfg.jpeg_quality);
			break;
      
    case CFG_SET_JPEG_ROTATION:
      rc = mt9p017_set_jpeg_rotation(cfg_data.cfg.jpeg_rotation);
      break;

		case CFG_SET_PREVIEW_MODE:
			rc = mt9p017_set_preview_mode(cfg_data.mode);
			break;
#endif
		case CFG_SET_ZOOM:
			rc = mt9p017_set_zoom(cfg_data.mode);  // use mode field
			break;

		default:
			rc = -EINVAL;
			break;
	}

	mutex_unlock(&mt9p017_mutex);

	return rc;
}

static const struct i2c_device_id mt9p017_i2c_id[] = {
	{ "mt9p017", 0},
	{ },
};

static int mt9p017_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9p017_wait_queue);
	return 0;
}

static int mt9p017_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	mt9p017_sensorw = kzalloc(sizeof(struct mt9p017_work), GFP_KERNEL);
	if (!mt9p017_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9p017_sensorw);
	mt9p017_init_client(client);
	mt9p017_client = client;

	CDBG("mt9p017_probe succeeded!\n");

	return rc;

probe_failure:
	printk(KERN_ERR "[ERROR]%s:mt9p017_probe failed!\n", __func__);
	return rc;
}

static struct i2c_driver mt9p017_i2c_driver = {
	.id_table = mt9p017_i2c_id,
	.probe  = mt9p017_i2c_probe,
	.remove = __exit_p(mt9p017_i2c_remove),
	.driver = {
		.name = "mt9p017",
	},
};

static ssize_t mclk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
// su310 dr.ryu@lge.com 2010-06-18 LAB1_FW compile error 
//	printk("mclk_rate = %d\n", mclk_rate);
	return 0;
}

static ssize_t mclk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	sscanf(buf, "%d", &value);
// su310 dr.ryu@lge.com 2010-06-18 LAB1_FW compile error 
//	mclk_rate = value;

//	printk("mclk_rate = %d\n", mclk_rate);
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

static ssize_t mt9p017_wb_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;
	return 0;

	if (mt9p017_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	if(val < CAMERA_WB_AUTO || val > CAMERA_WB_SHADE) {
		printk("mt9p017: invalid white balance input\n");
		return 0;
	}

	rc = mt9p017_set_wb(val);
	if (rc < 0)
		printk("mt9p017: failed to set white balance\n");

	return n;
}
static DEVICE_ATTR(wb, S_IRUGO|S_IWUGO, NULL, mt9p017_wb_store);

static ssize_t mt9p017_effect_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;
	return 0;

	if (mt9p017_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	if(val < CAMERA_EFFECT_OFF || val > CAMERA_EFFECT_MAX) {
		printk("mt9p017: invalid effect input\n");
		return 0;
	}

	rc = mt9p017_set_effect(val);
	if (rc < 0)
		printk("mt9p017: failed to set effect\n");

	return n;
}
static DEVICE_ATTR(effect, S_IRUGO|S_IWUGO, NULL, mt9p017_effect_store);


static ssize_t mt9p017_zoom_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("zoom ration = %x\n", zoom_ratio);
  
	return sprintf(buf, "0x%x\n", zoom_ratio);;
}

static ssize_t mt9p017_zoom_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int value;

	sscanf(buf, "%x", &value);
	zoom_ratio = value;

	printk("ration = %x\n", zoom_ratio);

  return n;
}

static DEVICE_ATTR(zoom, S_IRUGO|S_IWUGO, mt9p017_zoom_show, mt9p017_zoom_store);

static ssize_t mt9p017_zoom_offset_x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("zoom offset x = %x\n", zoom_offset_x);
  
	return sprintf(buf, "0x%x\n", zoom_offset_x);;
}

static ssize_t mt9p017_zoom_offset_x_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int value;

	sscanf(buf, "%x", &value);
	zoom_offset_x = value;

	printk("zoom offset x = %x\n", zoom_offset_x);

  return n;
}

static DEVICE_ATTR(zoom_x, S_IRUGO|S_IWUGO, mt9p017_zoom_offset_x_show, mt9p017_zoom_offset_x_store);

static ssize_t mt9p017_zoom_offset_y_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("zoom offset y = %x\n", zoom_offset_y);
  
	return sprintf(buf, "0x%x\n", zoom_offset_y);;
}

static ssize_t mt9p017_zoom_offset_y_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int value;

	sscanf(buf, "%x", &value);
	zoom_offset_y = value;

	printk("zoom offset y = %x\n", zoom_offset_y);

  return n;
}

static DEVICE_ATTR(zoom_y, S_IRUGO|S_IWUGO, mt9p017_zoom_offset_y_show, mt9p017_zoom_offset_y_store);


static int mt9p017_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&mt9p017_i2c_driver);
	if (rc < 0 || mt9p017_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;
	}

	s->s_init = mt9p017_sensor_init;
	s->s_release = mt9p017_sensor_release;
	s->s_config  = mt9p017_sensor_config;

	rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_mclk);
	if (rc < 0) {
		printk("device_create_file error!\n");
		return rc;
	}

	rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_always_on);
	if (rc < 0) {
		printk("device_create_file error!\n");
		return rc;
	}

  rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_wb);
  if (rc < 0) {
    printk("device_create_file error!\n");
    return rc;
  }

  rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_effect);
  if (rc < 0) {
    printk("device_create_file error!\n");
    return rc;
  }
  
  rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_zoom);
  if (rc < 0) {
    printk("device_create_file error!\n");
    return rc;
  }

  rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_zoom_x);
  if (rc < 0) {
    printk("device_create_file error!\n");
    return rc;
  }
  rc = device_create_file(&mt9p017_pdev->dev, &dev_attr_zoom_y);
  if (rc < 0) {
    printk("device_create_file error!\n");
    return rc;
  }

probe_done:
	CDBG("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __mt9p017_probe(struct platform_device *pdev)
{
	mt9p017_pdev = pdev;
	return msm_camera_drv_start(pdev, mt9p017_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __mt9p017_probe,
	.driver = {
		.name = "msm_camera_mt9p017",
		.owner = THIS_MODULE,
	},
};

static int __init mt9p017_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

late_initcall(mt9p017_init);
