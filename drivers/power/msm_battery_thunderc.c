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
#include <mach/msm_battery_thunderc.h>

#include <mach/rpc_hsusb.h>

/* LGE_CHANGE_S [woonghee.park@lge.com] 2010-03-18, ALARM */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC)
#include <linux/wakelock.h>
#endif
/* LGE_CHANGE_E [woonghee.park@lge.com] 2010-03-18, ALARM */

#define BATTERY_RPC_PROG	0x30000089
#define BATTERY_RPC_VERS	0x00010001

#define BATTERY_RPC_CB_PROG	0x31000089
#define BATTERY_RPC_CB_VERS	0x00010001

#define CHG_RPC_PROG		0x3000001a
#define CHG_RPC_VERS		0x00010001

#define BATTERY_REGISTER_PROC 				2
#define BATTERY_GET_CLIENT_INFO_PROC 			3
#define BATTERY_MODIFY_CLIENT_PROC 			4
#define BATTERY_DEREGISTER_CLIENT_PROC 			5
#define BATTERY_SERVICE_TABLES_PROC 			6
#define BATTERY_IS_SERVICING_TABLES_ENABLED_PROC 	7
#define BATTERY_ENABLE_TABLE_SERVICING_PROC 		8
#define BATTERY_DISABLE_TABLE_SERVICING_PROC 		9
#define BATTERY_READ_PROC 				10
#define BATTERY_MIMIC_LEGACY_VBATT_READ_PROC 		11
#define BATTERY_CB_TYPE_PROC 1

#define BATTERY_CB_ID_ALL_ACTIV       1

#define BATTERY_CB_ID_LOW_VOL         2

#define BATTERY_LOW             3200
#define BATTERY_HIGH            4200

#define BATTERY_LOW_CORECTION   100

#define ONCRPC_CHG_IS_CHARGING_PROC 2
#define ONCRPC_CHG_IS_CHARGER_VALID_PROC 3
#define ONCRPC_CHG_IS_BATTERY_VALID_PROC 4
#define ONCRPC_CHG_UI_EVENT_READ_PROC 5

#define ONCRPC_CHG_GET_GENERAL_STATUS_PROC 12

#define ONCRPC_LG_CHG_GET_GENERAL_STATUS_PROC 20

#define CHARGER_API_VERSION  			0x00010004
#define ONCRPC_CHARGER_API_VERSIONS_PROC 	0xffffffff

#define BATT_RPC_TIMEOUT    10000	/* 10 sec */

#define INVALID_BATT_HANDLE    -1

#define RPC_TYPE_REQ     0
#define RPC_TYPE_REPLY   1
#define RPC_REQ_REPLY_COMMON_HEADER_SIZE   (3 * sizeof(uint32_t))

#define DEBUG  1

#if DEBUG
#define DBG(x...) printk(KERN_INFO x)
#else
#define DBG(x...) do {} while (0)
#endif

/* LGE_CHANGE_S [woonghee.park@lge.com] 2010-03-18, ALARM */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC)
struct wake_lock battery_wake_lock;
#endif
/* LGE_CHANGE_E [woonghee.park@lge.com] 2010-03-18, ALARM */

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
	BATTERY_LAST_ERROR = 32,
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

struct msm_battery_info {

	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 chg_api_version;
	u32 batt_technology;

	u32 avail_chg_sources;
	u32 current_chg_source;

	u32 batt_status;
	u32 batt_health;
	u32 voltage_now;
	u32 charger_valid;
	u32 batt_valid;
	u32 batt_capacity;
	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_INFO_TEMP*/
  u32 battery_temp;	
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/
	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
  u32 valid_battery_id;
  u32 battery_therm;
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/

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

	atomic_t stop_cb_thread;
	atomic_t cb_thread_stopped;
};

static void msm_batt_wait_for_batt_chg_event(struct work_struct *work);

static DECLARE_WORK(msm_batt_cb_work, msm_batt_wait_for_batt_chg_event);

static int msm_batt_cleanup(void);

static struct msm_battery_info msm_batt_info = {
	.batt_handle = INVALID_BATT_HANDLE,
};

static struct pseudo_batt_info_type pseudo_batt_info = {
  .mode = 0,
};
static int block_charging_state = 1;//1 : charging , 0: block charging

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
			//printk(KERN_INFO "power supply=%s  online = %d \n",
			//       psy->name, val->intval);
		}

		if (psy->type == POWER_SUPPLY_TYPE_USB) {

			val->intval = msm_batt_info.current_chg_source & USB_CHG
			    ? 1 : 0;
			//printk(KERN_INFO "power supply=%s  online = %d \n",
			//       psy->name, val->intval);
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
  /* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_INFO_TEMP*/
  POWER_SUPPLY_PROP_TEMP,
  /* LGE_CHANGES_E [woonghee.park@lge.com]*/
  /* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
  POWER_SUPPLY_PROP_BATTERY_ID_CHECK,
  POWER_SUPPLY_PROP_BATTERY_TEMP_ADC,
  POWER_SUPPLY_PROP_PSEUDO_BATT,
  POWER_SUPPLY_PROP_BLOCK_CHARGING,
  /* LGE_CHANGES_E [woonghee.park@lge.com]*/
};

static void msm_batt_update_psy_status(void);

static void msm_batt_external_power_changed(struct power_supply *psy)
{
	power_supply_changed(psy);
}

static int msm_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	switch (psp) {

	case POWER_SUPPLY_PROP_STATUS:
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.charging;
    else
		  val->intval = msm_batt_info.batt_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = msm_batt_info.batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
    if(pseudo_batt_info.mode == 1)
    {
      if(pseudo_batt_info.id == 1 || pseudo_batt_info.therm != 0)
        val->intval = 1;
      else
        val->intval = 0;
    }
    else
    {
      if(msm_batt_info.batt_valid == 1 || msm_batt_info.battery_therm != 0)    
        val->intval = 1;
      else
        val->intval = 0;
    }		
    /* LGE_COMMENT_OUT
         val->intval = msm_batt_info.batt_valid; */
		/* LGE_CHANGES_E [woonghee.park@lge.com]*/
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
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.volt;
    else
		  val->intval = msm_batt_info.voltage_now;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.capacity;
    else
		  val->intval = msm_batt_info.batt_capacity;
		break;

	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_INFO_TEMP*/
  case POWER_SUPPLY_PROP_TEMP:
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.temp;
    else
      val->intval = msm_batt_info.battery_temp;
    break;
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/
	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
	case POWER_SUPPLY_PROP_BATTERY_ID_CHECK:
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.id;
    else
      val->intval = msm_batt_info.valid_battery_id;
    break;
	case POWER_SUPPLY_PROP_BATTERY_TEMP_ADC:
    if(pseudo_batt_info.mode == 1)
      val->intval = pseudo_batt_info.therm;
    else
      val->intval = msm_batt_info.battery_therm;
    break;
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/

  case POWER_SUPPLY_PROP_PSEUDO_BATT:
    val->intval = pseudo_batt_info.mode;
    break;

  case POWER_SUPPLY_PROP_BLOCK_CHARGING:
    val->intval = block_charging_state;
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

enum charger_type {  // it  comes from msm_hsusb.c
	CHG_HOST_PC,
	CHG_WALL = 2,
	CHG_UNDEFINED,
};

int charger_hw_type;

/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
extern void battery_info_get(struct batt_info* rsp);
extern void pseudo_batt_info_set(struct pseudo_batt_info_type*);

int pseudo_batt_set(struct pseudo_batt_info_type* info)
{
  pseudo_batt_info.mode = info->mode;
  pseudo_batt_info.id = info->id;
  pseudo_batt_info.therm = info->therm;
  pseudo_batt_info.temp = info->temp;
  pseudo_batt_info.volt = info->volt;
  pseudo_batt_info.capacity = info->capacity;
  pseudo_batt_info.charging = info->charging;

	power_supply_changed(&msm_psy_batt);
  pseudo_batt_info_set(&pseudo_batt_info);
  return 0;
}
EXPORT_SYMBOL(pseudo_batt_set);
extern void block_charging_set(int);
void batt_block_charging_set(int block)
{
	block_charging_state = block;
	block_charging_set(block);
}
EXPORT_SYMBOL(batt_block_charging_set);

struct batt_info batt_info_buf;
/* LGE_CHANGES_E [woonghee.park@lge.com]*/

struct rpc_reply_batt_chg {
	struct rpc_reply_hdr hdr;
	u32 	more_data;

	u32	battery_level;
	u32  battery_voltage;
  u32  battery_id;
  u32  battery_therm;
	u32	battery_temp;
  u32  battery_valid;
  u32  battery_charging;
  u32  charger_valid;
  u32  chg_batt_event;
};

static struct rpc_reply_batt_chg rep_batt_chg;

static int msm_batt_get_batt_chg_status_v1(u32 *batt_charging,
					u32 *charger_valid,
					u32 *chg_batt_event)
{
	int rc ;
	struct rpc_req_batt_chg {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_batt_chg;

	*batt_charging = 0;
	*chg_batt_event = CHG_UI_EVENT_INVALID;
	*charger_valid = 0;

	req_batt_chg.more_data = cpu_to_be32(1);

	memset(&rep_batt_chg, 0, sizeof(rep_batt_chg));

	rc = msm_rpc_call_reply(msm_batt_info.chg_ep,
				ONCRPC_LG_CHG_GET_GENERAL_STATUS_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_batt_chg, sizeof(rep_batt_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		printk(KERN_ERR
		       "%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
		       __func__, ONCRPC_LG_CHG_GET_GENERAL_STATUS_PROC, rc);
		return rc;
	} else if (be32_to_cpu(rep_batt_chg.more_data)) {
		
		//printk(KERN_ERR	"%s: rpc read batt general info\n", __func__);

		rep_batt_chg.battery_level =
			be32_to_cpu(rep_batt_chg.battery_level);

		rep_batt_chg.battery_voltage =
			be32_to_cpu(rep_batt_chg.battery_voltage);

		rep_batt_chg.battery_id =
			be32_to_cpu(rep_batt_chg.battery_id);

		rep_batt_chg.battery_therm =
			be32_to_cpu(rep_batt_chg.battery_therm);

		rep_batt_chg.battery_temp =
			be32_to_cpu(rep_batt_chg.battery_temp);

	  rep_batt_chg.battery_valid =
		  be32_to_cpu(rep_batt_chg.battery_valid);
	
  	rep_batt_chg.battery_charging =
  		be32_to_cpu(rep_batt_chg.battery_charging);

  	rep_batt_chg.charger_valid =
  		be32_to_cpu(rep_batt_chg.charger_valid);

  	rep_batt_chg.chg_batt_event =
  		be32_to_cpu(rep_batt_chg.chg_batt_event);

    msm_batt_info.batt_capacity = rep_batt_chg.battery_level;
    msm_batt_info.voltage_now = rep_batt_chg.battery_voltage;
		batt_info_buf.valid_batt_id = rep_batt_chg.battery_id;
    batt_info_buf.batt_therm = rep_batt_chg.battery_therm;
    batt_info_buf.batt_temp = rep_batt_chg.battery_temp;
    msm_batt_info.batt_valid = rep_batt_chg.battery_valid;
    *batt_charging = rep_batt_chg.battery_charging;
    *charger_valid = rep_batt_chg.charger_valid;
    *chg_batt_event = rep_batt_chg.chg_batt_event;
	} else {
		printk(KERN_INFO "%s():No more data in batt_chg rpc reply\n",
				__func__);
		return -EIO;
	}

	charger_hw_type = msm_hsusb_get_charger_type();
	return 0;
}

static int msm_batt_get_batt_chg_status(u32 *batt_charging,
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

	{
    	int temp;

	    temp = be32_to_cpu(rep_volt.voltage);
    	msm_batt_info.batt_capacity = temp & 0xFFFF; //lower  2 bytes for capacity
	    msm_batt_info.voltage_now = (temp >> 16);  //upeer 2 bytes for voltage
	}

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

	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
  battery_info_get((struct batt_info*)&batt_info_buf);
  //	printk(KERN_ERR "battery_info_get : auth_id=%d batt_therm_adc=%d batt_temp=%d\n",
  //						batt_info_buf.valid_batt_id, batt_info_buf.batt_therm, batt_info_buf.batt_temp);	
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/

	charger_hw_type = msm_hsusb_get_charger_type();

	return 0;
}

static void msm_batt_update_psy_status(void)
{
	u32 batt_charging = 0;
	u32 chg_batt_event = CHG_UI_EVENT_INVALID;
	u32 charger_valid = 0;

  if (msm_batt_info.chg_api_version >= CHARGER_API_VERSION)
  {
    msm_batt_get_batt_chg_status_v1(&batt_charging, &charger_valid, &chg_batt_event);
		batt_info_buf.valid_batt_id = rep_batt_chg.battery_id;
		batt_info_buf.batt_therm = rep_batt_chg.battery_therm;
		batt_info_buf.batt_temp = rep_batt_chg.battery_temp;
  }
  else
  	msm_batt_get_batt_chg_status(&batt_charging, &charger_valid, &chg_batt_event);

	DBG(KERN_INFO "batt_charging = %u  batt_valid = %u batt_volt = %u\n"
			"batt_level = %u charger_valid = %u chg_batt_event = %u\n"
      "batt_id = %u batt_therm = %u batt_temp = %u\n",
			batt_charging, msm_batt_info.batt_valid,msm_batt_info.voltage_now,
			msm_batt_info.batt_capacity, charger_valid, chg_batt_event,
      batt_info_buf.valid_batt_id, batt_info_buf.batt_therm, batt_info_buf.batt_temp);

	//printk(KERN_INFO "Previous charger valid status = %u"
	//		"  current charger valid status = %u\n",
	//		msm_batt_info.charger_valid, charger_valid);
	
  /* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
  msm_batt_info.valid_battery_id = batt_info_buf.valid_batt_id;
	/* LGE_CHANGES_E [woonghee.park@lge.com]*/

	//NEED_TO_CHECK
	if (msm_batt_info.charger_valid != charger_valid) {
		/* LGE_CHANGE_S [woonghee.park@lge.com] 2010-03-18, ALARM */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC)
			wake_lock_timeout(&battery_wake_lock, 5*HZ);
#endif
		/* LGE_CHANGE_E [woonghee.park@lge.com] 2010-03-18, ALARM */
		msm_batt_info.charger_valid = charger_valid;
		if (msm_batt_info.charger_valid && charger_hw_type == CHG_HOST_PC) {
			msm_batt_info.current_chg_source |= USB_CHG;
			msm_batt_info.current_chg_source &= ~AC_CHG;
			power_supply_changed(&msm_psy_usb);			
    	} else if (msm_batt_info.charger_valid && charger_hw_type == CHG_WALL) {
			msm_batt_info.current_chg_source |= AC_CHG;
			msm_batt_info.current_chg_source &= ~USB_CHG;
			power_supply_changed(&msm_psy_ac);			
	    } else {
			msm_batt_info.current_chg_source &= ~(USB_CHG|AC_CHG);
    	}
	}

	if (msm_batt_info.batt_valid) {

		if (msm_batt_info.voltage_now >
		    msm_batt_info.voltage_max_design)
			msm_batt_info.batt_health =
			    POWER_SUPPLY_HEALTH_OVERVOLTAGE;

		else if (msm_batt_info.voltage_now
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
    {
			msm_batt_info.batt_status = POWER_SUPPLY_STATUS_FULL;
      msm_batt_info.batt_capacity = 100;
    }

  	/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM*/
  	msm_batt_info.battery_temp = batt_info_buf.batt_temp * 10;
  	msm_batt_info.battery_therm = batt_info_buf.batt_therm;	
  	/* LGE_CHANGES_E [woonghee.park@lge.com]*/

	} else {
		msm_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		msm_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		msm_batt_info.batt_capacity = 0;
		/* LGE_CHANGES_S [woonghee.park@lge.com] 2010-02-09, [VS740], LG_FW_BATT_ID_CHECK, LG_FW_BATT_THM
		 * Edit by seonghwan.hong 2010-09-02
		 * for imeplements displaing battery temp at battery overheat
		 * Change 0 to batt_info_buf.batt_temp * 10
		 */
		msm_batt_info.battery_temp =  batt_info_buf.batt_temp * 10;

		msm_batt_info.battery_therm = batt_info_buf.batt_therm;		
		/* LGE_CHANGES_E [woonghee.park@lge.com] */

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
		printk(KERN_INFO " batt_clnt_handle = %d\n", rc);
		return rc;
	}
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

int batt_rpc_rx_count=0;
int batt_rpc_rx_err_count =0;
int batt_rpc_rx_reply_count=0;
int batt_rpc_rx_reply_err_count=0;


static void msm_batt_wait_for_batt_chg_event(struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&msm_batt_info.lock, flags);
	msm_batt_info.cb_thread = current;
	spin_unlock_irqrestore(&msm_batt_info.lock, flags);

	//printk(KERN_INFO "%s: Batt RPC call back thread"
	//       " started.\n", __func__);

	if (!atomic_read(&msm_batt_info.stop_cb_thread)) {
	    batt_rpc_rx_count++;
		msm_batt_update_psy_status();
	}
}

static int msm_batt_stop_cb_thread(void)
{
	int rc;
	unsigned long flags;

	rc = 0;

	spin_lock_irqsave(&msm_batt_info.lock, flags);

	if (msm_batt_info.cb_thread) {
		atomic_set(&msm_batt_info.stop_cb_thread, 1);
		spin_unlock_irqrestore(&msm_batt_info.lock, flags);

		rc = wait_event_interruptible(msm_batt_info.wait_q,
			atomic_read(&msm_batt_info.  cb_thread_stopped));

		if (rc == -ERESTARTSYS)
			printk(KERN_ERR "%s(): suspend thread got signal"
				"while wating for batt thread to finish\n",
				__func__);
		else if (rc < 0)
			printk(KERN_ERR "%s(): suspend thread wait returned "
				"error while waiting for batt thread"
				"to finish. rc =%d\n", __func__, rc);
		else
			printk(KERN_INFO "%s(): suspend thread wait returned "
				"rc =%d\n", __func__, rc);

		atomic_set(&msm_batt_info.cb_thread_stopped, 0);
	} else {
		atomic_set(&msm_batt_info.stop_cb_thread, 1);
		spin_unlock_irqrestore(&msm_batt_info.lock, flags);
	}

	return rc;
}

static void msm_batt_start_cb_thread(void)
{
	atomic_set(&msm_batt_info.stop_cb_thread, 0);
	atomic_set(&msm_batt_info.cb_thread_stopped, 0);
  queue_work(msm_batt_info.msm_batt_wq, &msm_batt_cb_work);
}

static int msm_batt_cleanup(void)
{
	int rc = 0;
	int rc_local;

	if (msm_batt_info.msm_batt_wq) {
		msm_batt_stop_cb_thread();
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

	return rc;
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
				" rc=%d\n", __func__, rc);
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
				" rc=%d\n", __func__, rc);
			msm_batt_cleanup();
			return rc;
		}
		msm_batt_info.msm_psy_usb = &msm_psy_usb;
		msm_batt_info.avail_chg_sources |= USB_CHG;
	}

	if (!msm_batt_info.msm_psy_ac && !msm_batt_info.msm_psy_usb) {

		dev_err(&pdev->dev,
			"%s: No external Power supply(AC or USB )"
			"is avilable\n", __func__);
		msm_batt_cleanup();
		return -ENODEV;
	}

	msm_batt_info.voltage_max_design = pdata->voltage_max_design;
	msm_batt_info.voltage_min_design = pdata->voltage_min_design;
	msm_batt_info.batt_technology = pdata->batt_technology;

	if (!msm_batt_info.voltage_min_design)
		msm_batt_info.voltage_min_design = BATTERY_LOW;
	if (!msm_batt_info.voltage_max_design)
		msm_batt_info.voltage_max_design = BATTERY_HIGH;

	if (msm_batt_info.batt_technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		msm_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;

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

	msm_batt_start_cb_thread();

	/* LGE_CHANGE_S [woonghee.park@lge.com] 2010-03-18, ALARM */
#if defined (CONFIG_MACH_MSM7X27_THUNDERC)
		wake_lock_init(&battery_wake_lock, WAKE_LOCK_SUSPEND, "msm_battery");
#endif
	/* LGE_CHANGE_E [woonghee.park@lge.com] 2010-03-18, ALARM */
	return 0;
}

int handle_batt_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	//printk(KERN_INFO "handle_batt_rpc_call: proc(0x%x)\n",
	//			 req->procedure);

	switch (req->procedure) {
  	case BATTERY_CB_TYPE_PROC: {
      //return success reply at  rpc_send_accepted_void_reply()
      queue_work(msm_batt_info.msm_batt_wq, &msm_batt_cb_work);
  		return 0;
  	}
  	default:
  		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server[] = {
	{
		.prog = BATTERY_RPC_CB_PROG,
		.vers = BATTERY_RPC_CB_VERS,
		.rpc_call = handle_batt_rpc_call,
	},
};

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

	printk(KERN_INFO "battery rpc: ept : 0x%x, prog : 0x%x, vers : 0x%x\n",
					(u32)msm_batt_info.batt_ep, BATTERY_RPC_PROG ,BATTERY_RPC_VERS);

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

	printk(KERN_INFO "charger rpc: ept : 0x%x, prog : 0x%x, vers : 0x%x\n",
					(u32)msm_batt_info.chg_ep, CHG_RPC_PROG ,CHG_RPC_VERS);

	msm_batt_info.chg_api_version =  msm_batt_get_charger_api_version();

	rc = platform_driver_register(&msm_batt_driver);

	if (rc < 0) {
		printk(KERN_ERR "%s: platform_driver_register failed for "
			"batt driver. rc = %d\n", __func__, rc);
		return rc;
	}

	init_waitqueue_head(&msm_batt_info.wait_q);

	rc = msm_rpc_create_server(&rpc_server[0]);
	if(rc < 0)
	{
		printk(KERN_ERR "%s: msm_rpc_create_server failed for "
             "batt driver. rc = %d\n", __func__, rc);
		return rc;
	}

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
