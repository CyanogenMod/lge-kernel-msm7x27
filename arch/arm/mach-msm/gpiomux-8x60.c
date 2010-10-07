/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <linux/module.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>
#include "gpiomux.h"

#define CONSOLE_UART	(GPIOMUX_FUNC_2 | GPIOMUX_DRV_8MA | GPIOMUX_VALID)

#ifdef CONFIG_I2C_QUP
#define GSBI3	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA | GPIOMUX_VALID)
#ifdef CONFIG_MSM_CAMERA
#define GSBI4	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA | GPIOMUX_VALID)
#else
#define GSBI4	0
#endif
#define GSBI7	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_12MA | GPIOMUX_VALID)
#define GSBI8	(GPIOMUX_FUNC_1 | GPIOMUX_VALID)
#else
#define GSBI3	0
#define GSBI4	0
#define GSBI7	0
#define GSBI8	0
#endif

#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
#define GSBI1	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA | GPIOMUX_VALID)
#else
#define GSBI1	0
#endif

#define PS_HOLD	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_12MA | GPIOMUX_VALID)

#define USB_SWITCH_EN_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE | GPIOMUX_VALID)
#define USB_SWITCH_CNTL_ACTV_CFG	(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE | GPIOMUX_VALID)
#define USB_HUB_RESET_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE | GPIOMUX_VALID)
#define USB_SWITCH_EN_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_VALID |\
					 GPIOMUX_PULL_DOWN)
#define USB_SWITCH_CNTL_SUSP_CFG	(GPIOMUX_FUNC_GPIO | GPIOMUX_VALID |\
					 GPIOMUX_PULL_DOWN)
#define USB_HUB_RESET_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_VALID |\
					 GPIOMUX_PULL_DOWN)

#define EBI2_A_D	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)
#define EBI2_OE		(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)
#define EBI2_WE		(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)
#define EBI2_CS2	(GPIOMUX_FUNC_2 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)
#define EBI2_CS3	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)
#define EBI2_ADV	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA |\
			 GPIOMUX_VALID)

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	#define SDCC1_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_10MA)
#else
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC1_CLK_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA)
#else
	#define SDCC1_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC1_CLK_ACTV_CFG 0
#endif

#define SDCC1_SUSPEND_CONFIG (GPIOMUX_VALID | GPIOMUX_PULL_UP)

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	#define SDCC2_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#else
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC2_CLK_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_16MA)
#else
	#define SDCC2_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC2_CLK_ACTV_CFG 0
#endif

#define SDCC2_SUSPEND_CONFIG (GPIOMUX_VALID | GPIOMUX_PULL_DOWN)

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
	#define SDCC5_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC5_8_BIT_SUPPORT
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#else
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC5_CLK_ACTV_CFG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_16MA)
#else
	#define SDCC5_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC5_CLK_ACTV_CFG 0
#endif

#define SDCC5_SUSPEND_CONFIG (GPIOMUX_VALID | GPIOMUX_PULL_DOWN)

#define AUX_PCM_ACTIVE_CONFIG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_2MA)

#define AUX_PCM_SUSPEND_CONFIG    (GPIOMUX_VALID | GPIOMUX_PULL_NONE)

#ifdef CONFIG_SERIAL_MSM_HS
#define UART1DM_ACTIVE     (GPIOMUX_VALID | GPIOMUX_FUNC_1 |		\
			    GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE)
#define UART1DM_SUSPENDED  (GPIOMUX_VALID | GPIOMUX_FUNC_GPIO |		\
			    GPIOMUX_DRV_2MA | GPIOMUX_PULL_DOWN)
#else
#define UART1DM_ACTIVE     0
#define UART1DM_SUSPENDED  0
#endif

static struct msm_gpiomux_config msm_gpiomux_configs[NR_GPIO_IRQS] = {
	[33] = {
		.suspended = GSBI1,
	},
	[34] = {
		.suspended = GSBI1,
	},
	[35] = {
		.suspended = GSBI1,
	},
	[36] = {
		.suspended = GSBI1,
	},
	[40] = {
		.suspended = EBI2_CS2,
	},
	[43] = {
		.suspended = GSBI3,
	},
	[44] = {
		.suspended = GSBI3,
	},
	[47] = {
		.suspended = GSBI4,
	},
	[48] = {
		.suspended = GSBI4,
	},
	[53] = { /* UARTDM_TX */
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	[54] = { /* UARTDM_RX */
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	[55] = { /* UARTDM_CTS */
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	[56] = { /* UARTDM_RFR */
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	[59] = {
		.suspended = GSBI7,
	},
	[60] = {
		.suspended = GSBI7,
	},
	[61] = {
		.active = GPIOMUX_PULL_NONE | GPIOMUX_DRV_2MA |
				GPIOMUX_VALID | GPIOMUX_FUNC_GPIO,
		.suspended = GPIOMUX_PULL_NONE | GPIOMUX_VALID,
	},
	[64] = {
		.suspended = GSBI8,
	},
	[65] = {
		.suspended = GSBI8,
	},
	[92] = {
		.suspended = PS_HOLD,
	},
	[111] = {
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	[112] = {
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	[113] = {
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	[114] = {
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	[115] = {
		.suspended = CONSOLE_UART,
	},
	[116] = {
		.suspended = CONSOLE_UART,
	},
	[117] = {
		.suspended = CONSOLE_UART,
	},
	[118] = {
		.suspended = CONSOLE_UART,
	},
	[123] = {
		.suspended = EBI2_A_D,
	},
	[124] = {
		.suspended = EBI2_A_D,
	},
	[125] = {
		.suspended = EBI2_A_D,
	},
	[126] = {
		.suspended = EBI2_A_D,
	},
	[127] = {
		.suspended = EBI2_A_D,
	},
	[128] = {
		.suspended = EBI2_A_D,
	},
	[129] = {
		.suspended = EBI2_A_D,
	},
	[130] = {
		.suspended = EBI2_A_D,
	},
	[133] = {
		.suspended = EBI2_CS3,
	},
	[135] = {
		.suspended = EBI2_A_D,
	},
	[136] = {
		.suspended = EBI2_A_D,
	},
	[137] = {
		.suspended = EBI2_A_D,
	},
	[138] = {
		.suspended = EBI2_A_D,
	},
	[139] = {
		.suspended = EBI2_A_D,
	},
	[140] = {
		.suspended = EBI2_A_D,
	},
	[141] = {
		.suspended = EBI2_A_D,
	},
	[142] = {
		.suspended = EBI2_A_D,
	},
	[143] = {
		.suspended = EBI2_A_D,
	},
	[144] = {
		.suspended = EBI2_A_D,
	},
	[145] = {
		.suspended = EBI2_A_D,
	},
	[146] = {
		.suspended = EBI2_A_D,
	},
	[147] = {
		.suspended = EBI2_A_D,
	},
	[148] = {
		.suspended = EBI2_A_D,
	},
	[149] = {
		.suspended = EBI2_A_D,
	},
	[150] = {
		.suspended = EBI2_A_D,
	},
	[151] = {
		.suspended = EBI2_OE,
	},
	[153] = {
		.suspended = EBI2_ADV,
	},
	[157] = {
		.suspended = EBI2_WE,
	},
	/* SDCC1 data[0] */
	[159] = {
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[1] */
	[160] = {
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[2] */
	[161] = {
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[3] */
	[162] = {
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[4] */
	[163] = {
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[5] */
	[164] = {
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[6] */
	[165] = {
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[7] */
	[166] = {
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 CLK */
	[167] = {
		.active = SDCC1_CLK_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 CMD */
	[168] = {
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
};

static int __init gpiomux_init(void)
{
	if (machine_is_msm8x60_qrdc()) {
		msm_gpiomux_configs[34].active = USB_HUB_RESET_ACTV_CFG;
		msm_gpiomux_configs[34].suspended = USB_HUB_RESET_SUSP_CFG;
		msm_gpiomux_configs[131].active = USB_SWITCH_CNTL_ACTV_CFG;
		msm_gpiomux_configs[131].suspended = USB_SWITCH_CNTL_SUSP_CFG;
		msm_gpiomux_configs[132].active = USB_SWITCH_EN_ACTV_CFG;
		msm_gpiomux_configs[132].suspended = USB_SWITCH_EN_SUSP_CFG;
	}

	return msm_gpiomux_init(msm_gpiomux_configs, NR_GPIO_IRQS);
}
postcore_initcall(gpiomux_init);
