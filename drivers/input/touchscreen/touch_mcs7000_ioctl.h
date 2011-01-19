/*
 * drivers/input/touchscreen/mcs7000_ts_ioctl.h
 *
 * Header file of Touch Driver
 * 
 * Copyright (C) 2008 LGE Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _MCS7000_TS_IOCTL_H_
#define _MCS7000_TS_IOCTL_H_

#include <asm/ioctl.h>

#define MCS7000_TS_IOCTL_MAGIC 'T'

struct mcs7000_ioctl_i2c_type {
	int ts_type;
	int addr;
	int data;
};

struct mcs7000_ioctl_i2c_burst_type {
	int ts_type;
	int addr;
	int size;
	unsigned char *pdata;
};

struct mcs7000_ts_ioctl_diag_emul_type {
	int ts_type;
	int x_coord;
	int y_coord;
};

#define MCS7000_TS_IOCTL_MAIN_ON 					_IO( MCS7000_TS_IOCTL_MAGIC, 0)
#define MCS7000_TS_IOCTL_MAIN_OFF 					_IO( MCS7000_TS_IOCTL_MAGIC, 1)
#define MCS7000_TS_IOCTL_SUB_ON 					_IO( MCS7000_TS_IOCTL_MAGIC, 2)
#define MCS7000_TS_IOCTL_SUB_OFF 					_IO( MCS7000_TS_IOCTL_MAGIC, 3)
#define MCS7000_TS_IOCTL_I2C_TEST_MAIN 				_IO( MCS7000_TS_IOCTL_MAGIC, 4)
#define MCS7000_TS_IOCTL_I2C_TEST_SUB 				_IO( MCS7000_TS_IOCTL_MAGIC, 5)
#define MCS7000_TS_IOCTL_DIAG_PRESS 				_IOW( MCS7000_TS_IOCTL_MAGIC, 6, struct mcs7000_ts_ioctl_diag_emul_type)
#define MCS7000_TS_IOCTL_DIAG_RELEASE 				_IOW( MCS7000_TS_IOCTL_MAGIC, 7, struct mcs7000_ts_ioctl_diag_emul_type)
#define MCS7000_TS_IOCTL_FW_VER 					_IO( MCS7000_TS_IOCTL_MAGIC, 8)
#define MCS7000_TS_IOCTL_DEBUG 						_IO( MCS7000_TS_IOCTL_MAGIC, 9)
#define MCS7000_TS_I2C_READ 						_IOWR( MCS7000_TS_IOCTL_MAGIC, 10, struct mcs7000_ioctl_i2c_type)
#define MCS7000_TS_I2C_WRITE						_IOWR( MCS7000_TS_IOCTL_MAGIC, 11, struct mcs7000_ioctl_i2c_type)
#define MCS7000_TS_IOCTL_CHANNEL_DEBUG 				_IO( MCS7000_TS_IOCTL_MAGIC, 12)
#define MCS7000_TS_I2C_READ_BURST 					_IOWR( MCS7000_TS_IOCTL_MAGIC, 13, struct mcs7000_ioctl_i2c_burst_type)

#define MCS7000_TS_IOCTL_MAXNR 14

#endif
