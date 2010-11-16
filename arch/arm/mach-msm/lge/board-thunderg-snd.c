/* linux/arch/arm/mach-msm/lge/board-thunderg-snd.c
 *
 * Copyright (C) 2010 LGE.
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
#include <mach/board.h>
#include "board-thunderg.h"

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
#if 0
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(TTY_HEADSET, 8),
	SND(TTY_VCO, 9),
	SND(TTY_HCO, 10),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(CURRENT, 27),
#else
/* LGE_CHANGE_S, [junyoub.an] , 2010-02-12, Define sound device*/
	SND(HANDSET_LOOPBACK,5),
	SND(HANDSET, 6),
	SND(HEADSET_LOOPBACK, 1),
	SND(HEADSET, 2),
	SND(HEADSET_STEREO, 3),
	SND(SPEAKER, 0),
	SND(SPEAKER_IN_CALL, 7),
	SND(SPEAKER_RING, 8),
	SND(HEADSET_AND_SPEAKER, 8),
	SND(FM_HEADSET, 10),
	SND(FM_SPEAKER, 11),
	SND(BT, 13),
	SND(TTY_HEADSET, 15),
	SND(TTY_VCO, 16),
	SND(TTY_HCO, 17),
	SND(TTY_HCO_SPEAKER, 18),
	SND(HANDSET_VR, 20),
	SND(HEADSET_VR, 21),
	SND(BT_VR, 23),
	SND(CURRENT, 30),
/* LGE_CHANGE_E, [junyoub.an] , 2010-02-12, Define sound device*/

#endif
};
#undef SND

static struct msm_snd_endpoints msm_device_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

struct platform_device msm_device_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_snd_endpoints
	},
};

#define DEC0_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
		    (1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
		    (1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
		    (1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
		    (1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
		    (1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC1_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
		    (1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
		    (1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
		    (1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
		    (1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
		    (1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC2_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
		    (1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
		    (1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
		    (1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
		    (1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
		    (1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC3_FORMAT ((1<<MSM_ADSP_CODEC_MP3)| \
		    (1<<MSM_ADSP_CODEC_AAC)|(1<<MSM_ADSP_CODEC_WMA)| \
		    (1<<MSM_ADSP_CODEC_WMAPRO)|(1<<MSM_ADSP_CODEC_AMRWB)| \
		    (1<<MSM_ADSP_CODEC_AMRNB)|(1<<MSM_ADSP_CODEC_WAV)| \
		    (1<<MSM_ADSP_CODEC_ADPCM)|(1<<MSM_ADSP_CODEC_YADPCM)| \
		    (1<<MSM_ADSP_CODEC_EVRC)|(1<<MSM_ADSP_CODEC_QCELP))
#define DEC4_FORMAT (1<<MSM_ADSP_CODEC_MIDI)

static unsigned int dec_concurrency_table[] = {
	/* Audio LP */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DMA)), 0,
	0, 0, 0,

	/* Concurrency 1 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	 /* Concurrency 2 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 3 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 4 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 5 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_TUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),

	/* Concurrency 6 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	0, 0, 0, 0,

	/* Concurrency 7 */
	(DEC0_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC1_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC2_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC3_FORMAT|(1<<MSM_ADSP_MODE_NONTUNNEL)|(1<<MSM_ADSP_OP_DM)),
	(DEC4_FORMAT),
};

#define DEC_INFO(name, queueid, decid, nr_codec) { .module_name = name, \
	.module_queueid = queueid, .module_decid = decid, \
	.nr_codec_support = nr_codec}

static struct msm_adspdec_info dec_info_list[] = {
	DEC_INFO("AUDPLAY0TASK", 13, 0, 11), /* AudPlay0BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY1TASK", 14, 1, 11),  /* AudPlay1BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY2TASK", 15, 2, 11),  /* AudPlay2BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY3TASK", 16, 3, 11),  /* AudPlay3BitStreamCtrlQueue */
	DEC_INFO("AUDPLAY4TASK", 17, 4, 1),  /* AudPlay4BitStreamCtrlQueue */
};

static struct msm_adspdec_database msm_device_adspdec_database = {
	.num_dec = ARRAY_SIZE(dec_info_list),
	.num_concurrency_support = (ARRAY_SIZE(dec_concurrency_table) / \
					ARRAY_SIZE(dec_info_list)),
	.dec_concurrency_table = dec_concurrency_table,
	.dec_info_list = dec_info_list,
};

struct platform_device msm_device_adspdec = {
	.name = "msm_adspdec",
	.id = -1,
	.dev    = {
		.platform_data = &msm_device_adspdec_database
	},
};
