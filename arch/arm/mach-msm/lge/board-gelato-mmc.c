/* arch/arm/mach-msm/lge/board-gelato-mmc.c
 * Copyright (C) 2010 LGE Corporation.
 * Author: SungEun Kim <cleaneye.kim@lge.com>
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <asm/mach/mmc.h>
#include <asm/mach-types.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include "board-gelato.h"

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned sd_detect_gpio = 41;
#endif

static void sdcc_gpio_init(void)
{
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	int rc = 0;
	if (gpio_request(sd_detect_gpio, "sdc1_status_pin_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");
	rc = gpio_tlmm_config(GPIO_CFG(sd_detect_gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
									GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
					__func__, rc);
	/* if (gpio_request(GPIO_MMC_COVER_DETECT, "sdc1_status_socket_irq")) */
	/* 	pr_err("failed to request gpio sdc1_status_irq\n"); */
	/* rc = gpio_tlmm_config(GPIO_CFG(GPIO_MMC_COVER_DETECT, 0, GPIO_INPUT, GPIO_PULL_UP, */
	/* 							   GPIO_2MA), GPIO_ENABLE); */
	/* if (rc) */
	/* 	printk(KERN_ERR "%s: Failed to configure GPIO %d\n", */
	/* 				__func__, rc); */
#endif
	/* SDC1 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	if (gpio_request(GPIO_SD_DATA_3, "sdc1_data_3"))
		pr_err("failed to request gpio sdc1_data_3\n");
	if (gpio_request(GPIO_SD_DATA_2, "sdc1_data_2"))
		pr_err("failed to request gpio sdc1_data_2\n");
	if (gpio_request(GPIO_SD_DATA_1, "sdc1_data_1"))
		pr_err("failed to request gpio sdc1_data_1\n");
	if (gpio_request(GPIO_SD_DATA_0, "sdc1_data_0"))
		pr_err("failed to request gpio sdc1_data_0\n");
	if (gpio_request(GPIO_SD_CMD, "sdc1_cmd"))
		pr_err("failed to request gpio sdc1_cmd\n");
	if (gpio_request(GPIO_SD_CLK, "sdc1_clk"))
		pr_err("failed to request gpio sdc1_clk\n");
#else
	if (gpio_request(51, "sdc1_data_3"))
		pr_err("failed to request gpio sdc1_data_3\n");
	if (gpio_request(52, "sdc1_data_2"))
		pr_err("failed to request gpio sdc1_data_2\n");
	if (gpio_request(53, "sdc1_data_1"))
		pr_err("failed to request gpio sdc1_data_1\n");
	if (gpio_request(54, "sdc1_data_0"))
		pr_err("failed to request gpio sdc1_data_0\n");
	if (gpio_request(55, "sdc1_cmd"))
		pr_err("failed to request gpio sdc1_cmd\n");
	if (gpio_request(56, "sdc1_clk"))
		pr_err("failed to request gpio sdc1_clk\n");
#endif
#endif

	/* SDC2 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (gpio_request(62, "sdc2_clk"))
		pr_err("failed to request gpio sdc2_clk\n");
	if (gpio_request(63, "sdc2_cmd"))
		pr_err("failed to request gpio sdc2_cmd\n");
	if (gpio_request(64, "sdc2_data_3"))
		pr_err("failed to request gpio sdc2_data_3\n");
	if (gpio_request(65, "sdc2_data_2"))
		pr_err("failed to request gpio sdc2_data_2\n");
	if (gpio_request(66, "sdc2_data_1"))
		pr_err("failed to request gpio sdc2_data_1\n");
	if (gpio_request(67, "sdc2_data_0"))
		pr_err("failed to request gpio sdc2_data_0\n");
#endif

	/* SDC3 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
	if (gpio_request(88, "sdc3_clk"))
		pr_err("failed to request gpio sdc3_clk\n");
	if (gpio_request(89, "sdc3_cmd"))
		pr_err("failed to request gpio sdc3_cmd\n");
	if (gpio_request(90, "sdc3_data_3"))
		pr_err("failed to request gpio sdc3_data_3\n");
	if (gpio_request(91, "sdc3_data_2"))
		pr_err("failed to request gpio sdc3_data_2\n");
	if (gpio_request(92, "sdc3_data_1"))
		pr_err("failed to request gpio sdc3_data_1\n");
	if (gpio_request(93, "sdc3_data_0"))
		pr_err("failed to request gpio sdc3_data_0\n");
#endif

	/* SDC4 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	if (gpio_request(19, "sdc4_data_3"))
		pr_err("failed to request gpio sdc4_data_3\n");
	if (gpio_request(20, "sdc4_data_2"))
		pr_err("failed to request gpio sdc4_data_2\n");
	if (gpio_request(21, "sdc4_data_1"))
		pr_err("failed to request gpio sdc4_data_1\n");
	if (gpio_request(107, "sdc4_cmd"))
		pr_err("failed to request gpio sdc4_cmd\n");
	if (gpio_request(108, "sdc4_data_0"))
		pr_err("failed to request gpio sdc4_data_0\n");
	if (gpio_request(109, "sdc4_clk"))
		pr_err("failed to request gpio sdc4_clk\n");
#endif
}

/* LGE_CHANGE_S, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
static unsigned sdcc_no_mmc_sleep_cfg_data[6] = {
	GPIO_CFG(GPIO_SD_DATA_3, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_2, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_1, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_0, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CMD, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
};
/* LGE_CHANGE_E, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */

static unsigned sdcc_cfg_data[][6] = {
	/* SDC1 configs */
#ifdef  CONFIG_MMC_MSM_CARD_HW_DETECTION
	{
/* LGE_CHANGE_S, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
#if 1	// Original Code
	GPIO_CFG(GPIO_SD_DATA_3, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_2, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_1, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_0, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CMD, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
#else
	GPIO_CFG(GPIO_SD_DATA_3, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_2, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_1, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_DATA_0, 1, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CMD, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(GPIO_SD_CLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA),
#endif
/* LGE_CHANGE_E, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
	},
#else	
	{
	GPIO_CFG(51, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(52, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(53, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(54, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(55, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(56, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	},
#endif	
	/* SDC2 configs */
	{
	GPIO_CFG(62, 2, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(63, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(64, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(65, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(66, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(67, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	},
	/* SDC3 configs */
	{
	GPIO_CFG(88, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	GPIO_CFG(89, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(90, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(91, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(92, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(93, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	},
	/* SDC4 configs */
	{
	GPIO_CFG(19, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(20, 3, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(21, 4, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(107, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(108, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_8MA),
	GPIO_CFG(109, 1, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_8MA),
	}
};

static unsigned long vreg_sts, gpio_sts;
static unsigned mpp_mmc = 2;
static struct vreg *vreg_mmc;

/* LGE_CHANGE_S, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
static void msm_sdcc_setup_gpio_no_mmc(int dev_id, unsigned int enable)
{
	int i, rc;

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return;

	if (enable)
		set_bit(dev_id, &gpio_sts);
	else
		clear_bit(dev_id, &gpio_sts);

	for (i = 0; i < ARRAY_SIZE(sdcc_no_mmc_sleep_cfg_data); i++) {
		rc = gpio_tlmm_config(sdcc_no_mmc_sleep_cfg_data[i],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc)
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, sdcc_no_mmc_sleep_cfg_data[i], rc);
	}
}
/* LGE_CHANGE_E, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int i, rc;

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return;

	if (enable)
		set_bit(dev_id, &gpio_sts);
	else
		clear_bit(dev_id, &gpio_sts);

	for (i = 0; i < ARRAY_SIZE(sdcc_cfg_data[dev_id - 1]); i++) {
		rc = gpio_tlmm_config(sdcc_cfg_data[dev_id - 1][i],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc)
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, sdcc_cfg_data[dev_id - 1][i], rc);
	}
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

/* LGE_CHANGE_S, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */
		if (pdev->id == 1)
		{
			msm_sdcc_setup_gpio_no_mmc(pdev->id, !!vdd);
		}
/* LGE_CHANGE_E, [hyuncheol0.kim@lge.com] , 2011-02-10, for current consumption */

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			if (machine_is_msm7x25_ffa() ||
					machine_is_msm7x27_ffa()) {
				rc = mpp_config_digital_out(mpp_mmc,
				     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
				     MPP_DLOGIC_OUT_CTRL_LOW));
			} else
				rc = vreg_disable(vreg_mmc);
			if (rc)
				printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
		}
		return 0;
	}

	if (!vreg_sts) {
		if (machine_is_msm7x25_ffa() || machine_is_msm7x27_ffa()) {
			rc = mpp_config_digital_out(mpp_mmc,
			     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
			     MPP_DLOGIC_OUT_CTRL_HIGH));
		} else {
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
			rc = vreg_set_level(vreg_mmc, VREG_SD_LEVEL);
#else		
			rc = vreg_set_level(vreg_mmc, 2850);
#endif
			if (!rc)
				rc = vreg_enable(vreg_mmc);
		}
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
	set_bit(pdev->id, &vreg_sts);

	return 0;
}

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int gelato_sdcc_slot_status(struct device *dev)
{
	return !gpio_get_value(sd_detect_gpio);
}
#endif

/* LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
#if defined(CONFIG_LGE_BCM432X_PATCH)
static unsigned int bcm432x_sdcc_wlan_slot_status(struct device *dev)
{
	printk(KERN_ERR "%s: %d %d\n", __func__, CONFIG_BCM4330_GPIO_WL_RESET, gpio_get_value(CONFIG_BCM4330_GPIO_WL_RESET));
    return gpio_get_value(CONFIG_BCM4330_GPIO_WL_RESET);
}

static struct mmc_platform_data bcm432x_sdcc_wlan_data = {
    .ocr_mask   	= MMC_VDD_30_31,
	.translate_vdd	= msm_sdcc_setup_power,
    .status     	= bcm432x_sdcc_wlan_slot_status,
	.status_irq		= MSM_GPIO_TO_INT(CONFIG_BCM4330_GPIO_WL_RESET),
    .irq_flags      = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
    .mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	// LGE_DEV_PORTING, 2011-02-22, jongpil.yoon@lge.com, <WiFi driver timeout issue>
	.dummy52_required = 1,
	// LGE_DEV_END, 2011-02-22, jongpil.yoon@lge.com, <WiFi driver timeout issue>
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	//.msmsdcc_fmax	= 49152000,
	.msmsdcc_fmax	= 24576000,
	// LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-04-22, <do not remove the sd card when wifi turn off after patching QCT 7125>
	//.nonremovable	= 1, /* <original> */
	.nonremovable = 0,
	// LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-04-22, <do not remove the sd card when wifi turn off after patching QCT 7125>
};
#endif  /* CONFIG_LGE_BCM432X_PATCH*/
/* LGE_CHANGE_E, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */

static struct mmc_platform_data msm7x2x_sdc1_data = {
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.ocr_mask		= MMC_VDD_30_31,
	.translate_vdd	= msm_sdcc_setup_power,
	.status 		= gelato_sdcc_slot_status,
	.status_irq 	= MSM_GPIO_TO_INT(GPIO_SD_DETECT_N),
	.irq_flags		= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.mmc_bus_width	= MMC_CAP_4_BIT_DATA,
#else
	.ocr_mask		= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif	
	.msmsdcc_fmin	= 144000,
	.msmsdcc_fmid	= 24576000,
	.msmsdcc_fmax	= 49152000,
	.nonremovable	= 0,
};

static void __init msm7x2x_init_mmc(void)
{
	if (!machine_is_msm7x25_ffa() && !machine_is_msm7x27_ffa()) {
		vreg_mmc = vreg_get(NULL, "wlan");
		if (IS_ERR(vreg_mmc)) {
			printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			       __func__, PTR_ERR(vreg_mmc));
			return;
		}
	}

	sdcc_gpio_init();
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	msm_add_sdcc(1, &msm7x2x_sdc1_data);
#endif	
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT

/* LGE_CHANGE_S [jisung.yang@lge.com] 2010-04-24, BCM4325 control */
#if defined(CONFIG_LGE_BCM432X_PATCH)

	/* GPIO config */
/* LGE_CHANGE_S, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
	/*gpio_tlmm_config(GPIO_CFG(CONFIG_BCM4330_GPIO_WL_RESET, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	gpio_set_value(CONFIG_BCM4330_GPIO_WL_RESET, 0);
	
	gpio_tlmm_config(GPIO_CFG(CONFIG_BCM4330_GPIO_WL_HOSTWAKEUP, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);*/

	/* Register platform device */
    msm_add_sdcc(2, &bcm432x_sdcc_wlan_data);

	/* Enable RESET IRQ for wlan card detect */
	/*enable_irq(gpio_to_irq(CONFIG_BCM4330_GPIO_WL_RESET));*/
/* LGE_CHANGE_E, [jongpil.yoon@lge.com], 2011-01-07, <Add BCM4330> */
#else /* qualcomm or google */
    msm_add_sdcc(2, &msm7x2x_sdc1_data);
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [jisung.yang@lge.com] 2010-04-24, BCM4325 control */
}
#else
#define msm7x2x_init_mmc() do {} while (0)
#endif

void __init lge_add_mmc_devices(void)
{
	msm7x2x_init_mmc();
}
