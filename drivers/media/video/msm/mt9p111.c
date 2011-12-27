/*
 * drivers/media/video/msm/mt9p111.c
 *
 * Aptina MT9P111 1/4-Inch 5MP System-On-Chip(SOC) CMOS Digital Image Sensor
 *
 * Copyright (C) 2011 LG Electronics.
 * Author: taiyou.kang@lge.com, 2011-03-10
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

#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#include "mt9p111.h"
#include "mt9p111_reg.h"


struct mt9p111_work
{
	struct work_struct work;
};
static struct mt9p111_work *mt9p111_sensorw;

static struct i2c_client *mt9p111_client;

struct mt9p111_ctrl
{
	const struct msm_camera_sensor_info *sensordata;
};
static struct mt9p111_ctrl *mt9p111_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(mt9p111_wait_queue);

DEFINE_MUTEX(mt9p111_mutex);

struct platform_device *mt9p111_pdev;

static int current_exposure = -1;
static int current_effect = -1;
static int current_wb = -1;
static int current_iso = -1;
static int current_brightness = -1;
static int current_zoom = -1;
static int current_scene = -1;
static int current_frame = -1;
static int current_af = -1;

int mt9p111_test_val = 0;
static int mt9p111_log_enable = 1;
#define MT9P111_LOG(fmt, args...) {if(mt9p111_log_enable) printk("[%s] " fmt, __func__, ##args);}

static int32_t mt9p111_i2c_txdata(unsigned short saddr, unsigned char *txdata, int length)
{
	int rc = 0;
	
	struct i2c_msg msg[] = 
	{
		{
			.addr = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		}
	};

	rc = i2c_transfer(mt9p111_client->adapter, msg, 1);
	if(rc < 0)
	{
		MT9P111_LOG("failed: %d\n", rc);
		return -EIO;
	}

	return 0;
}

static int32_t mt9p111_i2c_write(unsigned short saddr, unsigned short waddr, unsigned int wdata, enum mt9p111_width width)
{
	int32_t rc = -EIO;
	int int_buf[2];
	unsigned char *pbuf = ((unsigned char*)int_buf) + 2; // to align multipul 4

	pbuf[0] = (waddr & 0xFF00) >> 8;
	pbuf[1] = (waddr & 0x00FF);
  
	switch (width)
	{
		case BYTE_LEN:	
			pbuf[2] = (unsigned char)wdata;
			break;
			
		case WORD_LEN:
			pbuf[2] = (unsigned char)((wdata & 0xFF00) >> 8);
			pbuf[3] = (unsigned char)(wdata & 0x00FF);
			break;
			
		case QUAD_LEN:
			break;

		default:
			break;
	}
  
	rc = mt9p111_i2c_txdata(saddr, pbuf, width + 2);
  
	if(rc < 0)
		MT9P111_LOG("failed: %d, addr: 0x%x, val: 0x%x\n", rc, waddr, wdata);

	return rc;
}

#if defined(USE_I2C_BURST_MODE)
static unsigned char burst_i2c_buf[MAX_I2C_BURST_NUM];
static int burst_i2c_cnt = 0;
static void mt9p111_i2c_add_burst_addr(unsigned short waddr)
{
	memset(burst_i2c_buf, 0x00, MAX_I2C_BURST_NUM);
	burst_i2c_cnt = 0;
	
	burst_i2c_buf[burst_i2c_cnt++] = (waddr & 0xFF00) >> 8;
	burst_i2c_buf[burst_i2c_cnt++] = (waddr & 0x00FF);
}

static void mt9p111_i2c_add_burst_data(unsigned int wdata, enum mt9p111_width width)
{
  	if(burst_i2c_cnt < MAX_I2C_BURST_NUM)
  	{
		switch(width)
		{
			case BYTE_LEN:	
				burst_i2c_buf[burst_i2c_cnt++] = (unsigned char)wdata;
				break;
				
			case WORD_LEN:
				burst_i2c_buf[burst_i2c_cnt++] = (unsigned char)((wdata & 0xFF00) >> 8);
				burst_i2c_buf[burst_i2c_cnt++] = (unsigned char)(wdata & 0x00FF);
				break;
				
			case QUAD_LEN:
				break;

			default:
				break;
		}
  	}
	else
		MT9P111_LOG("burst_i2c_cnt exceed max value\n");
}
#endif

static int mt9p111_i2c_rxdata(unsigned short saddr, unsigned char *rxdata, int length)
{
	int rc = 0;
	struct i2c_msg msgs[] =
	{
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

	rc = i2c_transfer(mt9p111_client->adapter, msgs, 2);

	if(rc < 0)
	{
		MT9P111_LOG("failed: %d\n", rc);
		return -EIO;
	}

	return 0;
}

static int32_t mt9p111_i2c_read(unsigned short saddr, unsigned short raddr, unsigned int *rdata, enum mt9p111_width width)
{
	int32_t rc = 0;
	int int_buf;
	unsigned char *pbuf = (unsigned char*)&int_buf;

	if(!rdata)
	{
		MT9P111_LOG("failed: radata is NULL\n");
		return -EIO;
	}

	pbuf[0] = (raddr & 0xFF00) >> 8;
	pbuf[1] = (raddr & 0x00FF);

	rc = mt9p111_i2c_rxdata(saddr, pbuf, (int)width);
	
	if (rc < 0)
		return rc;

	switch (width)
	{
		case BYTE_LEN:
			*rdata = pbuf[0];
			break;
			
		case WORD_LEN:
			*rdata = pbuf[0] << 8 | pbuf[1];
			break;

		case QUAD_LEN:
			break;

		default:
			break;
	}

	if (rc < 0)
		MT9P111_LOG("failed: %d, raddr: 0x%x\n", rc, raddr);

	return rc;
}

static int32_t mt9p111_i2c_write_table(struct mt9p111_register_pair const *reg_conf_tbl, int num_of_items_in_table)
{
	int i, j;
	int32_t rc = -EIO;
	int read_val = 0;

	for(i=0; i<num_of_items_in_table; i++)
	{
		if(reg_conf_tbl->register_type == CMD_WRITE)
		{
			rc = mt9p111_i2c_write(mt9p111_client->addr, reg_conf_tbl->register_address, reg_conf_tbl->register_value, reg_conf_tbl->register_length);
		}
		else if(reg_conf_tbl->register_type == CMD_POLL)
		{
			for(j=0; j<reg_conf_tbl->register_length; j++)
			{
				rc = mt9p111_i2c_read(mt9p111_client->addr, reg_conf_tbl->register_address, &read_val, BYTE_LEN);
				if(read_val == reg_conf_tbl->register_value)
				{
					MT9P111_LOG("CMD_POLL reg[%d] OK: 0x%x, val: 0x%x, read: 0x%x\n", j, reg_conf_tbl->register_address, reg_conf_tbl->register_value, read_val);
					break;
				}
				MT9P111_LOG("CMD_POLL reg[%d]: 0x%x, val: 0x%x, read: 0x%x\n", j, reg_conf_tbl->register_address, reg_conf_tbl->register_value, read_val);
				//mdelay(10);
				msleep(10);
			}
		}
		else if(reg_conf_tbl->register_type == CMD_DELAY)
		{
			msleep(reg_conf_tbl->register_value);
		}
#if defined(USE_I2C_BURST_MODE)		
		else if(reg_conf_tbl->register_type == CMD_WRITE_BURST_S)
		{
			MT9P111_LOG("burst write: start addr: 0x%04x\n", reg_conf_tbl->register_address);
			mt9p111_i2c_add_burst_addr(reg_conf_tbl->register_address);
			while(reg_conf_tbl->register_type != CMD_WRITE_BURST_E)
			{
				mt9p111_i2c_add_burst_data(reg_conf_tbl->register_value, reg_conf_tbl->register_length);
				reg_conf_tbl++;
				i++;
			}
			mt9p111_i2c_add_burst_data(reg_conf_tbl->register_value, reg_conf_tbl->register_length);
			reg_conf_tbl++;
			i++;

			rc = mt9p111_i2c_txdata(mt9p111_client->addr, burst_i2c_buf, burst_i2c_cnt);
			MT9P111_LOG("burst write: count: %d\n", burst_i2c_cnt);
		}
#endif		
		else
		{
			MT9P111_LOG("invalide reg type: %d\n", reg_conf_tbl->register_type);
			break;
		}
			
		if(rc < 0)
		{
			MT9P111_LOG("faild: %d\n", rc);
			break;
		}
		reg_conf_tbl++;
	}

	return rc;
}

static int mt9p111_reg_init(void)
{
	int rc = 0;
	int read_v = 0;
	
	MT9P111_LOG("start\n");

#if defined(CAM_TXT_TUNING)
	rc = cam_txt_from_file(CAM_TXT_FILE(INIT_FILE));
#else
	rc = mt9p111_i2c_write_table(mt9p111_regs.init_settings, mt9p111_regs.init_settings_size);
#endif
	if(rc  < 0)
		MT9P111_LOG("init failed: %d\n", rc);

	rc = mt9p111_i2c_read(mt9p111_client->addr, 0xE023, &read_v, BYTE_LEN);
	if(rc  < 0)
		MT9P111_LOG("read(0xE023) failed: %d\n", rc);

		MT9P111_LOG("read(0xE023): 0x%02x\n", read_v);
	
	if(read_v == 0xC1)
	{
#if defined(CAM_TXT_TUNING)
		rc = cam_txt_from_file(CAM_TXT_FILE(LENS_ZONE0_FILE));
#else
		rc = mt9p111_i2c_write_table(lens_zone0_settings_array, ARRAY_SIZE(lens_zone0_settings_array));
#endif
		if(rc < 0)
			MT9P111_LOG("lens zone0 failed: %d\n", rc);
		MT9P111_LOG("lens zone0 loaded\n");
	}
	else
	{
#if defined(CAM_TXT_TUNING)
		rc = cam_txt_from_file(CAM_TXT_FILE(LENS_CHECK_FILE));
#else
		rc = mt9p111_i2c_write_table(lens_check_settings_array, ARRAY_SIZE(lens_check_settings_array));
#endif
		if (rc < 0)
			MT9P111_LOG("lens check failed: %d\n", rc);

		rc = mt9p111_i2c_read(mt9p111_client->addr, 0xE023, &read_v, BYTE_LEN);
		if (rc < 0)
			MT9P111_LOG("read(0xE023) failed: %d\n", rc);
	
		MT9P111_LOG("read(0xE023): 0x%02x\n", read_v);

		if(read_v == 0xC1)
		{
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(LENS_ZONE0_FILE));
#else
			 rc = mt9p111_i2c_write_table(lens_zone0_settings_array, ARRAY_SIZE(lens_zone0_settings_array));
#endif
			if (rc < 0)
				MT9P111_LOG("lens zone0 failed: %d\n", rc);

			MT9P111_LOG("lens zone0 loaded\n");
	}
	else
	{
#if defined(CAM_TXT_TUNING)
		rc = cam_txt_from_file(CAM_TXT_FILE(LENS_DEFAULT_FILE));
#else	
		rc = mt9p111_i2c_write_table(lens_default_settings_array, ARRAY_SIZE(lens_default_settings_array));
#endif
		if (rc < 0)
				MT9P111_LOG("lens default failed: %d\n", rc);
			
			MT9P111_LOG("lens default loaded\n");
		}
	}

#if defined(CAM_TXT_TUNING)
	rc = cam_txt_from_file(CAM_TXT_FILE(INIT2_FILE));
#else
	rc = mt9p111_i2c_write_table(mt9p111_regs.init2_settings, mt9p111_regs.init2_settings_size);
#endif
	if (rc < 0)
		MT9P111_LOG("failed: %d\n", rc);

	current_exposure = -1;
	current_effect = -1;
	current_wb = -1;
	current_iso = -1;
	current_brightness = -1;
	current_zoom = -1;
	current_scene = -1;
	current_frame = -1;
	current_af = -1;
	
	MT9P111_LOG("end\n");
	return rc;
}

static int mt9p111_reg_preview(void)
{
	int rc = 0;

	MT9P111_LOG("start\n");

#if defined(CAM_TXT_TUNING)
	rc = cam_txt_from_file(CAM_TXT_FILE(PREVIEW_FILE));
#else
	rc = mt9p111_i2c_write_table(mt9p111_regs.prev_settings, mt9p111_regs.prev_settings_size);
#endif
	if(rc  < 0)
		MT9P111_LOG("failed: %d\n", rc);

#if defined(USE_CONTINUOUS_AF)
	rc = mt9p111_i2c_write(mt9p111_client->addr, 0xB006, 0x01, BYTE_LEN); // AF_PROGRESS
	if (rc < 0)
		MT9P111_LOG("AF_PROGRESS failed: %d\n", rc);
#endif	
	
	MT9P111_LOG("end\n");
	return rc;
}

static int mt9p111_reg_snapshot(void)
{
	int rc = 0;

	MT9P111_LOG("start\n");

#if defined(CAM_TXT_TUNING)
	rc = cam_txt_from_file(CAM_TXT_FILE(SNAPSHOT_FILE));
#else
	rc = mt9p111_i2c_write_table(mt9p111_regs.snap_settings, mt9p111_regs.snap_settings_size);
#endif
	if(rc  < 0)
		MT9P111_LOG("failed: %d\n", rc);
	
	MT9P111_LOG("end\n");
	return rc;
}

static int mt9p111_snapshot_config(int width, int height)
{
	int rc = 0;
	struct mt9p111_register_pair const check_cmd[] =
	{
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	};

	MT9P111_LOG("%dx%d\n", width, height);
  
	rc = mt9p111_i2c_write(mt9p111_client->addr, 0xC8C0, width, WORD_LEN); // CAM_OUTPUT_1_IMAGE_WIDTH
	if (rc < 0)
		return rc;

	rc = mt9p111_i2c_write(mt9p111_client->addr, 0xC8C2, height, WORD_LEN); // CAM_OUTPUT_1_IMAGE_HEIGHT
	if (rc < 0)
		return rc;
  
	rc = mt9p111_i2c_write_table(check_cmd, ARRAY_SIZE(check_cmd));
	if (rc < 0)
		return rc;

	rc = mt9p111_reg_snapshot();

	MT9P111_LOG("end\n");

	return rc;
}

static int mt9p111_raw_snapshot_config(int width, int height)
{
	int rc = 0;
	struct mt9p111_register_pair const check_cmd[] =
	{
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	};

	MT9P111_LOG("%dx%d\n", width, height);
  
	rc = mt9p111_i2c_write(mt9p111_client->addr, 0xC8C0, width, WORD_LEN); // CAM_OUTPUT_1_IMAGE_WIDTH
	if (rc < 0)
		return rc;

	rc = mt9p111_i2c_write(mt9p111_client->addr, 0xC8C2, height, WORD_LEN); // CAM_OUTPUT_1_IMAGE_HEIGHT
	if (rc < 0)
		return rc;
  
	rc = mt9p111_i2c_write_table(check_cmd, ARRAY_SIZE(check_cmd));
	if (rc < 0)
		return rc;

	rc = mt9p111_reg_snapshot();

	MT9P111_LOG("end\n");

	return rc;
}

static int mt9p111_set_sensor_mode(int mode, int width, int height)
{
	int rc = 0;

	MT9P111_LOG("mode: %d, width: %d, height: %d\n", mode, width, height);

	switch(mode)
	{
		case SENSOR_PREVIEW_MODE:
			rc = mt9p111_reg_preview();
			if(rc < 0)
				MT9P111_LOG("SENSOR_PREVIEW_MODE failed: %d\n", rc);
			break;

		case SENSOR_SNAPSHOT_MODE:
			rc = mt9p111_snapshot_config(width, height);
			if(rc < 0)
				MT9P111_LOG("SENSOR_SNAPSHOT_MODE failed: %d\n", rc);
			break;
    
		case SENSOR_RAW_SNAPSHOT_MODE:
			rc = mt9p111_raw_snapshot_config(width, height);
			if (rc < 0)
				MT9P111_LOG("SENSOR_RAW_SNAPSHOT_MODE failed: %d\n", rc);
			break;

		default:
			return -EINVAL;
	}

	return rc;
}

static int mt9p111_check_af(void)
{
	int8_t rc = 0;
	int read_v;

	MT9P111_LOG("start\n");
	
	rc = mt9p111_i2c_read(mt9p111_client->addr, 0xB000, &read_v, WORD_LEN); // SEQ_STATE
	if(rc < 0)
		MT9P111_LOG("read_v read failed: %d\n", rc);
	MT9P111_LOG("read_v: 0x%x\n", read_v);
	
	if(!(read_v & 0x0010)) //[4]bit : 0=not finished, 1 = finished
	{
		rc = MT9P111_AF_SEARCHING;
		MT9P111_LOG("MT9P111_AF_SEARCHING\n");
	}
	else if(read_v & 0x8000) //[15]bit : 1=fail, 0=success
	{
		rc = MT9P111_AF_FAIL;
		MT9P111_LOG("MT9P111_AF_FAIL\n");
	}
	else
	{
		rc = MT9P111_AF_SUCCESS;
		MT9P111_LOG("MT9P111_AF_SUCCESS\n");
	}

	return rc;
}

static int mt9p111_af_polling(void)
{
	int i = 0;
	int af_status = MT9P111_AF_FAIL;

	MT9P111_LOG("start\n");
	
	while((af_status != MT9P111_AF_SUCCESS) && (i<10))
	{
		af_status = mt9p111_check_af();

		if(af_status == MT9P111_AF_SUCCESS)
		{
			MT9P111_LOG("af success(%d)\n", i);
			break;
		}
		else if(af_status == MT9P111_AF_SEARCHING)
		{
			MT9P111_LOG("af searching(%d)\n", i);
		}
		else if(af_status == MT9P111_AF_FAIL)
		{
			MT9P111_LOG("af failed(%d)\n", i);
			break;
		}
		
		i++;
		msleep(200);
	}

	return af_status;
}

static int mt9p111_set_af(int mode)
{
	int rc = 0;
#if 1 // Auto focus workaround
	struct mt9p111_register_pair const af_cmd[] =
	{
		{CMD_WRITE, 0xB006, 0x01, BYTE_LEN}, // AF_PROGRESS
		{CMD_POLL, 0xB006, 0x00, 100},
	};
#endif

	MT9P111_LOG("mode: %d\n", mode);

	switch(mode)
	{	
	 	case FOCUS_NORMAL:
		case FOCUS_MANUAL:
			break;

		case FOCUS_MACRO:
		case FOCUS_AUTO:
#if 1 // Auto focus workaround
			rc = mt9p111_i2c_write_table(af_cmd, ARRAY_SIZE(af_cmd));
			if (rc < 0)
				return rc;
#else
			rc = mt9p111_i2c_write(mt9p111_client->addr, 0x098E, 0xB006, WORD_LEN); // LOGICAL_ADDRESS_ACCESS [AF_PROGRESS]
			if (rc < 0)
				return rc;

			rc = mt9p111_i2c_write(mt9p111_client->addr, 0xB006, 0x01, BYTE_LEN); // AF_PROGRESS
			if (rc < 0)
				return rc;
#endif			
			
			rc = mt9p111_af_polling();
			if(rc == MT9P111_AF_SUCCESS)
				rc = 0;
			else
				rc = -1;
			
			break;
		default:
			return -EINVAL;
	}

	return rc;
}

static int mt9p111_cancel_af(int mode)
{
	int rc = 0;

	MT9P111_LOG("mode: %d\n", mode);
	
	return rc;
}

#if defined(USE_CONTINUOUS_AF)
static int mt9p111_get_continuous_af(int *status)
{
	int rc = 0;
	int check_v = 1;
	int read_v = 0;
	static int caf_fail_count = 0;

	rc = mt9p111_i2c_read(mt9p111_client->addr, 0xB006, &check_v, BYTE_LEN); // SEQ_STATE
	if(rc < 0)
		MT9P111_LOG("check_v read failed: %d\n", rc);

#if 1 // CAF recovery workaround
	rc = mt9p111_i2c_read(mt9p111_client->addr, 0xB000, &read_v, WORD_LEN); // SEQ_STATE
	if(rc < 0)
		MT9P111_LOG("read_v read failed: %d\n", rc);

	if(read_v == 0) caf_fail_count++;

	if(caf_fail_count > 10)
	{
		rc = mt9p111_i2c_write(mt9p111_client->addr, 0xB006, 0x01, BYTE_LEN); // AF_PROGRESS
		if (rc < 0)
			MT9P111_LOG("AF_PROGRESS failed: %d\n", rc);
		MT9P111_LOG("CAF recovery activated\n");
		caf_fail_count = 0;
	}
#else
	read_v = 0;
	caf_fail_count = 0;
#endif	
	//MT9P111_LOG("check_v: 0x%x, read_v: 0x%x\n", check_v, read_v);

	if(check_v == 0)
		*status = 2;
	else
		*status = 1;
	
	return rc;
}
#endif

static int mt9p111_set_default_focus(int mode)
{
	int rc = 0;

	MT9P111_LOG("mode: %d\n", mode);
	
	return rc;
}

static int mt9p111_set_move_focus(int step)
{
	int rc = 0;

	MT9P111_LOG("step: %d\n", step);


	if(step < FOCUS_MANUAL_MIN)
	{
		MT9P111_LOG("invalid step: %d\n", step);
		return -EINVAL;
	}

	if(step > FOCUS_MANUAL_MAX)
	{
		MT9P111_LOG("invalid step: %d\n", step);
		return -EINVAL;
	}

#if defined(CAM_TXT_TUNING)
	if(step == FOCUS_MANUAL_POS_0)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_0_FILE));
	else if(step == FOCUS_MANUAL_POS_1)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_1_FILE));
	else if(step == FOCUS_MANUAL_POS_2)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_2_FILE));
	else if(step == FOCUS_MANUAL_POS_3)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_3_FILE));
	else if(step == FOCUS_MANUAL_POS_4)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_4_FILE));
	else if(step == FOCUS_MANUAL_POS_5)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_5_FILE));
	else if(step == FOCUS_MANUAL_POS_6)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_6_FILE));
	else if(step == FOCUS_MANUAL_POS_7)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_7_FILE));
	else if(step == FOCUS_MANUAL_POS_8)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_8_FILE));
	else if(step == FOCUS_MANUAL_POS_9)
		rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MANUAL_9_FILE));
#else
	rc = mt9p111_i2c_write_table(af_manual_settings_array[step], ARRAY_SIZE(af_manual_settings_array[step]));
#endif
	if(rc < 0)
		MT9P111_LOG("failed: %d\n", rc);	
	
	return rc;
}

static int mt9p111_af_mode(int mode)
{
	int rc = 0;

	if(current_af == mode)
	{
		MT9P111_LOG("af mode already applied: %d\n", mode);
		return rc;
	}
#if defined(USE_CONTINUOUS_AF)	
	else if(current_af == -1 && mode == FOCUS_CONTINUOUS_CAMERA)
#else
	else if(current_af == -1 && mode == FOCUS_AUTO)
#endif
	{
		current_af = mode;
		MT9P111_LOG("First af mode after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_af = mode;

	MT9P111_LOG("mode: %d\n", mode);

	switch(mode)
	{	
		case FOCUS_MACRO:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_MACRO_FILE));
#else		
			rc = mt9p111_i2c_write_table(mt9p111_regs.af_macro_settings, mt9p111_regs.af_macro_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("FOCUS_MACRO failed: %d\n", rc);
			break;

		case FOCUS_CONTINUOUS_VIDEO:
		case FOCUS_NORMAL:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_CONTINUOUS_VIDEO_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.af_continuous_video_settings, mt9p111_regs.af_continuous_video_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("FOCUS_CONTINUOUS_VIDEO failed: %d\n", rc);
			break;

		case FOCUS_AUTO:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_AUTO_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.af_auto_settings, mt9p111_regs.af_auto_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("FOCUS_AUTO failed: %d\n", rc);
			break;

		case FOCUS_CONTINUOUS_CAMERA:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(FOCUS_CONTINUOUS_CAMERA_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.af_continuous_camera_settings, mt9p111_regs.af_continuous_camera_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("FOCUS_CONTINUOUS_CAMERA failed: %d\n", rc);
			break;
			
		default:
			return -EINVAL;
	}		

	return rc;
}

static int mt9p111_focus_rect(int mode)
{
	int rc = 0;

	MT9P111_LOG("mode: %d\n", mode);
	
	return rc;
}

static int mt9p111_set_scene(int mode)
{
	int rc = 0;

	if(current_scene == mode)
	{
		MT9P111_LOG("scene already applied: %d\n", mode);
		return rc;
	}
	else if(current_scene == -1 && mode == SCENE_AUTO)
	{
		current_scene = mode;
		MT9P111_LOG("First scene after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_scene = mode;

	MT9P111_LOG("mode: %d\n", mode);
	
	switch(mode)
	{
	 	case SCENE_AUTO:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_AUTO_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_auto_settings, mt9p111_regs.scene_auto_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_AUTO failed: %d\n", rc);
			break;

	 	case SCENE_LANDSCAPE:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_LANDSCAPE_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_landscape_settings, mt9p111_regs.scene_landscape_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_LANDSCAPE failed: %d\n", rc);
			break;

	 	case SCENE_SUNSET:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_SUNSET_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_sunset_settings, mt9p111_regs.scene_sunset_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_SUNSET failed: %d\n", rc);
			break;

	 	case SCENE_NIGHT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_NIGHT_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_night_settings, mt9p111_regs.scene_night_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_NIGHT failed: %d\n", rc);
			break;

	 	case SCENE_PORTRAIT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_PORTRAIT_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_portrait_settings, mt9p111_regs.scene_portrait_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_PORTRAIT failed: %d\n", rc);
			break;

	 	case SCENE_SPORTS:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(SCENE_SPORTS_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.scene_sports_settings, mt9p111_regs.scene_sports_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("SCENE_SPORTS failed: %d\n", rc);
			break;
			
		default:
			return -EINVAL;
	}

	return rc;
}

static int mt9p111_set_exposure(int mode)
{
	int rc = 0;

	if(current_exposure == mode)
	{
		MT9P111_LOG("exposure already applied: %d\n", mode);
		return rc;
	}
	else if(current_exposure == -1 && mode == EXPOSURE_NORMAL)
	{
		current_exposure = mode;
		MT9P111_LOG("First exposure after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_exposure = mode;

	MT9P111_LOG("mode: %d\n", mode);
	
	switch(mode)
	{
	 	case EXPOSURE_NORMAL:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EXPOSURE_NORMAL_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.exposure_normal_settings, mt9p111_regs.exposure_normal_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EXPOSURE_NORMAL failed: %d\n", rc);
			break;

		case EXPOSURE_SPOT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EXPOSURE_SPOT_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.exposure_spot_settings, mt9p111_regs.exposure_spot_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EXPOSURE_SPOT failed: %d\n", rc);
			break;

		case EXPOSURE_AVG:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EXPOSURE_AVG_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.exposure_avg_settings, mt9p111_regs.exposure_avg_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EXPOSURE_AVG failed: %d\n", rc);
			break;
		default:
			return -EINVAL;
	}

	return rc;
}

static int mt9p111_set_frame(int mode)
{
	int rc = 0;

	if(current_frame == mode)
	{
		MT9P111_LOG("frame mode already applied: %d\n", mode);
		return rc;
	}
	else if(current_frame == -1 && mode == FPS_VARIABLE)
	{
		current_frame = mode;
		MT9P111_LOG("First frame mode after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_frame = mode;
	
	MT9P111_LOG("mode: %d\n", mode);

	if(mode == FPS_VARIABLE)
	{
#if defined(CAM_TXT_TUNING)
		rc = cam_txt_from_file(CAM_TXT_FILE(FPS_VARIABLE_FILE));
#else
		rc = mt9p111_i2c_write_table(variable_frame_settings_array, ARRAY_SIZE(variable_frame_settings_array));
#endif
	}
	else if(mode == FPS_FIXED)
	{
#if defined(CAM_TXT_TUNING)
		rc = cam_txt_from_file(CAM_TXT_FILE(FPS_FIXED_FILE));
#else
		rc = mt9p111_i2c_write_table(fixed_frame_settings_array, ARRAY_SIZE(fixed_frame_settings_array));
#endif
	}

	if(rc < 0)
		MT9P111_LOG("set_frame failed: %d\n", rc);	
	
	return rc;
}

static int mt9p111_set_effect(int effect)
{
	int rc = 0;

	if(current_effect == effect)
	{
		MT9P111_LOG("effect already applied: %d\n", effect);
		return rc;
	}
	else if(current_effect == -1 && effect == EFFECT_OFF)
	{
		current_effect = effect;
		MT9P111_LOG("First effect after init (skip): %d\n", effect);
		return rc;
	}
	else
		current_effect = effect;

	MT9P111_LOG("effect: %d\n", effect);
  
	switch(effect)
	{
		case EFFECT_OFF:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_OFF_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_off_settings, mt9p111_regs.effect_off_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_OFF failed: %d\n", rc);
			break;

		case EFFECT_GRAY:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_GRAY_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_gray_settings, mt9p111_regs.effect_gray_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_GRAY failed: %d\n", rc);
			break;

		case EFFECT_NEGATIVE:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_NEGATIVE_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_negative_settings, mt9p111_regs.effect_negative_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_NEGATIVE failed: %d\n", rc);
			break;

		case EFFECT_SOLARIZE:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_SOLARIZE_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_solarize_settings, mt9p111_regs.effect_solarize_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_SOLARIZE failed: %d\n", rc);
			break;

		case EFFECT_SEPIA:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_SEPIA_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_sepia_settings, mt9p111_regs.effect_sepia_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_SEPIA failed: %d\n", rc);
			break;

		case EFFECT_GREEN:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_GREEN_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_green_settings, mt9p111_regs.effect_green_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_GREEN failed: %d\n", rc);
			break;

		case EFFECT_COOL:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_COOL_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_cool_settings, mt9p111_regs.effect_cool_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_COOL failed: %d\n", rc);
			break;

		case EFFECT_YELLOW:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_YELLOW_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_yellow_settings, mt9p111_regs.effect_yellow_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_YELLOW failed: %d\n", rc);
			break;

		case EFFECT_AQUA:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_AQUA_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_aqua_settings, mt9p111_regs.effect_aqua_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_AQUA failed: %d\n", rc);
			break;

		case EFFECT_PURPLE:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_PURPLE_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_purple_settings, mt9p111_regs.effect_purple_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_PURPLE failed: %d\n", rc);
			break;

		case EFFECT_RED:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_RED_FILE));
#else				
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_red_settings, mt9p111_regs.effect_red_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_RED failed: %d\n", rc);
			break;

		case EFFECT_PINK:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(EFFECT_PINK_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.effect_pink_settings, mt9p111_regs.effect_pink_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("EFFECT_PINK failed: %d\n", rc);
			break;

		default:
			return -EINVAL;
	}

	return rc;
}

static int mt9p111_set_wb(int mode)
{
	int rc = 0;

	if(current_wb == mode)
	{
		MT9P111_LOG("wb already applied: %d\n", mode);
		return rc;
	}
	else if(current_wb == -1 && mode == WB_AUTO)
	{
		current_wb = mode;
		MT9P111_LOG("First wb after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_wb = mode;

	MT9P111_LOG("mode: %d\n", mode);

	switch(mode)
	{
		case WB_AUTO:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(WB_AUTO_FILE));
#else
			rc = mt9p111_i2c_write_table(mt9p111_regs.wb_auto_settings, mt9p111_regs.wb_auto_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("WB_AUTO failed: %d\n", rc);
			break;

		case WB_INCANDESCENT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(WB_INCANDESCENT_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.wb_incandescent_settings, mt9p111_regs.wb_incandescent_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("WB_INCANDESCENT failed: %d\n", rc);
			break;

		case WB_FLUORESCENT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(WB_FLUORESCENT_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.wb_fluorescent_settings, mt9p111_regs.wb_fluorescent_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("WB_FLUORESCENT failed: %d\n", rc);
			break;
			
		case WB_DAYLIGHT:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(WB_DAYLIGHT_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.wb_daylight_settings, mt9p111_regs.wb_daylight_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("WB_DAYLIGHT failed: %d\n", rc);
			break;

		case WB_CLOUDY:
#if defined(CAM_TXT_TUNING)
			rc = cam_txt_from_file(CAM_TXT_FILE(WB_CLOUDY_FILE));
#else			
			rc = mt9p111_i2c_write_table(mt9p111_regs.wb_cloudy_settings, mt9p111_regs.wb_cloudy_settings_size);
#endif
			if(rc < 0)
				MT9P111_LOG("WB_CLOUDY failed: %d\n", rc);
			break;

		default:
			return -EINVAL;
	}
	
	return rc;
}

static int mt9p111_set_antibanding(int mode)
{
	int rc = 0;
	
	MT9P111_LOG("mode: %d\n", mode);

#if 0
	switch(mode)
	{
		case CAMERA_ANTIBANDING_OFF:
			break;

		case CAMERA_ANTIBANDING_60HZ:
			break;

		case CAMERA_ANTIBANDING_50HZ:
			break;

		case CAMERA_ANTIBANDING_AUTO:
			break;

		case CAMERA_MAX_ANTIBANDING:
			break;

		default:
			return -EINVAL;
	}
#endif	

	return rc;	
}

static int mt9p111_set_iso(int iso)
{
	int32_t rc = 0;

	if(current_iso == iso)
	{
		MT9P111_LOG("iso already applied: %d\n", iso);
		return rc;
	}
	else if(current_iso == -1 && iso == ISO_AUTO)
	{
		current_iso = iso;
		MT9P111_LOG("First iso after init (skip): %d\n", iso);
		return rc;
	}
	else
		current_iso = iso;

	MT9P111_LOG("iso: %d\n", iso);

	if(iso < ISO_AUTO)
	{
		MT9P111_LOG("invalid level: %d\n", iso);
		return -EINVAL;
	}

	if(iso > ISO_MAX)
	{
		MT9P111_LOG("invalid level: %d\n", iso);
		return -EINVAL;
	}

#if defined(CAM_TXT_TUNING)
	if(iso == ISO_AUTO)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_AUTO_FILE));
	else if(iso == ISO_50)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_50_FILE));
	else if(iso == ISO_100)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_100_FILE));
	else if(iso == ISO_200)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_200_FILE));
	else if(iso == ISO_400)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_400_FILE));
	else if(iso == ISO_800)
		rc = cam_txt_from_file(CAM_TXT_FILE(ISO_800_FILE));
#else	
	rc = mt9p111_i2c_write_table(iso_settings_array[iso], ARRAY_SIZE(iso_settings_array[iso]));
#endif
	if(rc < 0)
		MT9P111_LOG("failed: %d\n", rc);

	return rc;
}

static int mt9p111_get_iso(int mode)
{
	int rc = 0;

	MT9P111_LOG("mode: %d\n", mode);	

	return rc;
}

static int mt9p111_get_snapshot(int mode)
{
	int rc = 0;

	MT9P111_LOG("mode: %d\n", mode);	

	return rc;
}

static int32_t mt9p111_set_brightness(int brightness)
{
	int32_t rc=0;

	if(current_brightness == brightness)
	{
		MT9P111_LOG("brightness already applied: %d\n", brightness);
		return rc;
	}
	else if(current_brightness == -1 && brightness == BRIGHTNESS_NORMAL)
	{
		current_brightness = brightness;
		MT9P111_LOG("First brightness after init (skip): %d\n", brightness);
		return rc;
	}
	else
		current_brightness = brightness;

	MT9P111_LOG("brightness: %d\n", brightness);

	if(brightness < BRIGHTNESS_MIN)
	{
		MT9P111_LOG("invalid level: %d\n", brightness);
		return -EINVAL;
	}

	if(brightness > BRIGHTNESS_MAX)
	{
		MT9P111_LOG("invalid level: %d\n", brightness);
		return -EINVAL;
	}

#if defined(CAM_TXT_TUNING)
	if(brightness == BRIGHTNESS_M5)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_M5_FILE));
	else if(brightness == BRIGHTNESS_M4)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_M4_FILE));
	else if(brightness == BRIGHTNESS_M3)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_M3_FILE));
	else if(brightness == BRIGHTNESS_M2)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_M2_FILE));
	else if(brightness == BRIGHTNESS_M1)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_M1_FILE));
	else if(brightness == BRIGHTNESS_NORMAL)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_NORMAL_FILE));
	else if(brightness == BRIGHTNESS_P1)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_P1_FILE));
	else if(brightness == BRIGHTNESS_P2)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_P2_FILE));
	else if(brightness == BRIGHTNESS_P3)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_P3_FILE));
	else if(brightness == BRIGHTNESS_P4)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_P4_FILE));
	else if(brightness == BRIGHTNESS_P5)
		rc = cam_txt_from_file(CAM_TXT_FILE(BRIGHTNESS_P5_FILE));
#else
	rc = mt9p111_i2c_write_table(brightness_settings_array[brightness], ARRAY_SIZE(brightness_settings_array[brightness]));
#endif
	if(rc < 0)
		MT9P111_LOG("failed: %d\n", rc);
	
	return rc;
}

static int32_t mt9p111_set_zoom(int32_t mode)
{
	int32_t rc = 0;

	if(current_zoom == mode)
	{
		MT9P111_LOG("zoom already applied: %d\n", mode);
		return rc;
	}
	else if(current_zoom == -1 && mode == ZOOM_MIN)
	{
		current_zoom = mode;
		MT9P111_LOG("First zoom after init (skip): %d\n", mode);
		return rc;
	}
	else
		current_zoom = mode;

  	MT9P111_LOG("zoom: %d\n", mode);

	if(mode < ZOOM_MIN)
	{
		MT9P111_LOG("invalid level: %d\n", mode);
		return -EINVAL;
	}

	if(mode > ZOOM_MAX)
	{
		MT9P111_LOG("invalid level: %d\n", mode);
		return -EINVAL;
	}

#if defined(CAM_TXT_TUNING)
	if(mode == ZOOM_1P0)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_1P0_FILE));
	else if(mode == ZOOM_1P2)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_1P2_FILE));
	else if(mode == ZOOM_1P4)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_1P4_FILE));
	else if(mode == ZOOM_1P6)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_1P6_FILE));
	else if(mode == ZOOM_1P8)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_1P8_FILE));
	else if(mode == ZOOM_2P0)
		rc = cam_txt_from_file(CAM_TXT_FILE(ZOOM_2P0_FILE));
#else
	rc = mt9p111_i2c_write_table(zoom_settings_array[mode], ARRAY_SIZE(zoom_settings_array[mode]));
#endif
	if(rc < 0)
		MT9P111_LOG("failed: %d\n", rc);

	return rc;
}

static int mt9p111_reset(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_reset, "mt9p111");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_reset, value);
	else
	{
		MT9P111_LOG("failed: %d\n", rc);
		return rc;
	}

	gpio_free(dev->sensor_reset);
	return rc;
}

static int mt9p111_pwdn(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_pwd, "mt9p111");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_pwd, value);
	else
	{
		MT9P111_LOG("failed: %d\n", rc);
		return rc;
	}

	gpio_free(dev->sensor_pwd);
	return rc;
}

#if 0 // MANUAL_FLICKER_WA
extern char lge_bd_mcc[4];

char *flicker_50hz_country[] =
{
	"AU", // Australia
	"CN", // China
	"XX" // End of list
};

char *flicker_60hz_country[] =
{
	"BR", // Brazil
	"MX", // Mexico
	"XX" // End of list
};

static int mt9p111_check_country(char **country)
{
	int i = 0;
	int ret = 0;
	
	do
	{
		if(strncmp(lge_bd_mcc, country[i], 2) == 0)
		{
			ret = 1;
			break;
		}
		i++;
	} while(strncmp("XX", country[i], 2) != 0);

	return ret;
}
#endif

static void mt9p111_set_flicker(void)
{
#if 0 // MANUAL_FLICKER_WA
	if(mt9p111_check_country(flicker_60hz_country))
	{
		mt9p111_regs.init2_settings = init2_flicker_60hz_mode_settings_array;
		mt9p111_regs.init2_settings_size = ARRAY_SIZE(init2_flicker_60hz_mode_settings_array);
		MT9P111_LOG("%s: 60hz\n", lge_bd_mcc);
	}
	else if(mt9p111_check_country(flicker_50hz_country))
	{
		mt9p111_regs.init2_settings = init2_flicker_50hz_mode_settings_array;
		mt9p111_regs.init2_settings_size = ARRAY_SIZE(init2_flicker_50hz_mode_settings_array);
		MT9P111_LOG("%s: 50hz\n", lge_bd_mcc);
	}
	else // Not defined: auto
	{
		mt9p111_regs.init2_settings = init2_mode_settings_array;
		mt9p111_regs.init2_settings_size = ARRAY_SIZE(init2_mode_settings_array);
		MT9P111_LOG("%s: auto flicker\n", lge_bd_mcc);
	}
#else
	MT9P111_LOG("default: auto flicker\n");
#endif	
}

static int mt9p111_init_sensor(const struct msm_camera_sensor_info *data)
{
	int rc;
	int nNum = 0;

	rc = mt9p111_reset(data, 0);
	if(rc < 0)
	{
		MT9P111_LOG("reset failed: %d\n", rc);
		goto init_fail;
	}
	
	rc = mt9p111_pwdn(data, 0);
	if(rc < 0)
	{
		MT9P111_LOG("pwdn failed: %d\n", rc);
		goto init_fail;
	}

	rc = data->pdata->camera_power_on();
	
	if(rc < 0)
	{
		MT9P111_LOG("power on failed: %d\n", rc);
		return rc;
	}

	MT9P111_LOG("power enabled\n");	
	mdelay(5); //typical 500ms
	
	/* Input MCLK = 24MHz */
	msm_camio_clk_rate_set(24000000);
	msm_camio_camif_pad_reg_reset();

	mdelay(1); //70 extclks
	MT9P111_LOG("msm_camio_camif_pad_reg_reset\n");	

	rc = mt9p111_reset(data, 1);
	if(rc < 0)
	{
		MT9P111_LOG("reset failed: %d\n", rc);
		goto init_fail;
	}
	mdelay(5); // 100 EXTCLKs
    
	mt9p111_set_flicker();
    
	mutex_lock(&mt9p111_mutex);
	
	/*pll register write*/
	rc = mt9p111_reg_init();
	
	if (rc < 0)
	{
		for(nNum = 0; nNum<5; nNum++)
		{
			msleep(2);
			MT9P111_LOG("reg_init error, retry: %d\n", nNum);
			rc = mt9p111_reg_init();
			if(rc < 0)
			{
				nNum++;
			}
			else
			{
				MT9P111_LOG("retry success: %d\n", nNum);
				break;
			}
		}
	}

	mutex_unlock(&mt9p111_mutex);

	return rc;

init_fail:
	MT9P111_LOG("sensor_init failed: %d\n", rc);
	kfree(mt9p111_ctrl);
	return rc;  
}

static int mt9p111_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	int rc;
	//unsigned int model_id = 0;

	MT9P111_LOG("start\n");

	rc = mt9p111_init_sensor(data);
	if(rc < 0)
	{
		MT9P111_LOG("init_sensor failed: %d\n", rc);
		goto init_probe_fail;
	}

	//rc  = mt9p111_i2c_read(mt9p111_client->addr, 0x0000, &model_id, WORD_LEN);
	//if(rc < 0)
	//{
	//	goto init_probe_fail;
	//}

	//MT9P111_LOG("model_id: %x\n", model_id);

	return rc;

init_probe_fail:
	MT9P111_LOG("sensor_init_probe failed: %d\n", rc);
	return rc;
}

int mt9p111_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc = 0;

	mt9p111_ctrl = (struct mt9p111_ctrl *)kzalloc(sizeof(struct mt9p111_ctrl), GFP_KERNEL);
	if(!mt9p111_ctrl)
	{
		MT9P111_LOG("mt9p111_ctrl is NULL\n");
		rc = -ENOMEM;
		goto init_done;
	}

	if(data)
		mt9p111_ctrl->sensordata = data;

	rc = mt9p111_sensor_init_probe(data);
	if(rc < 0)
	{
		MT9P111_LOG("sensor_init failed: %d\n", rc);
		goto init_fail;
	}

init_done:
	return rc;

init_fail:
	kfree(mt9p111_ctrl);
	return rc;
}

int mt9p111_sensor_release(void)
{
	int rc = 0;

	mutex_lock(&mt9p111_mutex);

	rc = mt9p111_reset(mt9p111_ctrl->sensordata, 0);
	if(rc < 0)
		MT9P111_LOG("reset failed: %d\n", rc);
	mdelay(1);

	rc = mt9p111_ctrl->sensordata->pdata->camera_power_off();
	kfree(mt9p111_ctrl);

	mutex_unlock(&mt9p111_mutex);

	if(rc < 0)
		MT9P111_LOG("power off failed: %d\n", rc);

	return rc;
}

int mt9p111_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;
	int rc;

	rc = copy_from_user(&cfg_data, (void *)argp, sizeof(struct sensor_cfg_data));

	if(rc < 0)
	{
		MT9P111_LOG("copy_from_user failed: %d\n", rc);
		return -EFAULT;
	}

#if defined(USE_CONTINUOUS_AF)
	if(cfg_data.cfgtype != CFG_CHECK_AF_DONE)
#endif
		MT9P111_LOG("cfgtype = %d, mode = %d\n", cfg_data.cfgtype, cfg_data.mode);

	mutex_lock(&mt9p111_mutex);

	switch(cfg_data.cfgtype)
	{
		case CFG_SET_MODE:
			rc = mt9p111_set_sensor_mode(cfg_data.mode, cfg_data.width, cfg_data.height);
			break;

		case CFG_SET_EFFECT:
			rc = mt9p111_set_effect(cfg_data.mode);
			break;

		case CFG_SET_DEFAULT_FOCUS:
			rc = mt9p111_set_default_focus(cfg_data.mode);
			break;

		case CFG_MOVE_FOCUS:
			rc = mt9p111_set_move_focus(cfg_data.cfg.focus.steps);
			break;

		case CFG_SET_FPS:
			rc = mt9p111_set_frame(cfg_data.mode);
			break;

		case CFG_SET_BRIGHTNESS:
			rc = mt9p111_set_brightness(cfg_data.mode);
			break;

		case CFG_SET_ZOOM:
			rc = mt9p111_set_zoom(cfg_data.mode);
			break;

		case CFG_SET_EXPOSURE_MODE:
			rc = mt9p111_set_exposure(cfg_data.mode);
			break;
			
		case CFG_SET_WB:
			rc = mt9p111_set_wb(cfg_data.mode);
			break;

		case CFG_SET_ANTIBANDING:
			rc= mt9p111_set_antibanding(cfg_data.mode);
			break;

		case CFG_GET_AF_MAX_STEPS:
			cfg_data.max_steps = MT9P111_TOTAL_STEPS_NEAR_TO_FAR;
			if(copy_to_user((void *)argp, &cfg_data, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
			break;

		case CFG_START_AF_FOCUS:
			rc = mt9p111_set_af(cfg_data.mode);
			break;

		case CFG_CHECK_AF_DONE:
#if defined(USE_CONTINUOUS_AF)			
			rc = mt9p111_get_continuous_af(&cfg_data.mode);
			if(copy_to_user((void *)argp, &cfg_data, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
#endif			
			break;

		case CFG_CHECK_AF_CANCEL:
			rc = mt9p111_cancel_af(cfg_data.mode);
			break;
			
		case CFG_SET_ISO:
			rc = mt9p111_set_iso(cfg_data.mode);
			break;

		case CFG_SET_FOCUS_RECT:
			rc = mt9p111_focus_rect(cfg_data.mode);
			break;

		case CFG_SET_SCENE:
			rc = mt9p111_set_scene(cfg_data.mode);
			break;

		case CFG_SET_PARM_AF_MODE:
			rc = mt9p111_af_mode(cfg_data.mode);
			break;

		case CFG_GET_CURRENT_ISO:
			rc = mt9p111_get_iso(cfg_data.mode);
			break;

		case CFG_GET_CHECK_SNAPSHOT:
			rc = mt9p111_get_snapshot(cfg_data.mode);
			break;
			
		default:
			rc = -EINVAL;
			break;
	}

	mutex_unlock(&mt9p111_mutex);

	return rc;
}

static const struct i2c_device_id mt9p111_i2c_id[] = 
{
	{ "mt9p111", 0},
	{ },
};

static int mt9p111_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&mt9p111_wait_queue);
	
	return 0;
}

static int mt9p111_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int rc = 0;
	
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	mt9p111_sensorw = (struct mt9p111_work *)kzalloc(sizeof(struct mt9p111_work), GFP_KERNEL);
	if (!mt9p111_sensorw)
	{
		MT9P111_LOG("mt9p111_sensorw is NULL.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, mt9p111_sensorw);
	mt9p111_init_client(client);
	mt9p111_client = client;

	MT9P111_LOG("succeeded\n");

	return rc;

probe_failure:
	MT9P111_LOG("mt9p111_probe failed\n");
	return rc;
}

static struct i2c_driver mt9p111_i2c_driver =
{
	.id_table = mt9p111_i2c_id,
	.probe  = mt9p111_i2c_probe,
	.remove = __exit_p(mt9p111_i2c_remove),
	.driver =
	{
		.name = "mt9p111",
	},
};

static ssize_t mt9p111_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	MT9P111_LOG("mt9p111_test_val: %d\n", mt9p111_test_val);

	return sprintf(buf, "0x%x\n", mt9p111_test_val);
}

static ssize_t mt9p111_test_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int value;

	sscanf(buf, "%x", &value);
	mt9p111_test_val = value;

	if(mt9p111_test_val < 0x10)
		mt9p111_log_enable = mt9p111_test_val;
	else if(mt9p111_test_val < 0x20)
		mt9p111_set_wb(mt9p111_test_val - 0x10);
	else if(mt9p111_test_val < 0x30)
		mt9p111_set_brightness(mt9p111_test_val - 0x20);
	else if(mt9p111_test_val < 0x40)
		mt9p111_set_af(mt9p111_test_val - 0x30);
	else if(mt9p111_test_val < 0x50)
		mt9p111_set_exposure(mt9p111_test_val - 0x40);
	else if(mt9p111_test_val < 0x60)
		mt9p111_set_frame(mt9p111_test_val - 0x50);
	else if(mt9p111_test_val < 0x70)
		mt9p111_set_zoom(mt9p111_test_val - 0x60);
	else if(mt9p111_test_val < 0x80)
		mt9p111_set_iso(mt9p111_test_val - 0x70);
	else if(mt9p111_test_val < 0x90)
		mt9p111_set_effect(mt9p111_test_val - 0x80);
	else if(mt9p111_test_val < 0xA0)
		mt9p111_set_scene(mt9p111_test_val - 0x90);	

	MT9P111_LOG("mt9p111_test_val: %d\n", mt9p111_test_val);

	return n;
}

static DEVICE_ATTR(test, 0664, mt9p111_test_show, mt9p111_test_store);

static int mt9p111_sensor_probe(const struct msm_camera_sensor_info *info, struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&mt9p111_i2c_driver);
	if(rc < 0 || mt9p111_client == NULL)
	{
		MT9P111_LOG("sensor_probe failed: %d\n", rc);
		rc = -ENOTSUPP;
		goto probe_done;
	}

	s->s_init = mt9p111_sensor_init;
	s->s_release = mt9p111_sensor_release;
	s->s_config  = mt9p111_sensor_config;

	rc = device_create_file(&mt9p111_pdev->dev, &dev_attr_test);
	if(rc < 0)
	{
		MT9P111_LOG("dev_attr_test failed: %d\n", rc);
		return rc;
	}

probe_done:
	MT9P111_LOG("sensor_probe succeeded\n");
	return rc;
}

static int __mt9p111_probe(struct platform_device *pdev)
{
	mt9p111_pdev = pdev;
	return msm_camera_drv_start(pdev, mt9p111_sensor_probe);
}

static struct platform_driver msm_camera_driver =
{
	.probe = __mt9p111_probe,
	.driver =
	{
		.name = "msm_camera_mt9p111",
		.owner = THIS_MODULE,
	},
};

static int __init mt9p111_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

#if defined(CAM_TXT_TUNING)
static unsigned short cam_txt_convert(char ch)
{
	unsigned short value;
	
	value = (unsigned short)ch;
	if(48<= value && value<=57) /* 0-9 */
		return (value-48);
	if(65<= value && value<=90) /* A-Z */       
		return (value-55);
	if(97<= value && value<=122) /* a- z */     
		return (value-87);  

	return 0;
}

static unsigned int cam_txt_to_hex(unsigned char N, char *cmd)
{
	unsigned char i = 0;
	unsigned int digit=0; 
	unsigned int value=0;

	for(i=0; i<N; i++)
	{       
		digit = cam_txt_convert(*(cmd+i));
		value  += (digit&0xFF)<<((N-i-1)*4);
	}

	return value;
}

static unsigned int cam_txt_to_dec(unsigned char N, char *cmd)
{
	char i = 0;
	char j = 0;
	unsigned int digit = 0;
	unsigned int value = 0;
	unsigned int power = 0;

	for(i=0; i<N; i++)
	{
		digit = cam_txt_convert(*(cmd+i));
		power = 1;
		for(j=N-i-1; j>0; j--)
			power = power*10;
		
		value  += digit*power;
	}
	
	return value;
}

static int cam_txt_check_type(unsigned char *type)
{
	int ret_value = -1;

	if(strncmp(type, "CMD", 3) == 0)
	{
		if(strncmp(type, "CMD_WRITE", 9) == 0) ret_value = CMD_WRITE;
		else if(strncmp(type, "CMD_POLL", 8) == 0) ret_value = CMD_POLL;
		else if(strncmp(type, "CMD_DELAY", 9) == 0) ret_value = CMD_DELAY;
		else ret_value = -1;
	}
	else
		ret_value = -1;

	return ret_value;
}

static int cam_txt_is_digit(unsigned char data)
{
	int ret_value = 0;
	static int prev_value = -1;
	
	if(data >= '0' && data <= '9') ret_value = DEC_VAL;
	else if(data >= 'a' && data <= 'f') ret_value = HEX_VAL;
	else if(data >= 'A' && data <= 'F') ret_value = HEX_VAL;
	else if(data == 'x' || data == 'X') ret_value = HEX_VAL;
	else ret_value = 0;

	if(prev_value == 0 && ret_value == HEX_VAL) ret_value = 0;
	else prev_value = ret_value;

	return ret_value;
}

static int cam_txt_get_data1(unsigned char *buf, int pos, int *reg, int *data, int *len)
{
	int ret_value = 0;
	int check_digit = 0;
	int dec_cnt = 0;
	int hex_cnt =0;
	int i =0;
	unsigned char temp[6];

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}

	if(hex_cnt > 0 && dec_cnt > 0)
		*reg = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9P111_LOG("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -1;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*data = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9P111_LOG("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -2;
	};

	*len = (i-2)/2;

	return ret_value;
}

static int cam_txt_get_data2(unsigned char *buf, int pos, int *reg, int *data, int *count)
{
	int ret_value = 0;
	int check_digit = 0;
	int dec_cnt = 0;
	int hex_cnt =0;
	int i =0;
	unsigned char temp[6];

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}

	if(hex_cnt > 0 && dec_cnt > 0)
		*reg = cam_txt_to_hex((i-2), &(temp[2]));
	else 
	{
		MT9P111_LOG("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -1;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*data = cam_txt_to_hex((i-2), &(temp[2]));
	else if(dec_cnt > 0 && hex_cnt == 0)
		*data = cam_txt_to_dec(i, temp);
	else 
	{
		MT9P111_LOG("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -2;
	}

	while(!cam_txt_is_digit(buf[pos++]));
	pos--;

	memset(temp, 0x00, 6);
	check_digit = 0;
	dec_cnt = 0;
	hex_cnt = 0;
	i = 0;
	while((check_digit = cam_txt_is_digit(buf[pos])) && (i <6))
	{
		if(check_digit == DEC_VAL) dec_cnt++;
		else if(check_digit == HEX_VAL) hex_cnt++;

		temp[i++] = buf[pos++];
	}
	
	if(hex_cnt > 0 && dec_cnt > 0)
		*count = cam_txt_to_hex((i-2), &(temp[2]));
	else if(dec_cnt > 0 && hex_cnt == 0)
		*count = cam_txt_to_dec(i, temp);
	else 
	{
		MT9P111_LOG("hex_cnt: %d, dec_cnt: %d\n", hex_cnt, dec_cnt);
		return -3;
	}

	return ret_value;
}

static void cam_txt_shift(unsigned char* data)
{
	int i;
	
	for(i=0; i<CMD_BUF_MAX_LENGTH-1; i++)
		*(data+i) = *(data+i+1);
}

int cam_txt_from_file(char *file_name)
{
	struct file *flip;
	mm_segment_t old_fs;
	
	int read_size;
	unsigned char *buffer = NULL;
	unsigned char *cmd_buffer = NULL;
  
	int i = 0;
	int ret_val = 0;
	int cmd_type = -1;
	int cmd_reg = 0;
	int cmd_data = 0;
	int cmd_len = 0;

	unsigned char cmd_buffer_str[CMD_BUF_MAX_LENGTH];
	unsigned char line_skip_flag = 0;
	int data_count = 0;

	int cam_txt_file_size = 0;
	int cam_txt_count = 0;
	struct mt9p111_register_pair *cam_txt_file_data = NULL;

	static int cam_txt_in_use = 0;

	while((cam_txt_in_use != 0) && (i<10))
	{
		msleep(100);
		i++;
	}
	i=0;

	cam_txt_in_use = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	
	flip = filp_open(file_name, O_RDONLY |O_LARGEFILE, S_IRUSR);
	if(IS_ERR(flip))
	{
		MT9P111_LOG("%s open failed: %d\n", file_name, (int)IS_ERR(flip));
		set_fs(old_fs);
		cam_txt_in_use = 0;
		return -EIO;
	}

	cam_txt_file_size = flip->f_op->llseek(flip, (loff_t)0, SEEK_END);
	MT9P111_LOG("%s: cam_txt_file_size: %d\n", file_name, cam_txt_file_size);

	buffer = (unsigned char *)kmalloc(cam_txt_file_size, GFP_KERNEL);
	if(!buffer)
	{
		MT9P111_LOG("alloc failed: %d\n", cam_txt_file_size);
		filp_close(flip, NULL);
		set_fs(old_fs);
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		return -ENOMEM;
	}

	flip->f_pos = 0;
	read_size = flip->f_op->read(flip, buffer, cam_txt_file_size, &flip->f_pos);
	MT9P111_LOG("read size: %d\n", read_size);

	filp_close(flip, NULL);
	set_fs(old_fs);
		
	i = 0;
	line_skip_flag = 0;
	memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
	
	while(i < cam_txt_file_size)
	{
		cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] = *(buffer+i);

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
		{
			line_skip_flag = 1;
		}

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '\r') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n'))
		{
			line_skip_flag = 0;
		}
			
		if((line_skip_flag == 0) && (cam_txt_check_type(cmd_buffer_str) > 0))
		{
			data_count++;
			memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		}
		cam_txt_shift(cmd_buffer_str);
		i++;
	}

	cam_txt_count = data_count;

	cmd_buffer = (unsigned char *)kmalloc(cam_txt_count*sizeof(struct mt9p111_register_pair), GFP_KERNEL);
	if(!cmd_buffer)
	{
		MT9P111_LOG("alloc failed: %d\n", (cam_txt_count*sizeof(struct mt9p111_register_pair)));
		kfree(buffer);
		cam_txt_count = 0;
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		return -ENOMEM;
	}
		
	cam_txt_file_data = (struct mt9p111_register_pair*)cmd_buffer;

	i = 0;
	line_skip_flag = 0;
	data_count = 0;
	memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		
	while((i < cam_txt_file_size) && (data_count < cam_txt_count))
	{
		cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] = *(buffer+i);
		i++;

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '/') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '/'))
		{
			line_skip_flag = 1;
		}

		if((cmd_buffer_str[CMD_BUF_MAX_LENGTH-2] == '\r') && (cmd_buffer_str[CMD_BUF_MAX_LENGTH-1] == '\n'))
		{
			line_skip_flag = 0;
		}

		cmd_type = cam_txt_check_type(cmd_buffer_str);

		if((line_skip_flag == 0) && (cmd_type == CMD_WRITE))
		{
			ret_val = cam_txt_get_data1(buffer, i, &cmd_reg, &cmd_data, &cmd_len);

			if(ret_val < 0)
			{
				MT9P111_LOG("invalid data format[1]: %d\n", ret_val);
				kfree(buffer);
				kfree(cmd_buffer);
				cam_txt_file_data = NULL;
				cam_txt_count = 0;
				cam_txt_file_size = 0;
				cam_txt_in_use = 0;
				return -EFAULT;
			}

			MT9P111_LOG("(%d) reg: 0x%04x, data: 0x%04x, count: %d\n", data_count, cmd_reg, cmd_data, cmd_len);

			cam_txt_file_data[data_count].register_type = cmd_type;
			cam_txt_file_data[data_count].register_address = (uint16_t)cmd_reg;
			cam_txt_file_data[data_count].register_value = (uint32_t)cmd_data;
			cam_txt_file_data[data_count].register_length = cmd_len;
				
			data_count++;
			memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		}
		else if((line_skip_flag == 0) && ((cmd_type == CMD_POLL) || (cmd_type == CMD_DELAY)))
		{
			ret_val = cam_txt_get_data2(buffer, i, &cmd_reg, &cmd_data, &cmd_len);

			if(ret_val < 0)
			{
				MT9P111_LOG("invalid data format[2]: %d, %d\n", ret_val, cmd_type);
				kfree(buffer);
				kfree(cmd_buffer);
				cam_txt_file_data = NULL;
				cam_txt_count = 0;
				cam_txt_file_size = 0;
				cam_txt_in_use = 0;
				return -EFAULT;
			}

			MT9P111_LOG("(%d) reg: 0x%04x, data: 0x%04x, count: %d\n", data_count, cmd_reg, cmd_data, cmd_len);

			cam_txt_file_data[data_count].register_type = cmd_type;
			cam_txt_file_data[data_count].register_address = (uint16_t)cmd_reg;
			cam_txt_file_data[data_count].register_value = (uint32_t)cmd_data;
			cam_txt_file_data[data_count].register_length = cmd_len;
				
			data_count++;
			memset(cmd_buffer_str, 0x00, CMD_BUF_MAX_LENGTH);
		}
		cam_txt_shift(cmd_buffer_str);
	}

	kfree(buffer);

	ret_val = mt9p111_i2c_write_table(cam_txt_file_data, cam_txt_count);
	if(ret_val < 0)
	{
		MT9P111_LOG("cam_txt i2c write failed: %d\n", ret_val);
		kfree(cmd_buffer);
		cam_txt_file_data = NULL;
		cam_txt_count = 0;
		cam_txt_file_size = 0;
		cam_txt_in_use = 0;
		return -EFAULT;
	}

	if(cam_txt_file_data != NULL)
	{
		kfree((unsigned char *)cam_txt_file_data);
		cam_txt_file_data = NULL;
		cam_txt_count = 0;
		cam_txt_file_size = 0;
	}

	cam_txt_in_use = 0;
	
	return ret_val;
}
#endif

late_initcall(mt9p111_init);

