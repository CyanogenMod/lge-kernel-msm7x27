/*
 *  arch/arm/mach-msm/lge/lge_ats_event_log.c
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

#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/lge_alohag_at.h>

#define DRIVER_NAME "ats_event_log"

static struct input_dev *ats_input_dev;

/* add ETA  key event logging for vs660 [younchan.kim 2010-05-31]*/
static struct input_handler input_handler;
static struct work_struct event_log_work;
struct ats_mtc_key_log_type ats_mtc_key_log1;

extern int ats_mtc_log_mask;
extern void ats_mtc_send_key_log_to_eta(struct ats_mtc_key_log_type *);

/* LGE_CHANGE
 * support MTC using diag port
 * 2010-07-11 taehung.kim@lge.com
 */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined(LG_FW_MTC)
extern unsigned char g_diag_mtc_check;
extern void mtc_send_key_log_data(struct ats_mtc_key_log_type* p_ats_mtc_key_log);
#endif

/* TODO :  need to modify key map for each model */
#define ETA_KEY_MAX     8
#define ETA_ABS_MAX	7


/* key list for VS660 & LGP-500 */
int eta_key_list[ETA_KEY_MAX]={
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

int eta_abs_event[ETA_ABS_MAX]={
	ABS_X,
	ABS_Y,
	ABS_Z,
	ABS_MT_TOUCH_MAJOR,
	ABS_MT_TOUCH_MINOR,
	ABS_MT_POSITION_X,
	ABS_MT_POSITION_Y,
};

//ACTION filed information
typedef enum{
	ETA_TOUCH_MOVETO = 0, /*Move the pointer to the specified location*/
	ETA_TOUCH_MOVEBY = 1, /*Move the pointer by the specified values*/
	ETA_TOUCH_TAB = 2, /*Tab at the current location*/
	ETA_TOUCH_DOUBLETAB = 3, /*Double tab at the current location*/
	ETA_TOUCH_DOWN = 4, /*Touch down at the current location*/
	ETA_TOUCH_UP = 5, /*Touch up at the current location*/
	ETA_TOUCH_DEFAULT = 0xff,
}eta_touch_event_action_type;
static char eta_prev_action = ETA_TOUCH_DEFAULT;

int touch_status = 0 ;


static int ats_event_log_connect(struct input_handler *handler,struct input_dev *dev,const struct input_device_id *id)
{
	int i;
	int ret;
	struct input_handle *handle;

	if(strcmp(dev->name, "ats_input") == 0)
		return 0;

	printk(" connect () %s \n\n",dev->name);

	for (i = 0 ; i < ETA_KEY_MAX - 1 ; i++){
		if (!test_bit(eta_key_list[i], dev->keybit))
			continue;
	}
	for (i = 0 ; i < ETA_ABS_MAX - 1 ; i++){
		if (!test_bit(eta_abs_event[i], dev->absbit))
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

static void ats_event_log_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}


static const struct input_device_id ats_event_log_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};


static void event_log_work_func(struct work_struct *work)
{
/* LGE_CHANGE
 * support MTC using diag port
 * 2010-07-11 taehung.kim@lge.com
 */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined(LG_FW_MTC)
	if(g_diag_mtc_check==1)
		mtc_send_key_log_data(&ats_mtc_key_log1);
	else
#endif
		ats_mtc_send_key_log_to_eta(&ats_mtc_key_log1);
}

static void ats_event_log_event(struct input_handle *handle, unsigned int type,unsigned int code, int value)
{
	if ( (type == EV_KEY) && (0x00000001 & ats_mtc_log_mask) ){
		ats_mtc_key_log1.log_id = 1; /* LOG_ID, 1 key, 2 touch */
		ats_mtc_key_log1.log_len = 18; /* LOG_LEN */
		ats_mtc_key_log1.x_hold = value; /* hold */
		ats_mtc_key_log1.y_code = code;
		schedule_work(&event_log_work);
	}
	else if ( (type == EV_ABS || type == EV_SYN) && (0x00000002 & ats_mtc_log_mask) ){
		switch(code){
			case ABS_MT_TOUCH_MAJOR:
				{
					touch_status++;
					if(value == 1){ /* value = 1 is touch pressed case */
						if (eta_prev_action == ETA_TOUCH_DOWN)
							ats_mtc_key_log1.action = (unsigned char)ETA_TOUCH_MOVETO;
						else
							ats_mtc_key_log1.action = (unsigned char)ETA_TOUCH_DOWN;

						eta_prev_action = ETA_TOUCH_DOWN;
					}
					else {
						ats_mtc_key_log1.action = (unsigned char)ETA_TOUCH_UP;
						eta_prev_action = ETA_TOUCH_UP;
					}
					break;
				}
			case ABS_MT_POSITION_X :
				{
					ats_mtc_key_log1.x_hold = value;
					touch_status++;
					break;
				}
			case ABS_MT_POSITION_Y:
				{
					ats_mtc_key_log1.y_code = value;
					touch_status++;
					break;
				}
		}
		if(touch_status == 3){
			ats_mtc_key_log1.log_id = 2; /* LOG_ID, 1 key, 2 touch */
			ats_mtc_key_log1.log_len = 22; /*LOG_LEN */
			touch_status = 0;
			schedule_work(&event_log_work);
		}
	}
}

int event_log_start(void)
{
	int ret = 0;

	input_handler.name = "key_log";
	input_handler.connect = ats_event_log_connect;
	input_handler.disconnect = ats_event_log_disconnect;
	input_handler.event = ats_event_log_event;
	input_handler.id_table = ats_event_log_ids;
	ret = input_register_handler(&input_handler);
	if (ret != 0)
		printk("%s:fail to registers input handler\n", __func__);

	INIT_WORK(&event_log_work,event_log_work_func);

	return 0;
}
EXPORT_SYMBOL(event_log_start);

int event_log_end(void)
{
	input_unregister_handler(&input_handler);
	return 0 ;
}
EXPORT_SYMBOL(event_log_end);

/* [END] add ETA  key event logging for vs660 [younchan.kim 2010-05-31]*/


static int  __init ats_event_log_probe(struct platform_device *pdev)
{
	int rc = 0 ;
	return rc;
}

static int ats_event_log_remove(struct platform_device *pdev)
{
	input_unregister_device(ats_input_dev);
	return 0;
}

static struct platform_driver __refdata ats_input_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.remove = ats_event_log_remove,
};

static int __init ats_input_init(void)
{
	return platform_driver_probe(&ats_input_driver, ats_event_log_probe);
}


static void __exit ats_input_exit(void)
{
	platform_driver_unregister(&ats_input_driver);
}

module_init(ats_input_init);
module_exit(ats_input_exit);

MODULE_AUTHOR("LG Electronics Inc.");
MODULE_DESCRIPTION("ATS_EVENT_LOG driver");
MODULE_LICENSE("GPL v2");
