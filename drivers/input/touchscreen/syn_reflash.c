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

#include <linux/delay.h>
#include <mach/gpio.h>

#include "syn_fw_1818_9.h"
#include "syn_fw_1818_A.h"
#include "syn_fw_1912.h"
#include "syn_reflash.h"

/* local structs, vars and defines */
struct rmi_function
{
	unsigned char querybase;
	unsigned char cmdbase;
	unsigned char contbase;
	unsigned char database;
	unsigned char intcount;
	unsigned char id;
};
struct rmi_function f34_flash;
struct rmi_function f01_common;

/* extern vars from rmi */
int is_need_update;
int is_fw_reflash;
unsigned char *firmware;

/* vars used for flashing image and configuration */
unsigned short bootload_id;
unsigned short bootload_img_id;
unsigned char  fw_img_ver;
unsigned char *fw_img_data;
unsigned char *config_img_data;
unsigned short fw_block_size;
unsigned short fw_block_count;
unsigned short config_block_size;
unsigned short config_block_count;
unsigned char  pagedata[0x200];
unsigned char  reg_status;
unsigned long  fw_img_size;
unsigned long  config_img_size;
unsigned long  file_size;

/* Stored register addresses used for image flashing and configuration */
unsigned short f01_database;
unsigned short ref_block_num;
unsigned short ref_block_data;
unsigned short ref_flash_control;
unsigned short ref_boot_id;
unsigned short ref_fw_block_size;
unsigned short ref_fw_block_count;
unsigned short ref_config_block_size;
unsigned short ref_config_block_count;
unsigned short ref_flash_property;

/* Read the Flash control register. The result should be */
static const unsigned char f34_ref_cmd_normal_result  = 0x80;
struct i2c_client *syn_touch_client;

#define ATTN_GPIO	92
#define NOREFLASHVER 0x06

unsigned char* firmware = NULL;

static int check_ts_firmware(enum firmware_type type, int fw_revision)
{
	switch (type) {
	case syn_1818:
		if (fw_revision == 0x06 || fw_revision == 0x0A) {
			printk("[T-Reflash] firmware is firmware_1818 0x0A\n");
			firmware = firmware_1818_A;
			file_size = sizeof(firmware_1818_A);
		} else {
			printk("[T-Reflash] firmware is firmware_1818 0x09\n");
			firmware = firmware_1818_9;
			file_size = sizeof(firmware_1818_9);
		}
		break;
	case syn_1912:
		printk("[T-Reflash] firmware is firmware_1912\n");
		firmware = firmware_1912;
		file_size = sizeof(firmware_1912);
		break;
	default:
		printk("[T-Reflash] firmware is firmware_1818 0x09\n");
		firmware = firmware_1818_9;
		file_size = sizeof(firmware_1818_9);
	}

	if (is_need_update == 0) {
		if(fw_revision < NOREFLASHVER) {
			printk("[T-Reflash] Do not reflash for Lower version\n");
			return -1;
		} else if(fw_revision == firmware[0x1F]) {
			printk("[T-Reflash] synaptics_ts_fw_angent : F/W version is up-to-date\n");
			return -1;
		}
	}

	return 0;
}

/* Special defined error codes used by Synaptics - the OEM should not define any error codes the same. */

int wait_for_attn(unsigned long msec)
{
	int retries = 0;
	while((gpio_get_value(ATTN_GPIO) != 0)) {
		if(retries < msec)
			retries++;
		else {
			printk("[T-Reflash] Wait For ATTN time over!!\n");
			break;
		}

		msleep(1);
	}

	if(gpio_get_value(ATTN_GPIO) == 0)
		return 0;

	return -1;
}

int write_reg(unsigned short   rmi_address, unsigned char  *data, unsigned int     length)
{
	int smbus_ret;
	unsigned short data_word;
	if(rmi_address > 0xFF) {
		printk("[T-Reflash] Error SMB Address\n");
		return -1;
	}

	if(length > 0xFF) {
		printk("[T-Reflash] Error SMB Length\n");
		return -1;
	}

	switch(length)
	{
		case 1:
			smbus_ret=i2c_smbus_write_byte_data(syn_touch_client,(unsigned char)rmi_address,*data);
			break;
		case 2:
			data_word=data[1]*0x100+data[0];
			smbus_ret=i2c_smbus_write_word_data(syn_touch_client,(unsigned char)rmi_address,data_word);
			break;
		default:
			smbus_ret=i2c_smbus_write_i2c_block_data(syn_touch_client,(unsigned char)rmi_address,(unsigned char)length, data);
			break;
	}

	/* if smbus success */
	if(smbus_ret >= 0)
	{
		return 0;
	}

	printk("[T-Reflash] smbbus write error : err_code:%d\n",smbus_ret);
	return smbus_ret;
}

int read_reg(unsigned short  rmi_address, unsigned char * data, unsigned int length)
{
	int smbus_ret;
	struct i2c_msg msg[2];
	unsigned char start_reg;

	if(rmi_address > 0xFF) {
		printk("[T-Reflash] Error SMB Address\n");
		return -1;
	}

	if(length > 0xFF) {
		printk("[T-Reflash] Error SMB Length\n");
		return -1;
	}

	if(length == 1) {
		smbus_ret = i2c_smbus_read_byte_data(syn_touch_client,(unsigned char)rmi_address);
		*data=(unsigned char)smbus_ret;
	} else {
		msg[0].addr = syn_touch_client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &start_reg;
		start_reg = (unsigned char)rmi_address;
		msg[1].addr = syn_touch_client->addr;
		msg[1].flags = I2C_M_RD;
		msg[1].len = length;
		msg[1].buf = data;

		smbus_ret = i2c_transfer(syn_touch_client->adapter, msg, 2);
	}

    /* if smbus success */
	if(smbus_ret >= 0)
		return 0;

	printk("[T-Reflash] smbbus read error  error number:%d\n",smbus_ret);
	return smbus_ret;
}

/* This function gets the firmware block size, block count and image size */
int firmware_info()
{
	unsigned char data[2];
	int ret;

	ret = read_reg(ref_fw_block_size, &data[0], 2);
	if (ret != 0)
		return -1;
	fw_block_size = data[0] | (data[1] << 8);

	ret = read_reg(ref_fw_block_count, &data[0], 2);
	if (ret != 0)
		return -1;
	fw_block_count = data[0] | (data[1] << 8);

	fw_img_size = fw_block_count * fw_block_size;
	printk("[T-Reflash] firmwareImgSize=0x%lx \n",fw_img_size);

	return 0;
}

/* This function gets config block count, config block size and image size */
int config_info()
{
	unsigned char data[2];
	int ret;

	ret = read_reg(ref_config_block_size, &data[0], 2);
	config_block_size = data[0] | (data[1] << 8);

	ret = read_reg(ref_config_block_count, &data[0], 2);
	config_block_count = data[0] | (data[1] << 8);

	config_img_size = config_block_size*config_block_count;
	printk("[T-Reflash] configImgSize=0x%lx \n",config_img_size);

	return ret;
}

int set_flash_addr()
{
	unsigned int RegFormat;
	int ret;

	ret = read_reg(ref_flash_property, &pagedata[0], 1);
	RegFormat = ((unsigned int)((pagedata[0] & 0x01) == 0x01));

	if (RegFormat) {
		ref_flash_control = f34_flash.database + fw_block_size + 2;
		ref_block_num = f34_flash.database;
		ref_block_data = f34_flash.database + 2;
	} else {
		ref_flash_control = f34_flash.database;
		ref_block_num = f34_flash.database + 1;
		ref_block_data = f34_flash.database + 3;
	}
	return ret;
}

/* This function reads the Page Descriptor Table and assigns the function 0x01 and
 * function 0x34 addresses for data, control, query, etc.                          */
int read_description()
{
	struct rmi_function Buffer;
	unsigned short address;
	int ret;

	f01_common.id = 0;
	f34_flash.id = 0;

	msleep(20);

	for (address = 0xe9; address > 0xbf; address -= sizeof(struct rmi_function)) {
		ret = read_reg(address, (unsigned char*)&Buffer, sizeof(Buffer));

		if(Buffer.id == 0)
			break;

		printk("[T-Reflash] read_description - Buffer.id = 0x%x\n",Buffer.id);

		switch(Buffer.id) {
		case 0x34:
			f34_flash = Buffer;
			break;

		case 0x01:
			f01_common = Buffer;
        	break;

		default:
			break;
		}
	}
	/* Initialize function address bases for function 0x01 and function 0x34. */
	f01_database = f01_common.database;

	ref_block_num = f34_flash.database;
	ref_block_data = f34_flash.database + 2;

	ref_boot_id = f34_flash.querybase;
	ref_flash_property = f34_flash.querybase + 2;
	ref_fw_block_size = f34_flash.querybase + 3;
	ref_fw_block_count = f34_flash.querybase + 5;
  	ref_config_block_size = f34_flash.querybase + 3;
	ref_config_block_count = f34_flash.querybase + 7;

	return ret;
}

/* Wait for ATTN assertion and see if it's idle and flash enabled */
void rmi4_wait_attn(void)
{
	int tot_err = 300;
	int err_count = 0;
	int ret;

	/* To work around the physical address error from Control Bridge */
	wait_for_attn(1000);

	do {
		ret = read_reg(ref_flash_control, &pagedata[0], 1);
		/* To work around the physical address error from control bridge */
		/* The default check count value is 3. But the value is larger for erase condition */
		if((ret != 0) && err_count < tot_err) {
			err_count++;
			pagedata[0] = 0;
			continue;
		}

		/* Clear the attention assertion by reading the interrupt status register */
		ret = read_reg(f01_common.database + 1, &reg_status, 1);
	} while( pagedata[0] != f34_ref_cmd_normal_result && err_count < tot_err);
}

int initialize(struct i2c_client *syn_touch)
{
	unsigned char page = 0x00;
	unsigned char f01_rmi_data[2];
	int ret;
	unsigned char status;

	syn_touch_client = syn_touch;

	/* Setup to read and write from page 0 */
	ret = write_reg(0xff, &page, 1);

	do {
		read_reg(0, &reg_status, 1);

		if(reg_status & 0x80) {
			/* unconfigured */
			break;
		}
	} while(reg_status & 0x40);

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
	if(f34_flash.id == 0) {
		printk("[T-Reflash] Function $34 is not supported\n");
	}

	if(f01_common.id == 0) {
		printk("[T-Reflash] Function $01 is not supported\n");
		f01_common.id = 0x01;
    	f01_common.database = 0;
		return -1;
	}

	/* Get device status */
	status = read_reg(f01_common.database, &f01_rmi_data[0], sizeof(f01_rmi_data));
	if (status != 0)
		return -1;
	/* Check Device Status */
	printk("[T-Reflash] Configured: %s\n", f01_rmi_data[0] & 0x80 ? "false" : "true");
	printk("[T-Reflash] FlashProg:  %s\n", f01_rmi_data[0] & 0x40 ? "true" : "false");
	printk("[T-Reflash] StatusCode: 0x%x \n", f01_rmi_data[0] & 0x0f );

	/* Initialize working variables */
	fw_img_data = 0;
	config_img_data = 0;

	return 0;
}

/* Enable Flash programming */
int enable_flashing()
{
	int count = 0;
	unsigned char data[2];
	int ret;

	read_reg(ref_boot_id, &data[0], 2);
	write_reg(ref_block_data, &data[0], 2);

	do {
		ret = read_reg(ref_flash_control, &pagedata[0], 1);
		/* To deal with an error when device is busy and not available for read */
		if((ret != 0) && count < 300) {
			count++;
			pagedata[0] = 0;
			continue;
		}

    /* Clear the attention assertion by reading the interrupt status register */
		ret = read_reg((unsigned short)(f01_common.database + 1), &reg_status, 1);
	} while(((pagedata[0] & 0x0f) != 0x00) && (count <= 300));

  /* Issue Enable flash command */
	data[0] = 0x0f;
	ret = write_reg(ref_flash_control, &data[0], 1);
	if(ret != 0)
		return -1;

	/* Wait for ATTN and check if flash command state is idle */
	rmi4_wait_attn();

	/* Read the page descriptor table */
	ret = read_description();
	if(ret != 0)
		return -1;

	ret = read_reg(ref_flash_control, &data[0], 1);
	if(ret != 0)
		return -1;

	if ( data[0] != 0x80 ) {
		printk("[T-Reflash] \nFlash failed\n");
		return -1;
	}
	return 0;
}

void cal_checksum(unsigned short * data, unsigned short len, unsigned long * datablock)
{
	unsigned long temp;
	unsigned long sum1 = 0xffffffff & 0xFFFF;
	unsigned long sum2 = 0xffffffff >> 16;

	*datablock = 0xffffffff;

	while (len--) {
		temp = *data++;
		sum1 += temp;
		sum2 += sum1;
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	*datablock = sum2 << 16 | sum1;
}

/*
 * Read the firmware header (SynaFirmwareImage.h) data and store the image and config info into
 * a local array and assign local variables to be used when burning the image and reflashing
 * the device configuration.
 */

/* Endian agnostic */
__inline unsigned long long_from_header(const unsigned char* image)
{
	return((unsigned long)image[0] +
		   (unsigned long)image[1]*0x100 +
		   (unsigned long)image[2]*0x10000 +
		   (unsigned long)image[3]*0x1000000);
}

int read_firmware_header(void)
{
	unsigned long fw_img_checksum = 0;
	unsigned long checksum_code = 0;
	int ret;

	//file_size = sizeof(firmware) - 1;

	checksum_code   = long_from_header(&(firmware[0]));
	bootload_img_id = (unsigned int)firmware[4] + (unsigned int)firmware[5]*0x100;
	fw_img_ver      = firmware[7];
	fw_img_size     = long_from_header(&(firmware[8]));
	config_img_size = long_from_header(&(firmware[12]));

	cal_checksum((unsigned short*)(&firmware[4]), (unsigned short)((file_size - 4) >> 1),
				 (unsigned long *)(&fw_img_checksum));

	if (fw_img_checksum != checksum_code) {
		printk("[T-Reflash] \nError: SynaFirmwareImage invalid checksum.\n");
	}

	if (file_size != (0x100+fw_img_size+config_img_size)) {
		printk("[T-Reflash] Error: SynaFirmwareImage actual size not expected size.\n");
	}

	if (fw_img_size != (unsigned long)fw_block_size*(unsigned long)fw_block_count) {
		printk("[T-Reflash] \nFirmware image size verfication failed.\n");
	}

	if (config_img_size != (unsigned long)config_block_size*(unsigned long)config_block_count) {
		printk("[T-Reflash] Configuration size verfication failed.\n");
	}

	fw_img_data = (unsigned char *)((&firmware[0])+0x100);
	config_img_data   = (unsigned char *)((&firmware[0])+0x100+fw_img_size);

	ret = read_reg(ref_flash_control, &pagedata[0], 1);
	if (ret != 0)
		return -1;

	return 0;
}

/**************************************************************
 This function writes firmware to device one block at a time
 **************************************************************/
int flash_fw_write()
{
	unsigned char *fw_data = fw_img_data;
	unsigned char data[2];
	unsigned short blocknum;

	for (blocknum = 0 ; blocknum < fw_block_count ; ++blocknum) {
		data[0] = blocknum & 0xff;
		data[1] = (blocknum & 0xff00) >> 8;

		write_reg(ref_block_num, &data[0], 2);
		write_reg(ref_block_data, fw_data, fw_block_size);

		/* Move to next data block */
		fw_data += fw_block_size;

		/* Issue Write Firmware Block command */
		data[0] = 2;
		write_reg(ref_flash_control, &data[0], 1);

		/* Wait ATTN until device is done writing the block and is ready for the next. */
		rmi4_wait_attn();
	}
	return 0;
}

int program_firmware()
{
	unsigned char data[2];
	int ret;

	/* Issue the firmware erase command */

	read_reg(ref_boot_id, &data[0], 2);
	write_reg(ref_block_data, &data[0], 2);

	data[0] = 3;
	ret = write_reg(ref_flash_control, &data[0], 1);
	if (ret != 0)
		return -1;

	rmi4_wait_attn();

	/* Write firmware image */
	flash_fw_write();
	return 0;
}

/**************************************************************
 This function writes config data to device one block at a time
 **************************************************************/
void prog_config(void)
{
	unsigned char data[2];
	unsigned char *pdata = config_img_data;
	unsigned short blockNum;

	for (blockNum = 0; blockNum < config_block_count; blockNum++) {
		data[0] = blockNum&0xff;
		data[1] = (blockNum&0xff00) >> 8;

		/* Write Configuration Block Number */
		write_reg(ref_block_num, &data[0], 2);

		/* Write Data Block */
		write_reg(ref_block_data, pdata, config_block_size);

		pdata += config_block_size;

		/* Issue Write Configuration Block command to flash command register */
		data[0] = 0x06;
		write_reg(ref_flash_control, &data[0], 1);

		/* Wait for ATTN */
		rmi4_wait_attn();
	}
}

int finalize_flash()
{
	unsigned char data[2];
	unsigned int error_count = 0;
	unsigned int i=0;
	int ret;
	/* Reset the sensor by issuing a reset command = 1 */
	data[0] = 1;
	ret = write_reg(f01_common.cmdbase, &data[0], 1);
	if (ret != 0)
		printk("[T-Reflash] Failed to write register\n");
	/* wait up to 200 milliseconds for sensor to come back from a reset */
	msleep(200);

	/* Wait for ATTN to be asserted to see if device is in idle state */
	if (wait_for_attn(300)!= 0) {
		printk("[T-Reflash] Wait_ATTN ; TimeOut\n");
	}

	do {
		ret = read_reg(ref_flash_control, &pagedata[0], 1);

    /* To work around an error from device if device not ready */
		if((ret!= 0) && error_count < 300) {
			error_count++;
			pagedata[0] = 0;
			continue;
		}

	} while(((pagedata[0] & 0x0f) != 0x00) && (error_count < 300));

	if (error_count ==  300)
		printk("[T-Reflash] Over Default Error Retry Count\n");

	/* Clear the attention assertion by reading the interrupt status register */
	ret = read_reg((unsigned short)(f01_common.database + 1), &reg_status, 1);
	if (ret != 0)
		printk("[T-Reflash] Failed to read register\n");

	/* Read F01 Flash Program Status, ensure the 6th bit is '0' */
	data[0] = 0xFF;

	for(i=500; i>0; i--) {
		msleep(1);
		ret = read_reg(f01_database, &data[0], 1);
		if (ret != 0)
			printk("[T-Reflash] Failed to read register\n");

		if((data[0] & 0x40) == 0) {
			break;
		}
	}

	if(!i) {
		if(ret != 0)
			printk("[T-Reflash] synaptics_ts_fw_angent : SynaFinalizeFlash - Error = 0x%x\n",ret);
		else
			printk("[T-Reflash] time is not enough!!\n");
	}

/* With a new flash image the page description table could change so re-read it. */
	read_description();

	return 0;
}

/*
 *  Performs all the steps needed to reflash the image.
 */
unsigned int firmware_reflash(struct i2c_client *syn_touch, int fw_revision,
		enum firmware_type type)
{

	if (check_ts_firmware(type, fw_revision) < 0)
		return -1;

	is_fw_reflash = 1;

	printk("[T-Reflash] synaptics_ts_fw_angent : F/W Upgrade!! - from 0x%x to 0x%x\n",fw_revision,firmware[0x1F]);

    /* Start reflash */
	if(initialize(syn_touch) != 0) {
		printk("[T-Reflash] ts_fw_agent : Initialize failed!!\n");
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
	prog_config();

	/* Reset the device and do checks to finalize reflashing */
	if(finalize_flash()!= 0) {
		return -1;
	}

	/* return the globally set status */
	return 0;
}
