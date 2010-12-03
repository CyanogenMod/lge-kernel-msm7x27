/* linux/drivers/usb/gadget/u_lgeusb.h
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

#ifndef __U_LGEUSB_H__
#define __U_LGEUSB_H__

/* Common Interface */
int lge_get_usb_serial_number(char *serial_number);
int lge_detect_factory_cable(void);

/* Per-class definitions */

/* CDMA Class */
#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_CDMA

#define LGE_FACTORY_CABLE_TYPE 3
#define LGE_FACTORY_CABLE_130K_TYPE 10
#define LT_ADB_CABLE 0xff

#endif /* CDMA */

/* GSM/WCDMA Class */
#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_GSM

#define LGE_FACTORY_CABLE_TYPE 1
#define MAX_IMEI_LEN 19
#define LGE_PIF_CABLE 2

#endif /* GSM/WCDMA */

/* LGE_CHANGE_S [hyunhui.park@lge.com] 2010-09-19, Detection of factory cable using wq */
#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_FACTORY_CABLE_WQ
extern int android_switch_composition_ext(u16 pid);
extern int android_get_pid_ext(void);
#endif
/* LGE_CHANGE_E [hyunhui.park@lge.com] 2010-09-19 */

#endif /* __U_LGE_USB_H__ */
