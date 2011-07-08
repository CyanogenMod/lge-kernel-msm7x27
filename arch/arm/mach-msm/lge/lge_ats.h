/* arch/arm/mach-msm/lge/lge_ats.h
 *
 * Copyright (C) 2010 LGE, Inc.
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

#ifndef _LGE_ATS_ATCMD_H_
#define _LGE_ATS_ATCMD_H_

// at_cmd value start
// !!! must same with oncrpc_xdr_types.h (others see dsatfactory.h) in ARM9 (AMSS) side  !!!
//////////////////////////////////////////////////////////////////
#define ATCMD_AT	1
//Information Check
#define ATCMD_FRST	2
#define ATCMD_SWV	3
#define ATCMD_INFO	4
#define ATCMD_IMEI	5
#define ATCMD_IMPL	6
#define ATCMD_SLEN	7
#define ATCMD_SCHK	8
#define ATCMD_SULC	9
#define ATCMD_IDDE	10
#define ATCMD_HLRV	11
#define ATCMD_ISSIM	12
#define ATCMD_FUSG	13
#define ATCMD_DRMCERT	14
#define ATCMD_DRMTYPE	15
#define ATCMD_DRMERASE	16
#define ATCMD_INITDB	17
#define ATCMD_SVN	18
#define ATCMD_HWVER	19
// Calibration & Auto Test
#define ATCMD_CAMP	20
#define ATCMD_BNDI	21
#define ATCMD_CALM	22
#define ATCMD_NETORD	23
#define ATCMD_NETMODE	24
#define ATCMD_CALCK	25
#define ATCMD_CALDT	26
#define ATCMD_TESTCK	27
#define ATCMD_RADCK	28
#define ATCMD_RESTART	29
#define ATCMD_SFTUNE	30
// Call & Special Function Test
#define ATCMD_ACS	31
#define ATCMD_ECALL	32
#define ATCMD_FKPD	33
#define ATCMD_GKPD	34
#define ATCMD_MID	35
#define ATCMD_VLC	36
#define ATCMD_BATI	37
#define ATCMD_BATL	38
#define ATCMD_ANTB	39
#define ATCMD_SPM	40
#define ATCMD_FMT	41
#define ATCMD_FMR	42
#define ATCMD_MPT	43
#define ATCMD_MPC	44
#define ATCMD_AVR	45
#define ATCMD_EMT	46
#define ATCMD_INISIM	47
#define ATCMD_IRDA	48
#define ATCMD_PSENT	49
#define ATCMD_BATC	50
// Bluetooth Test	
#define ATCMD_BDCK	51
#define ATCMD_BTAD	52
#define ATCMD_BTTM	53 // hotaek 20070608 for BTTM
// DMB Test
#define ATCMD_DVBH	54
#define ATCMD_TDMB	55
#define ATCMD_CAS	56
// Misc
#define ATCMD_SYNC	57
#define ATCMD_BOFF	58//hotaek 2007/1/8 BOFF 
#define ATCMD_FC	59
#define ATCMD_FO	60
#define ATCMD_CONFIG	61
#define ATCMD_NDER	62
#define ATCMD_SPCK	63
#define ATCMD_SPUC	64
#define ATCMD_BCPL	65
#define ATCMD_SIMOFF	66
#define ATCMD_CGATT	67
#define ATCMD_RNO	68
#define ATCMD_LCD	69
#define ATCMD_CAM	70
#define ATCMD_MOT	71
// Wireless LAN
#define ATCMD_WLAN	72
#define ATCMD_WLANT	73
#define ATCMD_WLANR	74
// PMIC
#define ATCMD_PMRST	75
// Wireless LAN extention
#define ATCMD_WLANRF	76
#define ATCMD_WLANCAL	77
#define ATCMD_WLANWL	78

#define ATCMD_ATD	  79
#define ATCMD_MTC	  80 //LGE_CHANGE_S [seypark@lge.com] for AT+MTC

//songth - more drm command 
#define ATCMD_DRMINDEX		81 //LGE_CHANGE_S [kageki@lge.com] 

//LGE_UPDATE_S 2009.07.07 [seypark@lge.com]
#define ATCMD_FLIGHT		82
#define ATCMD_LANG			83
//LGE_UPDATE_E

// yorong drm command
#define ATCMD_DRMIMEI		84
#define ATCMD_POWERDOWN		85  // LGE_CHANGE [jinwoonam@lge.com] Powerdown system when no battery
#define ATCMD_PROXIMITY		95
#define ATCMD_ACCEL			96
#define ATCMD_COMPASS		97
#define ATCMD_MMCFORMAT    129
#define ATCMD_TOUCHFWVER   130
#define ATCMD_MMCFACTORYFORMAT	131	//LGE_UPDATE BCPARK 2010-10-19 for reset sdcard
#define ATCMD_LEDON		132		// LGE_UPDATE seungin.choi@lge.com 2011-04-01

//////////////////////////////////////////////////////////////////
// at_cmd value end

// at_act value  start
// !!! must same with dsatfactory.h in ARM9 (AMSS) side  !!!
//action/query/range/assign
//////////////////////////////////////////////////////////////////
#define ATCMD_ACTION	0
#define ATCMD_QUERY		1
#define ATCMD_RANGE		2
#define ATCMD_ASSIGN	3
//////////////////////////////////////////////////////////////////
// at_act value  end

// dsatHandleAT_ARM11 return Value
//////////////////////////////////////////////////////////////////
#define HANDLE_OK  0
#define HANDLE_FAIL 1
#define HANDLE_ERROR 2
#define HANDLE_OK_MIDDLE 4

struct rpc_ats_atcmd_args {
	uint32_t at_cmd;
	uint32_t at_act;
	uint32_t at_param;
};

struct rpc_ats_atcmd_eta_args {
	uint32_t at_cmd;
	uint32_t at_act;
	uint32_t sendNum;
	uint32_t endofBuffer;
	uint32_t buffersize;
	AT_SEND_BUFFER_t buffer[MAX_SEND_SIZE_BUFFER];
};

int lge_ats_handle_atcmd(struct msm_rpc_server *server,
							 struct rpc_request_hdr *req, unsigned len,
							 void (*update_atcmd_state)(char *cmd, int state));
int lge_ats_handle_atcmd_eta(struct msm_rpc_server *server,
								 struct rpc_request_hdr *req, unsigned len);

//LGE_CHAGE[irene.park@lge.com] 2010-06- 04 - to get flex value from ARM9 
int lge_ats_handle_flex(struct msm_rpc_server *server,
							 struct rpc_request_hdr *req, unsigned len);

#endif /* _LGE_ATS_ATCMD_H_ */
