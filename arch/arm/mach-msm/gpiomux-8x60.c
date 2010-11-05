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
#define MSM_SNDDEV_ACTIVE_CONFIG	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_NONE |\
					 GPIOMUX_DRV_2MA | GPIOMUX_VALID)
#define MSM_SNDDEV_SUSPEND_CONFIG	(GPIOMUX_VALID | GPIOMUX_PULL_DOWN)

#define WLAN_PWDN_N_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_UP | GPIOMUX_VALID)
#define WLAN_PWDN_N_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_VALID |\
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

#define MI2S_ACTIVE_CFG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_2MA)

#define MI2S_SUSPEND_CFG (GPIOMUX_VALID | GPIOMUX_PULL_DOWN)

#define LCDC_ACTIVE_CFG (GPIOMUX_VALID | GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA)

#define LCDC_SUSPEND_CFG (GPIOMUX_VALID | GPIOMUX_PULL_DOWN)
#define HDMI_SUSPEND_CFG (GPIOMUX_VALID | GPIOMUX_PULL_DOWN)

static struct msm_gpiomux_config msm8x60_gsbi_configs[] __initdata = {
	{
		.gpio      = 33,
		.suspended = GSBI1,
	},
	{
		.gpio      = 34,
		.suspended = GSBI1,
	},
	{
		.gpio      = 35,
		.suspended = GSBI1,
	},
	{
		.gpio      = 36,
		.suspended = GSBI1,
	},
	{
		.gpio      = 43,
		.suspended = GSBI3,
	},
	{
		.gpio      = 44,
		.suspended = GSBI3,
	},
	{
		.gpio      = 47,
		.suspended = GSBI4,
	},
	{
		.gpio      = 48,
		.suspended = GSBI4,
	},
	{
		.gpio      = 59,
		.suspended = GSBI7,
	},
	{
		.gpio      = 60,
		.suspended = GSBI7,
	},
	{
		.gpio      = 64,
		.suspended = GSBI8,
	},
	{
		.gpio      = 65,
		.suspended = GSBI8,
	},
};

static struct msm_gpiomux_config msm8x60_ebi2_configs[] __initdata = {
	{
		.gpio      = 40,
		.suspended = EBI2_CS2,
	},
	{
		.gpio      = 92,
		.suspended = PS_HOLD,
	},
	{
		.gpio      = 123,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 124,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 125,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 126,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 127,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 128,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 129,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 130,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 133,
		.suspended = EBI2_CS3,
	},
	{
		.gpio      = 135,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 136,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 137,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 138,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 139,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 140,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 141,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 142,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 143,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 144,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 145,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 146,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 147,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 148,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 149,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 150,
		.suspended = EBI2_A_D,
	},
	{
		.gpio      = 151,
		.suspended = EBI2_OE,
	},
	{
		.gpio      = 153,
		.suspended = EBI2_ADV,
	},
	{
		.gpio      = 157,
		.suspended = EBI2_WE,
	},
};

static struct msm_gpiomux_config msm8x60_uart_configs[] __initdata = {
	{ /* UARTDM_TX */
		.gpio      = 53,
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	{ /* UARTDM_RX */
		.gpio      = 54,
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	{ /* UARTDM_CTS */
		.gpio      = 55,
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	{ /* UARTDM_RFR */
		.gpio      = 56,
		.active    = UART1DM_ACTIVE,
		.suspended = UART1DM_SUSPENDED,
	},
	{
		.gpio      = 115,
		.suspended = CONSOLE_UART,
	},
	{
		.gpio      = 116,
		.suspended = CONSOLE_UART,
	},
	{
		.gpio      = 117,
		.suspended = CONSOLE_UART,
	},
	{
		.gpio      = 118,
		.suspended = CONSOLE_UART,
	},
};

static struct msm_gpiomux_config msm8x60_tmg200_configs[] __initdata = {
	{
		.gpio = 58,
		.suspended = GPIOMUX_PULL_DOWN | GPIOMUX_VALID,
	},
	{
		.gpio = 61,
		.active = GPIOMUX_PULL_NONE | GPIOMUX_DRV_2MA |
				GPIOMUX_VALID | GPIOMUX_FUNC_GPIO,
		.suspended = GPIOMUX_PULL_NONE | GPIOMUX_VALID,
	},
};

static struct msm_gpiomux_config msm8x60_aux_pcm_configs[] __initdata = {
	{
		.gpio = 111,
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	{
		.gpio = 112,
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	{
		.gpio = 113,
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
	{
		.gpio = 114,
		.active = AUX_PCM_ACTIVE_CONFIG,
		.suspended = AUX_PCM_SUSPEND_CONFIG
	},
};

static struct msm_gpiomux_config msm8x60_sdc_configs[] __initdata = {
	/* SDCC1 data[0] */
	{
		.gpio = 159,
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[1] */
	{
		.gpio = 160,
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[2] */
	{
		.gpio = 161,
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[3] */
	{
		.gpio = 162,
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[4] */
	{
		.gpio = 163,
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[5] */
	{
		.gpio = 164,
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[6] */
	{
		.gpio = 165,
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 data[7] */
	{
		.gpio = 166,
		.active = SDCC1_DAT_4_7_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 CLK */
	{
		.gpio = 167,
		.active = SDCC1_CLK_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
	/* SDCC1 CMD */
	{
		.gpio = 168,
		.active = SDCC1_DAT_0_3_CMD_ACTV_CFG,
		.suspended = SDCC1_SUSPEND_CONFIG
	},
};

static struct msm_gpiomux_config msm_qrdc_sdc_configs[] __initdata = {
	{
		.gpio      = 118,
		.active    = WLAN_PWDN_N_ACTV_CFG,
		.suspended = WLAN_PWDN_N_SUSP_CFG,
	},
};

static struct msm_gpiomux_config msm_qrdc_usb_configs[] __initdata = {
	{
		.gpio      = 34,
		.active    = USB_HUB_RESET_ACTV_CFG,
		.suspended = USB_HUB_RESET_SUSP_CFG,
	},
	{
		.gpio      = 131,
		.active    = USB_SWITCH_CNTL_ACTV_CFG,
		.suspended = USB_SWITCH_CNTL_SUSP_CFG,
	},
	{
		.gpio      = 132,
		.active    = USB_SWITCH_EN_ACTV_CFG,
		.suspended = USB_SWITCH_EN_SUSP_CFG,
	},
};

static struct msm_gpiomux_config msm8x60_snd_configs[] __initdata = {
	{
		.gpio = 108,
		.active = MSM_SNDDEV_ACTIVE_CONFIG,
		.suspended = MSM_SNDDEV_SUSPEND_CONFIG,
	},
	{
		.gpio = 109,
		.active = MSM_SNDDEV_ACTIVE_CONFIG,
		.suspended = MSM_SNDDEV_SUSPEND_CONFIG,
	},
};

static struct msm_gpiomux_config msm8x60_mi2s_configs[] __initdata = {
	/* MI2S WS */
	{
		.gpio = 101,
		.active = MI2S_ACTIVE_CFG,
		.suspended = MI2S_SUSPEND_CFG
	},
	/* MI2S SCLK */
	{
		.gpio = 102,
		.active = MI2S_ACTIVE_CFG,
		.suspended = MI2S_SUSPEND_CFG
	},
	/* MI2S MCLK */
	{
		.gpio = 103,
		.active = MI2S_ACTIVE_CFG,
		.suspended = MI2S_SUSPEND_CFG
	},
	/* MI2S SD3 */
	{
		.gpio = 107,
		.active = MI2S_ACTIVE_CFG,
		.suspended = MI2S_SUSPEND_CFG
	},
};

static struct msm_gpiomux_config msm8x60_lcdc_configs[] __initdata = {
	/* lcdc_pclk */
	{
		.gpio = 0,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_hsync */
	{
		.gpio = 1,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_vsync */
	{
		.gpio = 2,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_den */
	{
		.gpio = 3,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red7 */
	{
		.gpio = 4,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red6 */
	{
		.gpio = 5,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red5 */
	{
		.gpio = 6,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red4 */
	{
		.gpio = 7,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red3 */
	{
		.gpio = 8,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red2 */
	{
		.gpio = 9,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red1 */
	{
		.gpio = 10,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_red0 */
	{
		.gpio = 11,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn7 */
	{
		.gpio = 12,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn6 */
	{
		.gpio = 13,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn5 */
	{
		.gpio = 14,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn4 */
	{
		.gpio = 15,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn3 */
	{
		.gpio = 16,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn2 */
	{
		.gpio = 17,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn1 */
	{
		.gpio = 18,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_grn0 */
	{
		.gpio = 19,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu7 */
	{
		.gpio = 20,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu6 */
	{
		.gpio = 21,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu5 */
	{
		.gpio = 22,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu4 */
	{
		.gpio = 23,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu3 */
	{
		.gpio = 24,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu2 */
	{
		.gpio = 25,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu1 */
	{
		.gpio = 26,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
	/* lcdc_blu0 */
	{
		.gpio = 27,
		.active = LCDC_ACTIVE_CFG,
		.suspended = LCDC_SUSPEND_CFG
	},
};

static struct msm_gpiomux_config msm8x60_hdmi_configs[] __initdata = {
	{
		.gpio = 169,
		.active = GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_VALID,
		.suspended = HDMI_SUSPEND_CFG,
	},
	{
		.gpio = 170,
		.active = GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA | GPIOMUX_VALID,
		.suspended = HDMI_SUSPEND_CFG,
	},
	{
		.gpio = 171,
		.active = GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA | GPIOMUX_VALID,
		.suspended = HDMI_SUSPEND_CFG,
	},
	{
		.gpio = 172,
		.active = GPIOMUX_FUNC_1 | GPIOMUX_PULL_DOWN | GPIOMUX_VALID,
		.suspended = HDMI_SUSPEND_CFG,
	},
};

/* Because PMIC drivers do not use gpio-management routines and PMIC
 * gpios must never sleep, a "good enough" config is obtained by placing
 * the active config in the 'suspended' slot and leaving the active
 * config invalid: the suspended config will be installed at boot
 * and never replaced.
 */
static struct msm_gpiomux_config msm8x60_pmic_configs[] __initdata = {
	{
		.gpio = 88,
		.suspended = GPIOMUX_VALID,
	},
	{
		.gpio = 91,
		.suspended = GPIOMUX_VALID,
	},
};

struct msm_gpiomux_cfg_block {
	struct msm_gpiomux_config *cfg;
	size_t                     ncfg;
};
static struct msm_gpiomux_cfg_block msm8x60_cfgs[] __initdata = {
	{msm8x60_gsbi_configs, ARRAY_SIZE(msm8x60_gsbi_configs)},
	{msm8x60_ebi2_configs, ARRAY_SIZE(msm8x60_ebi2_configs)},
	{msm8x60_uart_configs, ARRAY_SIZE(msm8x60_uart_configs)},
	{msm8x60_tmg200_configs, ARRAY_SIZE(msm8x60_tmg200_configs)},
	{msm8x60_aux_pcm_configs, ARRAY_SIZE(msm8x60_aux_pcm_configs)},
	{msm8x60_sdc_configs, ARRAY_SIZE(msm8x60_sdc_configs)},
	{msm8x60_snd_configs, ARRAY_SIZE(msm8x60_snd_configs)},
	{msm8x60_mi2s_configs, ARRAY_SIZE(msm8x60_mi2s_configs)},
	{msm8x60_lcdc_configs, ARRAY_SIZE(msm8x60_lcdc_configs)},
	{msm8x60_hdmi_configs, ARRAY_SIZE(msm8x60_hdmi_configs)},
	{msm8x60_pmic_configs, ARRAY_SIZE(msm8x60_pmic_configs)},
};

static struct msm_gpiomux_cfg_block qrdc_cfgs[] __initdata = {
	{msm_qrdc_usb_configs, ARRAY_SIZE(msm_qrdc_usb_configs)},
	{msm_qrdc_sdc_configs, ARRAY_SIZE(msm_qrdc_sdc_configs)},
};

static int __init gpiomux_init(void)
{
	int rc = 0;
	unsigned n;

	rc = msm_gpiomux_init(NR_GPIO_IRQS);
	if (rc)
		return rc;

	for (n = 0; n < ARRAY_SIZE(msm8x60_cfgs); ++n)
		msm_gpiomux_install(msm8x60_cfgs[n].cfg, msm8x60_cfgs[n].ncfg);

	if (machine_is_msm8x60_qrdc()) {
		for (n = 0; n < ARRAY_SIZE(qrdc_cfgs); ++n) {
			msm_gpiomux_install(qrdc_cfgs[n].cfg,
					    qrdc_cfgs[n].ncfg);
		}
	}

	return rc;
}
postcore_initcall(gpiomux_init);
