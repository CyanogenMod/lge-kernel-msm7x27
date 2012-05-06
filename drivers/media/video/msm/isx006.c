/* drivers/media/video/msm/isx006.c 
*
* This software is for SONY 3M sensor 
*  
* Copyright (C) 2009-2011 LGE Inc.  
* Author: Hyungtae Lee <leehyungtae>  
* (GPL License)  
* This software is licensed under the terms of the GNU General Public  
* License version 2, as published by the Free Software Foundation, and  
* may be copied, distributed, and modified under those terms.  
*  
* This program is distributed in the hope that it will be useful,  
* but WITHOUT ANY WARRANTY; without even the implied warranty of  
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
* GNU General Public License for more details. 
*/


#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include "isx006.h"
#include <mach/vreg.h>
#include <linux/byteorder/little_endian.h>
#include <linux/kthread.h>

/***********************************************************/
// 값이 set되면 sdcard에서 register값을 읽어서 카메라 초기화작업을 실시한다.
// 파일의 마지막은 항상 $ 로 마무리 해야 한다..( 조건문에 걸려잇다.)
#define ALESSI_TUNING_SET										0 // (0 : 일반적인 버젼 1: 튜닝을 위해서 setting)

//ALESSI_TUNING_SET == 1이어야만 의미가 잇다.
#define ALESSI_TUNING_LOG										0 // (0 : log를 생성안한다. 1: debugging을 위해 log set)
/***********************************************************/

/***********************************************************/
/** isx006reg.c , Qualcommhardware.cpp 도 같이 change해라..*/
#define ALESSI_JPEG_IF_SET										0 // (0 : YUV mode , 1 : Jpeg mode )

/** isx006reg.c , isx006.c(vendor), QualcommHardware.cpp 도 같이 change해라..*/
#if ALESSI_JPEG_IF_SET
#define ALESSI_JPEG_USE_ADDRESS									1 // (0 : normal parsing , 1 : parsing to use address )
#else
#define ALESSI_JPEG_USE_ADDRESS									0
#endif
/***********************************************************/	

/* Sensor Core Registers */
#define  REG_ISX006_MODEL_ID 0x0000
#define  ISX006_MODEL_ID     0x0520

/* BEGIN: 0004743 hyungtae.lee 2010-03-08 */
/* MOD: 0004743: [camera] modification of capture mode on night mode */ 
#define PREVIEW_MODE 0
#define CAPTURE_MODE 1

#define NOT_NIGHT_MODE 0
#define NIGHT_MODE 1

int current_scene = 0;
int previous_mode = 0;
/* END: 0004743 hyungtae.lee 2010-03-08 */

int isx006_mclk_rate = 27000000;

int focus_mode = 0;
static int debug_mask = 1;

static unsigned char* read_buffer_isx006 = NULL;
static int prev_af_mode;
static int enable_capturemode = 0;

#if ALESSI_TUNING_SET
static struct isx006_register_address_value_pair  ext_reg_settings[4000] = {0,};
#endif
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_ALESSI)
/* _S. Change code to apply new LUT for display quality. 2010-08-13. minjong.gong */
extern mdp_load_thunder_lut(int lut_type);
#endif

module_param_named(debug_mask, debug_mask, int, S_IRUGO|S_IWUSR|S_IWGRP);
struct isx006_work {
	struct work_struct work;
};

static struct  isx006_work *isx006_sensorw;
static struct  i2c_client *isx006_client;

struct isx006_ctrl {
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

static struct isx006_ctrl *isx006_ctrl;

static DECLARE_WAIT_QUEUE_HEAD(isx006_wait_queue);
DECLARE_MUTEX(isx006_sem);

static long isx006_set_capture_zoom(int zoom);
DEFINE_MUTEX(isx006_tuning_mutex);
static int tuning_thread_run = 0;
static int statuscheck = 0;
#define CFG_WQ_SIZE		64

struct config_work_queue {
	int cfgtype;
	int mode;
};

static struct config_work_queue *cfg_wq;
static int cfg_wq_num;

struct current_pict_size{
	int width;
	int height;
};
static struct current_pict_size pict_size;

DEFINE_MUTEX(isx006_mutex);

/*=============================================================
	EXTERNAL DECLARATIONS
==============================================================*/
extern struct isx006_reg isx006_regs;


/*=============================================================*/

static int32_t isx006_i2c_txdata(unsigned short saddr,
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

	if (i2c_transfer(isx006_client->adapter, msg, 1) < 0 ) {
		printk("isx006_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t isx006_i2c_write(unsigned short saddr,
	unsigned short waddr, uint32_t wdata, enum isx006_width width)
{
	int i = 0, fail_count = 0;
	int retry ;
	int32_t rc = -EIO;
	unsigned char buf_dbl[6];
	unsigned char buf_wrd[4];
	unsigned char buf_bte[3];
	udelay(5);	
	switch (width) {
	case DOBULE_LEN: {		
		memset(buf_dbl, 0, sizeof(buf_dbl));
		buf_dbl[0] = (waddr & 0xFF00)>>8;
		buf_dbl[1] = (waddr & 0x00FF);
		buf_dbl[2] = (wdata & 0xFF000000)>>24;
		buf_dbl[3] = (wdata & 0x00FF0000)>>16;
		buf_dbl[4] = (wdata & 0x0000FF00)>>8;
		buf_dbl[5] = (wdata & 0x000000FF);
		rc = isx006_i2c_txdata(saddr, buf_dbl, 6);
	}
		break;
		
	case WORD_LEN: {		
		memset(buf_wrd, 0, sizeof(buf_wrd));
		buf_wrd[0] = (waddr & 0xFF00)>>8;
		buf_wrd[1] = (waddr & 0x00FF);
		buf_wrd[2] = (wdata & 0xFF00)>>8;
		buf_wrd[3] = (wdata & 0x00FF);
		rc = isx006_i2c_txdata(saddr, buf_wrd, 4);
	}
		break;

	case BYTE_LEN: {		
		memset(buf_bte, 0, sizeof(buf_bte));
		buf_bte[0] = (waddr & 0xFF00)>>8;
		buf_bte[1] = (waddr & 0x00FF);
		buf_bte[2] = wdata;
		rc = isx006_i2c_txdata(saddr, buf_bte, 3);
	}
		break;
	}

// I2C fail발생이 되는 경우가 잇어서 방어추가한다.
#if ALESSI_TUNING_SET
	if(rc < 0){
		if(debug_mask)   
			printk("isx006_i2c_write first error!!!\n");
		
		for(retry = 0; retry < 3; retry++){
			if(width == DOBULE_LEN)
				rc = isx006_i2c_txdata(saddr, buf_dbl, 6);
			else if(width == WORD_LEN)
				rc = isx006_i2c_txdata(saddr, buf_wrd, 4);
			else if(width == BYTE_LEN)
				rc = isx006_i2c_txdata(saddr, buf_bte, 3);   				

			if(rc >= 0)
			{
				fail_count = 0;
               	retry = 3;
			}
			else
			{				
				fail_count = fail_count + 1;
					if(debug_mask)   
						printk("isx006_i2c_write #%d\n", fail_count);
			}
		}
	}
#endif

	if (rc < 0){
		printk("i2c_write failed, addr = 0x%x, val = 0x%x!\n",waddr, wdata);
	}
	return rc;	
}

static int32_t isx006_i2c_write_table(
	struct isx006_i2c_reg_conf const *reg_conf_tbl,
	int num_of_items_in_table)
{
	int i = 0, fail_count = 0;
	int retry ;
	int32_t rc = -EIO;
	
	for (i = 0; i < num_of_items_in_table; i++) {
		rc = isx006_i2c_write(isx006_client->addr,
		reg_conf_tbl->waddr, reg_conf_tbl->wdata,
		reg_conf_tbl->width);
		
		if (reg_conf_tbl->mdelay_time != 0)
			mdelay(reg_conf_tbl->mdelay_time);
     
		if(rc < 0){
    		for(retry = 0; retry < 3; retry++){
   				rc = isx006_i2c_write(isx006_client->addr,
		    			reg_conf_tbl->waddr, reg_conf_tbl->wdata,
		    			reg_conf_tbl->width);
           
            	if(rc >= 0)
               	retry = 3;
            
            	fail_count = fail_count + 1;
				if(debug_mask)   
					printk("isx006: i2c fail #%d\n", fail_count);
	
         	}
         	reg_conf_tbl++;
		}else
         	reg_conf_tbl++;
	}
	return rc;
}

static int32_t isx006_i2c_write_table_one(
	struct isx006_i2c_reg_conf const *reg_conf_tbl,	int selected)
{
	int i = 0, fail_count = 0;
	int retry ;
	int32_t rc = -EIO;

	if(selected != 0){
		for (i = 0; i < selected; i++) {
			reg_conf_tbl++;
		}
	}
	
	rc = isx006_i2c_write(isx006_client->addr,
	reg_conf_tbl->waddr, reg_conf_tbl->wdata,
	reg_conf_tbl->width);
	
	if (reg_conf_tbl->mdelay_time != 0)
		mdelay(reg_conf_tbl->mdelay_time);
 
	if(rc < 0){
		for(retry = 0; retry < 3; retry++){
				rc = isx006_i2c_write(isx006_client->addr,
	    			reg_conf_tbl->waddr, reg_conf_tbl->wdata,
	    			reg_conf_tbl->width);
       
        	if(rc >= 0)
           	retry = 3;
        
        	fail_count = fail_count + 1;
			if(debug_mask)   
				printk("isx006: isx006_i2c_write_table_one :: i2c fail #%d\n", fail_count);
     	}     	
	}	
	return rc;
}

static int isx006_i2c_rxdata(unsigned short saddr,
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

	if (i2c_transfer(isx006_client->adapter, msgs, 2) < 0) {
		printk("isx006_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t isx006_i2c_read(unsigned short   saddr,
	unsigned short raddr, unsigned short *rdata, enum isx006_width width)
{
	int32_t rc = 0;
	unsigned char buf[4];
	printk("isx006: isx006_i2c_read\n");

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	switch (width) {
	case WORD_LEN: {
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = isx006_i2c_rxdata(saddr, buf, 2);
		if (rc < 0)
			return rc;

		*rdata = buf[0] << 8 | buf[1];
		}
		break;
   	
   case BYTE_LEN:{
		buf[0] = (raddr & 0xFF00)>>8;
		buf[1] = (raddr & 0x00FF);

		rc = isx006_i2c_rxdata(saddr, buf, 2);
		if (rc < 0)
			return rc;
		
		*rdata = buf[0];
		}
		break;           
	}

	if (rc < 0)
		printk("isx006_i2c_read failed!\n");
   
	return rc;
}

static int isx006_check_thread_run(void)
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

static int isx006_reset(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_reset, "isx006");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_reset, value);
	else{
		printk("isx006_reset: reset gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_reset);
	return rc;
}

static int isx006_pwdn(const struct msm_camera_sensor_info *dev, int value)
{
	int rc = 0;
	
	rc = gpio_request(dev->sensor_pwd, "isx006");
	if (!rc) 
		rc = gpio_direction_output(dev->sensor_pwd, value);
	else{
		printk("isx006_pwdn: pwdn gpio_direction_output fail\n");
		return rc;
	}

	gpio_free(dev->sensor_pwd);
	return rc;
}

void isx006_sensor_power_enable(void)
{
	isx006_ctrl->sensordata->pdata->camera_power_on();
}

void isx006_sensor_power_disable(void)
{
	isx006_ctrl->sensordata->pdata->camera_power_off();
}

static long isx006_snapshot_config(int mode,int width, int height)
{
	int32_t rc;
	int picture_width, picture_height;
	
	if(debug_mask)
	   printk("isx006_snapshot_config:	Input resolution[%d * %d]\n", width, height);

	// 16배수의 경우는 cpu_to_be16를 이용해서 적용하면 되지만 1944가 16배수가 아니라서 1936으로 들어가는걸 방지.
	if((width == 2592) && (height == 1944))
	{
		rc = isx006_i2c_write(isx006_client->addr, 0x0024, 0x200A, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x002A, 0x9807, WORD_LEN);
		if (rc < 0)
			return rc;		
	}
	else
	{
		picture_width  = cpu_to_be16(width);
		picture_height = cpu_to_be16(height);	

		rc = isx006_i2c_write(isx006_client->addr, 0x0024, picture_width, WORD_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x002A, picture_height, WORD_LEN);
		if (rc < 0)
			return rc;
	}

	rc = isx006_i2c_write(isx006_client->addr, 0x04886, 0x01, BYTE_LEN);  //AF Stopped
	if (rc < 0)
		return rc;
	
#if ALESSI_JPEG_IF_SET
	// 2MP미만은 YUV I/F 적용한다.
	if(width < 1600)
	{		
		rc = isx006_i2c_write(isx006_client->addr, 0x0224, 0x00, BYTE_LEN);  //SIZE_HOLD_EN
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x0014, 0x00, BYTE_LEN);  //CAPNUM
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x001D, 0x00, BYTE_LEN);  //OUTFMT_CAP     YUV MODE
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0011, 0x02, BYTE_LEN); // CAPTURE COMMAND
		if (rc < 0)
			return rc;	
	}
	else
	{
#if 1
		mdelay(100);  // msm memory ready time (정말 중요..)
		
#if ALESSI_JPEG_USE_ADDRESS
		rc = isx006_i2c_write(isx006_client->addr, 0x02D3, 0x01, BYTE_LEN);  //VIFADRDUMP_MODE(YUV address Table Output On)
		if (rc < 0)
			return rc;
#endif

		//648 * 486을 지원하면서 화각 차이를 줄이기 위해서. (1.02배 적용.)
		rc = isx006_i2c_write(isx006_client->addr, 0x0032, 0x0501, WORD_LEN);  //EZOOM_MAG
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0224, 0x01, BYTE_LEN);  //SIZE_HOLD_EN
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0014, 0x01, BYTE_LEN);  //CAPNUM
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x001D, 0x1B, BYTE_LEN);  //OUTFMT_CAP     INTERLEAVE MODE SPOOF FREE-RUNNING MODE
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0011, 0x02, BYTE_LEN); // CAPTURE COMMAND
		if (rc < 0)
			return rc;	
#else //현재 잘 동작안한다.
		rc = isx006_i2c_write(isx006_client->addr, 0x0039, 0x01, BYTE_LEN);  //VIF_MASK
		if (rc < 0)
			return rc;

		mdelay(280); 	// 2 frames skip (VIF MASK Preview data를 받지 않을 시간을 번다.)

		// delay를 방지하기 위해서 VLanch를 이용한다.. (multi driver setting 방법)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A01, 0x39, BYTE_LEN); // VLanch이용 (VIF_MASK)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A02, 0x00, BYTE_LEN); // VLanch이용 (VIF_MASK)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A03, 0x00, BYTE_LEN); // VLanch이용 (VIF_MASK)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A04, 0x11, BYTE_LEN); // VLanch이용 (Capture mode)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A05, 0x00, BYTE_LEN); // VLanch이용 (Capture mode)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A06, 0x02, BYTE_LEN); // VLanch이용 (Capture mode)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A07, 0x10, BYTE_LEN); // VLanch이용 (Capture mode Fix)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A08, 0x00, BYTE_LEN); // VLanch이용 (Capture mode Fix)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A09, 0x02, BYTE_LEN); // VLanch이용 (Capture mode Fix)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A0A, 0x00, BYTE_LEN); // VLanch이용 (이후 reg dummy 방지.)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A0B, 0x00, BYTE_LEN); // VLanch이용 (이후 reg dummy 방지.)
		rc = isx006_i2c_write(isx006_client->addr, 0x6A0C, 0x00, BYTE_LEN); // VLanch이용 (이후 reg dummy 방지.)
		
		if (rc < 0)
			return rc;	
#endif
	}
#else
		//648 * 486을 지원하면서 화각 차이를 줄이기 위해서. (1.02배 적용.)
		rc = isx006_i2c_write(isx006_client->addr, 0x0032, 0x0501, WORD_LEN);  //EZOOM_MAG
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x0384, 0x05, BYTE_LEN);  //FPSTYPE_CAP
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x0224, 0x00, BYTE_LEN);  //SIZE_HOLD_EN
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0014, 0x00, BYTE_LEN);  //CAPNUM
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr, 0x001D, 0x00, BYTE_LEN);  //OUTFMT_CAP     YUV MODE
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr, 0x0011, 0x02, BYTE_LEN); // CAPTURE COMMAND
		if (rc < 0)
			return rc;
#endif	
	
	isx006_ctrl->sensormode = mode;
	return 0;
}

static int32_t isx006_cancel_focus(int mode)
{
	int32_t rc = 0;
	int lense_po_back=0;
	unsigned short cm_changed_sts, cm_changed_clr, af_pos;
	int i = 0;	
	if(debug_mask)
		printk("isx006: cancel focus, mode = %d\n", mode);

	switch(mode){
	case FOCUS_AUTO:
	case FOCUS_NORMAL:
		if(debug_mask)
	   		printk("back to the infinity\n");

		lense_po_back = 0xC800;		
		break;
	
	case FOCUS_MACRO:
		if(debug_mask)
	   		printk("back to the macro\n");

		lense_po_back = 0x5802;		
		break;

	case FOCUS_MANUAL:
		return rc;
		break;
	}

	rc = isx006_i2c_write(isx006_client->addr, 0x002E, 0x02, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx006_i2c_write(isx006_client->addr, 0x4852, lense_po_back, WORD_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx006_i2c_write(isx006_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	rc = isx006_i2c_write(isx006_client->addr, 0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
	
//	mdelay(60); // 1 frame delay to get lense move
			/* check lens position */
	for(i=0; i<15; i++){
		rc = isx006_i2c_read(isx006_client->addr,
				0x6D7A, &af_pos, WORD_LEN);
		if (rc < 0){
			printk("isx006: reading af_lenspos fail\n");
		}
	
		if(af_pos == lense_po_back){
            if(debug_mask)
                printk("af_lenspos is equal with af_manual_pos\n");
			break;
		}
		
		if(debug_mask)
			printk("[isx006.c] lens position(0x%x) is not ready yet\n",af_pos);
		
		mdelay(60); // 1 frame skip
	}	
	rc = isx006_i2c_write(isx006_client->addr, 0x00FC, 0x1F, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	return rc;
}

static long isx006_set_sensor_mode(int mode,int width, int height)
{
	int32_t rc = 0;   
	
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
			if(debug_mask)
				printk("isx006_set_sensor_mode: sensor mode is PREVIEW\n");			
		
		if(previous_mode == CAPTURE_MODE){
			mdelay(60); // 1 frame skip ==> total 2 frames skip
			rc = isx006_i2c_write_table(&isx006_regs.prev_reg_settings[0], isx006_regs.prev_reg_settings_size);
			if(rc<0){
				printk("isx006: preview writing fail!\n");
				return rc;
			}

			mdelay(200);  // 2 frames skip
			rc = isx006_cancel_focus(focus_mode);
			if(rc<0)
				return rc;
			/* BEGIN: 0005153 hyungtae.lee 2010-03-18 */
			/* MOD: 0005153: [camera] adjustment of preview timing after capture */ 
#if 0

			if(current_scene == NIGHT_MODE){
				if(debug_mask)
					printk("[isx006.c] current scene is NIGHT\n");
				
				mdelay(800);
		   }
#endif
		  /* END: 0005153 hyungtae.lee 2010-03-18 */ 
			previous_mode = PREVIEW_MODE;
		}
	    break;

	case SENSOR_SNAPSHOT_MODE:
	case SENSOR_RAW_SNAPSHOT_MODE:
		if(debug_mask)
			printk("isx006_set_sensor_mode: sensor mode is SNAPSHOT\n");
		if(tuning_thread_run)
		{
			rc = isx006_check_thread_run();
			if (rc < 0)
			{
				printk("isx006_check_thread_run error\n");
			}
			mdelay(500);
		}
	
		rc = isx006_snapshot_config(mode, width, height);
		
		/* BEGIN: 0005118 hyungtae.lee 2010-03-17 */
		/* MOD: 0005118: [camera] reduce power-off time and capture time */
		//mdelay(80);    // 1 frame skip
		
#if 0
		if(current_scene == NIGHT_MODE){
			if(debug_mask)
				printk("[isx006.c] current scene is NIGHT\n");

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
static int32_t isx006_check_af_lock_and_clear(void)
{
	int32_t rc;
	int i;
	unsigned short af_lock;
	
	/* check AF lock status */
	for(i=0; i<5; i++){
		rc = isx006_i2c_read(isx006_client->addr, 0x00F8, &af_lock, BYTE_LEN);
		if (rc < 0){
			printk("isx006: reading af_lock fail\n");
			return rc;
		}
	
		if((af_lock & 0x10)==0x10){
            if(debug_mask)
			    printk(" af_lock is locking\n");
			break;
		}
		if(debug_mask)
			printk("isx006: af_lock( 0x%x ) is not ready yet\n", af_lock);
		mdelay(70);
	}	
	
	rc = isx006_i2c_write(isx006_client->addr, 0x00FC, 0x10, BYTE_LEN);
	if (rc < 0)
		return rc;
	
	/* check AF lock status */
	for(i=0; i<5; i++){
		rc = isx006_i2c_read(isx006_client->addr, 0x00F8, &af_lock, BYTE_LEN);
		if (rc < 0){
			printk("isx006: reading af_lock fail\n");
			return rc;
		}
	
		if((af_lock & 0x10)==0x00){
			printk("af_lock is released\n");
			break;
		}
		if(debug_mask)
			printk("isx006: af_lock( 0x%x ) is not clear yet\n", af_lock);
		mdelay(70);
	}	

	return rc;
}

#define FOCUS_STEP_GARO 	4
#define FOCUS_STEP_SERO 	4
#define FOCUS_STEP_MAX 		(FOCUS_STEP_GARO * FOCUS_STEP_SERO)
#define FOCUS_FULLSIZE_X 	2592
#define FOCUS_FULLSIZE_Y 	1944
#define FOCUS_PREVIEW_X 	640
#define FOCUS_PREVIEW_Y 	480

static int32_t isx006_set_focus(void)
{
	int32_t rc = 0;
	int lock = 0;

	rc = isx006_i2c_write_table(&isx006_regs.AF_reg_settings[0], isx006_regs.AF_reg_settings_size);
	if(rc<0){
		printk("[isx006.c]%s: fail in writing for focus\n",__func__);
		return rc;
	}
//	msleep(60);  // 1 frame skip
	
//	rc = isx006_check_focus(&lock);

	return rc;
}

static long isx006_set_focus_rect(int focus_rect)
{	
	// focus_rect --> focus window값이 내려온다. (16step이다.)
	int32_t rc;	
	int temp_x_position = 0, temp_y_position = 0;	
	int x_position = 0, y_position = 0;	
	int real_x_position = 0, real_y_position = 0;	
	int rect_W = 0, rect_H = 0;	
	int real_rect_W = 0, real_rect_H = 0;	

	if(debug_mask)
		printk("isx006_set_focus_rect : called, new focus_rect: %d\n", focus_rect);

	if((focus_rect <= 0) || (focus_rect > 25))
		return 0;
	isx006_set_focus();

	if(focus_rect <= FOCUS_STEP_MAX)
	{
		temp_x_position = focus_rect / FOCUS_STEP_GARO;
		temp_y_position = (focus_rect - 1) % FOCUS_STEP_SERO;
		x_position = (int)(temp_x_position * 160 * FOCUS_FULLSIZE_X / FOCUS_PREVIEW_X + 8 + 41);	// 8 + 41은 sony의 연산방법 참조.
		real_x_position = cpu_to_be16(x_position);
		
		y_position = (int)(temp_y_position * 120 * FOCUS_FULLSIZE_Y / FOCUS_PREVIEW_Y + 4); 		// 4는 sony의 연산방법 참조.
		real_y_position = cpu_to_be16(y_position);		
	}
	else
	{
		temp_x_position = (focus_rect / FOCUS_STEP_GARO) - (FOCUS_STEP_MAX / FOCUS_STEP_GARO);
		temp_y_position = (focus_rect - 1) % (FOCUS_STEP_SERO - 1) - 1;
		x_position = (int)((80 + temp_x_position * 160) * FOCUS_FULLSIZE_X / FOCUS_PREVIEW_X + 8 + 41);	// 8 + 41은 sony의 연산방법 참조.
		real_x_position = cpu_to_be16(x_position);
		
		y_position = (int)((60 + temp_y_position * 120) * FOCUS_FULLSIZE_Y / FOCUS_PREVIEW_Y + 4); 		// 4는 sony의 연산방법 참조.
		real_y_position = cpu_to_be16(y_position);		
	}

	rect_W = (FOCUS_PREVIEW_X / FOCUS_STEP_GARO) * (FOCUS_FULLSIZE_X / FOCUS_PREVIEW_X);
	real_rect_W = cpu_to_be16(rect_W);
	rect_H = (FOCUS_PREVIEW_Y / FOCUS_STEP_SERO) * (FOCUS_FULLSIZE_Y / FOCUS_PREVIEW_Y);
	real_rect_H = cpu_to_be16(rect_H);

	if(debug_mask)
	{
		printk("isx006_set_focus_rect : called, x_position = %d, y_position = %d\n", x_position, y_position);
		printk("isx006_set_focus_rect : called, rect_W = %d, rect_H = %d\n", rect_W, rect_H);
	}	

	rc = isx006_i2c_write(isx006_client->addr, 0x4C4C, real_x_position, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr, 0x4C4E, real_y_position, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr, 0x4C50, real_rect_W, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr, 0x4C52, real_rect_H, WORD_LEN);
	if (rc < 0)
		return rc;
	
	return rc;	
}

static int isx006_check_af_lock(void)
{
	int rc;
	int i;
	unsigned short af_lock;
	
	/* INT clear */
	rc = isx006_i2c_write(isx006_client->addr,
		0x00FC, 0x10, BYTE_LEN);
	if (rc < 0)
		return rc;

	for (i = 0; i < 10; ++i) {
		/*INT state read to confirm INT release state*/
		rc = isx006_i2c_read(isx006_client->addr,
				0x00F8, &af_lock, BYTE_LEN);
		
		if (rc < 0) {
			printk("isx006: reading af_lock fail\n");
			return rc;
		}

		if ((af_lock & 0x10) == 0x00) {
			printk("af_lock is released\n");
			break;
		}
		msleep(10);
	}

	return rc;
}

static int isx006_check_focus(int *lock)
{
	int rc;
	unsigned short af_status;
	unsigned short af_result;

	printk("isx006_check_focus\n");

	/*af status check  0:load, 1: init,  8: af_lock */
	rc = isx006_i2c_read(isx006_client->addr,
		0x6D76, &af_status, BYTE_LEN);

	if (af_status != 0x8)
		return -ETIME;

	
	isx006_check_af_lock();
	
	/* af result read  success/ fail*/
	rc = isx006_i2c_read(isx006_client->addr, 0x6D77, &af_result, BYTE_LEN);
	if (rc < 0) {
		printk("[isx006.c]%s: fai; in reading af_result\n",__func__);
		return rc;
	}


	/* single autofocus off */
	rc = isx006_i2c_write(isx006_client->addr, 0x002E, 0x03, BYTE_LEN);
	if (rc < 0)
		return rc;
		
	/* single autofocus refresh*/
	rc = isx006_i2c_write(isx006_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	if (af_result == 1) {
		mdelay(60);
		*lock = CFG_AF_LOCKED;  // success
		return rc;
	} else {
		*lock = CFG_AF_UNLOCKED; //0: focus fail or 2: during focus
		return rc;
	}

	return -ETIME;
}
#if 0
static int32_t isx006_check_focus(int *lock)
{
	int32_t rc;
	int num = 0;
	unsigned short af_status, af_result, af_result_ex;
	
	while(num < 200){
		rc = isx006_i2c_read(isx006_client->addr, 0x6D76, &af_status, BYTE_LEN);
		if (rc < 0){
			printk("isx006: reading af_status fail\n");
			return rc;
		}

		if(af_status == 8){
			if(debug_mask)
				printk("isx006: af_status is lock done\n");
			break;
		}
		
		if(af_status != 8){
			printk("isx006: af_status = %d, waiting untill af_status changes to lock done \n", af_status);
		}
		mdelay(30);
		num = num + 1;
	}
	
	isx006_check_af_lock_and_clear();
	
	rc = isx006_i2c_read(isx006_client->addr, 0x6D3A, &af_result, BYTE_LEN);
	if (rc < 0){
		printk("isx006: reading af_result fail\n");
		return rc;
	}

	rc = isx006_i2c_read(isx006_client->addr, 0x6D52, &af_result_ex, BYTE_LEN);
	if (rc < 0){
		printk("isx006: reading af_result fail_ex\n");
		return rc;
	}	

	rc = isx006_i2c_write(isx006_client->addr, 0x002E, 0x03, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr, 0x0012, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;
				
	if((af_result == 0x01) || (af_result_ex == 0x01)){
		if(debug_mask)
			printk("isx006: Now is good focus\n");
		return 0;
	}

	if(debug_mask)
		printk("isx006: af is time out af_res = %d, af_res1 = %d\n",af_result, af_result_ex);

	return -ETIME;
}
#endif

static int32_t isx006_set_auto_focus(void)
{
	int32_t rc;

	if(debug_mask)
		printk("isx006: auto focus\n");

	rc = isx006_i2c_write_table(&isx006_regs.AF_nomal_reg_settings[0], isx006_regs.AF_nomal_reg_settings_size);
	if(rc<0){
		if(debug_mask)
			printk("isx006: AF auto focus writing fail!\n");
		return rc;
	}	
	return rc;
}

static int32_t isx006_set_macro_focus(void)
{
	int32_t rc;
	
	if(debug_mask)
		printk("isx006: macro focus\n");

	rc = isx006_i2c_write_table(&isx006_regs.AF_macro_reg_settings[0], isx006_regs.AF_macro_reg_settings_size);
	if(rc<0){
		if(debug_mask)
			printk("isx006: AF macro focus writing fail!\n");
		return rc;
	}	
	return rc;
}


static int32_t isx006_focus_config(int mode)
{
	int i;
	int32_t rc = 0;
	unsigned short af_driver_check, af_driver_status;

	if(prev_af_mode == mode)
	{		
		rc = isx006_set_focus();
	}

	else
	{
		switch(mode){
			case FOCUS_AUTO:
			case FOCUS_NORMAL:
				rc = isx006_set_auto_focus();
				break;
			
			case FOCUS_MACRO:				
				rc = isx006_set_macro_focus();
				break;

			case FOCUS_MANUAL:
				rc = isx006_i2c_write_table(&isx006_regs.manual_focus_reg_settings[0],
						isx006_regs.manual_focus_reg_settings_size);
				break;
				
			default:
				if(debug_mask)
				printk("[isx006.c] isx006_focus_config: invalid af %d\n",mode);
				return -EINVAL;
		}

		rc = isx006_set_focus();
	}

	prev_af_mode = mode;	
	return rc;		
}

static int32_t isx006_move_focus(int32_t steps)
{
	int32_t rc;
	unsigned short cm_changed_sts, cm_changed_clr, af_pos, manual_pos;
	int i, temp;
	prev_af_mode = FOCUS_MANUAL;
	focus_mode = FOCUS_MANUAL;
	if(debug_mask)
		printk("[isx006.c] move focus: step is %d\n",steps);
	
	/* MF ON */
	/* BEGIN: 0004908 hyungtae.lee 2010-03-12 */
  	/* MOD: 0004908: [camera] change brightness on night mode when snap */ 

	rc = isx006_i2c_write_table(&isx006_regs.manual_focus_reg_settings[0],
			isx006_regs.manual_focus_reg_settings_size);
	if(rc<0){
		printk("[isx006.c]%s: fail in writing for move focus\n",__func__);
		return rc;
	}
	/* END: 0004908 hyungtae.lee 2010-03-12 */

	/* check cm_changed_sts */
	for(i=0; i<4; i++){
		rc = isx006_i2c_read(isx006_client->addr,
				0x00F8, &cm_changed_sts, BYTE_LEN);
		if (rc < 0){
			printk("isx006: reading cm_changed_sts fail\n");
			return rc;
		}
	
		if((cm_changed_sts & 0x02)==0x02){
			printk("cm_changed_sts -> 02\n");
			break;
		}
		if(debug_mask)
			printk("isx006: cm_changed_sts( 0x%x ) for MF is not ready yet\n", cm_changed_sts);
		mdelay(60); // 1 frame skip
	}	
	/* clear the interrupt register */
	rc = isx006_i2c_write(isx006_client->addr,
			0x00FC, 0x10, BYTE_LEN);
	if (rc < 0)
		return rc;

	/* check cm_changed_clr */
	for(i=0; i<4; i++){
		rc = isx006_i2c_read(isx006_client->addr,
				0x00FC, &cm_changed_sts, BYTE_LEN);
		if (rc < 0){
			printk("isx006: reading cm_changed_clr fail\n");
			return rc;
		}
	
		if((cm_changed_clr & 0x00)==0x00){
			printk("cm_changed_clr -> 0\n");
			break;
		}
		if(debug_mask)
			printk("isx006: cm_changed_clr( 0x%x ) for MF is not ready yet\n", cm_changed_clr);
		mdelay(60); // 1 frame skip
	}

	printk("isx006: move focus steps = %d\n", steps);

	rc = isx006_i2c_write(isx006_client->addr,
			0x002e, 0x02, BYTE_LEN);
	steps++;
	switch(steps)
	{
		case 1:
			manual_pos = 0x0802;
			break;
		case 2:
			manual_pos = 0xec01;
			break;	
		case 3:			
			manual_pos = 0xde01;
			break;
		case 4:
			manual_pos = 0xd001;			
			break;	
		case 5:
			manual_pos = 0xc301;				
			break;
		case 6:
			manual_pos = 0xb501;				
			break;	
		case 7:
			manual_pos = 0xa701;	
			break;
		case 8:
			manual_pos = 0x9901;			
			break;	
		case 9:
			manual_pos = 0x8c01;			
			break;
		case 10:
			manual_pos = 0x7e01;				
			break;	
		case 11:
			manual_pos = 0x7001;				
			break;
		case 12:
			manual_pos = 0x6201;				
			break;	
		case 13:
			manual_pos = 0x5501;			
			break;
		case 14:
			manual_pos = 0x4701;				
			break;	
		case 15:
			manual_pos = 0x3901;			
			break;
		case 16:
			manual_pos = 0x2c01;	
			break;
		default:
			manual_pos = 0x2c01;
			break;
	}

	
	if(debug_mask)
		printk("[isx006.c] manual position is 0x%x\n", manual_pos);

	rc = isx006_i2c_write(isx006_client->addr,
			0x4852, manual_pos, WORD_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr,
			0x4850, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	rc = isx006_i2c_write(isx006_client->addr,
			0x0012, 0x01, WORD_LEN);	
	rc = isx006_i2c_write(isx006_client->addr,
			0x0015, 0x01, BYTE_LEN);
	if (rc < 0)
		return rc;

	mdelay(30);

	isx006_check_af_lock_and_clear();
	
	/* check lens position */
	for(i=0; i<4; i++){
		rc = isx006_i2c_read(isx006_client->addr,
				0x6D7A, &af_pos, WORD_LEN);
		if (rc < 0){
			printk("isx006: reading af_lenspos fail\n");
		}
	
		if(af_pos == manual_pos){
            if(debug_mask)
                printk("af_lenspos is equal with af_manual_pos\n");
			break;
		}
		
		if(debug_mask)
			printk("[isx006.c] lens position(0x%x) is not ready yet\n",af_pos);
		
		mdelay(60); // 1 frame skip
	}	
	return rc;
}

static long isx006_set_effect(int effect)
{
	int32_t rc;

   switch (effect) {
	case CAMERA_EFFECT_OFF: 
		if(debug_mask)
			printk("isx006_set_effect: effect is OFF\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x00, BYTE_LEN);
		if (rc < 0)
			return rc;
	
	
      break;

	case CAMERA_EFFECT_MONO: 
		if(debug_mask)
			printk("isx006_set_effect: effect is MONO\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x04, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   case CAMERA_EFFECT_SEPIA: 
		if(debug_mask)
			printk("isx006_set_effect: effect is SEPIA\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x03, BYTE_LEN);
		if (rc < 0)			
			return rc;
		break;

	case CAMERA_EFFECT_NEGATIVE: 
		if(debug_mask)
			printk("isx006_set_effect: effect is NAGATIVE\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   case CAMERA_EFFECT_NEGATIVE_SEPIA:
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x02, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   case CAMERA_EFFECT_BLUE:
		rc = isx006_i2c_write(isx006_client->addr,
			0x005F, 0x03, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   case CAMERA_EFFECT_SOLARIZE: 
		if(debug_mask)
			printk("isx006_set_effect: effect is SOLARIZE\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x01, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   case CAMERA_EFFECT_PASTEL: 
		if(debug_mask)
			printk("isx006_set_effect: effect is PASTEL\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x005F, 0x05, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

   default: 
		printk("isx006_set_effect: wrong effect mode\n");
		return -EINVAL;	
	}
	
	return 0;
}

static long isx006_set_zoom(int8_t zoom)
{
	int32_t rc;	
	
	if(debug_mask)
		printk("isx006_set_zoom : called, new zoom: %d\n", zoom);
		
	switch (zoom){
	case 0:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.0\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x0001, WORD_LEN);
		break;

	case 1:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.15\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x2601, WORD_LEN);
		break;		
	
	case 2:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.3\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x4C01, WORD_LEN);
		break;	
		
	case 3:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.45\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x7301, WORD_LEN);
		break;
		
	case 4:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.6\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x9901, WORD_LEN);
		break;
		
	case 5:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.75\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0xC001, WORD_LEN);
		break;
		
	case 6:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 1.9\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0xE601, WORD_LEN);
		break;
		
	case 7:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.05\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x0C02, WORD_LEN);
		break;
		
	case 8:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.2\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x3302, WORD_LEN);
		break;
		
	case 9:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.35\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x5902, WORD_LEN);
		break;
		
	case 10:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.5\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x8002, WORD_LEN);
		break;
		
	case 11:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.65\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0xA602, WORD_LEN);
		break;
		
	case 12:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.8\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0xCC02, WORD_LEN);
		break;
		
	case 13:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 2.95\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0xF302, WORD_LEN);
		break;
	
	case 14:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 3.1\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x1903, WORD_LEN);
		break;	
		
	case 15:
		if(debug_mask)
			printk("isx006_set_zoom: zoom is 3.2\n");
		
		rc = isx006_i2c_write(isx006_client->addr,0x0032, 0x3303, WORD_LEN);
		break;		
	
	default:
		printk("isx006: wrong zoom value\n");
		return -EFAULT;
	}
	if (rc < 0)
		return rc;
	
	isx006_ctrl->zoom = zoom;
	return rc;
}

static long isx006_set_zoom_sensor(int zoom)
{
	int32_t rc;	
	
	if(debug_mask)
		printk("isx006_set_zoom_sensor : called, new zoom: %d\n", zoom);
		
	rc = isx006_set_capture_zoom(zoom);
	if(rc < 0){
		printk("isx006: isx006_set_zoom_sensor setting fail!\n");
		return rc;
	}
	
	isx006_ctrl->zoom = zoom;
	return rc;
}

static long isx006_set_capture_zoom(int zoom)
{
	int32_t rc;
	int zoom_depth = 16;
	
	if(debug_mask)
		printk("isx006_set_capture_zoom : called, new zoom: %d\n", zoom);

	if((isx006_ctrl->zoom % zoom_depth) != 0)
		rc = isx006_i2c_write(isx006_client->addr,0x034F, 0x01, BYTE_LEN);
	else
		rc = isx006_i2c_write(isx006_client->addr,0x034F, 0x00, BYTE_LEN);
	
	if(zoom >= 0 && zoom < zoom_depth)
		rc = isx006_i2c_write_table_one(&isx006_regs.zoom_mode_capture_405_reg_settings[0], zoom);
	else if(zoom >= zoom_depth && zoom < (zoom_depth * 2))
		rc = isx006_i2c_write_table_one(&isx006_regs.zoom_mode_capture_203_reg_settings[0], zoom - zoom_depth);
	else if(zoom >= (zoom_depth * 2) && zoom < (zoom_depth * 3))
		rc = isx006_i2c_write_table_one(&isx006_regs.zoom_mode_capture_162_reg_settings[0], zoom - (zoom_depth * 2));
	else if(zoom >= (zoom_depth * 3) && zoom < (zoom_depth * 4))
		rc = isx006_i2c_write_table_one(&isx006_regs.zoom_mode_capture_127_reg_settings[0], zoom - (zoom_depth * 3));
	else
		printk("isx006: isx006_set_capture_zoom wrong value\n");	

	if(rc<0){
		printk("isx006: isx006_set_capture_zoom setting fail!\n");
		return rc;
	}	
	return rc;
}

static long isx006_set_wb(int8_t wb)
{
	int32_t rc = 0;
   
	if(debug_mask)
		printk("isx006_set_wb : called, new wb: %d\n", wb);

	switch (wb) {
	case CAMERA_WB_AUTO:
		if(debug_mask)
			printk("isx006_set_wb: wb is AUTO\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
			0x4453, 0x7B, BYTE_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x0102, 0x20, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

	case CAMERA_WB_INCANDESCENT:
		if(debug_mask)
			printk("isx006_set_wb: wb is INCANDESCENT\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x4453,0x3B, BYTE_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr,
			0x0102,0x28, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;
	
	case CAMERA_WB_DAYLIGHT:
		if(debug_mask)
			printk("isx006_set_wb: wb is SUNNY\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x4453,0x3B, BYTE_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr,
			0x0102,0x24, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

	case CAMERA_WB_FLUORESCENT:
		if(debug_mask)
			printk("isx006_set_wb: wb is FLUORESCENT\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x4453,0x3B, BYTE_LEN);
		if (rc < 0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr,
			0x0102,0x27, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

	case CAMERA_WB_CLOUDY_DAYLIGHT:
		if(debug_mask)
			printk("isx006_set_wb: wb is CLOUDY\n");
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x4453, 0x3B, BYTE_LEN);
		if (rc < 0)
			return rc;

		rc = isx006_i2c_write(isx006_client->addr,
			0x0102,0x26, BYTE_LEN);
		if (rc < 0)
			return rc;
		break;

	default:
		printk("isx006: wrong white balance value\n");
		return -EFAULT;
	}
	isx006_ctrl->wb = wb;

	return 0;
}

static int32_t isx006_set_iso(int8_t iso)
{
	int32_t rc = 0;	

	if(debug_mask)
		printk("isx006_set_iso : called, new iso: %d\n", iso);
	
	switch (iso){
	case CAMERA_ISO_AUTO:
		if(debug_mask)
			printk("[isx006.c] iso is AUTO\n");
	
		rc = isx006_i2c_write_table(&isx006_regs.iso_auto_reg_settings[0],
            isx006_regs.iso_auto_reg_settings_size);
		break;                                                    
                                                        
	case CAMERA_ISO_100:
		if(debug_mask)
			printk("[isx006.c] iso is 100\n");
		
		rc = isx006_i2c_write_table(&isx006_regs.iso_100_reg_settings[0],
            isx006_regs.iso_100_reg_settings_size);
		break;

	case CAMERA_ISO_200:
		if(debug_mask)
			printk("[isx006.c] iso is 200\n");
		
		rc = isx006_i2c_write_table(&isx006_regs.iso_200_reg_settings[0],
            isx006_regs.iso_200_reg_settings_size);
		break;

	case CAMERA_ISO_400:
		if(debug_mask)
			printk("[isx006.c] iso is 400\n");
		
		rc = isx006_i2c_write_table(&isx006_regs.iso_400_reg_settings[0],
            isx006_regs.iso_400_reg_settings_size);
		break;
		
	case CAMERA_ISO_800:
		if(debug_mask)
			printk("[isx006.c] iso is 800\n");
		
		rc = isx006_i2c_write_table(&isx006_regs.iso_800_reg_settings[0],
            isx006_regs.iso_800_reg_settings_size);
		break;

	default:
		printk("[isx006.c] incorrect iso value\n");
		rc = -EINVAL;
	}	
	
	return rc;
}

static long isx006_set_scene_mode(int8_t mode)
{
	int32_t rc = 0;

	current_scene = NOT_NIGHT_MODE;
	switch (mode) {
	case CAMERA_SCENE_AUTO:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is normal\n");
			
		rc = isx006_i2c_write_table(&isx006_regs.scene_normal_reg_settings[0],
                isx006_regs.scene_normal_reg_settings_size);
		break;
	
	case CAMERA_SCENE_PORTRAIT:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is portrait\n");
		
		rc = isx006_i2c_write_table(&isx006_regs.scene_portrait_reg_settings[0],
                isx006_regs.scene_portrait_reg_settings_size);
     	break;
	
	case CAMERA_SCENE_LANDSCAPE:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is landscape\n");
	
		rc = isx006_i2c_write_table(&isx006_regs.scene_landscape_reg_settings[0],
                isx006_regs.scene_landscape_reg_settings_size);
		break;
	
	case CAMERA_SCENE_SPORTS:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is sport\n");
	
		rc = isx006_i2c_write_table(&isx006_regs.scene_sport_reg_settings[0],
                isx006_regs.scene_sport_reg_settings_size);
		break;
	
	case CAMERA_SCENE_SUNSET:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is sunset\n");
	
		rc = isx006_i2c_write_table(&isx006_regs.scene_sunset_reg_settings[0],
                isx006_regs.scene_sunset_reg_settings_size);
		break;
	
	case CAMERA_SCENE_NIGHT:
		if(debug_mask)
			printk("isx006_set_scene_mode: mode is night\n");
	
		rc = isx006_i2c_write_table(&isx006_regs.scene_night_reg_settings[0],
                isx006_regs.scene_night_reg_settings_size);
		current_scene = NIGHT_MODE;
		break;
	
	default:
		printk("isx006: wrong scene mode value, set to the normal\n");
	}   
	if (rc < 0)
		return rc;

	isx006_ctrl->scene = mode;

	return rc;
   
}
/* BEGIN: 0005280 hyungtae.lee 2010-03-22 */
/* MOD: 0005280: [camera] modification for brightness */ 
static int32_t isx006_set_brightness(int8_t ev)
{
	int32_t rc=0;
	ev++;	
	switch (ev) {
	case 1:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;
		
		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x50, BYTE_LEN);
		if(rc<0)
			return rc;
		
		break;

	case 2:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;	
	
		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x60, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 3:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;	
	
		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x70, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 4:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0xCD, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x80, BYTE_LEN);
		if(rc<0)
		return rc;	

		break;

	case 5:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0xEF, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 6:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x00, BYTE_LEN);
		if(rc<0)
			return rc;	
	
		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 7:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x18, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x80, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 8:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x7F, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x8A, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 9:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x7F, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0x9C, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 10:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x7F, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0xAA, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;

	case 11:
		rc = isx006_i2c_write(isx006_client->addr,
				0x0060, 0x7F, BYTE_LEN);
		if(rc<0)
			return rc;	

		rc = isx006_i2c_write(isx006_client->addr,
				0x0061, 0xC8, BYTE_LEN);
		if(rc<0)
			return rc;	

		break;
	
	default:
		printk("[isx006.c] incoreect brightness value\n");
	}
	
	isx006_ctrl->brightness = ev;
	return rc;
}
/* END: 0005280 hyungtae.lee 2010-03-22 */

static int isx006_sensor_init_probe(const struct msm_camera_sensor_info *data)
{
	uint16_t model_id = 0;
	int32_t  rc;
	printk("isx006_sensor_init_probe\n");


	/* Read the Model ID of the sensor */
	rc = isx006_i2c_read(isx006_client->addr,
		REG_ISX006_MODEL_ID, &model_id, WORD_LEN);
	if (rc < 0)
		goto init_probe_fail;

	printk("isx006 model_id = 0x%x\n", model_id);
	/* Check if it matches it with the value in Datasheet */
	if (model_id != ISX006_MODEL_ID) {
		rc = -EINVAL;
		goto init_probe_fail;
	}

	return rc;
   
init_probe_fail:
    return rc;
}

#if ALESSI_TUNING_SET
#define LOOP_INTERVAL		20
#define IS_NUM(c)			((0x30<=c)&&(c<=0x39))
#define IS_CHAR_C(c)		((0x41<=c)&&(c<=0x46))						// Capital Letter
#define IS_CHAR_S(c)		((0x61<=c)&&(c<=0x66))						// Small Letter
#define IS_VALID(c)			(IS_NUM(c)||IS_CHAR_C(c)||IS_CHAR_S(c))		// NUM or CHAR
#define TO_BE_NUM_OFFSET(c)	(IS_NUM(c) ? 0x30 : (IS_CHAR_C(c) ? 0x37 : 0x57))	
#define TO_BE_READ_SIZE		 4000*40									// 8pages (4000x8)

char *file_buf_alloc_pages=NULL;

static long isx006_read_ext_reg(char *filename)
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

	file_buf_alloc_pages = kmalloc(TO_BE_READ_SIZE, GFP_KERNEL);	
	
	if(!file_buf_alloc_pages) {
		printk("%s : mem alloc error!\n", __func__);
		return 0;
	}

	set_fs(get_ds());
	phMscd_Filp->f_op->read(phMscd_Filp, file_buf_alloc_pages, TO_BE_READ_SIZE-1, &phMscd_Filp->f_pos);
	set_fs(old_fs);

	do
	{		
		if (file_buf_alloc_pages[read_idx]=='0' && file_buf_alloc_pages[read_idx+1]=='x' 
			&& file_buf_alloc_pages[read_idx + 2] != ' ') {	// skip : 0x
			read_idx += 2;			

			if(length == ADDRESS_TUNE)
			{
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x1000 \
						+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x100\
						+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]));

				read_idx = read_idx + 4;

				ext_reg_settings[i++].register_address = value;
#if ALESSI_TUNING_LOG
				printk("%s : length == ADDRESS_TUNE, i = %d\n", __func__, i);
#endif
			}
			else if(length == BYTE_LEN)
			{
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]));

				read_idx = read_idx + 2;

				ext_reg_settings[j].register_value = value;
				ext_reg_settings[j++].register_length = length;
#if ALESSI_TUNING_LOG
				printk("%s : length == BYTE_LEN, j = %d\n", __func__, j);
#endif
			}
			else if(length == WORD_LEN)
			{
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x1000 \
						+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x100\
						+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]));

				read_idx = read_idx + 4;

				ext_reg_settings[j].register_value = value;
				ext_reg_settings[j++].register_length = length;
#if ALESSI_TUNING_LOG
				printk("%s : length == WORD_LEN, j = %d\n", __func__, j);
#endif
			}
			else
			{				
				value = (file_buf_alloc_pages[read_idx]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx]))*0x10000000 \
						+ (file_buf_alloc_pages[read_idx+1]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+1]))*0x1000000\
						+ (file_buf_alloc_pages[read_idx+2]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+2]))*0x100000 \
						+ (file_buf_alloc_pages[read_idx+3]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+3]))*0x10000\
						+ (file_buf_alloc_pages[read_idx+4]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+4]))*0x1000 \
						+ (file_buf_alloc_pages[read_idx+5]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+5]))*0x100\
						+ (file_buf_alloc_pages[read_idx+6]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+6]))*0x10 \
							+ (file_buf_alloc_pages[read_idx+7]-TO_BE_NUM_OFFSET(file_buf_alloc_pages[read_idx+7]));

				read_idx = read_idx + 8;

				ext_reg_settings[j].register_value = value;
				ext_reg_settings[j++].register_length = length;
#if ALESSI_TUNING_LOG
				printk("%s : length == DOBULE_LEN, j = %d\n", __func__, j);
#endif
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
					else if(file_buf_alloc_pages[read_idx + k] == 'U')
					{
						length = DOBULE_LEN;
						break;
					}
				}
			}	
			else
			{
				length = ADDRESS_TUNE;
			}
		}
		else
		{
			++read_idx;
		}	

#if ALESSI_TUNING_LOG		
		printk("%s : read external file! position : %d\n", __func__, read_idx);
#endif
	}while(file_buf_alloc_pages[read_idx] != '$');

	kfree(file_buf_alloc_pages);
	file_buf_alloc_pages=NULL;
	filp_close(phMscd_Filp,NULL);

	return i;
}

static long isx006_reg_write_ext_table(uint16_t array_length)
{
	int i = 0, rc = 0;	

#if ALESSI_TUNING_LOG
	printk("%s : write registers from external file!\n", __func__);
#endif

	for (i = 0; i < array_length; i++) {
		rc = isx006_i2c_write(isx006_client->addr,
		  ext_reg_settings[i].register_address,
		  ext_reg_settings[i].register_value,
		  ext_reg_settings[i].register_length);		 

#if ALESSI_TUNING_LOG
		printk("isx006_reg_write_ext_table, addr = 0x%x, val = 0x%x, width = %d\n",
			    ext_reg_settings[i].register_address, ext_reg_settings[i].register_value, ext_reg_settings[i].register_length);
#endif
		if (rc < 0) {
			printk("I2C Fail\n");
			return rc;
		}
	}

	return 1;
}

static long isx006_reg_pll_ext(void)
{
	uint16_t length = 0;	
	uint16_t ap_check = 0, ap_check_16 = 0, real_ap_check = 0;

	printk("%s : isx006_reg_pll_ext enter!!\n", __func__);
	length = isx006_read_ext_reg("/sdcard/pll_settings_array.txt");
	printk("%s : length = %d!\n", __func__, length);	

#if 0
	if (!length)
		return 0;
	else	
		return isx006_reg_write_ext_table(length);
#else
	if (!length)
	{
		return 0;
	}
	else
	{
		isx006_reg_write_ext_table(length - 1);		
		isx006_i2c_read(isx006_client->addr, 0x0368, &ap_check, WORD_LEN); // OTP_CHIPID_L		

		ap_check_16 = ap_check * 0x1111; // 16진수로 change

		printk("isx006: ap_check_16 value = 0x%x\n", ap_check_16);
		real_ap_check = ((ap_check_16 & 0x000000FF) >> 2) & 0x000F; // OTP_CHIPID_L[29:26]
		printk("isx006: real_ap_check value = 0x%x\n", real_ap_check);

		if(real_ap_check == 0x01) //OTP_CHIPID_L[29:26]
			isx006_i2c_write_table(&isx006_regs.ap003_16bit_settings[0], isx006_regs.ap003_16bit_settings);
		else if(real_ap_check == 0x04) //OTP_CHIPID_L[29:26]
			isx006_i2c_write_table(&isx006_regs.ap001_16bit_settings[0], isx006_regs.ap001_16bit_settings);
		else
			printk("isx006: real_ap_check error\n");

		//마지막 0x0009 setting을 해준다.
		return isx006_i2c_write(isx006_client->addr, ext_reg_settings[length - 1].register_address,
		                 ext_reg_settings[length - 1].register_value, ext_reg_settings[length - 1].register_length);		
	}
#endif
}

static long isx006_reg_init_ext(void)
{	
	uint16_t length = 0;

	printk("%s : isx006_reg_init_ext enter!!\n", __func__);
	length = isx006_read_ext_reg("/sdcard/init_settings_array.txt");
	printk("%s : length = %d!\n", __func__, length);	

	if (!length)
		return 0;
	else
		return isx006_reg_write_ext_table(length);
}
#endif

static int dequeue_sensor_config(int cfgtype, int mode)
{
	int rc = 1;

	switch (cfgtype) {
			case CFG_SET_MODE:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_MODE\n");
					
				rc = isx006_set_sensor_mode(mode, pict_size.width, pict_size.height);
				break;
		
#if !ALESSI_TUNING_SET
			case CFG_SET_EFFECT:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_EFFECT\n");
					
				rc = isx006_set_effect(mode);
				break;
#endif
#if 0 
			case CFG_SET_ZOOM_VIDEO:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_ZOOM\n");
					
				rc = isx006_set_zoom(cfg_data.cfg.zoom);
				break;
#endif


#if 1
#if !ALESSI_TUNING_SET
			case CFG_SET_ZOOM_SENSOR:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_ZOOM_SENSOR\n");
					
				rc = isx006_set_zoom_sensor(mode);
				break;
		
			case CFG_SET_FOCUS_RECT:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_FOCUS_RECT\n");
					
				rc = isx006_set_focus_rect(mode);
				break;
#endif
				
			case CFG_START_AF_FOCUS:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_START_AF_FOCUS = %d\n", focus_mode);
				   
				focus_mode = mode;

				if(tuning_thread_run)
				{
					rc = isx006_check_thread_run();
					if(rc < 0)
					{
						printk("isx006_check_thread_run error\n");					
					}
					mdelay(500);
				}
				
				rc = isx006_focus_config(mode);		
				break;
		
			case CFG_SET_PARM_AF_MODE:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_PARM_AF_MODE = %d\n", focus_mode);
				   
				focus_mode = mode;
				rc = isx006_cancel_focus(focus_mode);
				break;
		
			case CFG_SET_DEFAULT_FOCUS:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_DEFAULT_FOCUS\n");
				   
				rc = isx006_set_focus();
				break;
		
			case CFG_MOVE_FOCUS:
				
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_MOVE_FOCUS: steps=%d\n",
								mode);
				rc = isx006_move_focus(mode);
				break;	
		
			case CFG_SET_CANCEL_FOCUS:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_CANCEL_FOCUS\n");
				   
				rc = isx006_cancel_focus(focus_mode);
				break;
					
#endif
		
#if !ALESSI_TUNING_SET		
			case CFG_SET_WB:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_WB\n");
					
				rc = isx006_set_wb(mode);
				break;
		
			case CFG_SET_ISO:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_ISO\n");
				rc = isx006_set_iso(mode);
				break;
		
			case CFG_SET_SCENE:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_SCENE_MODE\n");
				rc = isx006_set_scene_mode(mode);
				break;
		
			case CFG_SET_BRIGHTNESS:
				if(debug_mask)
					printk("isx006_sensor_config: command is CFG_SET_EXPOSURE_VALUE\n");
				rc = isx006_set_brightness(mode);
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
	printk("enqueue cfg_wq_num = %d,cfgtype = %d, mode = %d \n", cfg_wq_num, cfgtype, mode);

	if(cfg_wq_num == CFG_WQ_SIZE)
		return;

	cfg_wq[cfg_wq_num].cfgtype = cfgtype;
	cfg_wq[cfg_wq_num].mode= mode;

	++cfg_wq_num;
}


int isx006_reg_tuning(void *data)
{
	int rc = 0;
	int i;

	mutex_lock(&isx006_tuning_mutex);
	cfg_wq = kmalloc(sizeof(struct config_work_queue) * CFG_WQ_SIZE,
		GFP_KERNEL);
	cfg_wq_num = 0;
	tuning_thread_run = 1;
	mutex_unlock(&isx006_tuning_mutex);

#if ALESSI_TUNING_SET
	rc = isx006_reg_init_ext();
#else
	rc = isx006_i2c_write_table(&isx006_regs.init[0], isx006_regs.init_size);	
#endif
	if(rc<0){
		printk("isx006: init writing failed\n");
		return rc; 
	}

	mutex_lock(&isx006_tuning_mutex);
	dequeue_cfg_wq(cfg_wq);
	kfree(cfg_wq);
	tuning_thread_run = 0;
	mutex_unlock(&isx006_tuning_mutex);

	return rc;
}

int isx006_sensor_init(const struct msm_camera_sensor_info *data)
{
	int i=0;
	int rc;	
	uint16_t ap_check = 0, ap_check_16 = 0, real_ap_check = 0;	

	printk("isx006_init start!\n");

	enable_capturemode = 0;
	prev_af_mode = -1;
	//prev_scene_mode = -1;
	previous_mode = 0;
	memset(&pict_size,0x0, sizeof(pict_size));
	
	isx006_ctrl = kzalloc(sizeof(struct isx006_ctrl), GFP_KERNEL);
	if (!isx006_ctrl) {
		printk("isx006_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	if (data)
		isx006_ctrl->sensordata = data;
	
	rc = isx006_ctrl->sensordata->pdata->camera_power_on();
	if(rc<0){
	   printk("isx006: pll writing fail\n");
	   goto init_fail;
	}


#if ALESSI_TUNING_SET
	isx006_reg_pll_ext();
#else
#if 1 //--> 20100702 : HongMiJi D요청으로 0x0009전에 해당 ap값 setting후 0x0009값 setting
      //--> preview 결함 보정 addition code

	rc = isx006_i2c_write_table(&isx006_regs.pll[0], (isx006_regs.pll_size - 1));
	if(rc<0){
	   printk("isx006: pll writing fail\n");
	   goto init_fail;
	}
	rc = isx006_i2c_read(isx006_client->addr, 0x0368, &ap_check, WORD_LEN); // OTP_CHIPID_L
	if(rc<0){
	   printk("isx006: ap writing fail\n");
	   goto init_fail;
	}

	ap_check_16 = ap_check * 0x1111; // 16진수로 change

	printk("isx006: ap_check_16 value = 0x%x\n", ap_check_16);
	real_ap_check = ((ap_check_16 & 0x000000FF) >> 2) & 0x000F; // OTP_CHIPID_L[29:26]
	printk("isx006: real_ap_check value = 0x%x\n", real_ap_check);

	if(real_ap_check == 0x01) //OTP_CHIPID_L[29:26]
		rc = isx006_i2c_write_table(&isx006_regs.ap003_16bit_settings[0], isx006_regs.ap003_16bit_settings);
	else if(real_ap_check == 0x04) //OTP_CHIPID_L[29:26]
		rc = isx006_i2c_write_table(&isx006_regs.ap001_16bit_settings[0], isx006_regs.ap001_16bit_settings);
	else
		printk("isx006: real_ap_check error\n");

	//마지막 0x0009 setting을 해준다.
	rc = isx006_i2c_write_table(&isx006_regs.pll[isx006_regs.pll_size - 1], 1);
	if(rc<0){
	   printk("isx006: pll writing fail\n");
	   goto init_fail;
	}	
#else
	rc = isx006_i2c_write_table(&isx006_regs.pll[0], isx006_regs.pll_size);
	if(rc<0){
	   printk("isx006: pll writing fail\n");
	   goto init_fail; 
	}
#endif	
#endif
	mdelay(16);  // T3+T4

#if 0 //ALESSI_TUNING_SET	
	rc = isx006_reg_init_ext();
	if(rc<0){
		printk("isx006: init writing failed\n");
		goto init_fail; 
	}	
#else	

#if 1	//0823
{
	struct task_struct *p;
	p = kthread_run(isx006_reg_tuning, 0, "reg_tunning");
	if(IS_ERR(p))
	{
		printk("isx006: init writing failed\n");
		goto init_fail; 
	}
	
}
#else
	rc = isx006_i2c_write_table(&isx006_regs.init[0], isx006_regs.init_size);
	if(rc<0){
		printk("isx006: init writing failed\n");
		goto init_fail; 
	}
#endif
	
	cfg_wq = 0;
	tuning_thread_run = 0;
	statuscheck = 1;	

#endif
	return rc;

init_done:
	return rc;

init_fail:
	printk("isx006: isx006_sensor_init failed\n");
	kfree(isx006_ctrl);
	return rc;
}

static int isx006_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&isx006_wait_queue);
	return 0;
}

int isx006_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cfg_data;	
	long  rc = 0;	
	 
	rc = copy_from_user(&cfg_data,(void *)argp,sizeof(struct sensor_cfg_data));
	if(rc < 0)
	{
		printk("isx006_ioctl mhlee cfgtype = %d, mode = %d\n",
			cfg_data.cfgtype, cfg_data.mode);
		return -EFAULT;
	}
	if(debug_mask)
		printk("isx006_ioctl m, cfgtype = %d, mode = %d\n",
			cfg_data.cfgtype, cfg_data.mode);
	
	mutex_lock(&isx006_tuning_mutex);
	if (statuscheck == 1) {		
		int i=0;	
		printk("Sensor Preview Mode check : start \n");
		// enable상태가 가능한지 check 
		for(i = 0; i < 200; i++)
		{
			unsigned short changed_status = 0;
			rc = isx006_i2c_read(isx006_client->addr, 0x0004, &changed_status, BYTE_LEN);	
			printk("Sensor Preview Mode check : %d-> success \n", changed_status);
			if(changed_status == 4)
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
	mutex_unlock(&isx006_tuning_mutex);
	
	#if 1
	mutex_lock(&isx006_tuning_mutex);
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
		mutex_unlock(&isx006_tuning_mutex);
		return 0;
	}
	mutex_unlock(&isx006_tuning_mutex);
	#endif

	mutex_lock(&isx006_mutex);
	switch (cfg_data.cfgtype) {
	case CFG_SET_MODE:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_MODE\n");

		pict_size.width = cfg_data.width;
		pict_size.height = cfg_data.height;		
		rc = isx006_set_sensor_mode(cfg_data.mode,cfg_data.width,cfg_data.height);
		break;

#if !ALESSI_TUNING_SET
	case CFG_SET_EFFECT:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_EFFECT\n");
			
		rc = isx006_set_effect(cfg_data.cfg.effect);
		break;
#endif
#if 0 
    case CFG_SET_ZOOM_VIDEO:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_ZOOM\n");
			
		rc = isx006_set_zoom(cfg_data.cfg.zoom);
		break;
#endif

#if !ALESSI_TUNING_SET
	case CFG_SET_ZOOM_SENSOR:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_ZOOM_SENSOR\n");
			
		rc = isx006_set_zoom_sensor(cfg_data.mode);
		break;

	case CFG_SET_FOCUS_RECT:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_FOCUS_RECT\n");
			
		rc = isx006_set_focus_rect(cfg_data.mode);
		break;
#endif
		
	case CFG_START_AF_FOCUS:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_START_AF_FOCUS = %d\n", focus_mode);
		   
		focus_mode = cfg_data.mode;
		
		rc = isx006_focus_config(cfg_data.mode);		
		break;

	case CFG_SET_PARM_AF_MODE:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_PARM_AF_MODE = %d\n", focus_mode);
		   
		focus_mode = cfg_data.mode;
		rc = isx006_cancel_focus(focus_mode);
		break;

	case CFG_SET_DEFAULT_FOCUS:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_DEFAULT_FOCUS\n");
		   
		rc = isx006_set_focus();
		break;

	case CFG_MOVE_FOCUS:
		
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_MOVE_FOCUS: steps=%d\n",
		    			cfg_data.cfg.focus.steps);
		rc = isx006_move_focus(cfg_data.cfg.focus.steps);
		break;	

	case CFG_SET_CANCEL_FOCUS:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_CANCEL_FOCUS\n");
		   
        rc = isx006_cancel_focus(focus_mode);
		break;
		 	
    case CFG_GET_AF_MAX_STEPS:
		cfg_data.max_steps = 20;
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;
	case CFG_CHECK_AF_DONE:
		rc = isx006_check_focus(&cfg_data.mode);
		if (copy_to_user((void *)argp,
				&cfg_data,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

#if !ALESSI_TUNING_SET		
	case CFG_SET_WB:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_WB\n");
			
		rc = isx006_set_wb(cfg_data.mode);
		break;

	case CFG_SET_ISO:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_ISO\n");
		rc = isx006_set_iso(cfg_data.mode);
		break;

	case CFG_SET_SCENE:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_SCENE_MODE\n");
		rc = isx006_set_scene_mode(cfg_data.mode);
		break;

	case CFG_SET_BRIGHTNESS:
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_SET_EXPOSURE_VALUE\n");
		rc = isx006_set_brightness(cfg_data.mode);
		break;
#endif
	case CFG_GET_CURRENT_ISO:
	{
		int iso_value = 0;
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_GET_CURRENT_ISO\n");
		rc = isx006_i2c_read(isx006_client->addr,
		0x00F0, &iso_value, BYTE_LEN);
		cfg_data.mode = iso_value;
		printk("isx006_sensor_config: iso current value = %d\n", iso_value);
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
	}
		break;

	case CFG_GET_CHECK_SNAPSHOT:
	{		
		if(debug_mask)
			printk("isx006_sensor_config: command is CFG_GET_CHECK_SNAPSHOT\n");		
		cfg_data.mode = enable_capturemode;
		printk("isx006_sensor_config: enable_capturemode = %d\n", enable_capturemode);
		if (copy_to_user((void *)argp,&cfg_data, sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
	}
		break;

#if !ALESSI_TUNING_SET  
	default:
		rc = -EFAULT;
#endif
	}
	
	mutex_unlock(&isx006_mutex);
	if (rc < 0)
		printk("isx006: ERROR in sensor_config, %ld\n", rc);

	if(debug_mask)
	    printk("isx006_sensor_config: end [%lu]\n", jiffies);
	
	return rc;
 	
}

/* =====================================================================================*/
/* isx006 sysf                                                                          */
/* =====================================================================================*/

static ssize_t isx006_write_byte_store(struct device* dev, struct device_attribute* attr,const char* buf, size_t n)
{
	unsigned int val;
	unsigned short waddr, wdata;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);
	waddr=(val & 0xffff00)>>8;
	wdata=(val & 0x0000ff);

	rc = isx006_i2c_write(isx006_client->addr, waddr, wdata, BYTE_LEN);
	if (rc < 0)
		printk("isx006: failed to write register\n");

	return n;
}

static DEVICE_ATTR(write_byte, S_IRUGO|S_IWUGO, NULL, isx006_write_byte_store);

static ssize_t isx006_write_word_store(struct device* dev, struct device_attribute* attr,const char* buf, size_t n)
{
	unsigned int val;
	unsigned short waddr, wdata;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);
	waddr=(val & 0xffff0000)>>16;
	wdata=(val & 0x0000ffff);

	rc = isx006_i2c_write(isx006_client->addr, waddr, wdata, WORD_LEN);
	if (rc < 0)
		printk("isx006: failed to write register\n");

	return n;
}

static DEVICE_ATTR(write_word, S_IRUGO|S_IWUGO, NULL, isx006_write_word_store);

static ssize_t isx006_af_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%d",&val);

	rc = isx006_focus_config(val);
	if (rc < 0)
		printk("isx006: failed to set autofocus\n");

	return n;
}

static DEVICE_ATTR(af, S_IRUGO|S_IWUGO, NULL, isx006_af_store);

static ssize_t isx006_move_focus_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%d",&val);

	rc = isx006_move_focus(val);
	if (rc < 0)
		printk("isx006: failed to set autofocus\n");

	return n;
}

static DEVICE_ATTR(mf, S_IRUGO|S_IWUGO, NULL, isx006_move_focus_store);

static ssize_t isx006_cancel_focus_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%d",&val);

	rc = isx006_cancel_focus(val);
	if (rc < 0)
		printk("isx006: failed to set cancel_focus\n");

	return n;
}

static DEVICE_ATTR(cf, S_IRUGO|S_IWUGO, NULL, isx006_cancel_focus_store);

static ssize_t isx006_zoom_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%d",&val);

	rc = isx006_set_zoom(val);
	if (rc < 0)
		printk("isx006: failed to set zoom\n");

	return n;
}

static DEVICE_ATTR(zoom, S_IRUGO|S_IWUGO, NULL, isx006_zoom_store);

static ssize_t isx006_brightness_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%d",&val);

	rc = isx006_set_brightness(val);
	if (rc < 0)
		printk("isx006: failed to set brightness\n");

	return n;
}

static DEVICE_ATTR(brightness, S_IRUGO|S_IWUGO, NULL, isx006_brightness_store);

static ssize_t isx006_scene_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	if(val < CAMERA_SCENE_AUTO || val > CAMERA_SCENE_SUNSET) {
		printk("[isx006.c] invalid scene mode input\n");
		return 0;
	}
	rc = isx006_set_scene_mode(val);
	if (rc < 0)
		printk("isx006: failed to set scene\n");

	return n;
}
static DEVICE_ATTR(scene, S_IRUGO|S_IWUGO, NULL, isx006_scene_store);

static ssize_t isx006_wb_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	if(val < CAMERA_WB_MIN_MINUS_1 || val > CAMERA_WB_MAX_PLUS_1) {
		printk("isx006: invalid white balance input\n");
		return 0;
	}

	rc = isx006_set_wb(val);
	if (rc < 0)
		printk("isx006: failed to set white balance\n");

	return n;
}
static DEVICE_ATTR(wb, S_IRUGO|S_IWUGO, NULL, isx006_wb_store);

static ssize_t isx006_effect_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t n)
{
	int val;
	long rc;

	if (isx006_ctrl == NULL)
		return 0;

	sscanf(buf,"%x",&val);

	if(val < CAMERA_EFFECT_OFF || val > CAMERA_EFFECT_MAX) {
		printk("isx006: invalid effect input\n");
		return 0;
	}

	rc = isx006_set_effect(val);
	if (rc < 0)
		printk("isx006: failed to set effect\n");

	return n;
}
static DEVICE_ATTR(effect, S_IRUGO|S_IWUGO, NULL, isx006_effect_store);

static struct attribute* isx006_sysfs_attrs[] = {
	&dev_attr_write_byte.attr,
	&dev_attr_write_word.attr,
	&dev_attr_af.attr,
	&dev_attr_mf.attr,
	&dev_attr_cf.attr,
	&dev_attr_effect.attr,
	&dev_attr_wb.attr,
	&dev_attr_scene.attr,
	&dev_attr_zoom.attr,
	&dev_attr_brightness.attr,  
	NULL
};

static void isx006_sysfs_add(struct kobject* kobj)
{
	int i, n, ret;
	n = ARRAY_SIZE(isx006_sysfs_attrs);
	for(i = 0; i < n; i++){
		if(isx006_sysfs_attrs[i]){
			ret = sysfs_create_file(kobj, isx006_sysfs_attrs[i]);
			if(ret < 0)
				printk("isx006 sysfs is not created\n");
		}
	}
}

/*======================================================================================*/
/*  end :  sysf                                                                         */
/*======================================================================================*/

int isx006_sensor_release(void)
{
	int rc = 0;
	int i = 0;
	mutex_lock(&isx006_mutex);

	rc = isx006_check_thread_run();
	if (rc < 0)
	{
		printk("isx006_check_thread_run error\n");
	}

	rc = isx006_ctrl->sensordata->pdata->camera_power_off();

	kfree(isx006_ctrl);

	mutex_unlock(&isx006_mutex);

	isx006_ctrl=NULL;
	
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_ALESSI)
		/* _S. Change code to apply new LUT for display quality. 2010-08-13. minjong.gong */
		mdp_load_thunder_lut(1);	// Normal LUT
#endif
	return rc;
}

static int isx006_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		rc = -ENOTSUPP;
		goto probe_failure;
	}

	isx006_sensorw = kzalloc(sizeof(struct isx006_work), GFP_KERNEL);

	if (!isx006_sensorw) {
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, isx006_sensorw);
	isx006_init_client(client);
	isx006_client = client;
	
	isx006_sysfs_add(&client->dev.kobj);

	if(debug_mask)
		printk("isx006: isx006_probe succeeded!\n");

	return 0;
	
probe_failure:
	kfree(isx006_sensorw);
	isx006_sensorw = NULL;
	printk("isx006_probe failed!\n");
	return rc;
}

static const struct i2c_device_id isx006_i2c_id[] = {
	{ "isx006", 0},
	{ },
};

static struct i2c_driver isx006_i2c_driver = {
	.id_table = isx006_i2c_id,
	.probe  = isx006_i2c_probe,
	.remove = __exit_p(isx006_i2c_remove),
	.driver = {
		.name = "isx006",
	},
};

static int isx006_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *s)
{
	int rc = i2c_add_driver(&isx006_i2c_driver);
	printk("isx006: isx006_sensor_probe\n");	

	printk("isx006: isx006_client = %d, rc = %d\n",isx006_client,rc);	

	if (rc < 0 || isx006_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;

	}

#if 0	
	isx006_sensor_power_enable();
	mdelay(1);
#endif	

#if 0
	rc = isx006_reset(info,1);
	if (rc < 0) {
		printk("isx006: reset failed!\n");
	}
	mdelay(10);

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_ALESSI)
	/* _S. Change code to apply new LUT for display quality. 2010-08-13. minjong.gong */
	mdp_load_thunder_lut(2);	// Camera LUT
#endif
	rc = isx006_sensor_init_probe(info);
	if (rc < 0)
		goto probe_done;
	
	mdelay(10);

	rc = isx006_pwdn(info,1);
	if (rc < 0) {
		printk("isx006: pwdn failed!\n");
	}
	mdelay(10);
	
	rc = isx006_pwdn(info,0);
	if (rc < 0) {
		printk("isx006: pwdn failed!\n");
	}
	mdelay(1);
	
	rc = isx006_reset(info,0);
	if (rc < 0) {
		printk("isx006: reset failed!\n");
	}
	mdelay(1);
	isx006_sensor_power_disable();
#endif
	s->s_init = isx006_sensor_init;
	s->s_release = isx006_sensor_release;
	s->s_config  = isx006_sensor_config;

probe_done:
	printk("%s %s:%d\n", __FILE__, __func__, __LINE__);
	return rc;
}

static int __isx006_probe(struct platform_device *pdev)
{
	printk("isx006: __isx006_probe\n");

	return msm_camera_drv_start(pdev, isx006_sensor_probe);	
}

static struct platform_driver msm_camera_driver = {
	.probe = __isx006_probe,
	.driver = {
		.name = "msm_camera_isx006",
		.owner = THIS_MODULE,
	},
};

static int __init isx006_init(void)
{
	return platform_driver_register(&msm_camera_driver);  
}

module_init(isx006_init);
