/* include/linux/kxsd9.h - AMI602 motion sensor driver
 *
 * Copyright (C) 2009 AMIT Technology Inc.
 * Author: Kyle Chen <sw-support@amit-inc.com>
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
/*
 * Definitions for kxsd9 motion sensor chip.
 */
#ifndef AMI602_H
#define AMI602_H

#include <linux/ioctl.h>
//#include <asm-arm/arch/regs-gpio.h>


#define AMI602_I2C_SLAVE_ADDR		0x30

/* AMI602 Internal Command  (Please refer to AMI602 Specifications) */
#define AMI602_CMD_GET_FIRMWARE		0x17
#define AMI602_CMD_GET_MES			0x14
#define AMI602_CMD_REQ_MES			0x55
#define AMI602_CMD_SET_AEN			0x74
#define AMI602_CMD_SET_MES_6CH_AUTO_START   0x64
#define AMI602_CMD_SET_MES_PED_AUTO_START   0x77
#define AMI602_CMD_GET_MES_SUSPEND			0x28
#define AMI602_CMD_GET_MES_PED_AUTO_SUSPEND 0x32 
#define AMI602_CMD_SET_MES_6CH_AUTO_STOP    0x65
#define AMI602_CMD_SET_MES_PED_AUTO_STOP    0x78
#define AMI602_CMD_SET_SUSPEND				0x75
#define AMI602_CMD_SET_PWR_DOWN				0x57

/* IOCTLs for kxsd9 misc. device library */
#define AMI602IO						   0x82
#define AMI602_IOCTL_INIT                  _IO(AMI602IO, 0x01)
#define AMI602_IOCTL_READ_CHIPINFO         _IOR(AMI602IO, 0x02, int)
#define AMI602_IOCTL_READ_SENSORDATA       _IOR(AMI602IO, 0x03, int)
#define AMI602_IOCTL_READ_POSTUREDATA      _IOR(AMI602IO, 0x04, int)
#define AMI602_IOCTL_READ_CALIDATA         _IOR(AMI602IO, 0x05, int)
#define AMI602_IOCTL_SET_MODE              _IOW(AMI602IO, 0x06, int)

/* IOCTLs for AMI602 middleware misc. device library */
#define AMI602MIDIO						   0x83
#define AMI602MID_IOCTL_SET_POSTURE        _IOW(AMI602MIDIO, 0x01, int)
#define AMI602MID_IOCTL_SET_CALIDATA       _IOW(AMI602MIDIO, 0x02, int)
#define AMI602MID_IOCTL_SET_CONTROL        _IOW(AMI602MIDIO, 0x03, int)
#define AMI602MID_IOCTL_GET_CONTROL        _IOR(AMI602MIDIO, 0x04, int)
#define AMI602_BUFSIZE				256

#endif

