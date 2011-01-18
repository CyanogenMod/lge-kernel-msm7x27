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


#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_CDMA

#define LGE_FACTORY_CABLE_TYPE 3
#define LGE_FACTORY_CABLE_130K_TYPE 10
#define LT_ADB_CABLE 0xff

#endif /* CDMA */

#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_GSM

#define LGE_FACTORY_CABLE_TYPE 1
#define MAX_IMEI_LEN 19
#define LGE_PIF_CABLE 2

#endif /* GSM/WCDMA */

#define LGE_FACTORY_PID 0x6000
#define LGE_DEFAULT_PID 0x618E
/* #define LGE_PLATFORM_PID 0x618E */

enum lgeusb_mode {
	LGEUSB_FACTORY_MODE = 0,
	LGEUSB_ANDROID_MODE,
};

struct lgeusb_info {
	uint32_t restore_pid;
	enum lgeusb_mode current_mode;
	void (*switch_func)(uint32_t pid, uint32_t need_reset);
	uint32_t (*get_pid)(void);
};

int lgeusb_detect_factory_cable(void);
int lgeusb_set_config(uint32_t pid, char *serialno);
void lgeusb_register_usbinfo(struct lgeusb_info *info);

void lgeusb_switch_factory_mode(uint32_t need_reset);
void lgeusb_switch_android_mode(uint32_t need_reset);
int lgeusb_get_current_mode(void);
void lgeusb_backup_pid(void);

#endif /* __U_LGEUSB_H__ */
