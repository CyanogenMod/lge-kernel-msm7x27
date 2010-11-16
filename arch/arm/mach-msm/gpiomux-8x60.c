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

#define CONSOLE_UART	(GPIOMUX_FUNC_2 | GPIOMUX_DRV_8MA)

#ifdef CONFIG_I2C_QUP
#define GSBI3	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA)
#ifdef CONFIG_MSM_CAMERA
#define GSBI4	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA)
#else
#define GSBI4	0
#endif
#define GSBI7	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_12MA)
#define GSBI8	(GPIOMUX_FUNC_1)
#else
#define GSBI3	0
#define GSBI4	0
#define GSBI7	0
#define GSBI8	0
#endif

#if defined(CONFIG_SPI_QUP) || defined(CONFIG_SPI_QUP_MODULE)
#define GSBI1	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_8MA)
#else
#define GSBI1	0
#endif

#define PS_HOLD	(GPIOMUX_FUNC_1 | GPIOMUX_DRV_12MA)

#define USB_SWITCH_EN_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE)
#define USB_SWITCH_CNTL_ACTV_CFG	(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE)
#define USB_HUB_RESET_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_NONE)
#define USB_SWITCH_EN_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_PULL_DOWN)
#define USB_SWITCH_CNTL_SUSP_CFG	(GPIOMUX_FUNC_GPIO | GPIOMUX_PULL_DOWN)
#define USB_HUB_RESET_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_PULL_DOWN)
#define MSM_SNDDEV_ACTIVE_CONFIG	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_NONE |\
					 GPIOMUX_DRV_2MA)
#define MSM_SNDDEV_SUSPEND_CONFIG	(GPIOMUX_PULL_DOWN)

#define WLAN_PWDN_N_ACTV_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA |\
					 GPIOMUX_PULL_UP)
#define WLAN_PWDN_N_SUSP_CFG		(GPIOMUX_FUNC_GPIO | GPIOMUX_PULL_DOWN)

#define EBI2_A_D	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)
#define EBI2_OE		(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)
#define EBI2_WE		(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)
#define EBI2_CS2	(GPIOMUX_FUNC_2 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)
#define EBI2_CS3	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)
#define EBI2_ADV	(GPIOMUX_FUNC_1 | GPIOMUX_PULL_UP | GPIOMUX_DRV_8MA)

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	#define SDCC1_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_10MA)
#else
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC1_CLK_ACTV_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA)
#else
	#define SDCC1_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC1_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC1_CLK_ACTV_CFG 0
#endif

#define SDCC1_SUSPEND_CONFIG (GPIOMUX_PULL_UP)

#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	#define SDCC2_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC2_8_BIT_SUPPORT
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#else
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC2_CLK_ACTV_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_16MA)
#else
	#define SDCC2_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC2_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC2_CLK_ACTV_CFG 0
#endif

#define SDCC2_SUSPEND_CONFIG (GPIOMUX_PULL_DOWN)

#ifdef CONFIG_MMC_MSM_SDC5_SUPPORT
	#define SDCC5_DAT_0_3_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#ifdef CONFIG_MMC_MSM_SDC5_8_BIT_SUPPORT
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG (GPIOMUX_PULL_UP\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_10MA)
#else
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG 0
#endif
	#define SDCC5_CLK_ACTV_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_2 | GPIOMUX_DRV_16MA)
#else
	#define SDCC5_DAT_0_3_CMD_ACTV_CFG 0
	#define SDCC5_DAT_4_7_CMD_ACTV_CFG 0
	#define SDCC5_CLK_ACTV_CFG 0
#endif

#define SDCC5_SUSPEND_CONFIG (GPIOMUX_PULL_DOWN)

#define AUX_PCM_ACTIVE_CONFIG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_2MA)

#define AUX_PCM_SUSPEND_CONFIG    (GPIOMUX_PULL_NONE)

#ifdef CONFIG_SERIAL_MSM_HS
#define UART1DM_ACTIVE     (GPIOMUX_FUNC_1 |\
			    GPIOMUX_DRV_8MA | GPIOMUX_PULL_NONE)
#define UART1DM_SUSPENDED  (GPIOMUX_FUNC_GPIO |\
			    GPIOMUX_DRV_2MA | GPIOMUX_PULL_DOWN)
#else
#define UART1DM_ACTIVE     0
#define UART1DM_SUSPENDED  0
#endif

#define MI2S_ACTIVE_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_2MA)

#define MI2S_SUSPEND_CFG (GPIOMUX_PULL_DOWN)

#define LCDC_ACTIVE_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_1 | GPIOMUX_DRV_16MA)

#define LCDC_SUSPEND_CFG (GPIOMUX_PULL_DOWN)
#define HDMI_SUSPEND_CFG (GPIOMUX_PULL_DOWN)


#define MDM2AP_ACTIVE_CFG (GPIOMUX_PULL_NONE\
					| GPIOMUX_FUNC_GPIO | GPIOMUX_DRV_2MA)

#define MDM2AP_SUSPEND_CFG (GPIOMUX_PULL_NONE)

static struct msm_gpiomux_config msm8x60_gsbi_configs[] __initdata = {
	{
		.gpio      = 33,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI1,
		},
	},
	{
		.gpio      = 34,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI1,
		},
	},
	{
		.gpio      = 35,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI1,
		},
	},
	{
		.gpio      = 36,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI1,
		},
	},
	{
		.gpio      = 43,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI3,
		},
	},
	{
		.gpio      = 44,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI3,
		},
	},
	{
		.gpio      = 47,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI4,
		},
	},
	{
		.gpio      = 48,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI4,
		},
	},
	{
		.gpio      = 59,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI7,
		},
	},
	{
		.gpio      = 60,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI7,
		},
	},
	{
		.gpio      = 64,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI8,
		},
	},
	{
		.gpio      = 65,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GSBI8,
		},
	},
};

static struct msm_gpiomux_config msm8x60_ebi2_configs[] __initdata = {
	{
		.gpio      = 40,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_CS2,
		},
	},
	{
		.gpio      = 92,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = PS_HOLD,
		},
	},
	{
		.gpio      = 123,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 124,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 125,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 126,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 127,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 128,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 129,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 130,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 133,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_CS3,
		},
	},
	{
		.gpio      = 135,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 136,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 137,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 138,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 139,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 140,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 141,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 142,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 143,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 144,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 145,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 146,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 147,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 148,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 149,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 150,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_A_D,
		},
	},
	{
		.gpio      = 151,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_OE,
		},
	},
	{
		.gpio      = 153,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_ADV,
		},
	},
	{
		.gpio      = 157,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = EBI2_WE,
		},
	},
};

static struct msm_gpiomux_config msm8x60_uart_configs[] __initdata = {
	{ /* UARTDM_TX */
		.gpio      = 53,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = UART1DM_ACTIVE,
			[GPIOMUX_CFG_SUSPENDED] = UART1DM_SUSPENDED,
		},
	},
	{ /* UARTDM_RX */
		.gpio      = 54,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = UART1DM_ACTIVE,
			[GPIOMUX_CFG_SUSPENDED] = UART1DM_SUSPENDED,
		},
	},
	{ /* UARTDM_CTS */
		.gpio      = 55,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = UART1DM_ACTIVE,
			[GPIOMUX_CFG_SUSPENDED] = UART1DM_SUSPENDED,
		},
	},
	{ /* UARTDM_RFR */
		.gpio      = 56,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = UART1DM_ACTIVE,
			[GPIOMUX_CFG_SUSPENDED] = UART1DM_SUSPENDED,
		},
	},
	{
		.gpio      = 115,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = CONSOLE_UART,
		},
	},
	{
		.gpio      = 116,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = CONSOLE_UART,
		},
	},
	{
		.gpio      = 117,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = CONSOLE_UART,
		},
	},
	{
		.gpio      = 118,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = CONSOLE_UART,
		},
	},
};

static struct msm_gpiomux_config msm8x60_ts_configs[] __initdata = {
	{
		/* TS_ATTN */
		.gpio = 58,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GPIOMUX_PULL_DOWN,
		},
	},
};

static struct msm_gpiomux_config msm8x60_tmg200_configs[] __initdata = {
	{
		.gpio = 61,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = GPIOMUX_PULL_NONE |
				GPIOMUX_DRV_2MA | GPIOMUX_FUNC_GPIO,
			[GPIOMUX_CFG_SUSPENDED] = GPIOMUX_PULL_NONE,
		},
	},
};

static struct msm_gpiomux_config msm8x60_tma300_configs[] __initdata = {
	{
		.gpio = 61,
		.configs = {
			[GPIOMUX_CFG_ACTIVE] = GPIOMUX_PULL_UP |
				GPIOMUX_DRV_6MA | GPIOMUX_FUNC_GPIO,
			[GPIOMUX_CFG_SUSPENDED] = GPIOMUX_PULL_NONE,
		},
	},
};

static struct msm_gpiomux_config msm8x60_aux_pcm_configs[] __initdata = {
	{
		.gpio = 111,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = AUX_PCM_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = AUX_PCM_SUSPEND_CONFIG,
		},
	},
	{
		.gpio = 112,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = AUX_PCM_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = AUX_PCM_SUSPEND_CONFIG,
		},
	},
	{
		.gpio = 113,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = AUX_PCM_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = AUX_PCM_SUSPEND_CONFIG,
		},
	},
	{
		.gpio = 114,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = AUX_PCM_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = AUX_PCM_SUSPEND_CONFIG,
		},
	},
};

static struct msm_gpiomux_config msm8x60_sdc_configs[] __initdata = {
	/* SDCC1 data[0] */
	{
		.gpio = 159,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_0_3_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[1] */
	{
		.gpio = 160,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_0_3_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[2] */
	{
		.gpio = 161,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_0_3_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[3] */
	{
		.gpio = 162,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_0_3_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[4] */
	{
		.gpio = 163,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_4_7_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[5] */
	{
		.gpio = 164,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_4_7_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[6] */
	{
		.gpio = 165,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_4_7_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 data[7] */
	{
		.gpio = 166,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_4_7_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 CLK */
	{
		.gpio = 167,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_CLK_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
	/* SDCC1 CMD */
	{
		.gpio = 168,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = SDCC1_DAT_0_3_CMD_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = SDCC1_SUSPEND_CONFIG,
		},
	},
};

static struct msm_gpiomux_config msm_qrdc_sdc_configs[] __initdata = {
	{
		.gpio      = 118,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = WLAN_PWDN_N_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = WLAN_PWDN_N_SUSP_CFG,
		},
	},
};

static struct msm_gpiomux_config msm_qrdc_usb_configs[] __initdata = {
	{
		.gpio      = 34,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = USB_HUB_RESET_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = USB_HUB_RESET_SUSP_CFG,
		},
	},
	{
		.gpio      = 131,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = USB_SWITCH_CNTL_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = USB_SWITCH_CNTL_SUSP_CFG,
		},
	},
	{
		.gpio      = 132,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = USB_SWITCH_EN_ACTV_CFG,
			[GPIOMUX_CFG_SUSPENDED] = USB_SWITCH_EN_SUSP_CFG,
		},
	},
};

static struct msm_gpiomux_config msm8x60_snd_configs[] __initdata = {
	{
		.gpio = 108,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MSM_SNDDEV_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = MSM_SNDDEV_SUSPEND_CONFIG,
		},
	},
	{
		.gpio = 109,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MSM_SNDDEV_ACTIVE_CONFIG,
			[GPIOMUX_CFG_SUSPENDED] = MSM_SNDDEV_SUSPEND_CONFIG,
		},
	},
};

static struct msm_gpiomux_config msm8x60_mi2s_configs[] __initdata = {
	/* MI2S WS */
	{
		.gpio = 101,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MI2S_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = MI2S_SUSPEND_CFG,
		},
	},
	/* MI2S SCLK */
	{
		.gpio = 102,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MI2S_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = MI2S_SUSPEND_CFG,
		},
	},
	/* MI2S MCLK */
	{
		.gpio = 103,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MI2S_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = MI2S_SUSPEND_CFG,
		},
	},
	/* MI2S SD3 */
	{
		.gpio = 107,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MI2S_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = MI2S_SUSPEND_CFG,
		},
	},
};

static struct msm_gpiomux_config msm8x60_lcdc_configs[] __initdata = {
	/* lcdc_pclk */
	{
		.gpio = 0,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_hsync */
	{
		.gpio = 1,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_vsync */
	{
		.gpio = 2,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_den */
	{
		.gpio = 3,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red7 */
	{
		.gpio = 4,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red6 */
	{
		.gpio = 5,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red5 */
	{
		.gpio = 6,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red4 */
	{
		.gpio = 7,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red3 */
	{
		.gpio = 8,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red2 */
	{
		.gpio = 9,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red1 */
	{
		.gpio = 10,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_red0 */
	{
		.gpio = 11,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn7 */
	{
		.gpio = 12,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn6 */
	{
		.gpio = 13,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn5 */
	{
		.gpio = 14,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn4 */
	{
		.gpio = 15,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn3 */
	{
		.gpio = 16,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn2 */
	{
		.gpio = 17,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn1 */
	{
		.gpio = 18,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_grn0 */
	{
		.gpio = 19,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu7 */
	{
		.gpio = 20,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu6 */
	{
		.gpio = 21,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu5 */
	{
		.gpio = 22,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu4 */
	{
		.gpio = 23,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu3 */
	{
		.gpio = 24,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu2 */
	{
		.gpio = 25,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu1 */
	{
		.gpio = 26,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
	/* lcdc_blu0 */
	{
		.gpio = 27,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = LCDC_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = LCDC_SUSPEND_CFG,
		},
	},
};

static struct msm_gpiomux_config msm8x60_hdmi_configs[] __initdata = {
	{
		.gpio = 169,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = GPIOMUX_FUNC_1 |
				GPIOMUX_PULL_UP,
			[GPIOMUX_CFG_SUSPENDED] = HDMI_SUSPEND_CFG,
		},
	},
	{
		.gpio = 170,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = GPIOMUX_FUNC_1 |
				GPIOMUX_DRV_16MA,
			[GPIOMUX_CFG_SUSPENDED] = HDMI_SUSPEND_CFG,
		},
	},
	{
		.gpio = 171,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = GPIOMUX_FUNC_1 |
				GPIOMUX_DRV_16MA,
			[GPIOMUX_CFG_SUSPENDED] = HDMI_SUSPEND_CFG,
		},
	},
	{
		.gpio = 172,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = GPIOMUX_FUNC_1 |
				GPIOMUX_PULL_DOWN,
			[GPIOMUX_CFG_SUSPENDED] = HDMI_SUSPEND_CFG,
		},
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
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GPIOMUX_FUNC_GPIO |
				GPIOMUX_DRV_2MA | GPIOMUX_PULL_NONE,
		},
	},
	{
		.gpio = 91,
		.configs = {
			[GPIOMUX_CFG_SUSPENDED] = GPIOMUX_FUNC_GPIO |
				GPIOMUX_DRV_2MA | GPIOMUX_PULL_NONE,
		},
	},
};


static struct msm_gpiomux_config msm8x60_common_configs[] __initdata = {
	/* MDM2AP_STATUS */
	{
		.gpio = 77,
		.configs = {
			[GPIOMUX_CFG_ACTIVE]    = MDM2AP_ACTIVE_CFG,
			[GPIOMUX_CFG_SUSPENDED] = MDM2AP_SUSPEND_CFG,
		},
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
	{msm8x60_ts_configs, ARRAY_SIZE(msm8x60_ts_configs)},
	{msm8x60_aux_pcm_configs, ARRAY_SIZE(msm8x60_aux_pcm_configs)},
	{msm8x60_sdc_configs, ARRAY_SIZE(msm8x60_sdc_configs)},
	{msm8x60_snd_configs, ARRAY_SIZE(msm8x60_snd_configs)},
	{msm8x60_mi2s_configs, ARRAY_SIZE(msm8x60_mi2s_configs)},
	{msm8x60_lcdc_configs, ARRAY_SIZE(msm8x60_lcdc_configs)},
	{msm8x60_hdmi_configs, ARRAY_SIZE(msm8x60_hdmi_configs)},
	{msm8x60_pmic_configs, ARRAY_SIZE(msm8x60_pmic_configs)},
	{msm8x60_common_configs, ARRAY_SIZE(msm8x60_common_configs)},
};

static struct msm_gpiomux_cfg_block qrdc_cfgs[] __initdata = {
	{msm_qrdc_usb_configs, ARRAY_SIZE(msm_qrdc_usb_configs)},
	{msm_qrdc_sdc_configs, ARRAY_SIZE(msm_qrdc_sdc_configs)},
};

static struct msm_gpiomux_cfg_block msm8x60_fluid_cfgs[] __initdata = {
	{msm8x60_tma300_configs, ARRAY_SIZE(msm8x60_tma300_configs)},
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

	if (machine_is_msm8x60_ffa() || machine_is_msm8x60_surf()) {
		msm_gpiomux_install(msm8x60_tmg200_configs,
				ARRAY_SIZE(msm8x60_tmg200_configs));
	}

	if (machine_is_msm8x60_qrdc()) {
		for (n = 0; n < ARRAY_SIZE(qrdc_cfgs); ++n) {
			msm_gpiomux_install(qrdc_cfgs[n].cfg,
					    qrdc_cfgs[n].ncfg);
		}
	}

	if (machine_is_msm8x60_fluid()) {
		for (n = 0; n < ARRAY_SIZE(msm8x60_fluid_cfgs); ++n) {
			msm_gpiomux_install(msm8x60_fluid_cfgs[n].cfg,
					    msm8x60_fluid_cfgs[n].ncfg);
		}
	}

	return rc;
}
postcore_initcall(gpiomux_init);
