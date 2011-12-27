/* arch/arm/mach-msm/lge/board-gelato-bt.c
 * Copyright (C) 2009 LGE, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>
#include "board-gelato.h"
#include <linux/delay.h>
#include <linux/rfkill.h>

#ifdef CONFIG_BT
static unsigned bt_config_power_on[] = {
	GPIO_CFG(BT_WAKE, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* WAKE */
	GPIO_CFG(BT_RFR, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RFR */
	GPIO_CFG(BT_CTS, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* CTS */
	GPIO_CFG(BT_RX, 2, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Rx */
	GPIO_CFG(BT_TX, 3, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Tx */
	GPIO_CFG(BT_PCM_DOUT, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DOUT */
	GPIO_CFG(BT_PCM_DIN, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DIN */
	GPIO_CFG(BT_PCM_SYNC, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_SYNC */
	GPIO_CFG(BT_PCM_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_CLK */
	GPIO_CFG(BT_HOST_WAKE, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* HOST_WAKE */
	GPIO_CFG(BT_RESET_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RESET_N */
};
static unsigned bt_config_power_off[] = {
	GPIO_CFG(BT_WAKE, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* WAKE */ 
	GPIO_CFG(BT_RFR, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RFR */ 
	GPIO_CFG(BT_CTS, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* CTS */ 
	GPIO_CFG(BT_RX, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Rx */ 
	GPIO_CFG(BT_TX, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* Tx */ 
	GPIO_CFG(BT_PCM_DOUT, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DOUT */
	GPIO_CFG(BT_PCM_DIN, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_DIN */
	GPIO_CFG(BT_PCM_SYNC, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_SYNC */
	GPIO_CFG(BT_PCM_CLK, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* PCM_CLK */
	GPIO_CFG(BT_HOST_WAKE, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* HOST_WAKE */
	GPIO_CFG(BT_RESET_N, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),	/* RESET_N */	
};

static int gelato_bluetooth_toggle_radio(void *data, bool state)
{
	int ret;
	int (*power_control)(int enable);

    power_control = ((struct bluetooth_platform_data *)data)->bluetooth_power;
	ret = (*power_control)((state == RFKILL_USER_STATE_SOFT_BLOCKED) ? 1 : 0);
	return ret;
}

static int gelato_bluetooth_power(int on)
{
	int pin, rc;
	
	printk(KERN_DEBUG "%s\n", __func__);
	printk( "%s %d\n", __func__, on);
	printk(KERN_DEBUG "%s\n", "KKKKKK");
	if (on) {
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on); pin++) {
			rc = gpio_tlmm_config(bt_config_power_on[pin],
					      GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_on[pin], rc);
				return -EIO;
			}
		}
        //Turn Bluetooth Power On if and only if not turned on by WLAN yet.
/* LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
        if (!gpio_get_value(CONFIG_BCM4330_GPIO_WL_REGON)) //#23
		    gpio_set_value(CONFIG_BCM4330_GPIO_WL_REGON, 1); //#23
/* LGE_CHANGE_E, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
		mdelay(100);
		gpio_set_value(BT_RESET_N, 0);
		mdelay(100);
		gpio_set_value(BT_RESET_N, 1);
		mdelay(100);

	} else {
        //Turn Bluetooth Power Off if and only if not used by WLAN anymore.
/* LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
        if (!gpio_get_value(CONFIG_BCM4330_GPIO_WL_RESET)) //#93
         gpio_set_value(CONFIG_BCM4330_GPIO_WL_REGON, 0); //#23
/* LGE_CHANGE_E, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */

		gpio_set_value(BT_RESET_N, 0);
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
			rc = gpio_tlmm_config(bt_config_power_off[pin],
					      GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
				return -EIO;
			}
		}
	}
	return 0;
}

static struct bluetooth_platform_data gelato_bluetooth_data = {
	.bluetooth_power = gelato_bluetooth_power,
	.bluetooth_toggle_radio = gelato_bluetooth_toggle_radio,
};

static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
	.dev = {
		.platform_data = &gelato_bluetooth_data,
	},		
};

static void __init bt_power_init(void)
{
	/* TODO: should be fixed */
#if 0
	gpio_set_value(23, 1);
	ssleep(1); /* 1 sec */
	gpio_set_value(23, 0);
#endif
//LGE_DEV_PORTING
	int pin, rc;
	for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
			rc = gpio_tlmm_config(bt_config_power_off[pin],
					      GPIO_CFG_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
			}
		}
//LGE_END
}
#else
#define bt_power_init(x) do {} while (0)
#endif

static struct resource bluesleep_resources[] = {
	{
		.name	= "gpio_host_wake",
		.start	= BT_HOST_WAKE,
		.end	= BT_HOST_WAKE,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "gpio_ext_wake",
		.start	= BT_WAKE,
		.end	= BT_WAKE,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "host_wake",
		.start	= MSM_GPIO_TO_INT(BT_HOST_WAKE),
		.end	= MSM_GPIO_TO_INT(BT_HOST_WAKE),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct bluesleep_platform_data gelato_bluesleep_data = {
	.bluetooth_port_num = 0,
};

static struct platform_device msm_bluesleep_device = {
	.name = "bluesleep",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(bluesleep_resources),
	.resource	= bluesleep_resources,
	.dev = {
		.platform_data = &gelato_bluesleep_data,
	},	
};

void __init lge_add_btpower_devices(void)
{
	bt_power_init();
#ifdef CONFIG_BT
	platform_device_register(&msm_bt_power_device);
#endif
	platform_device_register(&msm_bluesleep_device);
}
