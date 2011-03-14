/*
 * Gadget Driver for LGE Android charge only feature.
 *
 * Copyright (C) 2011 LG Electronics Inc.
 * Author: Hyeon H. Park <hyunhui.park@lge.com>
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

/* This function driver is implemented for charge only feature of
 * LGE Android USB connection mode. This function do nothing about
 * the real USB data transfer. It is for only interface between Android
 * platform and linux android gadget driver.
 * 2011-03-11, hyunhui.park@lge.com
 */

#define DEBUG
#define VERBOSE_DEBUG

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>

#include <linux/types.h>
#include <linux/device.h>

#include <linux/usb/android_composite.h>

#include "u_lgeusb.h"

static const char f_name[] = "charge_only";

struct charge_only_dev {
	struct usb_function function;
	struct usb_composite_dev *cdev;
	spinlock_t lock;
};

/* temporary variable used between adb_open() and adb_gadget_bind() */
static struct charge_only_dev *_charge_only_dev;

static inline struct charge_only_dev *func_to_dev(struct usb_function *f)
{
	return container_of(f, struct charge_only_dev, function);
}

static int
charge_only_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct charge_only_dev	*dev = func_to_dev(f);

	dev->cdev = cdev;
	DBG(cdev, "charge_only_function_bind dev: %p\n", dev);

	/* do nothing special */

	return 0;
}

static void
charge_only_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	kfree(_charge_only_dev);
	_charge_only_dev = NULL;
}

static int charge_only_function_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "charge_only_function_set_alt intf: %d alt: %d\n", intf, alt);

	/* do nothing special */

	return 0;
}

static void charge_only_function_disable(struct usb_function *f)
{
	struct charge_only_dev	*dev = func_to_dev(f);
	struct usb_composite_dev	*cdev = dev->cdev;

	DBG(cdev, "charge_only_function_disable\n");

	/* do nothing special */

	VDBG(cdev, "%s disabled\n", dev->function.name);
}

static int charge_only_bind_config(struct usb_configuration *c)
{
	struct charge_only_dev *dev;
	int ret;

	pr_debug("charge_only_bind_config\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* As this function is just pseudo interface,
	 * It does not have any descriptors.
	 */
	dev->cdev = c->cdev;
	dev->function.name = f_name;
	dev->function.descriptors = NULL;
	dev->function.hs_descriptors = NULL;
	dev->function.bind = charge_only_function_bind;
	dev->function.unbind = charge_only_function_unbind;
	dev->function.set_alt = charge_only_function_set_alt;
	dev->function.disable = charge_only_function_disable;

	/* start disabled */
	dev->function.disabled = 1;

	_charge_only_dev = dev;

	ret = usb_add_function(c, &dev->function);
	if (ret)
		goto err1;

	return 0;

err1:
	kfree(dev);
	pr_err("charge only gadget driver failed to initialize\n");
	return ret;
}

static struct android_usb_function charge_only_function = {
	.name = "charge_only",
	.bind_config = charge_only_bind_config,
};

static int __init init(void)
{
	pr_debug("f_charge_only init\n");
	android_register_function(&charge_only_function);
	return 0;
}
module_init(init);
