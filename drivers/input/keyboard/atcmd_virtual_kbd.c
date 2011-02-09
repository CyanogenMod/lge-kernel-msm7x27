/*
 *  drivers/input/keyboard/atcmd_virtual_kbd.c
 *
 *  This driver is made for virtual keyboard of AT COMMAND. ynj.kim@lge.com
 *
 *  Copyright (c) 2010 LGE.
 *  
 *  All source code in this file is licensed under the following license
 *  except where indicated.
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, you can find it at http://www.fsf.org
 */

#include <linux/platform_device.h>
#include <linux/input.h>
#include <mach/board_lge.h>

#define DRIVER_VERSION "v1.0"
#define KEY_DRIVER_NAME "atcmd_virtual_kbd"

static struct input_dev *atcmd_virtual_kbd_dev;
static struct atcmd_virtual_platform_data *atcmd_virtual_pdata;

static int atcmd_virtual_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int atcmd_virtual_resume(struct platform_device *pdev)
{
	return 0;
}

static int  __init atcmd_virtual_probe(struct platform_device *pdev)
{
	int rc;
	int key_idx;
	unsigned keycode = KEY_UNKNOWN;

	printk("[YJ] %s, start!!", __FUNCTION__);

	if (!pdev || !pdev->dev.platform_data) {
		printk(KERN_ERR"%s : pdev or platform data is null\n", __func__);
		return -ENODEV;
	}
	atcmd_virtual_pdata = pdev->dev.platform_data;

	atcmd_virtual_kbd_dev = input_allocate_device();
	if (!atcmd_virtual_kbd_dev) {
		printk(KERN_ERR "%s: not enough memory for input device\n", __func__);
		return -ENOMEM;
	}

	atcmd_virtual_kbd_dev->name = KEY_DRIVER_NAME;
	atcmd_virtual_kbd_dev->phys = "atcmd_virtual/input1";
	atcmd_virtual_kbd_dev->id.bustype = BUS_HOST;
	atcmd_virtual_kbd_dev->id.vendor = 0x0001;
	atcmd_virtual_kbd_dev->id.product = 0x0001;
	atcmd_virtual_kbd_dev->id.version = 0x0100;
	atcmd_virtual_kbd_dev->dev.parent = &pdev->dev;
	atcmd_virtual_kbd_dev->evbit[0] = BIT_MASK(EV_KEY);

	atcmd_virtual_kbd_dev->keycode = atcmd_virtual_pdata->keycode;
	atcmd_virtual_kbd_dev->keycodesize = sizeof(unsigned short);
	atcmd_virtual_kbd_dev->keycodemax = atcmd_virtual_pdata->keypad_row * atcmd_virtual_pdata->keypad_col;
	atcmd_virtual_kbd_dev->mscbit[0] = 0;
	
	for (key_idx = 0; key_idx <= atcmd_virtual_kbd_dev->keycodemax; key_idx++) {
		keycode = atcmd_virtual_pdata->keycode[2 * key_idx];
		if (keycode != KEY_UNKNOWN)
				set_bit(keycode, atcmd_virtual_kbd_dev->keybit);
	}

	rc = input_register_device(atcmd_virtual_kbd_dev);
	if (rc)
		printk(KERN_ERR"%s : input_register_device failed\n", __func__);

	return rc;

	printk("[YJ] %s, end!", __FUNCTION__);
}

static struct platform_driver __refdata atcmd_virtual_kbd_driver = {
	.driver = {
		.name = KEY_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.suspend = atcmd_virtual_suspend,
	.resume  = atcmd_virtual_resume,
};

static int __init atcmd_virtual_init(void)
{
	return platform_driver_probe(&atcmd_virtual_kbd_driver, atcmd_virtual_probe);
}


static void __exit atcmd_virtual_exit(void)
{
	platform_driver_unregister(&atcmd_virtual_kbd_driver);
}

module_init(atcmd_virtual_init);
module_exit(atcmd_virtual_exit);

MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION("atcommand virtual keyboard driver");
MODULE_LICENSE("GPL v2");
