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
 *
 */
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/mfd/msm-adie-codec.h>
#include <mach/qdsp6v2/audio_dev_ctl.h>
#include <mach/board.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>
#include "snddev_icodec.h"
#include "snddev_ecodec.h"
#include "timpani_profile_8x60.h"
#include "snddev_hdmi.h"
#include "apr_audio.h"

/* define the value for BT_SCO */

static struct adie_codec_action_unit iearpiece_48KHz_osr256_actions[] =
	EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_settings),
};

static struct snddev_icodec_data snddev_iearpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = 7,
	.profile = &iearpiece_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_rx_route_config,
	.pamp_off = msm_snddev_rx_route_deconfig,
	.max_voice_rx_vol[VOC_NB_INDEX] = -200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1700
};

static struct platform_device msm_iearpiece_device = {
	.name = "snddev_icodec",
	.id = 0,
	.dev = { .platform_data = &snddev_iearpiece_data },
};

static struct adie_codec_action_unit imic_48KHz_osr256_actions[] =
	AMIC_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry imic_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_settings,
	.setting_sz = ARRAY_SIZE(imic_settings),
};

static enum hsed_controller imic_pmctl_id[] = {PM_HSED_CONTROLLER_0};

static struct snddev_icodec_data snddev_imic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 1,
	.acdb_id = 4,
	.profile = &imic_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_imic_device = {
	.name = "snddev_icodec",
	.id = 1,
	.dev = { .platform_data = &snddev_imic_data },
};

static struct adie_codec_action_unit headset_ab_cpls_48KHz_osr256_actions[] =
	HEADSET_AB_CPLS_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_ab_cpls_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_ab_cpls_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(headset_ab_cpls_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile headset_ab_cpls_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_ab_cpls_settings,
	.setting_sz = ARRAY_SIZE(headset_ab_cpls_settings),
};

static struct snddev_icodec_data snddev_ihs_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.acdb_id = 10,
	.profile = &headset_ab_cpls_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
};

static struct platform_device msm_headset_ab_cpls_device = {
	.name = "snddev_icodec",
	.id = 34,
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data },
};

static struct adie_codec_action_unit ispkr_stereo_48KHz_osr256_actions[] =
	SPEAKER_PRI_STEREO_48000_OSR_256;

static struct adie_codec_hwsetting_entry ispkr_stereo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_stereo_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispkr_stereo_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispkr_stereo_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_stereo_settings,
	.setting_sz = ARRAY_SIZE(ispkr_stereo_settings),
};

static struct snddev_icodec_data snddev_ispkr_stereo_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.acdb_id = 15,
	.profile = &ispkr_stereo_profile,
	.channel_mode = 2,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_poweramp_on,
	.pamp_off = msm_snddev_poweramp_off,
	.max_voice_rx_vol[VOC_NB_INDEX] = -200,
	.min_voice_rx_vol[VOC_NB_INDEX] = -1700,
	.max_voice_rx_vol[VOC_WB_INDEX] = -200,
	.min_voice_rx_vol[VOC_WB_INDEX] = -1700
};

static struct platform_device msm_ispkr_stereo_device = {
	.name = "snddev_icodec",
	.id = 2,
	.dev = { .platform_data = &snddev_ispkr_stereo_data },
};

static struct adie_codec_action_unit idmic_mono_48KHz_osr256_actions[] =
	DMIC1_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry idmic_mono_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idmic_mono_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idmic_mono_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idmic_mono_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idmic_mono_settings,
	.setting_sz = ARRAY_SIZE(idmic_mono_settings),
};

static struct snddev_icodec_data snddev_ispkr_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.acdb_id = 11,
	.profile = &idmic_mono_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_dmic_power,
	.pamp_off = msm_snddev_disable_dmic_power,
};

static struct platform_device msm_ispkr_mic_device = {
	.name = "snddev_icodec",
	.id = 3,
	.dev = { .platform_data = &snddev_ispkr_mic_data },
};

static struct adie_codec_action_unit iearpiece_ffa_48KHz_osr256_actions[] =
	EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_ffa_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_ffa_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_ffa_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_ffa_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_ffa_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_ffa_settings),
};

static struct snddev_icodec_data snddev_iearpiece_ffa_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.acdb_id = 7,
	.profile = &iearpiece_ffa_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_rx_route_config,
	.pamp_off = msm_snddev_rx_route_deconfig,
	.max_voice_rx_vol[VOC_NB_INDEX] = -700,
	.min_voice_rx_vol[VOC_NB_INDEX] = -2200,
	.max_voice_rx_vol[VOC_WB_INDEX] = -1400,
	.min_voice_rx_vol[VOC_WB_INDEX] = -2900,
};

static struct platform_device msm_iearpiece_ffa_device = {
	.name = "snddev_icodec",
	.id = 19,
	.dev = { .platform_data = &snddev_iearpiece_ffa_data },
};

static struct snddev_icodec_data snddev_imic_ffa_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = PRIMARY_I2S_TX,
	.acdb_id = 4,
	.profile = &idmic_mono_profile,
	.channel_mode = 1,
	.pmctl_id = imic_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(imic_pmctl_id),
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_dmic_power,
	.pamp_off = msm_snddev_disable_dmic_power,
};

static struct platform_device msm_imic_ffa_device = {
	.name = "snddev_icodec",
	.id = 20,
	.dev = { .platform_data = &snddev_imic_ffa_data },
};

static struct adie_codec_action_unit idual_mic_endfire_8KHz_osr256_actions[] =
	DMIC1_PRI_STEREO_8000_OSR_256;

static struct adie_codec_hwsetting_entry idual_mic_endfire_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = idual_mic_endfire_8KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(idual_mic_endfire_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile idual_mic_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = idual_mic_endfire_settings,
	.setting_sz = ARRAY_SIZE(idual_mic_endfire_settings),
};

static enum hsed_controller idual_mic_endfire_pmctl_id[] = {
	PM_HSED_CONTROLLER_0, PM_HSED_CONTROLLER_2};

static struct snddev_icodec_data snddev_idual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx",
	.copp_id = PRIMARY_I2S_TX,
	.acdb_id = 6,
	.profile = &idual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pmctl_id = idual_mic_endfire_pmctl_id,
	.pmctl_id_sz = ARRAY_SIZE(idual_mic_endfire_pmctl_id),
	.pamp_on = msm_snddev_enable_dmic_power,
	.pamp_off = msm_snddev_disable_dmic_power,
};

static struct platform_device msm_handset_dual_mic_endfire_device = {
	.name = "snddev_icodec",
	.id = 12,
	.dev = { .platform_data = &snddev_idual_mic_endfire_data },
};


static struct snddev_hdmi_data snddev_hdmi_stereo_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "hdmi_stereo_rx",
	.copp_id = HDMI_RX,
	.acdb_id = 18,
	.channel_mode = 0,
	.default_sample_rate = 48000,
};

static struct platform_device msm_snddev_hdmi_stereo_rx_device = {
	.name = "snddev_hdmi",
	.id = 0,
	.dev = { .platform_data = &snddev_hdmi_stereo_rx_data },
};


static struct adie_codec_action_unit iheadset_mic_tx_osr256_actions[] =
	HEADSET_AMIC2_TX_MONO_PRI_OSR_256;

static struct adie_codec_hwsetting_entry iheadset_mic_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iheadset_mic_tx_osr256_actions,
		.action_sz = ARRAY_SIZE(iheadset_mic_tx_osr256_actions),
	}
};

static struct adie_codec_dev_profile iheadset_mic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = iheadset_mic_tx_settings,
	.setting_sz = ARRAY_SIZE(iheadset_mic_tx_settings),
};

static struct snddev_icodec_data snddev_headset_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.acdb_id = 8,
	.profile = &iheadset_mic_profile,
	.channel_mode = 1,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_headset_mic_device = {
	.name = "snddev_icodec",
	.id = 33,
	.dev = { .platform_data = &snddev_headset_mic_data },
};

static struct snddev_ecodec_data snddev_bt_sco_earpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_rx",
	.copp_id = PCM_RX,
	.acdb_id = 22,
	.channel_mode = 1,
};

static struct snddev_ecodec_data snddev_bt_sco_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_tx",
	.copp_id = PCM_TX,
	.acdb_id = 21,
	.channel_mode = 1,
};

struct platform_device msm_bt_sco_earpiece_device = {
	.name = "msm_snddev_ecodec",
	.id = 0,
	.dev = { .platform_data = &snddev_bt_sco_earpiece_data },
};

struct platform_device msm_bt_sco_mic_device = {
	.name = "msm_snddev_ecodec",
	.id = 1,
	.dev = { .platform_data = &snddev_bt_sco_mic_data },
};

static struct adie_codec_action_unit itty_mono_tx_actions[] =
	TTY_HEADSET_MONO_TX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_tx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_tx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = itty_mono_tx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_tx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.acdb_id = 16,
	.profile = &itty_mono_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pmctl_id = NULL,
	.pmctl_id_sz = 0,
	.pamp_on = msm_snddev_tx_route_config,
	.pamp_off = msm_snddev_tx_route_deconfig,
};

static struct platform_device msm_itty_mono_tx_device = {
	.name = "snddev_icodec",
	.id = 16,
	.dev = { .platform_data = &snddev_itty_mono_tx_data },
};

static struct adie_codec_action_unit itty_mono_rx_actions[] =
	TTY_HEADSET_MONO_RX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_rx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_rx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = itty_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_rx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_rx",
	.copp_id = PRIMARY_I2S_RX,
	.acdb_id = 17,
	.profile = &itty_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_rx_route_config,
	.pamp_off = msm_snddev_rx_route_deconfig,
};

static struct platform_device msm_itty_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 17,
	.dev = { .platform_data = &snddev_itty_mono_rx_data },
};

static struct platform_device *snd_devices_ffa[] __initdata = {
	&msm_iearpiece_ffa_device,
	&msm_imic_ffa_device,
	&msm_ispkr_stereo_device,
	&msm_snddev_hdmi_stereo_rx_device,
	&msm_headset_mic_device,
	&msm_ispkr_mic_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_headset_ab_cpls_device,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_handset_dual_mic_endfire_device,
};

static struct platform_device *snd_devices_surf[] __initdata = {
	&msm_iearpiece_device,
	&msm_imic_device,
	&msm_ispkr_stereo_device,
	&msm_snddev_hdmi_stereo_rx_device,
	&msm_headset_mic_device,
	&msm_ispkr_mic_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_headset_ab_cpls_device,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_handset_dual_mic_endfire_device,
};

void __init msm_snddev_init(void)
{
	/* Auto detect device base on machine info */
	if (machine_is_msm8x60_surf()) {
		platform_add_devices(snd_devices_surf,
		ARRAY_SIZE(snd_devices_surf));
	} else if (machine_is_msm8x60_ffa()) {
		platform_add_devices(snd_devices_ffa,
		ARRAY_SIZE(snd_devices_ffa));
	}
}
