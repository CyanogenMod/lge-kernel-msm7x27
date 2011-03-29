/*
 * SYNAPTICS 2000 series touchscreen firmware image download
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "syn_fw.h"
#include "syn_reflash.h"

#include <linux/delay.h>

/* local structs, vars and defines */
struct RMI4FunctionDescriptor
{
  unsigned char QueryBase;
  unsigned char CommandBase;
  unsigned char ControlBase;
  unsigned char DataBase;
  unsigned char IntSourceCount;
  unsigned char ID;
};
struct RMI4FunctionDescriptor SynaPdtF34Flash;
struct RMI4FunctionDescriptor SynaPdtF01Common;

/* vars used for flashing image and configuration */
unsigned short SynaBootloadID;
unsigned short SynabootloadImgID;
unsigned char  SynafirmwareImgVersion;
unsigned char *SynafirmwareImgData;
unsigned char *SynaconfigImgData;
unsigned short SynafirmwareBlockSize;
unsigned short SynafirmwareBlockCount;
unsigned short SynaconfigBlockSize;
unsigned short SynaconfigBlockCount;
unsigned char  SynauPageData[0x200];
unsigned long  SynafirmwareImgSize;
unsigned long  SynaconfigImgSize;
unsigned long  SynafileSize;

/* Stored register addresses used for image flashing and configuration */
unsigned short SynauF01RMI_DataBase;
unsigned short SynauF34Reflash_BlockNum;
unsigned short SynauF34Reflash_BlockData;
unsigned short SynauF34Reflash_FlashControl;
unsigned short SynauF34ReflashQuery_BootID;
unsigned short SynauF34ReflashQuery_FirmwareBlockSize;
unsigned short SynauF34ReflashQuery_FirmwareBlockCount;
unsigned short SynauF34ReflashQuery_ConfigBlockSize;
unsigned short SynauF34ReflashQuery_ConfigBlockCount;
unsigned short SynauF34ReflashQuery_FlashPropertyQuery;

static const unsigned char s_uF34ReflashCmd_NormalResult  = 0x80; //Read the Flash control register. The result should be

extern int is_need_forced_update;
extern int is_fw_reflash;
/* Functions...*/
/*
 * Functions to read the register format and determine how to initialize the
 * flash format register addresses which vary from one format to another.
 */
#include <linux/delay.h>
#include <mach/gpio.h>

struct i2c_client *syn_touch_client;

#define SYNAPTICS_ATTN_GPIO	92

/* Special defined error codes used by Synaptics - the OEM should not define any error codes the same. */

int wait_attn(unsigned long MilliSeconds)
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

		msleep(1);
	}

	if(gpio_get_value(SYNAPTICS_ATTN_GPIO)==0)
	{
		return 0;
	}

	return -1;
}

int write_reg(unsigned short   uRmiAddress, unsigned char  *data, unsigned int     length)
{
	s32 smbus_ret;
	u16 data_word;

	if(uRmiAddress>0xFF) {
		printk("Error SMB Address\n");
		return -1;
	}

	if(length>0xFF) {
		printk("Error SMB Length\n");
		return -1;
	}

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
		return 0;
	}

	printk("smbbus write error : err_code:%d\n",smbus_ret);
	return smbus_ret;
}

int read_reg(unsigned short  uRmiAddress, unsigned char * data, unsigned int length)
{
	s32 smbus_ret;
	struct i2c_msg msg[2];
	uint8_t start_reg;

	if(uRmiAddress>0xFF) {
		printk("Error SMB Address\n");
		return -1;
	}

	if(length>0xFF) {
		printk("Error SMB Length\n");
		return -1;
	}

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
		return 0;

	printk("smbbus read error  error number:%d\n",smbus_ret);
	return smbus_ret;
}

/* This function gets the firmware block size, block count and image size */
int firmware_info()
{
  unsigned char uData[2];
  int ret;

  ret = read_reg(SynauF34ReflashQuery_FirmwareBlockSize, &uData[0], 2);
  if (ret != 0)
	  return -1;
  SynafirmwareBlockSize = uData[0] | (uData[1] << 8);

  ret = read_reg(SynauF34ReflashQuery_FirmwareBlockCount, &uData[0], 2);
  if (ret != 0)
	  return -1;
  SynafirmwareBlockCount = uData[0] | (uData[1] << 8);

  SynafirmwareImgSize = SynafirmwareBlockCount*SynafirmwareBlockSize;
  printk(KERN_INFO "firmwareImgSize=0x%lx \n",SynafirmwareImgSize);

  return 0;
}

/* This function gets config block count, config block size and image size */
int config_info()
{
  unsigned char uData[2];
  int ret;

  ret = read_reg(SynauF34ReflashQuery_ConfigBlockSize, &uData[0], 2);
  SynaconfigBlockSize = uData[0] | (uData[1] << 8);

  ret = read_reg(SynauF34ReflashQuery_ConfigBlockCount, &uData[0], 2);
  SynaconfigBlockCount = uData[0] | (uData[1] << 8);

  SynaconfigImgSize = SynaconfigBlockSize*SynaconfigBlockCount;
  printk(KERN_INFO "configImgSize=0x%lx \n",SynaconfigImgSize);

  return ret;
}

int set_flash_addr()
{
  unsigned int RegFormat;
  int ret;

  ret = read_reg(SynauF34ReflashQuery_FlashPropertyQuery, &SynauPageData[0], 1);
  RegFormat = ((unsigned int)((SynauPageData[0] & 0x01) == 0x01));

  if (RegFormat)
  {
    SynauF34Reflash_FlashControl = SynaPdtF34Flash.DataBase + SynafirmwareBlockSize + 2;
    SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase;
    SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 2;
  }
  else
  {
    SynauF34Reflash_FlashControl = SynaPdtF34Flash.DataBase;
    SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase + 1;
    SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 3;
  }
  return ret;
}

/* This function reads the Page Descriptor Table and assigns the function 0x01 and
 * function 0x34 addresses for data, control, query, etc.                          */
int read_description()
{
	struct RMI4FunctionDescriptor Buffer;
	unsigned short uAddress;
	int ret;

	SynaPdtF01Common.ID = 0;
	SynaPdtF34Flash.ID = 0;

	msleep(20);

	for (uAddress = 0xe9; uAddress > 0xbf; uAddress -= sizeof(struct RMI4FunctionDescriptor))
	{
		ret = read_reg(uAddress, (unsigned char*)&Buffer, sizeof(Buffer));

		if(Buffer.ID == 0)
			break;

		printk(KERN_INFO "read_description - Buffer.ID = 0x%x\n",Buffer.ID);

		switch(Buffer.ID)
		{
			case 0x34:
				SynaPdtF34Flash = Buffer;
				break;

			case 0x01:
				SynaPdtF01Common = Buffer;
        	break;

			default:
				break;
		}
	}
	/* Initialize function address bases for function 0x01 and function 0x34. */
	SynauF01RMI_DataBase = SynaPdtF01Common.DataBase;

	SynauF34Reflash_BlockNum = SynaPdtF34Flash.DataBase;
	SynauF34Reflash_BlockData = SynaPdtF34Flash.DataBase + 2;

	SynauF34ReflashQuery_BootID = SynaPdtF34Flash.QueryBase;
	SynauF34ReflashQuery_FlashPropertyQuery = SynaPdtF34Flash.QueryBase + 2;
	SynauF34ReflashQuery_FirmwareBlockSize = SynaPdtF34Flash.QueryBase + 3;
	SynauF34ReflashQuery_FirmwareBlockCount = SynaPdtF34Flash.QueryBase + 5;
  	SynauF34ReflashQuery_ConfigBlockSize = SynaPdtF34Flash.QueryBase + 3;
	SynauF34ReflashQuery_ConfigBlockCount = SynaPdtF34Flash.QueryBase + 7;

	return ret;
}

// Wait for ATTN assertion and see if it's idle and flash enabled
int RMI4WaitATTN(void)
{
  int errorCount = 300;
  int uErrorCount = 0;
  int ret;
  unsigned char status;

  // To work around the physical address error from Control Bridge
  wait_attn(1000);

  do {
	  ret = read_reg(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    // To work around the physical address error from control bridge
    // The default check count value is 3. But the value is larger for erase condition
    if((ret != 0) && uErrorCount < errorCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    // Clear the attention assertion by reading the interrupt status register
    ret = read_reg(SynaPdtF01Common.DataBase + 1, &status, 1);

  } while( SynauPageData[0] != s_uF34ReflashCmd_NormalResult && uErrorCount < errorCount);

  return ret;
}


int initialize(struct i2c_client *syn_touch)
{
	unsigned char uPage = 0x00;
	unsigned char uF01_RMI_Data[2];
	int ret;
	unsigned char status;

	syn_touch_client = syn_touch;

	/* Setup to read and write from page 0 */
	ret = write_reg(0xff, &uPage, 1);

	do
	{
		read_reg(0, &status, 1);

		if(status & 0x80)
		{
			/* unconfigured */
			break;
		}
	} while(status & 0x40);

	/* Read in the PDT to get addresses for functions 0x01 and 0x34 */
	ret = read_description();
	if(ret != 0)
		return -1;

	/* Read block size and block count for config and firmware image */
	ret = config_info();
	if(ret != 0)
		return -1;

  	ret = firmware_info();
	if(ret != 0)
		return -1;

	ret = set_flash_addr();
	if(ret != 0)
		return -1;

	/* Check that we got them OK - it's a fatal error if not */
	if(SynaPdtF34Flash.ID == 0)
	{
		printk("Function $34 is not supported\n");
	}

	if(SynaPdtF01Common.ID == 0)
	{
		printk("Function $01 is not supported\n");
		SynaPdtF01Common.ID = 0x01;
    	SynaPdtF01Common.DataBase = 0;
		return -1;
	}

	// Get device status
	ret = read_reg(SynaPdtF01Common.DataBase, &uF01_RMI_Data[0], sizeof(uF01_RMI_Data));
	if (ret != 0)
		return -1;
	// Check Device Status
	printk(KERN_INFO "Configured: %s\n", uF01_RMI_Data[0] & 0x80 ? "false" : "true");
	printk(KERN_INFO "FlashProg:  %s\n", uF01_RMI_Data[0] & 0x40 ? "true" : "false");
	printk(KERN_INFO "StatusCode: 0x%x \n", uF01_RMI_Data[0] & 0x0f );

	/* Initialize working variables */
	SynafirmwareImgData = 0;
	SynaconfigImgData = 0;

	return 0;
}

/* Enable Flash programming */
int enable_flashing()
{
  int count = 0;
  unsigned char uData[2];
  int ret;
  unsigned char status;

  do {
    ret = read_reg(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);
    /* To deal with an error when device is busy and not available for read */
    if((ret != 0) && count < 300)
    {
      count++;
      SynauPageData[0] = 0;
      continue;
    }


    /* Clear the attention assertion by reading the interrupt status register */
    ret = read_reg((unsigned short)(SynaPdtF01Common.DataBase + 1), &status, 1);
  } while(((SynauPageData[0] & 0x0f) != 0x00) && (count <= 300));

  /* Issue Enable flash command */
  uData[0] = 0x0f;
  ret = write_reg(SynauF34Reflash_FlashControl, &uData[0], 1);
  if(ret != 0)
	  return -1;

  /* Read the page descriptor table */
  ret = read_description();
	if(ret != 0)
		return -1;

  // Wait for ATTN and check if flash command state is idle
  RMI4WaitATTN();

  ret = read_reg(SynauF34Reflash_FlashControl, &uData[0], 1);
  if(ret != 0)
	  return -1;

  if ( uData[0] != 0x80 )
  {
	  printk("\nFlash failed\n");
	  return -1;
  }
  return 0;
}

void SynaCalculateChecksum(unsigned short * data, unsigned short len, unsigned long * dataBlock)
{
  unsigned long temp;
  unsigned long sum1 = 0xffffffff & 0xFFFF;
  unsigned long sum2 = 0xffffffff >> 16;

  *dataBlock = 0xffffffff;

  while (len--)
  {
    temp = *data++;
    sum1 += temp;
    sum2 += sum1;
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }

  *dataBlock = sum2 << 16 | sum1;
}

/*
 * Read the firmware header (SynaFirmwareImage.h) data and store the image and config info into
 * a local array and assign local variables to be used when burning the image and reflashing
 * the device configuration.
 */
unsigned long ExtractLongFromHeader(const unsigned char* SynaImage)  // Endian agnostic
{
  return((unsigned long)SynaImage[0] +
         (unsigned long)SynaImage[1]*0x100 +
         (unsigned long)SynaImage[2]*0x10000 +
         (unsigned long)SynaImage[3]*0x1000000);
}

unsigned long SynaGetConfigSize(void)
{
	return (unsigned long)SynaconfigBlockSize*(unsigned long)SynaconfigBlockCount;
}

unsigned long SynaGetFirmwareSize(void)
{
	return (unsigned long)SynafirmwareBlockSize*(unsigned long)SynafirmwareBlockCount;
}

int read_firmware_header(void)
{
  unsigned long firmwareImgFileCheckSum = 0;
  unsigned long checkSumCode = 0;
  int ret;

  SynafileSize = sizeof(SynaFirmware) - 1;

  checkSumCode            = ExtractLongFromHeader(&(SynaFirmware[0]));
  SynabootloadImgID       = (unsigned int)SynaFirmware[4] + (unsigned int)SynaFirmware[5]*0x100;
  SynafirmwareImgVersion  = SynaFirmware[7];
  SynafirmwareImgSize     = ExtractLongFromHeader(&(SynaFirmware[8]));
  SynaconfigImgSize       = ExtractLongFromHeader(&(SynaFirmware[12]));

  SynaCalculateChecksum((unsigned short*)(&SynaFirmware[4]), (unsigned short)((SynafileSize - 4) >> 1),
                        (unsigned long *)(&firmwareImgFileCheckSum));

  if (firmwareImgFileCheckSum != checkSumCode)
  {
    printk("\nError: SynaFirmwareImage invalid checksum.\n");
    return -1;
  }

  if (SynafileSize != (0x100+SynafirmwareImgSize+SynaconfigImgSize))
  {
    printk("Error: SynaFirmwareImage actual size not expected size.\n");
    return -1;
  }

  if (SynafirmwareImgSize != SynaGetFirmwareSize())
  {
    printk("\nFirmware image size verfication failed.\n");
    return -1;
  }

  if (SynaconfigImgSize != SynaGetConfigSize())
  {
    printk("Configuration size verfication failed.\n");
    return -1;
  }

  SynafirmwareImgData = (unsigned char *)((&SynaFirmware[0])+0x100);
  SynaconfigImgData   = (unsigned char *)((&SynaFirmware[0])+0x100+SynafirmwareImgSize);

  ret = read_reg(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);
  if (ret != 0)
	  return -1;
  return 0;
}

/* Wait for ATTN assertion and see if the status is idle and we're ready to continue */
void SynaWaitATTN(int errorCount)
{
  int uErrorCount = 0;
  int ret;
  unsigned char status;

  if (wait_attn(300))
  {
    printk(KERN_ERR "synaptics_ts_fw_angent - SynaWaitATTN Time out Error\n");
  }

  /* after we get the ATTN line signaled we need to loop and read the interrupt status register
   * until we see the status indicating the sensor is ready to continue. This can take a while
   * depending on the command that was sent. Reading the interrups status register will clear
   * the attention line. */
  do {
    ret = read_reg(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    /* To work around the physical address error. */

    if(ret && uErrorCount < errorCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    /* Clear the attention assertion by reading the interrupt status register */
    read_reg((unsigned short)(SynaPdtF01Common.DataBase + 1), &status, 1);
  } while( SynauPageData[0] != 0x80 && uErrorCount < errorCount);
}

/**************************************************************
 This function writes firmware to device one block at a time
 **************************************************************/
int SynaFlashFirmwareWrite()
{
  unsigned char *puFirmwareData = SynafirmwareImgData;
  unsigned char uData[2];
  /* Move to next data block */
  puFirmwareData += SynafirmwareBlockSize;

  /* Issue Write Firmware Block command */
  uData[0] = 2;
  write_reg(SynauF34Reflash_FlashControl, &uData[0], 1);

  /* Wait ATTN until device is done writing the block and is ready for the next. */
  RMI4WaitATTN();

  return 0;
}

int program_firmware()
{
  unsigned char uData[1];
  int ret;

  /* Issue the firmware erase command */
  uData[0] = 3;
  ret = write_reg(SynauF34Reflash_FlashControl, &uData[0], 1);
  if (ret != 0)
	  return -1;

  RMI4WaitATTN();

  /* Write firmware image */
  SynaFlashFirmwareWrite();
  return 0;
}

/**************************************************************
 This function writes config data to device one block at a time
 **************************************************************/
void SynaProgramConfiguration(void)
{
  unsigned char uData[2];
  unsigned char *puData = SynaconfigImgData;
  unsigned short blockNum;

  for (blockNum = 0; blockNum < SynaconfigBlockCount; blockNum++)
  {
	uData[0] = blockNum&0xff;
	uData[1] = (blockNum&0xff00) >> 8;

    /* Write Configuration Block Number */
    write_reg(SynauF34Reflash_BlockNum, &uData[0], 2);

    /* Write Data Block */
    write_reg(SynauF34Reflash_BlockData, puData, SynaconfigBlockSize);

    puData += SynaconfigBlockSize;

    /* Issue Write Configuration Block command to flash command register */
    uData[0] = 0x06;
    write_reg(SynauF34Reflash_FlashControl, &uData[0], 1);

    // Wait for ATTN
    RMI4WaitATTN();
  }
}

int SynaFinalizeFlash()
{
  unsigned char uData[2];
  unsigned int uErrorCount = 0;
  unsigned int i=0;
  int ret;
  unsigned char status;

  /* Reset the sensor by issuing a reset command = 1 */
  uData[0] = 1;
  ret = write_reg(SynaPdtF01Common.CommandBase, &uData[0], 1);
  if (ret != 0)
	  printk("Failed to write register\n");
  /* wait up to 200 milliseconds for sensor to come back from a reset */
  msleep(200);

  /* Wait for ATTN to be asserted to see if device is in idle state */
  if (wait_attn(300)!= 0)
  {
	  printk("Wait_ATTN ; TimeOut\n");
  }

  do {
    ret = read_reg(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    /* To work around an error from device if device not ready */
    if((ret!= 0) && uErrorCount < 300)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

  } while(((SynauPageData[0] & 0x0f) != 0x00) && (uErrorCount < 300));
  printk("Over Default Error Retry Count\n");

  /* Clear the attention assertion by reading the interrupt status register */
  ret = read_reg((unsigned short)(SynaPdtF01Common.DataBase + 1), &status, 1);
  if (ret != 0)
	  printk("Failed to read register\n");

  /* Read F01 Flash Program Status, ensure the 6th bit is '0' */
  uData[0] = 0xFF;

  for(i=500; i!=0; i--)
  {
    msleep(1);
    ret = read_reg(SynauF01RMI_DataBase, &uData[0], 1);
	if (ret != 0)
		printk("Failed to read register\n");

    if((uData[0] & 0x40) == 0)
    {
		break;
	}
  }

  if(!i)
  {
	if(ret != 0)
		printk(KERN_INFO "synaptics_ts_fw_angent : SynaFinalizeFlash - Error = 0x%x\n",ret);
	else
		printk("time is not enough!!\n");
  }

/* With a new flash image the page description table could change so re-read it. */
  read_description();

  return 0;
}

/*
 *  Performs all the steps needed to reflash the image.
 */
unsigned int firmware_reflash(struct i2c_client *syn_touch, int fw_revision)
{
	int ret;
  if((fw_revision == SynaFirmware[0x1F]) && (is_need_forced_update == 0))
  {
    printk(KERN_INFO "synaptics_ts_fw_angent : F/W version is up-to-date\n");
  	return -1;
  }

  is_fw_reflash = 1;

  printk(KERN_INFO "synaptics_ts_fw_angent : F/W Upgrade!! - from 0x%x to 0x%x\n",fw_revision,SynaFirmware[0x1F]);

  //Start reflash
  if(initialize(syn_touch) != 0)
  {
    printk(KERN_INFO "ts_fw_agent : Initialize failed!!\n");
	return -1;
  }

  /* Enable flash mode */
  if (enable_flashing() !=0)
	  return -1;

  /* Read the firmware header info from the byte array defined in SynaFirmwareImage.h */
  if (read_firmware_header() != 0)
	  return -1;

  /* Program the firmware image */
  if (program_firmware() != 0)
	  return -1;

  /* Program the new configuration */
  SynaProgramConfiguration();

  /* Reset the device and do checks to finalize reflashing */
  if(SynaFinalizeFlash()!= 0)
  {
    return -1;
  }

  /* return the globally set status */
  return ret;
}
