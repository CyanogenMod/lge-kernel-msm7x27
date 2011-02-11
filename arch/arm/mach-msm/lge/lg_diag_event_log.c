/*
 *  arch/arm/mach-msm/lge/lge_diag_event_log.c
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

#define DRIVER_NAME "diag_event_log"

static struct input_dev *ats_input_dev;
static struct input_handler input_handler;
extern uint8_t lgf_factor_key_test_rsp(char);

int diag_log_status = 0;

/* TODO :  need to modify key map for each model */
#define ETA_KEY_MAX     8

/* key list for VS660 */
int diag_key_list[ETA_KEY_MAX]={
	/* thunder keypad key */
	KEY_MENU,
	KEY_HOME,
	KEY_VOLUMEUP,
	KEY_SEARCH,
	KEY_BACK,
	KEY_VOLUMEDOWN,
	/* 7k_handset key */
	KEY_MEDIA,
	KEY_END,
};

static int diag_event_log_connect(struct input_handler *handler,struct input_dev *dev,const struct input_device_id *id)
{
	int i;
	int ret;
	struct input_handle *handle;
	printk(" connect () %s \n\n",dev->name);

	for (i = 0 ; i < ETA_KEY_MAX - 1 ; i++){
		if (!test_bit(diag_key_list[i], dev->keybit))
			continue;
	}
	
	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if(!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "event_log";
	handle->private = NULL;

	ret = input_register_handle(handle);
	if (ret)
		goto err_input_register_handle;

	ret = input_open_device(handle);
	if (ret)
		goto err_input_open_device;

	return 0;
err_input_open_device:
	input_unregister_handle(handle);
err_input_register_handle:
	kfree(handle);
	return ret;
}

static void diag_event_log_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}


static const struct input_device_id diag_event_log_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};

static void diag_event_log_event(struct input_handle *handle, unsigned int type,unsigned int code, int value)
{
	if ( (type == EV_KEY)&& (value == 1) ){
		lgf_factor_key_test_rsp((uint8_t)code);
		printk ("->> diag_event_log_event  code is [%x]\n",code);
	}
}

int diag_event_log_start(void)
{
	int ret = 0;

	if(diag_log_status == 0){
		input_handler.name = "diag_key_log";
		input_handler.connect = diag_event_log_connect;
		input_handler.disconnect = diag_event_log_disconnect;
		input_handler.event = diag_event_log_event;
		input_handler.id_table = diag_event_log_ids;
		ret = input_register_handler(&input_handler);
		if (ret != 0)
			printk("%s:fail to registers input handler\n", __func__);

		diag_log_status	= 1;
	}

	return 0;
}
EXPORT_SYMBOL(diag_event_log_start);

int diag_event_log_end(void)
{
	if(diag_log_status==1){
		input_unregister_handler(&input_handler);
		printk("call diag_event_log_end \n");
		diag_log_status = 0;
	}
	return 0 ;
}
EXPORT_SYMBOL(diag_event_log_end);

static int  __init diag_event_log_probe(struct platform_device *pdev)
{
	int rc = 0 ;
	return rc;
}

static int diag_event_log_remove(struct platform_device *pdev)
{
	input_unregister_device(ats_input_dev);
	return 0;
}

static struct platform_driver diag_input_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.remove = diag_event_log_remove,
};

static int __init diag_input_init(void)
{
	return platform_driver_probe(&diag_input_driver, diag_event_log_probe);
}


static void __exit diag_input_exit(void)
{
	platform_driver_unregister(&diag_input_driver);
}

module_init(diag_input_init);
module_exit(diag_input_exit);

MODULE_AUTHOR("LG Electronics Inc.");
MODULE_DESCRIPTION("DIAG_EVENT_LOG driver");
MODULE_LICENSE("GPL v2");
