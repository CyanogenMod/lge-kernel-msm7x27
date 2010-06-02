/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <linux/earlysuspend.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include <asm/atomic.h>

#include <mach/msm_rpcrouter.h>
#include <mach/msm_battery.h>

#define BATTERY_RPC_PROG	0x30000089
#define BATTERY_RPC_VERS	0x00010001

#define BATTERY_RPC_CB_PROG	0x31000089
#define BATTERY_RPC_CB_VERS	0x00010001

#define CHG_RPC_PROG		0x3000001a
#define CHG_RPC_VERS		0x00010001


#define BATTERY_REGISTER_PROC                          	2
#define BATTERY_GET_CLIENT_INFO_PROC                   	3
#define BATTERY_MODIFY_CLIENT_PROC                     	4
#define BATTERY_DEREGISTER_CLIENT_PROC			5
#define BATTERY_SERVICE_TABLES_PROC                    	6
#define BATTERY_IS_SERVICING_TABLES_ENABLED_PROC       	7
#define BATTERY_ENABLE_TABLE_SERVICING_PROC            	8
#define BATTERY_DISABLE_TABLE_SERVICING_PROC           	9
#define BATTERY_READ_PROC                              	10
#define BATTERY_MIMIC_LEGACY_VBATT_READ_PROC           	11
#define BATTERY_ENABLE_DISABLE_FILTER_PROC 		14

#define VBATT_FILTER			2

#define BATTERY_CB_TYPE_PROC 		1
#define BATTERY_CB_ID_ALL_ACTIV       	1
#define BATTERY_CB_ID_LOW_VOL		2

#define BATTERY_LOW            	3200
#define BATTERY_HIGH           	4200

#define BATTERY_LOW_CORECTION   	100

#define ONCRPC_CHG_IS_CHARGING_PROC 		2
#define ONCRPC_CHG_IS_CHARGER_VALID_PROC 	3
#define ONCRPC_CHG_IS_BATTERY_VALID_PROC 	4
#define ONCRPC_CHG_UI_EVENT_READ_PROC 		5
#define ONCRPC_CHG_GET_GENERAL_STATUS_PROC 	12
#define ONCRPC_CHARGER_API_VERSIONS_PROC 	0xffffffff

#define CHARGER_API_VERSION  			0x00010003

#define DEFAULT_CHARGER_API_VERSION		0x00010001


#define BATT_RPC_TIMEOUT    10000	/* 10 sec */

#define INVALID_BATT_HANDLE    -1

#define RPC_TYPE_REQ     0
#define RPC_TYPE_REPLY   1
#define RPC_REQ_REPLY_COMMON_HEADER_SIZE   (3 * sizeof(uint32_t))


#define SUSPEND_EVENT		(1UL << 0)
#define RESUME_EVENT		(1UL << 1)
#define CLEANUP_EVENT		(1UL << 2)


#define DEBUG  1

#if DEBUG
#define DBG(x...) printk(KERN_INFO x)
#else
#define DBG(x...) do {} while (0)
#endif


enum {
	BATTERY_REGISTRATION_SUCCESSFUL = 0,
	BATTERY_DEREGISTRATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_MODIFICATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_INTERROGATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_CLIENT_TABLE_FULL = 1,
	BATTERY_REG_PARAMS_WRONG = 2,
	BATTERY_DEREGISTRATION_FAILED = 4,
	BATTERY_MODIFICATION_FAILED = 8,
	BATTERY_INTERROGATION_FAILED = 16,
	/* Client's filter could not be set because perhaps it does not exist */
	BATTERY_SET_FILTER_FAILED         = 32,
	/* Client's could not be found for enabling or disabling the individual
	 * client */
	BATTERY_ENABLE_DISABLE_INDIVIDUAL_CLIENT_FAILED  = 64,
	BATTERY_LAST_ERROR = 128,
};

enum {
	BATTERY_VOLTAGE_UP = 0,
	BATTERY_VOLTAGE_DOWN,
	BATTERY_VOLTAGE_ABOVE_THIS_LEVEL,
	BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
	BATTERY_VOLTAGE_LEVEL,
	BATTERY_ALL_ACTIVITY,
	VBATT_CHG_EVENTS,
	BATTERY_VOLTAGE_UNKNOWN,
};

enum {
	CHG_UI_EVENT_IDLE,	/* Starting point, no charger.  */
	CHG_UI_EVENT_NO_POWER,	/* No/Weak Battery + Weak Charger. */
	CHG_UI_EVENT_VERY_LOW_POWER,	/* No/Weak Battery + Strong Charger. */
	CHG_UI_EVENT_LOW_POWER,	/* Low Battery + Strog Charger.  */
	CHG_UI_EVENT_NORMAL_POWER, /* Enough Power for most applications. */
	CHG_UI_EVENT_DONE,	/* Done charging, batt full.  */
	CHG_UI_EVENT_INVALID,
	CHG_UI_EVENT_MAX32 = 0x7fffffff
};

/*
 * This enum contains defintions of the charger hardware status
 */
enum chg_charger_status_type {
    /* The charger is good      */
    CHARGER_STATUS_GOOD,
    /* The charger is bad       */
    CHARGER_STATUS_BAD,
    /* The charger is weak      */
    CHARGER_STATUS_WEAK,
    /* Invalid charger status.  */
    CHARGER_STATUS_INVALID
};

static char *charger_status[] = {
	"good", "bad", "weak", "invalid"
};

/*
 *This enum contains defintions of the charger hardware type
 */
enum chg_charger_hardware_type {
    /* The charger is removed                 */
    CHARGER_TYPE_NONE,
    /* The charger is a regular wall charger   */
    CHARGER_TYPE_WALL,
    /* The charger is a PC USB                 */
    CHARGER_TYPE_USB_PC,
    /* The charger is a wall USB charger       */
    CHARGER_TYPE_USB_WALL,
    /* The charger is a USB carkit             */
    CHARGER_TYPE_USB_CARKIT,
    /* Invalid charger hardware status.        */
    CHARGER_TYPE_INVALID
};

static char *charger_type[] = {
	"No charger", "wall", "USB PC", "USB wall", "USB car kit",
	"invalid charger"
};

/*
 *  This enum contains defintions of the battery status
 */
enum chg_battery_status_type {
    /* The battery is good        */
    BATTERY_STATUS_GOOD,
    /* The battery is cold/hot    */
    BATTERY_STATUS_BAD_TEMP,
    /* The battery is bad         */
    BATTERY_STATUS_BAD,
    /* Invalid battery status.    */
    BATTERY_STATUS_INVALID
};

static char *battery_status[] = {
	"good ", "bad temperature", "bad", "invalid"
};


/*
 *This enum contains defintions of the battery voltage level
 */
enum chg_battery_level_type {
    /* The battery voltage is dead/very low (less than 3.2V)        */
    BATTERY_LEVEL_DEAD,
    /* The battery voltage is weak/low (between 3.2V and 3.4V)      */
    BATTERY_LEVEL_WEAK,
    /* The battery voltage is good/normal(between 3.4V and 4.2V)  */
    BATTERY_LEVEL_GOOD,
    /* The battery voltage is up to full (close to 4.2V)            */
    BATTERY_LEVEL_FULL,
    /* Invalid battery voltage level.                               */
    BATTERY_LEVEL_INVALID
};

static char *battery_level[] = {
	"dead", "weak", "good", "full", "invalid"
};


/* Generic type definition used to enable/disable charger functions */
enum {
	CHG_CMD_DISABLE,
	CHG_CMD_ENABLE,
	CHG_CMD_INVALID,
	CHG_CMD_MAX32 = 0x7fffffff
};

struct batt_client_registration_req {

	struct rpc_request_hdr hdr;

	/* The voltage at which callback (CB) should be called. */
	u32 desired_batt_voltage;

	/* The direction when the CB should be called. */
	u32 voltage_direction;

	/* The registered callback to be called when voltage and
	 * direction specs are met. */
	u32 batt_cb_id;

	/* The call back data */
	u32 cb_data;
	u32 more_data;
	u32 batt_error;
};

struct batt_client_registration_rep {
	struct rpc_reply_hdr hdr;
	u32 batt_clnt_handle;
};

struct rpc_reply_batt_chg {
	struct rpc_reply_hdr hdr;
	u32 	more_data;

	u32	charger_status;
	u32	charger_type;
	u32	battery_status;
	u32	battery_level;
	u32     battery_voltage;
	u32	battery_temp;
	u32	chg_current;
};

static struct rpc_reply_batt_chg rep_batt_chg;

struct msm_battery_info {

	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 chg_api_version;
	u32 batt_technology;

	u32 avail_chg_sources;
	u32 current_chg_source;

	u32 batt_status;
	u32 batt_health;
	u32 charger_valid;
	u32 batt_valid;
	u32 batt_capacity;

	u32 chg_current;
	s32 charger_status;
	s32 charger_type;
	s32 battery_status;
	s32 battery_level;
	s32 battery_voltage;
	s32 battery_temp;

	u32(*calculate_capacity) (u32 voltage);

	s32 batt_handle;

	spinlock_t lock;

	struct power_supply *msm_psy_ac;
	struct power_supply *msm_psy_usb;
	struct power_supply *msm_psy_batt;

	struct msm_rpc_endpoint *batt_ep;
	struct msm_rpc_endpoint *chg_ep;

	struct workqueue_struct *msm_batt_wq;

	struct task_struct *cb_thread;

	wait_queue_head_t wait_q;

	struct early_suspend early_suspend;

	atomic_t handle_event;
	atomic_t event_handled;

	u32 type_of_event;
	uint32_t vbatt_rpc_req_xid;
};

static void msm_batt_wait_for_batt_chg_event(struct work_struct *work);

static DECLARE_WORK(msm_batt_cb_work, msm_batt_wait_for_batt_chg_event);

static int msm_batt_cleanup(void);

static struct msm_battery_info msm_batt_info = {
	.batt_handle = INVALID_BATT_HANDLE,
	.charger_status = -1,
	.charger_type = -1,
	.battery_status = -1,
	.battery_level = -1,
	.battery_voltage = -1,
	.battery_temp = -1,
};

static enum power_supply_property msm_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *msm_power_supplied_to[] = {
	"battery",
};

static int msm_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:

		if (psy->type == POWER_SUPPLY_TYPE_MAINS) {

			val->intval = msm_batt_info.current_chg_source & AC_CHG
			    ? 1 : 0;
			printk(KERN_INFO "%s(): power supply = %s online = %d\n"
					, __func__, psy->name, val->intval);

		}

		if (psy->type == POWER_SUPPLY_TYPE_USB) {

			val->intval = msm_batt_info.current_chg_source & USB_CHG
			    ? 1 : 0;

			printk(KERN_INFO "%s(): power supply = %s online = %d\n"
					, __func__, psy->name, val->intval);
		}

		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_ac = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static struct power_supply msm_psy_usb = {
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};


static void msm_batt_update_psy_status_v0(void);
static void msm_batt_update_psy_status_v1(void);

static void msm_batt_external_power_changed(struct power_supply *psy)
{
	printk(KERN_INFO "%s() : external power supply changed for %s\n",
			__func__, psy->name);
	power_supply_changed(psy);
}

static int msm_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	switch (psp) {

	case POWER_SUPPLY_PROP_STATUS:
		val->intval = msm_batt_info.batt_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = msm_batt_info.batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = msm_batt_info.batt_valid;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = msm_batt_info.batt_technology;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = msm_batt_info.voltage_max_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = msm_batt_info.voltage_min_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = msm_batt_info.battery_voltage;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = msm_batt_info.batt_capacity;
		break;
    case POWER_SUPPLY_PROP_TEMP:
       	val->intval = msm_batt_info.battery_temp;
		break;
    case POWER_SUPPLY_PROP_CURRENT_NOW:
       	val->intval = msm_batt_info.chg_current;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_batt = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = msm_batt_power_props,
	.num_properties = ARRAY_SIZE(msm_batt_power_props),
	.get_property = msm_batt_power_get_property,
	.external_power_changed = msm_batt_external_power_changed,
};


static int msm_batt_get_batt_chg_status_v1(void)
{
	int rc ;
	struct rpc_req_batt_chg {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_batt_chg;

	req_batt_chg.more_data = cpu_to_be32(1);

	memset(&rep_batt_chg, 0, sizeof(rep_batt_chg));

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_GET_GENERAL_STATUS_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_batt_chg, sizeof(rep_batt_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_GET_GENERAL_STATUS_PROC, rc);
		return rc;
	} else if (be32_to_cpu(rep_batt_chg.more_data)) {

		rep_batt_chg.charger_status =
			be32_to_cpu(rep_batt_chg.charger_status);

		rep_batt_chg.charger_type =
			be32_to_cpu(rep_batt_chg.charger_type);

		rep_batt_chg.battery_status =
			be32_to_cpu(rep_batt_chg.battery_status);

		rep_batt_chg.battery_level =
			be32_to_cpu(rep_batt_chg.battery_level);

		rep_batt_chg.battery_voltage =
			be32_to_cpu(rep_batt_chg.battery_voltage);

		rep_batt_chg.battery_temp =
			be32_to_cpu(rep_batt_chg.battery_temp);
		
		rep_batt_chg.chg_current =
			be32_to_cpu(rep_batt_chg.chg_current);

		printk(KERN_INFO "charger_status = %s, charger_type = %s,"
				" batt_status = %s, batt_level = %s,"
				" batt_volt = %u, batt_temp = %u,\n",
				charger_status[rep_batt_chg.charger_status],
				charger_type[rep_batt_chg.charger_type],
				battery_status[rep_batt_chg.battery_status],
				battery_level[rep_batt_chg.battery_level],
				rep_batt_chg.battery_voltage,
				rep_batt_chg.battery_temp);

	} else {
		printk(KERN_INFO "%s():No more data in batt_chg rpc reply\n",
				__func__);
		return -EIO;
	}

	return 0;
}

static void msm_batt_update_psy_status_v1(void)
{
	msm_batt_get_batt_chg_status_v1();

	if (msm_batt_info.charger_status == rep_batt_chg.charger_status &&
		msm_batt_info.charger_type == rep_batt_chg.charger_type &&
		msm_batt_info.battery_status ==  rep_batt_chg.battery_status &&
		msm_batt_info.battery_level ==  rep_batt_chg.battery_level &&
		msm_batt_info.battery_voltage  == rep_batt_chg.battery_voltage
		&& msm_batt_info.battery_temp ==  rep_batt_chg.battery_temp) {

		printk(KERN_NOTICE "%s() : Got unnecessary event from Modem "
			"PMIC VBATT driver. Nothing changed in Battery or "
			"charger status\n", __func__);
		return;
	}
	msm_batt_info.battery_voltage    =	rep_batt_chg.battery_voltage >>16;

	msm_batt_info.battery_temp =    rep_batt_chg.battery_temp *10;  // battery service use 0.1 C resolustion

	if (rep_batt_chg.battery_status != BATTERY_STATUS_INVALID) {

		msm_batt_info.batt_valid = 1;
		if (rep_batt_chg.battery_status == BATTERY_STATUS_BAD) {
			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_DEAD;
			msm_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		} else if (rep_batt_chg.battery_status ==
				BATTERY_STATUS_BAD_TEMP) {

			msm_batt_info.batt_health =
				POWER_SUPPLY_HEALTH_OVERHEAT;

			if (rep_batt_chg.charger_status == CHARGER_STATUS_BAD
				|| rep_batt_chg.charger_status ==
				CHARGER_STATUS_INVALID)
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_UNKNOWN;
			else
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_NOT_CHARGING;

		} else if ((rep_batt_chg.charger_status == CHARGER_STATUS_GOOD
			|| rep_batt_chg.charger_status == CHARGER_STATUS_WEAK)
			&& (rep_batt_chg.battery_status ==
				BATTERY_STATUS_GOOD)) {

			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;

			if (rep_batt_chg.battery_level == BATTERY_LEVEL_FULL)
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_FULL;
			else
				msm_batt_info.batt_status =
					POWER_SUPPLY_STATUS_CHARGING;

		} else if ((rep_batt_chg.charger_status == CHARGER_STATUS_BAD
				|| rep_batt_chg.charger_status ==
				CHARGER_STATUS_INVALID) &&
				(rep_batt_chg.battery_status ==
					BATTERY_STATUS_GOOD)) {

			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			msm_batt_info.batt_status =
				POWER_SUPPLY_STATUS_DISCHARGING;
		}
		msm_batt_info.batt_capacity =
			msm_batt_info.calculate_capacity(
					rep_batt_chg.battery_voltage);
	} else {
		msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		msm_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		msm_batt_info.batt_capacity = 0;
	}

	if (msm_batt_info.charger_type != rep_batt_chg.charger_type) {

		msm_batt_info.charger_type = rep_batt_chg.charger_type ;
	       /* WALL Charger means USB WALL charger in LG charger */
		if (msm_batt_info.charger_type == CHARGER_TYPE_USB_WALL) {
			msm_batt_info.current_chg_source &= ~USB_CHG;
			msm_batt_info.current_chg_source |= AC_CHG;

			printk(KERN_INFO "%s() : charger_type = WALL\n",
					__func__);

			power_supply_changed(&msm_psy_ac);
		} else if (msm_batt_info.charger_type == CHARGER_TYPE_USB_PC) {
			msm_batt_info.current_chg_source &= ~AC_CHG;
			msm_batt_info.current_chg_source |= USB_CHG;

			printk(KERN_INFO "%s() : charger_type = %s\n",
			__func__,
			charger_type[msm_batt_info.charger_type]);

			power_supply_changed(&msm_psy_usb);

		} else {

			printk(KERN_INFO "%s() : charger_type = %s\n",
			__func__,
			charger_type[msm_batt_info.charger_type]);

			msm_batt_info.batt_status =
				POWER_SUPPLY_STATUS_DISCHARGING;

			if (msm_batt_info.current_chg_source & AC_CHG) {

				msm_batt_info.current_chg_source &= ~AC_CHG;

				printk(KERN_INFO "%s() : AC WALL charger"
					" removed\n", __func__);

				power_supply_changed(&msm_psy_ac);

			} else if (msm_batt_info.current_chg_source & USB_CHG) {

				msm_batt_info.current_chg_source &= ~USB_CHG;
				printk(KERN_INFO "%s() : USB charger removed\n",
						__func__);

				power_supply_changed(&msm_psy_usb);
			} else
				power_supply_changed(&msm_psy_batt);
		}
	} else
		power_supply_changed(&msm_psy_batt);

	msm_batt_info.charger_status 	= 	rep_batt_chg.charger_status;
	msm_batt_info.charger_type 	= 	rep_batt_chg.charger_type;
	msm_batt_info.battery_status 	=  	rep_batt_chg.battery_status;
	msm_batt_info.battery_level 	=  	rep_batt_chg.battery_level;
	msm_batt_info.chg_current   =    rep_batt_chg.chg_current;
	
	DBG("[LG_FW] chg_status = %s, chg_type = %s, \n"
				"[LG_FW] batt_volt = %u, batt_temp = %u, batt_current = %u\n",
				charger_status[msm_batt_info.charger_status],
				charger_type[msm_batt_info.charger_type],
				msm_batt_info.battery_voltage, msm_batt_info.battery_temp, msm_batt_info.chg_current);
}


static int msm_batt_get_batt_chg_status_v0(u32 *batt_charging,
					u32 *charger_valid,
					u32 *chg_batt_event)
{
	struct rpc_request_hdr req_batt_chg;

	struct rpc_reply_batt_volt {
		struct rpc_reply_hdr hdr;
		u32 voltage;
	} rep_volt;

	struct rpc_reply_chg_reply {
		struct rpc_reply_hdr hdr;
		u32 chg_batt_data;
	} rep_chg;

	int rc;
	*batt_charging = 0;
	*chg_batt_event = CHG_UI_EVENT_INVALID;
	*charger_valid = 0;

	rc = msm_rpc_call_reply(msm_batt_info.batt_ep,
				BATTERY_READ_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_volt, sizeof(rep_volt),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, BATTERY_READ_PROC, rc);

		return rc;
	}
	msm_batt_info.battery_voltage = be32_to_cpu(rep_volt.voltage);

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_IS_CHARGING_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_IS_CHARGING_PROC, rc);
		return rc;
	}
	*batt_charging = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_IS_CHARGER_VALID_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_IS_CHARGER_VALID_PROC, rc);
		return rc;
	}
	*charger_valid = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_IS_BATTERY_VALID_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_IS_BATTERY_VALID_PROC, rc);
		return rc;
	}
	msm_batt_info.batt_valid = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_CHG_UI_EVENT_READ_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_CHG_UI_EVENT_READ_PROC, rc);
		return rc;
	}
	*chg_batt_event = be32_to_cpu(rep_chg.chg_batt_data);

	return 0;
}

static void msm_batt_update_psy_status_v0(void)
{
	u32 batt_charging = 0;
	u32 chg_batt_event = CHG_UI_EVENT_INVALID;
	u32 charger_valid = 0;

	msm_batt_get_batt_chg_status_v0(&batt_charging, &charger_valid,
				     &chg_batt_event);

	printk(KERN_INFO "batt_charging = %u  batt_valid = %u "
			" batt_volt = %u\n charger_valid = %u "
			" chg_batt_event = %u\n",
			batt_charging, msm_batt_info.batt_valid,
			msm_batt_info.battery_voltage,
			charger_valid, chg_batt_event);

	printk(KERN_INFO "Previous charger valid status = %u"
			"  current charger valid status = %u\n",
			msm_batt_info.charger_valid, charger_valid);

	if (msm_batt_info.charger_valid != charger_valid) {

		msm_batt_info.charger_valid = charger_valid;
		if (msm_batt_info.charger_valid)
			msm_batt_info.current_chg_source |= USB_CHG;
		else
			msm_batt_info.current_chg_source &= ~USB_CHG;
		power_supply_changed(&msm_psy_usb);
	}

	if (msm_batt_info.batt_valid) {

		if (msm_batt_info.battery_voltage >
		    msm_batt_info.voltage_max_design)
			msm_batt_info.batt_health =
			    POWER_SUPPLY_HEALTH_OVERVOLTAGE;

		else if (msm_batt_info.battery_voltage
			 < msm_batt_info.voltage_min_design)
			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_DEAD;
		else
			msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;

		if (batt_charging && msm_batt_info.charger_valid)
			msm_batt_info.batt_status =
			    POWER_SUPPLY_STATUS_CHARGING;
		else if (!batt_charging)
			msm_batt_info.batt_status =
			    POWER_SUPPLY_STATUS_DISCHARGING;

		if (chg_batt_event == CHG_UI_EVENT_DONE)
			msm_batt_info.batt_status = POWER_SUPPLY_STATUS_FULL;

		msm_batt_info.batt_capacity =
		    msm_batt_info.calculate_capacity(
				    msm_batt_info.battery_voltage);

	} else {
		msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		msm_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		msm_batt_info.batt_capacity = 0;
	}

	power_supply_changed(&msm_psy_batt);
}

static int msm_batt_register(u32 desired_batt_voltage,
			     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	struct batt_client_registration_req req;
	struct batt_client_registration_rep rep;
	int rc;

	req.desired_batt_voltage = cpu_to_be32(desired_batt_voltage);
	req.voltage_direction = cpu_to_be32(voltage_direction);
	req.batt_cb_id = cpu_to_be32(batt_cb_id);
	req.cb_data = cpu_to_be32(cb_data);
	req.more_data = cpu_to_be32(1);
	req.batt_error = cpu_to_be32(0);

	rc = msm_rpc_call_reply(msm_batt_info.batt_ep,
				BATTERY_REGISTER_PROC, &req,
				sizeof(req), &rep, sizeof(rep),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, BATTERY_REGISTER_PROC, rc);
		return rc;
	} else {
		rc = be32_to_cpu(rep.batt_clnt_handle);
		printk(KERN_INFO "batt_clnt_handle = %d\n", rc);
		return rc;
	}
}

static int msm_batt_modify_client(u32 client_handle, u32 desired_batt_voltage,
	     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	int rc;

	struct batt_modify_client_req {
		struct rpc_request_hdr hdr;

		u32 client_handle;

		/* The voltage at which callback (CB) should be called. */
		u32 desired_batt_voltage;

		/* The direction when the CB should be called. */
		u32 voltage_direction;

		/* The registered callback to be called when voltage and
		 * direction specs are met. */
		u32 batt_cb_id;

		/* The call back data */
		u32 cb_data;
	} req;

	req.client_handle = cpu_to_be32(client_handle);
	req.desired_batt_voltage = cpu_to_be32(desired_batt_voltage);
	req.voltage_direction = cpu_to_be32(voltage_direction);
	req.batt_cb_id = cpu_to_be32(batt_cb_id);
	req.cb_data = cpu_to_be32(cb_data);

	msm_rpc_setup_req(&req.hdr, BATTERY_RPC_PROG, BATTERY_RPC_VERS,
			 BATTERY_MODIFY_CLIENT_PROC);

	msm_batt_info.vbatt_rpc_req_xid = req.hdr.xid;

	rc = msm_rpc_write(msm_batt_info.batt_ep, &req, sizeof(req));

	if (rc < 0) {
		printk(KERN_ERR
		       "%s(): msm_rpc_write failed.  proc = 0x%08x rc = %d\n",
		       __func__, BATTERY_MODIFY_CLIENT_PROC, rc);
		return rc;
	}

	return 0;
}

static int msm_batt_deregister(u32 handle)
{
	int rc;
	struct batt_client_deregister_req {
		struct rpc_request_hdr req;
		s32 handle;
	} batt_deregister_rpc_req;

	struct batt_client_deregister_reply {
		struct rpc_reply_hdr hdr;
		u32 batt_error;
	} batt_deregister_rpc_reply;

	batt_deregister_rpc_req.handle = cpu_to_be32(handle);
	batt_deregister_rpc_reply.batt_error = cpu_to_be32(BATTERY_LAST_ERROR);

	rc = msm_rpc_call_reply(msm_batt_info.batt_ep,
				BATTERY_DEREGISTER_CLIENT_PROC,
				&batt_deregister_rpc_req,
				sizeof(batt_deregister_rpc_req),
				&batt_deregister_rpc_reply,
				sizeof(batt_deregister_rpc_reply),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, BATTERY_DEREGISTER_CLIENT_PROC, rc);
		return rc;
	}

	if (be32_to_cpu(batt_deregister_rpc_reply.batt_error) !=
			BATTERY_DEREGISTRATION_SUCCESSFUL) {

		printk(KERN_ERR "%s: vBatt deregistration Failed "
		       "  proce_num = %d,"
		       " batt_clnt_handle = %d\n",
		       __func__, BATTERY_DEREGISTER_CLIENT_PROC, handle);
		return -EIO;
	}
	return 0;
}

static int  msm_batt_handle_suspend(void)
{
	int rc;

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				BATTERY_LOW, BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
				BATTERY_CB_ID_LOW_VOL, BATTERY_LOW);

		if (rc < 0) {
			printk(KERN_ERR
			       "%s(): failed to modify client for registering"
			       " call back when  voltage goes below %u\n",
			       __func__, BATTERY_LOW);

			return rc;
		}
	}

	return 0;
}

static int  msm_batt_handle_resume(void)
{
	int rc;

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = msm_batt_modify_client(msm_batt_info.batt_handle,
				BATTERY_LOW, BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
		if (rc < 0) {
			printk(KERN_ERR
			       "%s(): failed to modify client for registering"
			       " call back for ALL activity \n", __func__);
			return rc;
		}
	}
	return 0;
}


static int  msm_batt_handle_event(void)
{
	int rc;

	if (!atomic_read(&msm_batt_info.handle_event)) {

		printk(KERN_ERR "%s(): batt call back thread while in "
			"msm_rpc_read got signal. Signal is not from "
			"early suspend or  from late resume or from Clean up "
			"thread.\n", __func__);
		return 0;
	}

	printk(KERN_INFO "%s(): batt call back thread while in msm_rpc_read "
			"got signal\n", __func__);

	if (msm_batt_info.type_of_event & SUSPEND_EVENT) {

		printk(KERN_INFO "%s(): Handle Suspend event. event = %08x\n",
				__func__, msm_batt_info.type_of_event);

		rc = msm_batt_handle_suspend();

		return rc;

	} else if (msm_batt_info.type_of_event & RESUME_EVENT) {

		printk(KERN_INFO "%s(): Handle Resume event. event = %08x\n",
				__func__, msm_batt_info.type_of_event);

		rc = msm_batt_handle_resume();

		return rc;

	} else if (msm_batt_info.type_of_event & CLEANUP_EVENT) {

		printk(KERN_INFO "%s(): Cleanup event occured. event = %08x\n",
				__func__, msm_batt_info.type_of_event);

		return 0;

	} else  {

		printk(KERN_ERR "%s(): Unknown event occured. event = %08x\n",
				__func__, msm_batt_info.type_of_event);
		return 0;
	}
}


static void msm_batt_handle_vbatt_rpc_reply(struct rpc_reply_hdr *reply)
{

	struct rpc_reply_vbatt_modify_client {
		struct rpc_reply_hdr hdr;
		u32 modify_client_result;
	} *rep_vbatt_modify_client;

	u32 modify_client_result;

	if (msm_batt_info.type_of_event & SUSPEND_EVENT) {
		printk(KERN_INFO "%s(): Suspend event. Got RPC REPLY for vbatt"
			" modify client RPC req. \n", __func__);
	} else if (msm_batt_info.type_of_event & RESUME_EVENT) {
		printk(KERN_INFO "%s(): Resume event. Got RPC REPLY for vbatt"
			" modify client RPC req. \n", __func__);
	}

	/* If an earlier call timed out, we could get the (no longer wanted)
	 * reply for it. Ignore replies that  we don't expect.
	 */
	if (reply->xid != msm_batt_info.vbatt_rpc_req_xid) {

		printk(KERN_ERR "%s(): returned RPC REPLY XID is not"
				" equall to VBATT RPC REQ XID \n", __func__);
		kfree(reply);

		return;
	}
	if (reply->reply_stat != RPCMSG_REPLYSTAT_ACCEPTED) {

		printk(KERN_ERR "%s(): reply_stat != "
			" RPCMSG_REPLYSTAT_ACCEPTED \n", __func__);
		kfree(reply);

		return;
	}

	if (reply->data.acc_hdr.accept_stat != RPC_ACCEPTSTAT_SUCCESS) {

		printk(KERN_ERR "%s(): reply->data.acc_hdr.accept_stat "
				" != RPCMSG_REPLYSTAT_ACCEPTED \n", __func__);
		kfree(reply);

		return;
	}

	rep_vbatt_modify_client =
		(struct rpc_reply_vbatt_modify_client *) reply;

	modify_client_result =
		be32_to_cpu(rep_vbatt_modify_client->modify_client_result);

	if (modify_client_result != BATTERY_MODIFICATION_SUCCESSFUL) {

		printk(KERN_ERR "%s() :  modify client failed."
			"modify_client_result  = %u\n", __func__,
			modify_client_result);
	} else {
		printk(KERN_INFO "%s() : modify client successful.\n",
				__func__);
	}

	kfree(reply);
}

static void msm_batt_wake_up_waiting_thread(u32 event)
{
	msm_batt_info.type_of_event &= ~event;

	atomic_set(&msm_batt_info.handle_event, 0);
	atomic_set(&msm_batt_info.event_handled, 1);
	wake_up(&msm_batt_info.wait_q);
}


static void msm_batt_wait_for_batt_chg_event(struct work_struct *work)
{
	void *rpc_packet;
	struct rpc_request_hdr *req;
	int rpc_packet_type;
	struct rpc_reply_hdr rpc_reply;
	int len;
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&msm_batt_info.lock, flags);
	msm_batt_info.cb_thread = current;
	spin_unlock_irqrestore(&msm_batt_info.lock, flags);

	printk(KERN_INFO "%s: Batt RPC call back thread started.\n", __func__);

	allow_signal(SIGCONT);

	printk(KERN_INFO "%s: First time Update Batt status without waiting for"
			" call back event from modem .\n", __func__);

	if (msm_batt_info.chg_api_version >= CHARGER_API_VERSION)
		msm_batt_update_psy_status_v1();
	else
		msm_batt_update_psy_status_v0();

	while (1) {

		rpc_packet = NULL;

		len = msm_rpc_read(msm_batt_info.batt_ep, &rpc_packet, -1, -1);

		if (len == -ERESTARTSYS) {

			flush_signals(current);

			rc = msm_batt_handle_event();

			if (msm_batt_info.type_of_event & CLEANUP_EVENT) {

				msm_batt_wake_up_waiting_thread(CLEANUP_EVENT);
				break;

			} else if (msm_batt_info.type_of_event &
					(SUSPEND_EVENT | RESUME_EVENT)) {

				if (rc < 0) {
					/*Could not sent VBATT modify rpc req */
					msm_batt_wake_up_waiting_thread(
						SUSPEND_EVENT | RESUME_EVENT);
				}
				/* Wait for RPC reply  for vbatt modify
				 * client RPC call. Then wake up suspend and
				 * resume threads.
				 */
				continue;
			}
		}

		printk(KERN_INFO "%s: Got some packet from modem Vbatt server\n"
				, __func__);

		if (len < 0) {
			printk(KERN_ERR "%s: msm_rpc_read failed while "
			       "waiting for call back packet. rc = %d\n",
			       __func__, len);
			continue;
		}

		if (len < RPC_REQ_REPLY_COMMON_HEADER_SIZE) {
			printk(KERN_ERR "%s: The pkt is neither req nor reply."
			       " len of pkt = %d\n", __func__, len);
			if (!rpc_packet)
				kfree(rpc_packet);
			continue;
		}

		req = (struct rpc_request_hdr *)rpc_packet;

		rpc_packet_type = be32_to_cpu(req->type);

		if (rpc_packet_type == RPC_TYPE_REPLY) {

			if (msm_batt_info.type_of_event &
				(SUSPEND_EVENT | RESUME_EVENT)) {

				msm_batt_handle_vbatt_rpc_reply(rpc_packet);

				msm_batt_wake_up_waiting_thread(
						SUSPEND_EVENT | RESUME_EVENT);
				printk(KERN_INFO "[LG_FW] %s: Update Batt status.\n", __func__);
				msm_batt_update_psy_status_v1();
			} else {
				printk(KERN_ERR "%s: Should not get rpc reply"
					" Type_of_packet = %d\n", __func__,
					rpc_packet_type);
				kfree(rpc_packet);
			}
			continue;
		}
		if (rpc_packet_type != RPC_TYPE_REQ) {
			printk(KERN_ERR "%s: Type_of_packet is neither req or"
			       " reply. Type_of_packet = %d\n",
			       __func__, rpc_packet_type);
			kfree(rpc_packet);
			continue;
		}

		req->type = be32_to_cpu(req->type);
		req->xid = be32_to_cpu(req->xid);
		req->rpc_vers = be32_to_cpu(req->rpc_vers);

		if (req->rpc_vers != 2) {
			printk(KERN_ERR "%s: incorrect rpc version = %d\n",
			       __func__, req->rpc_vers);
			kfree(rpc_packet);
			continue;
		}

		req->prog = be32_to_cpu(req->prog);
		if (req->prog != BATTERY_RPC_CB_PROG) {
			printk(KERN_ERR "%s: Invalid Prog number for rpc call"
			       " back req. prog number = %d\n",
			       __func__, req->prog);
			kfree(rpc_packet);
			continue;
		}

		req->procedure = be32_to_cpu(req->procedure);

		if (req->procedure != BATTERY_CB_TYPE_PROC) {
			printk(KERN_ERR "%s: Invalid procedure num  rpc call"
			       " back req. req->procedure = %d\n",
			       __func__, req->procedure);
			kfree(rpc_packet);
			continue;
		}

		rpc_reply.xid = cpu_to_be32(req->xid);
		rpc_reply.type = cpu_to_be32(RPC_TYPE_REPLY);
		rpc_reply.reply_stat = cpu_to_be32(RPCMSG_REPLYSTAT_ACCEPTED);
		rpc_reply.data.acc_hdr.accept_stat =
		    cpu_to_be32(RPC_ACCEPTSTAT_SUCCESS);
		rpc_reply.data.acc_hdr.verf_flavor = 0;
		rpc_reply.data.acc_hdr.verf_length = 0;

		len = msm_rpc_write(msm_batt_info.batt_ep,
				    &rpc_reply, sizeof(rpc_reply));
		if (len < 0)
			printk(KERN_ERR "%s: could not send rpc reply for"
			       " call back from  batt server."
			       " reply  write response %d\n", __func__, len);

		kfree(rpc_packet);

		printk(KERN_INFO "%s: Update Batt status.\n", __func__);

		if (msm_batt_info.chg_api_version >= CHARGER_API_VERSION)
			msm_batt_update_psy_status_v1();
		else
			msm_batt_update_psy_status_v0();
	}

	printk(KERN_INFO "%s: Batt RPC call back thread stopped.\n", __func__);
}

static int msm_batt_send_event(u32 type_of_event)
{
	int rc;
	unsigned long flags;

	rc = 0;

	spin_lock_irqsave(&msm_batt_info.lock, flags);


	if (type_of_event & SUSPEND_EVENT)
		printk(KERN_INFO "%s() : Suspend event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else if (type_of_event & RESUME_EVENT)
		printk(KERN_INFO "%s() : Resume event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else if (type_of_event & CLEANUP_EVENT)
		printk(KERN_INFO "%s() : Cleanup event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else {
		printk(KERN_ERR "%s() : Unknown event ocurred."
				"events = %08x\n", __func__, type_of_event);

		spin_unlock_irqrestore(&msm_batt_info.lock, flags);
		return -EIO;
	}

	msm_batt_info.type_of_event |=  type_of_event;

	if (msm_batt_info.cb_thread) {
		atomic_set(&msm_batt_info.handle_event, 1);
		send_sig(SIGCONT, msm_batt_info.cb_thread, 0);
		spin_unlock_irqrestore(&msm_batt_info.lock, flags);

		rc = wait_event_interruptible(msm_batt_info.wait_q,
			atomic_read(&msm_batt_info.event_handled));

		if (rc == -ERESTARTSYS) {

			printk(KERN_ERR "%s(): Suspend/Resume/cleanup thread "
				"got a signal while waiting for batt call back"
				" thread to finish\n", __func__);

		} else if (rc < 0) {

			printk(KERN_ERR "%s(): Suspend/Resume/cleanup thread "
				"wait returned error while waiting for batt "
				"call back thread to finish. rc = %d\n",
				__func__, rc);
		} else
			printk(KERN_INFO "%s(): Suspend/Resume/cleanup thread "
				"wait returned rc = %d\n", __func__, rc);

		atomic_set(&msm_batt_info.event_handled, 0);
	} else {
		printk(KERN_INFO "%s(): Battery call Back thread not Started.",
				__func__);

		atomic_set(&msm_batt_info.handle_event, 1);
		spin_unlock_irqrestore(&msm_batt_info.lock, flags);
	}

	return rc;
}

static void msm_batt_start_cb_thread(void)
{
	atomic_set(&msm_batt_info.handle_event, 0);
	atomic_set(&msm_batt_info.event_handled, 0);
	queue_work(msm_batt_info.msm_batt_wq, &msm_batt_cb_work);
}

static void msm_batt_early_suspend(struct early_suspend *h);

static int msm_batt_cleanup(void)
{
	int rc = 0;
	int rc_local;

	if (msm_batt_info.msm_batt_wq) {
		msm_batt_send_event(CLEANUP_EVENT);
		destroy_workqueue(msm_batt_info.msm_batt_wq);
	}

	if (msm_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = msm_batt_deregister(msm_batt_info.batt_handle);
		if (rc < 0)
			printk(KERN_ERR
			       "%s: msm_batt_deregister failed rc=%d\n",
			       __func__, rc);
	}
	msm_batt_info.batt_handle = INVALID_BATT_HANDLE;

	if (msm_batt_info.msm_psy_ac)
		power_supply_unregister(msm_batt_info.msm_psy_ac);

	if (msm_batt_info.msm_psy_usb)
		power_supply_unregister(msm_batt_info.msm_psy_usb);
	if (msm_batt_info.msm_psy_batt)
		power_supply_unregister(msm_batt_info.msm_psy_batt);

	if (msm_batt_info.batt_ep) {
		rc_local = msm_rpc_close(msm_batt_info.batt_ep);
		if (rc_local < 0) {
			printk(KERN_ERR
			       "%s: msm_rpc_close failed for batt_ep rc=%d\n",
			       __func__, rc_local);
			if (!rc)
				rc = rc_local;
		}
	}

	if (msm_batt_info.chg_ep) {
		rc_local = msm_rpc_close(msm_batt_info.chg_ep);
		if (rc_local < 0) {
			printk(KERN_ERR
			       "%s: msm_rpc_close failed for chg_ep rc=%d\n",
			       __func__, rc_local);
			if (!rc)
				rc = rc_local;
		}
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (msm_batt_info.early_suspend.suspend == msm_batt_early_suspend)
		unregister_early_suspend(&msm_batt_info.early_suspend);
#endif
	return rc;
}

#define MIN_BATT_CAPACITY 0
#define MAX_BATT_CAPACITY 95
static u32 msm_batt_capacity(u32 current_voltage)
{
       u16 scaled_batt_capacity = 0;
       u16 batt_capacity = (current_voltage & 0xFFFF);  // lower 2 bytes is capacity
       
       if(batt_capacity < MIN_BATT_CAPACITY)
       {
       	scaled_batt_capacity = 0 ;
       }
       else if(batt_capacity > MAX_BATT_CAPACITY)
       {
       	scaled_batt_capacity = 100;

       }else
             scaled_batt_capacity = (batt_capacity - MIN_BATT_CAPACITY)*100 /(MAX_BATT_CAPACITY - MIN_BATT_CAPACITY);

	return scaled_batt_capacity;  
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void msm_batt_early_suspend(struct early_suspend *h)
{
	int rc;

	printk(KERN_INFO "%s(): going to early suspend\n", __func__);

	rc = msm_batt_send_event(SUSPEND_EVENT);

	printk(KERN_INFO "%s(): Handled early suspend event."
	       " rc = %d\n", __func__, rc);
}

void msm_batt_late_resume(struct early_suspend *h)
{
	int rc;

	printk(KERN_INFO "%s(): going to resume\n", __func__);

	rc = msm_batt_send_event(RESUME_EVENT);

	printk(KERN_INFO "%s(): Handled Late resume event."
	       " rc = %d\n", __func__, rc);
}
#endif


static int msm_batt_enable_filter(u32 vbatt_filter)
{
	int rc;
	struct rpc_req_vbatt_filter {
		struct rpc_request_hdr hdr;
		u32 batt_handle;
		u32 enable_filter;
		u32 vbatt_filter;
	} req_vbatt_filter;

	struct rpc_rep_vbatt_filter {
		struct rpc_reply_hdr hdr;
		u32 filter_enable_result;
	} rep_vbatt_filter;

	req_vbatt_filter.batt_handle = cpu_to_be32(msm_batt_info.batt_handle);
	req_vbatt_filter.enable_filter = cpu_to_be32(1);
	req_vbatt_filter.vbatt_filter = cpu_to_be32(vbatt_filter);

	rc = msm_rpc_call_reply(msm_batt_info.batt_ep,
				BATTERY_ENABLE_DISABLE_FILTER_PROC,
				&req_vbatt_filter, sizeof(req_vbatt_filter),
				&rep_vbatt_filter, sizeof(rep_vbatt_filter),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
			"%s: msm_rpc_call_reply failed! proc = %d rc = %d\n",
			__func__, BATTERY_ENABLE_DISABLE_FILTER_PROC, rc);
		return rc;
	} else {
		rc =  be32_to_cpu(rep_vbatt_filter.filter_enable_result);

		if (rc != BATTERY_DEREGISTRATION_SUCCESSFUL) {
			printk(KERN_ERR "%s: vbatt Filter enable failed."
				" proc = %d  filter enable result = %d"
				" filter number = %d\n", __func__,
				BATTERY_ENABLE_DISABLE_FILTER_PROC, rc,
				vbatt_filter);
			return -EIO;
		}

		printk(KERN_INFO "%s: vbatt Filter enabled successfully.\n",
				__func__);
		return 0;
	}
}


static int __devinit msm_batt_probe(struct platform_device *pdev)
{
	int rc;
	struct msm_psy_batt_pdata *pdata = pdev->dev.platform_data;

	if (pdev->id != -1) {
		dev_err(&pdev->dev,
			"%s: MSM chipsets Can only support one"
			" battery ", __func__);
		return -EINVAL;
	}

	if (pdata->avail_chg_sources & AC_CHG) {
		rc = power_supply_register(&pdev->dev, &msm_psy_ac);
		if (rc < 0) {
			dev_err(&pdev->dev,
				"%s: power_supply_register failed"
				" rc = %d\n", __func__, rc);
			msm_batt_cleanup();
			return rc;
		}
		msm_batt_info.msm_psy_ac = &msm_psy_ac;
		msm_batt_info.avail_chg_sources |= AC_CHG;
	}

	if (pdata->avail_chg_sources & USB_CHG) {
		rc = power_supply_register(&pdev->dev, &msm_psy_usb);
		if (rc < 0) {
			dev_err(&pdev->dev,
				"%s: power_supply_register failed"
				" rc = %d\n", __func__, rc);
			msm_batt_cleanup();
			return rc;
		}
		msm_batt_info.msm_psy_usb = &msm_psy_usb;
		msm_batt_info.avail_chg_sources |= USB_CHG;
	}

	if (!msm_batt_info.msm_psy_ac && !msm_batt_info.msm_psy_usb) {

		dev_err(&pdev->dev,
			"%s: No external Power supply(AC or USB)"
			"is avilable\n", __func__);
		msm_batt_cleanup();
		return -ENODEV;
	}

	msm_batt_info.voltage_max_design = pdata->voltage_max_design;
	msm_batt_info.voltage_min_design = pdata->voltage_min_design;
	msm_batt_info.batt_technology = pdata->batt_technology;
	msm_batt_info.calculate_capacity = pdata->calculate_capacity;

	if (!msm_batt_info.voltage_min_design)
		msm_batt_info.voltage_min_design = BATTERY_LOW;
	if (!msm_batt_info.voltage_max_design)
		msm_batt_info.voltage_max_design = BATTERY_HIGH;

	if (msm_batt_info.batt_technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		msm_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;

	if (!msm_batt_info.calculate_capacity)
		msm_batt_info.calculate_capacity = msm_batt_capacity;

	rc = power_supply_register(&pdev->dev, &msm_psy_batt);
	if (rc < 0) {
		dev_err(&pdev->dev, "%s: power_supply_register failed"
			" rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.msm_psy_batt = &msm_psy_batt;

	rc = msm_batt_register(BATTERY_LOW, BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_register failed rc=%d\n", __func__, rc);
		msm_batt_cleanup();
		return rc;
	}
	msm_batt_info.batt_handle = rc;

	rc =  msm_batt_enable_filter(VBATT_FILTER);

#ifdef CONFIG_HAS_EARLYSUSPEND
	msm_batt_info.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	msm_batt_info.early_suspend.suspend = msm_batt_early_suspend;
	msm_batt_info.early_suspend.resume = msm_batt_late_resume;
	register_early_suspend(&msm_batt_info.early_suspend);
#endif
	msm_batt_start_cb_thread();

	return 0;
}


int msm_batt_get_charger_api_version(void)
{
	int rc ;
	struct rpc_reply_hdr *reply;

	struct rpc_req_chg_api_ver {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_chg_api_ver;

	struct rpc_rep_chg_api_ver {
		struct rpc_reply_hdr hdr;
		u32 num_of_chg_api_versions;
		u32 *chg_api_versions;
	};

	u32 num_of_versions;

	struct rpc_rep_chg_api_ver *rep_chg_api_ver;


	req_chg_api_ver.more_data = cpu_to_be32(1);

	msm_rpc_setup_req(&req_chg_api_ver.hdr, CHG_RPC_PROG, CHG_RPC_VERS,
			 ONCRPC_CHARGER_API_VERSIONS_PROC);

	rc = msm_rpc_write(msm_batt_info.chg_ep, &req_chg_api_ver,
			sizeof(req_chg_api_ver));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s(): msm_rpc_write failed.  proc = 0x%08x rc = %d\n",
		       __func__, ONCRPC_CHARGER_API_VERSIONS_PROC, rc);
		return rc;
	}

	for (;;) {
		rc = msm_rpc_read(msm_batt_info.chg_ep, (void *) &reply, -1,
				BATT_RPC_TIMEOUT);
		if (rc < 0)
			return rc;
		if (rc < RPC_REQ_REPLY_COMMON_HEADER_SIZE) {
			printk(KERN_ERR "%s(): msm_rpc_read failed. read"
					" returned invalid packet which is"
					" neither rpc req nor rpc reply. "
					"legnth of packet = %d\n",
					__func__, rc);

			rc = -EIO;
			break;
		}
		/* we should not get RPC REQ or call packets -- ignore them */
		if (reply->type == RPC_TYPE_REQ) {

			printk(KERN_ERR "%s(): returned RPC REQ packets while"
				" waiting for RPC REPLY replay read \n",
				__func__);
			kfree(reply);
			continue;
		}

		/* If an earlier call timed out, we could get the (no
		 * longer wanted) reply for it.	 Ignore replies that
		 * we don't expect
		 */
		if (reply->xid != req_chg_api_ver.hdr.xid) {

			printk(KERN_ERR "%s(): returned RPC REPLY XID is not"
					" equall to RPC REQ XID \n", __func__);
			kfree(reply);
			continue;
		}
		if (reply->reply_stat != RPCMSG_REPLYSTAT_ACCEPTED) {
			rc = -EPERM;
			break;
		}
		if (reply->data.acc_hdr.accept_stat !=
				RPC_ACCEPTSTAT_SUCCESS) {
			rc = -EINVAL;
			break;
		}

		rep_chg_api_ver = (struct rpc_rep_chg_api_ver *)reply;

		num_of_versions =
			be32_to_cpu(rep_chg_api_ver->num_of_chg_api_versions);

		rep_chg_api_ver->chg_api_versions =  (u32 *)
			((u8 *) reply + sizeof(struct rpc_reply_hdr) +
			sizeof(rep_chg_api_ver->num_of_chg_api_versions));

		rc = be32_to_cpu(
			rep_chg_api_ver->chg_api_versions[num_of_versions - 1]);

		printk(KERN_INFO "%s(): num_of_chg_api_versions = %u"
			"  The chg api version = 0x%08x\n", __func__,
			num_of_versions, rc);
		break;
	}
	kfree(reply);
	return rc;
}


static struct platform_driver msm_batt_driver;
static int __devinit msm_batt_init_rpc(void)
{
	int rc;

	spin_lock_init(&msm_batt_info.lock);

	msm_batt_info.msm_batt_wq =
	    create_singlethread_workqueue("msm_battery");

	if (!msm_batt_info.msm_batt_wq) {
		printk(KERN_ERR "%s: create workque failed \n", __func__);
		return -ENOMEM;
	}

	msm_batt_info.batt_ep =
	    msm_rpc_connect_compatible(BATTERY_RPC_PROG, BATTERY_RPC_VERS, 0);

	if (msm_batt_info.batt_ep == NULL) {
		return -ENODEV;
	} else if (IS_ERR(msm_batt_info.batt_ep)) {
		int rc = PTR_ERR(msm_batt_info.batt_ep);
		printk(KERN_ERR
		       "%s: rpc connect failed for BATTERY_RPC_PROG."
		       " rc = %d\n ", __func__, rc);
		msm_batt_info.batt_ep = NULL;
		return rc;
	}
	msm_batt_info.chg_ep =
	    msm_rpc_connect_compatible(CHG_RPC_PROG, CHG_RPC_VERS, 0);

	if (msm_batt_info.chg_ep == NULL) {
		return -ENODEV;
	} else if (IS_ERR(msm_batt_info.chg_ep)) {
		int rc = PTR_ERR(msm_batt_info.chg_ep);
		printk(KERN_ERR
		       "%s: rpc connect failed for CHG_RPC_PROG. rc = %d\n",
		       __func__, rc);
		msm_batt_info.chg_ep = NULL;
		return rc;
	}

	msm_batt_info.chg_api_version =  msm_batt_get_charger_api_version();

	if (msm_batt_info.chg_api_version < 0)
		msm_batt_info.chg_api_version = DEFAULT_CHARGER_API_VERSION;

	rc = platform_driver_register(&msm_batt_driver);

	if (rc < 0) {
		printk(KERN_ERR "%s: platform_driver_register failed for "
			"batt driver. rc = %d\n", __func__, rc);
		return rc;
	}

	init_waitqueue_head(&msm_batt_info.wait_q);

	return 0;
}

static int __devexit msm_batt_remove(struct platform_device *pdev)
{
	int rc;
	rc = msm_batt_cleanup();

	if (rc < 0) {
		dev_err(&pdev->dev,
			"%s: msm_batt_cleanup  failed rc=%d\n", __func__, rc);
		return rc;
	}
	return 0;
}

static struct platform_driver msm_batt_driver = {
	.probe = msm_batt_probe,
	.remove = __devexit_p(msm_batt_remove),
	.driver = {
		   .name = "msm-battery",
		   .owner = THIS_MODULE,
		   },
};

static int __init msm_batt_init(void)
{
	int rc;

	rc = msm_batt_init_rpc();

	if (rc < 0) {
		printk(KERN_ERR "%s: msm_batt_init_rpc Failed  rc=%d\n",
		       __func__, rc);
		msm_batt_cleanup();
		return rc;
	}

	return 0;
}

static void __exit msm_batt_exit(void)
{
	platform_driver_unregister(&msm_batt_driver);
}

module_init(msm_batt_init);
module_exit(msm_batt_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Kiran Kandi, Qualcomm Innovation Center, Inc.");
MODULE_DESCRIPTION("Battery driver for Qualcomm MSM chipsets.");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:msm_battery");
