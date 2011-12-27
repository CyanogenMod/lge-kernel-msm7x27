#include "synaptics_abstractionlayer.h"
#include <linux/delay.h>
#include <mach/gpio.h>

struct i2c_client *syn_touch_client;

#define SYNAPTICS_ATTN_GPIO	92
#define ATTN_TIMEOUT 5000

void SynaPrintMsg(char *msg)
{
	printk(KERN_ERR "synaptics_ts_fw_angent : %s\n",msg);
	return;
}

void SynaExit()
{
	return;
}

void SynaSleep(unsigned int MilliSeconds)
{
	msleep(MilliSeconds);
	//mdelay(MilliSeconds);
}

int SynaWaitForATTN(unsigned long MilliSeconds)
{
	int trialTime=0;
	while((gpio_get_value(SYNAPTICS_ATTN_GPIO)!=0))
	{
		if(trialTime < MilliSeconds)
			trialTime++;
		else
		{
			printk(KERN_INFO "SynaWaitForATTN time over!!\n");
			break;
		}

		SynaSleep(1);
	}

	if(gpio_get_value(SYNAPTICS_ATTN_GPIO)==0)
	{
		return ESuccess;
	}

	return EErrorTimeout;
}

int SynaWriteRegister(unsigned short   uRmiAddress, unsigned char  *data, unsigned int     length)
{
	s32 smbus_ret;
	u16 data_word;

	if(uRmiAddress>0xFF)
		return EErrorSMBAddress;

	if(length>0xFF)
		return EErrorSMBlength;

	switch(length)
	{
		case 1:
			smbus_ret=i2c_smbus_write_byte_data(syn_touch_client,(u8)uRmiAddress,*data);
			break;
		case 2:
			data_word=data[1]*0x100+data[0];
			smbus_ret=i2c_smbus_write_word_data(syn_touch_client,(u8)uRmiAddress,data_word);
			break;
		default:
			smbus_ret=i2c_smbus_write_i2c_block_data(syn_touch_client,(u8)uRmiAddress,(u8)length, data);
			break;
	}

	if(smbus_ret>=0)  //if smbus success
	{
		return ESuccess;
	}

	printk("smbbus write error : err_code:%d\n",smbus_ret);
	return smbus_ret;
}

int SynaReadRegister(unsigned short  uRmiAddress, unsigned char * data, unsigned int    length)
{
	s32 smbus_ret;
	struct i2c_msg msg[2];
	uint8_t start_reg;

	if(uRmiAddress>0xFF)
		return EErrorSMBAddress;

	if(length>0xFF)
		return EErrorSMBlength;

	if(length==1)
	{
		smbus_ret=i2c_smbus_read_byte_data(syn_touch_client,(u8)uRmiAddress);
		*data=(u8)smbus_ret;
	}
	else
	{
		msg[0].addr = syn_touch_client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &start_reg;
		start_reg = (u8)uRmiAddress;
		msg[1].addr = syn_touch_client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = length;
		msg[1].buf = data;

		smbus_ret = i2c_transfer(syn_touch_client->adapter, msg, 2);
	}

	if(smbus_ret >= 0)  //if smbus success
		return ESuccess;

	printk("smbbus read error  error number:%d\n",smbus_ret);
	return smbus_ret;
}

void SynaI2CClientInit(struct i2c_client *syn_touch)
{
	syn_touch_client = syn_touch;
	printk(KERN_INFO "SynaI2CClientInit = 0x%x\n", syn_touch->addr);
}
