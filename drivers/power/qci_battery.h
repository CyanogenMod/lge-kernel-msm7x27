/* Header file for Quanta I2C Battery Driver
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
 *  The Driver with I/O communications via the I2C Interface for ON2 of AP BU.
 *  And it is only working on the nuvoTon WPCE775x Embedded Controller.
 *
 */

#ifndef __QCI_BATTERY_H__
#define __QCI_BATTERY_H__

#define BAT_I2C_ADDRESS 0x1A
#define BATTERY_ID_NAME          "qci-i2cbat"
#define EC_FLAG_ADAPTER_IN		0x01
#define EC_FLAG_POWER_ON		0x02
#define EC_FLAG_ENTER_S3		0x04
#define EC_FLAG_ENTER_S4		0x08
#define EC_FLAG_IN_STANDBY		0x10
#define EC_FLAG_SYSTEM_ON		0x20
#define EC_FLAG_WAIT_HWPG		0x40
#define EC_FLAG_S5_POWER_ON	0x80

#define MAIN_BATTERY_STATUS_BAT_IN			0x01
#define MAIN_BATTERY_STATUS_BAT_FULL			0x02
#define MAIN_BATTERY_STATUS_BAT_ABNORMAL	0x04
#define MAIN_BATTERY_STATUS_BAT_CHARGING	0x08
#define MAIN_BATTERY_STATUS_BAT_CRITICAL		0x10
#define MAIN_BATTERY_STATUS_BAT_LOW			0x20
#define MAIN_BATTERY_STATUS_BAT_DISCHRG		0x40
#define MAIN_BATTERY_STATUS_BAT_SMB_VALID	0x80

#define CHG_STATUS_BAT_CHARGE			0x01
#define CHG_STATUS_BAT_PRECHG				0x02
#define CHG_STATUS_BAT_OVERTEMP			0x04
#define CHG_STATUS_BAT_TYPE				0x08
#define CHG_STATUS_BAT_GWROK				0x10
#define CHG_STATUS_BAT_INCHARGE			0x20
#define CHG_STATUS_BAT_WAKECHRG			0x40
#define CHG_STATUS_BAT_CHGTIMEOUT		0x80

#define EC_ADAPTER_PRESENT		0x1
#define EC_BAT_PRESENT		0x1
#define EC_ADAPTER_NOT_PRESENT		0x0
#define EC_BAT_NOT_PRESENT		0x0
#define I2C_BAT_BUFFER_LEN		12

#endif
