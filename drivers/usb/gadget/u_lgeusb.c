/* linux/drivers/usb/gadget/u_lgeusb.c
 *
 * Copyright (C) 2008 Google, Inc.
 * Copyright (C) 2010 LGE.
 * Author : Hyeon H. Park <hyunhui.park@lge.com>
 *			Youn Suk Song <younsuk.song@lge.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <mach/msm_hsusb.h>
#include <mach/rpc_hsusb.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android.h>
#endif
#include <mach/board.h>
#ifdef CONFIG_MACH_LGE
#include <mach/board_lge.h>
#endif

#include "u_lgeusb.h"

/* FIXME: This length must be same as MAX_SERIAL_LEN in android.c */
#define MAX_SERIAL_NO_LEN 256

/* -------- CDMA Class Utils -------- */

#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_CDMA

extern int msm_chg_LG_cable_type(void);
extern void msm_get_MEID_type(char* sMeid);

static int do_get_usb_serial_number(char *serial_number)
{
/* LGE_CHANGES_S [younsuk.song@lge.com] 2010-06-21, Set MEID */
	memset(serial_number, 0, MAX_SERIAL_NO_LEN);

	msm_get_MEID_type(serial_number);

	if(!strcmp(serial_number,"00000000000000")) 
		serial_number[0] = '\0';
#if 0
	if(msm_chg_LG_cable_type() == LT_ADB_CABLE)
	{
		sprintf(serial_number,"%s","LGE_ANDROID_DE");
	}
#endif		

	return 0;
/* LGE_CHANGES_E [younsuk.song@lge.com] 2010-06-21 */
}

static int do_detect_factory_cable(void)
{
	int cable_type =  msm_chg_LG_cable_type();

	if((cable_type == LG_FACTORY_CABLE_TYPE) ||
			(cable_type == LG_FACTORY_CABLE_130K_TYPE))
		return 1;
	else
		return 0;
}

#endif /* CONFIG_USB_SUPPORT_LGE_GADGET_CDMA */

/* -------- WCDMA(GSM) Class Utils -------- */

#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_GSM

static int do_get_usb_serial_number(char *serial_number)
{
	unsigned char nv_imei_ptr[MAX_IMEI_LEN];
	int ret = -1;

	memset(serial_number, 0, MAX_SERIAL_NO_LEN);

	ret = msm_nv_imei_get(nv_imei_ptr);
	if (ret < 0) {
		nv_imei_ptr[0] = '\0';
		pr_info("%s : IMEI is NULL\n", __func__);
	} else {
		pr_info("%s : IMEI %s\n", __func__, nv_imei_ptr);
	}

	if (nv_imei_ptr[0] != '\0') {
		if ((nv_imei_ptr[0] == '8') && (nv_imei_ptr[1] == '0') &&
				(nv_imei_ptr[2] == 'A'))
			/* We set serialno include header "80A" */
			memcpy(serial_number, nv_imei_ptr, MAX_IMEI_LEN);
		else
			serial_number[0] = '\0';
	} else 
		serial_number[0] = '\0';

	return 0;
}

static int do_detect_factory_cable(void)
{
	int pif_detect = 0;

#ifdef CONFIG_LGE_DETECT_PIF_PATCH
	pif_detect = lge_get_pif_info();
#endif
	pr_info("%s : Using PIF ZIG (%d)\n", __func__, pif_detect);

	if (pif_detect == LGE_PIF_CABLE)
		return 1;
	else
		return 0;
}

#endif /* CONFIG_USB_SUPPORT_LGE_GADGET_GSM */


/* Common Interface */

/*
 * lge_detect_factory_cable
 *
 * If factory cable (PIF or LT) is connected,
 * return 1, otherwise return 0.
 *
 */
int lge_detect_factory_cable(void)
{
	return do_detect_factory_cable();
}
EXPORT_SYMBOL(lge_detect_factory_cable);

/*
 * lge_get_usb_serial_number
 *
 * Get USB serial number from ARM9 using RPC or RAPI.
 * return -1 : If any error
 * return 0 : success
 *
 */
int lge_get_usb_serial_number(char *serial_number)
{
	return do_get_usb_serial_number(serial_number);
}
EXPORT_SYMBOL(lge_get_usb_serial_number);
