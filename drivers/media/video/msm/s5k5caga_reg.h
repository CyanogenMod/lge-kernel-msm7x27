/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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

#ifndef S5K5CAGA_REG_H
#define S5K5CAGA_REG_H
/*************************************************************************/
/** lgcam_rear_sensor.c , 0xQualcommhardware.cpp 도 같이 change해라..*/
#define ALESSI_JPEG_IF_SET										0 // (0 : YUV mode , 0x1 : Jpeg mode )

/** lgcam_rear_sensor.c , 0xlgcam_rear_sensor.c(vendor), 0xQualcommHardware.cpp 도 같이 change해라..*/
#if ALESSI_JPEG_IF_SET
#define ALESSI_JPEG_USE_ADDRESS									1 // (0 : normal parsing , 0x1 : parsing to use address )
#else
#define ALESSI_JPEG_USE_ADDRESS									0
#endif
/***********************************************************/	

#include "s5k5caga.h"

static const struct lgcam_rear_sensor_i2c_reg_conf const pll_settings_array[] = {
	{0xFCFC, 0xD000,WORD_LEN},
	{0x0010, 0x0001,WORD_LEN},
	{0x1030, 0x0000,WORD_LEN},
	{0x0014, 0x0001,WORD_LEN},
};
static const struct lgcam_rear_sensor_i2c_reg_conf const init_settings_array[] = {

// #Start T&P part

// #DO NOT DELETE T&P SECTION COMMENTS! They are required to debug T&P related issues.

// #svn://transrdsrv/svn/svnroot/System/Software/tcevb/SDK+FW/ISP_5CA/Firmware

// #Rev: 32375-32375

// #Signature:

// #md5 78c1a0d32ef22ba270994f08d64a05a0 .btp

// #md5 6765ffc40fde4420aab81f0039a60c38 .htp

// #md5 956e8c724c34dd8b76dd297b92f59677 .RegsMap.h

// #md5 7db8e8f88de22128b8b909128f087a53 .RegsMap.bin

// #md5 506b4144bd48cdb79cbecdda4f7176ba .base.RegsMap.h

// #md5 fd8f92f13566c1a788746b23691c5f5f .base.RegsMap.bin

//

{0x0028, 0x7000, WORD_LEN},

{0x002A, 0x2CF8, WORD_LEN}, 



//mhlee

{0xFFFE,  0xB510, BURST_LEN}, 
{0x0,  0x4827, BURST_LEN}, 
{0x0,  0x21C0, BURST_LEN}, 
{0x0,  0x8041, BURST_LEN},  
{0x0,  0x4825, BURST_LEN},  
{0x0,  0x4A26, BURST_LEN},  
{0x0,  0x3020, BURST_LEN},  
{0x0,  0x8382, BURST_LEN},  
{0x0,  0x1D12, BURST_LEN},  
{0x0,  0x83C2, BURST_LEN},  
{0x0,  0x4822, BURST_LEN},  
{0x0,  0x3040, BURST_LEN},  
{0x0,  0x8041, BURST_LEN},  
{0x0,  0x4821, BURST_LEN},  
{0x0,  0x4922, BURST_LEN},  
{0x0,  0x3060, BURST_LEN},  
{0x0,  0x8381, BURST_LEN},  
{0x0,  0x1D09, BURST_LEN},  
{0x0,  0x83C1, BURST_LEN},  
{0x0,  0x4821, BURST_LEN},  
{0x0,  0x491D, BURST_LEN},  
{0x0,  0x8802, BURST_LEN},  
{0x0,  0x3980, BURST_LEN},  
{0x0,  0x804A, BURST_LEN},  
{0x0,  0x8842, BURST_LEN},  
{0x0,  0x808A, BURST_LEN},  
{0x0,  0x8882, BURST_LEN},  
{0x0,  0x80CA, BURST_LEN},  
{0x0,  0x88C2, BURST_LEN},  
{0x0,  0x810A, BURST_LEN},  
{0x0,  0x8902, BURST_LEN},  
{0x0,  0x491C, BURST_LEN},  
{0x0,  0x80CA, BURST_LEN},  
{0x0,  0x8942, BURST_LEN},  
{0x0,  0x814A, BURST_LEN},  
{0x0,  0x8982, BURST_LEN},  
{0x0,  0x830A, BURST_LEN},  
{0x0,  0x89C2, BURST_LEN},  
{0x0,  0x834A, BURST_LEN},  
{0x0,  0x8A00, BURST_LEN},  
{0x0,  0x4918, BURST_LEN},  
{0x0,  0x8188, BURST_LEN},  
{0x0,  0x4918, BURST_LEN},  
{0x0,  0x4819, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xFA0E, BURST_LEN},  
{0x0,  0x4918, BURST_LEN},  
{0x0,  0x4819, BURST_LEN},  
{0x0,  0x6341, BURST_LEN},  
{0x0,  0x4919, BURST_LEN},  
{0x0,  0x4819, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xFA07, BURST_LEN},  
{0x0,  0x4816, BURST_LEN},  
{0x0,  0x4918, BURST_LEN},  
{0x0,  0x3840, BURST_LEN},  
{0x0,  0x62C1, BURST_LEN},  
{0x0,  0x4918, BURST_LEN},  
{0x0,  0x3880, BURST_LEN},  
{0x0,  0x63C1, BURST_LEN},  
{0x0,  0x4917, BURST_LEN},  
{0x0,  0x6301, BURST_LEN},  
{0x0,  0x4917, BURST_LEN},  
{0x0,  0x3040, BURST_LEN},  
{0x0,  0x6181, BURST_LEN},  
{0x0,  0x4917, BURST_LEN},  
{0x0,  0x4817, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9F7, BURST_LEN},  
{0x0,  0x4917, BURST_LEN},  
{0x0,  0x4817, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9F3, BURST_LEN},  
{0x0,  0x4917, BURST_LEN},  
{0x0,  0x4817, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9EF, BURST_LEN},  
{0x0,  0xBC10, BURST_LEN},  
{0x0,  0xBC08, BURST_LEN},  
{0x0,  0x4718, BURST_LEN},  
{0x0,  0x1100, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x267C, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x2CE8, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x3274, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xF400, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0xF520, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x2DF1, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x89A9, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x2E43, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x0140, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2E7D, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xB4F7, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x2F07, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2F2B, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2FD1, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2FE5, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2FB9, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x013D, BURST_LEN},  
{0x0,  0x0001, BURST_LEN},  
{0x0,  0x306B, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x5823, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x30B9, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xD789, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0xB570, BURST_LEN},  
{0x0,  0x6804, BURST_LEN},  
{0x0,  0x6845, BURST_LEN},  
{0x0,  0x6881, BURST_LEN},  
{0x0,  0x6840, BURST_LEN},  
{0x0,  0x2900, BURST_LEN},  
{0x0,  0x6880, BURST_LEN},  
{0x0,  0xD007, BURST_LEN},  
{0x0,  0x49C3, BURST_LEN},  
{0x0,  0x8949, BURST_LEN},  
{0x0,  0x084A, BURST_LEN},  
{0x0,  0x1880, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9BA, BURST_LEN},  
{0x0,  0x80A0, BURST_LEN},  
{0x0,  0xE000, BURST_LEN},  
{0x0,  0x80A0, BURST_LEN},  
{0x0,  0x88A0, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD010, BURST_LEN},  
{0x0,  0x68A9, BURST_LEN},  
{0x0,  0x6828, BURST_LEN},  
{0x0,  0x084A, BURST_LEN},  
{0x0,  0x1880, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9AE, BURST_LEN},  
{0x0,  0x8020, BURST_LEN},  
{0x0,  0x1D2D, BURST_LEN},  
{0x0,  0xCD03, BURST_LEN},  
{0x0,  0x084A, BURST_LEN},  
{0x0,  0x1880, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9A7, BURST_LEN},  
{0x0,  0x8060, BURST_LEN},  
{0x0,  0xBC70, BURST_LEN},  
{0x0,  0xBC08, BURST_LEN},  
{0x0,  0x4718, BURST_LEN},  
{0x0,  0x2000, BURST_LEN},  
{0x0,  0x8060, BURST_LEN},  
{0x0,  0x8020, BURST_LEN},  
{0x0,  0xE7F8, BURST_LEN},  
{0x0,  0xB510, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF9A2, BURST_LEN},  
{0x0,  0x48B2, BURST_LEN},  
{0x0,  0x8A40, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD00C, BURST_LEN},  
{0x0,  0x48B1, BURST_LEN},  
{0x0,  0x49B2, BURST_LEN},  
{0x0,  0x8800, BURST_LEN},  
{0x0,  0x4AB2, BURST_LEN},  
{0x0,  0x2805, BURST_LEN},  
{0x0,  0xD003, BURST_LEN},  
{0x0,  0x4BB1, BURST_LEN},  
{0x0,  0x795B, BURST_LEN},  
{0x0,  0x2B00, BURST_LEN},  
{0x0,  0xD005, BURST_LEN},  
{0x0,  0x2001, BURST_LEN},  
{0x0,  0x8008, BURST_LEN},  
{0x0,  0x8010, BURST_LEN},  
{0x0,  0xBC10, BURST_LEN},  
{0x0,  0xBC08, BURST_LEN},  
{0x0,  0x4718, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD1FA, BURST_LEN},  
{0x0,  0x2000, BURST_LEN},  
{0x0,  0x8008, BURST_LEN},  
{0x0,  0x8010, BURST_LEN},  
{0x0,  0xE7F6, BURST_LEN},  
{0x0,  0xB5F8, BURST_LEN},  
{0x0,  0x2407, BURST_LEN},  
{0x0,  0x2C06, BURST_LEN},  
{0x0,  0xD035, BURST_LEN},  
{0x0,  0x2C07, BURST_LEN},  
{0x0,  0xD033, BURST_LEN},  
{0x0,  0x48A3, BURST_LEN},  
{0x0,  0x8BC1, BURST_LEN},  
{0x0,  0x2900, BURST_LEN},  
{0x0,  0xD02A, BURST_LEN},  
{0x0,  0x00A2, BURST_LEN},  
{0x0,  0x1815, BURST_LEN},  
{0x0,  0x4AA4, BURST_LEN},  
{0x0,  0x6DEE, BURST_LEN},  
{0x0,  0x8A92, BURST_LEN},  
{0x0,  0x4296, BURST_LEN},  
{0x0,  0xD923, BURST_LEN},  
{0x0,  0x0028, BURST_LEN},  
{0x0,  0x3080, BURST_LEN},  
{0x0,  0x0007, BURST_LEN},  
{0x0,  0x69C0, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF96B, BURST_LEN},  
{0x0,  0x1C71, BURST_LEN},  
{0x0,  0x0280, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF967, BURST_LEN},  
{0x0,  0x0006, BURST_LEN},  
{0x0,  0x4898, BURST_LEN},  
{0x0,  0x0061, BURST_LEN},  
{0x0,  0x1808, BURST_LEN},  
{0x0,  0x8D80, BURST_LEN},  
{0x0,  0x0A01, BURST_LEN},  
{0x0,  0x0600, BURST_LEN},  
{0x0,  0x0E00, BURST_LEN},  
{0x0,  0x1A08, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF96A, BURST_LEN},  
{0x0,  0x0002, BURST_LEN},  
{0x0,  0x6DE9, BURST_LEN},  
{0x0,  0x6FE8, BURST_LEN},  
{0x0,  0x1A08, BURST_LEN},  
{0x0,  0x4351, BURST_LEN},  
{0x0,  0x0300, BURST_LEN},  
{0x0,  0x1C49, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF953, BURST_LEN},  
{0x0,  0x0401, BURST_LEN},  
{0x0,  0x0430, BURST_LEN},  
{0x0,  0x0C00, BURST_LEN},  
{0x0,  0x4301, BURST_LEN},  
{0x0,  0x61F9, BURST_LEN},  
{0x0,  0xE004, BURST_LEN},  
{0x0,  0x00A2, BURST_LEN},  
{0x0,  0x4990, BURST_LEN},  
{0x0,  0x1810, BURST_LEN},  
{0x0,  0x3080, BURST_LEN},  
{0x0,  0x61C1, BURST_LEN},  
{0x0,  0x1E64, BURST_LEN},  
{0x0,  0xD2C5, BURST_LEN},  
{0x0,  0x2006, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF959, BURST_LEN},  
{0x0,  0x2007, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF956, BURST_LEN},  
{0x0,  0xBCF8, BURST_LEN},  
{0x0,  0xBC08, BURST_LEN},  
{0x0,  0x4718, BURST_LEN},  
{0x0,  0xB510, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF958, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD00A, BURST_LEN},  
{0x0,  0x4881, BURST_LEN},  
{0x0,  0x8B81, BURST_LEN},  
{0x0,  0x0089, BURST_LEN},  
{0x0,  0x1808, BURST_LEN},  
{0x0,  0x6DC1, BURST_LEN},  
{0x0,  0x4883, BURST_LEN},  
{0x0,  0x8A80, BURST_LEN},  
{0x0,  0x4281, BURST_LEN},  
{0x0,  0xD901, BURST_LEN},  
{0x0,  0x2001, BURST_LEN},  
{0x0,  0xE7A1, BURST_LEN},  
{0x0,  0x2000, BURST_LEN},  
{0x0,  0xE79F, BURST_LEN},  
{0x0,  0xB5F8, BURST_LEN},  
{0x0,  0x0004, BURST_LEN},  
{0x0,  0x4F80, BURST_LEN},  
{0x0,  0x227D, BURST_LEN},  
{0x0,  0x8938, BURST_LEN},  
{0x0,  0x0152, BURST_LEN},  
{0x0,  0x4342, BURST_LEN},  
{0x0,  0x487E, BURST_LEN},  
{0x0,  0x9000, BURST_LEN},  
{0x0,  0x8A01, BURST_LEN},  
{0x0,  0x0848, BURST_LEN},  
{0x0,  0x1810, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF91D, BURST_LEN},  
{0x0,  0x210F, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF940, BURST_LEN},  
{0x0,  0x497A, BURST_LEN},  
{0x0,  0x8C49, BURST_LEN},  
{0x0,  0x090E, BURST_LEN},  
{0x0,  0x0136, BURST_LEN},  
{0x0,  0x4306, BURST_LEN},  
{0x0,  0x4979, BURST_LEN},  
{0x0,  0x2C00, BURST_LEN},  
{0x0,  0xD003, BURST_LEN},  
{0x0,  0x2001, BURST_LEN},  
{0x0,  0x0240, BURST_LEN},  
{0x0,  0x4330, BURST_LEN},  
{0x0,  0x8108, BURST_LEN},  
{0x0,  0x4876, BURST_LEN},  
{0x0,  0x2C00, BURST_LEN},  
{0x0,  0x8D00, BURST_LEN},  
{0x0,  0xD001, BURST_LEN},  
{0x0,  0x2501, BURST_LEN},  
{0x0,  0xE000, BURST_LEN},  
{0x0,  0x2500, BURST_LEN},  
{0x0,  0x4972, BURST_LEN},  
{0x0,  0x4328, BURST_LEN},  
{0x0,  0x8008, BURST_LEN},  
{0x0,  0x207D, BURST_LEN},  
{0x0,  0x00C0, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF92E, BURST_LEN},  
{0x0,  0x2C00, BURST_LEN},  
{0x0,  0x496E, BURST_LEN},  
{0x0,  0x0328, BURST_LEN},  
{0x0,  0x4330, BURST_LEN},  
{0x0,  0x8108, BURST_LEN},  
{0x0,  0x88F8, BURST_LEN},  
{0x0,  0x2C00, BURST_LEN},  
{0x0,  0x01AA, BURST_LEN},  
{0x0,  0x4310, BURST_LEN},  
{0x0,  0x8088, BURST_LEN},  
{0x0,  0x9800, BURST_LEN},  
{0x0,  0x8A01, BURST_LEN},  
{0x0,  0x486A, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8F1, BURST_LEN},  
{0x0,  0x496A, BURST_LEN},  
{0x0,  0x8809, BURST_LEN},  
{0x0,  0x4348, BURST_LEN},  
{0x0,  0x0400, BURST_LEN},  
{0x0,  0x0C00, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF918, BURST_LEN},  
{0x0,  0x0020, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF91D, BURST_LEN},  
{0x0,  0x4866, BURST_LEN},  
{0x0,  0x7004, BURST_LEN},  
{0x0,  0xE7A3, BURST_LEN},  
{0x0,  0xB510, BURST_LEN},  
{0x0,  0x0004, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF91E, BURST_LEN},  
{0x0,  0x6020, BURST_LEN},  
{0x0,  0x4963, BURST_LEN},  
{0x0,  0x8B49, BURST_LEN},  
{0x0,  0x0789, BURST_LEN},  
{0x0,  0xD001, BURST_LEN},  
{0x0,  0x0040, BURST_LEN},  
{0x0,  0x6020, BURST_LEN},  
{0x0,  0xE74C, BURST_LEN},  
{0x0,  0xB510, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF91B, BURST_LEN},  
{0x0,  0x485F, BURST_LEN},  
{0x0,  0x8880, BURST_LEN},  
{0x0,  0x0601, BURST_LEN},  
{0x0,  0x4854, BURST_LEN},  
{0x0,  0x1609, BURST_LEN},  
{0x0,  0x8141, BURST_LEN},  
{0x0,  0xE742, BURST_LEN},  
{0x0,  0xB5F8, BURST_LEN},  
{0x0,  0x000F, BURST_LEN},  
{0x0,  0x4C55, BURST_LEN},  
{0x0,  0x3420, BURST_LEN},  
{0x0,  0x2500, BURST_LEN},  
{0x0,  0x5765, BURST_LEN},  
{0x0,  0x0039, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF913, BURST_LEN},  
{0x0,  0x9000, BURST_LEN},  
{0x0,  0x2600, BURST_LEN},  
{0x0,  0x57A6, BURST_LEN},  
{0x0,  0x4C4C, BURST_LEN},  
{0x0,  0x42AE, BURST_LEN},  
{0x0,  0xD01B, BURST_LEN},  
{0x0,  0x4D54, BURST_LEN},  
{0x0,  0x8AE8, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD013, BURST_LEN},  
{0x0,  0x484D, BURST_LEN},  
{0x0,  0x8A01, BURST_LEN},  
{0x0,  0x8B80, BURST_LEN},  
{0x0,  0x4378, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8B5, BURST_LEN},  
{0x0,  0x89A9, BURST_LEN},  
{0x0,  0x1A41, BURST_LEN},  
{0x0,  0x484E, BURST_LEN},  
{0x0,  0x3820, BURST_LEN},  
{0x0,  0x8AC0, BURST_LEN},  
{0x0,  0x4348, BURST_LEN},  
{0x0,  0x17C1, BURST_LEN},  
{0x0,  0x0D89, BURST_LEN},  
{0x0,  0x1808, BURST_LEN},  
{0x0,  0x1280, BURST_LEN},  
{0x0,  0x8961, BURST_LEN},  
{0x0,  0x1A08, BURST_LEN},  
{0x0,  0x8160, BURST_LEN},  
{0x0,  0xE003, BURST_LEN},  
{0x0,  0x88A8, BURST_LEN},  
{0x0,  0x0600, BURST_LEN},  
{0x0,  0x1600, BURST_LEN},  
{0x0,  0x8160, BURST_LEN},  
{0x0,  0x200A, BURST_LEN},  
{0x0,  0x5E20, BURST_LEN},  
{0x0,  0x42B0, BURST_LEN},  
{0x0,  0xD011, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8AB, BURST_LEN},  
{0x0,  0x1D40, BURST_LEN},  
{0x0,  0x00C3, BURST_LEN},  
{0x0,  0x1A18, BURST_LEN},  
{0x0,  0x214B, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF897, BURST_LEN},  
{0x0,  0x211F, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8BA, BURST_LEN},  
{0x0,  0x210A, BURST_LEN},  
{0x0,  0x5E61, BURST_LEN},  
{0x0,  0x0FC9, BURST_LEN},  
{0x0,  0x0149, BURST_LEN},  
{0x0,  0x4301, BURST_LEN},  
{0x0,  0x483D, BURST_LEN},  
{0x0,  0x81C1, BURST_LEN},  
{0x0,  0x9800, BURST_LEN},  
{0x0,  0xE74A, BURST_LEN},  
{0x0,  0xB5F1, BURST_LEN},  
{0x0,  0xB082, BURST_LEN},  
{0x0,  0x2500, BURST_LEN},  
{0x0,  0x483A, BURST_LEN},  
{0x0,  0x9001, BURST_LEN},  
{0x0,  0x2400, BURST_LEN},  
{0x0,  0x2028, BURST_LEN},  
{0x0,  0x4368, BURST_LEN},  
{0x0,  0x4A39, BURST_LEN},  
{0x0,  0x4925, BURST_LEN},  
{0x0,  0x1887, BURST_LEN},  
{0x0,  0x1840, BURST_LEN},  
{0x0,  0x9000, BURST_LEN},  
{0x0,  0x9800, BURST_LEN},  
{0x0,  0x0066, BURST_LEN},  
{0x0,  0x9A01, BURST_LEN},  
{0x0,  0x1980, BURST_LEN},  
{0x0,  0x218C, BURST_LEN},  
{0x0,  0x5A09, BURST_LEN},  
{0x0,  0x8A80, BURST_LEN},  
{0x0,  0x8812, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8CA, BURST_LEN},  
{0x0,  0x53B8, BURST_LEN},  
{0x0,  0x1C64, BURST_LEN},  
{0x0,  0x2C14, BURST_LEN},  
{0x0,  0xDBF1, BURST_LEN},  
{0x0,  0x1C6D, BURST_LEN},  
{0x0,  0x2D03, BURST_LEN},  
{0x0,  0xDBE6, BURST_LEN},  
{0x0,  0x9802, BURST_LEN},  
{0x0,  0x6800, BURST_LEN},  
{0x0,  0x0600, BURST_LEN},  
{0x0,  0x0E00, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8C5, BURST_LEN},  
{0x0,  0xBCFE, BURST_LEN},  
{0x0,  0xBC08, BURST_LEN},  
{0x0,  0x4718, BURST_LEN},  
{0x0,  0xB570, BURST_LEN},  
{0x0,  0x6805, BURST_LEN},  
{0x0,  0x2404, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8C5, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD103, BURST_LEN},  
{0x0,  0xF000, BURST_LEN},  
{0x0,  0xF8C9, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x2400, BURST_LEN},  
{0x0,  0x3540, BURST_LEN},  
{0x0,  0x88E8, BURST_LEN},  
{0x0,  0x0500, BURST_LEN},  
{0x0,  0xD403, BURST_LEN},  
{0x0,  0x4822, BURST_LEN},  
{0x0,  0x89C0, BURST_LEN},  
{0x0,  0x2800, BURST_LEN},  
{0x0,  0xD002, BURST_LEN},  
{0x0,  0x2008, BURST_LEN},  
{0x0,  0x4304, BURST_LEN},  
{0x0,  0xE001, BURST_LEN},  
{0x0,  0x2010, BURST_LEN},  
{0x0,  0x4304, BURST_LEN},  
{0x0,  0x481F, BURST_LEN},  
{0x0,  0x8B80, BURST_LEN},  
{0x0,  0x0700, BURST_LEN},  
{0x0,  0x0F81, BURST_LEN},  
{0x0,  0x2001, BURST_LEN},  
{0x0,  0x2900, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x4304, BURST_LEN},  
{0x0,  0x491C, BURST_LEN},  
{0x0,  0x8B0A, BURST_LEN},  
{0x0,  0x42A2, BURST_LEN},  
{0x0,  0xD004, BURST_LEN},  
{0x0,  0x0762, BURST_LEN},  
{0x0,  0xD502, BURST_LEN},  
{0x0,  0x4A19, BURST_LEN},  
{0x0,  0x3220, BURST_LEN},  
{0x0,  0x8110, BURST_LEN},  
{0x0,  0x830C, BURST_LEN},  
{0x0,  0xE691, BURST_LEN},  
{0x0,  0x0C3C, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x3274, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x26E8, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x6100, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x6500, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x1A7C, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x1120, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xFFFF, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x3374, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x1D6C, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x167C, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xF400, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x2C2C, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x40A0, BURST_LEN},  
{0x0,  0x00DD, BURST_LEN},  
{0x0,  0xF520, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x2C29, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x1A54, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x1564, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xF2A0, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x2440, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x05A0, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x2894, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0x1224, BURST_LEN},  
{0x0,  0x7000, BURST_LEN},  
{0x0,  0xB000, BURST_LEN},  
{0x0,  0xD000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x1A3F, BURST_LEN},  
{0x0,  0x0001, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xF004, BURST_LEN},  
{0x0,  0xE51F, BURST_LEN},  
{0x0,  0x1F48, BURST_LEN},  
{0x0,  0x0001, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x24BD, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x36DD, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xB4CF, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xB5D7, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x36ED, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xF53F, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xF5D9, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x013D, BURST_LEN},  
{0x0,  0x0001, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xF5C9, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xFAA9, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x3723, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0x5823, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xD771, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x4778, BURST_LEN},  
{0x0,  0x46C0, BURST_LEN},  
{0x0,  0xC000, BURST_LEN},  
{0x0,  0xE59F, BURST_LEN},  
{0x0,  0xFF1C, BURST_LEN},  
{0x0,  0xE12F, BURST_LEN},  
{0x0,  0xD75B, BURST_LEN},  
{0x0,  0x0000, BURST_LEN},  
{0x0,  0x8117, BURST_LEN},  
{0xFFFF,  0x0000, BURST_LEN},  



//

// #End T&P part



// #Start tuning part



//============================================================

// #CIS/APS/Analog setting- 400LSBSYSCLK 58MHz

//============================================================

// #This registers are for FACTORY ONLY. If you change it without prior notification

// #YOU are RESPONSIBLE for the FAILURE that will happen in the future.

//============================================================

{0x0028, 0x7000, WORD_LEN},

{0x002A, 0x157A, WORD_LEN},

{0x0F12, 0x0001, WORD_LEN},

{0x002A, 0x1578, WORD_LEN},

{0x0F12, 0x0001, WORD_LEN},

{0x002A, 0x1576, WORD_LEN},

{0x0F12, 0x0020, WORD_LEN},

{0x002A, 0x1574, WORD_LEN},

{0x0F12, 0x0006, WORD_LEN},

{0x002A, 0x156E, WORD_LEN},

{0x0F12, 0x0001, WORD_LEN},  // #Slope calibration tolerance in units of 1/256

{0x002A, 0x1568, WORD_LEN},

{0x0F12, 0x00FC, WORD_LEN},



//ADC control

{0x002A, 0x155A, WORD_LEN},

{0x0F12, 0x01CC, WORD_LEN},  //ADC SAT of 450mV for 10bit default in EVT1

{0x002A, 0x157E, WORD_LEN},

{0x0F12, 0x0C80, WORD_LEN},  // #3200 Max. Reset ramp DCLK counts (default 2048 800)

{0x0F12, 0x0578, WORD_LEN},  // #1400 Max. Reset ramp DCLK counts for x3.5

{0x002A, 0x157C, WORD_LEN},

{0x0F12, 0x0190, WORD_LEN},  // #400 Reset ramp for x1 in DCLK counts

{0x002A, 0x1570, WORD_LEN},

{0x0F12, 0x00A0, WORD_LEN},  // #256 LSB

{0x0F12, 0x0010, WORD_LEN},  // #reset threshold

{0x002A, 0x12C4, WORD_LEN},

{0x0F12, 0x006A, WORD_LEN},  // #106 additional timing columns.

{0x002A, 0x12C8, WORD_LEN},

{0x0F12, 0x08AC, WORD_LEN},  // #2220 ADC columns in normal mode including Hold & Latch

{0x0F12, 0x0050, WORD_LEN},  // #80 addition of ADC columns in Y-ave mode (default 244 74)

{0x002A, 0x1696, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  // #based on APS guidelines

{0x0F12, 0x0000, WORD_LEN},  // #based on APS guidelines

{0x0F12, 0x00C6, WORD_LEN},  // #default. 1492 used for ADC dark characteristics

{0x0F12, 0x00C6, WORD_LEN},

{0x002A, 0x12B8, WORD_LEN},

{0x0F12, 0x0100, WORD_LEN},  // #disable CINTR 0

{0x002A, 0x1690, WORD_LEN},

{0x0F12, 0x0001, WORD_LEN},  // #when set double sampling is activated - requires different set of pointers

{0x002A, 0x12B0, WORD_LEN},

{0x0F12, 0x0055, WORD_LEN},  // #comp and pixel bias control F40E - default for EVT1

{0x0F12, 0x005A, WORD_LEN},  // #comp and pixel bias control F40E for binning mode

{0x002A, 0x337A, WORD_LEN},

{0x0F12, 0x0006, WORD_LEN},  // #[7] - is used for rest-only mode (EVT0 value is D and HW 6)

{0x0F12, 0x0068, WORD_LEN},  // #104M

{0x002A, 0x327C, WORD_LEN},

{0x0F12, 0x1000, WORD_LEN},  // #[11]: Enable DBLR Regulation

{0x0F12, 0x6998, WORD_LEN},  // #[3:0]: VPIX ~2.8V ****

{0x0F12, 0x0078, WORD_LEN},  // #[0]: Static RC-filter

{0x0F12, 0x04FE, WORD_LEN},  // #[7:4]: Full RC-filter

{0x0F12, 0x8800, WORD_LEN},  // #[11]: Add load to CDS block

{0x002A, 0x3274, WORD_LEN},

{0x0F12, 0x0155, WORD_LEN},  //#Tune_TP_IO_DrivingCurrent_D0_D4_cs10Set IO driving current 

{0x0F12, 0x0155, WORD_LEN},  //#Tune_TP_IO_DrivingCurrent_D9_D5_cs10Set IO driving current

{0x0F12, 0x1555, WORD_LEN},  //#Tune_TP_IO_DrivingCurrent_GPIO_cd10 Set IO driving current

{0x0F12, 0x0555, WORD_LEN},  //#Tune_TP_IO_DrivingCurrent_CLKs_cd10 Set IO driving current

{0x002A, 0x169E, WORD_LEN},

{0x0F12, 0x0007, WORD_LEN},  // #[3:0]- specifies the target (default 7)- DCLK = 64MHz instead of 116MHz.

{0x002A, 0x0BF6, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  //from ALEX, BURST_LEN},  //Enable Bayer Downscaler



//Asserting CDS pointers - Long exposure MS Normal

// #Conditions: 10bit ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60

{0x0028, 0x7000, WORD_LEN},

{0x002A, 0x12D2, WORD_LEN},



//mhlee

{0xFFFE, 0x0003, BURST_LEN}, // #senHal_pContSenModesRegsArray[0][0] 2 700012D2

{0x0, 0x0003, BURST_LEN}, // #senHal_pContSenModesRegsArray[0][1] 2 700012D4

{0x0, 0x0003, BURST_LEN}, // #senHal_pContSenModesRegsArray[0][2] 2 700012D6

{0x0, 0x0003, BURST_LEN}, // #senHal_pContSenModesRegsArray[0][3] 2 700012D8

{0x0, 0x0884, BURST_LEN}, // #senHal_pContSenModesRegsArray[1][0] 2 700012DA

{0x0, 0x08CF, BURST_LEN}, // #senHal_pContSenModesRegsArray[1][1] 2 700012DC

{0x0, 0x0500, BURST_LEN}, // #senHal_pContSenModesRegsArray[1][2] 2 700012DE

{0x0, 0x054B, BURST_LEN}, // #senHal_pContSenModesRegsArray[1][3] 2 700012E0

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[2][0] 2 700012E2

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[2][1] 2 700012E4

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[2][2] 2 700012E6

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[2][3] 2 700012E8

{0x0, 0x0885, BURST_LEN}, // #senHal_pContSenModesRegsArray[3][0] 2 700012EA

{0x0, 0x0467, BURST_LEN}, // #senHal_pContSenModesRegsArray[3][1] 2 700012EC

{0x0, 0x0501, BURST_LEN}, // #senHal_pContSenModesRegsArray[3][2] 2 700012EE

{0x0, 0x02A5, BURST_LEN}, // #senHal_pContSenModesRegsArray[3][3] 2 700012F0

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[4][0] 2 700012F2

{0x0, 0x046A, BURST_LEN}, // #senHal_pContSenModesRegsArray[4][1] 2 700012F4

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[4][2] 2 700012F6

{0x0, 0x02A8, BURST_LEN}, // #senHal_pContSenModesRegsArray[4][3] 2 700012F8

{0x0, 0x0885, BURST_LEN}, // #senHal_pContSenModesRegsArray[5][0] 2 700012FA

{0x0, 0x08D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[5][1] 2 700012FC

{0x0, 0x0501, BURST_LEN}, // #senHal_pContSenModesRegsArray[5][2] 2 700012FE

{0x0, 0x054C, BURST_LEN}, // #senHal_pContSenModesRegsArray[5][3] 2 70001300

{0x0, 0x0006, BURST_LEN}, // #senHal_pContSenModesRegsArray[6][0] 2 70001302

{0x0, 0x0020, BURST_LEN}, // #senHal_pContSenModesRegsArray[6][1] 2 70001304

{0x0, 0x0006, BURST_LEN}, // #senHal_pContSenModesRegsArray[6][2] 2 70001306

{0x0, 0x0020, BURST_LEN}, // #senHal_pContSenModesRegsArray[6][3] 2 70001308

{0x0, 0x0881, BURST_LEN}, // #senHal_pContSenModesRegsArray[7][0] 2 7000130A

{0x0, 0x0463, BURST_LEN}, // #senHal_pContSenModesRegsArray[7][1] 2 7000130C

{0x0, 0x04FD, BURST_LEN}, // #senHal_pContSenModesRegsArray[7][2] 2 7000130E

{0x0, 0x02A1, BURST_LEN}, // #senHal_pContSenModesRegsArray[7][3] 2 70001310

{0x0, 0x0006, BURST_LEN}, // #senHal_pContSenModesRegsArray[8][0] 2 70001312

{0x0, 0x0489, BURST_LEN}, // #senHal_pContSenModesRegsArray[8][1] 2 70001314

{0x0, 0x0006, BURST_LEN}, // #senHal_pContSenModesRegsArray[8][2] 2 70001316

{0x0, 0x02C7, BURST_LEN}, // #senHal_pContSenModesRegsArray[8][3] 2 70001318

{0x0, 0x0881, BURST_LEN}, // #senHal_pContSenModesRegsArray[9][0] 2 7000131A

{0x0, 0x08CC, BURST_LEN}, // #senHal_pContSenModesRegsArray[9][1] 2 7000131C

{0x0, 0x04FD, BURST_LEN}, // #senHal_pContSenModesRegsArray[9][2] 2 7000131E

{0x0, 0x0548, BURST_LEN}, // #senHal_pContSenModesRegsArray[9][3] 2 70001320

{0x0, 0x03A2, BURST_LEN}, // #senHal_pContSenModesRegsArray[10][0]2 70001322

{0x0, 0x01D3, BURST_LEN}, // #senHal_pContSenModesRegsArray[10][1]2 70001324

{0x0, 0x01E0, BURST_LEN}, // #senHal_pContSenModesRegsArray[10][2]2 70001326

{0x0, 0x00F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[10][3]2 70001328

{0x0, 0x03F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[11][0]2 7000132A

{0x0, 0x0223, BURST_LEN}, // #senHal_pContSenModesRegsArray[11][1]2 7000132C

{0x0, 0x0230, BURST_LEN}, // #senHal_pContSenModesRegsArray[11][2]2 7000132E

{0x0, 0x0142, BURST_LEN}, // #senHal_pContSenModesRegsArray[11][3]2 70001330

{0x0, 0x03A2, BURST_LEN}, // #senHal_pContSenModesRegsArray[12][0]2 70001332

{0x0, 0x063C, BURST_LEN}, // #senHal_pContSenModesRegsArray[12][1]2 70001334

{0x0, 0x01E0, BURST_LEN}, // #senHal_pContSenModesRegsArray[12][2]2 70001336

{0x0, 0x0399, BURST_LEN}, // #senHal_pContSenModesRegsArray[12][3]2 70001338

{0x0, 0x03F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[13][0]2 7000133A

{0x0, 0x068C, BURST_LEN}, // #senHal_pContSenModesRegsArray[13][1]2 7000133C

{0x0, 0x0230, BURST_LEN}, // #senHal_pContSenModesRegsArray[13][2]2 7000133E

{0x0, 0x03E9, BURST_LEN}, // #senHal_pContSenModesRegsArray[13][3]2 70001340

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[14][0]2 70001342

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[14][1]2 70001344

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[14][2]2 70001346

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[14][3]2 70001348

{0x0, 0x003C, BURST_LEN}, // #senHal_pContSenModesRegsArray[15][0]2 7000134A

{0x0, 0x003C, BURST_LEN}, // #senHal_pContSenModesRegsArray[15][1]2 7000134C

{0x0, 0x003C, BURST_LEN}, // #senHal_pContSenModesRegsArray[15][2]2 7000134E

{0x0, 0x003C, BURST_LEN}, // #senHal_pContSenModesRegsArray[15][3]2 70001350

{0x0, 0x01D3, BURST_LEN}, // #senHal_pContSenModesRegsArray[16][0]2 70001352

{0x0, 0x01D3, BURST_LEN}, // #senHal_pContSenModesRegsArray[16][1]2 70001354

{0x0, 0x00F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[16][2]2 70001356

{0x0, 0x00F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[16][3]2 70001358

{0x0, 0x020B, BURST_LEN}, // #senHal_pContSenModesRegsArray[17][0]2 7000135A

{0x0, 0x024A, BURST_LEN}, // #senHal_pContSenModesRegsArray[17][1]2 7000135C

{0x0, 0x012A, BURST_LEN}, // #senHal_pContSenModesRegsArray[17][2]2 7000135E

{0x0, 0x0169, BURST_LEN}, // #senHal_pContSenModesRegsArray[17][3]2 70001360

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[18][0]2 70001362

{0x0, 0x046B, BURST_LEN}, // #senHal_pContSenModesRegsArray[18][1]2 70001364

{0x0, 0x0002, BURST_LEN}, // #senHal_pContSenModesRegsArray[18][2]2 70001366

{0x0, 0x02A9, BURST_LEN}, // #senHal_pContSenModesRegsArray[18][3]2 70001368

{0x0, 0x0419, BURST_LEN}, // #senHal_pContSenModesRegsArray[19][0]2 7000136A

{0x0, 0x04A5, BURST_LEN}, // #senHal_pContSenModesRegsArray[19][1]2 7000136C

{0x0, 0x0257, BURST_LEN}, // #senHal_pContSenModesRegsArray[19][2]2 7000136E

{0x0, 0x02E3, BURST_LEN}, // #senHal_pContSenModesRegsArray[19][3]2 70001370

{0x0, 0x0630, BURST_LEN}, // #senHal_pContSenModesRegsArray[20][0]2 70001372

{0x0, 0x063C, BURST_LEN}, // #senHal_pContSenModesRegsArray[20][1]2 70001374

{0x0, 0x038D, BURST_LEN}, // #senHal_pContSenModesRegsArray[20][2]2 70001376

{0x0, 0x0399, BURST_LEN}, // #senHal_pContSenModesRegsArray[20][3]2 70001378

{0x0, 0x0668, BURST_LEN}, // #senHal_pContSenModesRegsArray[21][0]2 7000137A

{0x0, 0x06B3, BURST_LEN}, // #senHal_pContSenModesRegsArray[21][1]2 7000137C

{0x0, 0x03C5, BURST_LEN}, // #senHal_pContSenModesRegsArray[21][2]2 7000137E

{0x0, 0x0410, BURST_LEN}, // #senHal_pContSenModesRegsArray[21][3]2 70001380

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[22][0]2 70001382

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[22][1]2 70001384

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[22][2]2 70001386

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[22][3]2 70001388

{0x0, 0x03A2, BURST_LEN}, // #senHal_pContSenModesRegsArray[23][0]2 7000138A

{0x0, 0x01D3, BURST_LEN}, // #senHal_pContSenModesRegsArray[23][1]2 7000138C

{0x0, 0x01E0, BURST_LEN}, // #senHal_pContSenModesRegsArray[23][2]2 7000138E

{0x0, 0x00F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[23][3]2 70001390

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[24][0]2 70001392

{0x0, 0x0461, BURST_LEN}, // #senHal_pContSenModesRegsArray[24][1]2 70001394

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[24][2]2 70001396

{0x0, 0x029F, BURST_LEN}, // #senHal_pContSenModesRegsArray[24][3]2 70001398

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[25][0]2 7000139A

{0x0, 0x063C, BURST_LEN}, // #senHal_pContSenModesRegsArray[25][1]2 7000139C

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[25][2]2 7000139E

{0x0, 0x0399, BURST_LEN}, // #senHal_pContSenModesRegsArray[25][3]2 700013A0

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[26][0]2 700013A2

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[26][1]2 700013A4

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[26][2]2 700013A6

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[26][3]2 700013A8

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[27][0]2 700013AA

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[27][1]2 700013AC

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[27][2]2 700013AE

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[27][3]2 700013B0

{0x0, 0x020C, BURST_LEN}, // #senHal_pContSenModesRegsArray[28][0]2 700013B2

{0x0, 0x024B, BURST_LEN}, // #senHal_pContSenModesRegsArray[28][1]2 700013B4

{0x0, 0x012B, BURST_LEN}, // #senHal_pContSenModesRegsArray[28][2]2 700013B6

{0x0, 0x016A, BURST_LEN}, // #senHal_pContSenModesRegsArray[28][3]2 700013B8

{0x0, 0x039F, BURST_LEN}, // #senHal_pContSenModesRegsArray[29][0]2 700013BA

{0x0, 0x045E, BURST_LEN}, // #senHal_pContSenModesRegsArray[29][1]2 700013BC

{0x0, 0x01DD, BURST_LEN}, // #senHal_pContSenModesRegsArray[29][2]2 700013BE

{0x0, 0x029C, BURST_LEN}, // #senHal_pContSenModesRegsArray[29][3]2 700013C0

{0x0, 0x041A, BURST_LEN}, // #senHal_pContSenModesRegsArray[30][0]2 700013C2

{0x0, 0x04A6, BURST_LEN}, // #senHal_pContSenModesRegsArray[30][1]2 700013C4

{0x0, 0x0258, BURST_LEN}, // #senHal_pContSenModesRegsArray[30][2]2 700013C6

{0x0, 0x02E4, BURST_LEN}, // #senHal_pContSenModesRegsArray[30][3]2 700013C8

{0x0, 0x062D, BURST_LEN}, // #senHal_pContSenModesRegsArray[31][0]2 700013CA

{0x0, 0x0639, BURST_LEN}, // #senHal_pContSenModesRegsArray[31][1]2 700013CC

{0x0, 0x038A, BURST_LEN}, // #senHal_pContSenModesRegsArray[31][2]2 700013CE

{0x0, 0x0396, BURST_LEN}, // #senHal_pContSenModesRegsArray[31][3]2 700013D0

{0x0, 0x0669, BURST_LEN}, // #senHal_pContSenModesRegsArray[32][0]2 700013D2

{0x0, 0x06B4, BURST_LEN}, // #senHal_pContSenModesRegsArray[32][1]2 700013D4

{0x0, 0x03C6, BURST_LEN}, // #senHal_pContSenModesRegsArray[32][2]2 700013D6

{0x0, 0x0411, BURST_LEN}, // #senHal_pContSenModesRegsArray[32][3]2 700013D8

{0x0, 0x087C, BURST_LEN}, // #senHal_pContSenModesRegsArray[33][0]2 700013DA

{0x0, 0x08C7, BURST_LEN}, // #senHal_pContSenModesRegsArray[33][1]2 700013DC

{0x0, 0x04F8, BURST_LEN}, // #senHal_pContSenModesRegsArray[33][2]2 700013DE

{0x0, 0x0543, BURST_LEN}, // #senHal_pContSenModesRegsArray[33][3]2 700013E0

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[34][0]2 700013E2

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[34][1]2 700013E4

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[34][2]2 700013E6

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[34][3]2 700013E8

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[35][0]2 700013EA

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[35][1]2 700013EC

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[35][2]2 700013EE

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[35][3]2 700013F0

{0x0, 0x020F, BURST_LEN}, // #senHal_pContSenModesRegsArray[36][0]2 700013F2

{0x0, 0x024E, BURST_LEN}, // #senHal_pContSenModesRegsArray[36][1]2 700013F4

{0x0, 0x012E, BURST_LEN}, // #senHal_pContSenModesRegsArray[36][2]2 700013F6

{0x0, 0x016D, BURST_LEN}, // #senHal_pContSenModesRegsArray[36][3]2 700013F8

{0x0, 0x039F, BURST_LEN}, // #senHal_pContSenModesRegsArray[37][0]2 700013FA

{0x0, 0x045E, BURST_LEN}, // #senHal_pContSenModesRegsArray[37][1]2 700013FC

{0x0, 0x01DD, BURST_LEN}, // #senHal_pContSenModesRegsArray[37][2]2 700013FE

{0x0, 0x029C, BURST_LEN}, // #senHal_pContSenModesRegsArray[37][3]2 70001400

{0x0, 0x041D, BURST_LEN}, // #senHal_pContSenModesRegsArray[38][0]2 70001402

{0x0, 0x04A9, BURST_LEN}, // #senHal_pContSenModesRegsArray[38][1]2 70001404

{0x0, 0x025B, BURST_LEN}, // #senHal_pContSenModesRegsArray[38][2]2 70001406

{0x0, 0x02E7, BURST_LEN}, // #senHal_pContSenModesRegsArray[38][3]2 70001408

{0x0, 0x062D, BURST_LEN}, // #senHal_pContSenModesRegsArray[39][0]2 7000140A

{0x0, 0x0639, BURST_LEN}, // #senHal_pContSenModesRegsArray[39][1]2 7000140C

{0x0, 0x038A, BURST_LEN}, // #senHal_pContSenModesRegsArray[39][2]2 7000140E

{0x0, 0x0396, BURST_LEN}, // #senHal_pContSenModesRegsArray[39][3]2 70001410

{0x0, 0x066C, BURST_LEN}, // #senHal_pContSenModesRegsArray[40][0]2 70001412

{0x0, 0x06B7, BURST_LEN}, // #senHal_pContSenModesRegsArray[40][1]2 70001414

{0x0, 0x03C9, BURST_LEN}, // #senHal_pContSenModesRegsArray[40][2]2 70001416

{0x0, 0x0414, BURST_LEN}, // #senHal_pContSenModesRegsArray[40][3]2 70001418

{0x0, 0x087C, BURST_LEN}, // #senHal_pContSenModesRegsArray[41][0]2 7000141A

{0x0, 0x08C7, BURST_LEN}, // #senHal_pContSenModesRegsArray[41][1]2 7000141C

{0x0, 0x04F8, BURST_LEN}, // #senHal_pContSenModesRegsArray[41][2]2 7000141E

{0x0, 0x0543, BURST_LEN}, // #senHal_pContSenModesRegsArray[41][3]2 70001420

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[42][0]2 70001422

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[42][1]2 70001424

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[42][2]2 70001426

{0x0, 0x0040, BURST_LEN}, // #senHal_pContSenModesRegsArray[42][3]2 70001428

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[43][0]2 7000142A

{0x0, 0x01D0, BURST_LEN}, // #senHal_pContSenModesRegsArray[43][1]2 7000142C

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[43][2]2 7000142E

{0x0, 0x00EF, BURST_LEN}, // #senHal_pContSenModesRegsArray[43][3]2 70001430

{0x0, 0x020F, BURST_LEN}, // #senHal_pContSenModesRegsArray[44][0]2 70001432

{0x0, 0x024E, BURST_LEN}, // #senHal_pContSenModesRegsArray[44][1]2 70001434

{0x0, 0x012E, BURST_LEN}, // #senHal_pContSenModesRegsArray[44][2]2 70001436

{0x0, 0x016D, BURST_LEN}, // #senHal_pContSenModesRegsArray[44][3]2 70001438

{0x0, 0x039F, BURST_LEN}, // #senHal_pContSenModesRegsArray[45][0]2 7000143A

{0x0, 0x045E, BURST_LEN}, // #senHal_pContSenModesRegsArray[45][1]2 7000143C

{0x0, 0x01DD, BURST_LEN}, // #senHal_pContSenModesRegsArray[45][2]2 7000143E

{0x0, 0x029C, BURST_LEN}, // #senHal_pContSenModesRegsArray[45][3]2 70001440

{0x0, 0x041D, BURST_LEN}, // #senHal_pContSenModesRegsArray[46][0]2 70001442

{0x0, 0x04A9, BURST_LEN}, // #senHal_pContSenModesRegsArray[46][1]2 70001444

{0x0, 0x025B, BURST_LEN}, // #senHal_pContSenModesRegsArray[46][2]2 70001446

{0x0, 0x02E7, BURST_LEN}, // #senHal_pContSenModesRegsArray[46][3]2 70001448

{0x0, 0x062D, BURST_LEN}, // #senHal_pContSenModesRegsArray[47][0]2 7000144A

{0x0, 0x0639, BURST_LEN}, // #senHal_pContSenModesRegsArray[47][1]2 7000144C

{0x0, 0x038A, BURST_LEN}, // #senHal_pContSenModesRegsArray[47][2]2 7000144E

{0x0, 0x0396, BURST_LEN}, // #senHal_pContSenModesRegsArray[47][3]2 70001450

{0x0, 0x066C, BURST_LEN}, // #senHal_pContSenModesRegsArray[48][0]2 70001452

{0x0, 0x06B7, BURST_LEN}, // #senHal_pContSenModesRegsArray[48][1]2 70001454

{0x0, 0x03C9, BURST_LEN}, // #senHal_pContSenModesRegsArray[48][2]2 70001456

{0x0, 0x0414, BURST_LEN}, // #senHal_pContSenModesRegsArray[48][3]2 70001458

{0x0, 0x087C, BURST_LEN}, // #senHal_pContSenModesRegsArray[49][0]2 7000145A

{0x0, 0x08C7, BURST_LEN}, // #senHal_pContSenModesRegsArray[49][1]2 7000145C

{0x0, 0x04F8, BURST_LEN}, // #senHal_pContSenModesRegsArray[49][2]2 7000145E

{0x0, 0x0543, BURST_LEN}, // #senHal_pContSenModesRegsArray[49][3]2 70001460

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[50][0]2 70001462

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[50][1]2 70001464

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[50][2]2 70001466

{0x0, 0x003D, BURST_LEN}, // #senHal_pContSenModesRegsArray[50][3]2 70001468

{0x0, 0x01D2, BURST_LEN}, // #senHal_pContSenModesRegsArray[51][0]2 7000146A

{0x0, 0x01D2, BURST_LEN}, // #senHal_pContSenModesRegsArray[51][1]2 7000146C

{0x0, 0x00F1, BURST_LEN}, // #senHal_pContSenModesRegsArray[51][2]2 7000146E

{0x0, 0x00F1, BURST_LEN}, // #senHal_pContSenModesRegsArray[51][3]2 70001470

{0x0, 0x020C, BURST_LEN}, // #senHal_pContSenModesRegsArray[52][0]2 70001472

{0x0, 0x024B, BURST_LEN}, // #senHal_pContSenModesRegsArray[52][1]2 70001474

{0x0, 0x012B, BURST_LEN}, // #senHal_pContSenModesRegsArray[52][2]2 70001476

{0x0, 0x016A, BURST_LEN}, // #senHal_pContSenModesRegsArray[52][3]2 70001478

{0x0, 0x03A1, BURST_LEN}, // #senHal_pContSenModesRegsArray[53][0]2 7000147A

{0x0, 0x0460, BURST_LEN}, // #senHal_pContSenModesRegsArray[53][1]2 7000147C

{0x0, 0x01DF, BURST_LEN}, // #senHal_pContSenModesRegsArray[53][2]2 7000147E

{0x0, 0x029E, BURST_LEN}, // #senHal_pContSenModesRegsArray[53][3]2 70001480

{0x0, 0x041A, BURST_LEN}, // #senHal_pContSenModesRegsArray[54][0]2 70001482

{0x0, 0x04A6, BURST_LEN}, // #senHal_pContSenModesRegsArray[54][1]2 70001484

{0x0, 0x0258, BURST_LEN}, // #senHal_pContSenModesRegsArray[54][2]2 70001486

{0x0, 0x02E4, BURST_LEN}, // #senHal_pContSenModesRegsArray[54][3]2 70001488

{0x0, 0x062F, BURST_LEN}, // #senHal_pContSenModesRegsArray[55][0]2 7000148A

{0x0, 0x063B, BURST_LEN}, // #senHal_pContSenModesRegsArray[55][1]2 7000148C

{0x0, 0x038C, BURST_LEN}, // #senHal_pContSenModesRegsArray[55][2]2 7000148E

{0x0, 0x0398, BURST_LEN}, // #senHal_pContSenModesRegsArray[55][3]2 70001490

{0x0, 0x0669, BURST_LEN}, // #senHal_pContSenModesRegsArray[56][0]2 70001492

{0x0, 0x06B4, BURST_LEN}, // #senHal_pContSenModesRegsArray[56][1]2 70001494

{0x0, 0x03C6, BURST_LEN}, // #senHal_pContSenModesRegsArray[56][2]2 70001496

{0x0, 0x0411, BURST_LEN}, // #senHal_pContSenModesRegsArray[56][3]2 70001498

{0x0, 0x087E, BURST_LEN}, // #senHal_pContSenModesRegsArray[57][0]2 7000149A

{0x0, 0x08C9, BURST_LEN}, // #senHal_pContSenModesRegsArray[57][1]2 7000149C

{0x0, 0x04FA, BURST_LEN}, // #senHal_pContSenModesRegsArray[57][2]2 7000149E

{0x0, 0x0545, BURST_LEN}, // #senHal_pContSenModesRegsArray[57][3]2 700014A0

{0x0, 0x03A2, BURST_LEN}, // #senHal_pContSenModesRegsArray[58][0]2 700014A2

{0x0, 0x01D3, BURST_LEN}, // #senHal_pContSenModesRegsArray[58][1]2 700014A4

{0x0, 0x01E0, BURST_LEN}, // #senHal_pContSenModesRegsArray[58][2]2 700014A6

{0x0, 0x00F2, BURST_LEN}, // #senHal_pContSenModesRegsArray[58][3]2 700014A8

{0x0, 0x03AF, BURST_LEN}, // #senHal_pContSenModesRegsArray[59][0]2 700014AA

{0x0, 0x01E0, BURST_LEN}, // #senHal_pContSenModesRegsArray[59][1]2 700014AC

{0x0, 0x01ED, BURST_LEN}, // #senHal_pContSenModesRegsArray[59][2]2 700014AE

{0x0, 0x00FF, BURST_LEN}, // #senHal_pContSenModesRegsArray[59][3]2 700014B0

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[60][0]2 700014B2

{0x0, 0x0461, BURST_LEN}, // #senHal_pContSenModesRegsArray[60][1]2 700014B4

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[60][2]2 700014B6

{0x0, 0x029F, BURST_LEN}, // #senHal_pContSenModesRegsArray[60][3]2 700014B8

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[61][0]2 700014BA

{0x0, 0x046E, BURST_LEN}, // #senHal_pContSenModesRegsArray[61][1]2 700014BC

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[61][2]2 700014BE

{0x0, 0x02AC, BURST_LEN}, // #senHal_pContSenModesRegsArray[61][3]2 700014C0

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[62][0]2 700014C2

{0x0, 0x063C, BURST_LEN}, // #senHal_pContSenModesRegsArray[62][1]2 700014C4

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[62][2]2 700014C6

{0x0, 0x0399, BURST_LEN}, // #senHal_pContSenModesRegsArray[62][3]2 700014C8

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[63][0]2 700014CA

{0x0, 0x0649, BURST_LEN}, // #senHal_pContSenModesRegsArray[63][1]2 700014CC

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[63][2]2 700014CE

{0x0, 0x03A6, BURST_LEN}, // #senHal_pContSenModesRegsArray[63][3]2 700014D0

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[64][0]2 700014D2

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[64][1]2 700014D4

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[64][2]2 700014D6

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[64][3]2 700014D8

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[65][0]2 700014DA

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[65][1]2 700014DC

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[65][2]2 700014DE

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[65][3]2 700014E0

{0x0, 0x03AA, BURST_LEN}, // #senHal_pContSenModesRegsArray[66][0]2 700014E2

{0x0, 0x01DB, BURST_LEN}, // #senHal_pContSenModesRegsArray[66][1]2 700014E4

{0x0, 0x01E8, BURST_LEN}, // #senHal_pContSenModesRegsArray[66][2]2 700014E6

{0x0, 0x00FA, BURST_LEN}, // #senHal_pContSenModesRegsArray[66][3]2 700014E8

{0x0, 0x03B7, BURST_LEN}, // #senHal_pContSenModesRegsArray[67][0]2 700014EA

{0x0, 0x01E8, BURST_LEN}, // #senHal_pContSenModesRegsArray[67][1]2 700014EC

{0x0, 0x01F5, BURST_LEN}, // #senHal_pContSenModesRegsArray[67][2]2 700014EE

{0x0, 0x0107, BURST_LEN}, // #senHal_pContSenModesRegsArray[67][3]2 700014F0

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[68][0]2 700014F2

{0x0, 0x0469, BURST_LEN}, // #senHal_pContSenModesRegsArray[68][1]2 700014F4

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[68][2]2 700014F6

{0x0, 0x02A7, BURST_LEN}, // #senHal_pContSenModesRegsArray[68][3]2 700014F8

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[69][0]2 700014FA

{0x0, 0x0476, BURST_LEN}, // #senHal_pContSenModesRegsArray[69][1]2 700014FC

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[69][2]2 700014FE

{0x0, 0x02B4, BURST_LEN}, // #senHal_pContSenModesRegsArray[69][3]2 70001500

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[70][0]2 70001502

{0x0, 0x0644, BURST_LEN}, // #senHal_pContSenModesRegsArray[70][1]2 70001504

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[70][2]2 70001506

{0x0, 0x03A1, BURST_LEN}, // #senHal_pContSenModesRegsArray[70][3]2 70001508

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[71][0]2 7000150A

{0x0, 0x0651, BURST_LEN}, // #senHal_pContSenModesRegsArray[71][1]2 7000150C

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[71][2]2 7000150E

{0x0, 0x03AE, BURST_LEN}, // #senHal_pContSenModesRegsArray[71][3]2 70001510

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[72][0]2 70001512

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[72][1]2 70001514

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[72][2]2 70001516

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[72][3]2 70001518

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[73][0]2 7000151A

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[73][1]2 7000151C

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[73][2]2 7000151E

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[73][3]2 70001520

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[74][0]2 70001522

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[74][1]2 70001524

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[74][2]2 70001526

{0x0, 0x0001, BURST_LEN}, // #senHal_pContSenModesRegsArray[74][3]2 70001528

{0x0, 0x000F, BURST_LEN}, // #senHal_pContSenModesRegsArray[75][0]2 7000152A

{0x0, 0x000F, BURST_LEN}, // #senHal_pContSenModesRegsArray[75][1]2 7000152C

{0x0, 0x000F, BURST_LEN}, // #senHal_pContSenModesRegsArray[75][2]2 7000152E

{0x0, 0x000F, BURST_LEN}, // #senHal_pContSenModesRegsArray[75][3]2 70001530

{0x0, 0x05AD, BURST_LEN}, // #senHal_pContSenModesRegsArray[76][0]2 70001532

{0x0, 0x03DE, BURST_LEN}, // #senHal_pContSenModesRegsArray[76][1]2 70001534

{0x0, 0x030A, BURST_LEN}, // #senHal_pContSenModesRegsArray[76][2]2 70001536

{0x0, 0x021C, BURST_LEN}, // #senHal_pContSenModesRegsArray[76][3]2 70001538

{0x0, 0x062F, BURST_LEN}, // #senHal_pContSenModesRegsArray[77][0]2 7000153A

{0x0, 0x0460, BURST_LEN}, // #senHal_pContSenModesRegsArray[77][1]2 7000153C

{0x0, 0x038C, BURST_LEN}, // #senHal_pContSenModesRegsArray[77][2]2 7000153E

{0x0, 0x029E, BURST_LEN}, // #senHal_pContSenModesRegsArray[77][3]2 70001540

{0x0, 0x07FC, BURST_LEN}, // #senHal_pContSenModesRegsArray[78][0]2 70001542

{0x0, 0x0847, BURST_LEN}, // #senHal_pContSenModesRegsArray[78][1]2 70001544

{0x0, 0x0478, BURST_LEN}, // #senHal_pContSenModesRegsArray[78][2]2 70001546

{0x0, 0x04C3, BURST_LEN}, // #senHal_pContSenModesRegsArray[78][3]2 70001548

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[79][0]2 7000154A

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[79][1]2 7000154C

{0x0, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[79][2]2 7000154E

{0xFFFF, 0x0000, BURST_LEN}, // #senHal_pContSenModesRegsArray[79][3]2 70001550



//============================================================

// #Analog Setting END

//============================================================



//============================================================ 

// AF Interface setting

//============================================================ 

{0x002A, 0x01D4, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  //REG_TC_IPRM_AuxGpios : 0 - no Flash

{0x002A, 0x01DE, WORD_LEN},

{0x0F12, 0x0003, WORD_LEN},  //REG_TC_IPRM_CM_Init_AfModeType : 3 - AFD_VCM_I2C

{0x0F12, 0x0000, WORD_LEN},  //REG_TC_IPRM_CM_Init_PwmConfig1 : 0 - no PWM

{0x002A, 0x01E4, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  //REG_TC_IPRM_CM_Init_GpioConfig1 : 0 - no GPIO

{0x002A, 0x01E8, WORD_LEN},

{0x0F12, 0x200C, WORD_LEN},  //REG_TC_IPRM_CM_Init_Mi2cBits : MSCL - GPIO1 MSDA - GPIO2 Device ID (0C)

{0x0F12, 0x0190, WORD_LEN},  //REG_TC_IPRM_CM_Init_Mi2cRateKhz : MI2C Speed - 400KHz



//============================================================ 

// AF Parameter setting

//============================================================ 

// AF Window Settings

{0x002A, 0x025A, WORD_LEN},

{0x0F12, 0x0100, WORD_LEN},  //#REG_TC_AF_FstWinStartX

{0x0F12, 0x00E3, WORD_LEN},  //#REG_TC_AF_FstWinStartY

{0x0F12, 0x0200, WORD_LEN},  //#REG_TC_AF_FstWinSizeX

{0x0F12, 0x0238, WORD_LEN},  //#REG_TC_AF_FstWinSizeY

{0x0F12, 0x018C, WORD_LEN},  //#REG_TC_AF_ScndWinStartX

{0x0F12, 0x0166, WORD_LEN},  //#REG_TC_AF_ScndWinStartY

{0x0F12, 0x00E6, WORD_LEN},  //#REG_TC_AF_ScndWinSizeX

{0x0F12, 0x0132, WORD_LEN},  //#REG_TC_AF_ScndWinSizeY

{0x0F12, 0x0001, WORD_LEN},  //#REG_TC_AF_WinSizesUpdated



// AF Setot Settings 

{0x002A, 0x0586, WORD_LEN},

{0x0F12, 0x00FF, WORD_LEN},  //#skl_af_StatOvlpExpFactor



// AF Scene Settings 

{0x002A, 0x115E, WORD_LEN},

{0x0F12, 0x0003, WORD_LEN},  //#af_scene_usSaturatedScene



// AF Fine Search Settings 

{0x002A, 0x10D4, WORD_LEN},

{0x0F12, 0x1000, WORD_LEN},  //FineSearch Disable  //#af_search_usSingleAfFlags

{0x002A, 0x10DE, WORD_LEN},

{0x0F12, 0x0004, WORD_LEN},  //#af_search_usFinePeakCount

{0x002A, 0x106C, WORD_LEN},

{0x0F12, 0x0202, WORD_LEN},  //#af_pos_usFineStepNumSize



// AF Peak Threshold Setting

{0x002A, 0x10CA, WORD_LEN},  //#af_search_usPeakThr

{0x0F12, 0x00C0, WORD_LEN}, 



// AF Default Position 

{0x002A, 0x1060, WORD_LEN},

{0x0F12, 0x0028, WORD_LEN},  //3C//#af_pos_usHomePos

{0x0F12, 0x6428, WORD_LEN},  //#af_pos_usLowConfPos



// AF LowConfThr Setting

{0x002A, 0x10F4, WORD_LEN},  //LowEdgeBoth GRAD

{0x0F12, 0x0280, WORD_LEN}, 

{0x002A, 0x1100, WORD_LEN},  //LowLight HPF

{0x0F12, 0x03A0, WORD_LEN},  

{0x0F12, 0x0320, WORD_LEN}, 



{0x002A, 0x1134, WORD_LEN},

{0x0F12, 0x0030, WORD_LEN},  //af_stat_usMinStatVal



// AF low Br Th

{0x002A, 0x1154, WORD_LEN},  // normBrThr

{0x0F12,  0x0060, WORD_LEN},



// AF Policy

{0x002A, 0x10E2, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  //#af_search_usCapturePolicy: Focus_Priority, 0002 : Shutter_Priority_Fixed, 0001 : Shutter_Priority_Last_BFP 0000: Shutter_Priority_Current

{0x002A, 0x1072, WORD_LEN},

{0x0F12, 0x003C, WORD_LEN},  //#af_pos_usCaptureFixedPos, BURST_LEN},  // 0x0008


{0x0028, 0x7000, WORD_LEN},
{0x002A, 0x116A, WORD_LEN},
{0x0F12, 0x0032, WORD_LEN},

// AF Lens Position Table Settings 

{0x002A, 0x1074, WORD_LEN},
//mhlee

{0xFFFE, 0x0010, BURST_LEN}, //0010 //#af_pos_usTableLastInd// 16 Steps 091222
{0x0, 0x0028, BURST_LEN}, //003C //#af_pos_usTable_0_// af_pos_usTable
{0x0, 0x002B, BURST_LEN}, //003F //#af_pos_usTable_1_
{0x0, 0x002E, BURST_LEN}, //0042 //#af_pos_usTable_2_
{0x0, 0x0031, BURST_LEN}, //0045 //#af_pos_usTable_3_
{0x0, 0x0034, BURST_LEN}, //0048 //#af_pos_usTable_4_
{0x0, 0x0037, BURST_LEN}, //004B //#af_pos_usTable_5_
{0x0, 0x003A, BURST_LEN}, //004E //#af_pos_usTable_6_
{0x0, 0x003D, BURST_LEN}, //0051 //#af_pos_usTable_7_
{0x0, 0x0040, BURST_LEN}, //0054 //#af_pos_usTable_8_
{0x0, 0x0043, BURST_LEN}, //0057 //#af_pos_usTable_9_
{0x0, 0x0046, BURST_LEN}, //005A //#af_pos_usTable_10_
{0x0, 0x004A, BURST_LEN}, //005E //#af_pos_usTable_11_
{0x0, 0x004D, BURST_LEN}, //0061 //#af_pos_usTable_12_
{0x0, 0x0050, BURST_LEN}, //0064 //#af_pos_usTable_13_
{0x0, 0x0054, BURST_LEN}, //0068 //#af_pos_usTable_14_
{0x0, 0x0058, BURST_LEN}, //006C //#af_pos_usTable_15_
{0xFFFF, 0x0064, BURST_LEN}, //0078 //#af_pos_usTable_16_



{0x002A, 0x0252, WORD_LEN},

{0x0F12, 0x0003, WORD_LEN},  //init 



//============================================================

// #ISP-FE Setting

//============================================================

{0x002A, 0x158A, WORD_LEN},

{0x0F12, 0xEAF0, WORD_LEN},
{0x002A, 0x15C6, WORD_LEN},

{0x0F12, 0x0020, WORD_LEN},
{0x0F12, 0x0060, WORD_LEN},
{0x002A, 0x15BC, WORD_LEN},

{0x0F12, 0x0200, WORD_LEN},


//Analog Offset for MSM

{0x002A, 0x1608, WORD_LEN}, 

{0x0F12, 0x0100, WORD_LEN},  // #gisp_msm_sAnalogOffset[0] 

{0x0F12, 0x0100, WORD_LEN},  // #gisp_msm_sAnalogOffset[1]

{0x0F12, 0x0100, WORD_LEN},  // #gisp_msm_sAnalogOffset[2]

{0x0F12, 0x0100, WORD_LEN},  // #gisp_msm_sAnalogOffset[3]



//============================================================

// #ISP-FE Setting END

//============================================================


//============================================================

// #Frame rate setting 

//============================================================

// #How to set

// #1. Exposure value

// #dec2hex((1 / (frame rate you want(ms))) * 100d * 4d)

// #2. Analog Digital gain

// #dec2hex((Analog gain you want) * 256d)

//============================================================

// #Set preview exposure time


{0x002A, 0x0530, WORD_LEN}, 

{0x0F12, 0x5DC0, WORD_LEN},  // #lt_uMaxExp1 60ms 

{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0x6590, WORD_LEN},  // #lt_uMaxExp2 65ms

{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x167C, WORD_LEN},

{0x0F12, 0x8CA0, WORD_LEN},  // #evt1_lt_uMaxExp3 90ms 

{0x0F12, 0x0000, WORD_LEN}, 

{0x0F12, 0xABE0, WORD_LEN},  // #evt1_lt_uMaxExp4 110ms

{0x0F12, 0x0000, WORD_LEN}, 



// #Set capture exposure time

{0x002A, 0x0538, WORD_LEN},
{0x0F12, 0x5DC0, WORD_LEN},  // #lt_uCapMaxExp1 60ms 

{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0x6590, WORD_LEN},  // #lt_uCapMaxExp2 65ms 

{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x1684, WORD_LEN},

{0x0F12, 0x8CA0, WORD_LEN},  // #evt1_lt_uCapMaxExp3 90ms

{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0xABE0, WORD_LEN},  // #evt1_lt_uCapMaxExp4 110ms

{0x0F12, 0x0000, WORD_LEN},


// #Set gain

{0x002A, 0x0540, WORD_LEN},

{0x0F12, 0x0150, WORD_LEN},  // #lt_uMaxAnGain1

{0x0F12, 0x0280, WORD_LEN},  // #lt_uMaxAnGain2

{0x002A, 0x168C, WORD_LEN},

{0x0F12, 0x0350, WORD_LEN},  // #evt1_lt_uMaxAnGain3

{0x0F12, 0x0800, WORD_LEN},  // #evt1_lt_uMaxAnGain4 



{0x002A, 0x0544, WORD_LEN},

{0x0F12, 0x0100, WORD_LEN},  // #lt_uMaxDigGain

{0x0F12, 0x8000, WORD_LEN},  // #lt_uMaxTotGain



{0x002A, 0x1694, WORD_LEN},

{0x0F12, 0x0001, WORD_LEN},  // #evt1_senHal_bExpandForbid



{0x002A, 0x051A, WORD_LEN},

{0x0F12, 0x0111, WORD_LEN},  // #lt_uLimitHigh 

{0x0F12, 0x00F0, WORD_LEN},  // #lt_uLimitLow





//================================================================================================

// #SET AE

//================================================================================================

// #AE target 

{0x002A, 0x0F70, WORD_LEN},

{0x0F12, 0x003B, WORD_LEN},  // #TVAR_ae_BrAve 091222

// #AE mode

{0x002A, 0x0F76, WORD_LEN},

{0x0F12, 0x000F, WORD_LEN},  // #Disable illumination & contrast, BURST_LEN},  // ##ae_StatMode

// #AE weight

{0x002A, 0x0F7E, WORD_LEN},

//mhlee

{0xFFFE,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_0_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_1_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_2_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_3_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_4_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_5_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_6_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_7_

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_8_

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_9_

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_10

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_11

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_12

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_13

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_14

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_15

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_16

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_17

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_18

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_19

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_20

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_21

{0x0,  0x0303, BURST_LEN},  // #ae_WeightTbl_16_22

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_23

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_24

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_25

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_26

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_27

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_28

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_29

{0x0,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_30

{0xFFFF,  0x0101, BURST_LEN},  // #ae_WeightTbl_16_31


//================================================================================================

// #SET FLICKER

//================================================================================================

{0x002A, 0x0C18, WORD_LEN},

{0x0F12, 0x0000, WORD_LEN},  // #0001: 60Hz start auto / 0000: 50Hz start auto

{0x002A, 0x04D2, WORD_LEN},

{0x0F12, 0x067F, WORD_LEN},


//================================================================================================

 // #SET GAS

//================================================================================================

// #GAS alpha

// #R Gr Gb B per light source

{0x002A, 0x06CE, WORD_LEN},

//mhlee

{0xFFFE,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[0], BURST_LEN},  // #Horizon

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[1]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[2]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[3]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[4], BURST_LEN},  // #IncandA

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[5]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[6]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[7]

{0x0,  0x00F0, BURST_LEN},  // #TVAR_ash_GASAlpha[8], BURST_LEN},  // #WW

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[9]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[10]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[11]

{0x0,  0x00E0, BURST_LEN},  // #TVAR_ash_GASAlpha[12], BURST_LEN},  // #CWF

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[13]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[14]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[15]

{0x0,  0x010A, BURST_LEN},  // #TVAR_ash_GASAlpha[16], BURST_LEN},  // #D50

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[17]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[18]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[19]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[20], BURST_LEN},  // #D65

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[21]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[22]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[23]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[24], BURST_LEN},  // #D75

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[25]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[26]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASAlpha[27]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASOutdoorAlpha[0], BURST_LEN},  // #Outdoor

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASOutdoorAlpha[1]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASOutdoorAlpha[2]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_GASOutdoorAlpha[3]

// #GAS beta  

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[0], BURST_LEN},  // #Horizon

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[1]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[2]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[3]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[4], BURST_LEN},  // #IncandA

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[5]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[6]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[7]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[8], BURST_LEN},  // #WW 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[9]

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[10] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[11] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[12], BURST_LEN},  // #CWF

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[13] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[14] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[15] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[16], BURST_LEN},  // #D50

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[17] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[18] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[19] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[20], BURST_LEN},  // #D65

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[21] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[22] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[23] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[24], BURST_LEN},  // #D75

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[25] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[26] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASBeta[27] 

{0x0,  0x0000, BURST_LEN},  // #ash_GASOutdoorBeta[0], BURST_LEN},  // #Outdoor

{0x0,  0x0000, BURST_LEN},  // #ash_GASOutdoorBeta[1]

{0x0,  0x0000, BURST_LEN},  // #ash_GASOutdoorBeta[2]

{0xFFFF,  0x0000, BURST_LEN},  // #ash_GASOutdoorBeta[3]



{0x002A, 0x06B4, WORD_LEN},

{0x0F12,  0x0000, WORD_LEN},  // #wbt_bUseOutdoorASH ON:1 OFF:0



// #Parabloic function

{0x002A, 0x075A, WORD_LEN},

{0x0F12,  0x0000, WORD_LEN},  // #ash_bParabolicEstimation

{0x0F12,  0x0400, WORD_LEN},  // #ash_uParabolicCenterX

{0x0F12,  0x0300, WORD_LEN},  // #ash_uParabolicCenterY

{0x0F12,  0x0010, WORD_LEN},  // #ash_uParabolicScalingA

{0x0F12,  0x0011, WORD_LEN},  // #ash_uParabolicScalingB



{0x002A, 0x06C6, WORD_LEN},

{0x0F12,  0x0100, WORD_LEN},  // #ash_CGrasAlphas_0_

{0x0F12,  0x0100, WORD_LEN},  // #ash_CGrasAlphas_1_

{0x0F12,  0x0100, WORD_LEN},  // #ash_CGrasAlphas_2_

{0x0F12,  0x0100, WORD_LEN},  // #ash_CGrasAlphas_3_



{0x002A, 0x0E3C, WORD_LEN},

{0x0F12, 0x00C0, WORD_LEN},  // #awbb_Alpha_Comp_Mode

{0x002A, 0x074E, WORD_LEN}, 

{0x0F12, 0x0000, WORD_LEN},  // #ash_bLumaMode, BURST_LEN},  //use Beta : 0001 not use Beta : 0000



// #GAS LUT start address, BURST_LEN},  // #7000_347C 

{0x002A, 0x0754, WORD_LEN},

{0x0F12,  0x347C, WORD_LEN},
{0x0F12,  0x7000, WORD_LEN},
// #GAS LUT

{0x002A, 0x347C, WORD_LEN},

//mhlee

{0xFFFE,  0x017E, BURST_LEN},  // #TVAR_ash_pGAS[0]

{0x0,  0x015B, BURST_LEN},  // #TVAR_ash_pGAS[1]

{0x0,  0x0125, BURST_LEN},  // #TVAR_ash_pGAS[2]

{0x0,  0x00FA, BURST_LEN},  // #TVAR_ash_pGAS[3]

{0x0,  0x00DC, BURST_LEN},  // #TVAR_ash_pGAS[4]

{0x0,  0x00CD, BURST_LEN},  // #TVAR_ash_pGAS[5]

{0x0,  0x00CA, BURST_LEN},  // #TVAR_ash_pGAS[6]

{0x0,  0x00D4, BURST_LEN},  // #TVAR_ash_pGAS[7]

{0x0,  0x00EE, BURST_LEN},  // #TVAR_ash_pGAS[8]

{0x0,  0x011A, BURST_LEN},  // #TVAR_ash_pGAS[9]

{0x0,  0x0158, BURST_LEN},  // #TVAR_ash_pGAS[10]

{0x0,  0x0197, BURST_LEN},  // #TVAR_ash_pGAS[11]

{0x0,  0x01F3, BURST_LEN},  // #TVAR_ash_pGAS[12]

{0x0,  0x016A, BURST_LEN},  // #TVAR_ash_pGAS[13]

{0x0,  0x0129, BURST_LEN},  // #TVAR_ash_pGAS[14]

{0x0,  0x00EE, BURST_LEN},  // #TVAR_ash_pGAS[15]

{0x0,  0x00BF, BURST_LEN},  // #TVAR_ash_pGAS[16]

{0x0,  0x009D, BURST_LEN},  // #TVAR_ash_pGAS[17]

{0x0,  0x008C, BURST_LEN},  // #TVAR_ash_pGAS[18]

{0x0,  0x0089, BURST_LEN},  // #TVAR_ash_pGAS[19]

{0x0,  0x0096, BURST_LEN},  // #TVAR_ash_pGAS[20]

{0x0,  0x00B3, BURST_LEN},  // #TVAR_ash_pGAS[21]

{0x0,  0x00DF, BURST_LEN},  // #TVAR_ash_pGAS[22]

{0x0,  0x0121, BURST_LEN},  // #TVAR_ash_pGAS[23]

{0x0,  0x016F, BURST_LEN},  // #TVAR_ash_pGAS[24]

{0x0,  0x01B3, BURST_LEN},  // #TVAR_ash_pGAS[25]

{0x0,  0x0144, BURST_LEN},  // #TVAR_ash_pGAS[26]

{0x0,  0x00FB, BURST_LEN},  // #TVAR_ash_pGAS[27]

{0x0,  0x00B5, BURST_LEN},  // #TVAR_ash_pGAS[28]

{0x0,  0x0085, BURST_LEN},  // #TVAR_ash_pGAS[29]

{0x0,  0x0062, BURST_LEN},  // #TVAR_ash_pGAS[30]

{0x0,  0x004E, BURST_LEN},  // #TVAR_ash_pGAS[31]

{0x0,  0x004C, BURST_LEN},  // #TVAR_ash_pGAS[32]

{0x0,  0x005A, BURST_LEN},  // #TVAR_ash_pGAS[33]

{0x0,  0x007A, BURST_LEN},  // #TVAR_ash_pGAS[34]

{0x0,  0x00AB, BURST_LEN},  // #TVAR_ash_pGAS[35]

{0x0,  0x00EA, BURST_LEN},  // #TVAR_ash_pGAS[36]

{0x0,  0x013E, BURST_LEN},  // #TVAR_ash_pGAS[37]

{0x0,  0x0190, BURST_LEN},  // #TVAR_ash_pGAS[38]

{0x0,  0x011E, BURST_LEN},  // #TVAR_ash_pGAS[39]

{0x0,  0x00D3, BURST_LEN},  // #TVAR_ash_pGAS[40]

{0x0,  0x008F, BURST_LEN},  // #TVAR_ash_pGAS[41]

{0x0,  0x005A, BURST_LEN},  // #TVAR_ash_pGAS[42]

{0x0,  0x0035, BURST_LEN},  // #TVAR_ash_pGAS[43]

{0x0,  0x001F, BURST_LEN},  // #TVAR_ash_pGAS[44]

{0x0,  0x001E, BURST_LEN},  // #TVAR_ash_pGAS[45]

{0x0,  0x002D, BURST_LEN},  // #TVAR_ash_pGAS[46]

{0x0,  0x004F, BURST_LEN},  // #TVAR_ash_pGAS[47]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[48]

{0x0,  0x00C6, BURST_LEN},  // #TVAR_ash_pGAS[49]

{0x0,  0x011C, BURST_LEN},  // #TVAR_ash_pGAS[50]

{0x0,  0x0174, BURST_LEN},  // #TVAR_ash_pGAS[51]

{0x0,  0x010F, BURST_LEN},  // #TVAR_ash_pGAS[52]

{0x0,  0x00C7, BURST_LEN},  // #TVAR_ash_pGAS[53]

{0x0,  0x007D, BURST_LEN},  // #TVAR_ash_pGAS[54]

{0x0,  0x0044, BURST_LEN},  // #TVAR_ash_pGAS[55]

{0x0,  0x001D, BURST_LEN},  // #TVAR_ash_pGAS[56]

{0x0,  0x0008, BURST_LEN},  // #TVAR_ash_pGAS[57]

{0x0,  0x0007, BURST_LEN},  // #TVAR_ash_pGAS[58]

{0x0,  0x0016, BURST_LEN},  // #TVAR_ash_pGAS[59]

{0x0,  0x0039, BURST_LEN},  // #TVAR_ash_pGAS[60]

{0x0,  0x006F, BURST_LEN},  // #TVAR_ash_pGAS[61]

{0x0,  0x00B6, BURST_LEN},  // #TVAR_ash_pGAS[62]

{0x0,  0x010C, BURST_LEN},  // #TVAR_ash_pGAS[63]

{0x0,  0x0168, BURST_LEN},  // #TVAR_ash_pGAS[64]

{0x0,  0x010B, BURST_LEN},  // #TVAR_ash_pGAS[65]

{0x0,  0x00C1, BURST_LEN},  // #TVAR_ash_pGAS[66]

{0x0,  0x0078, BURST_LEN},  // #TVAR_ash_pGAS[67]

{0x0,  0x003E, BURST_LEN},  // #TVAR_ash_pGAS[68]

{0x0,  0x0016, BURST_LEN},  // #TVAR_ash_pGAS[69]

{0x0,  0x0001, BURST_LEN},  // #TVAR_ash_pGAS[70]

{0x0,  0x0000, BURST_LEN},  // #TVAR_ash_pGAS[71]

{0x0,  0x0010, BURST_LEN},  // #TVAR_ash_pGAS[72]

{0x0,  0x0034, BURST_LEN},  // #TVAR_ash_pGAS[73]

{0x0,  0x006C, BURST_LEN},  // #TVAR_ash_pGAS[74]

{0x0,  0x00B4, BURST_LEN},  // #TVAR_ash_pGAS[75]

{0x0,  0x010B, BURST_LEN},  // #TVAR_ash_pGAS[76]

{0x0,  0x0165, BURST_LEN},  // #TVAR_ash_pGAS[77]

{0x0,  0x0116, BURST_LEN},  // #TVAR_ash_pGAS[78]

{0x0,  0x00CA, BURST_LEN},  // #TVAR_ash_pGAS[79]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[80]

{0x0,  0x004A, BURST_LEN},  // #TVAR_ash_pGAS[81]

{0x0,  0x0021, BURST_LEN},  // #TVAR_ash_pGAS[82]

{0x0,  0x000C, BURST_LEN},  // #TVAR_ash_pGAS[83]

{0x0,  0x000A, BURST_LEN},  // #TVAR_ash_pGAS[84]

{0x0,  0x001B, BURST_LEN},  // #TVAR_ash_pGAS[85]

{0x0,  0x0040, BURST_LEN},  // #TVAR_ash_pGAS[86]

{0x0,  0x0078, BURST_LEN},  // #TVAR_ash_pGAS[87]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[88]

{0x0,  0x0119, BURST_LEN},  // #TVAR_ash_pGAS[89]

{0x0,  0x0172, BURST_LEN},  // #TVAR_ash_pGAS[90]

{0x0,  0x0133, BURST_LEN},  // #TVAR_ash_pGAS[91]

{0x0,  0x00EA, BURST_LEN},  // #TVAR_ash_pGAS[92]

{0x0,  0x00A1, BURST_LEN},  // #TVAR_ash_pGAS[93]

{0x0,  0x0069, BURST_LEN},  // #TVAR_ash_pGAS[94]

{0x0,  0x0040, BURST_LEN},  // #TVAR_ash_pGAS[95]

{0x0,  0x002B, BURST_LEN},  // #TVAR_ash_pGAS[96]

{0x0,  0x002A, BURST_LEN},  // #TVAR_ash_pGAS[97]

{0x0,  0x003B, BURST_LEN},  // #TVAR_ash_pGAS[98]

{0x0,  0x0060, BURST_LEN},  // #TVAR_ash_pGAS[99]

{0x0,  0x0097, BURST_LEN},  // #TVAR_ash_pGAS[100]

{0x0,  0x00DF, BURST_LEN},  // #TVAR_ash_pGAS[101]

{0x0,  0x0138, BURST_LEN},  // #TVAR_ash_pGAS[102]

{0x0,  0x018C, BURST_LEN},  // #TVAR_ash_pGAS[103]

{0x0,  0x015A, BURST_LEN},  // #TVAR_ash_pGAS[104]

{0x0,  0x0113, BURST_LEN},  // #TVAR_ash_pGAS[105]

{0x0,  0x00CC, BURST_LEN},  // #TVAR_ash_pGAS[106]

{0x0,  0x0095, BURST_LEN},  // #TVAR_ash_pGAS[107]

{0x0,  0x006F, BURST_LEN},  // #TVAR_ash_pGAS[108]

{0x0,  0x0059, BURST_LEN},  // #TVAR_ash_pGAS[109]

{0x0,  0x0058, BURST_LEN},  // #TVAR_ash_pGAS[110]

{0x0,  0x0069, BURST_LEN},  // #TVAR_ash_pGAS[111]

{0x0,  0x008E, BURST_LEN},  // #TVAR_ash_pGAS[112]

{0x0,  0x00C3, BURST_LEN},  // #TVAR_ash_pGAS[113]

{0x0,  0x0109, BURST_LEN},  // #TVAR_ash_pGAS[114]

{0x0,  0x015F, BURST_LEN},  // #TVAR_ash_pGAS[115]

{0x0,  0x01AD, BURST_LEN},  // #TVAR_ash_pGAS[116]

{0x0,  0x0188, BURST_LEN},  // #TVAR_ash_pGAS[117]

{0x0,  0x014C, BURST_LEN},  // #TVAR_ash_pGAS[118]

{0x0,  0x0106, BURST_LEN},  // #TVAR_ash_pGAS[119]

{0x0,  0x00D4, BURST_LEN},  // #TVAR_ash_pGAS[120]

{0x0,  0x00B0, BURST_LEN},  // #TVAR_ash_pGAS[121]

{0x0,  0x009E, BURST_LEN},  // #TVAR_ash_pGAS[122]

{0x0,  0x009D, BURST_LEN},  // #TVAR_ash_pGAS[123]

{0x0,  0x00AE, BURST_LEN},  // #TVAR_ash_pGAS[124]

{0x0,  0x00CE, BURST_LEN},  // #TVAR_ash_pGAS[125]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_ash_pGAS[126]

{0x0,  0x0143, BURST_LEN},  // #TVAR_ash_pGAS[127]

{0x0,  0x0192, BURST_LEN},  // #TVAR_ash_pGAS[128]

{0x0,  0x01DC, BURST_LEN},  // #TVAR_ash_pGAS[129]

{0x0,  0x01B5, BURST_LEN},  // #TVAR_ash_pGAS[130]

{0x0,  0x0185, BURST_LEN},  // #TVAR_ash_pGAS[131]

{0x0,  0x0148, BURST_LEN},  // #TVAR_ash_pGAS[132]

{0x0,  0x0114, BURST_LEN},  // #TVAR_ash_pGAS[133]

{0x0,  0x00F4, BURST_LEN},  // #TVAR_ash_pGAS[134]

{0x0,  0x00E4, BURST_LEN},  // #TVAR_ash_pGAS[135]

{0x0,  0x00E5, BURST_LEN},  // #TVAR_ash_pGAS[136]

{0x0,  0x00F3, BURST_LEN},  // #TVAR_ash_pGAS[137]

{0x0,  0x0111, BURST_LEN},  // #TVAR_ash_pGAS[138]

{0x0,  0x0141, BURST_LEN},  // #TVAR_ash_pGAS[139]

{0x0,  0x0181, BURST_LEN},  // #TVAR_ash_pGAS[140]

{0x0,  0x01C6, BURST_LEN},  // #TVAR_ash_pGAS[141]

{0x0,  0x0237, BURST_LEN},  // #TVAR_ash_pGAS[142]

{0x0,  0x0144, BURST_LEN},  // #TVAR_ash_pGAS[143]

{0x0,  0x0127, BURST_LEN},  // #TVAR_ash_pGAS[144]

{0x0,  0x00F5, BURST_LEN},  // #TVAR_ash_pGAS[145]

{0x0,  0x00CD, BURST_LEN},  // #TVAR_ash_pGAS[146]

{0x0,  0x00B2, BURST_LEN},  // #TVAR_ash_pGAS[147]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[148]

{0x0,  0x009F, BURST_LEN},  // #TVAR_ash_pGAS[149]

{0x0,  0x00A7, BURST_LEN},  // #TVAR_ash_pGAS[150]

{0x0,  0x00BF, BURST_LEN},  // #TVAR_ash_pGAS[151]

{0x0,  0x00E3, BURST_LEN},  // #TVAR_ash_pGAS[152]

{0x0,  0x0115, BURST_LEN},  // #TVAR_ash_pGAS[153]

{0x0,  0x014C, BURST_LEN},  // #TVAR_ash_pGAS[154]

{0x0,  0x019E, BURST_LEN},  // #TVAR_ash_pGAS[155]

{0x0,  0x0134, BURST_LEN},  // #TVAR_ash_pGAS[156]

{0x0,  0x00F8, BURST_LEN},  // #TVAR_ash_pGAS[157]

{0x0,  0x00C5, BURST_LEN},  // #TVAR_ash_pGAS[158]

{0x0,  0x009C, BURST_LEN},  // #TVAR_ash_pGAS[159]

{0x0,  0x007E, BURST_LEN},  // #TVAR_ash_pGAS[160]

{0x0,  0x006E, BURST_LEN},  // #TVAR_ash_pGAS[161]

{0x0,  0x006C, BURST_LEN},  // #TVAR_ash_pGAS[162]

{0x0,  0x0076, BURST_LEN},  // #TVAR_ash_pGAS[163]

{0x0,  0x008E, BURST_LEN},  // #TVAR_ash_pGAS[164]

{0x0,  0x00B2, BURST_LEN},  // #TVAR_ash_pGAS[165]

{0x0,  0x00E8, BURST_LEN},  // #TVAR_ash_pGAS[166]

{0x0,  0x012A, BURST_LEN},  // #TVAR_ash_pGAS[167]

{0x0,  0x016A, BURST_LEN},  // #TVAR_ash_pGAS[168]

{0x0,  0x0117, BURST_LEN},  // #TVAR_ash_pGAS[169]

{0x0,  0x00D2, BURST_LEN},  // #TVAR_ash_pGAS[170]

{0x0,  0x0096, BURST_LEN},  // #TVAR_ash_pGAS[171]

{0x0,  0x006E, BURST_LEN},  // #TVAR_ash_pGAS[172]

{0x0,  0x004E, BURST_LEN},  // #TVAR_ash_pGAS[173]

{0x0,  0x003D, BURST_LEN},  // #TVAR_ash_pGAS[174]

{0x0,  0x003C, BURST_LEN},  // #TVAR_ash_pGAS[175]

{0x0,  0x0048, BURST_LEN},  // #TVAR_ash_pGAS[176]

{0x0,  0x0063, BURST_LEN},  // #TVAR_ash_pGAS[177]

{0x0,  0x0089, BURST_LEN},  // #TVAR_ash_pGAS[178]

{0x0,  0x00BE, BURST_LEN},  // #TVAR_ash_pGAS[179]

{0x0,  0x0106, BURST_LEN},  // #TVAR_ash_pGAS[180]

{0x0,  0x014E, BURST_LEN},  // #TVAR_ash_pGAS[181]

{0x0,  0x00F6, BURST_LEN},  // #TVAR_ash_pGAS[182]

{0x0,  0x00B4, BURST_LEN},  // #TVAR_ash_pGAS[183]

{0x0,  0x0078, BURST_LEN},  // #TVAR_ash_pGAS[184]

{0x0,  0x004A, BURST_LEN},  // #TVAR_ash_pGAS[185]

{0x0,  0x002A, BURST_LEN},  // #TVAR_ash_pGAS[186]

{0x0,  0x0017, BURST_LEN},  // #TVAR_ash_pGAS[187]

{0x0,  0x0016, BURST_LEN},  // #TVAR_ash_pGAS[188]

{0x0,  0x0025, BURST_LEN},  // #TVAR_ash_pGAS[189]

{0x0,  0x0042, BURST_LEN},  // #TVAR_ash_pGAS[190]

{0x0,  0x006D, BURST_LEN},  // #TVAR_ash_pGAS[191]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[192]

{0x0,  0x00E9, BURST_LEN},  // #TVAR_ash_pGAS[193]

{0x0,  0x0138, BURST_LEN},  // #TVAR_ash_pGAS[194]

{0x0,  0x00E7, BURST_LEN},  // #TVAR_ash_pGAS[195]

{0x0,  0x00A5, BURST_LEN},  // #TVAR_ash_pGAS[196]

{0x0,  0x0068, BURST_LEN},  // #TVAR_ash_pGAS[197]

{0x0,  0x0038, BURST_LEN},  // #TVAR_ash_pGAS[198]

{0x0,  0x0017, BURST_LEN},  // #TVAR_ash_pGAS[199]

{0x0,  0x0005, BURST_LEN},  // #TVAR_ash_pGAS[200]

{0x0,  0x0005, BURST_LEN},  // #TVAR_ash_pGAS[201]

{0x0,  0x0013, BURST_LEN},  // #TVAR_ash_pGAS[202]

{0x0,  0x0032, BURST_LEN},  // #TVAR_ash_pGAS[203]

{0x0,  0x005E, BURST_LEN},  // #TVAR_ash_pGAS[204]

{0x0,  0x0097, BURST_LEN},  // #TVAR_ash_pGAS[205]

{0x0,  0x00DF, BURST_LEN},  // #TVAR_ash_pGAS[206]

{0x0,  0x012E, BURST_LEN},  // #TVAR_ash_pGAS[207]

{0x0,  0x00E2, BURST_LEN},  // #TVAR_ash_pGAS[208]

{0x0,  0x00A0, BURST_LEN},  // #TVAR_ash_pGAS[209]

{0x0,  0x0063, BURST_LEN},  // #TVAR_ash_pGAS[210]

{0x0,  0x0034, BURST_LEN},  // #TVAR_ash_pGAS[211]

{0x0,  0x0013, BURST_LEN},  // #TVAR_ash_pGAS[212]

{0x0,  0x0001, BURST_LEN},  // #TVAR_ash_pGAS[213]

{0x0,  0x0000, BURST_LEN},  // #TVAR_ash_pGAS[214]

{0x0,  0x0010, BURST_LEN},  // #TVAR_ash_pGAS[215]

{0x0,  0x002F, BURST_LEN},  // #TVAR_ash_pGAS[216]

{0x0,  0x005D, BURST_LEN},  // #TVAR_ash_pGAS[217]

{0x0,  0x0097, BURST_LEN},  // #TVAR_ash_pGAS[218]

{0x0,  0x00DD, BURST_LEN},  // #TVAR_ash_pGAS[219]

{0x0,  0x012C, BURST_LEN},  // #TVAR_ash_pGAS[220]

{0x0,  0x00EA, BURST_LEN},  // #TVAR_ash_pGAS[221]

{0x0,  0x00A7, BURST_LEN},  // #TVAR_ash_pGAS[222]

{0x0,  0x006D, BURST_LEN},  // #TVAR_ash_pGAS[223]

{0x0,  0x003D, BURST_LEN},  // #TVAR_ash_pGAS[224]

{0x0,  0x001C, BURST_LEN},  // #TVAR_ash_pGAS[225]

{0x0,  0x000B, BURST_LEN},  // #TVAR_ash_pGAS[226]

{0x0,  0x000A, BURST_LEN},  // #TVAR_ash_pGAS[227]

{0x0,  0x001B, BURST_LEN},  // #TVAR_ash_pGAS[228]

{0x0,  0x003B, BURST_LEN},  // #TVAR_ash_pGAS[229]

{0x0,  0x006A, BURST_LEN},  // #TVAR_ash_pGAS[230]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[231]

{0x0,  0x00EA, BURST_LEN},  // #TVAR_ash_pGAS[232]

{0x0,  0x0137, BURST_LEN},  // #TVAR_ash_pGAS[233]

{0x0,  0x0102, BURST_LEN},  // #TVAR_ash_pGAS[234]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[235]

{0x0,  0x0086, BURST_LEN},  // #TVAR_ash_pGAS[236]

{0x0,  0x0058, BURST_LEN},  // #TVAR_ash_pGAS[237]

{0x0,  0x0037, BURST_LEN},  // #TVAR_ash_pGAS[238]

{0x0,  0x0026, BURST_LEN},  // #TVAR_ash_pGAS[239]

{0x0,  0x0027, BURST_LEN},  // #TVAR_ash_pGAS[240]

{0x0,  0x0038, BURST_LEN},  // #TVAR_ash_pGAS[241]

{0x0,  0x0059, BURST_LEN},  // #TVAR_ash_pGAS[242]

{0x0,  0x0086, BURST_LEN},  // #TVAR_ash_pGAS[243]

{0x0,  0x00BE, BURST_LEN},  // #TVAR_ash_pGAS[244]

{0x0,  0x0106, BURST_LEN},  // #TVAR_ash_pGAS[245]

{0x0,  0x0150, BURST_LEN},  // #TVAR_ash_pGAS[246]

{0x0,  0x0124, BURST_LEN},  // #TVAR_ash_pGAS[247]

{0x0,  0x00E5, BURST_LEN},  // #TVAR_ash_pGAS[248]

{0x0,  0x00A9, BURST_LEN},  // #TVAR_ash_pGAS[249]

{0x0,  0x007E, BURST_LEN},  // #TVAR_ash_pGAS[250]

{0x0,  0x005E, BURST_LEN},  // #TVAR_ash_pGAS[251]

{0x0,  0x004E, BURST_LEN},  // #TVAR_ash_pGAS[252]

{0x0,  0x004F, BURST_LEN},  // #TVAR_ash_pGAS[253]

{0x0,  0x0061, BURST_LEN},  // #TVAR_ash_pGAS[254]

{0x0,  0x0081, BURST_LEN},  // #TVAR_ash_pGAS[255]

{0x0,  0x00AC, BURST_LEN},  // #TVAR_ash_pGAS[256]

{0x0,  0x00E2, BURST_LEN},  // #TVAR_ash_pGAS[257]

{0x0,  0x0129, BURST_LEN},  // #TVAR_ash_pGAS[258]

{0x0,  0x016C, BURST_LEN},  // #TVAR_ash_pGAS[259]

{0x0,  0x014B, BURST_LEN},  // #TVAR_ash_pGAS[260]

{0x0,  0x0114, BURST_LEN},  // #TVAR_ash_pGAS[261]

{0x0,  0x00DA, BURST_LEN},  // #TVAR_ash_pGAS[262]

{0x0,  0x00B0, BURST_LEN},  // #TVAR_ash_pGAS[263]

{0x0,  0x0095, BURST_LEN},  // #TVAR_ash_pGAS[264]

{0x0,  0x0087, BURST_LEN},  // #TVAR_ash_pGAS[265]

{0x0,  0x0088, BURST_LEN},  // #TVAR_ash_pGAS[266]

{0x0,  0x009A, BURST_LEN},  // #TVAR_ash_pGAS[267]

{0x0,  0x00B8, BURST_LEN},  // #TVAR_ash_pGAS[268]

{0x0,  0x00DF, BURST_LEN},  // #TVAR_ash_pGAS[269]

{0x0,  0x0115, BURST_LEN},  // #TVAR_ash_pGAS[270]

{0x0,  0x0153, BURST_LEN},  // #TVAR_ash_pGAS[271]

{0x0,  0x0194, BURST_LEN},  // #TVAR_ash_pGAS[272]

{0x0,  0x0172, BURST_LEN},  // #TVAR_ash_pGAS[273]

{0x0,  0x0146, BURST_LEN},  // #TVAR_ash_pGAS[274]

{0x0,  0x0116, BURST_LEN},  // #TVAR_ash_pGAS[275]

{0x0,  0x00EB, BURST_LEN},  // #TVAR_ash_pGAS[276]

{0x0,  0x00CD, BURST_LEN},  // #TVAR_ash_pGAS[277]

{0x0,  0x00C3, BURST_LEN},  // #TVAR_ash_pGAS[278]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_ash_pGAS[279]

{0x0,  0x00D6, BURST_LEN},  // #TVAR_ash_pGAS[280]

{0x0,  0x00F4, BURST_LEN},  // #TVAR_ash_pGAS[281]

{0x0,  0x011C, BURST_LEN},  // #TVAR_ash_pGAS[282]

{0x0,  0x014F, BURST_LEN},  // #TVAR_ash_pGAS[283]

{0x0,  0x0185, BURST_LEN},  // #TVAR_ash_pGAS[284]

{0x0,  0x01E5, BURST_LEN},  // #TVAR_ash_pGAS[285]

{0x0,  0x0147, BURST_LEN},  // #TVAR_ash_pGAS[286]

{0x0,  0x012C, BURST_LEN},  // #TVAR_ash_pGAS[287]

{0x0,  0x00FB, BURST_LEN},  // #TVAR_ash_pGAS[288]

{0x0,  0x00D1, BURST_LEN},  // #TVAR_ash_pGAS[289]

{0x0,  0x00B6, BURST_LEN},  // #TVAR_ash_pGAS[290]

{0x0,  0x00AA, BURST_LEN},  // #TVAR_ash_pGAS[291]

{0x0,  0x00AB, BURST_LEN},  // #TVAR_ash_pGAS[292]

{0x0,  0x00BA, BURST_LEN},  // #TVAR_ash_pGAS[293]

{0x0,  0x00D6, BURST_LEN},  // #TVAR_ash_pGAS[294]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_ash_pGAS[295]

{0x0,  0x0136, BURST_LEN},  // #TVAR_ash_pGAS[296]

{0x0,  0x0172, BURST_LEN},  // #TVAR_ash_pGAS[297]

{0x0,  0x01BE, BURST_LEN},  // #TVAR_ash_pGAS[298]

{0x0,  0x013B, BURST_LEN},  // #TVAR_ash_pGAS[299]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_ash_pGAS[300]

{0x0,  0x00CA, BURST_LEN},  // #TVAR_ash_pGAS[301]

{0x0,  0x00A1, BURST_LEN},  // #TVAR_ash_pGAS[302]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[303]

{0x0,  0x0076, BURST_LEN},  // #TVAR_ash_pGAS[304]

{0x0,  0x0077, BURST_LEN},  // #TVAR_ash_pGAS[305]

{0x0,  0x0087, BURST_LEN},  // #TVAR_ash_pGAS[306]

{0x0,  0x00A4, BURST_LEN},  // #TVAR_ash_pGAS[307]

{0x0,  0x00CD, BURST_LEN},  // #TVAR_ash_pGAS[308]

{0x0,  0x0107, BURST_LEN},  // #TVAR_ash_pGAS[309]

{0x0,  0x014C, BURST_LEN},  // #TVAR_ash_pGAS[310]

{0x0,  0x0189, BURST_LEN},  // #TVAR_ash_pGAS[311]

{0x0,  0x011E, BURST_LEN},  // #TVAR_ash_pGAS[312]

{0x0,  0x00D8, BURST_LEN},  // #TVAR_ash_pGAS[313]

{0x0,  0x009C, BURST_LEN},  // #TVAR_ash_pGAS[314]

{0x0,  0x0072, BURST_LEN},  // #TVAR_ash_pGAS[315]

{0x0,  0x0054, BURST_LEN},  // #TVAR_ash_pGAS[316]

{0x0,  0x0043, BURST_LEN},  // #TVAR_ash_pGAS[317]

{0x0,  0x0045, BURST_LEN},  // #TVAR_ash_pGAS[318]

{0x0,  0x0055, BURST_LEN},  // #TVAR_ash_pGAS[319]

{0x0,  0x0075, BURST_LEN},  // #TVAR_ash_pGAS[320]

{0x0,  0x00A1, BURST_LEN},  // #TVAR_ash_pGAS[321]

{0x0,  0x00D7, BURST_LEN},  // #TVAR_ash_pGAS[322]

{0x0,  0x0121, BURST_LEN},  // #TVAR_ash_pGAS[323]

{0x0,  0x016A, BURST_LEN},  // #TVAR_ash_pGAS[324]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_ash_pGAS[325]

{0x0,  0x00BA, BURST_LEN},  // #TVAR_ash_pGAS[326]

{0x0,  0x007F, BURST_LEN},  // #TVAR_ash_pGAS[327]

{0x0,  0x0051, BURST_LEN},  // #TVAR_ash_pGAS[328]

{0x0,  0x0031, BURST_LEN},  // #TVAR_ash_pGAS[329]

{0x0,  0x001D, BURST_LEN},  // #TVAR_ash_pGAS[330]

{0x0,  0x001D, BURST_LEN},  // #TVAR_ash_pGAS[331]

{0x0,  0x002E, BURST_LEN},  // #TVAR_ash_pGAS[332]

{0x0,  0x0050, BURST_LEN},  // #TVAR_ash_pGAS[333]

{0x0,  0x007E, BURST_LEN},  // #TVAR_ash_pGAS[334]

{0x0,  0x00B5, BURST_LEN},  // #TVAR_ash_pGAS[335]

{0x0,  0x00FE, BURST_LEN},  // #TVAR_ash_pGAS[336]

{0x0,  0x014D, BURST_LEN},  // #TVAR_ash_pGAS[337]

{0x0,  0x00F2, BURST_LEN},  // #TVAR_ash_pGAS[338]

{0x0,  0x00AF, BURST_LEN},  // #TVAR_ash_pGAS[339]

{0x0,  0x0071, BURST_LEN},  // #TVAR_ash_pGAS[340]

{0x0,  0x0040, BURST_LEN},  // #TVAR_ash_pGAS[341]

{0x0,  0x001D, BURST_LEN},  // #TVAR_ash_pGAS[342]

{0x0,  0x000A, BURST_LEN},  // #TVAR_ash_pGAS[343]

{0x0,  0x0009, BURST_LEN},  // #TVAR_ash_pGAS[344]

{0x0,  0x001A, BURST_LEN},  // #TVAR_ash_pGAS[345]

{0x0,  0x003A, BURST_LEN},  // #TVAR_ash_pGAS[346]

{0x0,  0x0069, BURST_LEN},  // #TVAR_ash_pGAS[347]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[348]

{0x0,  0x00E9, BURST_LEN},  // #TVAR_ash_pGAS[349]

{0x0,  0x013B, BURST_LEN},  // #TVAR_ash_pGAS[350]

{0x0,  0x00EF, BURST_LEN},  // #TVAR_ash_pGAS[351]

{0x0,  0x00AA, BURST_LEN},  // #TVAR_ash_pGAS[352]

{0x0,  0x006D, BURST_LEN},  // #TVAR_ash_pGAS[353]

{0x0,  0x003D, BURST_LEN},  // #TVAR_ash_pGAS[354]

{0x0,  0x0019, BURST_LEN},  // #TVAR_ash_pGAS[355]

{0x0,  0x0005, BURST_LEN},  // #TVAR_ash_pGAS[356]

{0x0,  0x0003, BURST_LEN},  // #TVAR_ash_pGAS[357]

{0x0,  0x0012, BURST_LEN},  // #TVAR_ash_pGAS[358]

{0x0,  0x0033, BURST_LEN},  // #TVAR_ash_pGAS[359]

{0x0,  0x0061, BURST_LEN},  // #TVAR_ash_pGAS[360]

{0x0,  0x009A, BURST_LEN},  // #TVAR_ash_pGAS[361]

{0x0,  0x00E1, BURST_LEN},  // #TVAR_ash_pGAS[362]

{0x0,  0x0131, BURST_LEN},  // #TVAR_ash_pGAS[363]

{0x0,  0x00FB, BURST_LEN},  // #TVAR_ash_pGAS[364]

{0x0,  0x00B6, BURST_LEN},  // #TVAR_ash_pGAS[365]

{0x0,  0x0079, BURST_LEN},  // #TVAR_ash_pGAS[366]

{0x0,  0x0048, BURST_LEN},  // #TVAR_ash_pGAS[367]

{0x0,  0x0024, BURST_LEN},  // #TVAR_ash_pGAS[368]

{0x0,  0x000E, BURST_LEN},  // #TVAR_ash_pGAS[369]

{0x0,  0x000C, BURST_LEN},  // #TVAR_ash_pGAS[370]

{0x0,  0x001B, BURST_LEN},  // #TVAR_ash_pGAS[371]

{0x0,  0x003A, BURST_LEN},  // #TVAR_ash_pGAS[372]

{0x0,  0x0068, BURST_LEN},  // #TVAR_ash_pGAS[373]

{0x0,  0x00A0, BURST_LEN},  // #TVAR_ash_pGAS[374]

{0x0,  0x00E7, BURST_LEN},  // #TVAR_ash_pGAS[375]

{0x0,  0x0135, BURST_LEN},  // #TVAR_ash_pGAS[376]

{0x0,  0x0117, BURST_LEN},  // #TVAR_ash_pGAS[377]

{0x0,  0x00D4, BURST_LEN},  // #TVAR_ash_pGAS[378]

{0x0,  0x0096, BURST_LEN},  // #TVAR_ash_pGAS[379]

{0x0,  0x0065, BURST_LEN},  // #TVAR_ash_pGAS[380]

{0x0,  0x0040, BURST_LEN},  // #TVAR_ash_pGAS[381]

{0x0,  0x002C, BURST_LEN},  // #TVAR_ash_pGAS[382]

{0x0,  0x0027, BURST_LEN},  // #TVAR_ash_pGAS[383]

{0x0,  0x0036, BURST_LEN},  // #TVAR_ash_pGAS[384]

{0x0,  0x0054, BURST_LEN},  // #TVAR_ash_pGAS[385]

{0x0,  0x007F, BURST_LEN},  // #TVAR_ash_pGAS[386]

{0x0,  0x00B5, BURST_LEN},  // #TVAR_ash_pGAS[387]

{0x0,  0x00FB, BURST_LEN},  // #TVAR_ash_pGAS[388]

{0x0,  0x0146, BURST_LEN},  // #TVAR_ash_pGAS[389]

{0x0,  0x013C, BURST_LEN},  // #TVAR_ash_pGAS[390]

{0x0,  0x00F9, BURST_LEN},  // #TVAR_ash_pGAS[391]

{0x0,  0x00BB, BURST_LEN},  // #TVAR_ash_pGAS[392]

{0x0,  0x008D, BURST_LEN},  // #TVAR_ash_pGAS[393]

{0x0,  0x0069, BURST_LEN},  // #TVAR_ash_pGAS[394]

{0x0,  0x0053, BURST_LEN},  // #TVAR_ash_pGAS[395]

{0x0,  0x004F, BURST_LEN},  // #TVAR_ash_pGAS[396]

{0x0,  0x005D, BURST_LEN},  // #TVAR_ash_pGAS[397]

{0x0,  0x0079, BURST_LEN},  // #TVAR_ash_pGAS[398]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[399]

{0x0,  0x00D5, BURST_LEN},  // #TVAR_ash_pGAS[400]

{0x0,  0x011B, BURST_LEN},  // #TVAR_ash_pGAS[401]

{0x0,  0x0160, BURST_LEN},  // #TVAR_ash_pGAS[402]

{0x0,  0x0167, BURST_LEN},  // #TVAR_ash_pGAS[403]

{0x0,  0x012C, BURST_LEN},  // #TVAR_ash_pGAS[404]

{0x0,  0x00EF, BURST_LEN},  // #TVAR_ash_pGAS[405]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[406]

{0x0,  0x00A2, BURST_LEN},  // #TVAR_ash_pGAS[407]

{0x0,  0x008D, BURST_LEN},  // #TVAR_ash_pGAS[408]

{0x0,  0x008A, BURST_LEN},  // #TVAR_ash_pGAS[409]

{0x0,  0x0095, BURST_LEN},  // #TVAR_ash_pGAS[410]

{0x0,  0x00B0, BURST_LEN},  // #TVAR_ash_pGAS[411]

{0x0,  0x00D2, BURST_LEN},  // #TVAR_ash_pGAS[412]

{0x0,  0x0106, BURST_LEN},  // #TVAR_ash_pGAS[413]

{0x0,  0x0145, BURST_LEN},  // #TVAR_ash_pGAS[414]

{0x0,  0x0188, BURST_LEN},  // #TVAR_ash_pGAS[415]

{0x0,  0x0197, BURST_LEN},  // #TVAR_ash_pGAS[416]

{0x0,  0x0164, BURST_LEN},  // #TVAR_ash_pGAS[417]

{0x0,  0x012F, BURST_LEN},  // #TVAR_ash_pGAS[418]

{0x0,  0x0101, BURST_LEN},  // #TVAR_ash_pGAS[419]

{0x0,  0x00E0, BURST_LEN},  // #TVAR_ash_pGAS[420]

{0x0,  0x00CD, BURST_LEN},  // #TVAR_ash_pGAS[421]

{0x0,  0x00CA, BURST_LEN},  // #TVAR_ash_pGAS[422]

{0x0,  0x00D4, BURST_LEN},  // #TVAR_ash_pGAS[423]

{0x0,  0x00EC, BURST_LEN},  // #TVAR_ash_pGAS[424]

{0x0,  0x0111, BURST_LEN},  // #TVAR_ash_pGAS[425]

{0x0,  0x0141, BURST_LEN},  // #TVAR_ash_pGAS[426]

{0x0,  0x0176, BURST_LEN},  // #TVAR_ash_pGAS[427]

{0x0,  0x01DD, BURST_LEN},  // #TVAR_ash_pGAS[428]

{0x0,  0x0105, BURST_LEN},  // #TVAR_ash_pGAS[429]

{0x0,  0x00FD, BURST_LEN},  // #TVAR_ash_pGAS[430]

{0x0,  0x00D3, BURST_LEN},  // #TVAR_ash_pGAS[431]

{0x0,  0x00B2, BURST_LEN},  // #TVAR_ash_pGAS[432]

{0x0,  0x00A0, BURST_LEN},  // #TVAR_ash_pGAS[433]

{0x0,  0x0096, BURST_LEN},  // #TVAR_ash_pGAS[434]

{0x0,  0x0097, BURST_LEN},  // #TVAR_ash_pGAS[435]

{0x0,  0x00A5, BURST_LEN},  // #TVAR_ash_pGAS[436]

{0x0,  0x00BC, BURST_LEN},  // #TVAR_ash_pGAS[437]

{0x0,  0x00E2, BURST_LEN},  // #TVAR_ash_pGAS[438]

{0x0,  0x0117, BURST_LEN},  // #TVAR_ash_pGAS[439]

{0x0,  0x0149, BURST_LEN},  // #TVAR_ash_pGAS[440]

{0x0,  0x0187, BURST_LEN},  // #TVAR_ash_pGAS[441]

{0x0,  0x0100, BURST_LEN},  // #TVAR_ash_pGAS[442]

{0x0,  0x00CA, BURST_LEN},  // #TVAR_ash_pGAS[443]

{0x0,  0x00A1, BURST_LEN},  // #TVAR_ash_pGAS[444]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[445]

{0x0,  0x006F, BURST_LEN},  // #TVAR_ash_pGAS[446]

{0x0,  0x0068, BURST_LEN},  // #TVAR_ash_pGAS[447]

{0x0,  0x006A, BURST_LEN},  // #TVAR_ash_pGAS[448]

{0x0,  0x0077, BURST_LEN},  // #TVAR_ash_pGAS[449]

{0x0,  0x0091, BURST_LEN},  // #TVAR_ash_pGAS[450]

{0x0,  0x00B4, BURST_LEN},  // #TVAR_ash_pGAS[451]

{0x0,  0x00E7, BURST_LEN},  // #TVAR_ash_pGAS[452]

{0x0,  0x0125, BURST_LEN},  // #TVAR_ash_pGAS[453]

{0x0,  0x015B, BURST_LEN},  // #TVAR_ash_pGAS[454]

{0x0,  0x00DF, BURST_LEN},  // #TVAR_ash_pGAS[455]

{0x0,  0x00A6, BURST_LEN},  // #TVAR_ash_pGAS[456]

{0x0,  0x0075, BURST_LEN},  // #TVAR_ash_pGAS[457]

{0x0,  0x0059, BURST_LEN},  // #TVAR_ash_pGAS[458]

{0x0,  0x0044, BURST_LEN},  // #TVAR_ash_pGAS[459]

{0x0,  0x003B, BURST_LEN},  // #TVAR_ash_pGAS[460]

{0x0,  0x003D, BURST_LEN},  // #TVAR_ash_pGAS[461]

{0x0,  0x004B, BURST_LEN},  // #TVAR_ash_pGAS[462]

{0x0,  0x0064, BURST_LEN},  // #TVAR_ash_pGAS[463]

{0x0,  0x0089, BURST_LEN},  // #TVAR_ash_pGAS[464]

{0x0,  0x00B9, BURST_LEN},  // #TVAR_ash_pGAS[465]

{0x0,  0x00FC, BURST_LEN},  // #TVAR_ash_pGAS[466]

{0x0,  0x013C, BURST_LEN},  // #TVAR_ash_pGAS[467]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[468]

{0x0,  0x008A, BURST_LEN},  // #TVAR_ash_pGAS[469]

{0x0,  0x0059, BURST_LEN},  // #TVAR_ash_pGAS[470]

{0x0,  0x0038, BURST_LEN},  // #TVAR_ash_pGAS[471]

{0x0,  0x0023, BURST_LEN},  // #TVAR_ash_pGAS[472]

{0x0,  0x0018, BURST_LEN},  // #TVAR_ash_pGAS[473]

{0x0,  0x0019, BURST_LEN},  // #TVAR_ash_pGAS[474]

{0x0,  0x0028, BURST_LEN},  // #TVAR_ash_pGAS[475]

{0x0,  0x0042, BURST_LEN},  // #TVAR_ash_pGAS[476]

{0x0,  0x0068, BURST_LEN},  // #TVAR_ash_pGAS[477]

{0x0,  0x0097, BURST_LEN},  // #TVAR_ash_pGAS[478]

{0x0,  0x00D7, BURST_LEN},  // #TVAR_ash_pGAS[479]

{0x0,  0x011D, BURST_LEN},  // #TVAR_ash_pGAS[480]

{0x0,  0x00B1, BURST_LEN},  // #TVAR_ash_pGAS[481]

{0x0,  0x007B, BURST_LEN},  // #TVAR_ash_pGAS[482]

{0x0,  0x004A, BURST_LEN},  // #TVAR_ash_pGAS[483]

{0x0,  0x0027, BURST_LEN},  // #TVAR_ash_pGAS[484]

{0x0,  0x0011, BURST_LEN},  // #TVAR_ash_pGAS[485]

{0x0,  0x0007, BURST_LEN},  // #TVAR_ash_pGAS[486]

{0x0,  0x0007, BURST_LEN},  // #TVAR_ash_pGAS[487]

{0x0,  0x0014, BURST_LEN},  // #TVAR_ash_pGAS[488]

{0x0,  0x002D, BURST_LEN},  // #TVAR_ash_pGAS[489]

{0x0,  0x0053, BURST_LEN},  // #TVAR_ash_pGAS[490]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[491]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[492]

{0x0,  0x010B, BURST_LEN},  // #TVAR_ash_pGAS[493]

{0x0,  0x00AC, BURST_LEN},  // #TVAR_ash_pGAS[494]

{0x0,  0x0076, BURST_LEN},  // #TVAR_ash_pGAS[495]

{0x0,  0x0046, BURST_LEN},  // #TVAR_ash_pGAS[496]

{0x0,  0x0022, BURST_LEN},  // #TVAR_ash_pGAS[497]

{0x0,  0x000C, BURST_LEN},  // #TVAR_ash_pGAS[498]

{0x0,  0x0000, BURST_LEN},  // #TVAR_ash_pGAS[499]

{0x0,  0x0000, BURST_LEN},  // #TVAR_ash_pGAS[500]

{0x0,  0x000C, BURST_LEN},  // #TVAR_ash_pGAS[501]

{0x0,  0x0024, BURST_LEN},  // #TVAR_ash_pGAS[502]

{0x0,  0x004B, BURST_LEN},  // #TVAR_ash_pGAS[503]

{0x0,  0x007B, BURST_LEN},  // #TVAR_ash_pGAS[504]

{0x0,  0x00BA, BURST_LEN},  // #TVAR_ash_pGAS[505]

{0x0,  0x00FD, BURST_LEN},  // #TVAR_ash_pGAS[506]

{0x0,  0x00B5, BURST_LEN},  // #TVAR_ash_pGAS[507]

{0x0,  0x0080, BURST_LEN},  // #TVAR_ash_pGAS[508]

{0x0,  0x0050, BURST_LEN},  // #TVAR_ash_pGAS[509]

{0x0,  0x002C, BURST_LEN},  // #TVAR_ash_pGAS[510]

{0x0,  0x0015, BURST_LEN},  // #TVAR_ash_pGAS[511]

{0x0,  0x0009, BURST_LEN},  // #TVAR_ash_pGAS[512]

{0x0,  0x0008, BURST_LEN},  // #TVAR_ash_pGAS[513]

{0x0,  0x0013, BURST_LEN},  // #TVAR_ash_pGAS[514]

{0x0,  0x002A, BURST_LEN},  // #TVAR_ash_pGAS[515]

{0x0,  0x0050, BURST_LEN},  // #TVAR_ash_pGAS[516]

{0x0,  0x007F, BURST_LEN},  // #TVAR_ash_pGAS[517]

{0x0,  0x00BB, BURST_LEN},  // #TVAR_ash_pGAS[518]

{0x0,  0x0102, BURST_LEN},  // #TVAR_ash_pGAS[519]

{0x0,  0x00D3, BURST_LEN},  // #TVAR_ash_pGAS[520]

{0x0,  0x009A, BURST_LEN},  // #TVAR_ash_pGAS[521]

{0x0,  0x006A, BURST_LEN},  // #TVAR_ash_pGAS[522]

{0x0,  0x0046, BURST_LEN},  // #TVAR_ash_pGAS[523]

{0x0,  0x002E, BURST_LEN},  // #TVAR_ash_pGAS[524]

{0x0,  0x0022, BURST_LEN},  // #TVAR_ash_pGAS[525]

{0x0,  0x0021, BURST_LEN},  // #TVAR_ash_pGAS[526]

{0x0,  0x002B, BURST_LEN},  // #TVAR_ash_pGAS[527]

{0x0,  0x0041, BURST_LEN},  // #TVAR_ash_pGAS[528]

{0x0,  0x0065, BURST_LEN},  // #TVAR_ash_pGAS[529]

{0x0,  0x0091, BURST_LEN},  // #TVAR_ash_pGAS[530]

{0x0,  0x00D0, BURST_LEN},  // #TVAR_ash_pGAS[531]

{0x0,  0x010F, BURST_LEN},  // #TVAR_ash_pGAS[532]

{0x0,  0x00F1, BURST_LEN},  // #TVAR_ash_pGAS[533]

{0x0,  0x00BE, BURST_LEN},  // #TVAR_ash_pGAS[534]

{0x0,  0x008D, BURST_LEN},  // #TVAR_ash_pGAS[535]

{0x0,  0x006A, BURST_LEN},  // #TVAR_ash_pGAS[536]

{0x0,  0x0052, BURST_LEN},  // #TVAR_ash_pGAS[537]

{0x0,  0x0046, BURST_LEN},  // #TVAR_ash_pGAS[538]

{0x0,  0x0046, BURST_LEN},  // #TVAR_ash_pGAS[539]

{0x0,  0x0050, BURST_LEN},  // #TVAR_ash_pGAS[540]

{0x0,  0x0064, BURST_LEN},  // #TVAR_ash_pGAS[541]

{0x0,  0x0084, BURST_LEN},  // #TVAR_ash_pGAS[542]

{0x0,  0x00B1, BURST_LEN},  // #TVAR_ash_pGAS[543]

{0x0,  0x00EC, BURST_LEN},  // #TVAR_ash_pGAS[544]

{0x0,  0x012A, BURST_LEN},  // #TVAR_ash_pGAS[545]

{0x0,  0x011F, BURST_LEN},  // #TVAR_ash_pGAS[546]

{0x0,  0x00F0, BURST_LEN},  // #TVAR_ash_pGAS[547]

{0x0,  0x00C2, BURST_LEN},  // #TVAR_ash_pGAS[548]

{0x0,  0x009F, BURST_LEN},  // #TVAR_ash_pGAS[549]

{0x0,  0x0089, BURST_LEN},  // #TVAR_ash_pGAS[550]

{0x0,  0x007E, BURST_LEN},  // #TVAR_ash_pGAS[551]

{0x0,  0x007B, BURST_LEN},  // #TVAR_ash_pGAS[552]

{0x0,  0x0085, BURST_LEN},  // #TVAR_ash_pGAS[553]

{0x0,  0x0098, BURST_LEN},  // #TVAR_ash_pGAS[554]

{0x0,  0x00B3, BURST_LEN},  // #TVAR_ash_pGAS[555]

{0x0,  0x00DE, BURST_LEN},  // #TVAR_ash_pGAS[556]

{0x0,  0x0113, BURST_LEN},  // #TVAR_ash_pGAS[557]

{0x0,  0x014D, BURST_LEN},  // #TVAR_ash_pGAS[558]

{0x0,  0x014B, BURST_LEN},  // #TVAR_ash_pGAS[559]

{0x0,  0x0127, BURST_LEN},  // #TVAR_ash_pGAS[560]

{0x0,  0x00FD, BURST_LEN},  // #TVAR_ash_pGAS[561]

{0x0,  0x00DB, BURST_LEN},  // #TVAR_ash_pGAS[562]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_ash_pGAS[563]

{0x0,  0x00BA, BURST_LEN},  // #TVAR_ash_pGAS[564]

{0x0,  0x00B7, BURST_LEN},  // #TVAR_ash_pGAS[565]

{0x0,  0x00BF, BURST_LEN},  // #TVAR_ash_pGAS[566]

{0x0,  0x00D3, BURST_LEN},  // #TVAR_ash_pGAS[567]

{0x0,  0x00ED, BURST_LEN},  // #TVAR_ash_pGAS[568]

{0x0,  0x0114, BURST_LEN},  // #TVAR_ash_pGAS[569]

{0x0,  0x0143, BURST_LEN},  // #TVAR_ash_pGAS[570]

{0xFFFF,  0x0195, BURST_LEN},  // #TVAR_ash_pGAS[571]


{0x002A, 0x0D30, WORD_LEN},

{0x0F12,  0x02A7, WORD_LEN},  // #awbb_GLocusR

{0x0F12,  0x0343, WORD_LEN},  // #awbb_GLocusB

{0x002A, 0x06B8, WORD_LEN},

{0x0F12,  0x00D0, WORD_LEN},  // #TVAR_ash_AwbAshCord_0_

{0x0F12,  0x0102, WORD_LEN},  // #TVAR_ash_AwbAshCord_1_

{0x0F12,  0x010E, WORD_LEN},  // #TVAR_ash_AwbAshCord_2_

{0x0F12,  0x0137, WORD_LEN},  // #TVAR_ash_AwbAshCord_3_

{0x0F12,  0x0171, WORD_LEN},  // #TVAR_ash_AwbAshCord_4_

{0x0F12,  0x0198, WORD_LEN},  // #TVAR_ash_AwbAshCord_5_

{0x0F12,  0x01A8, WORD_LEN},  // #TVAR_ash_AwbAshCord_6_



//================================================================================================

// #SET CCM

//================================================================================================ 

// #CCM start address, BURST_LEN},  // #7000_33A4


{0x002A, 0x0698, WORD_LEN},

{0x0F12,  0x33A4, WORD_LEN},
{0x0F12,  0x7000, WORD_LEN},
// #Horizon

{0x002A, 0x33A4, WORD_LEN},

//mhlee

{0xFFFE,  0x01CB, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF8E, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFFD2, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFFDF, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x01BD, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms

{0x0,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms

// #Inca

{0x0,  0x01C7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[18]

{0x0,  0xFF88, BURST_LEN},  // #TVAR_wbt_pBaseCcms[19]

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms[20]

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms[21]

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms[22]

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms[23]

{0x0,  0xFFEE, BURST_LEN},  // #TVAR_wbt_pBaseCcms[24]

{0x0,  0xFFF7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[25]

{0x0,  0x01A0, BURST_LEN},  // #TVAR_wbt_pBaseCcms[26]

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[27]

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms[28]

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms[29]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms[30]

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[31]

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms[32]

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms[33]

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms[34]

{0x0,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms[35]

// #W

{0x0,  0x01C7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[36]

{0x0,  0xFF88, BURST_LEN},  // #TVAR_wbt_pBaseCcms[37]

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms[38]

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms[39]

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms[40]

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms[41]

{0x0,  0xFFEE, BURST_LEN},  // #TVAR_wbt_pBaseCcms[42]

{0x0,  0xFFF7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[43]

{0x0,  0x01A0, BURST_LEN},  // #TVAR_wbt_pBaseCcms[44]

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[45]

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms[46]

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms[47]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms[48]

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[49]

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms[50]

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms[51]

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms[52]

{0x0,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms[53]

// #C

{0x0,  0x01C7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[54]

{0x0,  0xFF88, BURST_LEN},  // #TVAR_wbt_pBaseCcms[55]

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms[56]

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms[57]

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms[58]

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms[59]

{0x0,  0xFFEE, BURST_LEN},  // #TVAR_wbt_pBaseCcms[60]

{0x0,  0xFFF7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[61]

{0x0,  0x01A0, BURST_LEN},  // #TVAR_wbt_pBaseCcms[62]

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[63]

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms[64]

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms[65]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms[66]

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[67]

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms[68]

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms[69]

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms[70]

{0x0,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms[71]

// #D

{0x0,  0x01C7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[72]

{0x0,  0xFF88, BURST_LEN},  // #TVAR_wbt_pBaseCcms[73]

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms[74]

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms[75]

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms[76]

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms[77]

{0x0,  0xFFEE, BURST_LEN},  // #TVAR_wbt_pBaseCcms[78]

{0x0,  0xFFF7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[79]

{0x0,  0x01A0, BURST_LEN},  // #TVAR_wbt_pBaseCcms[80]

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[81]

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms[82]

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms[83]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms[84]

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[85]

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms[86]

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms[87]

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms[88]

{0x0,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms[89]

// #D

{0x0,  0x01C7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[90]

{0x0,  0xFF88, BURST_LEN},  // #TVAR_wbt_pBaseCcms[91]

{0x0,  0xFFE9, BURST_LEN},  // #TVAR_wbt_pBaseCcms[92]

{0x0,  0xFF64, BURST_LEN},  // #TVAR_wbt_pBaseCcms[93]

{0x0,  0x01B2, BURST_LEN},  // #TVAR_wbt_pBaseCcms[94]

{0x0,  0xFF35, BURST_LEN},  // #TVAR_wbt_pBaseCcms[95]

{0x0,  0xFFEE, BURST_LEN},  // #TVAR_wbt_pBaseCcms[96]

{0x0,  0xFFF7, BURST_LEN},  // #TVAR_wbt_pBaseCcms[97]

{0x0,  0x01A0, BURST_LEN},  // #TVAR_wbt_pBaseCcms[98]

{0x0,  0x011C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[99]

{0x0,  0x011B, BURST_LEN},  // #TVAR_wbt_pBaseCcms[100]

{0x0,  0xFF43, BURST_LEN},  // #TVAR_wbt_pBaseCcms[101]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pBaseCcms[102]

{0x0,  0xFF4C, BURST_LEN},  // #TVAR_wbt_pBaseCcms[103]

{0x0,  0x01CC, BURST_LEN},  // #TVAR_wbt_pBaseCcms[104]

{0x0,  0xFF33, BURST_LEN},  // #TVAR_wbt_pBaseCcms[105]

{0x0,  0x0173, BURST_LEN},  // #TVAR_wbt_pBaseCcms[106]

{0xFFFF,  0x012F, BURST_LEN},  // #TVAR_wbt_pBaseCcms[107]

// #Outdoor CCM address, BURST_LEN},  // #7000_3380

{0x002A, 0x06A0, WORD_LEN},
{0x0F12, 0x3380, WORD_LEN},
{0x0F12, 0x7000, WORD_LEN},
// #Outdoor CCM

{0x002A, 0x3380, WORD_LEN},


{0xFFFE,  0x01C5, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[0]

{0x0,  0xFF90, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[1]

{0x0,  0xFFDB, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[2]

{0x0,  0xFF61, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[3]

{0x0,  0x01BD, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[4]

{0x0,  0xFF34, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[5]

{0x0,  0xFFFE, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[6]

{0x0,  0xFFF6, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[7]

{0x0,  0x019D, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[8]

{0x0,  0x0107, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[9]

{0x0,  0x010F, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[10] 

{0x0,  0xFF67, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[11] 

{0x0,  0x016C, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[12] 

{0x0,  0xFF54, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[13] 

{0x0,  0x01FC, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[14] 

{0x0,  0xFF82, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[15] 

{0x0,  0x015D, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[16] 

{0xFFFF,  0x00FD, BURST_LEN},  // #TVAR_wbt_pOutdoorCcm[17] 

//================================================================================================

// #SET AWB

//================================================================================================

// #Indoor boundary

{0x002A, 0x0C48, WORD_LEN},


//mhlee
{0xFFFE,  0x03E3, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[0] 

{0x0,  0x03F8, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[1] 

{0x0,  0x03A0, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[2] 

{0x0,  0x03F7, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[3] 

{0x0,  0x0359, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[4] 

{0x0,  0x03E6, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[5] 

{0x0,  0x0311, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[6] 

{0x0,  0x03B4, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[7] 

{0x0,  0x02D5, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[8] 

{0x0,  0x0385, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[9] 

{0x0,  0x02B0, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[10]

{0x0,  0x0355, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[11]

{0x0,  0x0293, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[12]

{0x0,  0x0327, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[13]

{0x0,  0x0278, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[14]

{0x0,  0x02F9, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[15]

{0x0,  0x025E, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[16]

{0x0,  0x02D9, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[17]

{0x0,  0x0246, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[18]

{0x0,  0x02C2, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[19]

{0x0,  0x022C, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[20]

{0x0,  0x02B0, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[21]

{0x0,  0x0214, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[22]

{0x0,  0x029B, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[23]

{0x0,  0x01FA, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[24]

{0x0,  0x0286, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[25]

{0x0,  0x01EC, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[26]

{0x0,  0x027B, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[27]

{0x0,  0x01E9, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[28]

{0x0,  0x0267, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[29]

{0x0,  0x01FE, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[30]

{0x0,  0x0248, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[31]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[32]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[33]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[34]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[35]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[36]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[37]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[38]

{0x0,  0x0000, BURST_LEN},  // #awbb_IndoorGrZones_m_BGrid[39]

{0xFFFF, 0x0005, BURST_LEN},  // #awbb_IndoorGrZones_m_GridStep

{0x002A, 0x0CA0, WORD_LEN},

{0x0F12, 0x0107, WORD_LEN},  // #awbb_IndoorGrZones_m_Boffs



// #Outdoor boundary

{0x002A, 0x0CA4, WORD_LEN},

//mhlee

{0xFFFE,  0x0277, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[0] 

{0x0,  0x0294, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[1] 

{0x0,  0x0243, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[2] 

{0x0,  0x028B, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[3] 

{0x0,  0x0222, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[4] 

{0x0,  0x027D, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[5] 

{0x0,  0x020B, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[6] 

{0x0,  0x026E, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[7] 

{0x0,  0x0201, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[8] 

{0x0,  0x025B, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[9] 

{0x0,  0x021A, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[10]

{0x0,  0x023A, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[11]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[12]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[13]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[14]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[15]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[16]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[17]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[18]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[19]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[20]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[21]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[22]

{0x0,  0x0000, BURST_LEN},  // #awbb_OutdoorGrZones_m_BGrid[23]

{0xFFFF,  0x0005, BURST_LEN},  // #awbb_OutdoorGrZones_m_GridStep

{0x002A, 0x0CDC, WORD_LEN},

{0x0F12,  0x0234, WORD_LEN},  // #awbb_OutdoorGrZones_m_Boffs

 

// #Outdoor detection zone??

{0x002A, 0x0D88, WORD_LEN},

{0xFFFE,  0xFFB6, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[0]_m_left

{0x0,  0x00B6, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[0]_m_right

{0x0,  0xFF38, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[1]_m_left

{0x0,  0x0118, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[1]_m_right

{0x0,  0xFEF1, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[2]_m_left

{0x0,  0x015F, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[2]_m_right

{0x0,  0xFEC0, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[3]_m_left

{0x0,  0x0199, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[3]_m_right

{0x0,  0xFE91, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[4]_m_left

{0x0,  0x01CF, BURST_LEN},  // #awbb_OutdoorDetectionZone_m_BGrid[4]_m_right

{0x0,  0x1388, BURST_LEN},  // #awbb_OutdoorDetectionZone_ZInfo_m_AbsGridStep

{0x0,  0x0000, BURST_LEN},
{0x0,  0x0005, BURST_LEN},  // #awbb_OutdoorDetectionZone_ZInfo_m_GridSz

{0x0,  0x0000, BURST_LEN},
{0x0,  0x1387, BURST_LEN},  // #awbb_OutdoorDetectionZone_ZInfo_m_NBoffs

{0x0,  0x0000, BURST_LEN},
{0x0,  0x05DC, BURST_LEN},  // #awbb_OutdoorDetectionZone_ZInfo_m_MaxNB, BURST_LEN},  // NB 1500

{0xFFFF,  0x0000, BURST_LEN},
              

// #LowBr boundary

{0x002A, 0x0CE0, WORD_LEN},


{0xFFFE,  0x03FC, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[0]

{0x0,  0x0416, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[1]

{0x0,  0x0381, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[2]

{0x0,  0x0453, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[3]

{0x0,  0x02F9, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[4]

{0x0,  0x042C, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[5]

{0x0,  0x029A, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[6]

{0x0,  0x03FA, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[7]

{0x0,  0x0265, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[8]

{0x0,  0x03A0, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[9]

{0x0,  0x023B, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[10]

{0x0,  0x0353, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[11]

{0x0,  0x020B, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[12]

{0x0,  0x030F, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[13]

{0x0,  0x01DF, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[14]

{0x0,  0x02D9, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[15]

{0x0,  0x01C9, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[16]

{0x0,  0x02AE, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[17]

{0x0,  0x01C6, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[18]

{0x0,  0x027A, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[19]

{0x0,  0x0202, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[20]

{0x0,  0x022C, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[21]

{0x0,  0x0000, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[22]

{0x0,  0x0000, BURST_LEN},  // #awbb_LowBrGrZones_m_BGrid[23]

{0xFFFF,  0x0006, BURST_LEN},  // #awbb_LowBrGrZones_m_GridStep

{0x002A, 0x0D18, WORD_LEN},

{0x0F12,  0x00A7, WORD_LEN},  // #awbb_LowBrGrZones_m_Boffs



// #AWB ETC

{0x002A, 0x0D1C, WORD_LEN},

{0x0F12, 0x036C, WORD_LEN},  // #awbb_CrclLowT_R_c

{0x002A, 0x0D20, 
WORD_LEN},
{0x0F12, 0x011D, WORD_LEN},  // #awbb_CrclLowT_B_c

{0x002A, 0x0D24, WORD_LEN},

{0x0F12,  0x62B8, WORD_LEN},  // #awbb_CrclLowT_Rad_c

 

{0x002A, 0x0D2C, WORD_LEN},
{0x0F12,  0x0135, WORD_LEN},  // #awbb_IntcR

{0x0F12,  0x012B, WORD_LEN},  // #awbb_IntcB

 

{0x002A, 0x0D28, WORD_LEN},
{0x0F12, 0x0269, WORD_LEN},  // #awbb_OutdoorWP_r

{0x0F12, 0x0240, WORD_LEN},  // #awbb_OutdoorWP_b

 

{0x002A, 0x0E4C, WORD_LEN},
{0x0F12,  0x0000, WORD_LEN},  // #awbboost_useBoosting4Outdoor

 

{0x002A, 0x0D4C, WORD_LEN},
{0x0F12,  0x0187, WORD_LEN},  // #awbb_GamutWidthThr1

{0x0F12,  0x00CF, WORD_LEN},  // #awbb_GamutHeightThr1

{0x0F12,  0x000D, WORD_LEN},  // #awbb_GamutWidthThr2

{0x0F12,  0x000A, WORD_LEN},  // #awbb_GamutHeightThr2

 

{0x002A, 0x0D5C, WORD_LEN},
{0x0F12,  0x7FFF, WORD_LEN},  // #awbb_LowTempRB

{0x0F12,  0x0050, WORD_LEN},  // #awbb_LowTemp_RBzone

 

{0x002A, 0x0D46, 
WORD_LEN},
{0x0F12,  0x0420, WORD_LEN},  // #awbb_MvEq_RBthresh

 

{0x002A, 0x0D4A, WORD_LEN},

{0x0F12,  0x000A, WORD_LEN},  //#awbb_MovingScale10

 

{0x002A, 0x0E3E, WORD_LEN},

{0x0F12,  0x0000, WORD_LEN},  // #awbb_rpl_InvalidOutdoor off

//002A 22DE

//{0x0F12,  0x0004, BURST_LEN},  // #Mon_AWB_ByPassMode, BURST_LEN},  // #[0]Outdoor [1]LowBr [2]LowTemp

 

//002A 337C

//{0x0F12,  0x00B3, BURST_LEN},  // #Tune_TP_ChMoveToNearR

//{0x0F12,  0x0040, BURST_LEN},  // #Tune_TP_AvMoveToGamutDist



// #AWB initial point

{0x002A, 0x0E44, WORD_LEN},

{0x0F12,  0x05CC, WORD_LEN},  // #define awbb_GainsInit_0_

{0x0F12,  0x0400, WORD_LEN},  // #define awbb_GainsInit_1_

{0x0F12,  0x0655, WORD_LEN},  // #define awbb_GainsInit_2_

// #Set AWB global offset

{0x002A, 0x0E36, WORD_LEN},

{0x0F12,  0x0000, WORD_LEN},  // #awbb_RGainOff

{0x0F12,  0x0000, WORD_LEN},  // #awbb_BGainOff

{0x0F12,  0x0000, WORD_LEN},  // #awbb_GGainOff



//================================================================================================

// #SET GRID OFFSET

//================================================================================================

{0x002A, 0x0DD4, WORD_LEN},
    

//mhlee                                                               

{0xFFFE,  0x0000, BURST_LEN},  //awbb_GridCorr_R[0]  //        	                                                                        

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[1]  //    

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[2] //    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[3]  //    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[4]  //    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[5]  //      	    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[6]  //    

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[7]  //    

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[8]  //    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[9]  //    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[10]  //   

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[11]  //     	    

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[12]  //   

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[13]  //   

{0x0,  0x0005, BURST_LEN},  //awbb_GridCorr_R[14]  //   

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[15]  //   

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[16]  //   

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_R[17]  //    	    



{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[0] //                                                                        

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[1] //                                                                          

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[2]//                                                                          

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[3] //                                                                          

{0x0,  0xFF38, BURST_LEN},  //awbb_GridCorr_B[4]  //                                                                          

{0x0,  0xFE70, BURST_LEN},  //awbb_GridCorr_B[5]  //                                                                           	

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[6]  //                                                                          

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[7]  //                                                                          

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[8]//                                                                          

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[9] //                                                                          

{0x0,  0xFF38, BURST_LEN},  //awbb_GridCorr_B[10] //                                                                         

{0x0,  0xFE70, BURST_LEN},  //awbb_GridCorr_B[11]  //                                                                           	

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[12]  //                                                                         

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[13]  //                                                                         

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[14]  //                                                                         

{0x0,  0x0000, BURST_LEN},  //awbb_GridCorr_B[15]  //                                                                         

{0x0,  0xFF38, BURST_LEN},  //awbb_GridCorr_B[16] //   

{0x0,  0xFE70, BURST_LEN},  //awbb_GridCorr_B[17] //                 

    	    

{0x0,  0x02D9, BURST_LEN},  //awbb_GridConst_1[0]  //                                                                     

{0x0,  0x0357, BURST_LEN},  //awbb_GridConst_1[1]  //                                                                     

{0x0,  0x03D1, BURST_LEN},  //awbb_GridConst_1[2]  //                                                                     

    	    

    	    

{0x0,  0x0DF6, BURST_LEN},  //awbb_GridConst_2[0] //                                                   

{0x0,  0x0E8C, BURST_LEN},  //awbb_GridConst_2[1] //                                                   

{0x0,  0x0F34, BURST_LEN},  //awbb_GridConst_2[2]  //                                                   

{0x0,  0x0F8F, BURST_LEN},  //awbb_GridConst_2[3]  //                                                   

{0x0,  0x0F9D, BURST_LEN},  //awbb_GridConst_2[4]  //                                                   

{0x0,  0x0FF3, BURST_LEN},  //awbb_GridConst_2[5]  //                                                   

    	    

{0x0,  0x00AC, BURST_LEN},  //awbb_GridCoeff_R_1  //awbb_GridCoeff_R_1                                                                   

{0x0,  0x00BD, BURST_LEN},  //awbb_GridCoeff_B_1                                                                   

{0x0,  0x0049, BURST_LEN},  //awbb_GridCoeff_R_2                                                                   

{0xFFFF,  0x00F5, BURST_LEN},  //awbb_GridCoeff_B_2        	    

{0x002A, 0x0E4A, WORD_LEN},                         

{0x0F12,  0x0002, WORD_LEN},  //awbb_GridEnable



//================================================================================================

// #SET GAMMA

//================================================================================================

//Our//old  //STW

{0x002A, 0x3288, WORD_LEN},

//mhlee  

{0xFFFE,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__0_ 70003288 

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__1_ 7000328A 

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__2_ 7000328C 

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__3_ 7000328E 

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__4_ 70003290 

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__5_ 70003292 

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__6_ 70003294 

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__7_ 70003296 

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__8_ 70003298 

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__9_ 7000329A 

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__10_ 7000329C 

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__11_ 7000329E 

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__12_ 700032A0 

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__13_ 700032A2 

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__14_ 700032A4

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__15_ 700032A6

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__16_ 700032A8

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__17_ 700032AA

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__18_ 700032AC

{0x0,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_0__19_ 700032AE

{0x0,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__0_ 700032B0

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__1_ 700032B2

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__2_ 700032B4

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__3_ 700032B6

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__4_ 700032B8

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__5_ 700032BA

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__6_ 700032BC

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__7_ 700032BE

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__8_ 700032C0

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__9_ 700032C2

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__10_ 700032C4

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__11_ 700032C6

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__12_ 700032C8

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__13_ 700032CA

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__14_ 700032CC

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__15_ 700032CE

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__16_ 700032D0

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__17_ 700032D2

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__18_ 700032D4

{0x0,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_1__19_ 700032D6

{0x0,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__0_ 700032D8

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__1_ 700032DA

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__2_ 700032DC

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__3_ 700032DE

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__4_ 700032E0

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__5_ 700032E2

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__6_ 700032E4

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__7_ 700032E6

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__8_ 700032E8

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__9_ 700032EA

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__10_ 700032EC

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__11_ 700032EE

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__12_ 700032F0

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__13_ 700032F2

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__14_ 700032F4

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__15_ 700032F6

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__16_ 700032F8

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__17_ 700032FA

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__18_ 700032FC

{0x0,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBIndoor_2__19_ 700032FE

 

{0x0,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__0_ 70003300

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__1_ 70003302

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__2_ 70003304

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__3_ 70003306

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__4_ 70003308

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__5_ 7000330A

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__6_ 7000330C

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__7_ 7000330E

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__8_ 70003310

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__9_ 70003312

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__10_70003314

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__11_70003316

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__12_70003318

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__13_7000331A

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__14_7000331C

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__15_7000331E

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__16_70003320

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__17_70003322

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__18_70003324

{0x0,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_0__19_70003326

{0x0,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__0_ 70003328

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__1_ 7000332A

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__2_ 7000332C

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__3_ 7000332E

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__4_ 70003330

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__5_ 70003332

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__6_ 70003334

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__7_ 70003336

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__8_ 70003338

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__9_ 7000333A

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__10_7000333C

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__11_7000333E

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__12_70003340

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__13_70003342

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__14_70003344

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__15_70003346

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__16_70003348

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__17_7000334A

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__18_7000334C

{0x0,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_1__19_7000334E

{0x0,  0x0000, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__0_ 70003350

{0x0,  0x0004, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__1_ 70003352

{0x0,  0x0010, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__2_ 70003354

{0x0,  0x002A, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__3_ 70003356

{0x0,  0x0062, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__4_ 70003358

{0x0,  0x00D5, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__5_ 7000335A

{0x0,  0x0138, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__6_ 7000335C

{0x0,  0x0161, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__7_ 7000335E

{0x0,  0x0186, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__8_ 70003360

{0x0,  0x01BC, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__9_ 70003362

{0x0,  0x01E8, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__10_70003364

{0x0,  0x020F, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__11_70003366

{0x0,  0x0232, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__12_70003368

{0x0,  0x0273, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__13_7000336A

{0x0,  0x02AF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__14_7000336C

{0x0,  0x0309, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__15_7000336E

{0x0,  0x0355, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__16_70003370

{0x0,  0x0394, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__17_70003372

{0x0,  0x03CE, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__18_70003374

{0xFFFF,  0x03FF, BURST_LEN},  // #SARR_usDualGammaLutRGBOutdoor_2__19_70003376

 

{0x002A, 0x06A6, WORD_LEN},

{0x0F12,  0x00D8, WORD_LEN},  // #SARR_AwbCcmCord_0_

{0x0F12,  0x00FC, WORD_LEN},  // #SARR_AwbCcmCord_1_

{0x0F12,  0x0120, WORD_LEN},  // #SARR_AwbCcmCord_2_

{0x0F12,  0x014C, WORD_LEN},  // #SARR_AwbCcmCord_3_

{0x0F12,  0x0184, WORD_LEN},  // #SARR_AwbCcmCord_4_

{0x0F12,  0x01AD, WORD_LEN},  // #SARR_AwbCcmCord_5_

 

//================================================================================================

// #SET AFIT

//================================================================================================

// #Noise index

{0x002A, 0x0764, WORD_LEN},

{0x0F12,  0x0041, WORD_LEN},  // #afit_uNoiseIndInDoor[0], BURST_LEN},  // #65

{0x0F12,  0x0063, WORD_LEN},  // #afit_uNoiseIndInDoor[1], BURST_LEN},  // #99

{0x0F12,  0x00C8, WORD_LEN},  // #afit_uNoiseIndInDoor[2], BURST_LEN},  // #200

{0x0F12,  0x015E, WORD_LEN},  // #afit_uNoiseIndInDoor[3], BURST_LEN},  // #350

{0x0F12,  0x028A, WORD_LEN},  // #afit_uNoiseIndInDoor[4], BURST_LEN},  // #650

// #AFIT table start address, BURST_LEN},  // #7000_07C4

{0x002A, 0x0770, WORD_LEN},

{0x0F12,  0x07C4, WORD_LEN},
{0x0F12,  0x7000, WORD_LEN},
// #AFIT table (Variables)

{0x002A, 0x07C4, WORD_LEN},
//mhlee

{0xFFFE,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[0]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[1]

{0x0,  0xFFFB, BURST_LEN},  // #TVAR_afit_pBaseVals[2]

{0x0,  0x0005, BURST_LEN},  // #TVAR_afit_pBaseVals[3]

{0x0,  0xFFF6, BURST_LEN},  // #TVAR_afit_pBaseVals[4]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_afit_pBaseVals[5]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[6]

{0x0,  0x009C, BURST_LEN},  // #TVAR_afit_pBaseVals[7]

{0x0,  0x017C, BURST_LEN},  // #TVAR_afit_pBaseVals[8]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[9]

{0x0,  0x000C, BURST_LEN},  // #TVAR_afit_pBaseVals[10]

{0x0,  0x0010, BURST_LEN},  // #TVAR_afit_pBaseVals[11]

{0x0,  0x012C, BURST_LEN},  // #TVAR_afit_pBaseVals[12]

{0x0,  0x03E8, BURST_LEN},  // #TVAR_afit_pBaseVals[13]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[14]

{0x0,  0x005A, BURST_LEN},  // #TVAR_afit_pBaseVals[15]

{0x0,  0x0070, BURST_LEN},  // #TVAR_afit_pBaseVals[16]

{0x0,  0x002D, BURST_LEN},  // #TVAR_afit_pBaseVals[17]

{0x0,  0x002D, BURST_LEN},  // #TVAR_afit_pBaseVals[18]

{0x0,  0x01AA, BURST_LEN},  // #TVAR_afit_pBaseVals[19]

{0x0,  0x0064, BURST_LEN},  // #TVAR_afit_pBaseVals[20]

{0x0,  0x0064, BURST_LEN},  // #TVAR_afit_pBaseVals[21]

{0x0,  0x000F, BURST_LEN},  // #TVAR_afit_pBaseVals[22]

{0x0,  0x000F, BURST_LEN},  // #TVAR_afit_pBaseVals[23]

{0x0,  0x0032, BURST_LEN},  // #TVAR_afit_pBaseVals[24]

{0x0,  0x0012, BURST_LEN},  // #TVAR_afit_pBaseVals[25]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[26]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[27]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[28]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[29]

{0x0,  0x0A24, BURST_LEN},  // #TVAR_afit_pBaseVals[30]

{0x0,  0x1701, BURST_LEN},  // #TVAR_afit_pBaseVals[31]

{0x0,  0x0229, BURST_LEN},  // #TVAR_afit_pBaseVals[32]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[33]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[34]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[35]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[36]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[37]

{0x0,  0x043B, BURST_LEN},  // #TVAR_afit_pBaseVals[38]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[39]

{0x0,  0x0301, BURST_LEN},  // #TVAR_afit_pBaseVals[40]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[41]

{0x0,  0x051E, BURST_LEN},  // #TVAR_afit_pBaseVals[42]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[43]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[44]

{0x0,  0x0A05, BURST_LEN},  // #TVAR_afit_pBaseVals[45]

{0x0,  0x0A3C, BURST_LEN},  // #TVAR_afit_pBaseVals[46]

{0x0,  0x0A28, BURST_LEN},  // #TVAR_afit_pBaseVals[47]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[48]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[49]

{0x0,  0x1102, BURST_LEN},  // #TVAR_afit_pBaseVals[50]

{0x0,  0x001B, BURST_LEN},  // #TVAR_afit_pBaseVals[51]

{0x0,  0x0900, BURST_LEN},  // #TVAR_afit_pBaseVals[52]

{0x0,  0x0600, BURST_LEN},  // #TVAR_afit_pBaseVals[53]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[54]

{0x0,  0x0305, BURST_LEN},  // #TVAR_afit_pBaseVals[55]

{0x0,  0x3C03, BURST_LEN},  // #TVAR_afit_pBaseVals[56]

{0x0,  0x006E, BURST_LEN},  // #TVAR_afit_pBaseVals[57]

{0x0,  0x0178, BURST_LEN},  //#TVAR_afit_pBaseVals[58]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[59]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[60]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[61]

{0x0,  0x5002, BURST_LEN},  // #TVAR_afit_pBaseVals[62]

{0x0,  0x8250, BURST_LEN},  // #TVAR_afit_pBaseVals[63]

{0x0,  0x2882, BURST_LEN},  // #TVAR_afit_pBaseVals[64]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[65]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[66]

{0x0,  0x1E0C, BURST_LEN},  // #TVAR_afit_pBaseVals[67]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[68]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[69]

{0x0,  0x4104, BURST_LEN},  // #TVAR_afit_pBaseVals[70]

{0x0,  0x123C, BURST_LEN},  // #TVAR_afit_pBaseVals[71]

{0x0,  0x4012, BURST_LEN},  // #TVAR_afit_pBaseVals[72]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[73]

{0x0,  0x1E03, BURST_LEN},  // #TVAR_afit_pBaseVals[74]

{0x0,  0x011E, BURST_LEN},  // #TVAR_afit_pBaseVals[75]

{0x0,  0x0201, BURST_LEN},  // #TVAR_afit_pBaseVals[76]

{0x0,  0x5050, BURST_LEN},  // #TVAR_afit_pBaseVals[77]

{0x0,  0x3C3C, BURST_LEN},  // #TVAR_afit_pBaseVals[78]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[79]

{0x0,  0x030A, BURST_LEN},  // #TVAR_afit_pBaseVals[80]

{0x0,  0x0714, BURST_LEN},  // #TVAR_afit_pBaseVals[81]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[82]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[83]

{0x0,  0x0432, BURST_LEN},  // #TVAR_afit_pBaseVals[84]

{0x0,  0x4050, BURST_LEN},  // #TVAR_afit_pBaseVals[85]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[86]

{0x0,  0x0440, BURST_LEN},  // #TVAR_afit_pBaseVals[87]

{0x0,  0x0302, BURST_LEN},  // #TVAR_afit_pBaseVals[88]

{0x0,  0x1E1E, BURST_LEN},  // #TVAR_afit_pBaseVals[89]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[90]

{0x0,  0x5002, BURST_LEN},  // #TVAR_afit_pBaseVals[91]

{0x0,  0x3C50, BURST_LEN},  // #TVAR_afit_pBaseVals[92]

{0x0,  0x283C, BURST_LEN},  // #TVAR_afit_pBaseVals[93]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[94]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[95]

{0x0,  0x1E07, BURST_LEN},  // #TVAR_afit_pBaseVals[96]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[97]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[98]

{0x0,  0x5004, BURST_LEN},  // #TVAR_afit_pBaseVals[99]

{0x0,  0x0F40, BURST_LEN},  // #TVAR_afit_pBaseVals[100]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[101]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[102]

{0x0,  0x0003, BURST_LEN},  // #TVAR_afit_pBaseVals[103]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[104]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[105]

{0x0,  0xFFFD, BURST_LEN},  // #TVAR_afit_pBaseVals[106]

{0x0,  0x0005, BURST_LEN},  // #TVAR_afit_pBaseVals[107]

{0x0,  0xFFF6, BURST_LEN},  // #TVAR_afit_pBaseVals[108]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_afit_pBaseVals[109]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[110]

{0x0,  0x009C, BURST_LEN},  // #TVAR_afit_pBaseVals[111]

{0x0,  0x017C, BURST_LEN},  // #TVAR_afit_pBaseVals[112]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[113]

{0x0,  0x000C, BURST_LEN},  // #TVAR_afit_pBaseVals[114]

{0x0,  0x0010, BURST_LEN},  // #TVAR_afit_pBaseVals[115]

{0x0,  0x012C, BURST_LEN},  // #TVAR_afit_pBaseVals[116]

{0x0,  0x03E8, BURST_LEN},  // #TVAR_afit_pBaseVals[117]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[118]

{0x0,  0x005A, BURST_LEN},  // #TVAR_afit_pBaseVals[119]

{0x0,  0x0070, BURST_LEN},  // #TVAR_afit_pBaseVals[120]

{0x0,  0x0023, BURST_LEN},  // #TVAR_afit_pBaseVals[121]

{0x0,  0x0023, BURST_LEN},  // #TVAR_afit_pBaseVals[122]

{0x0,  0x01AA, BURST_LEN},  // #TVAR_afit_pBaseVals[123]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[124]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[125]

{0x0,  0x000F, BURST_LEN},  // #TVAR_afit_pBaseVals[126]

{0x0,  0x000F, BURST_LEN},  // #TVAR_afit_pBaseVals[127]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[128]

{0x0,  0x0019, BURST_LEN},  // #TVAR_afit_pBaseVals[129]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[130]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[131]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[132]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[133]

{0x0,  0x0A24, BURST_LEN},  // #TVAR_afit_pBaseVals[134]

{0x0,  0x1701, BURST_LEN},  // #TVAR_afit_pBaseVals[135]

{0x0,  0x0229, BURST_LEN},  // #TVAR_afit_pBaseVals[136]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[137]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[138]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[139]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[140]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[141]

{0x0,  0x043B, BURST_LEN},  // #TVAR_afit_pBaseVals[142]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[143]

{0x0,  0x0301, BURST_LEN},  // #TVAR_afit_pBaseVals[144]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[145]

{0x0,  0x051E, BURST_LEN},  // #TVAR_afit_pBaseVals[146]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[147]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[148]

{0x0,  0x0A03, BURST_LEN},  // #TVAR_afit_pBaseVals[149]

{0x0,  0x0A3C, BURST_LEN},  // #TVAR_afit_pBaseVals[150]

{0x0,  0x0A28, BURST_LEN},  // #TVAR_afit_pBaseVals[151]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[152]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[153]

{0x0,  0x1102, BURST_LEN},  // #TVAR_afit_pBaseVals[154]

{0x0,  0x001B, BURST_LEN},  // #TVAR_afit_pBaseVals[155]

{0x0,  0x0900, BURST_LEN},  // #TVAR_afit_pBaseVals[156]

{0x0,  0x0600, BURST_LEN},  // #TVAR_afit_pBaseVals[157]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[158]

{0x0,  0x0305, BURST_LEN},  // #TVAR_afit_pBaseVals[159]

{0x0,  0x4603, BURST_LEN},  // #TVAR_afit_pBaseVals[160]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[161]

{0x0,  0x0180, BURST_LEN},  // #TVAR_afit_pBaseVals[162]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[163]

{0x0,  0x1919, BURST_LEN},  // #TVAR_afit_pBaseVals[164]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[165]

{0x0,  0x3C02, BURST_LEN},  // #TVAR_afit_pBaseVals[166]

{0x0,  0x6E3C, BURST_LEN},  // #TVAR_afit_pBaseVals[167]

{0x0,  0x286E, BURST_LEN},  // #TVAR_afit_pBaseVals[168]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[169]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[170]

{0x0,  0x1E0C, BURST_LEN},  // #TVAR_afit_pBaseVals[171]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[172]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[173]

{0x0,  0x4104, BURST_LEN},  // #TVAR_afit_pBaseVals[174]

{0x0,  0x123C, BURST_LEN},  // #TVAR_afit_pBaseVals[175]

{0x0,  0x4012, BURST_LEN},  // #TVAR_afit_pBaseVals[176]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[177]

{0x0,  0x1E03, BURST_LEN},  // #TVAR_afit_pBaseVals[178]

{0x0,  0x011E, BURST_LEN},  // #TVAR_afit_pBaseVals[179]

{0x0,  0x0201, BURST_LEN},  // #TVAR_afit_pBaseVals[180]

{0x0,  0x3232, BURST_LEN},  // #TVAR_afit_pBaseVals[181]

{0x0,  0x3C3C, BURST_LEN},  // #TVAR_afit_pBaseVals[182]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[183]

{0x0,  0x030A, BURST_LEN},  // #TVAR_afit_pBaseVals[184]

{0x0,  0x0714, BURST_LEN},  // #TVAR_afit_pBaseVals[185]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[186]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[187]

{0x0,  0x0432, BURST_LEN},  // #TVAR_afit_pBaseVals[188]

{0x0,  0x4050, BURST_LEN},  // #TVAR_afit_pBaseVals[189]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[190]

{0x0,  0x0440, BURST_LEN},  // #TVAR_afit_pBaseVals[191]

{0x0,  0x0302, BURST_LEN},  // #TVAR_afit_pBaseVals[192]

{0x0,  0x1E1E, BURST_LEN},  // #TVAR_afit_pBaseVals[193]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[194]

{0x0,  0x3202, BURST_LEN},  // #TVAR_afit_pBaseVals[195]

{0x0,  0x3C32, BURST_LEN},  // #TVAR_afit_pBaseVals[196]

{0x0,  0x283C, BURST_LEN},  // #TVAR_afit_pBaseVals[197]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[198]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[199]

{0x0,  0x1E07, BURST_LEN},  // #TVAR_afit_pBaseVals[200]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[201]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[202]

{0x0,  0x5004, BURST_LEN},  // #TVAR_afit_pBaseVals[203]

{0x0,  0x0F40, BURST_LEN},  // #TVAR_afit_pBaseVals[204]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[205]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[206]

{0x0,  0x0003, BURST_LEN},  // #TVAR_afit_pBaseVals[207]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[208]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[209]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[210]

{0x0,  0x0005, BURST_LEN},  // #TVAR_afit_pBaseVals[211]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[212]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_afit_pBaseVals[213]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[214]

{0x0,  0x009C, BURST_LEN},  // #TVAR_afit_pBaseVals[215]

{0x0,  0x017C, BURST_LEN},  // #TVAR_afit_pBaseVals[216]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[217]

{0x0,  0x000C, BURST_LEN},  // #TVAR_afit_pBaseVals[218]

{0x0,  0x0010, BURST_LEN},  // #TVAR_afit_pBaseVals[219]

{0x0,  0x012C, BURST_LEN},  // #TVAR_afit_pBaseVals[220]

{0x0,  0x03E8, BURST_LEN},  // #TVAR_afit_pBaseVals[221]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[222]

{0x0,  0x0078, BURST_LEN},  // #TVAR_afit_pBaseVals[223]

{0x0,  0x0070, BURST_LEN},  // #TVAR_afit_pBaseVals[224]

{0x0,  0x0014, BURST_LEN},  // #TVAR_afit_pBaseVals[225]

{0x0,  0x0014, BURST_LEN},  // #TVAR_afit_pBaseVals[226]

{0x0,  0x01AA, BURST_LEN},  // #TVAR_afit_pBaseVals[227]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[228]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[229]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[230]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[231]

{0x0,  0x0064, BURST_LEN},  // #TVAR_afit_pBaseVals[232]

{0x0,  0x001B, BURST_LEN},  // #TVAR_afit_pBaseVals[233]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[234]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[235]

{0x0,  0x002A, BURST_LEN},  // #TVAR_afit_pBaseVals[236]

{0x0,  0x0024, BURST_LEN},  // #TVAR_afit_pBaseVals[237]

{0x0,  0x0A24, BURST_LEN},  // #TVAR_afit_pBaseVals[238]

{0x0,  0x1701, BURST_LEN},  // #TVAR_afit_pBaseVals[239]

{0x0,  0x0229, BURST_LEN},  // #TVAR_afit_pBaseVals[240]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[241]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[242]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[243]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[244]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[245]

{0x0,  0x043B, BURST_LEN},  // #TVAR_afit_pBaseVals[246]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[247]

{0x0,  0x0301, BURST_LEN},  // #TVAR_afit_pBaseVals[248]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[249]

{0x0,  0x051E, BURST_LEN},  // #TVAR_afit_pBaseVals[250]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[251]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[252]

{0x0,  0x0A03, BURST_LEN},  // #TVAR_afit_pBaseVals[253]

{0x0,  0x0A3C, BURST_LEN},  // #TVAR_afit_pBaseVals[254]

{0x0,  0x0528, BURST_LEN},  // #TVAR_afit_pBaseVals[255]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[256]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[257]

{0x0,  0x1102, BURST_LEN},  // #TVAR_afit_pBaseVals[258]

{0x0,  0x001B, BURST_LEN},  // #TVAR_afit_pBaseVals[259]

{0x0,  0x0900, BURST_LEN},  // #TVAR_afit_pBaseVals[260]

{0x0,  0x0600, BURST_LEN},  // #TVAR_afit_pBaseVals[261]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[262]

{0x0,  0x0305, BURST_LEN},  // #TVAR_afit_pBaseVals[263]

{0x0,  0x4603, BURST_LEN},  // #TVAR_afit_pBaseVals[264]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[265]

{0x0,  0x0180, BURST_LEN},  // #TVAR_afit_pBaseVals[266]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[267]

{0x0,  0x2323, BURST_LEN},  // #TVAR_afit_pBaseVals[268]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[269]

{0x0,  0x2A02, BURST_LEN},  // #TVAR_afit_pBaseVals[270]

{0x0,  0x462A, BURST_LEN},  // #TVAR_afit_pBaseVals[271]

{0x0,  0x2846, BURST_LEN},  // #TVAR_afit_pBaseVals[272]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[273]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[274]

{0x0,  0x1E0C, BURST_LEN},  // #TVAR_afit_pBaseVals[275]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[276]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[277]

{0x0,  0x4B04, BURST_LEN},  // #TVAR_afit_pBaseVals[278]

{0x0,  0x0F40, BURST_LEN},  // #TVAR_afit_pBaseVals[279]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[280]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[281]

{0x0,  0x2303, BURST_LEN},  // #TVAR_afit_pBaseVals[282]

{0x0,  0x0123, BURST_LEN},  // #TVAR_afit_pBaseVals[283]

{0x0,  0x0201, BURST_LEN},  // #TVAR_afit_pBaseVals[284]

{0x0,  0x262A, BURST_LEN},  // #TVAR_afit_pBaseVals[285]

{0x0,  0x2C2C, BURST_LEN},  // #TVAR_afit_pBaseVals[286]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[287]

{0x0,  0x030A, BURST_LEN},  // #TVAR_afit_pBaseVals[288]

{0x0,  0x0714, BURST_LEN},  // #TVAR_afit_pBaseVals[289]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[290]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[291]

{0x0,  0x0432, BURST_LEN},  // #TVAR_afit_pBaseVals[292]

{0x0,  0x4050, BURST_LEN},  // #TVAR_afit_pBaseVals[293]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[294]

{0x0,  0x0440, BURST_LEN},  // #TVAR_afit_pBaseVals[295]

{0x0,  0x0302, BURST_LEN},  // #TVAR_afit_pBaseVals[296]

{0x0,  0x2323, BURST_LEN},  // #TVAR_afit_pBaseVals[297]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[298]

{0x0,  0x2A02, BURST_LEN},  // #TVAR_afit_pBaseVals[299]

{0x0,  0x2C26, BURST_LEN},  // #TVAR_afit_pBaseVals[300]

{0x0,  0x282C, BURST_LEN},  // #TVAR_afit_pBaseVals[301]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[302]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[303]

{0x0,  0x1E07, BURST_LEN},  // #TVAR_afit_pBaseVals[304]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[305]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[306]

{0x0,  0x5004, BURST_LEN},  // #TVAR_afit_pBaseVals[307]

{0x0,  0x0F40, BURST_LEN},  // #TVAR_afit_pBaseVals[308]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[309]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[310]

{0x0,  0x0003, BURST_LEN},  // #TVAR_afit_pBaseVals[311]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[312]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[313]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[314]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[315]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[316]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_afit_pBaseVals[317]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[318]

{0x0,  0x009C, BURST_LEN},  // #TVAR_afit_pBaseVals[319]

{0x0,  0x017C, BURST_LEN},  // #TVAR_afit_pBaseVals[320]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[321]

{0x0,  0x000C, BURST_LEN},  // #TVAR_afit_pBaseVals[322]

{0x0,  0x0010, BURST_LEN},  // #TVAR_afit_pBaseVals[323]

{0x0,  0x00C8, BURST_LEN},  // #TVAR_afit_pBaseVals[324]

{0x0,  0x0384, BURST_LEN},  // #TVAR_afit_pBaseVals[325]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[326]

{0x0,  0x0082, BURST_LEN},  // #TVAR_afit_pBaseVals[327]

{0x0,  0x0070, BURST_LEN},  // #TVAR_afit_pBaseVals[328]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[329]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[330]

{0x0,  0x01AA, BURST_LEN},  // #TVAR_afit_pBaseVals[331]

{0x0,  0x001E, BURST_LEN},  // #TVAR_afit_pBaseVals[332]

{0x0,  0x001E, BURST_LEN},  // #TVAR_afit_pBaseVals[333]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[334]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[335]

{0x0,  0x010E, BURST_LEN},  // #TVAR_afit_pBaseVals[336]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[337]

{0x0,  0x0032, BURST_LEN},  // #TVAR_afit_pBaseVals[338]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[339]

{0x0,  0x0032, BURST_LEN},  // #TVAR_afit_pBaseVals[340]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[341]

{0x0,  0x0A24, BURST_LEN},  // #TVAR_afit_pBaseVals[342]

{0x0,  0x1701, BURST_LEN},  // #TVAR_afit_pBaseVals[343]

{0x0,  0x0229, BURST_LEN},  // #TVAR_afit_pBaseVals[344]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[345]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[346]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[347]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[348]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[349]

{0x0,  0x043B, BURST_LEN},  // #TVAR_afit_pBaseVals[350]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[351]

{0x0,  0x0301, BURST_LEN},  // #TVAR_afit_pBaseVals[352]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[353]

{0x0,  0x051E, BURST_LEN},  // #TVAR_afit_pBaseVals[354]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[355]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[356]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[357]

{0x0,  0x0A3C, BURST_LEN},  // #TVAR_afit_pBaseVals[358]

{0x0,  0x0532, BURST_LEN},  // #TVAR_afit_pBaseVals[359]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[360]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[361]

{0x0,  0x1002, BURST_LEN},  // #TVAR_afit_pBaseVals[362]

{0x0,  0x001E, BURST_LEN},  // #TVAR_afit_pBaseVals[363]

{0x0,  0x0900, BURST_LEN},  // #TVAR_afit_pBaseVals[364]

{0x0,  0x0600, BURST_LEN},  // #TVAR_afit_pBaseVals[365]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[366]

{0x0,  0x0305, BURST_LEN},  // #TVAR_afit_pBaseVals[367]

{0x0,  0x4602, BURST_LEN},  // #TVAR_afit_pBaseVals[368]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[369]

{0x0,  0x0180, BURST_LEN},  // #TVAR_afit_pBaseVals[370]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[371]

{0x0,  0x2328, BURST_LEN},  // #TVAR_afit_pBaseVals[372]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[373]

{0x0,  0x2A02, BURST_LEN},  // #TVAR_afit_pBaseVals[374]

{0x0,  0x2828, BURST_LEN},  // #TVAR_afit_pBaseVals[375]

{0x0,  0x2828, BURST_LEN},  // #TVAR_afit_pBaseVals[376]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[377]

{0x0,  0x1903, BURST_LEN},  // #TVAR_afit_pBaseVals[378]

{0x0,  0x1E0F, BURST_LEN},  // #TVAR_afit_pBaseVals[379]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[380]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[381]

{0x0,  0x9604, BURST_LEN},  // #TVAR_afit_pBaseVals[382]

{0x0,  0x0F42, BURST_LEN},  // #TVAR_afit_pBaseVals[383]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[384]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[385]

{0x0,  0x2805, BURST_LEN},  // #TVAR_afit_pBaseVals[386]

{0x0,  0x0123, BURST_LEN},  // #TVAR_afit_pBaseVals[387]

{0x0,  0x0201, BURST_LEN},  // #TVAR_afit_pBaseVals[388]

{0x0,  0x2024, BURST_LEN},  // #TVAR_afit_pBaseVals[389]

{0x0,  0x1C1C, BURST_LEN},  // #TVAR_afit_pBaseVals[390]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[391]

{0x0,  0x030A, BURST_LEN},  // #TVAR_afit_pBaseVals[392]

{0x0,  0x0A0A, BURST_LEN},  // #TVAR_afit_pBaseVals[393]

{0x0,  0x0A2D, BURST_LEN},  // #TVAR_afit_pBaseVals[394]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[395]

{0x0,  0x0432, BURST_LEN},  // #TVAR_afit_pBaseVals[396]

{0x0,  0x4050, BURST_LEN},  // #TVAR_afit_pBaseVals[397]

{0x0,  0x0F0F, BURST_LEN},  // #TVAR_afit_pBaseVals[398]

{0x0,  0x0440, BURST_LEN},  // #TVAR_afit_pBaseVals[399]

{0x0,  0x0302, BURST_LEN},  // #TVAR_afit_pBaseVals[400]

{0x0,  0x2328, BURST_LEN},  // #TVAR_afit_pBaseVals[401]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[402]

{0x0,  0x3C02, BURST_LEN},  // #TVAR_afit_pBaseVals[403]

{0x0,  0x1C3C, BURST_LEN},  // #TVAR_afit_pBaseVals[404]

{0x0,  0x281C, BURST_LEN},  // #TVAR_afit_pBaseVals[405]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[406]

{0x0,  0x0A03, BURST_LEN},  // #TVAR_afit_pBaseVals[407]

{0x0,  0x2D0A, BURST_LEN},  // #TVAR_afit_pBaseVals[408]

{0x0,  0x070A, BURST_LEN},  // #TVAR_afit_pBaseVals[409]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[410]

{0x0,  0x5004, BURST_LEN},  // #TVAR_afit_pBaseVals[411]

{0x0,  0x0F40, BURST_LEN},  // #TVAR_afit_pBaseVals[412]

{0x0,  0x400F, BURST_LEN},  // #TVAR_afit_pBaseVals[413]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[414]

{0x0,  0x0003, BURST_LEN},  // #TVAR_afit_pBaseVals[415]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[416]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[417]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[418]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[419]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[420]

{0x0,  0x00C4, BURST_LEN},  // #TVAR_afit_pBaseVals[421]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[422]

{0x0,  0x009C, BURST_LEN},  // #TVAR_afit_pBaseVals[423]

{0x0,  0x017C, BURST_LEN},  // #TVAR_afit_pBaseVals[424]

{0x0,  0x03FF, BURST_LEN},  // #TVAR_afit_pBaseVals[425]

{0x0,  0x000C, BURST_LEN},  // #TVAR_afit_pBaseVals[426]

{0x0,  0x0010, BURST_LEN},  // #TVAR_afit_pBaseVals[427]

{0x0,  0x00C8, BURST_LEN},  // #TVAR_afit_pBaseVals[428]

{0x0,  0x0320, BURST_LEN},  // #TVAR_afit_pBaseVals[429]

{0x0,  0x0046, BURST_LEN},  // #TVAR_afit_pBaseVals[430]

{0x0,  0x015E, BURST_LEN},  // #TVAR_afit_pBaseVals[431]

{0x0,  0x0070, BURST_LEN},  // #TVAR_afit_pBaseVals[432]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[433]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[434]

{0x0,  0x01AA, BURST_LEN},  // #TVAR_afit_pBaseVals[435]

{0x0,  0x0014, BURST_LEN},  // #TVAR_afit_pBaseVals[436]

{0x0,  0x0014, BURST_LEN},  // #TVAR_afit_pBaseVals[437]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[438]

{0x0,  0x0008, BURST_LEN},  // #TVAR_afit_pBaseVals[439]

{0x0,  0x0140, BURST_LEN},  // #TVAR_afit_pBaseVals[440]

{0x0,  0x003C, BURST_LEN},  // #TVAR_afit_pBaseVals[441]

{0x0,  0x0032, BURST_LEN},  // #TVAR_afit_pBaseVals[442]

{0x0,  0x0023, BURST_LEN},  // #TVAR_afit_pBaseVals[443]

{0x0,  0x0023, BURST_LEN},  // #TVAR_afit_pBaseVals[444]

{0x0,  0x0032, BURST_LEN},  // #TVAR_afit_pBaseVals[445]

{0x0,  0x0A24, BURST_LEN},  // #TVAR_afit_pBaseVals[446]

{0x0,  0x1701, BURST_LEN},  // #TVAR_afit_pBaseVals[447]

{0x0,  0x0229, BURST_LEN},  // #TVAR_afit_pBaseVals[448]

{0x0,  0x1403, BURST_LEN},  // #TVAR_afit_pBaseVals[449]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[450]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[451]

{0x0,  0x0505, BURST_LEN},  // #TVAR_afit_pBaseVals[452]

{0x0,  0x00FF, BURST_LEN},  // #TVAR_afit_pBaseVals[453]

{0x0,  0x043B, BURST_LEN},  // #TVAR_afit_pBaseVals[454]

{0x0,  0x1414, BURST_LEN},  // #TVAR_afit_pBaseVals[455]

{0x0,  0x0301, BURST_LEN},  // #TVAR_afit_pBaseVals[456]

{0x0,  0xFF07, BURST_LEN},  // #TVAR_afit_pBaseVals[457]

{0x0,  0x051E, BURST_LEN},  // #TVAR_afit_pBaseVals[458]

{0x0,  0x0A1E, BURST_LEN},  // #TVAR_afit_pBaseVals[459]

{0x0,  0x0000, BURST_LEN},  // #TVAR_afit_pBaseVals[460]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[461]

{0x0,  0x143C, BURST_LEN},  // #TVAR_afit_pBaseVals[462]

{0x0,  0x0532, BURST_LEN},  // #TVAR_afit_pBaseVals[463]

{0x0,  0x0002, BURST_LEN},  // #TVAR_afit_pBaseVals[464]

{0x0,  0x0096, BURST_LEN},  // #TVAR_afit_pBaseVals[465]

{0x0,  0x1002, BURST_LEN},  // #TVAR_afit_pBaseVals[466]

{0x0,  0x001E, BURST_LEN},  // #TVAR_afit_pBaseVals[467]

{0x0,  0x0900, BURST_LEN},  // #TVAR_afit_pBaseVals[468]

{0x0,  0x0600, BURST_LEN},  // #TVAR_afit_pBaseVals[469]

{0x0,  0x0504, BURST_LEN},  // #TVAR_afit_pBaseVals[470]

{0x0,  0x0305, BURST_LEN},  // #TVAR_afit_pBaseVals[471]

{0x0,  0x5A02, BURST_LEN},  // #TVAR_afit_pBaseVals[472]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[473]

{0x0,  0x0180, BURST_LEN},  // #TVAR_afit_pBaseVals[474]

{0x0,  0x0080, BURST_LEN},  // #TVAR_afit_pBaseVals[475]

{0x0,  0x5050, BURST_LEN},  // #TVAR_afit_pBaseVals[476]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[477]

{0x0,  0x1C02, BURST_LEN},  // #TVAR_afit_pBaseVals[478]

{0x0,  0x141C, BURST_LEN},  // #TVAR_afit_pBaseVals[479]

{0x0,  0x2814, BURST_LEN},  // #TVAR_afit_pBaseVals[480]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[481]

{0x0,  0x1E03, BURST_LEN},  // #TVAR_afit_pBaseVals[482]

{0x0,  0x1E0F, BURST_LEN},  // #TVAR_afit_pBaseVals[483]

{0x0,  0x0508, BURST_LEN},  // #TVAR_afit_pBaseVals[484]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[485]

{0x0,  0xAA04, BURST_LEN},  // #TVAR_afit_pBaseVals[486]

{0x0,  0x1452, BURST_LEN},  // #TVAR_afit_pBaseVals[487]

{0x0,  0x4015, BURST_LEN},  // #TVAR_afit_pBaseVals[488]

{0x0,  0x0604, BURST_LEN},  // #TVAR_afit_pBaseVals[489]

{0x0,  0x5006, BURST_LEN},  // #TVAR_afit_pBaseVals[490]

{0x0,  0x0150, BURST_LEN},  // #TVAR_afit_pBaseVals[491]

{0x0,  0x0201, BURST_LEN},  // #TVAR_afit_pBaseVals[492]

{0x0,  0x1E1E, BURST_LEN},  // #TVAR_afit_pBaseVals[493]

{0x0,  0x1212, BURST_LEN},  // #TVAR_afit_pBaseVals[494]

{0x0,  0x0028, BURST_LEN},  // #TVAR_afit_pBaseVals[495]

{0x0,  0x030A, BURST_LEN},  // #TVAR_afit_pBaseVals[496]

{0x0,  0x0A10, BURST_LEN},  // #TVAR_afit_pBaseVals[497]

{0x0,  0x0819, BURST_LEN},  // #TVAR_afit_pBaseVals[498]

{0x0,  0xFF05, BURST_LEN},  // #TVAR_afit_pBaseVals[499]

{0x0,  0x0432, BURST_LEN},  // #TVAR_afit_pBaseVals[500]

{0x0,  0x4052, BURST_LEN},  // #TVAR_afit_pBaseVals[501]

{0x0,  0x1514, BURST_LEN},  // #TVAR_afit_pBaseVals[502]

{0x0,  0x0440, BURST_LEN},  // #TVAR_afit_pBaseVals[503]

{0x0,  0x0302, BURST_LEN},  // #TVAR_afit_pBaseVals[504]

{0x0,  0x5050, BURST_LEN},  // #TVAR_afit_pBaseVals[505]

{0x0,  0x0101, BURST_LEN},  // #TVAR_afit_pBaseVals[506]

{0x0,  0x1E02, BURST_LEN},  // #TVAR_afit_pBaseVals[507]

{0x0,  0x121E, BURST_LEN},  // #TVAR_afit_pBaseVals[508]

{0x0,  0x2812, BURST_LEN},  // #TVAR_afit_pBaseVals[509]

{0x0,  0x0A00, BURST_LEN},  // #TVAR_afit_pBaseVals[510]

{0x0,  0x1003, BURST_LEN},  // #TVAR_afit_pBaseVals[511]

{0x0,  0x190A, BURST_LEN},  // #TVAR_afit_pBaseVals[512]

{0x0,  0x0508, BURST_LEN},  // #TVAR_afit_pBaseVals[513]

{0x0,  0x32FF, BURST_LEN},  // #TVAR_afit_pBaseVals[514]

{0x0,  0x5204, BURST_LEN},  // #TVAR_afit_pBaseVals[515]

{0x0,  0x1440, BURST_LEN},  // #TVAR_afit_pBaseVals[516]

{0x0,  0x4015, BURST_LEN},  // #TVAR_afit_pBaseVals[517]

{0x0,  0x0204, BURST_LEN},  // #TVAR_afit_pBaseVals[518]

{0xFFFF,  0x0003, BURST_LEN},  // #TVAR_afit_pBaseVals[519]



// #AFIT table (Constants)

{0x0F12,  0x7F7A, WORD_LEN},  // #afit_pConstBaseVals[0]

{0x0F12,  0x7FBD, WORD_LEN},  // #afit_pConstBaseVals[1]

{0x0F12,  0xBEFC, WORD_LEN},  // #afit_pConstBaseVals[2]

{0x0F12,  0xF7BC, WORD_LEN},  // #afit_pConstBaseVals[3]

{0x0F12,  0x7E06, WORD_LEN},  // #afit_pConstBaseVals[4]

{0x0F12,  0x0053, WORD_LEN},  // #afit_pConstBaseVals[5]



// #Update Changed Registers

{0x002A, 0x0664, WORD_LEN},

{0x0F12,  0x013E, WORD_LEN},  // #seti_uContrastCenter



//================================================================================================

// #SET PLL

//================================================================================================

// #How to set

// #1. MCLK

//hex(CLK you want) * 1000)

// // #2. System CLK

//hex((CLK you want) * 1000 / 4)

// // #3. PCLK

//hex((CLK you want) * 1000 / 4)

//================================================================================================

// #Set input CLK // #24MHz

{0x002A,  0x01CC, WORD_LEN},
{0x0F12,  0x5DC0, WORD_LEN},  //#REG_TC_IPRM_InClockLSBs

{0x0F12,  0x0000, WORD_LEN},  //#REG_TC_IPRM_InClockMSBs

{0x002A,  0x01EE, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  //#REG_TC_IPRM_UseNPviClocks, BURST_LEN},  // #Number of PLL setting

// #Set system CLK  // #60MHz (0x3A98)

{0x002A,  0x01F6, WORD_LEN},
{0x0F12,  0x3A98, WORD_LEN},  // #REG_TC_IPRM_OpClk4KHz_0

// #Set pixel CLK, // #52MHz (32C8)

{0x0F12,  0x32B8, WORD_LEN},  // #REG_TC_IPRM_MinOutRate4KHz_0

{0x0F12,  0x32D8, WORD_LEN},  // #REG_TC_IPRM_MaxOutRate4KHz_0

// #Update PLL

{0x002A,  0x0208, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_IPRM_InitParamsUpdated



//================================================================================================

// #SET PREVIEW CONFIGURATION_0

// ## Foramt : YUV422

// ## Size: VGA

// ## FPS : 22~10fps

//================================================================================================

{0x002A,  0x026C, WORD_LEN},
//mhlee
{0xFFFE,  0x0280, BURST_LEN},  //#REG_0TC_PCFG_usWidth, BURST_LEN},  //640 

{0x0,  0x01E0, BURST_LEN},  //#REG_0TC_PCFG_usHeight, BURST_LEN},  //480

{0x0,  0x0005, BURST_LEN},  // #REG_0TC_PCFG_Format

{0x0,  0x32D8, BURST_LEN},  // #REG_0TC_PCFG_usMaxOut4KHzRate

{0x0,  0x32B8, BURST_LEN},  // #REG_0TC_PCFG_usMinOut4KHzRate

{0x0,  0x0100, BURST_LEN},  // #REG_0TC_PCFG_OutClkPerPix88

{0x0,  0x0800, BURST_LEN},  // #REG_0TC_PCFG_uMaxBpp88

{0x0,  0x0052, BURST_LEN},  // #REG_0TC_PCFG_PVIMask, BURST_LEN},  //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800, BURST_LEN},  //reg

{0x0,  0x4000, BURST_LEN},  // #REG_0TC_PCFG_OIFMask

{0x0,  0x01E0, BURST_LEN},  // #REG_0TC_PCFG_usJpegPacketSize

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_usJpegTotalPackets

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_uClockInd

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_usFrTimeType

{0x0,  0x0001, BURST_LEN},  // #REG_0TC_PCFG_FrRateQualityType

{0x0,  0x03E8, BURST_LEN},  // #REG_0TC_PCFG_usMaxFrTimeMsecMult10, BURST_LEN},  //10fps
//{0x0,  0x014D, BURST_LEN},  // #REG_0TC_PCFG_usMaxFrTimeMsecMult10, BURST_LEN},  //fixed 30fps

{0x0,  0x014D, BURST_LEN},  // #REG_0TC_PCFG_usMinFrTimeMsecMult10, BURST_LEN},  //30fps

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_bSmearOutput

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_sSaturation

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_sSharpBlur

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_sColorTemp

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_uDeviceGammaIndex

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_uPrevMirror

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_uCaptureMirror

{0xFFFF,  0x0000, BURST_LEN},  // #REG_0TC_PCFG_uRotation



//================================================================================================

// #APPLY PREVIEW CONFIGURATION & RUN PREVIEW

//================================================================================================

{0x002A,  0x023C, WORD_LEN},
{0x0F12,  0x0000, WORD_LEN},  // #REG_TC_GP_ActivePrevConfig, BURST_LEN},  // #Select preview configuration_0

{0x002A,  0x0240, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_GP_PrevOpenAfterChange

{0x002A,  0x0230, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_GP_NewConfigSync, BURST_LEN},  // #Update preview configuration

{0x002A,  0x023E, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_GP_PrevConfigChanged

{0x002A,  0x0220, WORD_LEN},
{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_GP_EnablePreview, BURST_LEN},  // #Start preview

{0x0F12,  0x0001, WORD_LEN},  // #REG_TC_GP_EnablePreviewChanged



//================================================================================================

// #SET CAPTURE CONFIGURATION_0

// ## Foramt :JPEG

// ## Size: QXGA

// ## FPS : 7.5fps

//================================================================================================

{0x002A,  0x035C, WORD_LEN}, 


{0xFFFE,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_uCaptureModeJpEG

{0x0,  0x0800, BURST_LEN},  // #REG_0TC_CCFG_usWidth 

{0x0,  0x0600, BURST_LEN},  // #REG_0TC_CCFG_usHeight

{0x0,  0x0005, BURST_LEN},  // #REG_0TC_CCFG_Format, BURST_LEN},  //5:YUV9:JPEG 

{0x0,  0x32D8, BURST_LEN},  // #REG_0TC_CCFG_usMaxOut4KHzRate

{0x0,  0x32B8, BURST_LEN},  // #REG_0TC_CCFG_usMinOut4KHzRate

{0x0,  0x0100, BURST_LEN},  // #REG_0TC_CCFG_OutClkPerPix88

{0x0,  0x0800, BURST_LEN},  // #REG_0TC_CCFG_uMaxBpp88 

{0x0,  0x0052, BURST_LEN},  // #REG_0TC_CCFG_PVIMask 

{0x0,  0x0050, BURST_LEN},  // #REG_0TC_CCFG_OIFMask 

{0x0,  0x03C0, BURST_LEN},  // #REG_0TC_CCFG_usJpegPacketSize

{0x0,  0x08fc, BURST_LEN},  // #REG_0TC_CCFG_usJpegTotalPackets

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_uClockInd 

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_usFrTimeType

{0x0,  0x0002, BURST_LEN},  // #REG_0TC_CCFG_FrRateQualityType 

{0x0,  0x0535, BURST_LEN},  // #REG_0TC_CCFG_usMaxFrTimeMsecMult10, BURST_LEN},  //7.5fps

{0x0,  0x0535, BURST_LEN},  // #REG_0TC_CCFG_usMinFrTimeMsecMult10, BURST_LEN},  //7.5fps 

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_bSmearOutput

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_sSaturation 

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_sSharpBlur

{0x0,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_sColorTemp

{0xFFFF,  0x0000, BURST_LEN},  // #REG_0TC_CCFG_uDeviceGammaIndex 





//================================================================================================

// #SET JPEG & SPOOF

//================================================================================================

{0x002A,  0x0454, WORD_LEN},
{0x0F12,  0x0055, WORD_LEN},  // #REG_TC_BRC_usCaptureQuality, BURST_LEN},  // #JPEG BRC (BitRateControl) value, BURST_LEN},  // #85



//================================================================================================

// ## SET THUMBNAIL

// ## Foramt : RGB565

// ## Size: VGA

//================================================================================================

{0x0F12,  0x0000, WORD_LEN},  // #thumbnail enable

{0x0F12,  0x0140, WORD_LEN},  // #Width, BURST_LEN},  //320

{0x0F12,  0x00F0, WORD_LEN},  // #Height, BURST_LEN},  //240

{0x0F12,  0x0000, WORD_LEN},  // #Thumbnail format : 0-RGB565 1-RGB888 2-Full-YUV (0-255)



{0x002A,  0x021A, WORD_LEN},
{0x0F12,  0x0000, WORD_LEN},  // #disable, BURST_LEN},  //#REG_TC_bUseMBR

 

// #Fill RAM with alternative op-codes

{0x0028, 0x7000, WORD_LEN},  // #start add MSW

{0x002A,  0x2CE8, WORD_LEN},  // #start add LSW

{0x0F12,  0x0007, WORD_LEN},  // #Modify LSB to control AWBB_YThreshLow

{0x0F12,  0x00e2 , WORD_LEN},
{0x0F12,  0x0005, WORD_LEN},  // #Modify LSB to control AWBB_YThreshLowBrLow

{0x0F12,  0x00e2, WORD_LEN},


{0x0028, 0xD000 , WORD_LEN},
{0x002A,  0x1000 , WORD_LEN},
{0x0F12,  0x0001 , WORD_LEN},


{0x0028, 0x7000, WORD_LEN},




  
};


static const struct lgcam_rear_sensor_i2c_reg_conf const preview_mode_reg_settings_array[] = {
{0x0028,0x7000,WORD_LEN},
{0x002A,0x023C,WORD_LEN},
{0x0F12,0x0000,WORD_LEN},	//#REG_TC_GP_ActivePrevConfig     //preview config0
{0x002A,0x0240,WORD_LEN},
{0x0F12,0x0001,WORD_LEN},	//#REG_TC_GP_PrevOpenAfterChange  //config change
{0x002A,0x0230,WORD_LEN},
{0x0F12,0x0001,WORD_LEN},	//#REG_TC_GP_NewConfigSync
{0x002A,0x023E,WORD_LEN},
{0x0F12,0x0001,WORD_LEN},	//#REG_TC_GP_PrevConfigChanged
{0x002A,0x0220,WORD_LEN},
{0x0F12,0x0001,WORD_LEN},	//#REG_TC_GP_EnablePreview
{0x0F12,0x0001,WORD_LEN},	//#REG_TC_GP_EnablePreviewChfanged
};

static const struct lgcam_rear_sensor_i2c_reg_conf const snapshot_mode_reg_settings_array[] = {
{0x0028, 0x7000, WORD_LEN},
{0x002a, 0x0244, WORD_LEN},	
{0x0f12, 0x0000, WORD_LEN}, //#REG_TC_GP_ActiveCapConfig    //capture config0:moto 1:TN 
{0x002a, 0x0230, WORD_LEN},
{0x0f12, 0x0001, WORD_LEN}, //#REG_TC_GP_NewConfigSync  //config change 
{0x002a, 0x0246, WORD_LEN},
{0x0f12, 0x0001, WORD_LEN}, //#REG_TC_GP_CapConfigChanged            
{0x002a, 0x0224, WORD_LEN},
{0x0f12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnableCapture 
{0x0f12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnableCaptureChanged
};
static const struct lgcam_rear_sensor_i2c_reg_conf const preview_mode_ap001_16bit_settings_array[] = {
};

static const struct lgcam_rear_sensor_i2c_reg_conf const preview_mode_ap003_16bit_settings_array[] = {
};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_normal_reg_settings_array[] = {
	//Auto(all off) 
	{0x0028, 0x7000, WORD_LEN}, 
	{0x002A, 0x246E, WORD_LEN}, //sunset_return
	{0x0F12, 0x0001, WORD_LEN}, 
		
	{0x002A, 0x020C, WORD_LEN}, //brightness & saturation
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x002A, 0x0210, WORD_LEN}, 
	{0x0F12, 0x0000, WORD_LEN}, 
		
	{0x002A, 0x0842, WORD_LEN}, //AFIT
	{0x0F12, 0x8250, WORD_LEN}, 
	{0x0F12, 0x2882, WORD_LEN}, 
	{0x002A, 0x0912, WORD_LEN}, 
	{0x0F12, 0x6e3c, WORD_LEN}, 
	{0x0F12, 0x286e, WORD_LEN}, 
	{0x002A, 0x09E2, WORD_LEN}, 
	{0x0F12, 0x462a, WORD_LEN}, 
	{0x0F12, 0x2846, WORD_LEN}, 
	{0x002A, 0x0AB2, WORD_LEN}, 
	{0x0F12, 0x2828, WORD_LEN}, 
	{0x0F12, 0x2822, WORD_LEN}, 
	{0x002A, 0x0B82, WORD_LEN}, 
	{0x0F12, 0x141C, WORD_LEN}, 
	{0x0F12, 0x2814, WORD_LEN}, 
		
	{0x002A, 0x0530, WORD_LEN}, // Frame rate setting // Set preview exposure time	
	{0x0F12, 0x5DC0, WORD_LEN}, // #lt_uMaxExp1 60ms	
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0x6590, WORD_LEN}, // #lt_uMaxExp2 65ms	
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x002A, 0x167C, WORD_LEN}, 
	{0x0F12, 0x8CA0, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0xABE0, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x0538, WORD_LEN}, // Set capture exposure time
	{0x0F12, 0x5DC0, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0x6590, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x1684, WORD_LEN},
	{0x0F12, 0x8CA0, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xABE0, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x0540, WORD_LEN}, // Set gain
	{0x0F12, 0x0150, WORD_LEN}, // #lt_uMaxAnGain1
	{0x0F12, 0x0280, WORD_LEN}, // #lt_uMaxAnGain2
	{0x002A, 0x168C, WORD_LEN},
	{0x0F12, 0x0350, WORD_LEN}, // #evt1_lt_uMaxAnGain3
	{0x0F12, 0x0800, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
	{0x002A, 0x0544, WORD_LEN},
	{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
	{0x0F12, 0x0A00, WORD_LEN}, // #lt_uMaxTotGain
		
	{0x002A, 0x0288, WORD_LEN}, 	
	{0x0F12, 0x03E8, WORD_LEN},  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps	
//	{0x0F12, 0x014D, WORD_LEN},  //#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //mhlee 0112 30fps	
	{0x002A, 0x037a, WORD_LEN}, 	
	{0x0F12, 0x0535, WORD_LEN},  //#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7.5fps 
		
	{0x002A, 0x0F7E, WORD_LEN}, // AE weight //backlight_return
	
	{0xFFFE, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_0_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_1_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_2_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_3_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_4_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_5_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_6_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_7_
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_8_
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_9_
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_10
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_11
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_12
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_13
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_14
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_15
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_16
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_17
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_18
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_19
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_20
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_21
	{0x0, 0x0303, BURST_LEN}, // #ae_WeightTbl_16_22
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_23
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_24
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_25
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_26
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_27
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_28
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_29
	{0x0, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_30
	{0xFFFF, 0x0101, BURST_LEN}, // #ae_WeightTbl_16_31
	
	{0x002A, 0x0208, WORD_LEN}, 
	{0x0F12, 0x0001, WORD_LEN}, 
		
	{0x0028, 0x7000, WORD_LEN}, 
	{0x002A, 0x023C, WORD_LEN}, 	
	{0x0F12, 0x0000, WORD_LEN}, //#REG_TC_GP_ActivePrevConfig	  //preview config0  
	{0x002A, 0x0240, WORD_LEN}, 													 
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevOpenAfterChange  //config change	 
	{0x002A, 0x0230, WORD_LEN}, 													 
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_NewConfigSync							 
	{0x002A, 0x023e, WORD_LEN}, 													 
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevConfigChanged						 
	{0x002A, 0x0220, WORD_LEN}, 													 
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreview							 
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreviewChfanged	


};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_portrait_reg_settings_array[] = {

//Portrait	
{0xFCFC, 0xD000, WORD_LEN},	
{0x0028, 0x7000, WORD_LEN},	
{0x002A, 0x020C, WORD_LEN},	
{0x0F12, 0x0000, WORD_LEN},	
{0x002A, 0x0210, WORD_LEN},	
{0x0F12, 0x0000, WORD_LEN},	
{0x0F12, 0xFFCC, WORD_LEN},	


};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_landscape_reg_settings_array[] = {

{0xFCFC, 0xD000, WORD_LEN},
{0x0028, 0x7000, WORD_LEN},
{0x002A, 0x020C, WORD_LEN},
{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x0210, WORD_LEN},
{0x0F12, 0x001E, WORD_LEN},
{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x0842, WORD_LEN},
{0x0F12, 0x6444, WORD_LEN},
{0x0F12, 0x465A, WORD_LEN},
{0x002A, 0x0912, WORD_LEN},
{0x0F12, 0x4B3A, WORD_LEN},
{0x0F12, 0x463F, WORD_LEN},
{0x002A, 0x09E2, WORD_LEN},
{0x0F12, 0x1A2D, WORD_LEN},
{0x0F12, 0x4628, WORD_LEN},
{0x002A, 0x0AB2, WORD_LEN},
{0x0F12, 0x1328, WORD_LEN},
{0x0F12, 0x3213, WORD_LEN},
{0x002A, 0x0B82, WORD_LEN},
{0x0F12, 0x0819, WORD_LEN},
{0x0F12, 0x3204, WORD_LEN},

};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_sport_reg_settings_array[] = {
	

//Sports	
{0x0028, 0x7000	, WORD_LEN},
{0x002A, 0x0530	, WORD_LEN},
{0x0F12, 0x0FA0	, WORD_LEN},
{0x002A, 0x0534	, WORD_LEN},
{0x0F12, 0x1450	, WORD_LEN},
{0x002A, 0x167C	, WORD_LEN},
{0x0F12, 0x2710	, WORD_LEN},
{0x002A, 0x1680	, WORD_LEN},
{0x0F12, 0x4e20	, WORD_LEN},
{0x002A, 0x0538	, WORD_LEN},
{0x0F12, 0x0FA0	, WORD_LEN},
{0x002A, 0x053C	, WORD_LEN},
{0x0F12, 0x1450	, WORD_LEN},
{0x002A, 0x1684	, WORD_LEN},
{0x0F12, 0x2710	, WORD_LEN},
{0x002A, 0x1688	, WORD_LEN},
{0x0F12, 0x4E20	, WORD_LEN},
{0x002A, 0x0540	, WORD_LEN},
{0x0F12, 0x0300	, WORD_LEN},
{0x0F12, 0x0400	, WORD_LEN},
{0x002A, 0x168C	, WORD_LEN},
{0x0F12, 0x0480	, WORD_LEN},
{0x0F12, 0x0800	, WORD_LEN},


};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_sunset_reg_settings_array[] = {
//Sunset	
{0x0028, 0x7000, WORD_LEN},	
{0x002A, 0x246E, WORD_LEN},	
{0x0F12, 0x0000, WORD_LEN},	
{0x002A, 0x04A0, WORD_LEN},	
{0x0F12,0x05E0, WORD_LEN},	
{0x0F12,0x0001, WORD_LEN},	
{0x0F12,0x0400, WORD_LEN},	
{0x0F12,0x0001, WORD_LEN},	
{0x0F12,0x0520, WORD_LEN},	
{0x0F12,0x0001, WORD_LEN}                                    
  
};

static const struct lgcam_rear_sensor_i2c_reg_conf const scene_mode_night_reg_settings_array[] = {

//Night	
{0x0028, 0x7000, WORD_LEN},	
{0x002A, 0x1680, WORD_LEN},	
{0x0F12, 0x86A0, WORD_LEN},	//#evt1_lt_uMaxExp4 //200ms                            
{0x0F12, 0x0001, WORD_LEN},	                                                       
{0x002A, 0x1688, WORD_LEN},	                                                        
{0x0F12, 0x86A0, WORD_LEN},	//#evt1_lt_uCapMaxExp //200ms                            
{0x0F12, 0x0001, WORD_LEN},	                           
{0x002A, 0x168E, WORD_LEN},	                                     
{0x0F12, 0x0780, WORD_LEN},	//#evt1_lt_uMaxAnGain4 X7.5                             
{0x002A, 0x0546, WORD_LEN},	
{0x0F12, 0x1000, WORD_LEN},	//#lt_uMaxTotGain  16X
	
{0x002A, 0x0288, WORD_LEN},	
{0x0F12, 0x07D0, WORD_LEN},	//#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //5fps
{0x002A, 0x037a, WORD_LEN},	
{0x0F12, 0x07D0, WORD_LEN},	//#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //5fps
	
{0x002A, 0x023C, WORD_LEN},	
{0x0F12, 0x0000, WORD_LEN},	//#REG_TC_GP_ActivePrevConfig 
{0x002A, 0x0240, WORD_LEN},	
{0x0F12, 0x0001, WORD_LEN},	//#REG_TC_GP_PrevOpenAfterChange
{0x002A, 0x0230, WORD_LEN},	
{0x0F12, 0x0001, WORD_LEN},	//#REG_TC_GP_NewConfigSync 
{0x002A, 0x023e, WORD_LEN},	 
{0x0F12, 0x0001, WORD_LEN},	//#REG_TC_GP_PrevConfigChanged



};

static const struct lgcam_rear_sensor_i2c_reg_conf const af_reg_settings_array[] = {
	{0x0028, 0x7000, WORD_LEN},
{0x002A, 0x1074, WORD_LEN},
//mhlee

{0xFFFE, 0x0010, BURST_LEN}, //0010 //#af_pos_usTableLastInd// 16 Steps 091222
{0x0, 0x0028, BURST_LEN}, //003C //#af_pos_usTable_0_// af_pos_usTable
{0x0, 0x002B, BURST_LEN}, //003F //#af_pos_usTable_1_
{0x0, 0x002E, BURST_LEN}, //0042 //#af_pos_usTable_2_
{0x0, 0x0031, BURST_LEN}, //0045 //#af_pos_usTable_3_
{0x0, 0x0034, BURST_LEN}, //0048 //#af_pos_usTable_4_
{0x0, 0x0037, BURST_LEN}, //004B //#af_pos_usTable_5_
{0x0, 0x003A, BURST_LEN}, //004E //#af_pos_usTable_6_
{0x0, 0x003D, BURST_LEN}, //0051 //#af_pos_usTable_7_
{0x0, 0x0040, BURST_LEN}, //0054 //#af_pos_usTable_8_
{0x0, 0x0043, BURST_LEN}, //0057 //#af_pos_usTable_9_
{0x0, 0x0046, BURST_LEN}, //005A //#af_pos_usTable_10_
{0x0, 0x004A, BURST_LEN}, //005E //#af_pos_usTable_11_
{0x0, 0x004D, BURST_LEN}, //0061 //#af_pos_usTable_12_
{0x0, 0x0050, BURST_LEN}, //0064 //#af_pos_usTable_13_
{0x0, 0x0054, BURST_LEN}, //0068 //#af_pos_usTable_14_
{0x0, 0x0058, BURST_LEN}, //006C //#af_pos_usTable_15_
{0xFFFF, 0x0064, BURST_LEN}, //0078 //#af_pos_usTable_16_





};

static const struct lgcam_rear_sensor_i2c_reg_conf const af_nomal_mode_reg_settings_array[] = {
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x0252, WORD_LEN},
	{0x0F12, 0x0003, WORD_LEN},

};

static const struct lgcam_rear_sensor_i2c_reg_conf const af_macro_mode_reg_settings_array[] = {
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x1074, WORD_LEN},
	{0xFFFE, 0x0010, BURST_LEN},	// #af_pos_usTableLastInd// 17 Steps
	{0x0, 0x005C, BURST_LEN},	// #af_pos_usTable_0_// af_pos_usTable 
	{0x0, 0x0060, BURST_LEN},	// #af_pos_usTable_1_
	{0x0, 0x0064, BURST_LEN},	// #af_pos_usTable_2_
	{0x0, 0x0068, BURST_LEN},	// #af_pos_usTable_3_
	{0x0, 0x006C, BURST_LEN},	// #af_pos_usTable_4_
	{0x0, 0x0070, BURST_LEN},	// #af_pos_usTable_5_
	{0x0, 0x0074, BURST_LEN},	// #af_pos_usTable_6_
	{0x0, 0x0078, BURST_LEN},	// #af_pos_usTable_7_
	{0x0, 0x007C, BURST_LEN},	// #af_pos_usTable_8_
	{0x0, 0x0080, BURST_LEN},	// #af_pos_usTable_9_
	{0x0, 0x0084, BURST_LEN},	// #af_pos_usTable_10_
	{0x0, 0x0088, BURST_LEN},	// #af_pos_usTable_11_
	{0x0, 0x008C, BURST_LEN},	// #af_pos_usTable_12_
	{0x0, 0x0090, BURST_LEN},	// #af_pos_usTable_13_
	{0x0, 0x0094, BURST_LEN},	// #af_pos_usTable_14_
	{0x0, 0x0098, BURST_LEN},	// #af_pos_usTable_15_
	{0xFFFF, 0x009C, BURST_LEN},	// #af_pos_usTable_16_
	{0x002A, 0x1066, WORD_LEN},
	{0x0F12, 0x1000, WORD_LEN},	// #af_pos_usMacroStartEnd 
	{0x002A, 0x0254, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN},	// #REG_TC_AF_AfCmdParam
	{0x002A, 0x0252, WORD_LEN},
	{0x0F12, 0x0005, WORD_LEN},	
};
static const struct lgcam_rear_sensor_i2c_reg_conf const manual_focus_mode_reg_settings_array[] = {
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x0254, WORD_LEN},

};

static const struct lgcam_rear_sensor_i2c_reg_conf const iso_mode_auto_reg_settings_array[] = {
	// CAMTUNING_ISO_AUTO
	{0xfcfc, 0xd000, WORD_LEN},  
	{0x0028, 0x7000, WORD_LEN}, 
			
	{0x002A, 0x12B8, WORD_LEN},   
	{0x0F12, 0x1000, WORD_LEN},  
	
	
	// #Set preview exposure time
	{0x002A, 0x0530, WORD_LEN},
	{0x0F12, 0x5DC0, WORD_LEN}, // #lt_uMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0x6590, WORD_LEN}, // #lt_uMaxExp2 65ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x167C, WORD_LEN},
	{0x0F12, 0x8CA0, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0xABE0, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN}, 
	
	// #Set capture exposure time
	{0x002A, 0x0538, WORD_LEN},
	{0x0F12, 0x5DC0, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0x6590, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x1684, WORD_LEN},
	{0x0F12, 0x8CA0, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xABE0, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	
	
	// #Set gain
	{0x002A, 0x0540, WORD_LEN},
	{0x0F12, 0x0150, WORD_LEN}, // #lt_uMaxAnGain1
	{0x0F12, 0x0280, WORD_LEN}, // #lt_uMaxAnGain2
	{0x002A, 0x168C, WORD_LEN},
	{0x0F12, 0x0350, WORD_LEN}, // #evt1_lt_uMaxAnGain3
	{0x0F12, 0x0800, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
	 
	
	{0x002A, 0x0544, WORD_LEN},   
	{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
	{0x0F12, 0x8000, WORD_LEN}, // #lt_uMaxTotGain
	
	{0x002A, 0x04B4, WORD_LEN},   
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoType
	{0x0F12, 0x0064, WORD_LEN}, // #REG_SF_USER_IsoVal
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoChanged

};

static const struct lgcam_rear_sensor_i2c_reg_conf const iso_mode_100_reg_settings_array[] = {
// CAMTUNING_ISO_AUTO
{0xfcfc, 0xd000, WORD_LEN}, 
{0x0028, 0x7000, WORD_LEN}, 
        
{0x002A, 0x12B8, WORD_LEN},   
{0x0F12, 0x1800, WORD_LEN},  


// #Set preview exposure time
{0x002A, 0x0530, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp1 60ms 
{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp2 65ms
{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x167C, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
{0x0F12, 0x0000, WORD_LEN}, 
{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
{0x0F12, 0x0000, WORD_LEN}, 

// #Set capture exposure time
{0x002A, 0x0538, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
{0x0F12, 0x0000, WORD_LEN},
{0x002A, 0x1684, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
{0x0F12, 0x0000, WORD_LEN},
{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
{0x0F12, 0x0000, WORD_LEN},


// #Set gain
{0x002A, 0x0540, WORD_LEN},
{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain1
{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain2
{0x002A, 0x168C, WORD_LEN},
{0x0F12, 0x0250, WORD_LEN}, // #evt1_lt_uMaxAnGain3
{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
 

{0x002A, 0x0544, WORD_LEN},   
{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
{0x0F12, 0x8000, WORD_LEN}, // #lt_uMaxTotGain

{0x002A, 0x04B4, WORD_LEN},   
{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoType
{0x0F12, 0x0096, WORD_LEN}, // #REG_SF_USER_IsoVal
{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoChanged	
};

static const struct lgcam_rear_sensor_i2c_reg_conf const iso_mode_200_reg_settings_array[] = {
	// CAMTUNING_ISO_AUTO
	{0xfcfc, 0xd000, WORD_LEN}, 
	{0x0028, 0x7000, WORD_LEN}, 
			
	{0x002A, 0x12B8, WORD_LEN},   
	{0x0F12, 0x2000, WORD_LEN},  
	
	
	// #Set preview exposure time
	{0x002A, 0x0530, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp2 65ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x167C, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN}, 
	
	// #Set capture exposure time
	{0x002A, 0x0538, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x1684, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	
	
	// #Set gain
	{0x002A, 0x0540, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain1
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain2
	{0x002A, 0x168C, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain3
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
	 
	
	{0x002A, 0x0544, WORD_LEN},   
	{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
	{0x0F12, 0x8000, WORD_LEN}, // #lt_uMaxTotGain
	
	{0x002A, 0x04B4, WORD_LEN},   
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoType
	{0x0F12, 0x00C8, WORD_LEN}, // #REG_SF_USER_IsoVal
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoChanged

};

static const struct lgcam_rear_sensor_i2c_reg_conf const iso_mode_400_reg_settings_array[] = {
	// CAMTUNING_ISO_AUTO
	{0xfcfc, 0xd000, WORD_LEN}, 
	{0x0028, 0x7000, WORD_LEN}, 
			
	{0x002A, 0x12B8, WORD_LEN},  
	{0x0F12, 0x3000, WORD_LEN},  
	
	
	// #Set preview exposure time
	{0x002A, 0x0530, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp2 65ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x167C, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN}, 
	
	// #Set capture exposure time
	{0x002A, 0x0538, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x1684, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	
	
	// #Set gain
	{0x002A, 0x0540, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain1
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain2
	{0x002A, 0x168C, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain3
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
	 
	
	{0x002A, 0x0544, WORD_LEN},   
	{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
	{0x0F12, 0x8000, WORD_LEN}, // #lt_uMaxTotGain
	
	{0x002A, 0x04B4, WORD_LEN},   
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoType
	{0x0F12, 0x012C, WORD_LEN},// #REG_SF_USER_IsoVal
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoChanged

};

static const struct lgcam_rear_sensor_i2c_reg_conf const iso_mode_800_reg_settings_array[] = {
	// CAMTUNING_ISO_AUTO
	{0xfcfc, 0xd000, WORD_LEN}, 
	{0x0028, 0x7000, WORD_LEN}, 
			
	{0x002A, 0x12B8, WORD_LEN},   
	{0x0F12, 0x6000, WORD_LEN},  
	
	
	// #Set preview exposure time
	{0x002A, 0x0530, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uMaxExp2 65ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x167C, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp3 90ms 
	{0x0F12, 0x0000, WORD_LEN}, 
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN}, 
	
	// #Set capture exposure time
	{0x002A, 0x0538, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp1 60ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #lt_uCapMaxExp2 65ms 
	{0x0F12, 0x0000, WORD_LEN},
	{0x002A, 0x1684, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp3 90ms
	{0x0F12, 0x0000, WORD_LEN},
	{0x0F12, 0xC350, WORD_LEN}, // #evt1_lt_uCapMaxExp4 110ms
	{0x0F12, 0x0000, WORD_LEN},
	
	
	// #Set gain
	{0x002A, 0x0540, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain1
	{0x0F12, 0x0200, WORD_LEN}, // #lt_uMaxAnGain2
	{0x002A, 0x168C, WORD_LEN},
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain3
	{0x0F12, 0x0200, WORD_LEN}, // #evt1_lt_uMaxAnGain4 
	 
	
	{0x002A, 0x0544, WORD_LEN},   
	{0x0F12, 0x0100, WORD_LEN}, // #lt_uMaxDigGain
	{0x0F12, 0x8000, WORD_LEN}, // #lt_uMaxTotGain
	
	{0x002A, 0x04B4, WORD_LEN},   
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoType
	{0x0F12, 0x0190, WORD_LEN},// #REG_SF_USER_IsoVal
	{0x0F12, 0x0001, WORD_LEN}, // #REG_SF_USER_IsoChanged

};

static const struct lgcam_rear_sensor_i2c_reg_conf const zoom_mode_capture_127_settings_array[] = {

};

static const struct lgcam_rear_sensor_i2c_reg_conf const zoom_mode_capture_162_settings_array[] = {

};

static const struct lgcam_rear_sensor_i2c_reg_conf const zoom_mode_capture_203_settings_array[] = {

};

static const struct lgcam_rear_sensor_i2c_reg_conf const zoom_mode_capture_405_settings_array[] = {

};

static const struct lgcam_rear_sensor_i2c_reg_conf const focus_rect_reg_settings_array[] = {
// AF Window Settings(default)
{0x0028, 0x7000, WORD_LEN},
{0x002A, 0x025A, WORD_LEN},
{0xFFFE, 0x0100, BURST_LEN}, //#REG_TC_AF_FstWinStartX
{0x0, 0x00E3, BURST_LEN}, //#REG_TC_AF_FstWinStartY
{0x0, 0x0200, BURST_LEN}, //#REG_TC_AF_FstWinSizeX
{0x0, 0x0238, BURST_LEN}, //#REG_TC_AF_FstWinSizeY
{0x0, 0x018C, BURST_LEN}, //#REG_TC_AF_ScndWinStartX
{0x0, 0x0166, BURST_LEN}, //#REG_TC_AF_ScndWinStartY
{0x0, 0x00E6, BURST_LEN}, //#REG_TC_AF_ScndWinSizeX
{0x0, 0x0132, BURST_LEN}, //#REG_TC_AF_ScndWinSizeY
{0xFFFF, 0x0001, BURST_LEN}, //#REG_TC_AF_WinSizesUpdated
};

static const struct lgcam_rear_sensor_i2c_reg_conf const auto_frame_reg_settings_array[] = {
	//normal 30fps
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x0286, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, // #REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x03E8, WORD_LEN}, // #REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps
	{0x0F12, 0x014D, WORD_LEN}, // #REG_0TC_PCFG_usMinFrTimeMsecMult10 //30fps
	
	//============================================================												   
	// Preview Configuration Update setting 											 
	//============================================================																				
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x023C, WORD_LEN},
	{0x0F12, 0x0000, WORD_LEN}, //#REG_TC_GP_ActivePrevConfig	  //preview config0
	{0x002A, 0x0240, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevOpenAfterChange  //config change
	{0x002A, 0x0230, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_NewConfigSync
	{0x002A, 0x023E, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreview
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreviewChfanged

};

static const struct lgcam_rear_sensor_i2c_reg_conf const fixed_frame_reg_settings_array[] = {
	//Fixed 30fps
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x0286, WORD_LEN},
	{0x0F12, 0x0000, WORD_LEN}, // #REG_0TC_PCFG_FrRateQualityType
	{0x0F12, 0x014D, WORD_LEN}, // #REG_0TC_PCFG_usMaxFrTimeMsecMult10 //30fps for test
	{0x0F12, 0x014D, WORD_LEN}, // #REG_0TC_PCFG_usMinFrTimeMsecMult10 //30fps for test
	
	//============================================================												   
	// Preview Configuration Update setting 											 
	//============================================================																				
	{0x0028, 0x7000, WORD_LEN},
	{0x002A, 0x023C, WORD_LEN},
	{0x0F12, 0x0000, WORD_LEN}, //#REG_TC_GP_ActivePrevConfig	  //preview config0
	{0x002A, 0x0240, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevOpenAfterChange  //config change
	{0x002A, 0x0230, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_NewConfigSync
	{0x002A, 0x023E, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_PrevConfigChanged
	{0x002A, 0x0220, WORD_LEN},
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreview
	{0x0F12, 0x0001, WORD_LEN}, //#REG_TC_GP_EnablePreviewChfanged

};


struct lgcam_rear_sensor_reg lgcam_rear_sensor_regs = {

	.pll = pll_settings_array,
	.pll_size= ARRAY_SIZE(pll_settings_array),

	.init = init_settings_array,
	.init_size= ARRAY_SIZE(init_settings_array),

	.prev_reg_settings = preview_mode_reg_settings_array,
	.prev_reg_settings_size = ARRAY_SIZE(preview_mode_reg_settings_array),
	.snap_reg_settings = snapshot_mode_reg_settings_array,
	.snap_reg_settings_size = ARRAY_SIZE(
		snapshot_mode_reg_settings_array),

	.ap001_16bit_settings = preview_mode_ap001_16bit_settings_array,
	.ap001_16bit_settings_size = ARRAY_SIZE(preview_mode_ap001_16bit_settings_array),

	.ap003_16bit_settings = preview_mode_ap003_16bit_settings_array,
	.ap003_16bit_settings_size = ARRAY_SIZE(preview_mode_ap003_16bit_settings_array),

	.scene_normal_reg_settings = scene_mode_normal_reg_settings_array,
	.scene_normal_reg_settings_size = ARRAY_SIZE(scene_mode_normal_reg_settings_array),

	.scene_portrait_reg_settings = scene_mode_portrait_reg_settings_array,
	.scene_portrait_reg_settings_size = ARRAY_SIZE(scene_mode_portrait_reg_settings_array),

	.scene_landscape_reg_settings = scene_mode_landscape_reg_settings_array,
	.scene_landscape_reg_settings_size = ARRAY_SIZE(scene_mode_landscape_reg_settings_array),

	.scene_sport_reg_settings = scene_mode_sport_reg_settings_array,
	.scene_sport_reg_settings_size = ARRAY_SIZE(scene_mode_sport_reg_settings_array),

	.scene_sunset_reg_settings = scene_mode_sunset_reg_settings_array,
	.scene_sunset_reg_settings_size = ARRAY_SIZE(scene_mode_sunset_reg_settings_array),

	.scene_night_reg_settings = scene_mode_night_reg_settings_array,
	.scene_night_reg_settings_size = ARRAY_SIZE(scene_mode_night_reg_settings_array),

	.AF_reg_settings = af_reg_settings_array,
	.AF_reg_settings_size = ARRAY_SIZE(af_reg_settings_array), 

	.AF_nomal_reg_settings = af_nomal_mode_reg_settings_array,
	.AF_nomal_reg_settings_size = ARRAY_SIZE(af_nomal_mode_reg_settings_array), 

	.AF_macro_reg_settings = af_macro_mode_reg_settings_array,
	.AF_macro_reg_settings_size = ARRAY_SIZE(af_macro_mode_reg_settings_array),
	
	.manual_focus_reg_settings = manual_focus_mode_reg_settings_array,
	.manual_focus_reg_settings_size = ARRAY_SIZE(manual_focus_mode_reg_settings_array),
	
	.iso_auto_reg_settings = iso_mode_auto_reg_settings_array,
	.iso_auto_reg_settings_size = ARRAY_SIZE(iso_mode_auto_reg_settings_array),
	
	.iso_100_reg_settings = iso_mode_100_reg_settings_array,
	.iso_100_reg_settings_size = ARRAY_SIZE(iso_mode_100_reg_settings_array),
	
	.iso_200_reg_settings = iso_mode_200_reg_settings_array,
	.iso_200_reg_settings_size = ARRAY_SIZE(iso_mode_200_reg_settings_array),
	
	.iso_400_reg_settings = iso_mode_400_reg_settings_array,
	.iso_400_reg_settings_size = ARRAY_SIZE(iso_mode_400_reg_settings_array),

	.iso_800_reg_settings = iso_mode_800_reg_settings_array,
	.iso_800_reg_settings_size = ARRAY_SIZE(iso_mode_800_reg_settings_array),

	.zoom_mode_capture_127_reg_settings = zoom_mode_capture_127_settings_array,
	.zoom_mode_capture_127_reg_settings_size = ARRAY_SIZE(zoom_mode_capture_127_settings_array),

	.zoom_mode_capture_162_reg_settings = zoom_mode_capture_162_settings_array,
	.zoom_mode_capture_162_reg_settings_size = ARRAY_SIZE(zoom_mode_capture_162_settings_array),

	.zoom_mode_capture_203_reg_settings = zoom_mode_capture_203_settings_array,
	.zoom_mode_capture_203_reg_settings_size = ARRAY_SIZE(zoom_mode_capture_203_settings_array),

	.zoom_mode_capture_405_reg_settings = zoom_mode_capture_405_settings_array,
	.zoom_mode_capture_405_reg_settings_size = ARRAY_SIZE(zoom_mode_capture_405_settings_array),
	.focus_rect_reg_settings = focus_rect_reg_settings_array,
	.focus_rect_reg_settings_size = ARRAY_SIZE(focus_rect_reg_settings_array),	

	.auto_frame_reg_settings = auto_frame_reg_settings_array,
	.auto_frame_reg_settings_size = ARRAY_SIZE(auto_frame_reg_settings_array),

	.fixed_frame_reg_settings = fixed_frame_reg_settings_array,
	.fixed_frame_reg_settings_size = ARRAY_SIZE(fixed_frame_reg_settings_array),		
};
#endif /* #define lgcam_rear_sensor_REG_H */

