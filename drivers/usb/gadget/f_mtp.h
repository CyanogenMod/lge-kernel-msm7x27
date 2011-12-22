/*
 * Gadget Driver for Android MTP
 *
 * Copyright (C) 2009 Motorola, Inc.
 * Author:
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

#ifndef __F_MTP_H
#define __F_MTP_H

struct usb_function *mtp_function_enable(int enable, int id);
int mtp_function_add(struct usb_composite_dev *cdev,
			struct usb_configuration *c);
#if defined(CONFIG_USB_GADGET_LG_MTP_DRIVER)
int mtp_function_init(void);
void mtp_function_exit(void);
#endif

/* ioctl related */
#define MTP_EVENT_SIZE   28
struct mtp_event_data {
    unsigned char data[MTP_EVENT_SIZE];
};

#define MTP_IOC_MAGIC    'm'
#define MTP_IOC_MAXNR    10

#define MTP_IOC_EVENT  _IOW(MTP_IOC_MAGIC, 1, struct mtp_event_data)
#define MTP_IOC_SEND_ZLP         _IO(MTP_IOC_MAGIC, 2)
#define MTP_IOC_GET_EP_SIZE_IN   _IOR(MTP_IOC_MAGIC, 3, int)
#define MTP_IOC_GET_VENDOR_FLAG  _IOR(MTP_IOC_MAGIC, 4, int)
#define MTP_IOC_CANCEL_IO        _IO(MTP_IOC_MAGIC, 5)
#define MTP_IOC_DEVICE_RESET     _IO(MTP_IOC_MAGIC, 6)

#endif /* __F_MTP_H */
