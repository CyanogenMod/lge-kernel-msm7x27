/* Quanta I2C Battery Driver
 *
 * Copyright (C) 2009 Quanta Computer Inc.
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

/*
 *
 *  The Driver with I/O communications via the I2C Interface for ST15 platform.
 *  And it is only working on the nuvoTon WPCE775x Embedded Controller.
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/wpce775x.h>

#include "qci_battery.h"

struct qci_bat_info {
	u8 type_id;
	u8 power_flag;
	u8 ec_ver_lsb;
	u8 ec_ver_msb;
	u8 mbat_rsoc;
	u8 mbat_volt_lsb;
	u8 mbat_volt_msb;
	u8 mbat_status;
	u8 mbchg_status;
	u8 mbat_temp_lsb;
	u8 mbat_temp_msb;
};

/* General structure to hold the driver data */
struct i2cbat_drv_data {
	struct i2c_client *bi2c_client;
	struct work_struct work;
	char batt_data[I2C_BAT_BUFFER_LEN+1];
	unsigned int qcibat_irq;
	unsigned int qcibat_gpio;
	struct qci_bat_info bif;
};

static struct i2cbat_drv_data context;
/*********************************************************************
 *		Power
 *********************************************************************/

static int qci_ac_get_prop(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	int ret = 0;
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (context.bif.power_flag & EC_FLAG_ADAPTER_IN)
			val->intval =  EC_ADAPTER_PRESENT;
		else
			val->intval =  EC_ADAPTER_NOT_PRESENT;
	break;
	default:
		ret = -EINVAL;
	break;
	}
	return ret;
}

static enum power_supply_property qci_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property qci_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_AMBIENT,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
};

static int qbat_get_status(union power_supply_propval *val)
{
	if ((context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN) == 0x0)
		val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
	else if (context.bif.mbchg_status & CHG_STATUS_BAT_INCHARGE)
		val->intval = POWER_SUPPLY_STATUS_CHARGING;
	else if (context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_FULL)
		val->intval = POWER_SUPPLY_STATUS_FULL;
	else
		val->intval = POWER_SUPPLY_STATUS_DISCHARGING;

	return 0;
}

static int qbat_get_present(union power_supply_propval *val)
{
	if (context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN)
		val->intval = EC_BAT_PRESENT;
	else
		val->intval = EC_BAT_NOT_PRESENT;
	return 0;
}

static int qbat_get_health(union power_supply_propval *val)
{
	if ((context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN) == 0x0)
		val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
	else
		val->intval = POWER_SUPPLY_HEALTH_GOOD;
	return 0;
}

static int qbat_get_voltage_avg(union power_supply_propval *val)
{
	val->intval = (context.bif.mbat_volt_msb << 8 |
		       context.bif.mbat_volt_lsb);
	return 0;
}

static int qbat_get_capacity(union power_supply_propval *val)
{
	if ((context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN) == 0x0)
		val->intval = 0xFF;
	else
		val->intval = context.bif.mbat_rsoc;
	return 0;
}

static int qbat_get_temp_avg(union power_supply_propval *val)
{
	if ((context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN) == 0x0)
		val->intval = 0xFFFF;
	else
		val->intval = ((context.bif.mbat_temp_msb << 8) |
			       context.bif.mbat_temp_lsb) - 2731;
	return 0;
}

static int qbat_get_mfr(union power_supply_propval *val)
{
	val->strval = "Unknown";
	return 0;
}

static int qbat_get_tech(union power_supply_propval *val)
{
	if ((context.bif.mbat_status & MAIN_BATTERY_STATUS_BAT_IN) == 0x0)
		val->intval = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
	else
		val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
	return 0;
}

/*********************************************************************
 *		Battery properties
 *********************************************************************/
static int qbat_get_property(struct power_supply *psy,
			     enum power_supply_property psp,
			     union power_supply_propval *val)
{
	int ret = 0;
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		ret = qbat_get_status(val);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		ret = qbat_get_present(val);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		ret = qbat_get_health(val);
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		ret = qbat_get_mfr(val);
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		ret = qbat_get_tech(val);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_AVG:
		ret = qbat_get_voltage_avg(val);
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		ret = qbat_get_capacity(val);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		ret = qbat_get_temp_avg(val);
		break;
	case POWER_SUPPLY_PROP_TEMP_AMBIENT:
		ret = qbat_get_temp_avg(val);
		break;
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
		break;
	case POWER_SUPPLY_PROP_SERIAL_NUMBER:
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*********************************************************************
 *		Initialisation
 *********************************************************************/

static struct power_supply qci_ac = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = qci_ac_props,
	.num_properties = ARRAY_SIZE(qci_ac_props),
	.get_property = qci_ac_get_prop,
};

static struct power_supply qci_bat = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = qci_bat_props,
	.num_properties = ARRAY_SIZE(qci_bat_props),
	.get_property = qbat_get_property,
	.use_for_apm = 1,
};

static irqreturn_t qbat_interrupt(int irq, void *dev_id)
{
	struct i2cbat_drv_data *ibat_drv_data = dev_id;
	schedule_work(&ibat_drv_data->work);
	return IRQ_HANDLED;
}

static int qci_get_bat_info(struct i2c_client *client, char *ec_data)
{
	struct i2c_msg bat_msg;
	bat_msg.addr = client->addr;
	bat_msg.flags = I2C_M_RD;
	bat_msg.len = I2C_BAT_BUFFER_LEN;
	bat_msg.buf = ec_data;
	return i2c_transfer(client->adapter, &bat_msg, 1);
}

static void qbat_work(struct work_struct *_work)
{
	struct i2cbat_drv_data *ibat_drv_data =
		container_of(_work, struct i2cbat_drv_data, work);
	struct i2c_client *ibatclient = ibat_drv_data->bi2c_client;

	qci_get_bat_info(ibatclient, ibat_drv_data->batt_data);
	memcpy(&context.bif,
	       ibat_drv_data->batt_data,
	       sizeof(struct qci_bat_info));
	power_supply_changed(&qci_ac);
	power_supply_changed(&qci_bat);
}

static struct platform_device *bat_pdev;

static int __init qbat_init(void)
{
	int err = 0;

	context.bi2c_client = wpce_get_i2c_client();
	if (context.bi2c_client == NULL)
		return -1;

	i2c_set_clientdata(context.bi2c_client, &context);
	context.qcibat_gpio = context.bi2c_client->irq;

	/*battery device register*/
	bat_pdev = platform_device_register_simple("battery", 0, NULL, 0);
	if (IS_ERR(bat_pdev))
		return PTR_ERR(bat_pdev);

	err = power_supply_register(&bat_pdev->dev, &qci_ac);
	if (err)
		goto ac_failed;

	qci_bat.name = bat_pdev->name;
	err = power_supply_register(&bat_pdev->dev, &qci_bat);
	if (err)
		goto battery_failed;

	/*battery irq configure*/
	INIT_WORK(&context.work, qbat_work);
	err = gpio_request(context.qcibat_gpio, "qci-bat");
	if (err) {
		dev_err(&context.bi2c_client->dev,
			"[BAT] err gpio request\n");
		goto gpio_request_fail;
	}
	context.qcibat_irq = gpio_to_irq(context.qcibat_gpio);
	err = request_irq(context.qcibat_irq, qbat_interrupt,
		IRQF_TRIGGER_FALLING, BATTERY_ID_NAME, &context);
	if (err) {
		dev_err(&context.bi2c_client->dev,
			"[BAT] unable to get IRQ\n");
		goto request_irq_fail;
	}
	err = qci_get_bat_info(context.bi2c_client, context.batt_data);

	goto success;

request_irq_fail:
	gpio_free(context.qcibat_gpio);

gpio_request_fail:
	power_supply_unregister(&qci_bat);

battery_failed:
	power_supply_unregister(&qci_ac);

ac_failed:
	platform_device_unregister(bat_pdev);

	i2c_set_clientdata(context.bi2c_client, NULL);
success:
	return err;
}

static void __exit qbat_exit(void)
{
	free_irq(context.qcibat_irq, &context);
	gpio_free(context.qcibat_gpio);
	power_supply_unregister(&qci_bat);
	power_supply_unregister(&qci_ac);
	platform_device_unregister(bat_pdev);
	i2c_set_clientdata(context.bi2c_client, NULL);
}

late_initcall(qbat_init);
module_exit(qbat_exit);

MODULE_AUTHOR("Quanta Computer Inc.");
MODULE_DESCRIPTION("Quanta Embedded Controller I2C Battery Driver");
MODULE_LICENSE("GPL v2");

