/*
* Customer code to add GPIO control during WLAN start/stop
* Copyright (C) 1999-2010, Broadcom Corporation
* 
*         Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2 (the "GPL"),
* available at http://www.broadcom.com/licenses/GPLv2.php, with the
* following added to such license:
* 
*      As a special exception, the copyright holders of this software give you
* permission to link this software with independent modules, and to copy and
* distribute the resulting executable under terms of your choice, provided that
* you also meet, for each linked independent module, the terms and conditions of
* the license of that module.  An independent module is a module which is not
* derived from this software.  The special exception does not apply to any
* modifications of the software.
* 
*      Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*
* $Id: dhd_custom_gpio.c,v 1.1.4.2.20.4.2.1 2010/10/01 12:12:41 Exp $
*/


#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#ifndef BCMDONGLEHOST
#include <wlc_cfg.h>
#else
#include <dngl_stats.h>
#include <dhd.h>
#endif

#include <wlioctl.h>
#include <wl_iw.h>

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-05-14, support start/stop */
#if defined(CONFIG_LGE_BCM432X_PATCH)
#include <asm/gpio.h>
#include <linux/interrupt.h>
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-05-14, support start/stop */

#ifndef BCMDONGLEHOST
#include <wlc_pub.h>
#include <wl_dbg.h>
#else
#define WL_ERROR(x) printf x
#define WL_TRACE(x)
#endif

#ifdef CUSTOMER_HW
extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);
#endif /* CUSTOMER_HW */
#ifdef CUSTOMER_HW2
int wifi_set_carddetect(int on);
int wifi_set_power(int on, unsigned long msec);
int wifi_get_irq_number(unsigned long *irq_flags_ptr);
#endif

#if defined(OOB_INTR_ONLY)

#if defined(BCMLXSDMMC)
extern int sdioh_mmc_irq(int irq);
#endif /* (BCMLXSDMMC)  */
#ifdef CUSTOMER_HW3
#include <mach/gpio.h>
#endif

/* Customer specific Host GPIO defintion  */
static int dhd_oob_gpio_num = -1; /* GG 19 */

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

int dhd_customer_oob_irq_map(unsigned long *irq_flags_ptr)
{
	int  host_oob_irq = 0;

#ifdef CUSTOMER_HW2
	host_oob_irq = wifi_get_irq_number(irq_flags_ptr);

#else /* for NOT  CUSTOMER_HW2 */
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW
	host_oob_irq = MSM_GPIO_TO_INT(dhd_oob_gpio_num);
#elif defined CUSTOMER_HW3
	gpio_request(dhd_oob_gpio_num, "oob irq");
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif
#endif /* CUSTOMER_HW2 */

	return (host_oob_irq);
}
#endif /* defined(OOB_INTR_ONLY) */

/* Customer function to control hw specific wlan gpios */
void
dhd_customer_gpio_wlan_ctrl(int onoff)
{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(2);
#endif /* CUSTOMER_HW */
#ifdef CUSTOMER_HW2
			wifi_set_power(0, 0);
#endif
			WL_ERROR(("=========== WLAN placed in RESET ========\n"));
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-05-14, support start/stop */
#if defined(CONFIG_LGE_BCM432X_PATCH)
            if (gpio_get_value(CONFIG_BCM4329_GPIO_WL_RESET)) {
                disable_irq(gpio_to_irq(CONFIG_BCM4329_GPIO_WL_RESET));
                gpio_set_value(CONFIG_BCM4329_GPIO_WL_RESET, 0);
            }
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-07-02, add BCM4329_GPIO_WL_REGON on /off when "DRIVER START/STOP */
#ifdef CONFIG_BCM4329_GPIO_WL_REGON
            if (!gpio_get_value(CONFIG_BCM4329_GPIO_BT_RESET)) {
                gpio_set_value(CONFIG_BCM4329_GPIO_WL_REGON, 0);
            }
#endif /* CONFIG_BCM4329_GPIO_WL_REGON */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-07-02, add BCM4329_GPIO_WL_REGON on /off when "DRIVER START/STOP */
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-05-14, support start/stop */

		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(2);
#endif /* CUSTOMER_HW */
#ifdef CUSTOMER_HW2
			wifi_set_power(1, 0);
#endif
			WL_ERROR(("=========== WLAN going back to live  ========\n"));
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-05-14, support start/stop */
#if defined(CONFIG_LGE_BCM432X_PATCH)
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-07-02, add BCM4329_GPIO_WL_REGON on /off when "DRIVER START/STOP */
#ifdef CONFIG_BCM4329_GPIO_WL_REGON
            if (!gpio_get_value(CONFIG_BCM4329_GPIO_WL_REGON)) {
                gpio_set_value(CONFIG_BCM4329_GPIO_WL_REGON, 1);
                mdelay(150);
            }
#endif /* CONFIG_BCM4329_GPIO_WL_REGON */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-07-02, add BCM4329_GPIO_WL_REGON on /off when "DRIVER START/STOP */
            if (!gpio_get_value(CONFIG_BCM4329_GPIO_WL_RESET)) {
                gpio_set_value(CONFIG_BCM4329_GPIO_WL_RESET, 1);
#ifndef CONFIG_BCM4329_GPIO_WL_REGON
                mdelay(150);
#endif /* CONFIG_BCM4329_GPIO_WL_REGON */
                enable_irq(gpio_to_irq(CONFIG_BCM4329_GPIO_WL_RESET));
            }
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-05-14, support start/stop */

		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(1);
#endif /* CUSTOMER_HW */
		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(1);
#endif /* CUSTOMER_HW */
			/* Lets customer power to get stable */
			OSL_DELAY(500);
		break;
	}
}
