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

#endif /* __U_LGE_USB_H__ */
