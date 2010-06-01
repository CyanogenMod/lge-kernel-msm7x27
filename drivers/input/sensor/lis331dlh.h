// File name: lis331dlh.h //

/************************* lis331dlh ******************************************

Application description       : lis331dlh Linux driver

                              : STMicroelectronics

Date                          : 14/10/09

Revision                      : 1-0-0

Changed Features              : First Release

Bug fixes                     : First Release

H/W platform                  : OMAP3530

MEMS platform                 : digital output LIS331DLH

S/W platform                  : gcc 4.2.1

Application Details           : lis331dlh Linux driver
                                

Copyright (c) 2009 STMicroelectronics.

THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

THIS DOCUMENT CONTAINS PROPRIETARY AND CONFIDENTIAL INFORMATION OF THE
STMICROELECTRONICS GROUP.
INFORMATION FURNISHED IS BELIEVED TO BE ACCURATE AND RELIABLE. HOWEVER, 
STMICROELECTRONICS ASSUMES NO RESPONSIBILITY FOR THE CONSEQUENCES OF USE
OF SUCH INFORMATION.
SPECIFICATIONS MENTIONED IN THIS PUBLICATION ARE SUBJECT TO CHANGE WITHOUT NOTICE.
THIS PUBLICATION SUPERSEDES AND REPLACES ALL INFORMATION PREVIOUSLY SUPPLIED.
STMICROELECTRONICS PRODUCTS ARE NOT AUTHORIZED FOR USE AS CRITICAL COMPONENTS IN LIFE
SUPPORT DEVICES OR SYSTEMS WITHOUT EXPRESS WRITTEN APPROVAL OF STMICROELECTRONICS.

STMicroelectronics GROUP OF COMPANIES

Australia - Belgium - Brazil - Canada - China - France - Germany - Italy - Japan - Korea -
Malaysia - Malta - Morocco - The Netherlands - Singapore - Spain - Sweden - Switzerland -
Taiwan - Thailand - United Kingdom - U.S.A.
STMicroelectronics Limited is a member of the STMicroelectronics Group.
********************************************************************************
Version History.

Revision 1-0-0 14/10/09
First Release

*******************************************************************************/

#ifndef __LIS331DLH_H__
#define __LIS331DLH_H__

#include <linux/ioctl.h>  /* For IOCTL macros */

/** This define controls compilation of the master device interface */
/*#define LIS331DLH_MASTER_DEVICE*/

//#define LIS331DLH_IOCTL_BASE 77
#define LIS331DLH_IOCTL_BASE 'B'

/** The following define the IOCTL command values via the ioctl macros */
#define LIS331DLH_IOCTL_SET_DELAY	_IOW(LIS331DLH_IOCTL_BASE, 0, int)
#define LIS331DLH_IOCTL_GET_DELAY	_IOWR(LIS331DLH_IOCTL_BASE, 1, int)
#define LIS331DLH_IOCTL_SET_ENABLE	_IOWR(LIS331DLH_IOCTL_BASE, 2, int)
#define LIS331DLH_IOCTL_GET_ENABLE	_IOWR(LIS331DLH_IOCTL_BASE, 3, int)
#define LIS331DLH_IOCTL_SET_G_RANGE	_IOWR(LIS331DLH_IOCTL_BASE, 4, int)
#define LIS331DLH_IOCTL_READ_ACCEL_XYZ _IOWR(LIS331DLH_IOCTL_BASE, 5, int)


#define LIS331DLH_G_2G 0x00
#define LIS331DLH_G_4G 0x10
#define LIS331DLH_G_8G 0x30

#ifdef __KERNEL__
struct lis331dlh_platform_data {
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*lis_init)(void);
	void (*lis_exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);

};

typedef struct  {
		short x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
			 y, /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
			 z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
} lis331dlh_acc_t;

#endif /* __KERNEL__ */

#endif  /* __LIS331DLH_H__ */


