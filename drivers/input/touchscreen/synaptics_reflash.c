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

#include "synaptics_abstractionlayer.h"
#include "synaptics_firmwareimage.h"
#include "synaptics_reflash.h"

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
int   SynaRet;
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
unsigned char  SynauStatus;
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
unsigned int SynaIsExpectedRegFormat()
{
  /* Flash Properties query 1: registration map format version 1
   *                        0: registration map format version 0
   */
  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FlashPropertyQuery, &SynauPageData[0], 1);
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaIsExpectedRegFormat- SynauF34ReflashQuery_FlashPropertyQuery = 0x%x\n",SynauPageData[0]);
  return ((unsigned int)((SynauPageData[0] & 0x01) == 0x01));
}

/* This function gets the firmware block size, block count and image size */
void SynaReadFirmwareInfo()
{
  unsigned char uData[2];

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FirmwareBlockSize, &uData[0], 2);
  SynafirmwareBlockSize = uData[0] | (uData[1] << 8);

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_FirmwareBlockCount, &uData[0], 2);
  SynafirmwareBlockCount = uData[0] | (uData[1] << 8);

  SynafirmwareImgSize = SynafirmwareBlockCount*SynafirmwareBlockSize;
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadFirmwareInfo -SynafirmwareBlockSize=0x%x \n",SynafirmwareBlockSize);
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadFirmwareInfo -SynafirmwareBlockCount=0x%x \n",SynafirmwareBlockCount);
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadFirmwareInfo -SynafirmwareImgSize=0x%lx \n",SynafirmwareImgSize);
}

/* This function gets config block count, config block size and image size */
void SynaReadConfigInfo()
{
  unsigned char uData[2];

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_ConfigBlockSize, &uData[0], 2);
  SynaconfigBlockSize = uData[0] | (uData[1] << 8);

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_ConfigBlockCount, &uData[0], 2);
  SynaconfigBlockCount = uData[0] | (uData[1] << 8);

  SynaconfigImgSize = SynaconfigBlockSize*SynaconfigBlockCount;
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadConfigInfo -SynaconfigBlockSize=0x%x \n",SynaconfigBlockSize);
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadConfigInfo -SynaconfigBlockCount=0x%x \n",SynaconfigBlockCount);
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadConfigInfo -SynaconfigImgSize=0x%lx \n",SynaconfigImgSize);
}

void SynaSetFlashAddrForDifFormat()
{
  if (SynaIsExpectedRegFormat())
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
}

/* This function reads the Page Descriptor Table and assigns the function 0x01 and
 * function 0x34 addresses for data, control, query, etc.                          */
void SynaReadPageDescriptionTable()
{
	struct RMI4FunctionDescriptor Buffer;
	unsigned short uAddress;

	SynaPdtF01Common.ID = 0;
	SynaPdtF34Flash.ID = 0;

	SynaSleep(20);

	for (uAddress = 0xe9; uAddress > 0xbf; uAddress -= sizeof(struct RMI4FunctionDescriptor))
	{
		SynaRet = SynaReadRegister(uAddress, (unsigned char*)&Buffer, sizeof(Buffer));

		if(Buffer.ID == 0)
			break;

		printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadPageDescriptionTable - Buffer.ID = 0x%x\n",Buffer.ID);

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
	printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadPageDescriptionTable - SynaPdtF34Flash.DataBase = 0x%x\n",SynaPdtF34Flash.DataBase);
	printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadPageDescriptionTable - SynaPdtF34Flash.QueryBase = 0x%x\n",SynaPdtF34Flash.QueryBase);

	printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadPageDescriptionTable - SynaPdtF01Common.DataBase = 0x%x\n",SynaPdtF01Common.DataBase);
	printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadPageDescriptionTable - SynaPdtF01Common.QueryBase = 0x%x\n",SynaPdtF01Common.QueryBase);

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

	return;
}

void RMI4CheckIfFatalError(int errCode)
{
	if(errCode != ESuccess)
	{
		printk("%s(): errCode:%d\n",__FUNCTION__,errCode);
	}

	return;
}

// Wait for ATTN assertion and see if it's idle and flash enabled
void RMI4WaitATTN(void)
{
  int errorCount = 300;
  int uErrorCount = 0;

  // To work around the physical address error from Control Bridge
  if (SynaWaitForATTN(1000) != ESuccess)
  {
    RMI4CheckIfFatalError(EErrorTimeout);
  }

  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    // To work around the physical address error from control bridge
    // The default check count value is 3. But the value is larger for erase condition
    if((SynaRet != ESuccess) && uErrorCount < errorCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    RMI4CheckIfFatalError(SynaRet);

    // Clear the attention assertion by reading the interrupt status register
    SynaRet = SynaReadRegister(SynaPdtF01Common.DataBase + 1, &SynauStatus, 1);
    RMI4CheckIfFatalError(SynaRet);

  } while( SynauPageData[0] != s_uF34ReflashCmd_NormalResult && uErrorCount < errorCount);
}


int SynaInitialize(struct i2c_client *syn_touch)
{
	unsigned char uPage = 0x00;
	unsigned char uF01_RMI_Data[2];
	int m_uStatus;

	SynaI2CClientInit(syn_touch);

	/* Setup to read and write from page 0 */
	SynaRet = SynaWriteRegister(0xff, &uPage, 1);
	RMI4CheckIfFatalError(SynaRet);

	do
	{
		SynaRet = SynaReadRegister(0, &SynauStatus, 1);
		RMI4CheckIfFatalError(SynaRet);

		if(SynauStatus & 0x80)
		{
			/* unconfigured */
			break;
		}
	} while(SynauStatus & 0x40);

	/* Read in the PDT to get addresses for functions 0x01 and 0x34 */
	SynaReadPageDescriptionTable();

	/* Read block size and block count for config and firmware image */
	SynaReadConfigInfo();
  	SynaReadFirmwareInfo();

	SynaSetFlashAddrForDifFormat();

	/* Check that we got them OK - it's a fatal error if not */
	if(SynaPdtF34Flash.ID == 0)
	{
		SynaPrintMsg("Function $34 is not supported\n");
		RMI4CheckIfFatalError( EErrorFunctionNotSupported );
	}

	if(SynaPdtF01Common.ID == 0)
	{
		SynaPrintMsg("Function $01 is not supported\n");
		SynaPdtF01Common.ID = 0x01;
    	SynaPdtF01Common.DataBase = 0;
		RMI4CheckIfFatalError( EErrorFunctionNotSupported );
		return -1;
	}

	// Get device status
	m_uStatus = SynaReadRegister(SynaPdtF01Common.DataBase, &uF01_RMI_Data[0], sizeof(uF01_RMI_Data));
	RMI4CheckIfFatalError(m_uStatus);

	// Check Device Status
	printk(KERN_INFO "Configured: %s\n", uF01_RMI_Data[0] & 0x80 ? "false" : "true");
	printk(KERN_INFO "FlashProg:  %s\n", uF01_RMI_Data[0] & 0x40 ? "true" : "false");
	printk(KERN_INFO "StatusCode: 0x%x \n", uF01_RMI_Data[0] & 0x0f );

	/* Initialize working variables */
	SynafirmwareImgData = 0;
	SynaconfigImgData = 0;

	return ESuccess;
}

void SynaSpecialCopyEndianAgnostic(unsigned char *dest, unsigned short src)
{
  dest[0] = src%0x100;
  dest[1] = src/0x100;
}

/* Read Bootloader ID from Block Data Registers as a 'key value' */
unsigned int SynaReadBootloadID()
{
  unsigned char uData[2];

  SynaRet = SynaReadRegister(SynauF34ReflashQuery_BootID, &uData[0], 2);

  SynaBootloadID = (unsigned int)uData[0] + (unsigned int)uData[1]*0x100;
  printk(KERN_INFO "synaptics_ts_fw_angent : SynaReadBootloadID - SynaBootloadID = 0x%x\n",SynaBootloadID);
  return SynaRet;
}

/* Write the bootloader ID to the sensor */
unsigned int SynaWriteBootloadID()
{
  unsigned char uData[2];

  SynaSpecialCopyEndianAgnostic(uData, SynaBootloadID);

  SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, &uData[0], 2);

  return SynaRet;
}

/* Enable Flash programming */
void SynaEnableFlashing()
{
  int uErrorCount = 0;
  unsigned char uData[2];

  /* Read bootload ID */
  SynaRet = SynaReadBootloadID();
  RMI4CheckIfFatalError(SynaRet);

  /* Write bootID to block data registers  */
  SynaRet = SynaWriteBootloadID();
  RMI4CheckIfFatalError(SynaRet);

  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    /* To deal with an error when device is busy and not available for read */
    if((SynaRet != ESuccess) && uErrorCount < DefaultErrorRetryCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

	RMI4CheckIfFatalError(SynaRet);

    /* Clear the attention assertion by reading the interrupt status register */
    SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);
	RMI4CheckIfFatalError(SynaRet);
  } while(((SynauPageData[0] & 0x0f) != 0x00) && (uErrorCount <= DefaultErrorRetryCount));

  /* Issue Enable flash command */
  uData[0] = 0x0f;
  SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  RMI4CheckIfFatalError(SynaRet);

  /* Read the page descriptor table */
  SynaReadPageDescriptionTable();

  // Wait for ATTN and check if flash command state is idle
  RMI4WaitATTN();

  SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  if ( uData[0] != 0x80 )
  {
    SynaPrintMsg("\nFlash failed\n");
    SynaExit();
  }
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

void SynaReadFirmwareHeader(void)
{
  unsigned long firmwareImgFileCheckSum = 0;
  unsigned long checkSumCode = 0;

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
    SynaPrintMsg("\nError: SynaFirmwareImage invalid checksum.\n");
    SynaExit();
  }

  if (SynafileSize != (0x100+SynafirmwareImgSize+SynaconfigImgSize))
  {
    SynaPrintMsg("Error: SynaFirmwareImage actual size not expected size.\n");
    SynaExit();
  }

  if (SynafirmwareImgSize != SynaGetFirmwareSize())
  {
    SynaPrintMsg("\nFirmware image size verfication failed.\n");
    SynaExit();
  }

  if (SynaconfigImgSize != SynaGetConfigSize())
  {
    SynaPrintMsg("Configuration size verfication failed.\n");
    SynaExit();
  }

  SynafirmwareImgData = (unsigned char *)((&SynaFirmware[0])+0x100);
  SynaconfigImgData   = (unsigned char *)((&SynaFirmware[0])+0x100+SynafirmwareImgSize);

  SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);
}

/* Wait for ATTN assertion and see if the status is idle and we're ready to continue */
void SynaWaitATTN(int errorCount)
{
  int uErrorCount = 0;

  if (SynaWaitForATTN(DefaultMillisecondTimeout))
  {
    printk(KERN_ERR "synaptics_ts_fw_angent - SynaWaitATTN Time out Error\n");
  }

  /* after we get the ATTN line signaled we need to loop and read the interrupt status register
   * until we see the status indicating the sensor is ready to continue. This can take a while
   * depending on the command that was sent. Reading the interrups status register will clear
   * the attention line. */
  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    /* To work around the physical address error. */

    if(SynaRet && uErrorCount < errorCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

    /* Clear the attention assertion by reading the interrupt status register */
    SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);
  } while( SynauPageData[0] != 0x80 && uErrorCount < errorCount);
}

/**************************************************************
 This function writes firmware to device one block at a time
 **************************************************************/
int SynaFlashFirmwareWrite()
{
  unsigned char *puFirmwareData = SynafirmwareImgData;
  unsigned char uData[2];
  unsigned short uBlockNum;

#if 0
  unsigned char temp[20];
  int i;
#endif

  for (uBlockNum = 0; uBlockNum < SynafirmwareBlockCount; ++uBlockNum)
  {
    uData[0] = uBlockNum & 0xff;
    uData[1] = (uBlockNum & 0xff00) >> 8;

    /* Write Block Number */
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockNum, &uData[0], 2);

    /* Write Data Block */
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, puFirmwareData, SynafirmwareBlockSize);

#if 0
	printk(KERN_INFO "SynaFlashFirmwareWrite : blockNum = %d\n",uBlockNum);
	for(i=0;i<SynafirmwareBlockSize;i++)
	{
		printk(KERN_INFO "0x%x  ",*(puFirmwareData+i));
	}
	printk(KERN_INFO "\n");

	SynaReadRegister(SynauF34Reflash_BlockData, &temp[0], SynafirmwareBlockSize);
	for(i=0;i<SynafirmwareBlockSize;i++)
	{
		printk(KERN_INFO "0x%x  ",temp[i]);
	}
	printk(KERN_INFO "\n");
#endif

    /* Move to next data block */
    puFirmwareData += SynafirmwareBlockSize;

    /* Issue Write Firmware Block command */
    uData[0] = 2;
    SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);

    /* Wait ATTN until device is done writing the block and is ready for the next. */
    RMI4WaitATTN();
  }

  return ESuccess;
}

void SynaProgramFirmware()
{
  unsigned char uData[1];

  SynaRet = SynaReadBootloadID();

  /* Write bootID to data block register */
  SynaRet = SynaWriteBootloadID();
  RMI4CheckIfFatalError(SynaRet);

  /* Issue the firmware erase command */
  uData[0] = 3;
  SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
  RMI4CheckIfFatalError(SynaRet);

  RMI4WaitATTN();

  /* Write firmware image */
  SynaRet = SynaFlashFirmwareWrite();
  RMI4CheckIfFatalError(SynaRet);
}

/**************************************************************
 This function writes config data to device one block at a time
 **************************************************************/
void SynaProgramConfiguration(void)
{
  unsigned char uData[2];
  unsigned char *puData = SynaconfigImgData;
  unsigned short blockNum;

#if 0
  unsigned char temp[20];
  int i;
#endif

  for (blockNum = 0; blockNum < SynaconfigBlockCount; blockNum++)
  {
    SynaSpecialCopyEndianAgnostic(&uData[0], blockNum);

    /* Write Configuration Block Number */
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockNum, &uData[0], 2);
    RMI4CheckIfFatalError(SynaRet);

    /* Write Data Block */
    SynaRet = SynaWriteRegister(SynauF34Reflash_BlockData, puData, SynaconfigBlockSize);
	RMI4CheckIfFatalError(SynaRet);

#if 0
		printk(KERN_INFO "SynaProgramConfiguration : blockNum = %d\n",blockNum);
		for(i=0;i<SynaconfigBlockSize;i++)
		{
			printk(KERN_INFO "0x%x	",*(puData+i));
		}
		printk(KERN_INFO "\n");

		SynaReadRegister(SynauF34Reflash_BlockData, &temp[0], SynaconfigBlockSize);
		for(i=0;i<SynaconfigBlockSize;i++)
		{
			printk(KERN_INFO "0x%x	",temp[i]);
		}
		printk(KERN_INFO "\n");
#endif

    puData += SynaconfigBlockSize;

    /* Issue Write Configuration Block command to flash command register */
    uData[0] = 0x06;
    SynaRet = SynaWriteRegister(SynauF34Reflash_FlashControl, &uData[0], 1);
	RMI4CheckIfFatalError(SynaRet);

    // Wait for ATTN
    RMI4WaitATTN();
  }
}

int SynaFinalizeFlash()
{
  unsigned char uData[2];
  unsigned int uErrorCount = 0;
  unsigned int i=0;

  /* Reset the sensor by issuing a reset command = 1 */
  uData[0] = 1;
  SynaRet = SynaWriteRegister(SynaPdtF01Common.CommandBase, &uData[0], 1);
  RMI4CheckIfFatalError(SynaRet);

  /* wait up to 200 milliseconds for sensor to come back from a reset */
  SynaSleep(DefaultWaitPeriodAfterReset);

  /* Wait for ATTN to be asserted to see if device is in idle state */
  if (SynaWaitForATTN(DefaultMillisecondTimeout)!= ESuccess)
  {
    RMI4CheckIfFatalError(EErrorTimeout);
  }

  do {
    SynaRet = SynaReadRegister(SynauF34Reflash_FlashControl, &SynauPageData[0], 1);

    /* To work around an error from device if device not ready */
    if((SynaRet!= ESuccess) && uErrorCount < DefaultErrorRetryCount)
    {
      uErrorCount++;
      SynauPageData[0] = 0;
      continue;
    }

  } while(((SynauPageData[0] & 0x0f) != 0x00) && (uErrorCount < DefaultErrorRetryCount));
  RMI4CheckIfFatalError(EErrorTimeout);

  /* Clear the attention assertion by reading the interrupt status register */
  SynaRet = SynaReadRegister((unsigned short)(SynaPdtF01Common.DataBase + 1), &SynauStatus, 1);
  RMI4CheckIfFatalError(EErrorTimeout);

  /* Read F01 Flash Program Status, ensure the 6th bit is '0' */
  uData[0] = 0xFF;

  for(i=500; i>0; i--)
  {
    SynaSleep(1);
    SynaRet = SynaReadRegister(SynauF01RMI_DataBase, &uData[0], 1);
	RMI4CheckIfFatalError(SynaRet);

    if((uData[0] & 0x40) == 0)
    {
		break;
	}
  }

  if(!i)
  {
	if(SynaRet)
		printk(KERN_INFO "synaptics_ts_fw_angent : SynaFinalizeFlash - Error = 0x%x\n",SynaRet);
	else
		SynaPrintMsg("time is not enough!!\n");
  }

/*
  do
  {
    SynaSleep(100);
    SynaRet = SynaReadRegister(SynauF01RMI_DataBase, &uData[0], 1);

  } while((uData[0] & 0x40)!= 0);
*/
  /* With a new flash image the page description table could change so re-read it. */
  SynaReadPageDescriptionTable();

  return ESuccess;
}

#if 0
/* write block data test */
void synaptics_test_write_block_data(void)
{
	// Write Data Block
	int i;
	unsigned short address;
	unsigned char write_data[16]={0x55};
	unsigned char read_data[16]={0x00};
	write_data[0]=0x11;
	write_data[1]=0x22;
	write_data[1]=0x33;
	write_data[15]=0x66;

	address=2;

	printk("%s(): synaptics test start\n",__FUNCTION__);

//	m_ret = SynaReadRegister(0xB4, read_data, 10);
//	printk("%s(): read B4 : %s\n",__FUNCTION__,read_data);

	printk("%s(): m_uF34Reflash_BlockData : 0x%x\n",__FUNCTION__,SynauF34Reflash_BlockData);

	/* byte data */
	SynaRet = SynaWriteRegister(address, write_data, 1);
	printk("%s(): write 1 byte: 0x%x\n",__FUNCTION__,write_data[0]);

	SynaRet = SynaReadRegister(address, read_data, 1);
	printk("%s(): read 1 byte: 0x%x\n",__FUNCTION__,read_data[0]);


	/* word data */
	SynaRet = SynaWriteRegister(address, write_data, 2);
	printk("%s(): write 2 bytes: 0x%x,  0x%x\n",__FUNCTION__,write_data[0],write_data[1]);

	SynaRet = SynaReadRegister(address, read_data, 2);
	printk("%s(): read 2 bytes: 0x%x,  0x%x\n",__FUNCTION__,read_data[0],read_data[1]);

	/* block data */
	for(i=0; i<16; i++)
		write_data[i]=i;

	SynaRet = SynaWriteRegister(address, write_data, 16);
	printk("%s(): write 16 bytes:",__FUNCTION__);
	for(i=0; i<16; i++)
		printk("  0x%x",write_data[i]);
	printk("\n");

	SynaRet = SynaReadRegister(address, read_data, 16);
	printk("%s(): read 16 bytes:",__FUNCTION__);
	for(i=0; i<16; i++)
		printk("  0x%x",read_data[i]);
	printk("\n");


}
#endif

/*
 *  Performs all the steps needed to reflash the image.
 */
unsigned int SynaDoReflash(struct i2c_client *syn_touch, int fw_revision)
{
#if 1
  if((fw_revision == SynaFirmware[0x1F]) && (is_need_forced_update == 0))
  {
    printk(KERN_INFO "synaptics_ts_fw_angent : F/W version is up-to-date\n");
  	return -1;
  }
#endif

  is_fw_reflash = 1;

  printk(KERN_INFO "synaptics_ts_fw_angent : F/W Upgrade!! - from 0x%x to 0x%x\n",fw_revision,SynaFirmware[0x1F]);

  //Start reflash
  if(SynaInitialize(syn_touch) != ESuccess)
  {
    printk(KERN_INFO "synaptics_ts_fw_angent : SynaInitialize failed!!\n");
	return -1;
  }

  /* Enable flash mode */
  SynaEnableFlashing();

#if 0
  /* test for block write */
  synaptics_test_write_block_data();
#endif

  /* Read the firmware header info from the byte array defined in SynaFirmwareImage.h */
  SynaReadFirmwareHeader();

  /* Program the firmware image */
  SynaProgramFirmware();

  /* Program the new configuration */
  SynaProgramConfiguration();

  /* Reset the device and do checks to finalize reflashing */
  if(SynaFinalizeFlash()!=ESuccess)
  {
    return -1;
  }

  /* return the globally set status */
  return SynaRet;
}
