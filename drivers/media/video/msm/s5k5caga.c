/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Sony 3M s5k5caga camera sensor driver
 * Auther: Han Jun-Yeong[junyeong.han@lge.com], 2010-09-03
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
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <linux/kthread.h>

#include "s5k5caga.h"
#include "s5k5caga_reg.h"

/***********************************************************/
// 값이 set되면 sdcard에서 register값을 읽어서 카메라 초기화작업을 실시한다.
// 파일의 마지막은 항상 $ 로 마무리 해야 한다..( 조건문에 걸려잇다.)
#define SENSOR_TUNING_SET										0 // (0 : 일반적인 버젼 1: 튜닝을 위해서 setting)

//SENSOR_TUNING_SET == 1이어야만 의미가 잇다.
#define SENSOR_TUNING_LOG										0 // (0 : log를 생성안한다. 1: debugging을 위해 log set)
/***********************************************************/

#define LGCAM_REAR_SENSOR_ENABLE											0	//mhlee
/* Sensor Core Registers */
#define  REG_LGCAM_REAR_SENSOR_MODEL_ID 0x0000
#define  LGCAM_REAR_SENSOR_MODEL_ID     0x0520

/* BEGIN: 0004743 hyungtae.lee@lge.com 2010-03-08 */
/* MOD: 0004743: [camera] modification of capture mode on night mode */ 
#define PREVIEW_MODE 0
#define CAPTURE_MODE 1

#define NOT_NIGHT_MODE 0
#define NIGHT_MODE 1
//#define LGCAM_REAR_SENSOR_THREAD_ENABLE //not defined for s5k5caga

int current_scene = 0;
int previous_mode = 0;
/* END: 0004743 hyungtae.lee@lge.com 2010-03-08 */

int focus_mode = 0;
static int debug_mask = 0;

static int prev_af_mode;
static int init_burst_mode = 0;
//#define TOUCH_FOCUS_TEST

#if SENSOR_TUNING_SET
static struct lgcam_rear_sensor_register_address_value_pair  ext_reg_settings[4000];
#else
static long lgcam_rear_sensor_set_capture_zoom(int zoom);
#endif


module_param_named(debug_mask, debug_mask, int, S_IRUGO|S_IWUSR|S_IWGRP);
struct lgcam_rear_sensor_work {
	struct work_struct work;
};

static struct  lgcam_rear_sensor_work *lgcam_rear_sensor_sensorw;
static struct  i2c_client *lgcam_rear_sensor_client;

struct lgcam_rear_sensor_ctrl {
	const struct msm_camera_sensor_info *sensordata;
	int8_t sensormode;
	unsigned char qfactor;

   /* for Video Camera */
	int8_t effect;
	int8_t wb;
	int8_t scene;
	unsigned char brightness;
	int8_t af;
	int8_t zoom;
   /* to write register */
	int16_t write_byte;
	int16_t write_word;
};

static struct lgcam_rear_sensor_ctrl *lgcam_rear_sensor_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(lgcam_rear_sensor_wait_queue);
DECLARE_MUTEX(lgcam_rear_sensor_sem);

DEFINE_MUTEX(lgcam_rear_sensor_tuning_mutex);
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE	
static int enable_capturemode = 0;
static int tuning_thread_run = 0;
static int statuscheck = 0;
#define CFG_WQ_SIZE		64

struct config_work_queue {
	int cfgtype;
	int mode;
};

static struct config_work_queue *cfg_wq;
static int cfg_wq_num;
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE	

struct current_pict_size{
	int width;
	int height;
};
static struct current_pict_size pict_size;
static unsigned char *sensor_burst_buffer;		//mhlee 0105
#define sensor_burst_size	750					//mhlee 0105
static int32_t lgcam_rear_sensor_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum lgcam_rear_sensor_width width);

DEFINE_MUTEX(lgcam_rear_sensor_mutex);

/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct lgcam_rear_sensor_reg lgcam_rear_sensor_regs;

/*=============================================================*/

static int32_t lgcam_rear_sensor_i2c_txdata(unsigned short saddr,
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

	if (i2c_transfer(lgcam_rear_sensor_client->adapter, msg, 1) < 0 ) {
		printk("lgcam_rear_sensor_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t lgcam_rear_sensor_i2c_write(unsigned short saddr,
	unsigned short waddr, uint32_t wdata, enum lgcam_rear_sensor_width width)
{
#if SENSOR_TUNING_SET
	int fail_count = 0;
	int retry ;
#endif
	int32_t rc = 0;//-EIO;
	unsigned char buf_dbl[6];
	unsigned char buf_wrd[4];
	unsigned char buf_bte[3];
	static int burst_num = 0;
	
	switch (width) {
		
		case DOBULE_LEN:
		memset(buf_dbl, 0, sizeof(buf_dbl));
		buf_dbl[0] = (waddr & 0xFF00)>>8;
		buf_dbl[1] = (waddr & 0x00FF);
		buf_dbl[2] = (wdata & 0xFF000000)>>24;
		buf_dbl[3] = (wdata & 0x00FF0000)>>16;
		buf_dbl[4] = (wdata & 0x0000FF00)>>8;
		buf_dbl[5] = (wdata & 0x000000FF);
		rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_dbl, 6);
		break;
		
		case WORD_LEN:
#if 1
		if(init_burst_mode)
		{
			if(waddr == 0x0F12)
			{
				if(burst_num == 0)
				{
					memset(sensor_burst_buffer, 0, sizeof(sensor_burst_buffer));
					sensor_burst_buffer[burst_num++] = 0x0F;
					sensor_burst_buffer[burst_num++] = 0x12;			
				}
				sensor_burst_buffer[burst_num++] = (wdata & 0xFF00)>>8; 	
				sensor_burst_buffer[burst_num++] = (wdata & 0x00FF);


			}
			else
			{
				if(burst_num > 0)
				{
					rc = lgcam_rear_sensor_i2c_txdata(saddr, (unsigned char*)sensor_burst_buffer, burst_num);	
			//		memset(sensor_burst_buffer, 0, sizeof(sensor_burst_buffer));

				}

				memset(buf_wrd, 0, sizeof(buf_wrd));
				buf_wrd[0] = (waddr & 0xFF00)>>8;
				buf_wrd[1] = (waddr & 0x00FF);
				buf_wrd[2] = (wdata & 0xFF00)>>8;
				buf_wrd[3] = (wdata & 0x00FF);
				rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_wrd, 4);		
				burst_num = 0;
			}
		}
		else
		{
			memset(buf_wrd, 0, sizeof(buf_wrd));
			buf_wrd[0] = (waddr & 0xFF00)>>8;
			buf_wrd[1] = (waddr & 0x00FF);
			buf_wrd[2] = (wdata & 0xFF00)>>8;
			buf_wrd[3] = (wdata & 0x00FF);
			rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_wrd, 4);
		}
#else
		memset(buf_wrd, 0, sizeof(buf_wrd));
		buf_wrd[0] = (waddr & 0xFF00)>>8;
		buf_wrd[1] = (waddr & 0x00FF);
		buf_wrd[2] = (wdata & 0xFF00)>>8;
		buf_wrd[3] = (wdata & 0x00FF);
		rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_wrd, 4);
#endif
		break;

		case BYTE_LEN:
		memset(buf_bte, 0, sizeof(buf_bte));
		buf_bte[0] = (waddr & 0xFF00)>>8;
		buf_bte[1] = (waddr & 0x00FF);
		buf_bte[2] = wdata;
		rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_bte, 3);
			break;

		case ADDRESS_TUNE:
		default:
			break;
	}

// I2C fail발생이 되는 경우가 잇어서 방어추가한다.
#if SENSOR_TUNING_SET
	if(rc < 0){
		if(debug_mask)   
			printk("lgcam_rear_sensor_i2c_write first error!!!\n");
		
		for(retry = 0; retry < 3; retry++){
			if((width == BURST_LEN) && (waddr == 0xFFFF))
				rc = lgcam_rear_sensor_i2c_txdata(saddr, (unsigned char*)sensor_burst_buffer, (burst_num+1));
			else if(width == DOBULE_LEN)
				rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_dbl, 6);
			else if(width == WORD_LEN)
				rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_wrd, 4);
			else if(width == BYTE_LEN)
				rc = lgcam_rear_sensor_i2c_txdata(saddr, buf_bte, 3);   				

			if(rc >= 0)
			{
				fail_count = 0;
               	retry = 3;
			}
			else
			{				
				fail_count = fail_count + 1;
					if(debug_mask)   
						printk("lgcam_rear_sensor_i2c_write #%d\n", fail_count);
			}
		}
	}
#endif

	if (rc < 0){
		printk("i2c_write failed, addr = 0x%x, val = 0x%x!\n",waddr, wdata);
	}
	return rc;	
}

static int32_t lgcam_rear_sensor_i2c_write_table(
	struct lgcam_rear_sensor_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i = 0, fail_count = 0;
	int retry ;
	int32_t rc = -EIO;
	
	for (i = 0; i < num_of_items_in_table; i++) {
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
		reg_conf_tbl->waddr, reg_conf_tbl->wdata,
		reg_conf_tbl->width);
		
		//if (reg_conf_tbl->mdelay_time != 0)
		//	mdelay(reg_conf_tbl->mdelay_time);
     
		if(rc < 0){
    		for(retry = 0; retry < 3; retry++){
   				rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
		    			reg_conf_tbl->waddr, reg_conf_tbl->wdata,
		    			reg_conf_tbl->width);
           
            	if(rc >= 0)
               	retry = 3;
            
            	fail_count = fail_count + 1;
				if(debug_mask)   
					printk("lgcam_rear_sensor: i2c fail #%d\n", fail_count);
	
         	}
         	reg_conf_tbl++;
		}else
         	reg_conf_tbl++;
	}
	return rc;
}
#if !SENSOR_TUNING_SET
static int32_t lgcam_rear_sensor_i2c_write_table_one(
	struct lgcam_rear_sensor_i2c_reg_conf const *reg_conf_tbl,	int selected)
{
	int i = 0, fail_count = 0;
	int retry ;
	int32_t rc = -EIO;

	if(selected != 0){
		for (i = 0; i < selected; i++) {
			reg_conf_tbl++;
		}
	}
	
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
	reg_conf_tbl->waddr, reg_conf_tbl->wdata,
	reg_conf_tbl->width);
	
	//if (reg_conf_tbl->mdelay_time != 0)
	//	mdelay(reg_conf_tbl->mdelay_time);
 
	if(rc < 0){
		for(retry = 0; retry < 3; retry++){
				rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
	    			reg_conf_tbl->waddr, reg_conf_tbl->wdata,
	    			reg_conf_tbl->width);
       
        	if(rc >= 0)
           	retry = 3;
        
        	fail_count = fail_count + 1;
			if(debug_mask)   
				printk("lgcam_rear_sensor: lgcam_rear_sensor_i2c_write_table_one :: i2c fail #%d\n", fail_count);
     	}     	
	}	
	return rc;
}
#endif //!SENSOR_TUNING_SET

static int lgcam_rear_sensor_i2c_rxdata(unsigned short saddr,
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

	if (i2c_transfer(lgcam_rear_sensor_client->adapter, msgs, 2) < 0) {
		printk("lgcam_rear_sensor_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t lgcam_rear_sensor_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum lgcam_rear_sensor_width width)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	switch (width) {
		case WORD_LEN: 
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = lgcam_rear_sensor_i2c_rxdata(saddr, buf, 2);
		if (rc < 0)
			return rc;

		*rdata = buf[0] << 8 | buf[1];
		break;
   	
		case BYTE_LEN:
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = lgcam_rear_sensor_i2c_rxdata(saddr, buf, 2);
		if (rc < 0)
			return rc;
		
		*rdata = buf[0];
			break;           

		case BURST_LEN:
		case DOBULE_LEN:
		case ADDRESS_TUNE:
		default:
			/* nothing to do */
		break;           
	}

	if (rc < 0)
		printk("lgcam_rear_sensor_i2c_read failed!\n");
   
	return rc;
}
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
static int lgcam_rear_sensor_check_thread_run(void)
{
	int32_t rc = -1;		
	int i = 0;
	
	for(i = 0; i < 1000; i++)
	{
		if(tuning_thread_run)
			mdelay(10);
		else
		{
			rc = 0;
			break;
		}

	}
	return rc;
}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE	

#if 0 /* not used, now */
static int lgcam_rear_sensor_reset(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_reset, "lgcam_rear_sensor");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_reset, value);
	else{
		printk("lgcam_rear_sensor_reset: reset gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_reset);
	return rc;
}

static int lgcam_rear_sensor_pwdn(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_pwd, "lgcam_rear_sensor");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_pwd, value);
	else{
		printk("lgcam_rear_sensor_pwdn: pwdn gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_pwd);
	return rc;
}
#endif

void lgcam_rear_sensor_sensor_power_enable(void)
{
	lgcam_rear_sensor_ctrl->sensordata->pdata->camera_power_on();
}

void lgcam_rear_sensor_sensor_power_disable(void)
{
	lgcam_rear_sensor_ctrl->sensordata->pdata->camera_power_off();
}

static long lgcam_rear_sensor_snapshot_config(int mode,int width, int height)
{
	int32_t rc;
//	uint16_t picture_width = 0;
//	uint16_t picture_height = 0;
	
//	picture_width  = width & 0xFFFF;
//	picture_height = height & 0xFFFF;	
	if(debug_mask)
	   printk("lgcam_rear_sensor_snapshot_config:	Input resolution[%d * %d]\n", width, height);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x035E, WORD_LEN);
	if (rc < 0)
		return rc;
	
	
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, width, WORD_LEN);
	if (rc < 0)
		return rc;
	
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, height, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.snap_reg_settings[0], lgcam_rear_sensor_regs.snap_reg_settings_size);
	if (rc < 0)
		return rc;
	lgcam_rear_sensor_ctrl->sensormode = mode;
	return 0;
}
static int32_t lgcam_rear_sensor_set_continuous_focus(void)
{
	int32_t rc;
	
	if(debug_mask)
		printk("lgcam_rear_sensor: continuous focus\n");

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x0252, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0006, WORD_LEN);

	if(rc<0){
		if(debug_mask)
			printk("lgcam_rear_sensor: AF continuous focus writing fail!\n");
		return rc;
	}	
	return rc;
}

static int32_t lgcam_rear_sensor_cancel_focus(int mode)
{
	int32_t rc = 0;
	int32_t i = 0;
	unsigned short af_pos = 0;
	if(debug_mask)
		printk("lgcam_rear_sensor: cancel focus, mode = %d\n", mode);

	switch(mode){
	case FOCUS_AUTO:
	case FOCUS_NORMAL:
	case FOCUS_CONTINUOUS_VIDEO:
	case FOCUS_CONTINUOUS_CAMERA:		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.AF_nomal_reg_settings[0], lgcam_rear_sensor_regs.AF_nomal_reg_settings_size);

		if(debug_mask)
	   		printk("back to the infinity\n");

		break;
	
	case FOCUS_MACRO:
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.AF_macro_reg_settings[0], lgcam_rear_sensor_regs.AF_macro_reg_settings_size);
		if(debug_mask)
	   		printk("back to the macro\n");

		break;

	case FOCUS_MANUAL:
			return rc;
		break;
	default:
		break;
	}

	if (rc < 0)
		return rc;
	for (i = 0; i < 10; ++i) {
		/*INT state read to confirm INT release state*/

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002C, 0x7000, WORD_LEN);		

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002E, 0x282E, WORD_LEN);
		
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr,
				0x0F12, &af_pos, WORD_LEN);
		
		if (rc < 0) {
			printk("lgcam_rear_sensor: reading af_lock fail\n");
			return rc;
		}
		if((mode == FOCUS_AUTO) || (mode == FOCUS_NORMAL))
		{
			if(af_pos == 0x0){
				if(debug_mask)
					printk("af_lenspos is equal with af_pos = 0x%x\n", af_pos);
				return rc;
			}
			else
			{
				if(debug_mask)
					printk("af_lenspos is equal with af_pos1 = 0x%x\n", af_pos);
			}
		}
		else if(mode == FOCUS_MACRO)
		{
			if((af_pos >= 0x5C) && (af_pos <= 0x9C)){
				if(debug_mask)
					printk("macro af_lenspos is equal with af_pos = 0x%x\n", af_pos);
				return rc;
			}
			else
				printk("macro af_lenspos is equal with af_pos1 = 0x%x\n", af_pos);
		}
		else if((mode == FOCUS_CONTINUOUS_VIDEO) || (mode == FOCUS_CONTINUOUS_CAMERA))
		{
			if(af_pos == 0x0){
				if(debug_mask)
					printk("caf_lenspos is equal with af_pos = 0x%x\n", af_pos);
				lgcam_rear_sensor_set_continuous_focus();
				return rc;
			}
			else
			{
				if(debug_mask)
					printk("caf_lenspos is equal with af_pos1 = 0x%x\n", af_pos);	
			}
		}
		else
		{
			if(debug_mask)
				printk("lens position(0x%x) is not ready yet\n",af_pos);
		}
		
		
		mdelay(60); // 1 frame skip
	}
	if((mode == FOCUS_CONTINUOUS_VIDEO) || (mode == FOCUS_CONTINUOUS_CAMERA))
			lgcam_rear_sensor_set_continuous_focus();
	return rc;
}

static long lgcam_rear_sensor_set_sensor_mode(int mode,int width, int height)
{
	int32_t rc = 0;   
	
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
			if(debug_mask)
				printk("lgcam_rear_sensor_set_sensor_mode: sensor mode is PREVIEW, previous mode = %d\n", previous_mode);			
		
		if(previous_mode == CAPTURE_MODE){
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.prev_reg_settings[0], lgcam_rear_sensor_regs.prev_reg_settings_size);
			if(rc<0){
				printk("lgcam_rear_sensor: preview writing fail!\n");
				return rc;
			}
			mdelay(150); // 1 frame skip ==> total 2 frames skip

	//		mdelay(200);  // 2 frames skip
			rc = lgcam_rear_sensor_cancel_focus(focus_mode);
			if(rc<0)
				return rc;
			/* BEGIN: 0005153 hyungtae.lee@lge.com 2010-03-18 */
			/* MOD: 0005153: [camera] adjustment of preview timing after capture */ 
#if 0

			if(current_scene == NIGHT_MODE){
				if(debug_mask)
					printk("[lgcam_rear_sensor.c] current scene is NIGHT\n");
				
				mdelay(800);
		   }
#endif
		  /* END: 0005153 hyungtae.lee@lge.com 2010-03-18 */ 
			previous_mode = PREVIEW_MODE;
		}
	    break;

	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_sensor_mode: sensor mode is SNAPSHOT\n");
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE	
		if(tuning_thread_run)
		{
			rc = lgcam_rear_sensor_check_thread_run();
			if (rc < 0)
			{
				printk("lgcam_rear_sensor_check_thread_run error\n");
			}
			mdelay(500);
		}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE
	
		rc = lgcam_rear_sensor_snapshot_config(mode, width, height);
		
		/* BEGIN: 0005118 hyungtae.lee@lge.com 2010-03-17 */
		/* MOD: 0005118: [camera] reduce power-off time and capture time */
		//mdelay(80);    // 1 frame skip
		
#if 0
		if(current_scene == NIGHT_MODE){
			if(debug_mask)
				printk("[lgcam_rear_sensor.c] current scene is NIGHT\n");

			mdelay(500);
		}
#endif
		if(rc < 0)
			return rc;
		previous_mode = CAPTURE_MODE;
		break;

	default:
		return -EINVAL;
	}
   
	return 0;
}
#if 0 /* not used, now */
static int32_t lgcam_rear_sensor_check_af_lock_and_clear(void)
{
	int32_t rc;
	int i;
	unsigned short af_lock;
	
	/* check AF lock status */
	for(i=0; i<5; i++){
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x00F8, &af_lock, BYTE_LEN);
		if (rc < 0){
			printk("lgcam_rear_sensor: reading af_lock fail\n");
			return rc;
		}
	
		if((af_lock & 0x10)==0x10){
            if(debug_mask)
			    printk(" af_lock is locking\n");
			break;
		}
		if(debug_mask)
			printk("lgcam_rear_sensor: af_lock( 0x%x ) is not ready yet\n", af_lock);
		mdelay(70);
	}	
	
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x00FC, 0x10, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	/* check AF lock status */
	for(i=0; i<5; i++){
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x00F8, &af_lock, BYTE_LEN);
		if (rc < 0){
			printk("lgcam_rear_sensor: reading af_lock fail\n");
			return rc;
		}
	
		if((af_lock & 0x10)==0x00){
			printk("af_lock is released\n");
			break;
		}
		if(debug_mask)
			printk("lgcam_rear_sensor: af_lock( 0x%x ) is not clear yet\n", af_lock);
		mdelay(70);
	}	

	return rc;
}
#endif

#define FOCUS_STEP_GARO 	4
#define FOCUS_STEP_SERO 	4
#define FOCUS_STEP_MAX 		(FOCUS_STEP_GARO * FOCUS_STEP_SERO)
#define FOCUS_FULLSIZE_X 	2048	//s5k5caga
#define FOCUS_FULLSIZE_Y 	1546 //s5k5caga
#define FOCUS_PREVIEW_X 	640
#define FOCUS_PREVIEW_Y 	480
#define FOCUS_FRSTWINSIZE_X 	320
#define FOCUS_FRSTWINSIZE_Y 	266
#define FOCUS_SCNDWINSIZE_X 	143
#define FOCUS_SCNDWINSIZE_Y 	143



#if !SENSOR_TUNING_SET
static long lgcam_rear_sensor_set_focus_rect(int focus_rect)
{	
	// focus_rect --> focus window값이 내려온다. (16step이다.)
	int32_t rc;	
	int temp_x_position = 0, temp_y_position = 0;	
	int x_position = 0, y_position = 0;	
	int real_x_position = 0, real_y_position = 0;	
//	int real_rect_W = 0, real_rect_H = 0;	

	if(debug_mask)
		printk("lgcam_rear_sensor_set_focus_rect : called, new focus_rect: %d\n", focus_rect);
#if defined (TOUCH_FOCUS_TEST)
	focus_rect = 16;
#endif

	if((focus_rect <= 0) || (focus_rect > 25))
		return 0;



	if(debug_mask)
	{
		printk("lgcam_rear_sensor_set_focus_rect : called, x_position = %d, y_position = %d\n", x_position, y_position);
	}	

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x025A, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x100, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00E3, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0200, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0238, WORD_LEN);
	if (rc < 0)
		return rc;
	if(focus_rect <= FOCUS_STEP_MAX)
	{
		temp_x_position = focus_rect / FOCUS_STEP_GARO;
		temp_y_position = (focus_rect - 1) % FOCUS_STEP_SERO;
		x_position = (int)(temp_x_position * 1024 / FOCUS_PREVIEW_X) ;
		real_x_position = cpu_to_be16(x_position);
		
		y_position = (int)(temp_y_position * 1024 / FOCUS_PREVIEW_Y);
		real_y_position = cpu_to_be16(y_position);		
#if defined (TOUCH_FOCUS_TEST)
		x_position = 0x003C ;
		y_position = 0x0266;
#endif
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, x_position, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, y_position, WORD_LEN);
	if (rc < 0)
		return rc;
	}
	else
	{
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x018C, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0166, WORD_LEN);
	if (rc < 0)
		return rc;
	}
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00E6, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0132, WORD_LEN);
	if (rc < 0)
		return rc;
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0001, WORD_LEN);
	if (rc < 0)
		return rc;
//	prev_af_mode = FOCUS_RECT;
	
	return rc;	
}
#endif //!SENSOR_TUNING_SET

static int lgcam_rear_sensor_check_af_lock(void)
{
	int rc;
	int i;
	unsigned short af_lock = 0;
	

	for (i = 0; i < 10; ++i) {
		/*INT state read to confirm INT release state*/

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002C, 0x7000, WORD_LEN);		

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002E, 0x26FE, WORD_LEN);
		
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr,
				0x0F12, &af_lock, WORD_LEN);
		
		if (rc < 0) {
			printk("lgcam_rear_sensor: reading af_lock fail\n");
			return rc;
		}

		if (af_lock == 0x02) {
			if(debug_mask)
			printk("af_lock is released\n");
			break;
		}
		else
		{
			printk("af_return = %d\n", af_lock);			
		}
		msleep(200);
	}

	return af_lock;
}

static int lgcam_rear_sensor_check_focus(int *lock)
{
	int rc;
#if 0
	unsigned short af_status;
#endif
	unsigned short af_result = 0;

	if(debug_mask)
	printk("lgcam_rear_sensor_check_focus\n");

#if 0	//mhlee 0107
	/*af status check  0:load, 1: init,  8: af_lock */
	rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr,
		0x6D76, &af_status, BYTE_LEN);

	if (af_status != 0x8)
		return -ETIME;

#endif

	af_result = lgcam_rear_sensor_check_af_lock();

	
	if (af_result == 2) {
		//mdelay(60);
		*lock = CFG_AF_LOCKED;  // success
//LGE_DEV_PORTING GELATO
		rc = 1;
//LGE_DEV_END
		return rc;
	} else {
		*lock = CFG_AF_UNLOCKED; //0: focus fail or 2: during focus
//LGE_DEV_PORTING GELATO
		rc = 0;
//LGE_DEV_END
		return rc;
	}

	return -ETIME;
}
static int lgcam_rear_sensor_check_continous_af_lock(void)
{
	int rc;
	unsigned short af_lock = 0;
	

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
		0x002C, 0x7000, WORD_LEN);		

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
		0x002E, 0x26FE, WORD_LEN);
	
	rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr,
			0x0F12, &af_lock, WORD_LEN);
	
	if (rc < 0) {
		printk("lgcam_rear_sensor: reading af_lock fail\n");
		return rc;
	}

	if (af_lock == 0x01) {
		if(debug_mask)
			printk("lgcam_rear_sensor_check_continous_af_lock\n");
	}
	else if(af_lock == 0x02) {
		if(debug_mask)
			printk("lgcam_rear_sensor_check_continous_af_lock success\n");
	}
	else
	{
		if(debug_mask)
			printk("lgcam_rear_sensor_check_continous_af_lock af_return = %d\n", af_lock);			
	}

	return af_lock;
}
static int lgcam_rear_sensor_status_continuous_af(int *lock)
{
	unsigned short af_result = 0;



	af_result = lgcam_rear_sensor_check_continous_af_lock();

	*lock = af_result;	
			return 0;

	return -ETIME;
}
#if 0
static int32_t lgcam_rear_sensor_check_focus(int *lock)
{
	int32_t rc;
	int num = 0;
	unsigned short af_status, af_result, af_result_ex;
	
	while(num < 200){
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x6D76, &af_status, BYTE_LEN);
		if (rc < 0){
			printk("lgcam_rear_sensor: reading af_status fail\n");
			return rc;
		}

		if(af_status == 8){
			if(debug_mask)
				printk("lgcam_rear_sensor: af_status is lock done\n");
			break;
		}
		
		if(af_status != 8){
			printk("lgcam_rear_sensor: af_status = %d, waiting untill af_status changes to lock done \n", af_status);
		}
		mdelay(30);
		num = num + 1;
	}
	
	lgcam_rear_sensor_check_af_lock_and_clear();
	
	rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x6D3A, &af_result, BYTE_LEN);
	if (rc < 0){
		printk("lgcam_rear_sensor: reading af_result fail\n");
		return rc;
	}

	rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x6D52, &af_result_ex, BYTE_LEN);
	if (rc < 0){
		printk("lgcam_rear_sensor: reading af_result fail_ex\n");
		return rc;
	}	

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002E, 0x03, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
				
	if((af_result == 0x01) || (af_result_ex == 0x01)){
		if(debug_mask)
			printk("lgcam_rear_sensor: Now is good focus\n");
		return 0;
	}

	if(debug_mask)
		printk("lgcam_rear_sensor: af is time out af_res = %d, af_res1 = %d\n",af_result, af_result_ex);

	return -ETIME;
}
#endif

static int32_t lgcam_rear_sensor_set_auto_focus(void)
{
	int32_t rc;

	if(debug_mask)
		printk("lgcam_rear_sensor: auto focus\n");

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x0252, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0005, WORD_LEN);

	if(rc<0){
		if(debug_mask)
			printk("lgcam_rear_sensor: AF auto focus writing fail!\n");
		return rc;
	}	
	return rc;
}

static int32_t lgcam_rear_sensor_set_macro_focus(void)
{
	int32_t rc;
	
	if(debug_mask)
		printk("lgcam_rear_sensor: macro focus\n");

	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x0252, WORD_LEN);
	rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0005, WORD_LEN);

	if(rc<0){
		if(debug_mask)
			printk("lgcam_rear_sensor: AF macro focus writing fail!\n");
		return rc;
	}	
	return rc;
}


static int32_t lgcam_rear_sensor_focus_config(int mode)
{
	int32_t rc = 0;

	if(prev_af_mode == FOCUS_RECT)
	{		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.focus_rect_reg_settings[0], lgcam_rear_sensor_regs.focus_rect_reg_settings_size);
	}

	{
		switch(mode){
			case FOCUS_AUTO:
			case FOCUS_NORMAL:
				rc = lgcam_rear_sensor_set_auto_focus();
				break;
			
			case FOCUS_MACRO:				
				rc = lgcam_rear_sensor_set_macro_focus();
				break;

			case FOCUS_MANUAL:
				//rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.manual_focus_reg_settings[0],
				//		lgcam_rear_sensor_regs.manual_focus_reg_settings_size);
				break;
			case FOCUS_CONTINUOUS_CAMERA:
			case FOCUS_CONTINUOUS_VIDEO:
				rc = lgcam_rear_sensor_set_continuous_focus();
				break;
				
			default:
				if(debug_mask)
				printk("[lgcam_rear_sensor.c] lgcam_rear_sensor_focus_config: invalid af %d\n",mode);
				return -EINVAL;
		}

	}

	prev_af_mode = mode;	
	return rc;		
}

static int32_t lgcam_rear_sensor_move_focus(int32_t steps)
{
	int32_t rc = 0;
	unsigned short manual_pos;

	prev_af_mode = FOCUS_MANUAL;
	focus_mode = FOCUS_MANUAL;
	if(debug_mask)
		printk("[lgcam_rear_sensor.c] move focus: step is %d\n",steps);
	
	/* MF ON */
	/* BEGIN: 0004908 hyungtae.lee@lge.com 2010-03-12 */
  	/* MOD: 0004908: [camera] change brightness on night mode when snap */ 

	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.manual_focus_reg_settings[0],
			lgcam_rear_sensor_regs.manual_focus_reg_settings_size);
	if(rc<0){
		printk("[lgcam_rear_sensor.c]%s: fail in writing for move focus\n",__func__);
		return rc;
	}
	/* END: 0004908 hyungtae.lee@lge.com 2010-03-12 */

	if(debug_mask)	
	printk("lgcam_rear_sensor: move focus steps = %d\n", steps);

//	steps++;
	switch(steps)
	{
		case 1:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x007C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);

			break;
		case 2:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0072, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);

			break;	
		case 3:			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x006C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);

			break;
		case 4:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0064, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);		
			break;	
		case 5:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x005E, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;
		case 6:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0057, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;	
		case 7:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0051, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);		
			break;
		case 8:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x004B, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);			
			break;	
		case 9:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0048, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;
		case 10:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0042, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;	
		case 11:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);					
			break;
		case 12:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;	
		case 13:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);			
			break;
		case 14:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);					
			break;	
		case 15:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);				
			break;
		case 16:
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x003C, WORD_LEN);
			mdelay(40);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x002A, 0x0252, WORD_LEN);			
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
					0x0F12, 0x0004, WORD_LEN);	

			break;
		default:
			manual_pos = 0x2c01;
			break;
	}

	
	if(debug_mask)
		printk("[lgcam_rear_sensor.c] manual position is 0x%x\n", manual_pos);

	
	return rc;
}

#if !SENSOR_TUNING_SET
static long lgcam_rear_sensor_set_effect(int effect)
{
	int32_t rc;

   switch (effect) {
	case CAMERA_EFFECT_OFF: 
		if(debug_mask)
			printk("lgcam_rear_sensor_set_effect: effect is OFF\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x021E, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0000, WORD_LEN);
		if (rc < 0)
			return rc;	
		
      break;

	case CAMERA_EFFECT_MONO: 
		if(debug_mask)
			printk("lgcam_rear_sensor_set_effect: effect is MONO\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x021E, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	

		break;

   case CAMERA_EFFECT_SEPIA: 
		if(debug_mask)
			printk("lgcam_rear_sensor_set_effect: effect is SEPIA\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x021E, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0004, WORD_LEN);
		if (rc < 0)
			return rc;	

		break;

	case CAMERA_EFFECT_NEGATIVE: 
		if(debug_mask)
			printk("lgcam_rear_sensor_set_effect: effect is NAGATIVE\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x021E, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0003, WORD_LEN);
		if (rc < 0)
			return rc;	

		break;

   case CAMERA_EFFECT_NEGATIVE_SEPIA:
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0028, 0x7000, WORD_LEN);
	   if (rc < 0)
		   return rc;
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x002A, 0x021E, WORD_LEN);
	   if (rc < 0)
		   return rc;  
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0F12, 0x0000, WORD_LEN);
	   if (rc < 0)
		   return rc;  

		break;

   case CAMERA_EFFECT_BLUE:
   case CAMERA_EFFECT_AQUA:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0028, 0x7000, WORD_LEN);
	   if (rc < 0)
		   return rc;
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x002A, 0x021E, WORD_LEN);
	   if (rc < 0)
		   return rc;  
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0F12, 0x0005, WORD_LEN);
	   if (rc < 0)
		   return rc;  

		break;

   case CAMERA_EFFECT_SOLARIZE: 
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0028, 0x7000, WORD_LEN);
	   if (rc < 0)
		   return rc;
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x002A, 0x021E, WORD_LEN);
	   if (rc < 0)
		   return rc;  
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0F12, 0x0000, WORD_LEN);
	   if (rc < 0)
		   return rc;  

		break;

   case CAMERA_EFFECT_PASTEL: 
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0028, 0x7000, WORD_LEN);
	   if (rc < 0)
		   return rc;
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x002A, 0x021E, WORD_LEN);
	   if (rc < 0)
		   return rc;  
	   rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			   0x0F12, 0x0000, WORD_LEN);
	   if (rc < 0)
			return rc;
		break;

   default: 
		printk("lgcam_rear_sensor_set_effect: wrong effect mode\n");
		return -EINVAL;	
	}
	
	return 0;
}
static long lgcam_rear_sensor_set_zoom_sensor(int zoom)
{
	int32_t rc;	
	
	if(debug_mask)
		printk("lgcam_rear_sensor_set_zoom_sensor : called, new zoom: %d\n", zoom);
		
	rc = lgcam_rear_sensor_set_capture_zoom(zoom);
	if(rc < 0){
		printk("lgcam_rear_sensor: lgcam_rear_sensor_set_zoom_sensor setting fail!\n");
		return rc;
	}
	
	lgcam_rear_sensor_ctrl->zoom = zoom;
	return rc;
}

static long lgcam_rear_sensor_set_capture_zoom(int zoom)
{
	int32_t rc;
	int zoom_depth = 16;
	
	if(debug_mask)
		printk("lgcam_rear_sensor_set_capture_zoom : called, new zoom: %d\n", zoom);

	if((lgcam_rear_sensor_ctrl->zoom % zoom_depth) != 0)
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,0x034F, 0x01, BYTE_LEN);
	else
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,0x034F, 0x00, BYTE_LEN);
	
	if(zoom >= 0 && zoom < zoom_depth)
		rc = lgcam_rear_sensor_i2c_write_table_one(&lgcam_rear_sensor_regs.zoom_mode_capture_405_reg_settings[0], zoom);
	else if(zoom >= zoom_depth && zoom < (zoom_depth * 2))
		rc = lgcam_rear_sensor_i2c_write_table_one(&lgcam_rear_sensor_regs.zoom_mode_capture_203_reg_settings[0], zoom - zoom_depth);
	else if(zoom >= (zoom_depth * 2) && zoom < (zoom_depth * 3))
		rc = lgcam_rear_sensor_i2c_write_table_one(&lgcam_rear_sensor_regs.zoom_mode_capture_162_reg_settings[0], zoom - (zoom_depth * 2));
	else if(zoom >= (zoom_depth * 3) && zoom < (zoom_depth * 4))
		rc = lgcam_rear_sensor_i2c_write_table_one(&lgcam_rear_sensor_regs.zoom_mode_capture_127_reg_settings[0], zoom - (zoom_depth * 3));
	else
		printk("lgcam_rear_sensor: lgcam_rear_sensor_set_capture_zoom wrong value\n");	

	if(rc<0){
		printk("lgcam_rear_sensor: lgcam_rear_sensor_set_capture_zoom setting fail!\n");
		return rc;
	}	
	return rc;
}


static long lgcam_rear_sensor_set_wb(int8_t wb)
{
	int32_t rc = 0;
   
	if(debug_mask)
		printk("lgcam_rear_sensor_set_wb : called, new wb: %d\n", wb);

	switch (wb) {
	case CAMERA_WB_AUTO:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_wb: wb is AUTO\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04D2, WORD_LEN);
		if (rc < 0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x067F, WORD_LEN);
		if (rc < 0)
			return rc;		
		break;

	case CAMERA_WB_INCANDESCENT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_wb: wb is INCANDESCENT\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04D2, WORD_LEN);
		if (rc < 0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0677, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04A0, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0400, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x040d, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0888, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		break;
	
	case CAMERA_WB_DAYLIGHT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_wb: wb is SUNNY\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04D2, WORD_LEN);
		if (rc < 0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0677, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04A0, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0530, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0400, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0590, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	

		break;

	case CAMERA_WB_FLUORESCENT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_wb: wb is FLUORESCENT\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04D2, WORD_LEN);
		if (rc < 0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0677, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04A0, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0490, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0400, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0760, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	

		break;

	case CAMERA_WB_CLOUDY_DAYLIGHT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_wb: wb is CLOUDY\n");
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0028, 0x7000, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04D2, WORD_LEN);
		if (rc < 0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0677, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x002A, 0x04A0, WORD_LEN);
		if (rc < 0)
			return rc;	

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0700, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0400, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0500, WORD_LEN);
		if (rc < 0)
			return rc;	
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
			0x0F12, 0x0001, WORD_LEN);
		if (rc < 0)
			return rc;	
		break;

	default:
		printk("lgcam_rear_sensor: wrong white balance value\n");
		return -EFAULT;
	}
	lgcam_rear_sensor_ctrl->wb = wb;

	return 0;
}

static int32_t lgcam_rear_sensor_set_iso(int8_t iso)
{
	int32_t rc = 0;	

	if(debug_mask)
		printk("lgcam_rear_sensor_set_iso : called, new iso: %d\n", iso);
	
	switch (iso){
	case CAMERA_ISO_AUTO:
		if(debug_mask)
			printk("[lgcam_rear_sensor.c] iso is AUTO\n");
	
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.iso_auto_reg_settings[0],
            lgcam_rear_sensor_regs.iso_auto_reg_settings_size);
		break;                                                    
                                                        
	case CAMERA_ISO_100:
		if(debug_mask)
			printk("[lgcam_rear_sensor.c] iso is 100\n");
		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.iso_100_reg_settings[0],
            lgcam_rear_sensor_regs.iso_100_reg_settings_size);
		break;

	case CAMERA_ISO_200:
		if(debug_mask)
			printk("[lgcam_rear_sensor.c] iso is 200\n");
		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.iso_200_reg_settings[0],
            lgcam_rear_sensor_regs.iso_200_reg_settings_size);
		break;

	case CAMERA_ISO_400:
		if(debug_mask)
			printk("[lgcam_rear_sensor.c] iso is 400\n");
		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.iso_400_reg_settings[0],
            lgcam_rear_sensor_regs.iso_400_reg_settings_size);
		break;
		
	case CAMERA_ISO_800:
		if(debug_mask)
			printk("[lgcam_rear_sensor.c] iso is 800\n");
		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.iso_800_reg_settings[0],
            lgcam_rear_sensor_regs.iso_800_reg_settings_size);
		break;

	default:
		printk("[lgcam_rear_sensor.c] incorrect iso value\n");
		rc = -EINVAL;
	}	
	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.prev_reg_settings[0], lgcam_rear_sensor_regs.prev_reg_settings_size);
	return rc;
}

static long lgcam_rear_sensor_set_scene_mode(int8_t mode)
{
	int32_t rc = 0;

	current_scene = NOT_NIGHT_MODE;
	switch (mode) {
	case CAMERA_SCENE_AUTO:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is normal\n");
			
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		break;
	
	case CAMERA_SCENE_PORTRAIT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is portrait\n");
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		if (rc < 0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_portrait_reg_settings[0],
                lgcam_rear_sensor_regs.scene_portrait_reg_settings_size);
     	break;
	
	case CAMERA_SCENE_LANDSCAPE:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is landscape\n");
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		if (rc < 0)
			return rc;
	
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_landscape_reg_settings[0],
                lgcam_rear_sensor_regs.scene_landscape_reg_settings_size);
		break;
	
	case CAMERA_SCENE_SPORTS:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is sport\n");
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		if (rc < 0)
			return rc;
	
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_sport_reg_settings[0],
                lgcam_rear_sensor_regs.scene_sport_reg_settings_size);
		break;
	
	case CAMERA_SCENE_SUNSET:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is sunset\n");
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		if (rc < 0)
			return rc;
	
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_sunset_reg_settings[0],
                lgcam_rear_sensor_regs.scene_sunset_reg_settings_size);
		break;
	
	case CAMERA_SCENE_NIGHT:
		if(debug_mask)
			printk("lgcam_rear_sensor_set_scene_mode: mode is night\n");
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_normal_reg_settings[0],
                lgcam_rear_sensor_regs.scene_normal_reg_settings_size);
		if (rc < 0)
			return rc;
	
		rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.scene_night_reg_settings[0],
                lgcam_rear_sensor_regs.scene_night_reg_settings_size);
		current_scene = NIGHT_MODE;
		break;
	
	default:
		printk("lgcam_rear_sensor: wrong scene mode value = %d, set to the normal\n", mode);
	}   
	if (rc < 0)
		return rc;

	lgcam_rear_sensor_ctrl->scene = mode;

	return rc;
   
}
/* BEGIN: 0005280 hyungtae.lee@lge.com 2010-03-22 */
/* MOD: 0005280: [camera] modification for brightness */ 
static int32_t lgcam_rear_sensor_set_brightness(int8_t ev)
{
	int32_t rc=0;
	ev++;	
	if(debug_mask)
		printk("lgcam_rear_sensor_set_brightness: ev is %d\n",ev);	
	switch (ev) {
	case 1:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0xFF60, WORD_LEN);
		if(rc<0)
			return rc;		
		break;

	case 2:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0xFF80, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 3:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0xFFA0, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 4:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0xFFC0, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 5:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0xFFE0, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 6:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0000, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 7:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0020, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 8:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0040, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 9:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0060, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 10:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x0080, WORD_LEN);
		if(rc<0)
			return rc;


		break;

	case 11:
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0028, 0x7000, WORD_LEN);
		if(rc<0)
			return rc;
		
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x002A, 0x020C, WORD_LEN);
		if(rc<0)
			return rc;

		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
				0x0F12, 0x00A0, WORD_LEN);
		if(rc<0)
			return rc;	

		break;
	
	default:
		printk("[lgcam_rear_sensor.c] incoreect brightness value\n");
	}
	
	lgcam_rear_sensor_ctrl->brightness = ev;
	return rc;
}
/* END: 0005280 hyungtae.lee@lge.com 2010-03-22 */
#endif //!SENSOR_TUNING_SET

#if SENSOR_TUNING_SET
#define LOOP_INTERVAL		20
#define IS_NUM(c)			((0x30<=c)&&(c<=0x39))
#define IS_CHAR_C(c)		((0x41<=c)&&(c<=0x46))						// Capital Letter
#define IS_CHAR_S(c)		((0x61<=c)&&(c<=0x66))						// Small Letter
#define IS_VALID(c)			(IS_NUM(c)||IS_CHAR_C(c)||IS_CHAR_S(c))		// NUM or CHAR
#define TO_BE_NUM_OFFSET(c)	(IS_NUM(c) ? 0x30 : (IS_CHAR_C(c) ? 0x37 : 0x57))	
#define TO_BE_READ_SIZE		 5000*40									// 8pages (4000x8)

char *file_buf_alloc_pages=NULL;
char file_buf_alloc_pages_temp[4]= {0,};

static long lgcam_rear_sensor_read_ext_reg(char *filename)
{	
	long value=0, length=ADDRESS_TUNE, read_idx=0, i=0, j=0, k=0;
	struct file *phMscd_Filp = NULL;
	mm_segment_t old_fs=get_fs();
	phMscd_Filp = filp_open(filename, O_RDONLY |O_LARGEFILE, 0);

	printk("%s : enter this function!\n", __func__);

	if (IS_ERR(phMscd_Filp)) {
		printk("%s : open error!\n", __func__);
		return 0;
	}

	file_buf_alloc_pages = kmalloc((int)TO_BE_READ_SIZE, GFP_KERNEL);	
	
	if(!file_buf_alloc_pages) {
		printk("%s : mem alloc error!\n", __func__);
		return 0;
	}

	set_fs(get_ds());
	phMscd_Filp->f_op->read(phMscd_Filp, file_buf_alloc_pages, TO_BE_READ_SIZE-1, &phMscd_Filp->f_pos);
	set_fs(old_fs);

	do
	{		
		if ((file_buf_alloc_pages[read_idx]=='0' && file_buf_alloc_pages[read_idx+1]=='x' 
			&& file_buf_alloc_pages[read_idx + 2] != ' ') &&
			(file_buf_alloc_pages[read_idx-2]=='{' 
|| file_buf_alloc_pages[read_idx-1]=='{' || 
			 file_buf_alloc_pages[read_idx-6]=='{' || file_buf_alloc_pages[read_idx-7]=='{' || 
			 file_buf_alloc_pages[read_idx-8]=='{' || file_buf_alloc_pages[read_idx-9]=='{' ||
			 file_buf_alloc_pages[read_idx-10]=='{' || file_buf_alloc_pages[read_idx-11]=='{' ||
			 file_buf_alloc_pages[read_idx-12]=='{' || file_buf_alloc_pages[read_idx-13]=='{')) {	// skip : 0x
			read_idx += 2;			
				if(file_buf_alloc_pages[read_idx-5]=='/' && file_buf_alloc_pages[read_idx-4]=='/')
				{ // comment skip routine
					read_idx += 20;
				}
				else
				{
					if(file_buf_alloc_pages[read_idx] == ',' || file_buf_alloc_pages[read_idx+1] == ',' ||
						file_buf_alloc_pages[read_idx+2] == ',') {// burst mode check (only address)

			if(length == ADDRESS_TUNE)
			{
							file_buf_alloc_pages_temp[0] = '0';
							file_buf_alloc_pages_temp[1] = 'F';
							file_buf_alloc_pages_temp[2] = '1';
							file_buf_alloc_pages_temp[3] = '2';
								
							value = (file_buf_alloc_pages_temp[0]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[0]))*0x1000 \
									+ (file_buf_alloc_pages_temp[1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[1]))*0x100\
									+ (file_buf_alloc_pages_temp[2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[2]))*0x10 \
										+ (file_buf_alloc_pages_temp[3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[3]));

							read_idx += 2;

							ext_reg_settings[i++].register_address = value;
#if SENSOR_TUNING_LOG
							printk("%s : length == ADDRESS_TUNE, i = %ld , burst mode\n", __func__, i);
#endif
						}				
					}
					else
					{
						if(length == ADDRESS_TUNE)
						{
							if((file_buf_alloc_pages[read_idx] == 'F' && file_buf_alloc_pages[read_idx+1] == 'F' &&
								file_buf_alloc_pages[read_idx+2] == 'F' && file_buf_alloc_pages[read_idx+3] == 'E') ||
								(file_buf_alloc_pages[read_idx] == 'F' && file_buf_alloc_pages[read_idx+1] == 'F' &&
								file_buf_alloc_pages[read_idx+2] == 'F' && file_buf_alloc_pages[read_idx+3] == 'F'))
							{
								file_buf_alloc_pages_temp[0] = '0';
								file_buf_alloc_pages_temp[1] = 'F';
								file_buf_alloc_pages_temp[2] = '1';
								file_buf_alloc_pages_temp[3] = '2';

								value = (file_buf_alloc_pages_temp[0]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[0]))*0x1000 \
									+ (file_buf_alloc_pages_temp[1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[1]))*0x100\
									+ (file_buf_alloc_pages_temp[2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[2]))*0x10 \
										+ (file_buf_alloc_pages_temp[3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages_temp[3]));

								read_idx += 2;
								
							}
							else
							{
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x1000 \
						+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x100\
						+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]));

								read_idx += 4;
							}
							
							ext_reg_settings[i++].register_address = value;
#if SENSOR_TUNING_LOG
							printk("%s : length == ADDRESS_TUNE, i = %ld\n", __func__, i);
#endif
						}
						else if(length == BYTE_LEN)
						{
							value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x10 \
										+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]));

							read_idx += 2;

				ext_reg_settings[j].register_value = value;
				ext_reg_settings[j++].register_length = length;
#if SENSOR_TUNING_LOG
				printk("%s : length == BYTE_LEN, j = %ld\n", __func__, j);
#endif
			}
						else if((length == WORD_LEN) || (length == BURST_LEN))
						{
							value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x1000 \
									+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x100\
									+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x10 \
										+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]));

							read_idx += 4;

							ext_reg_settings[j].register_value = value;
							ext_reg_settings[j++].register_length = WORD_LEN;
#if SENSOR_TUNING_LOG
							if(length == BURST_LEN)
								printk("%s : length == BURST_LEN, j = %ld\n", __func__, j);
							else
								printk("%s : length == WORD_LEN, j = %ld\n", __func__, j);
#endif
						}
						else if(length == DOBULE_LEN)
			{				
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x10000000 \
						+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x1000000\
						+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x100000 \
						+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]))*0x10000\
						+ (file_buf_alloc_pages[read_idx+4]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+4]))*0x1000 \
						+ (file_buf_alloc_pages[read_idx+5]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+5]))*0x100\
						+ (file_buf_alloc_pages[read_idx+6]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+6]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+7]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+7]));

							read_idx += 8;

							ext_reg_settings[j].register_value = value;
							ext_reg_settings[j++].register_length = length;
#if SENSOR_TUNING_LOG
							printk("%s : length == DOBULE_LEN, j = %ld\n", __func__, j);
#endif
						}				
						else{}
					}
					
			if(length == ADDRESS_TUNE)
			{			
				for(k=0; k < LOOP_INTERVAL; k++)
				{
					if(file_buf_alloc_pages[read_idx + k] == 'W')
					{
						length = WORD_LEN;
						break;
					}
					else if(file_buf_alloc_pages[read_idx + k] == 'Y')
					{
						length = BYTE_LEN;
						break;
					}			
							else if(file_buf_alloc_pages[read_idx + k] == 'L')
							{
								length = DOBULE_LEN;
								break;
							}
							else if(file_buf_alloc_pages[read_idx + k] == 'U')
							{
								length = BURST_LEN;
								break;
					}
				}
			}	
			else
			{
				length = ADDRESS_TUNE;
			}
		}
			}
		else
		{
			++read_idx;
		}	

#if 0 //SENSOR_TUNING_LOG		
		printk("%s : read external file! position : %d\n", __func__, read_idx);
#endif
	}while(file_buf_alloc_pages[read_idx] != '$');

	kfree(file_buf_alloc_pages);
	file_buf_alloc_pages=NULL;
	filp_close(phMscd_Filp,NULL);

	return i;
}

static long lgcam_rear_sensor_reg_write_ext_table(uint16_t array_length)
{
	int i = 0, rc = 0;	

#if SENSOR_TUNING_LOG
	printk("%s : write registers from external file!\n", __func__);
#endif

	for (i = 0; i < array_length; i++) {
		rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr,
		  ext_reg_settings[i].register_address,
		  ext_reg_settings[i].register_value,
		  ext_reg_settings[i].register_length);		 

#if SENSOR_TUNING_LOG
		printk("lgcam_rear_sensor_reg_write_ext_table, addr = 0x%x, val = 0x%x, width = %d\n",
			    ext_reg_settings[i].register_address, ext_reg_settings[i].register_value, ext_reg_settings[i].register_length);
#endif
		if (rc < 0) {
			printk("I2C Fail\n");
			return rc;
		}
	}

	return 1;
}

static long lgcam_rear_sensor_reg_pll_ext(void)
{
	uint16_t length = 0;	
	

	printk("%s : lgcam_rear_sensor_reg_pll_ext enter!!\n", __func__);
#if 1
	length = lgcam_rear_sensor_read_ext_reg("/sdcard/pll_settings_array.txt");
#else
	length = lgcam_rear_sensor_read_ext_reg("/data/local/pll_settings_array.txt");
#endif
	printk("%s : length = %d!\n", __func__, length);

	if (!length)
		return 0;
	else	
		return lgcam_rear_sensor_reg_write_ext_table(length);
}

static long lgcam_rear_sensor_reg_init_ext(void)
{	
	uint16_t length = 0;

	printk("%s : lgcam_rear_sensor_reg_init_ext enter!!\n", __func__);
#if 1
	length = lgcam_rear_sensor_read_ext_reg("/sdcard/init_settings_array.txt");
#else
	length = lgcam_rear_sensor_read_ext_reg("/data/local/init_settings_array.txt");
#endif
	printk("%s : length = %d!\n", __func__, length);	

	if (!length)
		return 0;
	else
		return lgcam_rear_sensor_reg_write_ext_table(length);
}
#endif

#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
static int dequeue_sensor_config(int cfgtype, int mode)
{
	int rc = 1;

	switch (cfgtype) {
			case CFG_SET_MODE:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_MODE\n");
					
				rc = lgcam_rear_sensor_set_sensor_mode(mode, pict_size.width, pict_size.height);
				break;
		
#if !SENSOR_TUNING_SET
			case CFG_SET_EFFECT:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_EFFECT\n");
					
				rc = lgcam_rear_sensor_set_effect(mode);
				break;
#endif
#if 0 
			case CFG_SET_ZOOM_VIDEO:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ZOOM\n");
					
				rc = lgcam_rear_sensor_set_zoom(cfg_data.cfg.zoom);
				break;
#endif


#if 1
#if !SENSOR_TUNING_SET
			case CFG_SET_ZOOM_SENSOR:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ZOOM_SENSOR\n");
					
				rc = lgcam_rear_sensor_set_zoom_sensor(mode);
				break;
		
			case CFG_SET_FOCUS_RECT:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_FOCUS_RECT\n");
					
				rc = lgcam_rear_sensor_set_focus_rect(mode);
				break;
#endif
				
			case CFG_START_AF_FOCUS:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_START_AF_FOCUS = %d\n", focus_mode);
				   
				focus_mode = mode;
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
				if(tuning_thread_run)
				{
					rc = lgcam_rear_sensor_check_thread_run();
					if(rc < 0)
					{
						printk("lgcam_rear_sensor_check_thread_run error\n");					
					}
					mdelay(500);
				}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE
				
				rc = lgcam_rear_sensor_focus_config(mode);		
//LGE_DEV_PORTING GELATO
				if((mode != FOCUS_CONTINUOUS_VIDEO) && (mode != FOCUS_CONTINUOUS_CAMERA))
					rc = lgcam_rear_sensor_check_focus(&cfg_data.mode);
//LGE_DEV_END
				break;
		
			case CFG_SET_PARM_AF_MODE:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_PARM_AF_MODE = %d, focus_mode = %d\n",  mode, focus_mode);
				   
				if((focus_mode == FOCUS_MACRO) && ((mode == FOCUS_AUTO) || (mode == FOCUS_CONTINUOUS_CAMERA)))
					rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.AF_reg_settings[0], lgcam_rear_sensor_regs.AF_reg_settings_size);
				focus_mode = mode;
				rc = lgcam_rear_sensor_cancel_focus(focus_mode);
				break;
		
			case CFG_SET_DEFAULT_FOCUS:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_DEFAULT_FOCUS\n");
				   
				rc = lgcam_rear_sensor_set_focus();
				break;
		
			case CFG_MOVE_FOCUS:
				
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_MOVE_FOCUS: steps=%d\n",
								mode);
				rc = lgcam_rear_sensor_move_focus(mode);
				break;	
		
			case CFG_SET_CANCEL_FOCUS:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_CANCEL_FOCUS\n");
				   
				rc = lgcam_rear_sensor_cancel_focus(focus_mode);
				break;
					
#endif
		
#if !SENSOR_TUNING_SET		
			case CFG_SET_WB:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_WB\n");
					
				rc = lgcam_rear_sensor_set_wb(mode);
				break;
		
			case CFG_SET_ISO:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ISO\n");
				rc = lgcam_rear_sensor_set_iso(mode);
				break;
		
			case CFG_SET_SCENE:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_SCENE_MODE\n");
				rc = lgcam_rear_sensor_set_scene_mode(mode);
				break;
		
			case CFG_SET_BRIGHTNESS:
				if(debug_mask)
					printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_EXPOSURE_VALUE\n");
				rc = lgcam_rear_sensor_set_brightness(mode);
				break;
#endif	  
			//default:
			//	rc = -EFAULT;

	}

	return 0;
}

static void dequeue_cfg_wq(struct config_work_queue *cfg_wq)
{
	int rc;
	int i;

	for (i = 0; i < cfg_wq_num; ++i) {
		if(debug_mask)
			printk("dequeue cfgtype = %d, mode = %d \n", cfg_wq[i].cfgtype, cfg_wq[i].mode);
		rc = dequeue_sensor_config(cfg_wq[i].cfgtype, cfg_wq[i].mode);
		if (rc < 0) {
			printk(KERN_ERR "[ERROR]%s: dequeue sensor config error!\n",
				__func__);
			return;
		}
	}

	enable_capturemode = 1;
	cfg_wq_num = 0;
}
static void enqueue_cfg_wq(int cfgtype, int mode)
{
	enable_capturemode = 0;

	if(debug_mask)
	printk("enqueue cfg_wq_num = %d,cfgtype = %d, mode = %d \n", cfg_wq_num, cfgtype, mode);

	if(cfg_wq_num == CFG_WQ_SIZE)
		return;

	cfg_wq[cfg_wq_num].cfgtype = cfgtype;
	cfg_wq[cfg_wq_num].mode= mode;

	++cfg_wq_num;
}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE


#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
int lgcam_rear_sensor_reg_tuning(void *data)
{
	int rc = 0;

	mutex_lock(&lgcam_rear_sensor_tuning_mutex);
	cfg_wq = kmalloc(sizeof(struct config_work_queue) * CFG_WQ_SIZE,
		GFP_KERNEL);
	cfg_wq_num = 0;
	tuning_thread_run = 1;
	mutex_unlock(&lgcam_rear_sensor_tuning_mutex);

	printk("lgcam_rear_sensor_reg_tuning write table start lgcam_rear_sensor_regs.init_size = %d\n",lgcam_rear_sensor_regs.init_size);	//mhlee

#if SENSOR_TUNING_SET
	rc = lgcam_rear_sensor_reg_init_ext();
#else
	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.init[0], lgcam_rear_sensor_regs.init_size);	

#endif
	printk("lgcam_rear_sensor_reg_tuning write table rc = %d\n",rc);	//mhlee
	if(rc<0){
		printk("lgcam_rear_sensor: init writing failed\n");
		return rc; 
	}

	mutex_lock(&lgcam_rear_sensor_tuning_mutex);
	dequeue_cfg_wq(cfg_wq);
	kfree(cfg_wq);
	tuning_thread_run = 0;
	mutex_unlock(&lgcam_rear_sensor_tuning_mutex);

	return rc;
}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE	

int lgcam_rear_sensor_sensor_init(const struct msm_camera_sensor_info *data)
{
	int rc;	
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
	struct task_struct *p;
	enable_capturemode = 0;
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE
#if SENSOR_TUNING_SET
memset(ext_reg_settings, 0x00, sizeof(ext_reg_settings));
#endif

	if(debug_mask)
	printk("lgcam_rear_sensor_init start! mmm\n");

	prev_af_mode = -1;
	//prev_scene_mode = -1;
	previous_mode = 0;
	memset(&pict_size,0x0, sizeof(pict_size));
	
	lgcam_rear_sensor_ctrl = kzalloc(sizeof(struct lgcam_rear_sensor_ctrl), GFP_KERNEL);
	if (!lgcam_rear_sensor_ctrl) {
		printk("lgcam_rear_sensor_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	sensor_burst_buffer = kzalloc(sensor_burst_size*2, GFP_KERNEL);	//mhlee
	if (!sensor_burst_buffer) {
		printk("sensor_burst_buffer failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}
	

	if (data)
		lgcam_rear_sensor_ctrl->sensordata = data;
	
	rc = lgcam_rear_sensor_ctrl->sensordata->pdata->camera_power_on();
	if(rc<0){
	   printk("lgcam_rear_sensor: pll writing fail\n");
	   goto init_fail;
	}


#if SENSOR_TUNING_SET
	lgcam_rear_sensor_reg_pll_ext();
#endif
	mdelay(16);  // T3+T4

	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.pll[0], lgcam_rear_sensor_regs.pll_size);
	
	mdelay(10);
#if !SENSOR_TUNING_SET	//0105 //mhlee
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE	
	p = kthread_run(lgcam_rear_sensor_reg_tuning, 0, "reg_tunning");
	if(IS_ERR(p))
	{
		printk("lgcam_rear_sensor: init writing failed\n");
		goto init_fail; 
	}
	printk("lgcam_rear_sensor: init writing done\n");	
	cfg_wq = 0;
	tuning_thread_run = 0;
	statuscheck = 1;	

#else
	init_burst_mode = 1;
	rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.init[0], lgcam_rear_sensor_regs.init_size);
	init_burst_mode = 0;
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE	
#else
	rc = lgcam_rear_sensor_reg_init_ext();

	printk("isx005_reg_init_ext write table rc = %d\n",rc);
	if(rc<0){
		printk("isx005: init writing_ext failed\n");
		goto init_fail;
	}
#endif

	return rc;

init_done:
	return rc;

init_fail:
	printk("lgcam_rear_sensor: lgcam_rear_sensor_sensor_init failed\n");
	kfree(lgcam_rear_sensor_ctrl);
	kfree(sensor_burst_buffer);
	return rc;
}

static int lgcam_rear_sensor_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&lgcam_rear_sensor_wait_queue);
	return 0;
}

int lgcam_rear_sensor_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;	
	long  rc = 0;	
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
	uint16_t err_info = 0;
	uint16_t errprev_info = 0;
	uint16_t errcap_info = 0;
#endif
	
	rc = copy_from_user(&cfg_data,(void *)argp,sizeof(struct sensor_cfg_data));
	if(rc < 0)
	{
		printk("lgcam_rear_sensor_ioctl mhlee cfgtype = %d, mode = %d\n",
			cfg_data.cfgtype, cfg_data.mode);
		return -EFAULT;
	}
	if(debug_mask)
		printk("lgcam_rear_sensor_ioctl m, cfgtype = %d, mode = %d\n",
			cfg_data.cfgtype, cfg_data.mode);

#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
	mutex_lock(&lgcam_rear_sensor_tuning_mutex);
	if (statuscheck == 1) {		
		int i=0;	
		if(debug_mask)
			printk("Sensor Preview Mode check : start \n");
		// enable상태가 가능한지 check 
		for(i = 0; i < 200; i++) {

			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002C, 0x7000, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002E, 0x020A, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x0F12, &err_info, WORD_LEN);	
			if(debug_mask)
				printk("Sensor Preview Mode check : %d-> success \n", err_info);


			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002C, 0x7000, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002E, 0x0242, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x0F12, &errprev_info, WORD_LEN);	
			if(debug_mask)
				printk("Sensor Preview Mode check : %d-> success \n", errprev_info);

			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002C, 0x7000, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002E, 0x0248, WORD_LEN);		
			rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr, 0x0F12, &errcap_info, WORD_LEN);	
			if(debug_mask)
				printk("Sensor Preview Mode check : %d-> success \n", errcap_info);
			
			if((errcap_info == 0)&&(errprev_info == 0)&&(errcap_info == 0))
			{		
				break;
			}
			else
			{
				msleep(5);
			}
		}
		statuscheck = 0;
	}
	mutex_unlock(&lgcam_rear_sensor_tuning_mutex);
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE

	//if(cfg_data.cfgtype != CFG_SET_MODE)
	//	return 0;	//mhlee 0105	
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE	
	mutex_lock(&lgcam_rear_sensor_tuning_mutex);
	if ((tuning_thread_run) && (cfg_data.cfgtype != CFG_GET_CHECK_SNAPSHOT)) {
		if (cfg_data.cfgtype == CFG_MOVE_FOCUS)
			cfg_data.mode = cfg_data.cfg.focus.steps;

		if((cfg_data.cfgtype == CFG_SET_MODE) && (cfg_data.mode == SENSOR_SNAPSHOT_MODE ||
			cfg_data.mode == SENSOR_RAW_SNAPSHOT_MODE))
		{
			pict_size.width = cfg_data.width;
			pict_size.height = cfg_data.height;
		}
			
		enqueue_cfg_wq(cfg_data.cfgtype, cfg_data.mode);		 
		mutex_unlock(&lgcam_rear_sensor_tuning_mutex);
		return 0;
	}
	mutex_unlock(&lgcam_rear_sensor_tuning_mutex);
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE

	mutex_lock(&lgcam_rear_sensor_mutex);
#if defined (TOUCH_FOCUS_TEST)
if(cfg_data.cfgtype == CFG_SET_PARM_AF_MODE)
cfg_data.cfgtype = CFG_SET_FOCUS_RECT;
#endif
	switch (cfg_data.cfgtype) {
	case CFG_SET_MODE:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_MODE\n");

		pict_size.width = cfg_data.width;
		pict_size.height = cfg_data.height;		
		rc = lgcam_rear_sensor_set_sensor_mode(cfg_data.mode,cfg_data.width,cfg_data.height);
		break;

#if !SENSOR_TUNING_SET
	case CFG_SET_EFFECT:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_EFFECT\n");
			
		rc = lgcam_rear_sensor_set_effect(cfg_data.cfg.effect);
		break;
#endif
#if 0 
    case CFG_SET_ZOOM_VIDEO:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ZOOM\n");
			
		rc = lgcam_rear_sensor_set_zoom(cfg_data.cfg.zoom);
		break;
#endif

#if !SENSOR_TUNING_SET
	case CFG_SET_ZOOM_SENSOR:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ZOOM_SENSOR\n");
			
		rc = lgcam_rear_sensor_set_zoom_sensor(cfg_data.mode);
		break;

	case CFG_SET_FOCUS_RECT:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_FOCUS_RECT\n");
			
		rc = lgcam_rear_sensor_set_focus_rect(cfg_data.mode);
		break;
#endif
		
	case CFG_START_AF_FOCUS:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_START_AF_FOCUS = %d\n", focus_mode);
		   
		focus_mode = cfg_data.mode;
		
		rc = lgcam_rear_sensor_focus_config(cfg_data.mode);		
//LGE_DEV_PORTING GELATO
		if((focus_mode != FOCUS_CONTINUOUS_VIDEO)&&(focus_mode != FOCUS_CONTINUOUS_CAMERA))
			rc = lgcam_rear_sensor_check_focus(&cfg_data.mode);
//LGE_DEV_END		
		break;

	case CFG_SET_PARM_AF_MODE:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_PARM_AF_MODE1 = %d\n", focus_mode);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x116A, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x0032, WORD_LEN);
		if(cfg_data.mode == FOCUS_AUTO)
		{
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.AF_reg_settings[0], lgcam_rear_sensor_regs.AF_reg_settings_size);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CA, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00C0, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CE, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00BE, WORD_LEN); 
		}
		else if(cfg_data.mode == FOCUS_CONTINUOUS_VIDEO)
		{
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.CAF_reg_settings[0], lgcam_rear_sensor_regs.CAF_reg_settings_size);	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CA, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00F0, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CE, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00F0, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x116A, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x000A, WORD_LEN);

		}
		else if(cfg_data.mode == FOCUS_CONTINUOUS_CAMERA)
		{
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.AF_reg_settings[0], lgcam_rear_sensor_regs.AF_reg_settings_size);	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0028, 0x7000, WORD_LEN);
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CA, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00C0, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x002A, 0x10CE, WORD_LEN); 	
			rc = lgcam_rear_sensor_i2c_write(lgcam_rear_sensor_client->addr, 0x0F12, 0x00BE, WORD_LEN); 

		}			
		focus_mode = cfg_data.mode;
		rc = lgcam_rear_sensor_cancel_focus(focus_mode);
		break;

	case CFG_SET_DEFAULT_FOCUS:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_DEFAULT_FOCUS\n");
		   
		//rc = lgcam_rear_sensor_set_focus();
		break;

	case CFG_MOVE_FOCUS:
		
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_MOVE_FOCUS: steps=%d\n",
		    			cfg_data.cfg.focus.steps);
		rc = lgcam_rear_sensor_move_focus(cfg_data.cfg.focus.steps);
		break;	

	case CFG_SET_CANCEL_FOCUS:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_CANCEL_FOCUS\n");
		   
        rc = lgcam_rear_sensor_cancel_focus(focus_mode);
		break;
		 	
    case CFG_GET_AF_MAX_STEPS:
		cfg_data.max_steps = 20;
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_CHECK_AF_DONE:
		rc = lgcam_rear_sensor_status_continuous_af(&cfg_data.mode);
		if (copy_to_user((void *)argp,
				&cfg_data,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;

		if (debug_mask)
			printk("lgcam_rear_sensor: CFG_CHECK_AF_DONE, %ld\n", rc);

		mutex_unlock(&lgcam_rear_sensor_mutex);
		return rc;
		break;

#if !SENSOR_TUNING_SET		
	case CFG_SET_WB:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_WB\n");
			
		rc = lgcam_rear_sensor_set_wb(cfg_data.mode);
		break;

	case CFG_SET_ISO:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_ISO\n");
		rc = lgcam_rear_sensor_set_iso(cfg_data.mode);
		break;

	case CFG_SET_SCENE:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_SCENE_MODE\n");
		rc = lgcam_rear_sensor_set_scene_mode(cfg_data.mode);
		break;

	case CFG_SET_BRIGHTNESS:
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_EXPOSURE_VALUE\n");
		rc = lgcam_rear_sensor_set_brightness(cfg_data.mode);
		break;
#endif
	case CFG_GET_CURRENT_ISO:
	{
		uint16_t iso_value = 0;
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_GET_CURRENT_ISO\n");
		rc = lgcam_rear_sensor_i2c_read(lgcam_rear_sensor_client->addr,
		0x00F0, &iso_value, BYTE_LEN);
		cfg_data.mode = iso_value;
		if(debug_mask)
		printk("lgcam_rear_sensor_sensor_config: iso current value = %d\n", iso_value);
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
	}
		break;

	case CFG_GET_CHECK_SNAPSHOT:
	{		
		if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_GET_CHECK_SNAPSHOT\n");		
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
		cfg_data.mode = enable_capturemode;
		if(debug_mask)
		printk("lgcam_rear_sensor_sensor_config: enable_capturemode = %d\n", enable_capturemode);
#else
		cfg_data.mode = 1;
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
	}
		break;

	case CFG_SET_FPS:
	//	if(debug_mask)
			printk("lgcam_rear_sensor_sensor_config: command is CFG_SET_FPS mode = %d, auto:0, fixed:1\n",cfg_data.mode);
		
		if(cfg_data.mode == 0)
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.auto_frame_reg_settings[0],lgcam_rear_sensor_regs.auto_frame_reg_settings_size);
		else
			rc = lgcam_rear_sensor_i2c_write_table(&lgcam_rear_sensor_regs.fixed_frame_reg_settings[0],lgcam_rear_sensor_regs.fixed_frame_reg_settings_size);
		
		break;

#if !SENSOR_TUNING_SET  
	default:
		rc = -EFAULT;
#endif
	}
	
	mutex_unlock(&lgcam_rear_sensor_mutex);
	if (rc < 0)
		printk("lgcam_rear_sensor: ERROR in sensor_config, %ld\n", rc);

	if(debug_mask)
	    printk("lgcam_rear_sensor_sensor_config: end [%lu]\n", jiffies);
	
	return rc;
 	
}


int lgcam_rear_sensor_sensor_release(void)
{
	int rc = 0;
	mutex_lock(&lgcam_rear_sensor_mutex);
#if defined LGCAM_REAR_SENSOR_THREAD_ENABLE
	rc = lgcam_rear_sensor_check_thread_run();
	if (rc < 0)
	{
		printk("lgcam_rear_sensor_check_thread_run error\n");
	}
#endif //LGCAM_REAR_SENSOR_THREAD_ENABLE

	rc = lgcam_rear_sensor_ctrl->sensordata->pdata->camera_power_off();

	kfree(lgcam_rear_sensor_ctrl);
	kfree(sensor_burst_buffer);

	mutex_unlock(&lgcam_rear_sensor_mutex);

	lgcam_rear_sensor_ctrl=NULL;
	
	return rc;
}

static int lgcam_rear_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	lgcam_rear_sensor_sensorw = kzalloc(sizeof(struct lgcam_rear_sensor_work), GFP_KERNEL);

	if (!lgcam_rear_sensor_sensorw) {
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, lgcam_rear_sensor_sensorw);
	lgcam_rear_sensor_init_client(client);
	lgcam_rear_sensor_client = client;

	if(debug_mask)
		printk("lgcam_rear_sensor: lgcam_rear_sensor_probe succeeded!\n");

	return 0;
	
probe_failure:
	kfree(lgcam_rear_sensor_sensorw);
	lgcam_rear_sensor_sensorw = NULL;
	printk("lgcam_rear_sensor_probe failed!\n");
	return rc;
}

static const struct i2c_device_id lgcam_rear_sensor_i2c_id[] = {
	{ "s5k5caga", 0},
	{ },
};

static struct i2c_driver lgcam_rear_sensor_i2c_driver = {
	.id_table = lgcam_rear_sensor_i2c_id,
	.probe  = lgcam_rear_sensor_i2c_probe,
	.remove = __exit_p(lgcam_rear_sensor_i2c_remove),
	.driver = {
		.name = "s5k5caga",
	},
};

static int lgcam_rear_sensor_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&lgcam_rear_sensor_i2c_driver);
	printk("lgcam_rear_sensor: lgcam_rear_sensor_sensor_probe\n");	


	if (rc < 0 || lgcam_rear_sensor_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;

	}

#if 0	
	lgcam_rear_sensor_sensor_power_enable();
	mdelay(1);
#endif	

#if 0
	rc = lgcam_rear_sensor_reset(info,1);
	if (rc < 0) {
		printk("lgcam_rear_sensor: reset failed!\n");
	}
	mdelay(10);

	rc = lgcam_rear_sensor_sensor_init_probe(info);
	if (rc < 0)
		goto probe_done;
	
	mdelay(10);

	rc = lgcam_rear_sensor_pwdn(info,1);
	if (rc < 0) {
		printk("lgcam_rear_sensor: pwdn failed!\n");
	}
	mdelay(10);
	
	rc = lgcam_rear_sensor_pwdn(info,0);
	if (rc < 0) {
		printk("lgcam_rear_sensor: pwdn failed!\n");
	}
	mdelay(1);
	
	rc = lgcam_rear_sensor_reset(info,0);
	if (rc < 0) {
		printk("lgcam_rear_sensor: reset failed!\n");
	}
	mdelay(1);
	lgcam_rear_sensor_sensor_power_disable();
#endif
	s->s_init = lgcam_rear_sensor_sensor_init;
	s->s_release = lgcam_rear_sensor_sensor_release;
	s->s_config  = lgcam_rear_sensor_sensor_config;
//LGE_DEV_PORTING
	s->s_mount_angle  = 0;
//LGE_DEV_END

probe_done:
	printk("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __lgcam_rear_sensor_probe(struct platform_device *pdev)
{
	printk("lgcam_rear_sensor: __lgcam_rear_sensor_probe\n");

	return msm_camera_drv_start(pdev, lgcam_rear_sensor_sensor_probe);	
}

static struct platform_driver msm_camera_driver = {
	.probe = __lgcam_rear_sensor_probe,
	.driver = {
		.name = "msm_camera_s5k5caga", //board-gelato-camera.c(platform_device name)
		.owner = THIS_MODULE,
	},
};

static int __init lgcam_rear_sensor_init(void)
{
	return platform_driver_register(&msm_camera_driver);  
}

late_initcall(lgcam_rear_sensor_init);
