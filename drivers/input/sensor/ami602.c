/* drivers/i2c/chips/ami602.c - AMI602 motion sensor driver
 *
 * Copyright (C) 2009 AMIT Technology Inc.
 * Author: Kyle Chen <sw-support@amit-inc.com>
 * Modified by EunYoung, Cho <eycho1004@lge.com> LGE Inc.
 *  - Modified power sequence
 *  - I2c driver register(probe, release)
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
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
//#include <linux/ami602.h> /*LGE_CHANGE 2009.09.27 [eycho1004@lge.com]*/
#include <linux/kobject.h>
#include <mach/hardware.h>
//#include <asm-arm/arch/regs-gpio.h> /*LGE_CHANGE 2009.09.27 [eycho1004@lge.com]*/
//#include <asm-arm/arch/irqs.h> /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]*/
#include <linux/spinlock.h>
#include <asm/io.h>

/*LGE_CHANGE_S 2009.09.27 [eycho1004@lge.com] - Add head file and defined debug message*/
#include <linux/interrupt.h>
#include <mach/vreg.h>
#include <mach/gpio.h>	

#include <mach/board_lge.h>

#include "ami602.h"

#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>

struct early_suspend ami602_sensor_early_suspend;

static void ami602_early_suspend(struct early_suspend *h);
static void ami602_late_resume(struct early_suspend *h);
#endif


#define AMI602_DEBUG_PRINT
#define AMI602_ERROR_PRINT
//#undef AMI602_DEBUG_PRINT
//#undef AMI602_ERROR_PRINT

#if defined(AMI602_DEBUG_PRINT)
#define AMID(fmt, args...)  printk(KERN_ERR "AMI602-DBG[%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)
#else
#define AMID(fmt, args...)
#endif


#if defined(AMI602_ERROR_PRINT)
#define AMIE(fmt, args...)  printk(KERN_ERR "AMI602-ERR[%-18s:%5d]" fmt, __FUNCTION__, __LINE__, ## args)
#else
#define AMIE(fmt, args...)
#endif

static DECLARE_WAIT_QUEUE_HEAD(wq_busy_rising);
static int busy_rising = 0;

#define AMI602_INIT			0
#define AMI602_HOST_ON		1	/*Polling   */
#define AMI602_SENSOR_ON	2	/*Interrupt */
#define AMI602_HOST_OFF		3
#define AMI602_SENSOR_OFF	4
#define AMI602_OFF			5

static int mode = AMI602_INIT;

/*LGE_CHANGE_E 2009.09.27 [eycho1004@lge.com]*/

static struct work_struct ami602_readmeasure_work;

static struct i2c_client *ami602_i2c_client = NULL;

/* Addresses to scan */
static unsigned short normal_i2c[] = { AMI602_I2C_SLAVE_ADDR, I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(ami602);

#if 0 /* AICHI STEEL Orginal Source Code */
static int ami602_i2c_attach_adapter(struct i2c_adapter *adapter);
static int ami602_i2c_detect(struct i2c_adapter *adapter, int address, int kind);
static int ami602_i2c_detach_client(struct i2c_client *client);

struct ami602_i2c_data {
	struct i2c_client client;
};

static struct i2c_driver ami602_i2c_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ami602",
	},
	.attach_adapter	= ami602_i2c_attach_adapter,
	.detach_client	= ami602_i2c_detach_client,	
	.id	        = I2C_DRIVERID_AMI602,
};
#endif

struct _ami602_data {
	rwlock_t lock;
//	int mode; /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]*/
	int rate;
	volatile int updated;
	int ch1;
	int ch2;
	int ch3;
	int ch4;
	int ch5;
	int ch6;
} ami602_data;

struct _ami602mid_data {
	rwlock_t datalock;
	rwlock_t ctrllock;
	int controldata[5];	
	int yaw;
	int roll;
	int pitch;
	int nmx;
	int nmy;
	int nmz;
	int nax;
	int nay;
	int naz;
	int mag_status;
} ami602mid_data;

#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
static int
s3c_irqext_type(unsigned int irq, unsigned int type)
{

	unsigned long newvalue = 0;


	/* Set the external interrupt to pointed trigger type */
	switch (type) {
	case IRQT_NOEDGE:
		printk(KERN_WARNING "No edge setting!\n");
		break;

	case IRQT_RISING:
		newvalue = S3C_EXTINT_RISEEDGE;
		break;

	case IRQT_FALLING:
		newvalue = S3C_EXTINT_FALLEDGE;
		break;

	case IRQT_BOTHEDGE:
		newvalue = S3C_EXTINT_BOTHEDGE;
		break;

	case IRQT_LOW:
		newvalue = S3C_EXTINT_LOWLEV;
		break;

	case IRQT_HIGH:
		newvalue = S3C_EXTINT_HILEV;
		break;

	default:
		printk(KERN_ERR "No such irq type %d", type);
		return -1;
	}

	switch (irq) {
	case AMI602_IRQ:
		s3c_gpio_cfgpin(S3C_GPN9, S3C_GPN9_EXTINT9);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 16)) |
			     (newvalue << 16), S3C_EINTCON0);
		break;

	case IRQ_EINT10:
		s3c_gpio_cfgpin(S3C_GPN10, S3C_GPN10_EXTINT10);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 20)) |
			     (newvalue << 20), S3C_EINTCON0);
		break;

	case IRQ_EINT11:
		s3c_gpio_cfgpin(S3C_GPN11, S3C_GPN11_EXTINT11);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 20)) |
			     (newvalue << 20), S3C_EINTCON0);
		break;
	case IRQ_EINT12:
		s3c_gpio_cfgpin(S3C_GPN12, S3C_GPN12_EXTINT12);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 24)) |
			     (newvalue << 24), S3C_EINTCON0);
		break;

	case IRQ_EINT13:
		s3c_gpio_cfgpin(S3C_GPN13, S3C_GPN13_EXTINT13);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 24)) |
			     (newvalue << 24), S3C_EINTCON0);
		break;

	case IRQ_EINT14:
		s3c_gpio_cfgpin(S3C_GPN14, S3C_GPN14_EXTINT14);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 28)) |
			     (newvalue << 28), S3C_EINTCON0);
		break;

	case IRQ_EINT15:
		s3c_gpio_cfgpin(S3C_GPN15, S3C_GPN15_EXTINT15);
		__raw_writel((__raw_readl(S3C_EINTCON0) & ~(0x7 << 28)) |
			     (newvalue << 28), S3C_EINTCON0);
		break;	
	default:
		printk(KERN_ERR
		       "s3c_irqext_type : Only support EINT9,10,11,12,13,14,15 interrupt.\n");
		break;

	}
	return 0;
}
#endif 
/*LGE_CHANGE_E 2009.10.02 [eycho1004@lge.com]*/

/*LGE_CHANGE_S 2009.09.27 [eycho1004@lge.com] - Changed GPIO API*/
#if 1
static int AMI602_SendTrigger(void)
{
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;
	
	gpio_set_value(gyro_pdata->pin_trg, 0);
	udelay(20);
	gpio_set_value(gyro_pdata->pin_trg, 1);
	udelay(20);
	return 0;
}

#else /* AICHI STEEL Orginal Source Code */
static int AMI602_SendTrigger(void)
{
	s3c_gpio_setpin(S3C_GPK0, 0);
	udelay(200);
	s3c_gpio_setpin(S3C_GPK0, 1);	
	udelay(100);
	return 0;
}
#endif
/*LGE_CHANGE_E 2009.09.27 [eycho1004@lge.com]*/

static int AMI602_SetMode(int newmode)
{
	u8 databuf[10];	
	int res = 0;
//	int mode = 0; /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- AICHI STEEL Orginal Source Code*/

#if 0 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	read_lock(&ami602_data.lock);
	mode = ami602_data.mode;
	read_unlock(&ami602_data.lock);	
#endif

	if (mode == newmode)
		return 0;	

	memset(databuf, 0, sizeof(u8)*10);
#if 0	/*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- AICHI STEEL Orginal Source Code*/
	if (mode==2) {	
#else
	if (mode > 2){
#endif		
		AMI602_SendTrigger();
		udelay(1000);		
	}	
#if 0	/*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- AICHI STEEL Orginal Source Code*/	
	if (newmode==0) {	
#else		
	if (newmode == AMI602_HOST_ON) {
#endif		
		//Stop 6CH sensor triggered measurement
		AMI602_SendTrigger();
		databuf[0] = AMI602_CMD_SET_MES_6CH_AUTO_STOP;
		databuf[1] = 'e';
		databuf[2] = 'd';		
		res = i2c_master_send(ami602_i2c_client, databuf, 0x03);	
		if (res<=0)
			goto exit_AMI602_SetMode;	
		udelay(350);
		res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
		if (res<=0)
			goto exit_AMI602_SetMode;	
		
		udelay(450);	
		AMI602_SendTrigger();
		//Set Enable of accelerometers
		databuf[0] = AMI602_CMD_SET_AEN;
		databuf[1] = 0x01;
		res = i2c_master_send(ami602_i2c_client, databuf, 0x02);	
		if (res<=0)
			goto exit_AMI602_SetMode;	
		udelay(350);
		res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
		if (res<=0)
			goto exit_AMI602_SetMode;

//		flush_scheduled_work();	/*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- AICHI STEEL Orginal Source Code*/
#if 1 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- Add MODE STATE STORE*/
		mode = AMI602_HOST_ON; 
#endif

	}
#if 0	/*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- AICHI STEEL Orginal Source Code*/
	else if (newmode == 1){
#else
	else if (newmode == AMI602_SENSOR_ON){
#endif		
		AMI602_SendTrigger();
		//Set Enable of accelerometers
		databuf[0] = AMI602_CMD_SET_AEN;
		databuf[1] = 0x01;
		res = i2c_master_send(ami602_i2c_client, databuf, 0x02);	
		if (res<=0)
			goto exit_AMI602_SetMode;	
		udelay(350);
		res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
		if (res<=0)
			goto exit_AMI602_SetMode;	
			
		udelay(30);			
		AMI602_SendTrigger();
		//Start 6CH sensor triggered measurement
		databuf[0] = AMI602_CMD_SET_MES_6CH_AUTO_START;
		databuf[1] = 0x01;
		databuf[2] = 's';
		databuf[3] = 't';
		
		res = i2c_master_send(ami602_i2c_client, databuf, 0x04);	
		if (res<=0)
			goto exit_AMI602_SetMode;	

		res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
		if (res<=0)
			goto exit_AMI602_SetMode;	
#if 1 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- Add MODE STATE STORE*/
		mode = AMI602_SENSOR_ON;
#endif
	}
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	else if (newmode == 2) {
#else		
	else if (newmode == AMI602_HOST_OFF || newmode == AMI602_SENSOR_OFF) {
#endif		
		AMI602_SendTrigger();
		//Transfer to Power Down mode
		databuf[0] = AMI602_CMD_SET_PWR_DOWN;
		databuf[1] = 'p';
		databuf[2] = 'd';
		
		res = i2c_master_send(ami602_i2c_client, databuf, 0x03);	
		if (res<=0)
			goto exit_AMI602_SetMode;	
		udelay(350);
		res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
		if (res<=0)
			goto exit_AMI602_SetMode;

#if 1 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- Add MODE STATE STORE*/
		if(mode == AMI602_HOST_ON)
			mode = AMI602_HOST_OFF;
		else
			mode = AMI602_SENSOR_OFF;
#endif		
	}
	else
		return -3;
	
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/					
	write_lock(&ami602_data.lock);
	ami602_data.mode = newmode;
	write_unlock(&ami602_data.lock);			
	printk(KERN_INFO "Set AMI602 to mode %d\n", newmode);
#endif	
	
exit_AMI602_SetMode:	
	if (res<=0) {
		return -1;
	}	
	else if (databuf[0] != 0) {
		return -2;
	}	
	return 0;
}

/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - sequence of power*/
static int AMI602_Power_On(void)
{
	int res = 0;
	
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;

	gpio_set_value(gyro_pdata->pin_trg, 1);

	gyro_pdata->power(1);
	udelay(150);
	
	gpio_set_value(gyro_pdata->pin_rst, 1);
	mdelay(35);
	
	wait_event_interruptible_timeout(wq_busy_rising, busy_rising, HZ);
	if (!busy_rising) {
		//param->result = AMID_BOOT_ERR;
		res = gpio_get_value(gyro_pdata->pin_busy);
		AMIE(" ERR - wait_event_interruptible_timeout: busy_rising= %d \n", res);
		return -1;
	}
	
	return res;
}	
/*LGE_CHANGE_E 2009.10.02 [eycho1004@lge.com]*/

#if 1 /*LGE_CHANGE_E 2009.10.02 [eycho1004@lge.com] Changed Host Trigger Mode initialization*/
static int AMI602_Init(void)
{
	u8 databuf[10];	
	int res = 0;

	AMI602_SendTrigger();
	
	memset(databuf, 0, sizeof(u8)*10);
// Enable AEN 
	databuf[0] = AMI602_CMD_SET_AEN;
	databuf[1] = 0x01;
	
	res = i2c_master_send(ami602_i2c_client, databuf, 0x02);	
	if (res<=0)
		goto exit_AMI602_Init;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init;	
	
	mode = AMI602_HOST_ON;

	udelay(30);
	AMI602_SendTrigger();
	//Stop 6CH sensor triggered measurement
	databuf[0] = AMI602_CMD_SET_MES_6CH_AUTO_STOP;
	databuf[1] = 'e';
	databuf[2] = 'd';		
	res = i2c_master_send(ami602_i2c_client, databuf, 0x03);	
	if (res<=0)
		goto exit_AMI602_Init;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init;	


exit_AMI602_Init:	
	if (res<=0) {
		AMIE("faild I2c send %d\n", res);
		return -1;
	}	
	else if (databuf[0] != 0) {
		AMIE("faild I2c recv %d\n", res);
		return -2;
	}
	
	return 0;
}

#else /*AICHI STEEL Orginal Source Code*/
static int AMI602_Init(void)
{
	u8 databuf[10];	
	int res = 0;
	
	write_lock(&ami602_data.lock);
	ami602_data.mode = 0;//host trigger mode
	write_unlock(&ami602_data.lock);	
		
	AMI602_SendTrigger();
	
	memset(databuf, 0, sizeof(u8)*10);	
	// Enable AEN 
	databuf[0] = AMI602_CMD_SET_AEN;
	databuf[1] = 0x01;
	res = i2c_master_send(ami602_i2c_client, databuf, 0x02);	
	if (res<=0)
		goto exit_AMI602_Init;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init;	
	
	udelay(30);
	AMI602_SendTrigger();
	//Stop 6CH sensor triggered measurement
	databuf[0] = AMI602_CMD_SET_MES_6CH_AUTO_STOP;
	databuf[1] = 'e';
	databuf[2] = 'd';		
	res = i2c_master_send(ami602_i2c_client, databuf, 0x03);	
	if (res<=0)
		goto exit_AMI602_Init;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init;	
			
exit_AMI602_Init:	
	if (res<=0) {
		return -1;
	}	
	else if (databuf[0] != 0) {
		return -2;
	}	
	return 0;
}
#endif

static int AMI602_Init_SensorTrigger(void)
{
	u8 databuf[10];	
	int res = 0;
	
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	AMID("SensorTrigger start... \n");
	write_lock(&ami602_data.lock);
	ami602_data.mode = 1;//sensor trigger mode
	write_unlock(&ami602_data.lock);	
#endif

	AMI602_SendTrigger();
	
	memset(databuf, 0, sizeof(u8)*10);	
	
	// Enable AEN 
	databuf[0] = AMI602_CMD_SET_AEN;
	databuf[1] = 0x01;
	res = i2c_master_send(ami602_i2c_client, databuf, 0x02);	
	if (res<=0)
		goto exit_AMI602_Init_SensorTrigger;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init_SensorTrigger;	
		
	udelay(30);	
	AMI602_SendTrigger();
	//Start 6CH sensor triggered measurement
	databuf[0] = AMI602_CMD_SET_MES_6CH_AUTO_START;
	databuf[1] = 0x01;   // 0x00:20ms, 0x01:40ms, 0x02:60ms, 0x03:80ms, 0x04:100ms
	databuf[2] = 's';
	databuf[3] = 't';
	
	res = i2c_master_send(ami602_i2c_client, databuf, 0x04);	
	if (res<=0)
		goto exit_AMI602_Init_SensorTrigger;	
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 0x01);
	if (res<=0)
		goto exit_AMI602_Init_SensorTrigger;	
	
#if 1 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- Add MODE STATE STORE*/			
	mode = AMI602_SENSOR_ON;
#endif	

exit_AMI602_Init_SensorTrigger:	
	if (res<=0) {
		return -1;
	}	
	else if (databuf[0] != 0) {
		return -2;
	}	
	return 0;
}

static int AMI602_ReadChipInfo(char *buf, int bufsize)
{
	u8 databuf[10];	
	char cmd;
	int res = 0;
	
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	int mode = 0;
	
	read_lock(&ami602_data.lock);
	mode = ami602_data.mode;
	read_unlock(&ami602_data.lock);		
#endif

	cmd = AMI602_CMD_GET_FIRMWARE;	
	memset(databuf, 0, sizeof(u8)*10);
		
	if ((!buf)||(bufsize<=50))
		return -1;
#if 0	/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	if (mode != 0){	
#else		
	if (mode != AMI602_HOST_ON){
#endif		
		sprintf(buf, "Only supported in Host Trigger Mode");
		return -5;		
	}		

	if (!ami602_i2c_client)
	{
		*buf = 0;
		return -2;
	}
	AMI602_SendTrigger();
	res = i2c_master_send(ami602_i2c_client, &cmd, 0x1);	
	if (res<=0)
		goto exit_AMI602_ReadChipInfo;	
	udelay(200);
	res = i2c_master_recv(ami602_i2c_client, databuf, 8);
	if (res<=0)
		goto exit_AMI602_ReadChipInfo;		
	
exit_AMI602_ReadChipInfo:	
	if (res<=0) {
		sprintf(buf, "I2C error: ret value=%d", res);
		return -3;
	}	
	else if (databuf[0] == 0) {
		sprintf(buf, "AMI602 Firmware Version:%x.%x, Date:%02x%02x/%02x/%02x"
		    , databuf[1], databuf[2], databuf[3], databuf[4], databuf[5], databuf[6]);
	}
	else {
		sprintf(buf, "Failed to execute I2C command");
		return -4;
	}
	return 0;
}

static int AMI602_ReadSensorDataFromChip(void)
{
	char cmd;
    u8 databuf[20];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);
	if (!ami602_i2c_client)
		return -3;
	AMI602_SendTrigger(); 
	
	cmd = AMI602_CMD_GET_MES_SUSPEND;
	res = i2c_master_send(ami602_i2c_client, &cmd, 0x1);	
	if (res<=0)
		goto exit_AMI602_ReadSensorDataFromChip;		
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 10);	
	if (res<=0)
		goto exit_AMI602_ReadSensorDataFromChip;	
	
	if (databuf[0] == 0) {
//		write_lock(&ami602_data.lock);  /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
		ami602_data.ch1 = ((int) databuf[1]) << 4 | ((int) databuf[2]) >> 4;
		ami602_data.ch2 = ((int) databuf[2] & 0x0f)   << 8  | ((int) databuf[3]);
		ami602_data.ch3 = ((int) databuf[4]) << 4 | ((int) databuf[5]) >> 4;
		ami602_data.ch4 = ((int) databuf[5] & 0x0f)   << 8  | ((int) databuf[6]);
		ami602_data.ch5 = ((int) databuf[7]) << 4 | ((int) databuf[8]) >> 4;
		ami602_data.ch6 = ((int) databuf[8] & 0x0f)   << 8  | ((int) databuf[9]);
		ami602_data.updated = 1;

//		write_unlock(&ami602_data.lock);		
	}				
	
exit_AMI602_ReadSensorDataFromChip:	
	if (res<=0) {
		return -1;
	}
	else if (databuf[0] != 0) {
		return -2;
	}
	return 0;
}

/*LGE_CHANGE_S [eycho1004@lge.com] changed ReadSensorData*/
static int AMI602_ReadSensorData_HostMode(char *buf, int bufsize)
{
	char cmd;
    u8 databuf[20];
	int ch1,ch2,ch3,ch4,ch5,ch6;
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);
		
	if ((!buf)||(bufsize<=80))
		return -1;
	if (!ami602_i2c_client)
	{
		*buf = 0;
		return -2;
	}
	
	AMI602_SendTrigger();
	cmd = AMI602_CMD_REQ_MES;

	res = i2c_master_send(ami602_i2c_client, &cmd, 0x1);	
	if (res<=0)
		goto exit_AMI602_ReadSensorData;
	udelay(350);

	res = i2c_master_recv(ami602_i2c_client, databuf, 1);
	if (res<=0)
		goto exit_AMI602_ReadSensorData;	

	msleep(4);
	AMI602_SendTrigger();
	//cmd = AMI602_CMD_GET_MES;
	cmd = 0x28;
	res = i2c_master_send(ami602_i2c_client, &cmd, 0x1);	
	if (res<=0)
		goto exit_AMI602_ReadSensorData;		
	udelay(350);
	res = i2c_master_recv(ami602_i2c_client, databuf, 10);	
	if (res<=0)
		goto exit_AMI602_ReadSensorData;	
		
	ch1 = ((int) databuf[1]) << 4 | ((int) databuf[2]) >> 4;
	ch2 = ((int) databuf[2] & 0x0f)   << 8  | ((int) databuf[3]);
	ch3 = ((int) databuf[4]) << 4 | ((int) databuf[5]) >> 4;
	ch4 = ((int) databuf[5] & 0x0f)   << 8  | ((int) databuf[6]);
	ch5 = ((int) databuf[7]) << 4 | ((int) databuf[8]) >> 4;
	ch6 = ((int) databuf[8] & 0x0f)   << 8  | ((int) databuf[9]);				
	
exit_AMI602_ReadSensorData:	
	if (res<=0) {
		sprintf(buf, "I2C error: ret value=%d", res);
		return -3;
	}
	else if (databuf[0] == 0) {
		sprintf(buf, "%04x %04x %04x %04x %04x %04x", ch1,ch2,ch3,ch4,ch5,ch6);
	}
	else {
		sprintf(buf, "Failed to execute I2C command");	
		return -4;
	}
	return 0;
}
/*LGE_CHANGE_E [eycho1004@lge.com] changed ReadSensorData*/

#if 0	/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
static int AMI602_ReadSensorDataHostMode(void)
{
	char cmd;
    u8 databuf[20];
	int res = 0;
	int updated = 0;

	memset(databuf, 0, sizeof(u8)*10);
	if (!ami602_i2c_client)
		return -3;
		
	AMI602_SendTrigger();
	
	cmd = AMI602_CMD_REQ_MES;
	res = i2c_master_send(ami602_i2c_client, &cmd, 0x1);	
	if (res<=0)
		goto exit_AMI602_ReadSensorDataHostMode;
	udelay(350);

	res = i2c_master_recv(ami602_i2c_client, databuf, 1);
	if (res<=0)
		goto exit_AMI602_ReadSensorDataHostMode;	
	
	write_lock(&ami602_data.lock);
	ami602_data.updated = 0;
	write_unlock(&ami602_data.lock);

	do	{
		read_lock(&ami602_data.lock);
		updated = ami602_data.updated;
		msleep(1);
		read_unlock(&ami602_data.lock);		
	} while (updated == 0);
	
exit_AMI602_ReadSensorDataHostMode:	
	if (res<=0) {
		return -1;
	}
	else if (databuf[0] != 0) {
		return -2;
	}
	return 0;
}
#endif

static int AMI602_ReadSensorData(char *buf, int bufsize)
{
	int ch1,ch2,ch3,ch4,ch5,ch6;
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	int mode = 0;
	int res=0;
#endif	
	
	if ((!buf)||(bufsize<=80))
		return -10;		
	
#if 0 /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	if (mode == AMI602_HOST_ON)
	{
		res = AMI602_ReadSensorDataHostMode();	
		if (res<0)
		{
			sprintf(buf, "Error in reading sensor!");		
			return res;
		}		
	}
#endif

//	read_lock(&ami602_data.lock);	/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	ch1 = ami602_data.ch1;
	ch2 = ami602_data.ch2;
	ch3 = ami602_data.ch3;
	ch4 = ami602_data.ch4;
	ch5 = ami602_data.ch5;
	ch6 = ami602_data.ch6;	
//	read_lock(&ami602_data.lock);	/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	

	sprintf(buf, "%04x %04x %04x %04x %04x %04x", ch1,ch2,ch3,ch4,ch5,ch6);
	return 0;
}

static int AMI602_ReadPostureData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;
	sprintf(buf, "%d %d %d %d", ami602mid_data.yaw, ami602mid_data.pitch, ami602mid_data.roll, ami602mid_data.mag_status);
	return 0;
}

static int AMI602_ReadCaliData(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;
//	read_lock(&ami602mid_data.datalock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	sprintf(buf, "%d %d %d %d %d %d %d", 
		ami602mid_data.nmx, ami602mid_data.nmy, ami602mid_data.nmz,ami602mid_data.nax,ami602mid_data.nay,ami602mid_data.naz, ami602mid_data.mag_status);		
//	read_unlock(&ami602mid_data.datalock);
	return 0;
}

static int AMI602_ReadMiddleControl(char *buf, int bufsize)
{
	if ((!buf)||(bufsize<=80))
		return -1;
//	read_lock(&ami602mid_data.datalock);  /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/	
	sprintf(buf, "%d %d %d %d %d", 
		ami602mid_data.controldata[0], ami602mid_data.controldata[1], ami602mid_data.controldata[2],ami602mid_data.controldata[3],ami602mid_data.controldata[4]);		
//	read_unlock(&ami602mid_data.ctrllock);	
	return 0;
}

static ssize_t show_chipinfo_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
	AMI602_ReadChipInfo(strbuf, AMI602_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);		
}

static ssize_t show_sensordata_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
#if 1 /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com]- SensorData*/	

	if(mode == AMI602_HOST_ON)
		AMI602_ReadSensorData_HostMode(strbuf, AMI602_BUFSIZE);
	else {
		AMI602_ReadSensorDataFromChip();
		AMI602_ReadSensorData(strbuf, AMI602_BUFSIZE);
	}
#else
	AMI602_ReadSensorData(strbuf, AMI602_BUFSIZE);
#endif
	return sprintf(buf, "%s\n", strbuf);			
}

static ssize_t show_posturedata_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
	AMI602_ReadPostureData(strbuf, AMI602_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);			
}

static ssize_t show_calidata_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
	AMI602_ReadCaliData(strbuf, AMI602_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);			
}

static ssize_t show_midcontrol_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
	AMI602_ReadMiddleControl(strbuf, AMI602_BUFSIZE);
	return sprintf(buf, "%s\n", strbuf);			
}

static ssize_t store_midcontrol_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
//	write_lock(&ami602mid_data.ctrllock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	sscanf(buf, "%d %d %d %d %d", &(ami602mid_data.controldata[0]), &(ami602mid_data.controldata[1]),
		   &(ami602mid_data.controldata[2]),&(ami602mid_data.controldata[3]),&(ami602mid_data.controldata[4]));	 		   
//	write_unlock(&ami602mid_data.ctrllock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
	return count;			
}
#if 1
/*LGE_CHANGE_S 2009.11.02 [eycho1004@lge.com] - mode to change*/
static ssize_t show_mode_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char *s;
	if(mode == AMI602_HOST_ON)
		s = "ami602 HostTrriger mode on";
	else if(mode == AMI602_HOST_OFF)
		s = "ami602 HostTrriger off";
	else if(mode == AMI602_SENSOR_ON)
		s = "ami602 SensorTrigger mode on";
	else if(mode == AMI602_SENSOR_OFF)
		s = "ami602 SensorTrigger mode off";
	else
		s = "unknown mode";

	return sprintf(buf, "%s\n", s);		
}

static ssize_t store_mode_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char sen_switch[8];
	sscanf(buf, "%4s", sen_switch);
	printk(KERN_INFO"%s\n", sen_switch);

	if (strncmp(sen_switch, "host", 1) == 0) {
		AMI602_SetMode(AMI602_HOST_ON);
		printk("ami602 HostTrigger mode on\n");
	}
	else if (strncmp(sen_switch, "sensor", 1) == 0) {
		AMI602_SetMode(AMI602_SENSOR_ON);
		printk("ami602 SensorTrigger mode on\n");
	}
	else if (strncmp(sen_switch, "off", 3) == 0) {
		if(mode == AMI602_HOST_ON)
			AMI602_SetMode(AMI602_HOST_OFF);
		else if(mode == AMI602_SENSOR_ON)
			AMI602_SetMode(AMI602_SENSOR_OFF);
		else
			AMIE("unknown mode\n");

		printk("ami602 host mode off\n");
	}

	return count;			
}
/*LGE_CHANGE_E 2009.11.02 [eycho1004@lge.com] */
#else

static ssize_t show_mode_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	char strbuf[AMI602_BUFSIZE];
	int mode=0;
	read_lock(&ami602_data.lock);
	mode = ami602_data.mode;
	read_unlock(&ami602_data.lock);		
	return sprintf(buf, "%d\n", mode);			
}

static ssize_t store_mode_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int mode = 0;
	sscanf(buf, "%d", &mode);	
    //echo ??1?? > /sys/bus/i2c/devices/0-0030/mode: may switch the sensor to sensor trigger mode, the sampling rate is set to be 25hz in the sample code. 
    //echo ??0?? > /sys/bus/i2c/devices/0-0030/mode: may switch the sensor to host trigger mode	
	AMI602_SetMode(mode);

	AMI602_Init_SensorTrigger();
	return count;			
}
#endif	

static DEVICE_ATTR(chipinfo, S_IRUGO, show_chipinfo_value, NULL);
static DEVICE_ATTR(sensordata, S_IRUGO, show_sensordata_value, NULL);
static DEVICE_ATTR(posturedata, S_IRUGO, show_posturedata_value, NULL);
static DEVICE_ATTR(calidata, S_IRUGO, show_calidata_value, NULL);
static DEVICE_ATTR(midcontrol, S_IRUGO | S_IWUSR, show_midcontrol_value, store_midcontrol_value );
static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR, show_mode_value, store_mode_value );

static int ami602mid_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int ami602mid_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int ami602mid_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	int valuebuf[4];
	int calidata[7];
	int controlbuf[5];
	void __user *data;
	int retval=0;
		
	switch (cmd) {			
		case AMI602MID_IOCTL_SET_POSTURE:
//			printk(KERN_INFO "Enter AMI602MID_IOCTL_SET_POSTURE\n"); /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&valuebuf, data, sizeof(valuebuf))) {
				retval = -EFAULT;
				goto err_out;
			}
//			printk(KERN_INFO "Set posture to be %d %d %d\n", valuebuf[0],valuebuf[1],valuebuf[2]); /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
//			write_lock(&ami602mid_data.datalock); /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			ami602mid_data.yaw   = valuebuf[0];
			ami602mid_data.pitch = valuebuf[1];
			ami602mid_data.roll  = valuebuf[2];
			ami602mid_data.mag_status = valuebuf[3];
//			write_unlock(&ami602mid_data.datalock); /*LGE_CHANGE 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			break;		
			
		case AMI602MID_IOCTL_SET_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(&calidata, data, sizeof(calidata))) {
				retval = -EFAULT;
				goto err_out;
			}
//			write_lock(&ami602mid_data.datalock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			ami602mid_data.nmx = calidata[0];
			ami602mid_data.nmy = calidata[1];
			ami602mid_data.nmz = calidata[2];
			ami602mid_data.nax = calidata[3];
			ami602mid_data.nay = calidata[4];
			ami602mid_data.naz = calidata[5];
			ami602mid_data.mag_status = calidata[6];	
//			write_unlock(&ami602mid_data.datalock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			break;								
	
		case AMI602MID_IOCTL_GET_CONTROL:
			memcpy(controlbuf, &ami602mid_data.controldata[0], sizeof(controlbuf));
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_to_user(data, controlbuf, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}					
			break;		
			
		case AMI602MID_IOCTL_SET_CONTROL:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			if (copy_from_user(controlbuf, data, sizeof(controlbuf))) {
				retval = -EFAULT;
				goto err_out;
			}
//			write_lock(&ami602mid_data.ctrllock); /*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			memcpy(&ami602mid_data.controldata[0], controlbuf, sizeof(controlbuf));									
//			write_unlock(&ami602mid_data.ctrllock);	/*LGE_CHANGE_S 2009.10.02 [eycho1004@lge.com] - AICHI STEEL Orginal Source Code*/
			break;					
		default:
			printk(KERN_ERR "%s not supported = 0x%04x", __FUNCTION__, cmd);
			retval = -ENOIOCTLCMD;
			break;
	}
	
err_out:
	return retval;	
}


static struct file_operations ami602mid_fops = {
	.owner = THIS_MODULE,
	.open = ami602mid_open,
	.release = ami602mid_release,
	.ioctl = ami602mid_ioctl,
};

static struct miscdevice ami602mid_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ami602mid",
	.fops = &ami602mid_fops,
};

static int ami602_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int ami602_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int ami602_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	char strbuf[AMI602_BUFSIZE];
	void __user *data;
	int retval=0;
		
	switch (cmd) {
		case AMI602_IOCTL_INIT:
			AMI602_Init();
			break;
			
		case AMI602_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI602_ReadChipInfo(strbuf, AMI602_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;	
			
		case AMI602_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;
			if(mode == AMI602_HOST_ON) /*eycho debugging*/
				AMI602_ReadSensorData_HostMode(strbuf, AMI602_BUFSIZE);

				AMI602_ReadSensorData(strbuf, AMI602_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;		
						
		case AMI602_IOCTL_READ_POSTUREDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI602_ReadPostureData(strbuf, AMI602_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
			break;			
	 
	        case AMI602_IOCTL_READ_CALIDATA:
			data = (void __user *) arg;
			if (data == NULL)
				break;	
			AMI602_ReadCaliData(strbuf, AMI602_BUFSIZE);
			if (copy_to_user(data, strbuf, strlen(strbuf)+1)) {
				retval = -EFAULT;
				goto err_out;
			}				
	        	break;
	        
		default:
			printk(KERN_ERR "%s not supported = 0x%04x", __FUNCTION__, cmd);
			retval = -ENOIOCTLCMD;
			break;
	}
	
err_out:
	return retval;	
}

static struct file_operations ami602_fops = {
	.owner = THIS_MODULE,
	.open = ami602_open,
	.release = ami602_release,
	.ioctl = ami602_ioctl,
};

static struct miscdevice ami602_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ami602",
	.fops = &ami602_fops,
};

/*
 * interrupt service routine
 */
 
void ami602_read_measure(struct work_struct *work)
{
	AMI602_ReadSensorDataFromChip();
}

/* LGE_CHANGE_S 2009.09.27 [eycho1004@lge.com]- Add GED 6-axis Sensor */
#if 1	
/* AMI602 BUSY rising isr */
static irqreturn_t ami602_busy_rising_isr(int irq, void *dev_id)
{
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;
	
	if(mode == AMI602_INIT)
	{
		busy_rising= gpio_get_value(gyro_pdata->pin_busy);
		//AMID("busy_rising = %d\n", busy_rising);
		wake_up_interruptible(&wq_busy_rising);
	}
	else
		schedule_work(&ami602_readmeasure_work);
	
	return IRQ_HANDLED;
}

static int __init ami602_probe(struct i2c_client *client, const struct i2c_device_id * devid)
{
	int ret = 0;
	struct gyro_platform_data* gyro_pdata;
	
	AMID("motion start....!\n");
	
	ami602_i2c_client = client;
	
	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		AMIE("adapter can NOT support I2C_FUNC_I2C.\n");
		return -ENODEV;
	}
	
	INIT_WORK(&ami602_readmeasure_work, ami602_read_measure);

	gyro_pdata = ami602_i2c_client->dev.platform_data;
	
	ret = request_irq(gpio_to_irq(gyro_pdata->pin_busy), ami602_busy_rising_isr,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"ami602_sensor", &client->dev);
	if (ret < 0) {
		AMIE("Failed to request irq for AMI602_BUSY (GPIO_%d).\n", gyro_pdata->pin_busy);
		goto exit_irq_request_failed;
	}
	
	AMI602_Power_On();
	
if(mode == AMI602_SENSOR_ON)
	AMI602_Init_SensorTrigger();
else
	AMI602_Init();
	
	
#if defined(CONFIG_HAS_EARLYSUSPEND)
	ami602_sensor_early_suspend.suspend = ami602_early_suspend;
	ami602_sensor_early_suspend.resume = ami602_late_resume;
	register_early_suspend(&ami602_sensor_early_suspend);
#endif
 
	/* Misc device register */
	ret = misc_register(&ami602_device);
	if (ret) {
		AMIE("ami602_device register failed\n");
		goto exit_misc_device_register_failed;
	}
	/* Misc device register */
	ret = misc_register(&ami602mid_device);
	if (ret) {
		AMIE("ami602_device register failed\n");
		goto exit_misc_device_register_failed;
	}
	
	ret = device_create_file(&client->dev, &dev_attr_chipinfo);
	if (ret) {
		AMIE("chipinfo: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	
	ret = device_create_file(&client->dev, &dev_attr_sensordata);
	if (ret) {
		AMIE("sensordata: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	
	ret = device_create_file(&client->dev, &dev_attr_posturedata);
	if (ret) {
		AMIE("onoff: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	ret = device_create_file(&client->dev, &dev_attr_calidata);
	if (ret) {
		AMIE("onoff: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	ret = device_create_file(&client->dev, &dev_attr_midcontrol);
	if (ret) {
		AMIE("onoff: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	
	ret = device_create_file(&client->dev, &dev_attr_mode);
	if (ret) {
		AMIE("onoff: ami602_device register failed\n");
		goto exit_device_create_failed;
	}
	
	return 0;

exit_device_create_failed:
	device_remove_file(&client->dev, &dev_attr_chipinfo);
	device_remove_file(&client->dev, &dev_attr_sensordata);
	device_remove_file(&client->dev, &dev_attr_posturedata);
	device_remove_file(&client->dev, &dev_attr_calidata);
	device_remove_file(&client->dev, &dev_attr_midcontrol);
	device_remove_file(&client->dev, &dev_attr_mode);
exit_misc_device_register_failed:
	misc_deregister(&ami602_device);
	misc_deregister(&ami602mid_device);
exit_irq_request_failed:
	free_irq(gpio_to_irq(gyro_pdata->pin_busy), &client->dev);

	return ret;
}

static int ami602_remove(struct	i2c_client *client)
{
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;
	
	device_remove_file(&client->dev, &dev_attr_chipinfo);
	device_remove_file(&client->dev, &dev_attr_sensordata);
	device_remove_file(&client->dev, &dev_attr_posturedata);
	device_remove_file(&client->dev, &dev_attr_calidata);
	device_remove_file(&client->dev, &dev_attr_midcontrol);
	device_remove_file(&client->dev, &dev_attr_mode);
	misc_deregister(&ami602_device);
	misc_deregister(&ami602mid_device);
	free_irq(gpio_to_irq(gyro_pdata->pin_busy), &client->dev);
	gpio_free(gyro_pdata->pin_trg);
	gpio_free(gyro_pdata->pin_busy);
	gpio_free(gyro_pdata->pin_rst);
	
#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&ami602_sensor_early_suspend);
#endif

		return 0;
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void ami602_early_suspend(struct early_suspend *h)
{
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;

#if 0
	if(mode == AMI602_HOST_ON)
		AMI602_SetMode(AMI602_HOST_OFF);
	else
		AMI602_SetMode(AMI602_SENSOR_OFF);
#endif
	gyro_pdata->power(0);
}

static void ami602_late_resume(struct early_suspend *h)
{
	struct gyro_platform_data* gyro_pdata;
	gyro_pdata = ami602_i2c_client->dev.platform_data;
	
#if 0
	if(mode == AMI602_HOST_OFF)
		AMI602_SetMode(AMI602_HOST_ON);
	else
		AMI602_SetMode(AMI602_SENSOR_ON);
#endif	
	gyro_pdata->power(1);
}
#endif


static const struct i2c_device_id motion_ids[] = {
		{ "ami602_sensor", 0 },
		{ },
};

MODULE_DEVICE_TABLE(i2c, motion_ids);

static struct i2c_driver ami602_i2c_driver = {
		.probe		= ami602_probe,
		.remove		= ami602_remove,
		.id_table	= motion_ids,
		.driver		= {
				.name	=	"ami602_sensor",
				.owner	=	THIS_MODULE,
		},
		.id	        = 1601,
};
/* LGE_CHANGE_E 2009.09.27 [eycho1004@lge.com]  */

#else	/* AICHI STEEL Orginal Source Code */
static int ami602_interrupt(int irq, void *dev_id)
{
	schedule_work(&ami602_readmeasure_work);
	return IRQ_HANDLED;
}

static int ami602_i2c_attach_adapter(struct i2c_adapter *adapter)
{
	int res = 0;
	printk(KERN_INFO "\n\nEnter ami602_i2c_attach_adapter!!\n");
	//int res = i2c_detect(adapter, &addr_data, ami602_i2c_detect);
	res = i2c_probe(adapter, &addr_data, ami602_i2c_detect);	
	printk(KERN_INFO "      res of ami602_i2c_attach_adapter= %d\n", res);
	return res;
}
  

static int ami602_i2c_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	struct ami602_i2c_data *data;
	int err = 0;

	printk(KERN_INFO "\n\nEnter ami602_i2c_detect!!\n");	
	if (!(data = kmalloc(sizeof(struct ami602_i2c_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}
	memset(data, 0, sizeof(struct ami602_i2c_data));

	new_client = &(data->client);
	i2c_set_clientdata(new_client, data);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &ami602_i2c_driver;
	new_client->flags = 0;

	strlcpy(new_client->name, "ami602_i2c", I2C_NAME_SIZE);
	ami602_i2c_client = new_client;
		
	if ((err = i2c_attach_client(new_client)))
		goto exit_kfree;
	
	INIT_WORK(&ami602_readmeasure_work, ami602_read_measure);
	// Configuration for S3C6410 GPIO and interrupt pins
	s3c_gpio_cfgpin(S3C_GPK0, S3C_GPK0_OUTP);
	s3c_gpio_cfgpin(S3C_GPN9, S3C_GPN9_EXTINT9);
	s3c_irqext_type(AMI602_IRQ, IRQT_RISING);   // Configure external interrupt EINT9 signal type; for S3C6410
    //s3c2410_gpio_cfgpin(AMI602_TRIG, S3C2410_GPIO_OUTPUT);	//For S3C2410

    err = request_irq(AMI602_IRQ, ami602_interrupt,  0, "ami602", &new_client->dev);    	
	if (err < 0) {
		printk(KERN_ERR "ami602 request irq error !!\n");
	}		

	
	AMI602_Init_SensorTrigger();	 // Initial sensor with sensor trigger mode
	//AMI602_Init();                 // Initial sensor with host trigger mode	
	printk(KERN_INFO "AMI602 registered I2C driver!\n");


	err = misc_register(&ami602_device);
	if (err) {
		printk(KERN_ERR
		       "ami602_device register failed\n");
		goto exit_misc_device_register_failed;
	}
		
	err = device_create_file(&new_client->dev, &dev_attr_chipinfo);
	err = device_create_file(&new_client->dev, &dev_attr_sensordata);
	err = device_create_file(&new_client->dev, &dev_attr_posturedata);
	err = device_create_file(&new_client->dev, &dev_attr_calidata);
	err = device_create_file(&new_client->dev, &dev_attr_midcontrol);
	err = device_create_file(&new_client->dev, &dev_attr_mode);
	
	err = misc_register(&ami602mid_device);
	if (err) {
		printk(KERN_ERR
		       "ami602mid_device register failed\n");
		goto exit_misc_device_register_failed;
	}	
		
	return 0;

exit_misc_device_register_failed:
exit_kfree:
	kfree(data);
exit:
	return err;
}


static int ami602_i2c_detach_client(struct i2c_client *client)
{
	int err;

	free_irq(AMI602_IRQ, &ami602_i2c_client->dev);	
	if ((err = i2c_detach_client(client)))
		return err;

	ami602_i2c_client = NULL;
	kfree(i2c_get_clientdata(client));
	misc_deregister(&ami602mid_device);
	misc_deregister(&ami602_device);
	return 0;
}

#endif

static int __init ami602_init(void)
{
#if 1 /*LGE_CHANGE 2009.09.27 [eycho1004@lge.com] - Add ERROR condition*/
	int ret;
	ami602mid_data.controldata[0] = 50000;
	ami602mid_data.controldata[1] = 0;
	
	ret = i2c_add_driver(&ami602_i2c_driver);
	if (ret) {
		AMIE("failed to probe i2c \n");
		i2c_del_driver(&ami602_i2c_driver);
	}
	return ret;
#else /* AICHI STEEL Orginal Source Code */
	printk(KERN_INFO "AMI602 MI sensor driver: init\n");
	rwlock_init(&ami602mid_data.ctrllock);
	rwlock_init(&ami602mid_data.datalock);
	rwlock_init(&ami602_data.lock);
	ami602mid_data.controldata[0] = 50000;
	ami602mid_data.controldata[1] = 0;
	return i2c_add_driver(&ami602_i2c_driver);;
#endif
}

static void __exit ami602_exit(void)
{
	i2c_del_driver(&ami602_i2c_driver);
}

module_init(ami602_init);
module_exit(ami602_exit);

MODULE_AUTHOR("Kyle K.Y. Chen");
MODULE_DESCRIPTION("AMI602 MI sensor driver");
MODULE_LICENSE("GPL");
