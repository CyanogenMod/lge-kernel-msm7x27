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
#include <linux/usb/android_composite.h>
#endif
#include <mach/board.h>
#ifdef CONFIG_MACH_LGE
#include <mach/board_lge.h>
#endif

#include "u_lgeusb.h"

static struct lgeusb_info *usb_info;

/* FIXME: This length must be same as MAX_STR_LEN in android.c */
#define MAX_SERIAL_NO_LEN 20

#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_CDMA
extern int msm_chg_LG_cable_type(void);
extern void msm_get_MEID_type(char* sMeid);

static int get_serial_number(char *serial_number)
{
	memset(serial_number, 0, MAX_SERIAL_NO_LEN);

	msm_get_MEID_type(serial_number);

	if(!strcmp(serial_number,"00000000000000"))
		serial_number[0] = '\0';

	return 0;
}

static int get_factory_cable(void)
{
	int cable_type =  msm_chg_LG_cable_type();
	int ret;

	switch(cable_type) {
		case LGE_FACTORY_CABLE_TYPE :
		case LGE_FACTORY_CABLE_130K_TYPE :
			ret = cable_type;
			break;
		default:
			ret = 0;
			break;
	}

	return ret;
}
#endif /* CONFIG_USB_SUPPORT_LGE_GADGET_CDMA */

#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_GSM
static int get_serial_number(char *serial_number)
{
	unsigned char nv_imei_ptr[MAX_IMEI_LEN];
	int ret = -1;

	ret = msm_nv_imei_get(nv_imei_ptr);
	if (ret < 0) {
		nv_imei_ptr[0] = '\0';
		pr_info("%s : IMEI is NULL\n", __func__);
	} else {
		pr_info("%s : IMEI %s\n", __func__, nv_imei_ptr);
	}

	if (nv_imei_ptr[0] != '\0') {
		if ((nv_imei_ptr[0] == '8') && (nv_imei_ptr[1] == '0') &&
				(nv_imei_ptr[2] == 'A')) {
			memset(serial_number, 0, MAX_SERIAL_NO_LEN);
			/* We set serialno include header "80A" */
			memcpy(serial_number, nv_imei_ptr, MAX_IMEI_LEN);
			return 0;
		} else {
			serial_number[0] = '\0';
		}
	} else {
		serial_number[0] = '\0';
	}

	return ret;
}

static int get_factory_cable(void)
{
	int pif_detect = 0;

#ifdef CONFIG_LGE_DETECT_PIF_PATCH
	pif_detect = lge_get_pif_info();
#endif
	pr_info("%s : Using PIF ZIG (%d)\n", __func__, pif_detect);

	if (pif_detect == LGE_PIF_CABLE)
		return LGE_FACTORY_CABLE_TYPE;
	else
		return 0;
}
#endif /* CONFIG_USB_SUPPORT_LGE_GADGET_GSM */

static void do_switch_mode(int pid, int need_reset)
{
	struct lgeusb_info *info = usb_info;

	pr_info("do_switch_mode : pid %x, need_reset %d\n", pid, need_reset);
	info->switch_func(pid, need_reset);
}

/* LGE_CHANGE
 * If factory cable (PIF or LT) is connected,
 * return 1, otherwise return 0.
 * 2011-01-13, hyunhui.park@lge.com
 */
int lgeusb_detect_factory_cable(void)
{
	return get_factory_cable();
}

/* LGE_CHANGE
 * If factory dedicated cable is connected,
 * switch usb mode to factory mode.
 * 2011-01-13, hyunhui.park@lge.com
 */
void lgeusb_switch_factory_mode(int need_reset)
{
	struct lgeusb_info *info = usb_info;

	info->current_mode = LGEUSB_FACTORY_MODE;
	do_switch_mode(LGE_FACTORY_PID, need_reset);
}

void lgeusb_switch_android_mode(int need_reset)
{
	struct lgeusb_info *info = usb_info;

	info->current_mode = LGEUSB_ANDROID_MODE;
	do_switch_mode(info->restore_pid, need_reset);
}

int lgeusb_get_current_mode(void)
{
	struct lgeusb_info *info = usb_info;

	return info->current_mode;
}

void lgeusb_backup_pid(void)
{
	struct lgeusb_info *info = usb_info;

	info->restore_pid = info->get_pid();
}

/* LGE_CHANGE
 * 1. If cable is factory cable, switch manufacturing mode.
 * 2. Get serial number from CP and set product id to CP.
 * 2011-01-13, hyunhui.park@lge.com
 */
int lgeusb_set_config(int pid, char *serialno, const char *defaultno)
{
	int ret;

	if (!serialno || !defaultno) {
		pr_info("%s: serial number args are invalid, skip configuration\n",
				__func__);
		return -EINVAL;
	}

	if (get_factory_cable()) {
		/* When manufacturing, do not use serial number */
		/* Without soft usb reset */
		lgeusb_switch_factory_mode(0);
		msm_hsusb_send_productID(LGE_FACTORY_PID);
		msm_hsusb_is_serial_num_null(1);
		serialno[0] = '\0';
		return LGE_FACTORY_PID;
	}

	lgeusb_switch_android_mode(0);

	ret = get_serial_number(serialno);

	msm_hsusb_send_productID(pid);
	msm_hsusb_is_serial_num_null(0);

	if (!ret && (serialno[0] != '\0'))
		msm_hsusb_send_serial_number(serialno);
	else
		msm_hsusb_send_serial_number(defaultno);

	if(ret < 0)
		pr_info("lge usb configuration: fail to get serial number, set to default serial number\n");

	return pid;
}

/* LGE_CHANGE
 * Register lge usb information(which include callback functions).
 * 2011-01-14, hyunhui.park@lge.com
 */
void lgeusb_register_usbinfo(struct lgeusb_info *info)
{
	if (info) {
		usb_info = info;
		pr_info("%s: switch_func %p, get_pid %p\n", __func__,
				usb_info->switch_func,
				usb_info->get_pid);
	} else {
		pr_info("%s : registering usb info failed\n", __func__);
	}
}



