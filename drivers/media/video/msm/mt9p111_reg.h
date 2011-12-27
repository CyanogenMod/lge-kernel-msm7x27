/*
 * drivers/media/video/msm/mt9p111_reg.h
 *
 * Aptina MT9P111 1/4-Inch 5MP System-On-Chip(SOC) CMOS Digital Image Sensor
 *
 * Copyright (C) 2011 LG Electronics.
 * Author: taiyou.kang@lge.com, 2011-03-10
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

#ifndef MT9P111_REG_H
#define MT9P111_REG_H

#include "mt9p111.h"

static struct mt9p111_register_pair const init_mode_settings_array[] =
{
	//for 24MHz input, PCLK=76.8MHz
	{CMD_WRITE, 0x0010, 0x0340, WORD_LEN},          // PLL_DIVIDERS
	{CMD_WRITE, 0x0012, 0x0090, WORD_LEN},          // PLL_P_DIVIDERS
	{CMD_WRITE, 0x0014, 0x2025, WORD_LEN},          // PLL_CONTROL
	{CMD_WRITE, 0x001E, 0x0665, WORD_LEN},          // PAD_SLEW_PAD_CONFIG
	{CMD_WRITE, 0x0022, 0x0030, WORD_LEN},          // VDD_DIS_COUNTER
	{CMD_WRITE, 0x002A, 0x7F7C, WORD_LEN},          // PLL_P4_P5_P6_DIVIDERS
	{CMD_WRITE, 0x002C, 0x0000, WORD_LEN},        
	{CMD_WRITE, 0x002E, 0x0000, WORD_LEN},        
	{CMD_WRITE, 0x0018, 0x400C, WORD_LEN},         // STANDBY_CONTROL_AND_STATUS
	{CMD_POLL, 0x0018, 0x20, 100},  //Wait for the core ready
	{CMD_WRITE, 0x0010, 0x0340, WORD_LEN},     // PLL_DIVIDERS                           
	// touch up sensor core state
	{CMD_WRITE, 0x301A, 0x0030, WORD_LEN},
	{CMD_WRITE, 0x301E, 0x0083, WORD_LEN},
	//timing_settings
	{CMD_WRITE, 0x098E, 0x483A, WORD_LEN}, 	// LOGICAL_ADDRESS_ACCESS                     
	{CMD_WRITE, 0xC84C, 0x0096, WORD_LEN}, 	// CAM_CORE_A_POWER_MODE                      
	{CMD_WRITE, 0xC851, 0x00, BYTE_LEN}, 	// CAM_CORE_A_PIXEL_ORDER                     
	{CMD_WRITE, 0xC852, 0x019C, WORD_LEN}, 	// CAM_CORE_A_FINE_CORRECTION                 
	{CMD_WRITE, 0xC862, 0x116c, WORD_LEN}, 	// CAM_CORE_A_MIN_LINE_LENGTH_PCLK            
	{CMD_WRITE, 0xC866, 0x7F7c, WORD_LEN}, 	// CAM_CORE_A_P4_5_6_DIVIDER                  
	{CMD_WRITE, 0xC86A, 0x116c, WORD_LEN}, 	// CAM_CORE_A_LINE_LENGTH_PCK                 
	{CMD_WRITE, 0xC858, 0x0000, WORD_LEN}, 	// CAM_CORE_A_COARSE_ITMIN                    
	{CMD_WRITE, 0x098E, 0x4872, WORD_LEN}, 	// LOGICAL_ADDRESS_ACCESS                     
	{CMD_WRITE, 0xC884, 0x005C, WORD_LEN}, 	// CAM_CORE_B_POWER_MODE                      
	{CMD_WRITE, 0xC894, 0x07EF, WORD_LEN}, 	// CAM_CORE_B_MIN_FRAME_LENGTH_LINES          
	{CMD_WRITE, 0xC89A, 0x1964, WORD_LEN}, 	// CAM_CORE_B_MIN_LINE_LENGTH_PCLK            
	{CMD_WRITE, 0xC89E, 0x7F7c, WORD_LEN}, 	// CAM_CORE_B_P4_5_6_DIVIDER                  
	{CMD_WRITE, 0xC8A0, 0x07EF, WORD_LEN}, 	// CAM_CORE_B_FRAME_LENGTH_LINES              
	{CMD_WRITE, 0xC8A2, 0x1F40, WORD_LEN}, 	// CAM_CORE_B_LINE_LENGTH_PCK                 
	{CMD_WRITE, 0xC8A8, 0x0124, WORD_LEN}, 	// CAM_CORE_B_RX_FIFO_TRIGGER_MARK            
	{CMD_WRITE, 0xC890, 0x0000, WORD_LEN}, 	// CAM_CORE_B_COARSE_ITMIN                    
	{CMD_WRITE, 0xC8CE, 0x0014, WORD_LEN}, 	// CAM_OUTPUT_1_JPEG_CONTROL                  
	{CMD_WRITE, 0xA010, 0x0104, WORD_LEN}, 	//104 FD_MIN_EXPECTED50HZ_FLICKER_PERIOD         
	{CMD_WRITE, 0xA012, 0x0127, WORD_LEN}, 	//127 FD_MAX_EXPECTED50HZ_FLICKER_PERIOD         
	{CMD_WRITE, 0xA014, 0x00C6, WORD_LEN}, 	//c6 FD_MIN_EXPECTED60HZ_FLICKER_PERIOD         
	{CMD_WRITE, 0xA016, 0x00E4, WORD_LEN},  //e4 FD_MAX_EXPECTED60HZ_FLICKER_PERIOD         
	{CMD_WRITE, 0xA018, 0x010E, WORD_LEN},  	// FD_EXPECTED50HZ_FLICKER_PERIOD_IN_CONTEXT_A
	{CMD_WRITE, 0xA01A, 0x0095, WORD_LEN},  	// FD_EXPECTED50HZ_FLICKER_PERIOD_IN_CONTEXT_B
	{CMD_WRITE, 0xA01C, 0x00DC, WORD_LEN},  	// DC FD_EXPECTED60HZ_FLICKER_PERIOD_IN_CONTEXT_A
	{CMD_WRITE, 0xA01E, 0x007B, WORD_LEN},  	// FD_EXPECTED60HZ_FLICKER_PERIOD_IN_CONTEXT_B                                                    
	//[k28a_rev3_FW_patch7]
	//NEW test Patch_7_7
	//k28a_rev03_patch07_CR30221_AWB_AFNOISE_YOFFSET_APGA_REV6
	{CMD_WRITE, 0x0982, 0x0000, WORD_LEN},  	// ACCESS_CTL_STAT          
	{CMD_WRITE, 0x098A, 0x0000 , WORD_LEN}, 	// PHYSICAL_ADDRESS_ACCESS  
	{CMD_WRITE_BURST_S, 0x886C, 0xC0F1, WORD_LEN},
	{CMD_WRITE, 0x886E, 0xC5E1, WORD_LEN},
	{CMD_WRITE, 0x8870, 0x246A, WORD_LEN},
	{CMD_WRITE, 0x8872, 0x1280, WORD_LEN},
	{CMD_WRITE, 0x8874, 0xC4E1, WORD_LEN},
	{CMD_WRITE, 0x8876, 0xD20F, WORD_LEN},
	{CMD_WRITE, 0x8878, 0x2069, WORD_LEN},
	{CMD_WRITE, 0x887A, 0x0000, WORD_LEN},
	{CMD_WRITE, 0x887C, 0x6A62, WORD_LEN},
	{CMD_WRITE, 0x887E, 0x1303, WORD_LEN},
	{CMD_WRITE, 0x8880, 0x0084, WORD_LEN},
	{CMD_WRITE, 0x8882, 0x1734, WORD_LEN},
	{CMD_WRITE, 0x8884, 0x7005, WORD_LEN},
	{CMD_WRITE, 0x8886, 0xD801, WORD_LEN},
	{CMD_WRITE, 0x8888, 0x8A41, WORD_LEN},
	{CMD_WRITE, 0x888A, 0xD900, WORD_LEN},
	{CMD_WRITE, 0x888C, 0x0D5A, WORD_LEN},
	{CMD_WRITE, 0x888E, 0x0664, WORD_LEN},
	{CMD_WRITE, 0x8890, 0x8B61, WORD_LEN},
	{CMD_WRITE, 0x8892, 0xE80B, WORD_LEN},
	{CMD_WRITE, 0x8894, 0x000D, WORD_LEN},
	{CMD_WRITE, 0x8896, 0x0020, WORD_LEN},
	{CMD_WRITE, 0x8898, 0xD508, WORD_LEN},
	{CMD_WRITE, 0x889A, 0x1504, WORD_LEN},
	{CMD_WRITE, 0x889C, 0x1400, WORD_LEN},
	{CMD_WRITE, 0x889E, 0x7840, WORD_LEN},
	{CMD_WRITE, 0x88A0, 0xD007, WORD_LEN},
	{CMD_WRITE, 0x88A2, 0x0DFB, WORD_LEN},
	{CMD_WRITE, 0x88A4, 0x9004, WORD_LEN},
	{CMD_WRITE, 0x88A6, 0xC4C1, WORD_LEN},
	{CMD_WRITE, 0x88A8, 0x2029, WORD_LEN},
	{CMD_WRITE, 0x88AA, 0x0300, WORD_LEN},
	{CMD_WRITE, 0x88AC, 0x0219, WORD_LEN},
	{CMD_WRITE, 0x88AE, 0x06C4, WORD_LEN},
	{CMD_WRITE, 0x88B0, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88B2, 0x08D0, WORD_LEN},                                               
	{CMD_WRITE, 0x88B4, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88B6, 0x086C, WORD_LEN},
	{CMD_WRITE, 0x88B8, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88BA, 0x08C0, WORD_LEN},
	{CMD_WRITE, 0x88BC, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88BE, 0x08D0, WORD_LEN},
	{CMD_WRITE, 0x88C0, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88C2, 0x08D8, WORD_LEN},                                               
	{CMD_WRITE, 0x88C4, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88C6, 0x0998, WORD_LEN},                                               
	{CMD_WRITE, 0x88C8, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x88CA, 0x11AC, WORD_LEN},                                               
	{CMD_WRITE, 0x88CC, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x88CE, 0x09BC, WORD_LEN},                                               
	{CMD_WRITE, 0x88D0, 0x0007, WORD_LEN},                                               
	{CMD_WRITE, 0x88D2, 0x0007, WORD_LEN},                                               
	{CMD_WRITE, 0x88D4, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x88D6, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x88D8, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x88DA, 0x0976, WORD_LEN},                                               
	{CMD_WRITE, 0x88DC, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x88DE, 0xDA14, WORD_LEN},                                               
	{CMD_WRITE, 0x88E0, 0xD0EF, WORD_LEN},                                               
	{CMD_WRITE, 0x88E2, 0xDE00, WORD_LEN},                                               
	{CMD_WRITE, 0x88E4, 0xD1EF, WORD_LEN},                                               
	{CMD_WRITE, 0x88E6, 0x2E41, WORD_LEN},                                               
	{CMD_WRITE, 0x88E8, 0x120C, WORD_LEN},                                               
	{CMD_WRITE, 0x88EA, 0xA895, WORD_LEN},                                               
	{CMD_WRITE, 0x88EC, 0xD5EE, WORD_LEN},                                               
	{CMD_WRITE, 0x88EE, 0xA8D4, WORD_LEN},                                               
	{CMD_WRITE, 0x88F0, 0xA8D6, WORD_LEN},                                               
	{CMD_WRITE, 0x88F2, 0x0F02, WORD_LEN},                                               
	{CMD_WRITE, 0x88F4, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x88F6, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x88F8, 0x1440, WORD_LEN},                                               
	{CMD_WRITE, 0x88FA, 0xD0EC, WORD_LEN},                                               
	{CMD_WRITE, 0x88FC, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x88FE, 0x1441, WORD_LEN},                                               
	{CMD_WRITE, 0x8900, 0x77A9, WORD_LEN},                                               
	{CMD_WRITE, 0x8902, 0xA515, WORD_LEN},                                               
	{CMD_WRITE, 0x8904, 0xD0EA, WORD_LEN},                                               
	{CMD_WRITE, 0x8906, 0xA020, WORD_LEN},                                               
	{CMD_WRITE, 0x8908, 0xD1EA, WORD_LEN},                                               
	{CMD_WRITE, 0x890A, 0x70E9, WORD_LEN},                                               
	{CMD_WRITE, 0x890C, 0x0EE6, WORD_LEN},                                               
	{CMD_WRITE, 0x890E, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8910, 0xDA44, WORD_LEN},                                               
	{CMD_WRITE, 0x8912, 0xD0E9, WORD_LEN},                                               
	{CMD_WRITE, 0x8914, 0xA502, WORD_LEN},                                               
	{CMD_WRITE, 0x8916, 0xD0E9, WORD_LEN},                                               
	{CMD_WRITE, 0x8918, 0xA0E0, WORD_LEN},                                               
	{CMD_WRITE, 0x891A, 0xD7E9, WORD_LEN},                                               
	{CMD_WRITE, 0x891C, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x891E, 0x1CC0, WORD_LEN},                                               
	{CMD_WRITE, 0x8920, 0x8720, WORD_LEN},                                               
	{CMD_WRITE, 0x8922, 0x0F82, WORD_LEN},                                               
	{CMD_WRITE, 0x8924, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8926, 0xDA34, WORD_LEN},                                               
	{CMD_WRITE, 0x8928, 0xD0E6, WORD_LEN},                                               
	{CMD_WRITE, 0x892A, 0xD1E7, WORD_LEN},                                               
	{CMD_WRITE, 0x892C, 0x1DD8, WORD_LEN},                                               
	{CMD_WRITE, 0x892E, 0x1000, WORD_LEN},                                               
	{CMD_WRITE, 0x8930, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x8932, 0x1CC0, WORD_LEN},                                               
	{CMD_WRITE, 0x8934, 0xA700, WORD_LEN},                                               
	{CMD_WRITE, 0x8936, 0xD0E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8938, 0xB0CB, WORD_LEN},                                               
	{CMD_WRITE, 0x893A, 0x8900, WORD_LEN},                                               
	{CMD_WRITE, 0x893C, 0xDB08, WORD_LEN},                                               
	{CMD_WRITE, 0x893E, 0xDAF0, WORD_LEN},                                               
	{CMD_WRITE, 0x8940, 0x19B0, WORD_LEN},                                               
	{CMD_WRITE, 0x8942, 0x00C2, WORD_LEN},                                               
	{CMD_WRITE, 0x8944, 0xB8A6, WORD_LEN},                                               
	{CMD_WRITE, 0x8946, 0xA900, WORD_LEN},                                               
	{CMD_WRITE, 0x8948, 0xD851, WORD_LEN},                                               
	{CMD_WRITE, 0x894A, 0x19B2, WORD_LEN},                                               
	{CMD_WRITE, 0x894C, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x894E, 0xD852, WORD_LEN},                                               
	{CMD_WRITE, 0x8950, 0x19B3, WORD_LEN},                                               
	{CMD_WRITE, 0x8952, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x8954, 0xD855, WORD_LEN},                                               
	{CMD_WRITE, 0x8956, 0x19B6, WORD_LEN},                                               
	{CMD_WRITE, 0x8958, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x895A, 0xD856, WORD_LEN},                                               
	{CMD_WRITE, 0x895C, 0x19B7, WORD_LEN},                                               
	{CMD_WRITE, 0x895E, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x8960, 0xD896, WORD_LEN},                                               
	{CMD_WRITE, 0x8962, 0x19B8, WORD_LEN},                                               
	{CMD_WRITE, 0x8964, 0x0004, WORD_LEN},                                               
	{CMD_WRITE, 0x8966, 0xD814, WORD_LEN},                                               
	{CMD_WRITE, 0x8968, 0x19BA, WORD_LEN},                                               
	{CMD_WRITE, 0x896A, 0x0004, WORD_LEN},                                               
	{CMD_WRITE, 0x896C, 0xD805, WORD_LEN},                                               
	{CMD_WRITE, 0x896E, 0xB111, WORD_LEN},                                               
	{CMD_WRITE, 0x8970, 0x19B1, WORD_LEN},                                               
	{CMD_WRITE, 0x8972, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8974, 0x19B4, WORD_LEN},                                               
	{CMD_WRITE, 0x8976, 0x00C2, WORD_LEN},                                               
	{CMD_WRITE, 0x8978, 0x19B5, WORD_LEN},                                               
	{CMD_WRITE, 0x897A, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x897C, 0xD1D4, WORD_LEN},                                               
	{CMD_WRITE, 0x897E, 0x2556, WORD_LEN},                                               
	{CMD_WRITE, 0x8980, 0x12C0, WORD_LEN},                                               
	{CMD_WRITE, 0x8982, 0x0E72, WORD_LEN},                                               
	{CMD_WRITE, 0x8984, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8986, 0xDA2C, WORD_LEN},                                               
	{CMD_WRITE, 0x8988, 0xD0D2, WORD_LEN},                                               
	{CMD_WRITE, 0x898A, 0x2556, WORD_LEN},                                               
	{CMD_WRITE, 0x898C, 0x12C1, WORD_LEN},                                               
	{CMD_WRITE, 0x898E, 0xA519, WORD_LEN},                                               
	{CMD_WRITE, 0x8990, 0xD0D1, WORD_LEN},                                               
	{CMD_WRITE, 0x8992, 0x0125, WORD_LEN},                                               
	{CMD_WRITE, 0x8994, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x8996, 0xA020, WORD_LEN},                                               
	{CMD_WRITE, 0x8998, 0xD0D0, WORD_LEN},                                               
	{CMD_WRITE, 0x899A, 0xD1C3, WORD_LEN},                                               
	{CMD_WRITE, 0x899C, 0x1984, WORD_LEN},                                               
	{CMD_WRITE, 0x899E, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x89A0, 0xD0CF, WORD_LEN},                                               
	{CMD_WRITE, 0x89A2, 0x1988, WORD_LEN},                                               
	{CMD_WRITE, 0x89A4, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x89A6, 0xD0CF, WORD_LEN},                                               
	{CMD_WRITE, 0x89A8, 0x198C, WORD_LEN},                                               
	{CMD_WRITE, 0x89AA, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x89AC, 0xD0CE, WORD_LEN},                                               
	{CMD_WRITE, 0x89AE, 0x1990, WORD_LEN},                                               
	{CMD_WRITE, 0x89B0, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x89B2, 0xD0CE, WORD_LEN},                                               
	{CMD_WRITE, 0x89B4, 0x2180, WORD_LEN},                                               
	{CMD_WRITE, 0x89B6, 0x0102, WORD_LEN},                                               
	{CMD_WRITE, 0x89B8, 0x7FE0, WORD_LEN},                                               
	{CMD_WRITE, 0x89BA, 0xA020, WORD_LEN},                                               
	{CMD_WRITE, 0x89BC, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x89BE, 0xC5E1, WORD_LEN},                                               
	{CMD_WRITE, 0x89C0, 0xD5B9, WORD_LEN},                                               
	{CMD_WRITE, 0x89C2, 0xD1CB, WORD_LEN},                                               
	{CMD_WRITE, 0x89C4, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x89C6, 0x1940, WORD_LEN},                                               
	{CMD_WRITE, 0x89C8, 0x0E2A, WORD_LEN},                                               
	{CMD_WRITE, 0x89CA, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x89CC, 0xDA38, WORD_LEN},                                               
	{CMD_WRITE, 0x89CE, 0xD0C9, WORD_LEN},                                               
	{CMD_WRITE, 0x89D0, 0xD1C9, WORD_LEN},                                               
	{CMD_WRITE, 0x89D2, 0xA021, WORD_LEN},                                               
	{CMD_WRITE, 0x89D4, 0xD0C9, WORD_LEN},                                               
	{CMD_WRITE, 0x89D6, 0x2555, WORD_LEN},                                               
	{CMD_WRITE, 0x89D8, 0x1941, WORD_LEN},                                               
	{CMD_WRITE, 0x89DA, 0x1DA8, WORD_LEN},                                               
	{CMD_WRITE, 0x89DC, 0x1000, WORD_LEN},                                               
	{CMD_WRITE, 0x89DE, 0xD0C8, WORD_LEN},                                               
	{CMD_WRITE, 0x89E0, 0xA020, WORD_LEN},                                               
	{CMD_WRITE, 0x89E2, 0xD0C8, WORD_LEN},                                               
	{CMD_WRITE, 0x89E4, 0x802E, WORD_LEN},                                               
	{CMD_WRITE, 0x89E6, 0x9117, WORD_LEN},                                               
	{CMD_WRITE, 0x89E8, 0x00DD, WORD_LEN},                                               
	{CMD_WRITE, 0x89EA, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x89EC, 0xB10E, WORD_LEN},                                               
	{CMD_WRITE, 0x89EE, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x89F0, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x89F2, 0x0862, WORD_LEN},                                               
	{CMD_WRITE, 0x89F4, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x89F6, 0xDB03, WORD_LEN},                                               
	{CMD_WRITE, 0x89F8, 0xD2C3, WORD_LEN},                                               
	{CMD_WRITE, 0x89FA, 0x8A2E, WORD_LEN},                                               
	{CMD_WRITE, 0x89FC, 0x8ACF, WORD_LEN},                                               
	{CMD_WRITE, 0x89FE, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8A00, 0x79C5, WORD_LEN},                                               
	{CMD_WRITE, 0x8A02, 0xDD65, WORD_LEN},                                               
	{CMD_WRITE, 0x8A04, 0x094F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A06, 0x00D1, WORD_LEN},                                               
	{CMD_WRITE, 0x8A08, 0xD90A, WORD_LEN},                                               
	{CMD_WRITE, 0x8A0A, 0x1A24, WORD_LEN},                                               
	{CMD_WRITE, 0x8A0C, 0x0042, WORD_LEN},                                               
	{CMD_WRITE, 0x8A0E, 0x8A24, WORD_LEN},                                               
	{CMD_WRITE, 0x8A10, 0xE1E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8A12, 0xF6C9, WORD_LEN},                                               
	{CMD_WRITE, 0x8A14, 0xD902, WORD_LEN},                                               
	{CMD_WRITE, 0x8A16, 0x2941, WORD_LEN},                                               
	{CMD_WRITE, 0x8A18, 0x0200, WORD_LEN},                                               
	{CMD_WRITE, 0x8A1A, 0xAA0E, WORD_LEN},                                               
	{CMD_WRITE, 0x8A1C, 0xAA2F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A1E, 0x70A9, WORD_LEN},                                               
	{CMD_WRITE, 0x8A20, 0xF014, WORD_LEN},                                               
	{CMD_WRITE, 0x8A22, 0xE1C8, WORD_LEN},                                               
	{CMD_WRITE, 0x8A24, 0x0036, WORD_LEN},                                               
	{CMD_WRITE, 0x8A26, 0x000B, WORD_LEN},                                               
	{CMD_WRITE, 0x8A28, 0xE0C8, WORD_LEN},                                               
	{CMD_WRITE, 0x8A2A, 0x003A, WORD_LEN},                                               
	{CMD_WRITE, 0x8A2C, 0x000A, WORD_LEN},                                               
	{CMD_WRITE, 0x8A2E, 0xD901, WORD_LEN},                                               
	{CMD_WRITE, 0x8A30, 0x2941, WORD_LEN},                                               
	{CMD_WRITE, 0x8A32, 0x0200, WORD_LEN},                                               
	{CMD_WRITE, 0x8A34, 0xAA0E, WORD_LEN},                                               
	{CMD_WRITE, 0x8A36, 0xAA2F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A38, 0xD848, WORD_LEN},                                               
	{CMD_WRITE, 0x8A3A, 0xF008, WORD_LEN},                                               
	{CMD_WRITE, 0x8A3C, 0xD900, WORD_LEN},                                               
	{CMD_WRITE, 0x8A3E, 0x2941, WORD_LEN},                                               
	{CMD_WRITE, 0x8A40, 0x0200, WORD_LEN},                                               
	{CMD_WRITE, 0x8A42, 0xAA0E, WORD_LEN},                                               
	{CMD_WRITE, 0x8A44, 0xAA2F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A46, 0xD820, WORD_LEN},                                               
	{CMD_WRITE, 0x8A48, 0xD2B0, WORD_LEN},                                               
	{CMD_WRITE, 0x8A4A, 0x8A26, WORD_LEN},                                               
	{CMD_WRITE, 0x8A4C, 0xB961, WORD_LEN},                                               
	{CMD_WRITE, 0x8A4E, 0xAA26, WORD_LEN},                                               
	{CMD_WRITE, 0x8A50, 0xF00D, WORD_LEN},                                               
	{CMD_WRITE, 0x8A52, 0x091F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A54, 0x0091, WORD_LEN},                                               
	{CMD_WRITE, 0x8A56, 0x8A24, WORD_LEN},                                               
	{CMD_WRITE, 0x8A58, 0xF1E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8A5A, 0x0913, WORD_LEN},                                               
	{CMD_WRITE, 0x8A5C, 0x0812, WORD_LEN},                                               
	{CMD_WRITE, 0x8A5E, 0x08E1, WORD_LEN},                                               
	{CMD_WRITE, 0x8A60, 0x8812, WORD_LEN},                                               
	{CMD_WRITE, 0x8A62, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x8A64, 0x0201, WORD_LEN},                                               
	{CMD_WRITE, 0x8A66, 0xAA2E, WORD_LEN},                                               
	{CMD_WRITE, 0x8A68, 0xAA6F, WORD_LEN},                                               
	{CMD_WRITE, 0x8A6A, 0x0055, WORD_LEN},                                               
	{CMD_WRITE, 0x8A6C, 0x06C4, WORD_LEN},                                               
	{CMD_WRITE, 0x8A6E, 0x09F7, WORD_LEN},                                               
	{CMD_WRITE, 0x8A70, 0x8051, WORD_LEN},                                               
	{CMD_WRITE, 0x8A72, 0x8A24, WORD_LEN},                                               
	{CMD_WRITE, 0x8A74, 0xF1F3, WORD_LEN},                                               
	{CMD_WRITE, 0x8A76, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8A78, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8A7A, 0x0FCE, WORD_LEN},                                               
	{CMD_WRITE, 0x8A7C, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8A7E, 0xD6A2, WORD_LEN},                                               
	{CMD_WRITE, 0x8A80, 0x7508, WORD_LEN},                                               
	{CMD_WRITE, 0x8A82, 0x8E01, WORD_LEN},                                               
	{CMD_WRITE, 0x8A84, 0xD1A1, WORD_LEN},                                               
	{CMD_WRITE, 0x8A86, 0x2046, WORD_LEN},                                               
	{CMD_WRITE, 0x8A88, 0x00C0, WORD_LEN},                                               
	{CMD_WRITE, 0x8A8A, 0xAE01, WORD_LEN},                                               
	{CMD_WRITE, 0x8A8C, 0x1145, WORD_LEN},                                               
	{CMD_WRITE, 0x8A8E, 0x0080, WORD_LEN},                                               
	{CMD_WRITE, 0x8A90, 0x1146, WORD_LEN},                                               
	{CMD_WRITE, 0x8A92, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8A94, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8A96, 0x7845, WORD_LEN},                                               
	{CMD_WRITE, 0x8A98, 0x0817, WORD_LEN},                                               
	{CMD_WRITE, 0x8A9A, 0x001E, WORD_LEN},                                               
	{CMD_WRITE, 0x8A9C, 0x8900, WORD_LEN},                                               
	{CMD_WRITE, 0x8A9E, 0x8941, WORD_LEN},                                               
	{CMD_WRITE, 0x8AA0, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8AA2, 0x7845, WORD_LEN},                                               
	{CMD_WRITE, 0x8AA4, 0x080B, WORD_LEN},                                               
	{CMD_WRITE, 0x8AA6, 0x00DE, WORD_LEN},                                               
	{CMD_WRITE, 0x8AA8, 0x70A9, WORD_LEN},                                               
	{CMD_WRITE, 0x8AAA, 0xFFD2, WORD_LEN},                                               
	{CMD_WRITE, 0x8AAC, 0x7508, WORD_LEN},                                               
	{CMD_WRITE, 0x8AAE, 0x1604, WORD_LEN},                                               
	{CMD_WRITE, 0x8AB0, 0x1090, WORD_LEN},                                               
	{CMD_WRITE, 0x8AB2, 0x0D93, WORD_LEN},                                               
	{CMD_WRITE, 0x8AB4, 0x1400, WORD_LEN},                                               
	{CMD_WRITE, 0x8AB6, 0x8EEA, WORD_LEN},                                               
	{CMD_WRITE, 0x8AB8, 0x8E0B, WORD_LEN},                                               
	{CMD_WRITE, 0x8ABA, 0x214A, WORD_LEN},                                               
	{CMD_WRITE, 0x8ABC, 0x2040, WORD_LEN},                                               
	{CMD_WRITE, 0x8ABE, 0x8E2D, WORD_LEN},                                               
	{CMD_WRITE, 0x8AC0, 0xBF08, WORD_LEN},                                               
	{CMD_WRITE, 0x8AC2, 0x7F05, WORD_LEN},                                               
	{CMD_WRITE, 0x8AC4, 0x8E0C, WORD_LEN},                                               
	{CMD_WRITE, 0x8AC6, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8AC8, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8ACA, 0x7710, WORD_LEN},                                               
	{CMD_WRITE, 0x8ACC, 0x21C2, WORD_LEN},                                               
	{CMD_WRITE, 0x8ACE, 0x244C, WORD_LEN},                                               
	{CMD_WRITE, 0x8AD0, 0x081D, WORD_LEN},                                               
	{CMD_WRITE, 0x8AD2, 0x03E3, WORD_LEN},                                               
	{CMD_WRITE, 0x8AD4, 0xD9FF, WORD_LEN},                                               
	{CMD_WRITE, 0x8AD6, 0x2702, WORD_LEN},                                               
	{CMD_WRITE, 0x8AD8, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8ADA, 0x2A05, WORD_LEN},                                               
	{CMD_WRITE, 0x8ADC, 0x037E, WORD_LEN},                                               
	{CMD_WRITE, 0x8ADE, 0x084A, WORD_LEN},                                               
	{CMD_WRITE, 0x8AE0, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x8AE2, 0x702F, WORD_LEN},                                               
	{CMD_WRITE, 0x8AE4, 0x7810, WORD_LEN},                                               
	{CMD_WRITE, 0x8AE6, 0x7F02, WORD_LEN},                                               
	{CMD_WRITE, 0x8AE8, 0x7FF0, WORD_LEN},                                               
	{CMD_WRITE, 0x8AEA, 0xF00B, WORD_LEN},                                               
	{CMD_WRITE, 0x8AEC, 0x78E2, WORD_LEN},                                               
	{CMD_WRITE, 0x8AEE, 0x2805, WORD_LEN},                                               
	{CMD_WRITE, 0x8AF0, 0x037E, WORD_LEN},                                               
	{CMD_WRITE, 0x8AF2, 0x0836, WORD_LEN},                                               
	{CMD_WRITE, 0x8AF4, 0x06E4, WORD_LEN},                                               
	{CMD_WRITE, 0x8AF6, 0x702F, WORD_LEN},                                               
	{CMD_WRITE, 0x8AF8, 0x7810, WORD_LEN},                                               
	{CMD_WRITE, 0x8AFA, 0x671F, WORD_LEN},                                               
	{CMD_WRITE, 0x8AFC, 0x7FF0, WORD_LEN},                                               
	{CMD_WRITE, 0x8AFE, 0x7FEF, WORD_LEN},                                               
	{CMD_WRITE, 0x8B00, 0x8E08, WORD_LEN},                                               
	{CMD_WRITE, 0x8B02, 0xBF06, WORD_LEN},                                               
	{CMD_WRITE, 0x8B04, 0xD182, WORD_LEN},                                               
	{CMD_WRITE, 0x8B06, 0xB8C3, WORD_LEN},                                               
	{CMD_WRITE, 0x8B08, 0x78E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8B0A, 0xB88F, WORD_LEN},                                               
	{CMD_WRITE, 0x8B0C, 0x1908, WORD_LEN},                                               
	{CMD_WRITE, 0x8B0E, 0x0024, WORD_LEN},                                               
	{CMD_WRITE, 0x8B10, 0x2841, WORD_LEN},                                               
	{CMD_WRITE, 0x8B12, 0x0201, WORD_LEN},                                               
	{CMD_WRITE, 0x8B14, 0x1E26, WORD_LEN},                                               
	{CMD_WRITE, 0x8B16, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8B18, 0x0D15, WORD_LEN},                                               
	{CMD_WRITE, 0x8B1A, 0x1423, WORD_LEN},                                               
	{CMD_WRITE, 0x8B1C, 0x1E27, WORD_LEN},                                               
	{CMD_WRITE, 0x8B1E, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8B20, 0x214C, WORD_LEN},                                               
	{CMD_WRITE, 0x8B22, 0xA000, WORD_LEN},                                               
	{CMD_WRITE, 0x8B24, 0x214A, WORD_LEN},                                               
	{CMD_WRITE, 0x8B26, 0x2040, WORD_LEN},                                               
	{CMD_WRITE, 0x8B28, 0x21C2, WORD_LEN},                                               
	{CMD_WRITE, 0x8B2A, 0x2442, WORD_LEN},                                               
	{CMD_WRITE, 0x8B2C, 0x8E21, WORD_LEN},                                               
	{CMD_WRITE, 0x8B2E, 0x214F, WORD_LEN},                                               
	{CMD_WRITE, 0x8B30, 0x0040, WORD_LEN},                                               
	{CMD_WRITE, 0x8B32, 0x090F, WORD_LEN},                                               
	{CMD_WRITE, 0x8B34, 0x2010, WORD_LEN},                                               
	{CMD_WRITE, 0x8B36, 0x2145, WORD_LEN},                                               
	{CMD_WRITE, 0x8B38, 0x0181, WORD_LEN},                                               
	{CMD_WRITE, 0x8B3A, 0xAE21, WORD_LEN},                                               
	{CMD_WRITE, 0x8B3C, 0xF003, WORD_LEN},                                               
	{CMD_WRITE, 0x8B3E, 0xB8A2, WORD_LEN},                                               
	{CMD_WRITE, 0x8B40, 0xAE01, WORD_LEN},                                               
	{CMD_WRITE, 0x8B42, 0x0BCE, WORD_LEN},                                               
	{CMD_WRITE, 0x8B44, 0xFFE3, WORD_LEN},                                               
	{CMD_WRITE, 0x8B46, 0x70A9, WORD_LEN},                                               
	{CMD_WRITE, 0x8B48, 0x075D, WORD_LEN},                                               
	{CMD_WRITE, 0x8B4A, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8B4C, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8B4E, 0xC5E1, WORD_LEN},                                               
	{CMD_WRITE, 0x8B50, 0xD56E, WORD_LEN},                                               
	{CMD_WRITE, 0x8B52, 0x8D24, WORD_LEN},                                               
	{CMD_WRITE, 0x8B54, 0x8D45, WORD_LEN},                                               
	{CMD_WRITE, 0x8B56, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8B58, 0x7945, WORD_LEN},                                               
	{CMD_WRITE, 0x8B5A, 0x0941, WORD_LEN},                                               
	{CMD_WRITE, 0x8B5C, 0x011E, WORD_LEN},                                               
	{CMD_WRITE, 0x8B5E, 0x8D26, WORD_LEN},                                               
	{CMD_WRITE, 0x8B60, 0x0939, WORD_LEN},                                               
	{CMD_WRITE, 0x8B62, 0x0093, WORD_LEN},                                               
	{CMD_WRITE, 0x8B64, 0xD168, WORD_LEN},                                               
	{CMD_WRITE, 0x8B66, 0xA907, WORD_LEN},                                               
	{CMD_WRITE, 0x8B68, 0xD066, WORD_LEN},                                               
	{CMD_WRITE, 0x8B6A, 0x802E, WORD_LEN},                                               
	{CMD_WRITE, 0x8B6C, 0x9117, WORD_LEN},                                               
	{CMD_WRITE, 0x8B6E, 0x0FC2, WORD_LEN},                                               
	{CMD_WRITE, 0x8B70, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8B72, 0x912E, WORD_LEN},                                               
	{CMD_WRITE, 0x8B74, 0x790F, WORD_LEN},                                               
	{CMD_WRITE, 0x8B76, 0x0911, WORD_LEN},                                               
	{CMD_WRITE, 0x8B78, 0x00B2, WORD_LEN},                                               
	{CMD_WRITE, 0x8B7A, 0x1541, WORD_LEN},                                               
	{CMD_WRITE, 0x8B7C, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8B7E, 0x0FB2, WORD_LEN},                                               
	{CMD_WRITE, 0x8B80, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8B82, 0x780F, WORD_LEN},                                               
	{CMD_WRITE, 0x8B84, 0x2840, WORD_LEN},                                               
	{CMD_WRITE, 0x8B86, 0x0201, WORD_LEN},                                               
	{CMD_WRITE, 0x8B88, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8B8A, 0x2841, WORD_LEN},                                               
	{CMD_WRITE, 0x8B8C, 0x0201, WORD_LEN},                                               
	{CMD_WRITE, 0x8B8E, 0x1D42, WORD_LEN},                                               
	{CMD_WRITE, 0x8B90, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8B92, 0x1D43, WORD_LEN},                                               
	{CMD_WRITE, 0x8B94, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8B96, 0xF003, WORD_LEN},                                               
	{CMD_WRITE, 0x8B98, 0xFFB8, WORD_LEN},                                               
	{CMD_WRITE, 0x8B9A, 0x072D, WORD_LEN},                                               
	{CMD_WRITE, 0x8B9C, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8B9E, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8BA0, 0xC0F1, WORD_LEN},
	{CMD_WRITE, 0x8BA2, 0xD15C, WORD_LEN},                                               
	{CMD_WRITE, 0x8BA4, 0x8906, WORD_LEN},                                               
	{CMD_WRITE, 0x8BA6, 0x8947, WORD_LEN},                                               
	{CMD_WRITE, 0x8BA8, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8BAA, 0x7845, WORD_LEN},                                               
	{CMD_WRITE, 0x8BAC, 0x262F, WORD_LEN},                                               
	{CMD_WRITE, 0x8BAE, 0xF007, WORD_LEN},                                               
	{CMD_WRITE, 0x8BB0, 0xF406, WORD_LEN},                                               
	{CMD_WRITE, 0x8BB2, 0xD050, WORD_LEN},                                               
	{CMD_WRITE, 0x8BB4, 0x8001, WORD_LEN},                                               
	{CMD_WRITE, 0x8BB6, 0x7840, WORD_LEN},                                               
	{CMD_WRITE, 0x8BB8, 0xC0D1, WORD_LEN},                                               
	{CMD_WRITE, 0x8BBA, 0x7EE0, WORD_LEN},                                               
	{CMD_WRITE, 0x8BBC, 0xB861, WORD_LEN},                                               
	{CMD_WRITE, 0x8BBE, 0x7810, WORD_LEN},                                               
	{CMD_WRITE, 0x8BC0, 0x2841, WORD_LEN},                                               
	{CMD_WRITE, 0x8BC2, 0x020C, WORD_LEN},                                               
	{CMD_WRITE, 0x8BC4, 0xA986, WORD_LEN},                                               
	{CMD_WRITE, 0x8BC6, 0xA907, WORD_LEN},                                               
	{CMD_WRITE, 0x8BC8, 0x780F, WORD_LEN},                                               
	{CMD_WRITE, 0x8BCA, 0x0815, WORD_LEN},                                               
	{CMD_WRITE, 0x8BCC, 0x0051, WORD_LEN},                                               
	{CMD_WRITE, 0x8BCE, 0xD047, WORD_LEN},                                               
	{CMD_WRITE, 0x8BD0, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8BD2, 0xD24D, WORD_LEN},                                               
	{CMD_WRITE, 0x8BD4, 0x8021, WORD_LEN},                                               
	{CMD_WRITE, 0x8BD6, 0x7960, WORD_LEN},                                               
	{CMD_WRITE, 0x8BD8, 0x8A07, WORD_LEN},                                               
	{CMD_WRITE, 0x8BDA, 0xF1F0, WORD_LEN},                                               
	{CMD_WRITE, 0x8BDC, 0xF1EE, WORD_LEN},                                               
	{CMD_WRITE, 0x8BDE, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8BE0, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8BE2, 0x0E72, WORD_LEN},                                               
	{CMD_WRITE, 0x8BE4, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8BE6, 0xD548, WORD_LEN},                                               
	{CMD_WRITE, 0x8BE8, 0x8D00, WORD_LEN},                                               
	{CMD_WRITE, 0x8BEA, 0x0841, WORD_LEN},                                               
	{CMD_WRITE, 0x8BEC, 0x01DE, WORD_LEN},                                               
	{CMD_WRITE, 0x8BEE, 0xB8A7, WORD_LEN},                                               
	{CMD_WRITE, 0x8BF0, 0x790F, WORD_LEN},                                               
	{CMD_WRITE, 0x8BF2, 0xD63E, WORD_LEN},                                               
	{CMD_WRITE, 0x8BF4, 0xAD00, WORD_LEN},                                               
	{CMD_WRITE, 0x8BF6, 0x091F, WORD_LEN},                                               
	{CMD_WRITE, 0x8BF8, 0x0050, WORD_LEN},                                               
	{CMD_WRITE, 0x8BFA, 0x0921, WORD_LEN},                                               
	{CMD_WRITE, 0x8BFC, 0x0110, WORD_LEN},                                               
	{CMD_WRITE, 0x8BFE, 0x0911, WORD_LEN},                                               
	{CMD_WRITE, 0x8C00, 0x0210, WORD_LEN},                                               
	{CMD_WRITE, 0x8C02, 0xD045, WORD_LEN},                                               
	{CMD_WRITE, 0x8C04, 0x0A6E, WORD_LEN},                                               
	{CMD_WRITE, 0x8C06, 0xFFE3, WORD_LEN},                                               
	{CMD_WRITE, 0x8C08, 0xA600, WORD_LEN},                                               
	{CMD_WRITE, 0x8C0A, 0xF00A, WORD_LEN},                                               
	{CMD_WRITE, 0x8C0C, 0x000F, WORD_LEN},                                               
	{CMD_WRITE, 0x8C0E, 0x0020, WORD_LEN},                                               
	{CMD_WRITE, 0x8C10, 0xD042, WORD_LEN},                                               
	{CMD_WRITE, 0x8C12, 0x000B, WORD_LEN},                                               
	{CMD_WRITE, 0x8C14, 0x0020, WORD_LEN},                                               
	{CMD_WRITE, 0x8C16, 0xD042, WORD_LEN},                                               
	{CMD_WRITE, 0x8C18, 0xD042, WORD_LEN},                                               
	{CMD_WRITE, 0x8C1A, 0xA600, WORD_LEN},                                               
	{CMD_WRITE, 0x8C1C, 0x8600, WORD_LEN},                                               
	{CMD_WRITE, 0x8C1E, 0x8023, WORD_LEN},                                               
	{CMD_WRITE, 0x8C20, 0x7960, WORD_LEN},                                               
	{CMD_WRITE, 0x8C22, 0xD801, WORD_LEN},                                               
	{CMD_WRITE, 0x8C24, 0xD800, WORD_LEN},                                               
	{CMD_WRITE, 0x8C26, 0xAD05, WORD_LEN},                                               
	{CMD_WRITE, 0x8C28, 0x1528, WORD_LEN},                                               
	{CMD_WRITE, 0x8C2A, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8C2C, 0x0817, WORD_LEN},                                               
	{CMD_WRITE, 0x8C2E, 0x01DE, WORD_LEN},                                               
	{CMD_WRITE, 0x8C30, 0xB8A7, WORD_LEN},                                               
	{CMD_WRITE, 0x8C32, 0x1D28, WORD_LEN},                                               
	{CMD_WRITE, 0x8C34, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8C36, 0xD02D, WORD_LEN},                                               
	{CMD_WRITE, 0x8C38, 0x8000, WORD_LEN},
	{CMD_WRITE, 0x8C3A, 0x8023, WORD_LEN},                                               
	{CMD_WRITE, 0x8C3C, 0x7960, WORD_LEN},                                               
	{CMD_WRITE, 0x8C3E, 0x1528, WORD_LEN},                                               
	{CMD_WRITE, 0x8C40, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8C42, 0x067D, WORD_LEN},                                               
	{CMD_WRITE, 0x8C44, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8C46, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8C48, 0xD22F, WORD_LEN},                                               
	{CMD_WRITE, 0x8C4A, 0x8A21, WORD_LEN},                                               
	{CMD_WRITE, 0x8C4C, 0xB9A1, WORD_LEN},                                               
	{CMD_WRITE, 0x8C4E, 0x782F, WORD_LEN},                                               
	{CMD_WRITE, 0x8C50, 0x7FE0, WORD_LEN},                                               
	{CMD_WRITE, 0x8C52, 0xAA21, WORD_LEN},                                               
	{CMD_WRITE, 0x8C54, 0xD134, WORD_LEN},                                               
	{CMD_WRITE, 0x8C56, 0xD235, WORD_LEN},                                               
	{CMD_WRITE, 0x8C58, 0x11B2, WORD_LEN},                                               
	{CMD_WRITE, 0x8C5A, 0x8903, WORD_LEN},                                               
	{CMD_WRITE, 0x8C5C, 0x1252, WORD_LEN},                                               
	{CMD_WRITE, 0x8C5E, 0x0100, WORD_LEN},                                               
	{CMD_WRITE, 0x8C60, 0x7B6F, WORD_LEN},                                               
	{CMD_WRITE, 0x8C62, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8C64, 0x2004, WORD_LEN},                                               
	{CMD_WRITE, 0x8C66, 0x0F80, WORD_LEN},                                               
	{CMD_WRITE, 0x8C68, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8C6A, 0xFF00, WORD_LEN},                                               
	{CMD_WRITE, 0x8C6C, 0x7865, WORD_LEN},                                               
	{CMD_WRITE, 0x8C6E, 0x19B2, WORD_LEN},                                               
	{CMD_WRITE, 0x8C70, 0x8024, WORD_LEN},                                               
	{CMD_WRITE, 0x8C72, 0xD02F, WORD_LEN},                                               
	{CMD_WRITE, 0x8C74, 0x8801, WORD_LEN},                                               
	{CMD_WRITE, 0x8C76, 0xB8E1, WORD_LEN},                                               
	{CMD_WRITE, 0x8C78, 0xD800, WORD_LEN},                                               
	{CMD_WRITE, 0x8C7A, 0xF404, WORD_LEN},                                               
	{CMD_WRITE, 0x8C7C, 0x1234, WORD_LEN},                                               
	{CMD_WRITE, 0x8C7E, 0x0080, WORD_LEN},                                               
	{CMD_WRITE, 0x8C80, 0x1955, WORD_LEN},                                               
	{CMD_WRITE, 0x8C82, 0x803C, WORD_LEN},                                               
	{CMD_WRITE, 0x8C84, 0x1233, WORD_LEN},                                               
	{CMD_WRITE, 0x8C86, 0x0080, WORD_LEN},                                               
	{CMD_WRITE, 0x8C88, 0xB802, WORD_LEN},                                               
	{CMD_WRITE, 0x8C8A, 0x1957, WORD_LEN},                                               
	{CMD_WRITE, 0x8C8C, 0x803C, WORD_LEN},                                               
	{CMD_WRITE, 0x8C8E, 0x1958, WORD_LEN},                                               
	{CMD_WRITE, 0x8C90, 0x803C, WORD_LEN},                                               
	{CMD_WRITE, 0x8C92, 0x1959, WORD_LEN},                                               
	{CMD_WRITE, 0x8C94, 0x803C, WORD_LEN},                                               
	{CMD_WRITE, 0x8C96, 0x195A, WORD_LEN},                                               
	{CMD_WRITE, 0x8C98, 0x803C, WORD_LEN},                                               
	{CMD_WRITE, 0x8C9A, 0x7EE0, WORD_LEN},                                               
	{CMD_WRITE, 0x8C9C, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8C9E, 0x0644, WORD_LEN},                                               
	{CMD_WRITE, 0x8CA0, 0x0000, WORD_LEN},
	{CMD_WRITE, 0x8CA2, 0xF978, WORD_LEN},                                               
	{CMD_WRITE, 0x8CA4, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CA6, 0x1268, WORD_LEN},                                               
	{CMD_WRITE, 0x8CA8, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CAA, 0x0D30, WORD_LEN},                                               
	{CMD_WRITE, 0x8CAC, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CAE, 0x0164, WORD_LEN},                                               
	{CMD_WRITE, 0x8CB0, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CB2, 0xF9AC, WORD_LEN},                                               
	{CMD_WRITE, 0x8CB4, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CB6, 0x0C54, WORD_LEN},                                               
	{CMD_WRITE, 0x8CB8, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CBA, 0x016C, WORD_LEN},                                               
	{CMD_WRITE, 0x8CBC, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CBE, 0x0194, WORD_LEN},                                               
	{CMD_WRITE, 0x8CC0, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CC2, 0x0F18, WORD_LEN},                                               
	{CMD_WRITE, 0x8CC4, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CC6, 0x0314, WORD_LEN},                                               
	{CMD_WRITE, 0x8CC8, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8CCA, 0x0694, WORD_LEN},
	{CMD_WRITE, 0x8CCC, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CCE, 0xF444, WORD_LEN},                                               
	{CMD_WRITE, 0x8CD0, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8CD2, 0x0DC4, WORD_LEN},                                               
	{CMD_WRITE, 0x8CD4, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CD6, 0x009C, WORD_LEN},                                               
	{CMD_WRITE, 0x8CD8, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8CDA, 0x0BE0, WORD_LEN},                                               
	{CMD_WRITE, 0x8CDC, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8CDE, 0x0B4C, WORD_LEN},                                               
	{CMD_WRITE, 0x8CE0, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CE2, 0x0C48, WORD_LEN},                                               
	{CMD_WRITE, 0x8CE4, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CE6, 0x0998, WORD_LEN},                                               
	{CMD_WRITE, 0x8CE8, 0x8000, WORD_LEN},
	{CMD_WRITE, 0x8CEA, 0x0008, WORD_LEN},
	{CMD_WRITE, 0x8CEC, 0x0000, WORD_LEN},
	{CMD_WRITE, 0x8CEE, 0xF3BC, WORD_LEN},                                               
	{CMD_WRITE, 0x8CF0, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8CF2, 0x1250, WORD_LEN},                                               
	{CMD_WRITE, 0x8CF4, 0x0000, WORD_LEN},
	{CMD_WRITE, 0x8CF6, 0x2F2C, WORD_LEN},                                               
	{CMD_WRITE, 0x8CF8, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8CFA, 0x0BA0, WORD_LEN},                                               
	{CMD_WRITE, 0x8CFC, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8CFE, 0x008C, WORD_LEN},                                               
	{CMD_WRITE, 0x8D00, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8D02, 0x06C8, WORD_LEN},                                               
	{CMD_WRITE, 0x8D04, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8D06, 0x0158, WORD_LEN},                                               
	{CMD_WRITE, 0x8D08, 0xFF80, WORD_LEN},
	{CMD_WRITE, 0x8D0A, 0x0290, WORD_LEN},                                               
	{CMD_WRITE, 0x8D0C, 0xFF00, WORD_LEN},                                               
	{CMD_WRITE, 0x8D0E, 0x0618, WORD_LEN},                                               
	{CMD_WRITE, 0x8D10, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8D12, 0x02CC, WORD_LEN},                                               
	{CMD_WRITE, 0x8D14, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8D16, 0xF1A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8D18, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8D1A, 0x12EC, WORD_LEN},                                               
	{CMD_WRITE, 0x8D1C, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8D1E, 0xF1B4, WORD_LEN},                                               
	{CMD_WRITE, 0x8D20, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x8D22, 0xF1C4, WORD_LEN},                                               
	{CMD_WRITE, 0x8D24, 0xFF00, WORD_LEN},                                               
	{CMD_WRITE, 0x8D26, 0x33CC, WORD_LEN},                                               
	{CMD_WRITE, 0x8D28, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8D2A, 0x0658, WORD_LEN},                                               
	{CMD_WRITE, 0x8D2C, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x8D2E, 0x0250, WORD_LEN},                                               
	{CMD_WRITE, 0x8D30, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8D32, 0x0D1E, WORD_LEN},                                               
	{CMD_WRITE, 0x8D34, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8D36, 0x1768, WORD_LEN},                                               
	{CMD_WRITE, 0x8D38, 0xF00D, WORD_LEN},                                               
	{CMD_WRITE, 0x8D3A, 0x8D2C, WORD_LEN},                                               
	{CMD_WRITE, 0x8D3C, 0x0985, WORD_LEN},                                               
	{CMD_WRITE, 0x8D3E, 0x0010, WORD_LEN},                                               
	{CMD_WRITE, 0x8D40, 0x17EC, WORD_LEN},                                               
	{CMD_WRITE, 0x8D42, 0xF002, WORD_LEN},                                               
	{CMD_WRITE, 0x8D44, 0x123A, WORD_LEN},                                               
	{CMD_WRITE, 0x8D46, 0x0083, WORD_LEN},                                               
	{CMD_WRITE, 0x8D48, 0x123B, WORD_LEN},                                               
	{CMD_WRITE, 0x8D4A, 0x008F, WORD_LEN},                                               
	{CMD_WRITE, 0x8D4C, 0x8D55, WORD_LEN},                                               
	{CMD_WRITE, 0x8D4E, 0xBB08, WORD_LEN},                                               
	{CMD_WRITE, 0x8D50, 0x7BE5, WORD_LEN},                                               
	{CMD_WRITE, 0x8D52, 0x8DF6, WORD_LEN},                                               
	{CMD_WRITE, 0x8D54, 0xBA08, WORD_LEN},                                               
	{CMD_WRITE, 0x8D56, 0x7AE5, WORD_LEN},                                               
	{CMD_WRITE, 0x8D58, 0x0B0D, WORD_LEN},                                               
	{CMD_WRITE, 0x8D5A, 0x00A3, WORD_LEN},                                               
	{CMD_WRITE, 0x8D5C, 0x8DC5, WORD_LEN},                                               
	{CMD_WRITE, 0x8D5E, 0x8D54, WORD_LEN},                                               
	{CMD_WRITE, 0x8D60, 0xAD45, WORD_LEN},                                               
	{CMD_WRITE, 0x8D62, 0xF026, WORD_LEN},                                               
	{CMD_WRITE, 0x8D64, 0x65DB, WORD_LEN},                                               
	{CMD_WRITE, 0x8D66, 0x8B6D, WORD_LEN},                                               
	{CMD_WRITE, 0x8D68, 0x0815, WORD_LEN},                                               
	{CMD_WRITE, 0x8D6A, 0x00E2, WORD_LEN},                                               
	{CMD_WRITE, 0x8D6C, 0x65DA, WORD_LEN},                                               
	{CMD_WRITE, 0x8D6E, 0x8A51, WORD_LEN},                                               
	{CMD_WRITE, 0x8D70, 0x0A0D, WORD_LEN},                                               
	{CMD_WRITE, 0x8D72, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x8D74, 0xE683, WORD_LEN},                                               
	{CMD_WRITE, 0x8D76, 0x22CA, WORD_LEN},                                               
	{CMD_WRITE, 0x8D78, 0x038B, WORD_LEN},                                               
	{CMD_WRITE, 0x8D7A, 0xF69A, WORD_LEN},                                               
	{CMD_WRITE, 0x8D7C, 0xDA00, WORD_LEN},                                               
	{CMD_WRITE, 0x8D7E, 0xAD45, WORD_LEN},                                               
	{CMD_WRITE, 0x8D80, 0x2540, WORD_LEN},                                               
	{CMD_WRITE, 0x8D82, 0x1343, WORD_LEN},                                               
	{CMD_WRITE, 0x8D84, 0xE180, WORD_LEN},                                               
	{CMD_WRITE, 0x8D86, 0x2540, WORD_LEN},                                               
	{CMD_WRITE, 0x8D88, 0x144F, WORD_LEN},                                               
	{CMD_WRITE, 0x8D8A, 0xF6D2, WORD_LEN},                                               
	{CMD_WRITE, 0x8D8C, 0x719F, WORD_LEN},                                               
	{CMD_WRITE, 0x8D8E, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8D90, 0x20A8, WORD_LEN},                                               
	{CMD_WRITE, 0x8D92, 0x0280, WORD_LEN},                                               
	{CMD_WRITE, 0x8D94, 0x8B20, WORD_LEN},                                               
	{CMD_WRITE, 0x8D96, 0x0813, WORD_LEN},                                               
	{CMD_WRITE, 0x8D98, 0x0043, WORD_LEN},                                               
	{CMD_WRITE, 0x8D9A, 0xE201, WORD_LEN},                                               
	{CMD_WRITE, 0x8D9C, 0x7A4F, WORD_LEN},                                               
	{CMD_WRITE, 0x8D9E, 0xAD45, WORD_LEN},                                               
	{CMD_WRITE, 0x8DA0, 0xE301, WORD_LEN},                                               
	{CMD_WRITE, 0x8DA2, 0xE701, WORD_LEN},                                               
	{CMD_WRITE, 0x8DA4, 0xF004, WORD_LEN},                                               
	{CMD_WRITE, 0x8DA6, 0x8F20, WORD_LEN},                                               
	{CMD_WRITE, 0x8DA8, 0x09F3, WORD_LEN},                                               
	{CMD_WRITE, 0x8DAA, 0x8002, WORD_LEN},                                               
	{CMD_WRITE, 0x8DAC, 0xD09F, WORD_LEN},                                               
	{CMD_WRITE, 0x8DAE, 0x8800, WORD_LEN},                                               
	{CMD_WRITE, 0x8DB0, 0xE803, WORD_LEN},                                               
	{CMD_WRITE, 0x8DB2, 0x0E11, WORD_LEN},                                               
	{CMD_WRITE, 0x8DB4, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8DB6, 0x17BE, WORD_LEN},                                               
	{CMD_WRITE, 0x8DB8, 0xF600, WORD_LEN},                                               
	{CMD_WRITE, 0x8DBA, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x8DBC, 0x8002, WORD_LEN},                                               
	{CMD_WRITE, 0x8DBE, 0x7840, WORD_LEN},                                               
	{CMD_WRITE, 0x8DC0, 0x04F5, WORD_LEN},                                               
	{CMD_WRITE, 0x8DC2, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8DC4, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8DC6, 0x0C8E, WORD_LEN},                                               
	{CMD_WRITE, 0x8DC8, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8DCA, 0x17BF, WORD_LEN},                                               
	{CMD_WRITE, 0x8DCC, 0xF60E, WORD_LEN},                                               
	{CMD_WRITE, 0x8DCE, 0x8E01, WORD_LEN},                                               
	{CMD_WRITE, 0x8DD0, 0xB8A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8DD2, 0xAE01, WORD_LEN},                                               
	{CMD_WRITE, 0x8DD4, 0x8E09, WORD_LEN},                                               
	{CMD_WRITE, 0x8DD6, 0xB8E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8DD8, 0xF29B, WORD_LEN},                                               
	{CMD_WRITE, 0x8DDA, 0x1754, WORD_LEN},                                               
	{CMD_WRITE, 0x8DDC, 0xF00D, WORD_LEN},                                               
	{CMD_WRITE, 0x8DDE, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x8DE0, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8DE2, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x8DE4, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8DE6, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8DE8, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8DEA, 0x16B8, WORD_LEN},                                               
	{CMD_WRITE, 0x8DEC, 0x1101, WORD_LEN},                                               
	{CMD_WRITE, 0x8DEE, 0x092D, WORD_LEN},                                               
	{CMD_WRITE, 0x8DF0, 0x0003, WORD_LEN},                                               
	{CMD_WRITE, 0x8DF2, 0x16B0, WORD_LEN},                                               
	{CMD_WRITE, 0x8DF4, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8DF6, 0x1E3C, WORD_LEN},                                               
	{CMD_WRITE, 0x8DF8, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8DFA, 0x16B1, WORD_LEN},                                               
	{CMD_WRITE, 0x8DFC, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8DFE, 0x1E3D, WORD_LEN},                                               
	{CMD_WRITE, 0x8E00, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8E02, 0x16B4, WORD_LEN},                                               
	{CMD_WRITE, 0x8E04, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8E06, 0x1E3E, WORD_LEN},                                               
	{CMD_WRITE, 0x8E08, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8E0A, 0x16B5, WORD_LEN},                                               
	{CMD_WRITE, 0x8E0C, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8E0E, 0x1E3F, WORD_LEN},                                               
	{CMD_WRITE, 0x8E10, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x8E12, 0x8E40, WORD_LEN},                                               
	{CMD_WRITE, 0x8E14, 0xBAA6, WORD_LEN},                                               
	{CMD_WRITE, 0x8E16, 0xAE40, WORD_LEN},                                               
	{CMD_WRITE, 0x8E18, 0x098F, WORD_LEN},                                               
	{CMD_WRITE, 0x8E1A, 0x0022, WORD_LEN},                                               
	{CMD_WRITE, 0x8E1C, 0x16BA, WORD_LEN},                                               
	{CMD_WRITE, 0x8E1E, 0x1102, WORD_LEN},                                               
	{CMD_WRITE, 0x8E20, 0x0A87, WORD_LEN},                                               
	{CMD_WRITE, 0x8E22, 0x0003, WORD_LEN},                                               
	{CMD_WRITE, 0x8E24, 0x16B2, WORD_LEN},                                               
	{CMD_WRITE, 0x8E26, 0x1084, WORD_LEN},                                               
	{CMD_WRITE, 0x8E28, 0x0DBE, WORD_LEN},                                               
	{CMD_WRITE, 0x8E2A, 0x0664, WORD_LEN},                                               
	{CMD_WRITE, 0x8E2C, 0x16B0, WORD_LEN},                                               
	{CMD_WRITE, 0x8E2E, 0x1083, WORD_LEN},                                               
	{CMD_WRITE, 0x8E30, 0x1E3C, WORD_LEN},                                               
	{CMD_WRITE, 0x8E32, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8E34, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x8E36, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8E38, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x8E3A, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8E3C, 0x16B3, WORD_LEN},                                               
	{CMD_WRITE, 0x8E3E, 0x1084, WORD_LEN},                                               
	{CMD_WRITE, 0x8E40, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8E42, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8E44, 0x16B8, WORD_LEN},                                               
	{CMD_WRITE, 0x8E46, 0x1101, WORD_LEN},                                               
	{CMD_WRITE, 0x8E48, 0x16BA, WORD_LEN},                                               
	{CMD_WRITE, 0x8E4A, 0x1102, WORD_LEN},                                               
	{CMD_WRITE, 0x8E4C, 0x0D9A, WORD_LEN},                                               
	{CMD_WRITE, 0x8E4E, 0x0664, WORD_LEN},                                               
	{CMD_WRITE, 0x8E50, 0x16B1, WORD_LEN},                                               
	{CMD_WRITE, 0x8E52, 0x1083, WORD_LEN},                                               
	{CMD_WRITE, 0x8E54, 0x1E3D, WORD_LEN},                                               
	{CMD_WRITE, 0x8E56, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8E58, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x8E5A, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8E5C, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x8E5E, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8E60, 0x16B6, WORD_LEN},                                               
	{CMD_WRITE, 0x8E62, 0x1084, WORD_LEN},                                               
	{CMD_WRITE, 0x8E64, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8E66, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8E68, 0x16B8, WORD_LEN},                                               
	{CMD_WRITE, 0x8E6A, 0x1101, WORD_LEN},                                               
	{CMD_WRITE, 0x8E6C, 0x16BA, WORD_LEN},                                               
	{CMD_WRITE, 0x8E6E, 0x1102, WORD_LEN},                                               
	{CMD_WRITE, 0x8E70, 0x0D76, WORD_LEN},                                               
	{CMD_WRITE, 0x8E72, 0x0664, WORD_LEN},                                               
	{CMD_WRITE, 0x8E74, 0x16B4, WORD_LEN},                                               
	{CMD_WRITE, 0x8E76, 0x1083, WORD_LEN},                                               
	{CMD_WRITE, 0x8E78, 0x1E3E, WORD_LEN},                                               
	{CMD_WRITE, 0x8E7A, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8E7C, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x8E7E, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8E80, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x8E82, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8E84, 0x16B7, WORD_LEN},                                               
	{CMD_WRITE, 0x8E86, 0x1084, WORD_LEN},                                               
	{CMD_WRITE, 0x8E88, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x8E8A, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x8E8C, 0x16B8, WORD_LEN},                                               
	{CMD_WRITE, 0x8E8E, 0x1101, WORD_LEN},                                               
	{CMD_WRITE, 0x8E90, 0x16BA, WORD_LEN},                                               
	{CMD_WRITE, 0x8E92, 0x1102, WORD_LEN},                                               
	{CMD_WRITE, 0x8E94, 0x0D52, WORD_LEN},                                               
	{CMD_WRITE, 0x8E96, 0x0664, WORD_LEN},                                               
	{CMD_WRITE, 0x8E98, 0x16B5, WORD_LEN},                                               
	{CMD_WRITE, 0x8E9A, 0x1083, WORD_LEN},                                               
	{CMD_WRITE, 0x8E9C, 0x1E3F, WORD_LEN},                                               
	{CMD_WRITE, 0x8E9E, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x8EA0, 0x8E00, WORD_LEN},                                               
	{CMD_WRITE, 0x8EA2, 0xB8A6, WORD_LEN},                                               
	{CMD_WRITE, 0x8EA4, 0xAE00, WORD_LEN},                                               
	{CMD_WRITE, 0x8EA6, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x8EA8, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8EAA, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x8EAC, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x8EAE, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8EB0, 0x7905, WORD_LEN},                                               
	{CMD_WRITE, 0x8EB2, 0x16BA, WORD_LEN},                                               
	{CMD_WRITE, 0x8EB4, 0x1100, WORD_LEN},                                               
	{CMD_WRITE, 0x8EB6, 0x085B, WORD_LEN},                                               
	{CMD_WRITE, 0x8EB8, 0x0042, WORD_LEN},                                               
	{CMD_WRITE, 0x8EBA, 0xD05D, WORD_LEN},                                               
	{CMD_WRITE, 0x8EBC, 0x9E31, WORD_LEN},                                               
	{CMD_WRITE, 0x8EBE, 0x904D, WORD_LEN},                                               
	{CMD_WRITE, 0x8EC0, 0x0A2B, WORD_LEN},                                               
	{CMD_WRITE, 0x8EC2, 0x0063, WORD_LEN},                                               
	{CMD_WRITE, 0x8EC4, 0x8E00, WORD_LEN},                                               
	{CMD_WRITE, 0x8EC6, 0x16B0, WORD_LEN},                                               
	{CMD_WRITE, 0x8EC8, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8ECA, 0x1E3C, WORD_LEN},                                               
	{CMD_WRITE, 0x8ECC, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8ECE, 0x16B1, WORD_LEN},                                               
	{CMD_WRITE, 0x8ED0, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8ED2, 0x1E3D, WORD_LEN},                                               
	{CMD_WRITE, 0x8ED4, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8ED6, 0x16B4, WORD_LEN},                                               
	{CMD_WRITE, 0x8ED8, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8EDA, 0x1E3E, WORD_LEN},                                               
	{CMD_WRITE, 0x8EDC, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8EDE, 0x16B5, WORD_LEN},                                               
	{CMD_WRITE, 0x8EE0, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8EE2, 0x1E3F, WORD_LEN},                                               
	{CMD_WRITE, 0x8EE4, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8EE6, 0xB886, WORD_LEN},                                               
	{CMD_WRITE, 0x8EE8, 0xF012, WORD_LEN},                                               
	{CMD_WRITE, 0x8EEA, 0x16B2, WORD_LEN},                                               
	{CMD_WRITE, 0x8EEC, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8EEE, 0xB8A6, WORD_LEN},                                               
	{CMD_WRITE, 0x8EF0, 0x1E3C, WORD_LEN},                                               
	{CMD_WRITE, 0x8EF2, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8EF4, 0x16B3, WORD_LEN},                                               
	{CMD_WRITE, 0x8EF6, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8EF8, 0x1E3D, WORD_LEN},                                               
	{CMD_WRITE, 0x8EFA, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8EFC, 0x16B6, WORD_LEN},                                               
	{CMD_WRITE, 0x8EFE, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F00, 0x1E3E, WORD_LEN},                                               
	{CMD_WRITE, 0x8F02, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8F04, 0x16B7, WORD_LEN},                                               
	{CMD_WRITE, 0x8F06, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F08, 0x1E3F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F0A, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x8F0C, 0xAE00, WORD_LEN},                                               
	{CMD_WRITE, 0x8F0E, 0x0C22, WORD_LEN},                                               
	{CMD_WRITE, 0x8F10, 0x0184, WORD_LEN},                                               
	{CMD_WRITE, 0x8F12, 0x03AD, WORD_LEN},                                               
	{CMD_WRITE, 0x8F14, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8F16, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x8F18, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x8F1A, 0x0B36, WORD_LEN},                                               
	{CMD_WRITE, 0x8F1C, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x8F1E, 0x7508, WORD_LEN},                                               
	{CMD_WRITE, 0x8F20, 0x087F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F22, 0x0111, WORD_LEN},                                               
	{CMD_WRITE, 0x8F24, 0xD643, WORD_LEN},                                               
	{CMD_WRITE, 0x8F26, 0x8E00, WORD_LEN},                                               
	{CMD_WRITE, 0x8F28, 0x0877, WORD_LEN},                                               
	{CMD_WRITE, 0x8F2A, 0x01DF, WORD_LEN},                                               
	{CMD_WRITE, 0x8F2C, 0x1780, WORD_LEN},                                               
	{CMD_WRITE, 0x8F2E, 0xF603, WORD_LEN},                                               
	{CMD_WRITE, 0x8F30, 0xB887, WORD_LEN},                                               
	{CMD_WRITE, 0x8F32, 0xAE00, WORD_LEN},                                               
	{CMD_WRITE, 0x8F34, 0x1334, WORD_LEN},                                               
	{CMD_WRITE, 0x8F36, 0x0081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F38, 0x1335, WORD_LEN},                                               
	{CMD_WRITE, 0x8F3A, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F3C, 0x1330, WORD_LEN},                                               
	{CMD_WRITE, 0x8F3E, 0x008F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F40, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F42, 0x7945, WORD_LEN},                                               
	{CMD_WRITE, 0x8F44, 0x1336, WORD_LEN},                                               
	{CMD_WRITE, 0x8F46, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F48, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F4A, 0xBF08, WORD_LEN},                                               
	{CMD_WRITE, 0x8F4C, 0x7945, WORD_LEN},                                               
	{CMD_WRITE, 0x8F4E, 0x1337, WORD_LEN},                                               
	{CMD_WRITE, 0x8F50, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F52, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F54, 0x7945, WORD_LEN},                                               
	{CMD_WRITE, 0x8F56, 0x1331, WORD_LEN},                                               
	{CMD_WRITE, 0x8F58, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F5A, 0x7F45, WORD_LEN},                                               
	{CMD_WRITE, 0x8F5C, 0x1332, WORD_LEN},                                               
	{CMD_WRITE, 0x8F5E, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F60, 0xBF08, WORD_LEN},                                               
	{CMD_WRITE, 0x8F62, 0x7F45, WORD_LEN},                                               
	{CMD_WRITE, 0x8F64, 0x1333, WORD_LEN},                                               
	{CMD_WRITE, 0x8F66, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8F68, 0xBF08, WORD_LEN},                                               
	{CMD_WRITE, 0x8F6A, 0x7F45, WORD_LEN},                                               
	{CMD_WRITE, 0x8F6C, 0x1761, WORD_LEN},                                               
	{CMD_WRITE, 0x8F6E, 0xF602, WORD_LEN},                                               
	{CMD_WRITE, 0x8F70, 0xA223, WORD_LEN},                                               
	{CMD_WRITE, 0x8F72, 0x133A, WORD_LEN},                                               
	{CMD_WRITE, 0x8F74, 0x0081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F76, 0xA2E2, WORD_LEN},                                               
	{CMD_WRITE, 0x8F78, 0x133B, WORD_LEN},                                               
	{CMD_WRITE, 0x8F7A, 0x008F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F7C, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F7E, 0x79E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8F80, 0xB228, WORD_LEN},                                               
	{CMD_WRITE, 0x8F82, 0x132A, WORD_LEN},                                               
	{CMD_WRITE, 0x8F84, 0x0081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F86, 0x132B, WORD_LEN},                                               
	{CMD_WRITE, 0x8F88, 0x008F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F8A, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F8C, 0x79E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8F8E, 0xB229, WORD_LEN},                                               
	{CMD_WRITE, 0x8F90, 0x1328, WORD_LEN},                                               
	{CMD_WRITE, 0x8F92, 0x0081, WORD_LEN},                                               
	{CMD_WRITE, 0x8F94, 0x1329, WORD_LEN},                                               
	{CMD_WRITE, 0x8F96, 0x008F, WORD_LEN},                                               
	{CMD_WRITE, 0x8F98, 0xB908, WORD_LEN},                                               
	{CMD_WRITE, 0x8F9A, 0x79E5, WORD_LEN},                                               
	{CMD_WRITE, 0x8F9C, 0xB22A, WORD_LEN},                                               
	{CMD_WRITE, 0x8F9E, 0x0F66, WORD_LEN},                                               
	{CMD_WRITE, 0x8FA0, 0x05A4, WORD_LEN},                                               
	{CMD_WRITE, 0x8FA2, 0x70A9, WORD_LEN},                                               
	{CMD_WRITE, 0x8FA4, 0x0D7F, WORD_LEN},                                               
	{CMD_WRITE, 0x8FA6, 0x1091, WORD_LEN},                                               
	{CMD_WRITE, 0x8FA8, 0xD522, WORD_LEN},                                               
	{CMD_WRITE, 0x8FAA, 0x8D00, WORD_LEN},                                               
	{CMD_WRITE, 0x8FAC, 0x0877, WORD_LEN},                                               
	{CMD_WRITE, 0x8FAE, 0x01DE, WORD_LEN},                                               
	{CMD_WRITE, 0x8FB0, 0x1750, WORD_LEN},                                               
	{CMD_WRITE, 0x8FB2, 0xF603, WORD_LEN},                                               
	{CMD_WRITE, 0x8FB4, 0x175E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FB6, 0xF601, WORD_LEN},                                               
	{CMD_WRITE, 0x8FB8, 0x83E3, WORD_LEN},                                               
	{CMD_WRITE, 0x8FBA, 0x8342, WORD_LEN},                                               
	{CMD_WRITE, 0x8FBC, 0x2F41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FBE, 0x160E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FC0, 0x1934, WORD_LEN},                                               
	{CMD_WRITE, 0x8FC2, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FC4, 0x2F41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FC6, 0x140E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FC8, 0x1935, WORD_LEN},                                               
	{CMD_WRITE, 0x8FCA, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FCC, 0x2F41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FCE, 0x120E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FD0, 0x1936, WORD_LEN},                                               
	{CMD_WRITE, 0x8FD2, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FD4, 0x2A41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FD6, 0x060E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FD8, 0x1930, WORD_LEN},                                               
	{CMD_WRITE, 0x8FDA, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FDC, 0x2A41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FDE, 0x040E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FE0, 0x1931, WORD_LEN},                                               
	{CMD_WRITE, 0x8FE2, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FE4, 0x2A41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FE6, 0x020E, WORD_LEN},                                               
	{CMD_WRITE, 0x8FE8, 0x1932, WORD_LEN},                                               
	{CMD_WRITE, 0x8FEA, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FEC, 0x93C8, WORD_LEN},                                               
	{CMD_WRITE, 0x8FEE, 0x1933, WORD_LEN},                                               
	{CMD_WRITE, 0x8FF0, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8FF2, 0x2E41, WORD_LEN},                                               
	{CMD_WRITE, 0x8FF4, 0x1202, WORD_LEN},                                               
	{CMD_WRITE, 0x8FF6, 0x193A, WORD_LEN},                                               
	{CMD_WRITE, 0x8FF8, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x8FFA, 0x193B, WORD_LEN},                                               
	{CMD_WRITE, 0x8FFC, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x8FFE, 0x93C9, WORD_LEN},                                               
	{CMD_WRITE, 0x9000, 0x1937, WORD_LEN},                                               
	{CMD_WRITE, 0x9002, 0x03C2, WORD_LEN},                                               
	{CMD_WRITE, 0x9004, 0x2E41, WORD_LEN},                                               
	{CMD_WRITE, 0x9006, 0x1202, WORD_LEN},                                               
	{CMD_WRITE, 0x9008, 0x192A, WORD_LEN},                                               
	{CMD_WRITE, 0x900A, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x900C, 0x934A, WORD_LEN},                                               
	{CMD_WRITE, 0x900E, 0x192B, WORD_LEN},                                               
	{CMD_WRITE, 0x9010, 0x0382, WORD_LEN},                                               
	{CMD_WRITE, 0x9012, 0xB8A7, WORD_LEN},                                               
	{CMD_WRITE, 0x9014, 0xAD00, WORD_LEN},                                               
	{CMD_WRITE, 0x9016, 0x2A41, WORD_LEN},                                               
	{CMD_WRITE, 0x9018, 0x020C, WORD_LEN},                                               
	{CMD_WRITE, 0x901A, 0x1928, WORD_LEN},                                               
	{CMD_WRITE, 0x901C, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x901E, 0x1929, WORD_LEN},                                               
	{CMD_WRITE, 0x9020, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x9022, 0x0295, WORD_LEN},                                               
	{CMD_WRITE, 0x9024, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x9026, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x9028, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x902A, 0x0168, WORD_LEN},                                               
	{CMD_WRITE, 0x902C, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x902E, 0x050C, WORD_LEN},                                               
	{CMD_WRITE, 0x9030, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x9032, 0x0828, WORD_LEN},                                               
	{CMD_WRITE, 0x9034, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x9036, 0x0986, WORD_LEN},                                               
	{CMD_WRITE, 0x9038, 0x0224, WORD_LEN},                                               
	{CMD_WRITE, 0x903A, 0xC5E1, WORD_LEN},                                               
	{CMD_WRITE, 0x903C, 0xD067, WORD_LEN},                                               
	{CMD_WRITE, 0x903E, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x9040, 0x880A, WORD_LEN},                                               
	{CMD_WRITE, 0x9042, 0x085B, WORD_LEN},                                               
	{CMD_WRITE, 0x9044, 0x019F, WORD_LEN},                                               
	{CMD_WRITE, 0x9046, 0xD166, WORD_LEN},                                               
	{CMD_WRITE, 0x9048, 0xD800, WORD_LEN},                                               
	{CMD_WRITE, 0x904A, 0xA90F, WORD_LEN},                                               
	{CMD_WRITE, 0x904C, 0xD165, WORD_LEN},                                               
	{CMD_WRITE, 0x904E, 0x81A1, WORD_LEN},                                               
	{CMD_WRITE, 0x9050, 0x8160, WORD_LEN},                                               
	{CMD_WRITE, 0x9052, 0xD165, WORD_LEN},                                               
	{CMD_WRITE, 0x9054, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x9056, 0x1602, WORD_LEN},                                               
	{CMD_WRITE, 0x9058, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x905A, 0x060C, WORD_LEN},                                               
	{CMD_WRITE, 0x905C, 0x1934, WORD_LEN},                                               
	{CMD_WRITE, 0x905E, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x9060, 0x1930, WORD_LEN},                                               
	{CMD_WRITE, 0x9062, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x9064, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x9066, 0x1402, WORD_LEN},                                               
	{CMD_WRITE, 0x9068, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x906A, 0x040C, WORD_LEN},                                               
	{CMD_WRITE, 0x906C, 0x1935, WORD_LEN},                                               
	{CMD_WRITE, 0x906E, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x9070, 0x1931, WORD_LEN},                                               
	{CMD_WRITE, 0x9072, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x9074, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x9076, 0x1202, WORD_LEN},                                               
	{CMD_WRITE, 0x9078, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x907A, 0x020C, WORD_LEN},                                               
	{CMD_WRITE, 0x907C, 0x1936, WORD_LEN},                                               
	{CMD_WRITE, 0x907E, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x9080, 0x1932, WORD_LEN},                                               
	{CMD_WRITE, 0x9082, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x9084, 0x1937, WORD_LEN},                                               
	{CMD_WRITE, 0x9086, 0x0342, WORD_LEN},                                               
	{CMD_WRITE, 0x9088, 0x1933, WORD_LEN},                                               
	{CMD_WRITE, 0x908A, 0x00C2, WORD_LEN},                                               
	{CMD_WRITE, 0x908C, 0xD157, WORD_LEN},                                               
	{CMD_WRITE, 0x908E, 0x8120, WORD_LEN},                                               
	{CMD_WRITE, 0x9090, 0x7208, WORD_LEN},                                               
	{CMD_WRITE, 0x9092, 0x81A8, WORD_LEN},                                               
	{CMD_WRITE, 0x9094, 0x7108, WORD_LEN},                                               
	{CMD_WRITE, 0x9096, 0x7D60, WORD_LEN},                                               
	{CMD_WRITE, 0x9098, 0x7308, WORD_LEN},                                               
	{CMD_WRITE, 0x909A, 0x022D, WORD_LEN},                                               
	{CMD_WRITE, 0x909C, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x909E, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x90A0, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x90A2, 0x08BE, WORD_LEN},                                               
	{CMD_WRITE, 0x90A4, 0x0224, WORD_LEN},                                               
	{CMD_WRITE, 0x90A6, 0xC5E1, WORD_LEN},                                               
	{CMD_WRITE, 0x90A8, 0xD14D, WORD_LEN},                                               
	{CMD_WRITE, 0x90AA, 0x894D, WORD_LEN},                                               
	{CMD_WRITE, 0x90AC, 0x0A63, WORD_LEN},                                               
	{CMD_WRITE, 0x90AE, 0x0002, WORD_LEN},                                               
	{CMD_WRITE, 0x90B0, 0xD04A, WORD_LEN},                                               
	{CMD_WRITE, 0x90B2, 0x8040, WORD_LEN},                                               
	{CMD_WRITE, 0x90B4, 0x8A0B, WORD_LEN},                                               
	{CMD_WRITE, 0x90B6, 0xB8A6, WORD_LEN},                                               
	{CMD_WRITE, 0x90B8, 0xAA0B, WORD_LEN},                                               
	{CMD_WRITE, 0x90BA, 0xD800, WORD_LEN},                                               
	{CMD_WRITE, 0x90BC, 0xA90F, WORD_LEN},                                               
	{CMD_WRITE, 0x90BE, 0xD149, WORD_LEN},                                               
	{CMD_WRITE, 0x90C0, 0x8161, WORD_LEN},                                               
	{CMD_WRITE, 0x90C2, 0x81A0, WORD_LEN},                                               
	{CMD_WRITE, 0x90C4, 0xD148, WORD_LEN},                                               
	{CMD_WRITE, 0x90C6, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x90C8, 0x0602, WORD_LEN},                                               
	{CMD_WRITE, 0x90CA, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x90CC, 0x160C, WORD_LEN},                                               
	{CMD_WRITE, 0x90CE, 0x1934, WORD_LEN},                                               
	{CMD_WRITE, 0x90D0, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x90D2, 0x1930, WORD_LEN},                                               
	{CMD_WRITE, 0x90D4, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x90D6, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x90D8, 0x0402, WORD_LEN},                                               
	{CMD_WRITE, 0x90DA, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x90DC, 0x140C, WORD_LEN},                                               
	{CMD_WRITE, 0x90DE, 0x1935, WORD_LEN},                                               
	{CMD_WRITE, 0x90E0, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x90E2, 0x1931, WORD_LEN},                                               
	{CMD_WRITE, 0x90E4, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x90E6, 0x2B41, WORD_LEN},                                               
	{CMD_WRITE, 0x90E8, 0x0202, WORD_LEN},                                               
	{CMD_WRITE, 0x90EA, 0x2D41, WORD_LEN},                                               
	{CMD_WRITE, 0x90EC, 0x120C, WORD_LEN},                                               
	{CMD_WRITE, 0x90EE, 0x1936, WORD_LEN},                                               
	{CMD_WRITE, 0x90F0, 0x0082, WORD_LEN},                                               
	{CMD_WRITE, 0x90F2, 0x1932, WORD_LEN},                                               
	{CMD_WRITE, 0x90F4, 0x0302, WORD_LEN},                                               
	{CMD_WRITE, 0x90F6, 0x1937, WORD_LEN},                                               
	{CMD_WRITE, 0x90F8, 0x00C2, WORD_LEN},                                               
	{CMD_WRITE, 0x90FA, 0x1933, WORD_LEN},                                               
	{CMD_WRITE, 0x90FC, 0x0342, WORD_LEN},                                               
	{CMD_WRITE, 0x90FE, 0xD13B, WORD_LEN},                                               
	{CMD_WRITE, 0x9100, 0x8120, WORD_LEN},                                               
	{CMD_WRITE, 0x9102, 0x7208, WORD_LEN},                                               
	{CMD_WRITE, 0x9104, 0x81A8, WORD_LEN},                                               
	{CMD_WRITE, 0x9106, 0x7108, WORD_LEN},                                               
	{CMD_WRITE, 0x9108, 0x7D60, WORD_LEN},                                               
	{CMD_WRITE, 0x910A, 0x7308, WORD_LEN},                                               
	{CMD_WRITE, 0x910C, 0xF003, WORD_LEN},                                               
	{CMD_WRITE, 0x910E, 0x0B3E, WORD_LEN},                                               
	{CMD_WRITE, 0x9110, 0x0204, WORD_LEN},                                               
	{CMD_WRITE, 0x9112, 0x01B5, WORD_LEN},                                               
	{CMD_WRITE, 0x9114, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x9116, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x9118, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x911A, 0x092E, WORD_LEN},                                               
	{CMD_WRITE, 0x911C, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x911E, 0xD532, WORD_LEN},                                               
	{CMD_WRITE, 0x9120, 0xD733, WORD_LEN},                                               
	{CMD_WRITE, 0x9122, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x9124, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x9126, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x9128, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x912A, 0x1528, WORD_LEN},                                               
	{CMD_WRITE, 0x912C, 0x108E, WORD_LEN},                                               
	{CMD_WRITE, 0x912E, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x9130, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x9132, 0x703A, WORD_LEN},                                               
	{CMD_WRITE, 0x9134, 0x152A, WORD_LEN},                                               
	{CMD_WRITE, 0x9136, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x9138, 0x152B, WORD_LEN},                                               
	{CMD_WRITE, 0x913A, 0x1081, WORD_LEN},                                               
	{CMD_WRITE, 0x913C, 0xBE08, WORD_LEN},                                               
	{CMD_WRITE, 0x913E, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x9140, 0x7825, WORD_LEN},                                               
	{CMD_WRITE, 0x9142, 0x701A, WORD_LEN},                                               
	{CMD_WRITE, 0x9144, 0x1529, WORD_LEN},                                               
	{CMD_WRITE, 0x9146, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x9148, 0x7E05, WORD_LEN},                                               
	{CMD_WRITE, 0x914A, 0x8700, WORD_LEN},                                               
	{CMD_WRITE, 0x914C, 0x800C, WORD_LEN},                                               
	{CMD_WRITE, 0x914E, 0x7840, WORD_LEN},                                               
	{CMD_WRITE, 0x9150, 0xD028, WORD_LEN},                                               
	{CMD_WRITE, 0x9152, 0x153B, WORD_LEN},                                               
	{CMD_WRITE, 0x9154, 0x1082, WORD_LEN},                                               
	{CMD_WRITE, 0x9156, 0x9039, WORD_LEN},                                               
	{CMD_WRITE, 0x9158, 0x153A, WORD_LEN},                                               
	{CMD_WRITE, 0x915A, 0x1080, WORD_LEN},                                               
	{CMD_WRITE, 0x915C, 0xB808, WORD_LEN},                                               
	{CMD_WRITE, 0x915E, 0x7845, WORD_LEN},                                               
	{CMD_WRITE, 0x9160, 0x7822, WORD_LEN},                                               
	{CMD_WRITE, 0x9162, 0xD11E, WORD_LEN},                                               
	{CMD_WRITE, 0x9164, 0x8120, WORD_LEN},                                               
	{CMD_WRITE, 0x9166, 0x912D, WORD_LEN},                                               
	{CMD_WRITE, 0x9168, 0x082F, WORD_LEN},                                               
	{CMD_WRITE, 0x916A, 0x0043, WORD_LEN},                                               
	{CMD_WRITE, 0x916C, 0x2941, WORD_LEN},                                               
	{CMD_WRITE, 0x916E, 0x2201, WORD_LEN},                                               
	{CMD_WRITE, 0x9170, 0x1D3A, WORD_LEN},                                               
	{CMD_WRITE, 0x9172, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x9174, 0x2841, WORD_LEN},                                               
	{CMD_WRITE, 0x9176, 0x2201, WORD_LEN},                                               
	{CMD_WRITE, 0x9178, 0x1D2A, WORD_LEN},                                               
	{CMD_WRITE, 0x917A, 0x1042, WORD_LEN},                                               
	{CMD_WRITE, 0x917C, 0x2E41, WORD_LEN},                                               
	{CMD_WRITE, 0x917E, 0x1200, WORD_LEN},                                               
	{CMD_WRITE, 0x9180, 0x1D28, WORD_LEN},                                               
	{CMD_WRITE, 0x9182, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x9184, 0x8700, WORD_LEN},                                               
	{CMD_WRITE, 0x9186, 0x1D2B, WORD_LEN},                                               
	{CMD_WRITE, 0x9188, 0x1402, WORD_LEN},                                               
	{CMD_WRITE, 0x918A, 0x800D, WORD_LEN},                                               
	{CMD_WRITE, 0x918C, 0x1D3B, WORD_LEN},                                               
	{CMD_WRITE, 0x918E, 0x1442, WORD_LEN},                                               
	{CMD_WRITE, 0x9190, 0x7860, WORD_LEN},                                               
	{CMD_WRITE, 0x9192, 0x1D29, WORD_LEN},                                               
	{CMD_WRITE, 0x9194, 0x1382, WORD_LEN},                                               
	{CMD_WRITE, 0x9196, 0x0111, WORD_LEN},                                               
	{CMD_WRITE, 0x9198, 0x0684, WORD_LEN},                                               
	{CMD_WRITE, 0x919A, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x919C, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x919E, 0x0946, WORD_LEN},                                               
	{CMD_WRITE, 0x91A0, 0x01C4, WORD_LEN},                                               
	{CMD_WRITE, 0x91A2, 0xD215, WORD_LEN},                                               
	{CMD_WRITE, 0x91A4, 0xD112, WORD_LEN},                                               
	{CMD_WRITE, 0x91A6, 0xA140, WORD_LEN},                                               
	{CMD_WRITE, 0x91A8, 0xC0D1, WORD_LEN},                                               
	{CMD_WRITE, 0x91AA, 0x7EE0, WORD_LEN},                                               
	{CMD_WRITE, 0x91AC, 0xC0F1, WORD_LEN},                                               
	{CMD_WRITE, 0x91AE, 0x08A6, WORD_LEN},                                               
	{CMD_WRITE, 0x91B0, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x91B2, 0xDA44, WORD_LEN},                                               
	{CMD_WRITE, 0x91B4, 0xD111, WORD_LEN},                                               
	{CMD_WRITE, 0x91B6, 0xD512, WORD_LEN},                                               
	{CMD_WRITE, 0x91B8, 0x76A9, WORD_LEN},                                               
	{CMD_WRITE, 0x91BA, 0x0E3A, WORD_LEN},                                               
	{CMD_WRITE, 0x91BC, 0x0664, WORD_LEN},                                               
	{CMD_WRITE, 0x91BE, 0x70C9, WORD_LEN},                                               
	{CMD_WRITE, 0x91C0, 0xD010, WORD_LEN},                                               
	{CMD_WRITE, 0x91C2, 0xA50D, WORD_LEN},                                               
	{CMD_WRITE, 0x91C4, 0xD010, WORD_LEN},                                               
	{CMD_WRITE, 0x91C6, 0xA50C, WORD_LEN},                                               
	{CMD_WRITE, 0x91C8, 0xD010, WORD_LEN},                                               
	{CMD_WRITE, 0x91CA, 0xA510, WORD_LEN},                                               
	{CMD_WRITE, 0x91CC, 0xD010, WORD_LEN},                                               
	{CMD_WRITE, 0x91CE, 0xA500, WORD_LEN},                                               
	{CMD_WRITE, 0x91D0, 0xD007, WORD_LEN},                                               
	{CMD_WRITE, 0x91D2, 0x00ED, WORD_LEN},                                               
	{CMD_WRITE, 0x91D4, 0x06A4, WORD_LEN},                                               
	{CMD_WRITE, 0x91D6, 0xA0C0, WORD_LEN},                                               
	{CMD_WRITE, 0x91D8, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x91DA, 0x000C, WORD_LEN},                                               
	{CMD_WRITE, 0x91DC, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x91DE, 0x040C, WORD_LEN},                                               
	{CMD_WRITE, 0x91E0, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x91E2, 0x00A8, WORD_LEN},                                               
	{CMD_WRITE, 0x91E4, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x91E6, 0x0250, WORD_LEN},                                               
	{CMD_WRITE, 0x91E8, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x91EA, 0x0038, WORD_LEN},                                               
	{CMD_WRITE, 0x91EC, 0x8000, WORD_LEN},                                               
	{CMD_WRITE, 0x91EE, 0x00A4, WORD_LEN},                                               
	{CMD_WRITE, 0x91F0, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x91F2, 0x0740, WORD_LEN},                                               
	{CMD_WRITE, 0x91F4, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x91F6, 0x1368, WORD_LEN},                                               
	{CMD_WRITE, 0x91F8, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x91FA, 0xF4BC, WORD_LEN},                                               
	{CMD_WRITE, 0x91FC, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x91FE, 0x1368, WORD_LEN},                                               
	{CMD_WRITE, 0x9200, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x9202, 0x10A0, WORD_LEN},                                               
	{CMD_WRITE, 0x9204, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x9206, 0x1034, WORD_LEN},                                               
	{CMD_WRITE, 0x9208, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x920A, 0x1118, WORD_LEN},                                               
	{CMD_WRITE, 0x920C, 0xFF80, WORD_LEN},                                               
	{CMD_WRITE, 0x920E, 0x119C, WORD_LEN},                                               
	{CMD_WRITE, 0x9210, 0xE280, WORD_LEN},                                               
	{CMD_WRITE, 0x9212, 0x24CA, WORD_LEN},                                               
	{CMD_WRITE, 0x9214, 0x7082, WORD_LEN},                                               
	{CMD_WRITE, 0x9216, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x9218, 0x20E8, WORD_LEN},                                               
	{CMD_WRITE, 0x921A, 0x01A2, WORD_LEN},                                               
	{CMD_WRITE, 0x921C, 0x1002, WORD_LEN},                                               
	{CMD_WRITE, 0x921E, 0x0D02, WORD_LEN},                                               
	{CMD_WRITE, 0x9220, 0x1902, WORD_LEN},                                               
	{CMD_WRITE, 0x9222, 0x0094, WORD_LEN},                                               
	{CMD_WRITE, 0x9224, 0x7FE0, WORD_LEN},                                               
	{CMD_WRITE, 0x9226, 0x7028, WORD_LEN},                                               
	{CMD_WRITE, 0x9228, 0x7308, WORD_LEN},                                               
	{CMD_WRITE, 0x922A, 0x1000, WORD_LEN},                                               
	{CMD_WRITE, 0x922C, 0x0900, WORD_LEN},                                               
	{CMD_WRITE, 0x922E, 0x7904, WORD_LEN},                                               
	{CMD_WRITE, 0x9230, 0x7947, WORD_LEN},                                               
	{CMD_WRITE, 0x9232, 0x1B00, WORD_LEN},                                               
	{CMD_WRITE, 0x9234, 0x0064, WORD_LEN},                                               
	{CMD_WRITE, 0x9236, 0x7EE0, WORD_LEN},                                               
	{CMD_WRITE, 0x9238, 0xE280, WORD_LEN},                                               
	{CMD_WRITE, 0x923A, 0x24CA, WORD_LEN},                                               
	{CMD_WRITE, 0x923C, 0x7082, WORD_LEN},                                               
	{CMD_WRITE, 0x923E, 0x78E0, WORD_LEN},                                               
	{CMD_WRITE, 0x9240, 0x20E8, WORD_LEN},                                               
	{CMD_WRITE, 0x9242, 0x01A2, WORD_LEN},                                               
	{CMD_WRITE, 0x9244, 0x1102, WORD_LEN},                                               
	{CMD_WRITE, 0x9246, 0x0502, WORD_LEN},                                               
	{CMD_WRITE, 0x9248, 0x1802, WORD_LEN},                                               
	{CMD_WRITE, 0x924A, 0x00B4, WORD_LEN},                                               
	{CMD_WRITE, 0x924C, 0x7FE0, WORD_LEN},                                               
	{CMD_WRITE, 0x924E, 0x7028, WORD_LEN},                                               
	{CMD_WRITE, 0x9250, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9252, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9254, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9256, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9258, 0x0000, WORD_LEN},                                               
	{CMD_WRITE_BURST_E, 0x925A, 0x0000, WORD_LEN}, 
	{CMD_WRITE, 0x925C, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x925E, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9260, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9262, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9264, 0x0000, WORD_LEN},                                               
	{CMD_WRITE, 0x9266, 0x0000, WORD_LEN}, 
	// return variable access to logical mode
	{CMD_WRITE, 0x098E, 0x0000, WORD_LEN},  	// LOGICAL_ADDRESS_ACCESS
	// execute patch
	{CMD_WRITE, 0x8016, 0x086C, WORD_LEN},
	{CMD_WRITE, 0x8002, 0x0001, WORD_LEN}, 
	// test for patch load complete
	{CMD_POLL, 0x8002, 0x00, 100},  //Wait for the core ready
	//char_settings //tuning_reg_settings_array
	{CMD_WRITE, 0x316E, 0xC400, WORD_LEN},  	// DAC_ECL          
	{CMD_WRITE, 0x3E2E, 0x00FF, WORD_LEN},  	// SAMP_SPARE       
	{CMD_WRITE, 0x316C, 0xB44F, WORD_LEN},  	// DAC_TXLO         
	{CMD_WRITE, 0x316E, 0xC6FB, WORD_LEN},  	// DAC_ECL          
	{CMD_WRITE, 0x3ED4, 0x00A3, WORD_LEN},  	// DAC_LD_8_9       
	{CMD_WRITE, 0x3EE6, 0xA541, WORD_LEN},  	// DAC_LD_26_27     
	{CMD_WRITE, 0x31E0, 0x0000, WORD_LEN},  	// PIX_DEF_ID       
	{CMD_WRITE, 0x3EE0, 0x4910, WORD_LEN},  	// DAC_LD_20_21     
	{CMD_WRITE, 0x3EE2, 0x09CF, WORD_LEN},  	// DAC_LD_22_23     
	{CMD_WRITE, 0x30B6, 0x0008, WORD_LEN},  	// AUTOLR_CONTROL   
	{CMD_WRITE, 0x3E1A, 0xA582, WORD_LEN},  	// SAMP_TX_BOOST    
	{CMD_WRITE, 0x3E2E, 0xEC05, WORD_LEN},  	// SAMP_SPARE       
	{CMD_WRITE, 0x3EE6, 0xA5C0, WORD_LEN},  	// DAC_LD_26_27     
	{CMD_WRITE, 0x316C, 0xF43F, WORD_LEN},  	// DAC_TXLO 

	{CMD_WRITE, 0xE02A, 0x0001, WORD_LEN},
	{CMD_DELAY, 0x00, 50, 0},
	{CMD_WRITE, 0x3812, 0x2124, WORD_LEN},
};

static struct mt9p111_register_pair const init2_mode_settings_array[] =
{			 										
	{CMD_WRITE, 0x3210, 0x49B8, WORD_LEN},
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},
	{CMD_WRITE, 0xAC02, 0x00FF, WORD_LEN},                                   
	{CMD_WRITE, 0xAC01, 0xEF, BYTE_LEN},  	// AWB_MODE                                   
	{CMD_WRITE, 0xAC3C, 0x26, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3D, 0x6E, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3E, 0x14, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC3F, 0x6F, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC40, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC41, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC42, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xAC43, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xACB0, 0x2F, BYTE_LEN},  	// AWB_RG_MIN                                 
	{CMD_WRITE, 0xACB1, 0x5F, BYTE_LEN},  	// AWB_RG_MAX                                 
	{CMD_WRITE, 0xACB4, 0x1B, BYTE_LEN},  	// AWB_BG_MIN                                 
	{CMD_WRITE, 0xACB5, 0x5B, BYTE_LEN},  	// AWB_BG_MAX                                 
	{CMD_WRITE, 0xACB2, 0x40, BYTE_LEN},  	// 41AWB_RG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB3, 0x46, BYTE_LEN},  	// AWB_RG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB6, 0x36, BYTE_LEN},  	// AWB_BG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB7, 0x3f, BYTE_LEN},  	// AWB_BG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB8, 0x0100, WORD_LEN},  // 100 AWB_START_NUM_INT_LINES                    
	{CMD_WRITE, 0xACBA, 0x00B0, WORD_LEN},  	// AWB_END_NUM_INT_LINES                      
	{CMD_WRITE, 0xAC09, 0x01, BYTE_LEN},  	// AWB_UNUSED1                                
	{CMD_WRITE, 0xAC22, 0x0000, WORD_LEN},  // AWB_LEFT_CCM_0	                              
	{CMD_WRITE, 0xAC46, 0x0233, WORD_LEN},  	// AWB_LEFT_CCM_0                             
	{CMD_WRITE, 0xAC48, 0xFEDA, WORD_LEN}, 	// AWB_LEFT_CCM_1                             
	{CMD_WRITE, 0xAC4A, 0xFFF2, WORD_LEN}, 	// AWB_LEFT_CCM_2                           
	{CMD_WRITE, 0xAC4C, 0xFF83, WORD_LEN}, 	// AWB_LEFT_CCM_3                             
	{CMD_WRITE, 0xAC4E, 0x019B, WORD_LEN}, 	// AWB_LEFT_CCM_4                             
	{CMD_WRITE, 0xAC50, 0xFFE0, WORD_LEN}, 	// AWB_LEFT_CCM_5                             
	{CMD_WRITE, 0xAC52, 0xFFE6, WORD_LEN}, 	// AWB_LEFT_CCM_6                             
	{CMD_WRITE, 0xAC54, 0xFEBF, WORD_LEN}, 	// AWB_LEFT_CCM_7                             
	{CMD_WRITE, 0xAC56, 0x025C, WORD_LEN}, 	// AWB_LEFT_CCM_8                             
	{CMD_WRITE, 0xAC58, 0x0091, WORD_LEN}, 	// AWB_LEFT_CCM_R2BRATIO                      
	{CMD_WRITE, 0xAC5C, 0x02B8, WORD_LEN}, 	// AWB_RIGHT_CCM_0                            
	{CMD_WRITE, 0xAC5E, 0xFEA7, WORD_LEN}, 	// AWB_RIGHT_CCM_1                            
	{CMD_WRITE, 0xAC60, 0xFF92, WORD_LEN}, 	// AWB_RIGHT_CCM_2                            
	{CMD_WRITE, 0xAC62, 0xFFE0, WORD_LEN}, 	// AWB_RIGHT_CCM_3                            
	{CMD_WRITE, 0xAC64, 0x0136, WORD_LEN}, 	// AWB_RIGHT_CCM_4                            
	{CMD_WRITE, 0xAC66, 0xFFEE, WORD_LEN}, 	// AWB_RIGHT_CCM_5                            
	{CMD_WRITE, 0xAC68, 0x002A, WORD_LEN}, 	// AWB_RIGHT_CCM_6                            
	{CMD_WRITE, 0xAC6A, 0xFF2C, WORD_LEN}, 	// AWB_RIGHT_CCM_7                            
	{CMD_WRITE, 0xAC6C, 0x01A5, WORD_LEN}, 	// AWB_RIGHT_CCM_8                  
	{CMD_WRITE, 0xAC6E, 0x0066, WORD_LEN}, 	// AWB_RIGHT_CCM_R2BRATIO           
	{CMD_WRITE, 0xB83F, 0x00, BYTE_LEN},  	// STAT_AWB_WINDOW_POS_Y            
	{CMD_WRITE, 0xB840, 0xFF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_X           
	{CMD_WRITE, 0xB841, 0xEF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_Y           
	{CMD_WRITE, 0xB842, 0x0030, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_X   
	{CMD_WRITE, 0xB844, 0x002b, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_Y   
	{CMD_WRITE, 0x3240, 0x0024, WORD_LEN}, 	// 34AWB_XY_SCALE                     
	{CMD_WRITE, 0x3244, 0xAA80, WORD_LEN}, 	// AWB_WEIGHT_R1                    
	{CMD_WRITE, 0x3246, 0x5580, WORD_LEN}, 	// AWB_WEIGHT_R2                    
	{CMD_WRITE, 0x3248, 0x3700, WORD_LEN}, 	// AWB_WEIGHT_R3                    
	{CMD_WRITE, 0x324A, 0x39C0, WORD_LEN}, 	// AWB_WEIGHT_R4                    
	{CMD_WRITE, 0x324C, 0x03E0, WORD_LEN}, 	// AWB_WEIGHT_R5                    
	{CMD_WRITE, 0x324E, 0x00AB, WORD_LEN}, 	// AWB_WEIGHT_R6                    
	{CMD_WRITE, 0x3250, 0x002A, WORD_LEN}, 	// AWB_WEIGHT_R7                    
	{CMD_WRITE, 0xAC97, 0x70, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_0 
	{CMD_WRITE, 0xAC98, 0x7A, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_1 
	{CMD_WRITE, 0xAC99, 0x84, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_2 
	{CMD_WRITE, 0xAC9A, 0x7A, BYTE_LEN}, 	// 80AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_0
	{CMD_WRITE, 0xAC9B, 0x80, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_1
	{CMD_WRITE, 0xAC9C, 0x7E, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_2
	{CMD_WRITE, 0xDC33, 0x20, BYTE_LEN}, 	// SYS_FIRST_BLACK_LEVEL       
	{CMD_WRITE, 0xDC35, 0x04, BYTE_LEN}, 	// SYS_UV_COLOR_BOOST          
	{CMD_WRITE, 0x326E, 0x0006, WORD_LEN}, 	// LOW_PASS_YUV_FILTER         
	{CMD_WRITE, 0xDC37, 0x62, BYTE_LEN}, 	// SYS_BRIGHT_COLORKILL        
	{CMD_WRITE, 0x35A4, 0x0596, WORD_LEN}, 	// BRIGHT_COLOR_KILL_CONTROLS  
	{CMD_WRITE, 0x35A2, 0x0094, WORD_LEN}, 	// DARK_COLOR_KILL_CONTROLS    
	{CMD_WRITE, 0xDC36, 0x24, BYTE_LEN}, 	// SYS_DARK_COLOR_KILL         
	{CMD_WRITE, 0xBC14, 0xFFFF, WORD_LEN},     // LL_GAMMA_FADE_TO_BLACK_START_POS        
	{CMD_WRITE, 0xBC18, 0x00, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_0   
	{CMD_WRITE, 0xBC19, 0x06, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_1   
	{CMD_WRITE, 0xBC1A, 0x14, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_2   
	{CMD_WRITE, 0xBC1B, 0x31, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_3   
	{CMD_WRITE, 0xBC1C, 0x53, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_4   
	{CMD_WRITE, 0xBC1D, 0x6E, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_5   
	{CMD_WRITE, 0xBC1E, 0x84, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_6   
	{CMD_WRITE, 0xBC1F, 0x98, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_7   
	{CMD_WRITE, 0xBC20, 0xA7, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_8   
	{CMD_WRITE, 0xBC21, 0xB5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_9   
	{CMD_WRITE, 0xBC22, 0xC1, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_10  
	{CMD_WRITE, 0xBC23, 0xCB, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_11  
	{CMD_WRITE, 0xBC24, 0xD5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_12  
	{CMD_WRITE, 0xBC25, 0xDD, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_13  
	{CMD_WRITE, 0xBC26, 0xE5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_14  
	{CMD_WRITE, 0xBC27, 0xEC, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_15  
	{CMD_WRITE, 0xBC28, 0xF3, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_16  
	{CMD_WRITE, 0xBC29, 0xF9, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_17  
	{CMD_WRITE, 0xBC2A, 0xFF, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_18  
	{CMD_WRITE, 0xBC2B, 0x00, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_0    
	{CMD_WRITE, 0xBC2C, 0x06, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_1    
	{CMD_WRITE, 0xBC2D, 0x14, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_2    
	{CMD_WRITE, 0xBC2E, 0x31, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_3    
	{CMD_WRITE, 0xBC2F, 0x53, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_4    
	{CMD_WRITE, 0xBC30, 0x6E, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_5    
	{CMD_WRITE, 0xBC31, 0x84, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_6    
	{CMD_WRITE, 0xBC32, 0x98, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_7    
	{CMD_WRITE, 0xBC33, 0xA7, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_8    
	{CMD_WRITE, 0xBC34, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_9    
	{CMD_WRITE, 0xBC35, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_10   
	{CMD_WRITE, 0xBC36, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_11   
	{CMD_WRITE, 0xBC37, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_12   
	{CMD_WRITE, 0xBC38, 0xDD, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_13   
	{CMD_WRITE, 0xBC39, 0xE5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_14             
	{CMD_WRITE, 0xBC3A, 0xEC, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_15             
	{CMD_WRITE, 0xBC3B, 0xF3, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_16             
	{CMD_WRITE, 0xBC3C, 0xF9, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_17             
	{CMD_WRITE, 0xBC3D, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_18             
	{CMD_WRITE, 0xBC3E, 0x00, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_0                   
	{CMD_WRITE, 0xBC3F, 0x07, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_1                   
	{CMD_WRITE, 0xBC40, 0x16, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_2                   
	{CMD_WRITE, 0xBC41, 0x30, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_3                   
	{CMD_WRITE, 0xBC42, 0x4F, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_4                   
	{CMD_WRITE, 0xBC43, 0x67, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_5                   
	{CMD_WRITE, 0xBC44, 0x7A, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_6                   
	{CMD_WRITE, 0xBC45, 0x8C, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_7                   
	{CMD_WRITE, 0xBC46, 0x9B, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_8                   
	{CMD_WRITE, 0xBC47, 0xA9, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_9                   
	{CMD_WRITE, 0xBC48, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_10                  
	{CMD_WRITE, 0xBC49, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_11                  
	{CMD_WRITE, 0xBC4A, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_12                  
	{CMD_WRITE, 0xBC4B, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_13                  
	{CMD_WRITE, 0xBC4C, 0xDE, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_14                  
	{CMD_WRITE, 0xBC4D, 0xE7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_15                 
	{CMD_WRITE, 0xBC4E, 0xEF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_16                  
	{CMD_WRITE, 0xBC4F, 0xF7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_17                  
	{CMD_WRITE, 0xBC50, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_18                  
	{CMD_WRITE, 0xB801, 0xE0, BYTE_LEN}, 	// STAT_MODE                             
	{CMD_WRITE, 0xB862, 0x04, BYTE_LEN}, 	// STAT_BMTRACKING_SPEED                 
	{CMD_WRITE, 0xB829, 0x02, BYTE_LEN}, 	// STAT_LL_BRIGHTNESS_METRIC_DIVISOR     
	{CMD_WRITE, 0xB863, 0x02, BYTE_LEN}, 	// STAT_BM_MUL                           
	{CMD_WRITE, 0xB827, 0x0F, BYTE_LEN}, 	// STAT_AE_EV_SHIFT                      
	{CMD_WRITE, 0xB82f, 0x08, BYTE_LEN}, 	// STAT_FD_SMOOTHING_FILTER_SIZE                      
	{CMD_WRITE, 0x840F, 0x02, BYTE_LEN},   // SEQ_STATE_CFG_0_FD
	{CMD_WRITE, 0x8417, 0x02, BYTE_LEN},   // SEQ_STATE_CFG_1_FD
	{CMD_WRITE, 0x841F, 0x02, BYTE_LEN},   // SEQ_STATE_CFG_2_FD
	{CMD_WRITE, 0xA002, 0x0001, WORD_LEN}, 	// FD_ALGO
	{CMD_WRITE, 0xA004, 0x3C, BYTE_LEN}, 	// FD_flicker_fre
	{CMD_WRITE, 0xA814, 0x00B2, WORD_LEN}, 	// AE_track_flicker_period
	{CMD_WRITE, 0xA401, 0x00, BYTE_LEN}, 	// AE_RULE_MODE                          
	{CMD_WRITE, 0xA409, 0x48, BYTE_LEN},  	//43 AE_RULE_BASE_TARGET                   
	{CMD_WRITE, 0xA801, 0x00, BYTE_LEN},  	// AE_TRACK_MODE                         
	{CMD_WRITE, 0xA805, 0x0A, BYTE_LEN},  	// AE_TRACK_GATE                         
	{CMD_WRITE, 0xA80E, 0x1A, BYTE_LEN},  	// AE_TRACK_MAX_BLACK_LEVEL              
	{CMD_WRITE, 0xA81A, 0x0733, WORD_LEN}, 	// AE_TRACK_MAX_INT_TIME_ROWS            
	{CMD_WRITE, 0xA818, 0x0450, WORD_LEN}, 	// AE_TRACK_TARGET_INT_TIME_ROWS         
	{CMD_WRITE, 0xA81E, 0x00F0, WORD_LEN}, 	// AE_TRACK_TARGET_AGAIN                 
	{CMD_WRITE, 0xA820, 0x014C, WORD_LEN}, 	// AE_TRACK_MAX_AGAIN                    
	{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, 	// AE_TRACK_MIN_DGAIN                   
	{CMD_WRITE, 0xA824, 0x0100, WORD_LEN}, 	// 1B0 AE_TRACK_MAX_DGAIN                   
	{CMD_WRITE, 0xC8E6, 0x014C, WORD_LEN}, 	// CAM_MAX_ANALOG_GAIN                  
	{CMD_WRITE, 0xC8E8, 0x0040, WORD_LEN}, 	// CAM_MIN_ANALOG_GAIN                  
	{CMD_WRITE, 0xBC52, 0x00C8, WORD_LEN}, 	// LL_START_BRIGHTNESS_METRIC           
	{CMD_WRITE, 0xBC58, 0x00FA, WORD_LEN}, 	// LL_START_GAIN_METRIC                 
	{CMD_WRITE, 0xBC5A, 0x0400, WORD_LEN}, 	// LL_END_GAIN_METRIC                   
	{CMD_WRITE, 0xBC5E, 0x00FA, WORD_LEN}, 	// LL_START_APERTURE_GAIN_BM            
	{CMD_WRITE, 0xBC60, 0x0258, WORD_LEN}, 	// LL_END_APERTURE_GAIN_BM              
	{CMD_WRITE, 0xBC66, 0x0154, WORD_LEN}, 	// LL_START_APERTURE_GM                 
	{CMD_WRITE, 0xBC68, 0x0390, WORD_LEN}, 	// LL_END_APERTURE_GM                   
	{CMD_WRITE, 0xBC86, 0x00c0, WORD_LEN}, 	// FA LL_START_FFNR_GM                     
	{CMD_WRITE, 0xBC88, 0x00a0, WORD_LEN}, 	// 390LL_END_FFNR_GM                       
	{CMD_WRITE, 0xBCCC, 0x00C8, WORD_LEN}, 	// LL_SFFB_START_MAX_GM                 
	{CMD_WRITE, 0xBCCE, 0x0390, WORD_LEN}, 	// LL_SFFB_END_MAX_GM                   
	{CMD_WRITE, 0xBC90, 0x00FA, WORD_LEN}, 	// LL_START_GRB_GM                      
	{CMD_WRITE, 0xBC92, 0x0390, WORD_LEN}, 	// LL_END_GRB_GM                        
	{CMD_WRITE, 0xBC0E, 0x0001, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_START_POS         
	{CMD_WRITE, 0xBC10, 0x00F0, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_MID_POS           
	{CMD_WRITE, 0xBC12, 0x0640, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_END_POS           
	{CMD_WRITE, 0xBCAA, 0x03e8, WORD_LEN}, 	// LL_CDC_THR_ADJ_START_POS             
	{CMD_WRITE, 0xBCAC, 0x012c, WORD_LEN}, 	// LL_CDC_THR_ADJ_MID_POS               
	{CMD_WRITE, 0xBCAE, 0x0009, WORD_LEN}, 	// LL_CDC_THR_ADJ_END_POS               
	{CMD_WRITE, 0xBCD8, 0x0154, WORD_LEN}, 	// LL_PCR_START_BM                      
	{CMD_WRITE, 0xBCDA, 0x0A28, WORD_LEN}, 	// LL_PCR_END_BM                        
	{CMD_WRITE, 0x3380, 0x0587, WORD_LEN},  // KERNEL_CONFIG 
	{CMD_WRITE, 0xBCB0, 0x2C, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SLOPE
	{CMD_WRITE, 0xBCB1, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SATUR
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0xFF, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB3, 0x3A, BYTE_LEN},         // LL_CDC_DARK_CLUS_SATUR
	{CMD_WRITE, 0xBCB4, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB5, 0x20, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB6, 0x80, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB7, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SATUR
	{CMD_WRITE, 0xBCB8, 0x3A, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SATUR
	{CMD_WRITE, 0xBCB9, 0x24, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SATUR                                
	{CMD_WRITE, 0xBC94, 0x12, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_0              
	{CMD_WRITE, 0xBC95, 0x0C, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_1              
	{CMD_WRITE, 0xBC9C, 0x37, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_0                
	{CMD_WRITE, 0xBC9D, 0x24, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_1                
	{CMD_WRITE, 0x33B0, 0x2A16, WORD_LEN}, 	// FFNR_ALPHA_BETA                      
	{CMD_WRITE, 0xBC8A, 0x20, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_Y             
	{CMD_WRITE, 0xBC8B, 0x40, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_Y               
	{CMD_WRITE, 0xBC8C, 0xB0, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_YGAIN         
	{CMD_WRITE, 0xBC8D, 0xFF, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_YGAIN          
	{CMD_WRITE, 0xBC8E, 0x10, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_GAIN         
	{CMD_WRITE, 0xBC8F, 0x00, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_GAIN           
	{CMD_WRITE, 0xBCC0, 0x20, BYTE_LEN}, 	// LL_SFFB_RAMP_START                  
	{CMD_WRITE, 0xBCC1, 0x03, BYTE_LEN}, 	// LL_SFFB_RAMP_STOP                   
	{CMD_WRITE, 0xBCC2, 0x1E, BYTE_LEN}, 	// LL_SFFB_SLOPE_START                
	{CMD_WRITE, 0xBCC3, 0x0F, BYTE_LEN}, 	// LL_SFFB_SLOPE_STOP                  
	{CMD_WRITE, 0xBCC4, 0x0A, BYTE_LEN}, 	// LL_SFFB_THSTART                     
	{CMD_WRITE, 0xBCC5, 0xAF, BYTE_LEN}, 	// LL_SFFB_THSTOP                      
	{CMD_WRITE, 0xBC6A, 0x04, BYTE_LEN}, 	// LL_START_APERTURE_INTEGER_GAIN      
	{CMD_WRITE, 0xBC6B, 0x00, BYTE_LEN}, 	// LL_END_APERTURE_INTEGER_GAIN        
	{CMD_WRITE, 0x33BA, 0x001F, WORD_LEN}, 	// APEDGE_CONTROL                      
	{CMD_WRITE, 0x33BE, 0x0000, WORD_LEN}, 	// UA_KNEE_L                           
	{CMD_WRITE, 0x33C2, 0x4300, WORD_LEN}, 	// UA_WEIGHTS                          
	{CMD_WRITE, 0xBC62, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KPGAIN            
	{CMD_WRITE, 0xBC63, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KPGAIN              
	{CMD_WRITE, 0xBC64, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KNGAIN            
	{CMD_WRITE, 0xBC65, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KNGAIN              
	{CMD_WRITE, 0xBCE2, 0x0A, BYTE_LEN}, 	// LL_START_POS_KNEE                   
	{CMD_WRITE, 0xBCE3, 0x30, BYTE_LEN}, 	// LL_END_POS_KNEE                     
	{CMD_WRITE, 0xBCE4, 0x0A, BYTE_LEN}, 	// LL_START_NEG_KNEE                   
	{CMD_WRITE, 0xBCE5, 0x30, BYTE_LEN}, 	// LL_END_NEG_KNEE                     
	{CMD_WRITE, 0xBC56, 0x64, BYTE_LEN}, 	// LL_START_CCM_SATURATION             
	{CMD_WRITE, 0xBC57, 0x3C, BYTE_LEN}, 	// LL_END_CCM_SATURATION               
	{CMD_WRITE, 0xBCDE, 0x03, BYTE_LEN}, 	// LL_START_SYS_THRESHOLD              
	{CMD_WRITE, 0xBCDF, 0x50, BYTE_LEN}, 	// LL_STOP_SYS_THRESHOLD               
	{CMD_WRITE, 0xBCE0, 0x08, BYTE_LEN}, 	// LL_START_SYS_GAIN                   
	{CMD_WRITE, 0xBCE1, 0x03, BYTE_LEN}, 	// LL_STOP_SYS_GAIN                    
	{CMD_WRITE, 0xBCC6, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_START            
	{CMD_WRITE, 0xBCC7, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_STOP             
	{CMD_WRITE, 0xBCC8, 0x2F, BYTE_LEN}, 	// LL_SFFB_FLATNESS_START              
	{CMD_WRITE, 0xBCC9, 0x40, BYTE_LEN}, 	// LL_SFFB_FLATNESS_STOP               
	{CMD_WRITE, 0xBCCA, 0x04, BYTE_LEN}, 	// LL_SFFB_TRANSITION_START            
	{CMD_WRITE, 0xBCCB, 0x00, BYTE_LEN}, 	// LL_SFFB_TRANSITION_STOP             
	{CMD_WRITE, 0xBCE6, 0x00, BYTE_LEN}, 	// LL_SFFB_ZERO_ENABLE                                   
	{CMD_WRITE, 0xC8ED, 0x00, BYTE_LEN}, 	// CAM_TX_ENABLE_MODE                  
	{CMD_WRITE, 0xC8BC, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8BD, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_1          
	{CMD_WRITE, 0xC8D2, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8D3, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_1   
	{CMD_WRITE, 0xDC3A, 0x23, BYTE_LEN}, 	// SYS_SEPIA_CR                 
	{CMD_WRITE, 0xDC3B, 0xB2, BYTE_LEN}, 	// SYS_SEPIA_CB                 
	{CMD_WRITE, 0xBC02, 0x01FF, WORD_LEN}, 	// LL_ALGO                      
	{CMD_WRITE, 0x8419, 0x03, BYTE_LEN}, 	// SEQ_STATE_CFG_1_AF           
	{CMD_WRITE, 0xC400, 0x88, BYTE_LEN}, 	// AFM_ALGO                     
	{CMD_WRITE, 0xB002, 0x0002, WORD_LEN}, 	// AF_MODE                      
	{CMD_WRITE, 0xB004, 0x0010, WORD_LEN}, 	// AF_ALGO                      
	{CMD_WRITE, 0xB045, 0x0015, WORD_LEN}, 	// AF_MODE_EX                   
	{CMD_WRITE, 0xB02C, 0x20, BYTE_LEN}, 	// AF_HC_STEP                   
	{CMD_WRITE, 0xB02D, 0x40, BYTE_LEN}, 	// AF_HC_LONG_STEP              
	{CMD_WRITE, 0xB02E, 0x50, BYTE_LEN}, 	// AF_HC_LONG_STEP_USE_THR      
	{CMD_WRITE, 0xB048, 0x01, BYTE_LEN}, 	// AF_C_HC_PROGRESS_TH          
	{CMD_WRITE, 0xB041, 0x02, BYTE_LEN}, 	// AF_DEBUG2                    
	{CMD_WRITE, 0xC856, 0x0423, WORD_LEN}, 	// CAM_CORE_A_UNUSED1           
	{CMD_WRITE, 0xC40A, 0x0028, WORD_LEN}, 	// AFM_POS_MIN                  
	{CMD_WRITE, 0xC40C, 0x00A0, WORD_LEN}, 	// AFM_POS_MAX                  
	{CMD_WRITE, 0xC402, 0x00, BYTE_LEN},  // AFM_MODE                     
	{CMD_WRITE, 0xB018, 0x00, BYTE_LEN}, 	// AF_FS_POS_0                  
	{CMD_WRITE, 0xB019, 0x14, BYTE_LEN}, 	// AF_FS_POS_1                  
	{CMD_WRITE, 0xB01A, 0x21, BYTE_LEN}, 	// AF_FS_POS_2                  
	{CMD_WRITE, 0xB01B, 0x2e, BYTE_LEN}, 	// AF_FS_POS_3                  
	{CMD_WRITE, 0xB01C, 0x3b, BYTE_LEN}, 	// AF_FS_POS_4                  
	{CMD_WRITE, 0xB01D, 0x48, BYTE_LEN}, 	// AF_FS_POS_5                  
	{CMD_WRITE, 0xB01E, 0x55, BYTE_LEN}, 	// AF_FS_POS_6                  
	{CMD_WRITE, 0xB01F, 0x62, BYTE_LEN}, 	// AF_FS_POS_7                  
	{CMD_WRITE, 0xB020, 0x6f, BYTE_LEN}, 	// AF_FS_POS_8                  
	{CMD_WRITE, 0xB021, 0x7c, BYTE_LEN}, 	// AF_FS_POS_9  
	{CMD_WRITE, 0xB022, 0x89, BYTE_LEN},	 	// AF_FS_POS_10 
	{CMD_WRITE, 0xB023, 0x96, BYTE_LEN},	 	// AF_FS_POS_11 
	{CMD_WRITE, 0xB024, 0xa3, BYTE_LEN},	 	// AF_FS_POS_12 
	{CMD_WRITE, 0xB025, 0xb0, BYTE_LEN},	 	// AF_FS_POS_13 
	{CMD_WRITE, 0xB026, 0xbd, BYTE_LEN},	 	// AF_FS_POS_14 
	{CMD_WRITE, 0xB027, 0xd4, BYTE_LEN},	 	// AF_FS_POS_15 
	{CMD_WRITE, 0xB028, 0xe1, BYTE_LEN},	 	// AF_FS_POS_16 
	{CMD_WRITE, 0xB029, 0xee, BYTE_LEN},	 	// AF_FS_POS_17 
	{CMD_WRITE, 0xB02A, 0xfc, BYTE_LEN},	 	// AF_FS_POS_18 
	{CMD_WRITE, 0xB02B, 0xff, BYTE_LEN},	 	// AF_FS_POS_19 
	{CMD_WRITE, 0xB012, 0x14, BYTE_LEN}, 	// AF_FS_NUM_STEPS              
	{CMD_WRITE, 0xB014, 0x0B, BYTE_LEN}, 	// AF_FS_STEP_SIZE 
	{CMD_WRITE, 0xB036, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_STARTING_MOT_EVAL
	{CMD_WRITE, 0xB03C, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_FOCUSING_AFTER_MOT               
	{CMD_WRITE, 0xB854, 0x52, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_X         
	{CMD_WRITE, 0xB855, 0x58, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_Y         
	{CMD_WRITE, 0xB856, 0x5D, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_X        
	{CMD_WRITE, 0xB857, 0x5A, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_Y
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},  // STANDBY_CONTROL_AND_STATUS 
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const init2_flicker_50hz_mode_settings_array[] =
{			 										
	{CMD_WRITE, 0x3210, 0x49B8, WORD_LEN},
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},
	{CMD_WRITE, 0xAC02, 0x00FF, WORD_LEN},                                   
	{CMD_WRITE, 0xAC01, 0xEF, BYTE_LEN},  	// AWB_MODE                                   
	{CMD_WRITE, 0xAC3C, 0x26, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3D, 0x6E, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3E, 0x14, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC3F, 0x6F, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC40, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC41, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC42, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xAC43, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xACB0, 0x2F, BYTE_LEN},  	// AWB_RG_MIN                                 
	{CMD_WRITE, 0xACB1, 0x5F, BYTE_LEN},  	// AWB_RG_MAX                                 
	{CMD_WRITE, 0xACB4, 0x1B, BYTE_LEN},  	// AWB_BG_MIN                                 
	{CMD_WRITE, 0xACB5, 0x5B, BYTE_LEN},  	// AWB_BG_MAX                                 
	{CMD_WRITE, 0xACB2, 0x40, BYTE_LEN},  	// 41AWB_RG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB3, 0x46, BYTE_LEN},  	// AWB_RG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB6, 0x36, BYTE_LEN},  	// AWB_BG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB7, 0x3f, BYTE_LEN},  	// AWB_BG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB8, 0x0100, WORD_LEN},  // AWB_START_NUM_INT_LINES                    
	{CMD_WRITE, 0xACBA, 0x00B0, WORD_LEN},  // AWB_END_NUM_INT_LINES                      
	{CMD_WRITE, 0xAC09, 0x01, BYTE_LEN},  	// AWB_UNUSED1  
	{CMD_WRITE, 0xAC22, 0x0000, WORD_LEN},  // AWB_LEFT_CCM_0	                              
	{CMD_WRITE, 0xAC46, 0x0233, WORD_LEN},  // AWB_LEFT_CCM_0                             
	{CMD_WRITE, 0xAC48, 0xFEDA, WORD_LEN}, 	// AWB_LEFT_CCM_1                             
	{CMD_WRITE, 0xAC4A, 0xFFF2, WORD_LEN}, 	// AWB_LEFT_CCM_2                           
	{CMD_WRITE, 0xAC4C, 0xFF83, WORD_LEN}, 	// AWB_LEFT_CCM_3                             
	{CMD_WRITE, 0xAC4E, 0x019B, WORD_LEN}, 	// AWB_LEFT_CCM_4                             
	{CMD_WRITE, 0xAC50, 0xFFE0, WORD_LEN}, 	// AWB_LEFT_CCM_5                             
	{CMD_WRITE, 0xAC52, 0xFFE6, WORD_LEN}, 	// AWB_LEFT_CCM_6                             
	{CMD_WRITE, 0xAC54, 0xFEBF, WORD_LEN}, 	// AWB_LEFT_CCM_7                             
	{CMD_WRITE, 0xAC56, 0x025C, WORD_LEN}, 	// AWB_LEFT_CCM_8                             
	{CMD_WRITE, 0xAC58, 0x0091, WORD_LEN}, 	// AWB_LEFT_CCM_R2BRATIO                      
	{CMD_WRITE, 0xAC5C, 0x02B8, WORD_LEN}, 	// AWB_RIGHT_CCM_0                            
	{CMD_WRITE, 0xAC5E, 0xFEA7, WORD_LEN}, 	// AWB_RIGHT_CCM_1                            
	{CMD_WRITE, 0xAC60, 0xFF92, WORD_LEN}, 	// AWB_RIGHT_CCM_2                            
	{CMD_WRITE, 0xAC62, 0xFFE0, WORD_LEN}, 	// AWB_RIGHT_CCM_3                            
	{CMD_WRITE, 0xAC64, 0x0136, WORD_LEN}, 	// AWB_RIGHT_CCM_4                            
	{CMD_WRITE, 0xAC66, 0xFFEE, WORD_LEN}, 	// AWB_RIGHT_CCM_5                            
	{CMD_WRITE, 0xAC68, 0x002A, WORD_LEN}, 	// AWB_RIGHT_CCM_6                            
	{CMD_WRITE, 0xAC6A, 0xFF2C, WORD_LEN}, 	// AWB_RIGHT_CCM_7                            
	{CMD_WRITE, 0xAC6C, 0x01A5, WORD_LEN}, 	// AWB_RIGHT_CCM_8                  
	{CMD_WRITE, 0xAC6E, 0x0066, WORD_LEN}, 	// AWB_RIGHT_CCM_R2BRATIO           
	{CMD_WRITE, 0xB83F, 0x00, BYTE_LEN},  	// STAT_AWB_WINDOW_POS_Y            
	{CMD_WRITE, 0xB840, 0xFF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_X           
	{CMD_WRITE, 0xB841, 0xEF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_Y           
	{CMD_WRITE, 0xB842, 0x0030, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_X   
	{CMD_WRITE, 0xB844, 0x002b, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_Y   
	{CMD_WRITE, 0x3240, 0x0024, WORD_LEN}, 	// 34AWB_XY_SCALE                     
	{CMD_WRITE, 0x3244, 0xAA80, WORD_LEN}, 	// AWB_WEIGHT_R1                    
	{CMD_WRITE, 0x3246, 0x5580, WORD_LEN}, 	// AWB_WEIGHT_R2                    
	{CMD_WRITE, 0x3248, 0x3700, WORD_LEN}, 	// AWB_WEIGHT_R3                    
	{CMD_WRITE, 0x324A, 0x39C0, WORD_LEN}, 	// AWB_WEIGHT_R4                    
	{CMD_WRITE, 0x324C, 0x03E0, WORD_LEN}, 	// AWB_WEIGHT_R5                    
	{CMD_WRITE, 0x324E, 0x00AB, WORD_LEN}, 	// AWB_WEIGHT_R6                    
	{CMD_WRITE, 0x3250, 0x002A, WORD_LEN}, 	// AWB_WEIGHT_R7                    
	{CMD_WRITE, 0xAC97, 0x70, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_0 
	{CMD_WRITE, 0xAC98, 0x7A, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_1 
	{CMD_WRITE, 0xAC99, 0x84, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_2 
	{CMD_WRITE, 0xAC9A, 0x7A, BYTE_LEN}, 	// 80AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_0
	{CMD_WRITE, 0xAC9B, 0x80, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_1
	{CMD_WRITE, 0xAC9C, 0x7E, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_2
	{CMD_WRITE, 0xDC33, 0x20, BYTE_LEN}, 	// SYS_FIRST_BLACK_LEVEL       
	{CMD_WRITE, 0xDC35, 0x04, BYTE_LEN}, 	// SYS_UV_COLOR_BOOST          
	{CMD_WRITE, 0x326E, 0x0006, WORD_LEN}, 	// LOW_PASS_YUV_FILTER         
	{CMD_WRITE, 0xDC37, 0x62, BYTE_LEN}, 	// SYS_BRIGHT_COLORKILL        
	{CMD_WRITE, 0x35A4, 0x0596, WORD_LEN}, 	// BRIGHT_COLOR_KILL_CONTROLS  
	{CMD_WRITE, 0x35A2, 0x0094, WORD_LEN}, 	// DARK_COLOR_KILL_CONTROLS    
	{CMD_WRITE, 0xDC36, 0x24, BYTE_LEN}, 	// SYS_DARK_COLOR_KILL 
	{CMD_WRITE, 0xBC14, 0xFFFF, WORD_LEN},     // LL_GAMMA_FADE_TO_BLACK_START_POS        
	{CMD_WRITE, 0xBC18, 0x00, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_0   
	{CMD_WRITE, 0xBC19, 0x06, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_1   
	{CMD_WRITE, 0xBC1A, 0x14, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_2   
	{CMD_WRITE, 0xBC1B, 0x31, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_3   
	{CMD_WRITE, 0xBC1C, 0x53, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_4   
	{CMD_WRITE, 0xBC1D, 0x6E, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_5   
	{CMD_WRITE, 0xBC1E, 0x84, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_6   
	{CMD_WRITE, 0xBC1F, 0x98, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_7   
	{CMD_WRITE, 0xBC20, 0xA7, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_8   
	{CMD_WRITE, 0xBC21, 0xB5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_9   
	{CMD_WRITE, 0xBC22, 0xC1, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_10  
	{CMD_WRITE, 0xBC23, 0xCB, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_11  
	{CMD_WRITE, 0xBC24, 0xD5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_12  
	{CMD_WRITE, 0xBC25, 0xDD, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_13  
	{CMD_WRITE, 0xBC26, 0xE5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_14  
	{CMD_WRITE, 0xBC27, 0xEC, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_15  
	{CMD_WRITE, 0xBC28, 0xF3, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_16  
	{CMD_WRITE, 0xBC29, 0xF9, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_17  
	{CMD_WRITE, 0xBC2A, 0xFF, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_18  
	{CMD_WRITE, 0xBC2B, 0x00, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_0    
	{CMD_WRITE, 0xBC2C, 0x06, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_1    
	{CMD_WRITE, 0xBC2D, 0x14, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_2    
	{CMD_WRITE, 0xBC2E, 0x31, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_3    
	{CMD_WRITE, 0xBC2F, 0x53, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_4    
	{CMD_WRITE, 0xBC30, 0x6E, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_5    
	{CMD_WRITE, 0xBC31, 0x84, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_6    
	{CMD_WRITE, 0xBC32, 0x98, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_7    
	{CMD_WRITE, 0xBC33, 0xA7, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_8    
	{CMD_WRITE, 0xBC34, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_9    
	{CMD_WRITE, 0xBC35, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_10   
	{CMD_WRITE, 0xBC36, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_11   
	{CMD_WRITE, 0xBC37, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_12   
	{CMD_WRITE, 0xBC38, 0xDD, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_13   
	{CMD_WRITE, 0xBC39, 0xE5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_14             
	{CMD_WRITE, 0xBC3A, 0xEC, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_15             
	{CMD_WRITE, 0xBC3B, 0xF3, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_16             
	{CMD_WRITE, 0xBC3C, 0xF9, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_17             
	{CMD_WRITE, 0xBC3D, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_18             
	{CMD_WRITE, 0xBC3E, 0x00, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_0                   
	{CMD_WRITE, 0xBC3F, 0x07, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_1                   
	{CMD_WRITE, 0xBC40, 0x16, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_2                   
	{CMD_WRITE, 0xBC41, 0x30, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_3                   
	{CMD_WRITE, 0xBC42, 0x4F, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_4                   
	{CMD_WRITE, 0xBC43, 0x67, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_5                   
	{CMD_WRITE, 0xBC44, 0x7A, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_6                   
	{CMD_WRITE, 0xBC45, 0x8C, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_7                   
	{CMD_WRITE, 0xBC46, 0x9B, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_8                   
	{CMD_WRITE, 0xBC47, 0xA9, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_9                   
	{CMD_WRITE, 0xBC48, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_10                  
	{CMD_WRITE, 0xBC49, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_11                  
	{CMD_WRITE, 0xBC4A, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_12                  
	{CMD_WRITE, 0xBC4B, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_13                  
	{CMD_WRITE, 0xBC4C, 0xDE, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_14                  
	{CMD_WRITE, 0xBC4D, 0xE7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_15                 
	{CMD_WRITE, 0xBC4E, 0xEF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_16                  
	{CMD_WRITE, 0xBC4F, 0xF7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_17                  
	{CMD_WRITE, 0xBC50, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_18                  
	{CMD_WRITE, 0xB801, 0xE0, BYTE_LEN}, 	// STAT_MODE                             
	{CMD_WRITE, 0xB862, 0x04, BYTE_LEN}, 	// STAT_BMTRACKING_SPEED                 
	{CMD_WRITE, 0xB829, 0x02, BYTE_LEN}, 	// STAT_LL_BRIGHTNESS_METRIC_DIVISOR     
	{CMD_WRITE, 0xB863, 0x02, BYTE_LEN}, 	// STAT_BM_MUL                           
	{CMD_WRITE, 0xB827, 0x0F, BYTE_LEN}, 	// STAT_AE_EV_SHIFT 
	{CMD_WRITE, 0xB82f, 0x08, BYTE_LEN}, 	// STAT_FD_SMOOTHING_FILTER_SIZE                      
	{CMD_WRITE, 0x840F, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_0_FD
	{CMD_WRITE, 0x8417, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_1_FD
	{CMD_WRITE, 0x841F, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_2_FD
	{CMD_WRITE, 0xa004, 0x32, BYTE_LEN},   // FD_EXPECTED_FLICKER_SOURCE_FREQUENCY
	{CMD_WRITE, 0xA814, 0x00d6, WORD_LEN}, 	// AE_TRACK_FLICKER_PERIOD
	{CMD_WRITE, 0xA002, 0x0001, WORD_LEN}, 	// FD_ALGO
	{CMD_WRITE, 0xA401, 0x00, BYTE_LEN}, 	// AE_RULE_MODE                          
	{CMD_WRITE, 0xA409, 0x48, BYTE_LEN},  	// AE_RULE_BASE_TARGET                   
	{CMD_WRITE, 0xA801, 0x00, BYTE_LEN},  	// AE_TRACK_MODE                         
	{CMD_WRITE, 0xA805, 0x0A, BYTE_LEN},  	// AE_TRACK_GATE                         
	{CMD_WRITE, 0xA80E, 0x1A, BYTE_LEN},  	// AE_TRACK_MAX_BLACK_LEVEL              
	{CMD_WRITE, 0xA81A, 0x0733, WORD_LEN}, 	// AE_TRACK_MAX_INT_TIME_ROWS            
	{CMD_WRITE, 0xA818, 0x0350, WORD_LEN}, 	// AE_TRACK_TARGET_INT_TIME_ROWS         
	{CMD_WRITE, 0xA81E, 0x0140, WORD_LEN}, 	// AE_TRACK_TARGET_AGAIN                 
	{CMD_WRITE, 0xA820, 0x014C, WORD_LEN}, 	// AE_TRACK_MAX_AGAIN                    
	{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, 	// AE_TRACK_MIN_DGAIN                   
	{CMD_WRITE, 0xA824, 0x0100, WORD_LEN}, 	// 1B0 AE_TRACK_MAX_DGAIN                   
	{CMD_WRITE, 0xC8E6, 0x014C, WORD_LEN}, 	// CAM_MAX_ANALOG_GAIN                  
	{CMD_WRITE, 0xC8E8, 0x0040, WORD_LEN}, 	// CAM_MIN_ANALOG_GAIN                  
	{CMD_WRITE, 0xBC52, 0x00C8, WORD_LEN}, 	// LL_START_BRIGHTNESS_METRIC           
	{CMD_WRITE, 0xBC58, 0x00FA, WORD_LEN}, 	// LL_START_GAIN_METRIC                 
	{CMD_WRITE, 0xBC5A, 0x0400, WORD_LEN}, 	// LL_END_GAIN_METRIC                   
	{CMD_WRITE, 0xBC5E, 0x00FA, WORD_LEN}, 	// LL_START_APERTURE_GAIN_BM            
	{CMD_WRITE, 0xBC60, 0x0258, WORD_LEN}, 	// LL_END_APERTURE_GAIN_BM              
	{CMD_WRITE, 0xBC66, 0x0154, WORD_LEN}, 	// LL_START_APERTURE_GM                 
	{CMD_WRITE, 0xBC68, 0x0390, WORD_LEN}, 	// LL_END_APERTURE_GM                   
	{CMD_WRITE, 0xBC86, 0x00c0, WORD_LEN}, 	// FA LL_START_FFNR_GM                     
	{CMD_WRITE, 0xBC88, 0x00a0, WORD_LEN}, 	// 390LL_END_FFNR_GM                       
	{CMD_WRITE, 0xBCCC, 0x00C8, WORD_LEN}, 	// LL_SFFB_START_MAX_GM                 
	{CMD_WRITE, 0xBCCE, 0x0390, WORD_LEN}, 	// LL_SFFB_END_MAX_GM                   
	{CMD_WRITE, 0xBC90, 0x00FA, WORD_LEN}, 	// LL_START_GRB_GM                      
	{CMD_WRITE, 0xBC92, 0x0390, WORD_LEN}, 	// LL_END_GRB_GM                        
	{CMD_WRITE, 0xBC0E, 0x0001, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_START_POS         
	{CMD_WRITE, 0xBC10, 0x00F0, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_MID_POS           
	{CMD_WRITE, 0xBC12, 0x0640, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_END_POS           
	{CMD_WRITE, 0xBCAA, 0x03e8, WORD_LEN}, 	// LL_CDC_THR_ADJ_START_POS             
	{CMD_WRITE, 0xBCAC, 0x012c, WORD_LEN}, 	// LL_CDC_THR_ADJ_MID_POS               
	{CMD_WRITE, 0xBCAE, 0x0009, WORD_LEN}, 	// LL_CDC_THR_ADJ_END_POS               
	{CMD_WRITE, 0xBCD8, 0x0154, WORD_LEN}, 	// LL_PCR_START_BM                      
	{CMD_WRITE, 0xBCDA, 0x0A28, WORD_LEN}, 	// LL_PCR_END_BM                        
	{CMD_WRITE, 0x3380, 0x0587, WORD_LEN},  // KERNEL_CONFIG 
	{CMD_WRITE, 0xBCB0, 0x2C, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SLOPE
	{CMD_WRITE, 0xBCB1, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SATUR
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0xFF, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB3, 0x3A, BYTE_LEN},         // LL_CDC_DARK_CLUS_SATUR
	{CMD_WRITE, 0xBCB4, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB5, 0x20, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB6, 0x80, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB7, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SATUR
	{CMD_WRITE, 0xBCB8, 0x3A, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SATUR
	{CMD_WRITE, 0xBCB9, 0x24, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SATUR                                
	{CMD_WRITE, 0xBC94, 0x12, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_0              
	{CMD_WRITE, 0xBC95, 0x0C, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_1              
	{CMD_WRITE, 0xBC9C, 0x37, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_0                
	{CMD_WRITE, 0xBC9D, 0x24, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_1                
	{CMD_WRITE, 0x33B0, 0x2A16, WORD_LEN}, 	// FFNR_ALPHA_BETA                      
	{CMD_WRITE, 0xBC8A, 0x20, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_Y             
	{CMD_WRITE, 0xBC8B, 0x40, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_Y               
	{CMD_WRITE, 0xBC8C, 0xb0, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_YGAIN         
	{CMD_WRITE, 0xBC8D, 0xff, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_YGAIN          
	{CMD_WRITE, 0xBC8E, 0x10, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_GAIN         
	{CMD_WRITE, 0xBC8F, 0x00, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_GAIN           
	{CMD_WRITE, 0xBCC0, 0x20, BYTE_LEN}, 	// LL_SFFB_RAMP_START                  
	{CMD_WRITE, 0xBCC1, 0x03, BYTE_LEN}, 	// LL_SFFB_RAMP_STOP                   
	{CMD_WRITE, 0xBCC2, 0x1E, BYTE_LEN}, 	// LL_SFFB_SLOPE_START                
	{CMD_WRITE, 0xBCC3, 0x0F, BYTE_LEN}, 	// LL_SFFB_SLOPE_STOP                  
	{CMD_WRITE, 0xBCC4, 0x0A, BYTE_LEN}, 	// LL_SFFB_THSTART                     
	{CMD_WRITE, 0xBCC5, 0xAF, BYTE_LEN}, 	// LL_SFFB_THSTOP                      
	{CMD_WRITE, 0xBC6A, 0x04, BYTE_LEN}, 	// LL_START_APERTURE_INTEGER_GAIN      
	{CMD_WRITE, 0xBC6B, 0x00, BYTE_LEN}, 	// LL_END_APERTURE_INTEGER_GAIN        
	{CMD_WRITE, 0x33BA, 0x001F, WORD_LEN}, 	// APEDGE_CONTROL                      
	{CMD_WRITE, 0x33BE, 0x0000, WORD_LEN}, 	// UA_KNEE_L                           
	{CMD_WRITE, 0x33C2, 0x4300, WORD_LEN}, 	// UA_WEIGHTS                          
	{CMD_WRITE, 0xBC62, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KPGAIN            
	{CMD_WRITE, 0xBC63, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KPGAIN              
	{CMD_WRITE, 0xBC64, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KNGAIN            
	{CMD_WRITE, 0xBC65, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KNGAIN              
	{CMD_WRITE, 0xBCE2, 0x0A, BYTE_LEN}, 	// LL_START_POS_KNEE                   
	{CMD_WRITE, 0xBCE3, 0x30, BYTE_LEN}, 	// LL_END_POS_KNEE                     
	{CMD_WRITE, 0xBCE4, 0x0A, BYTE_LEN}, 	// LL_START_NEG_KNEE                   
	{CMD_WRITE, 0xBCE5, 0x30, BYTE_LEN}, 	// LL_END_NEG_KNEE                     
	{CMD_WRITE, 0xBC56, 0x64, BYTE_LEN}, 	// LL_START_CCM_SATURATION             
	{CMD_WRITE, 0xBC57, 0x3C, BYTE_LEN}, 	// LL_END_CCM_SATURATION               
	{CMD_WRITE, 0xBCDE, 0x03, BYTE_LEN}, 	// LL_START_SYS_THRESHOLD              
	{CMD_WRITE, 0xBCDF, 0x50, BYTE_LEN}, 	// LL_STOP_SYS_THRESHOLD               
	{CMD_WRITE, 0xBCE0, 0x08, BYTE_LEN}, 	// LL_START_SYS_GAIN                   
	{CMD_WRITE, 0xBCE1, 0x03, BYTE_LEN}, 	// LL_STOP_SYS_GAIN                    
	{CMD_WRITE, 0xBCC6, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_START            
	{CMD_WRITE, 0xBCC7, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_STOP             
	{CMD_WRITE, 0xBCC8, 0x2F, BYTE_LEN}, 	// LL_SFFB_FLATNESS_START              
	{CMD_WRITE, 0xBCC9, 0x40, BYTE_LEN}, 	// LL_SFFB_FLATNESS_STOP               
	{CMD_WRITE, 0xBCCA, 0x04, BYTE_LEN}, 	// LL_SFFB_TRANSITION_START            
	{CMD_WRITE, 0xBCCB, 0x00, BYTE_LEN}, 	// LL_SFFB_TRANSITION_STOP             
	{CMD_WRITE, 0xBCE6, 0x00, BYTE_LEN}, 	// LL_SFFB_ZERO_ENABLE                                   
	{CMD_WRITE, 0xC8ED, 0x00, BYTE_LEN}, 	// CAM_TX_ENABLE_MODE                  
	{CMD_WRITE, 0xC8BC, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8BD, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_1          
	{CMD_WRITE, 0xC8D2, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8D3, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_1   
	{CMD_WRITE, 0xDC3A, 0x23, BYTE_LEN}, 	// SYS_SEPIA_CR                 
	{CMD_WRITE, 0xDC3B, 0xB2, BYTE_LEN}, 	// SYS_SEPIA_CB                 
	{CMD_WRITE, 0xBC02, 0x01FF, WORD_LEN}, 	// LL_ALGO                      
	{CMD_WRITE, 0x8419, 0x03, BYTE_LEN}, 	// SEQ_STATE_CFG_1_AF           
	{CMD_WRITE, 0xC400, 0x88, BYTE_LEN}, 	// AFM_ALGO                     
	{CMD_WRITE, 0xB002, 0x0002, WORD_LEN}, 	// AF_MODE                      
	{CMD_WRITE, 0xB004, 0x0010, WORD_LEN}, 	// AF_ALGO                      
	{CMD_WRITE, 0xB045, 0x0015, WORD_LEN}, 	// AF_MODE_EX                   
	{CMD_WRITE, 0xB02C, 0x20, BYTE_LEN}, 	// AF_HC_STEP                   
	{CMD_WRITE, 0xB02D, 0x40, BYTE_LEN}, 	// AF_HC_LONG_STEP              
	{CMD_WRITE, 0xB02E, 0x50, BYTE_LEN}, 	// AF_HC_LONG_STEP_USE_THR      
	{CMD_WRITE, 0xB048, 0x01, BYTE_LEN}, 	// AF_C_HC_PROGRESS_TH          
	{CMD_WRITE, 0xB041, 0x02, BYTE_LEN}, 	// AF_DEBUG2                    
	{CMD_WRITE, 0xC856, 0x0423, WORD_LEN}, 	// CAM_CORE_A_UNUSED1           
	{CMD_WRITE, 0xC40A, 0x0028, WORD_LEN}, 	// AFM_POS_MIN                  
	{CMD_WRITE, 0xC40C, 0x00a0, WORD_LEN}, 	// AFM_POS_MAX                  
	{CMD_WRITE, 0xC402, 0x00, BYTE_LEN},  // AFM_MODE                     
	{CMD_WRITE, 0xB018, 0x00, BYTE_LEN}, 	// AF_FS_POS_0                  
	{CMD_WRITE, 0xB019, 0x14, BYTE_LEN}, 	// AF_FS_POS_1                  
	{CMD_WRITE, 0xB01A, 0x21, BYTE_LEN}, 	// AF_FS_POS_2                  
	{CMD_WRITE, 0xB01B, 0x2e, BYTE_LEN}, 	// AF_FS_POS_3                  
	{CMD_WRITE, 0xB01C, 0x3b, BYTE_LEN}, 	// AF_FS_POS_4                  
	{CMD_WRITE, 0xB01D, 0x48, BYTE_LEN}, 	// AF_FS_POS_5                  
	{CMD_WRITE, 0xB01E, 0x55, BYTE_LEN}, 	// AF_FS_POS_6                  
	{CMD_WRITE, 0xB01F, 0x62, BYTE_LEN}, 	// AF_FS_POS_7                  
	{CMD_WRITE, 0xB020, 0x6f, BYTE_LEN}, 	// AF_FS_POS_8                  
	{CMD_WRITE, 0xB021, 0x7c, BYTE_LEN}, 	// AF_FS_POS_9  
	{CMD_WRITE, 0xB022, 0x89, BYTE_LEN},	 	// AF_FS_POS_10 
	{CMD_WRITE, 0xB023, 0x96, BYTE_LEN},	 	// AF_FS_POS_11 
	{CMD_WRITE, 0xB024, 0xa3, BYTE_LEN},	 	// AF_FS_POS_12 
	{CMD_WRITE, 0xB025, 0xb0, BYTE_LEN},	 	// AF_FS_POS_13 
	{CMD_WRITE, 0xB026, 0xbd, BYTE_LEN},	 	// AF_FS_POS_14 
	{CMD_WRITE, 0xB027, 0xd4, BYTE_LEN},	 	// AF_FS_POS_15 
	{CMD_WRITE, 0xB028, 0xe1, BYTE_LEN},	 	// AF_FS_POS_16 
	{CMD_WRITE, 0xB029, 0xee, BYTE_LEN},	 	// AF_FS_POS_17 
	{CMD_WRITE, 0xB02A, 0xfc, BYTE_LEN},	 	// AF_FS_POS_18 
	{CMD_WRITE, 0xB02B, 0xff, BYTE_LEN},	 	// AF_FS_POS_19 
	{CMD_WRITE, 0xB012, 0x14, BYTE_LEN}, 	// AF_FS_NUM_STEPS              
	{CMD_WRITE, 0xB014, 0x0B, BYTE_LEN}, 	// AF_FS_STEP_SIZE 
	{CMD_WRITE, 0xB036, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_STARTING_MOT_EVAL
	{CMD_WRITE, 0xB03C, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_FOCUSING_AFTER_MOT               
	{CMD_WRITE, 0xB854, 0x52, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_X         
	{CMD_WRITE, 0xB855, 0x58, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_Y         
	{CMD_WRITE, 0xB856, 0x5D, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_X        
	{CMD_WRITE, 0xB857, 0x5A, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_Y
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},  // STANDBY_CONTROL_AND_STATUS 
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const init2_flicker_60hz_mode_settings_array[] =
{			 										
	{CMD_WRITE, 0x3210, 0x49B8, WORD_LEN},
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},
	{CMD_WRITE, 0xAC02, 0x00FF, WORD_LEN},                                   
	{CMD_WRITE, 0xAC01, 0xEF, BYTE_LEN},  	// AWB_MODE                                   
	{CMD_WRITE, 0xAC3C, 0x26, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3D, 0x6E, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_R2G_RATIO         
	{CMD_WRITE, 0xAC3E, 0x14, BYTE_LEN},  	// AWB_MIN_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC3F, 0x6F, BYTE_LEN},  	// AWB_MAX_ACCEPTED_PRE_AWB_B2G_RATIO         
	{CMD_WRITE, 0xAC40, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC41, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_R2G_RATIO        
	{CMD_WRITE, 0xAC42, 0x64, BYTE_LEN},  	// AWB_MIN_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xAC43, 0x66, BYTE_LEN},  	// AWB_MAX_ACCEPTED_POST_AWB_B2G_RATIO        
	{CMD_WRITE, 0xACB0, 0x2F, BYTE_LEN},  	// AWB_RG_MIN                                 
	{CMD_WRITE, 0xACB1, 0x5F, BYTE_LEN},  	// AWB_RG_MAX                                 
	{CMD_WRITE, 0xACB4, 0x1B, BYTE_LEN},  	// AWB_BG_MIN                                 
	{CMD_WRITE, 0xACB5, 0x5B, BYTE_LEN},  	// AWB_BG_MAX                                 
	{CMD_WRITE, 0xACB2, 0x40, BYTE_LEN},  	// 41AWB_RG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB3, 0x46, BYTE_LEN},  	// AWB_RG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB6, 0x36, BYTE_LEN},  	// AWB_BG_MIN_BRIGHT                          
	{CMD_WRITE, 0xACB7, 0x3f, BYTE_LEN},  	// AWB_BG_MAX_BRIGHT                          
	{CMD_WRITE, 0xACB8, 0x0100, WORD_LEN},  // AWB_START_NUM_INT_LINES                    
	{CMD_WRITE, 0xACBA, 0x00B0, WORD_LEN},  // AWB_END_NUM_INT_LINES                      
	{CMD_WRITE, 0xAC09, 0x01, BYTE_LEN},  	// AWB_UNUSED1  
	{CMD_WRITE, 0xAC22, 0x0000, WORD_LEN},  // AWB_LEFT_CCM_0	                              
	{CMD_WRITE, 0xAC46, 0x0233, WORD_LEN},  // AWB_LEFT_CCM_0                             
	{CMD_WRITE, 0xAC48, 0xFEDA, WORD_LEN}, 	// AWB_LEFT_CCM_1                             
	{CMD_WRITE, 0xAC4A, 0xFFF2, WORD_LEN}, 	// AWB_LEFT_CCM_2                           
	{CMD_WRITE, 0xAC4C, 0xFF83, WORD_LEN}, 	// AWB_LEFT_CCM_3                             
	{CMD_WRITE, 0xAC4E, 0x019B, WORD_LEN}, 	// AWB_LEFT_CCM_4                             
	{CMD_WRITE, 0xAC50, 0xFFE0, WORD_LEN}, 	// AWB_LEFT_CCM_5                             
	{CMD_WRITE, 0xAC52, 0xFFE6, WORD_LEN}, 	// AWB_LEFT_CCM_6                             
	{CMD_WRITE, 0xAC54, 0xFEBF, WORD_LEN}, 	// AWB_LEFT_CCM_7                             
	{CMD_WRITE, 0xAC56, 0x025C, WORD_LEN}, 	// AWB_LEFT_CCM_8                             
	{CMD_WRITE, 0xAC58, 0x0091, WORD_LEN}, 	// AWB_LEFT_CCM_R2BRATIO                      
	{CMD_WRITE, 0xAC5C, 0x02B8, WORD_LEN}, 	// AWB_RIGHT_CCM_0                            
	{CMD_WRITE, 0xAC5E, 0xFEA7, WORD_LEN}, 	// AWB_RIGHT_CCM_1                            
	{CMD_WRITE, 0xAC60, 0xFF92, WORD_LEN}, 	// AWB_RIGHT_CCM_2                            
	{CMD_WRITE, 0xAC62, 0xFFE0, WORD_LEN}, 	// AWB_RIGHT_CCM_3                            
	{CMD_WRITE, 0xAC64, 0x0136, WORD_LEN}, 	// AWB_RIGHT_CCM_4                            
	{CMD_WRITE, 0xAC66, 0xFFEE, WORD_LEN}, 	// AWB_RIGHT_CCM_5                            
	{CMD_WRITE, 0xAC68, 0x002A, WORD_LEN}, 	// AWB_RIGHT_CCM_6                            
	{CMD_WRITE, 0xAC6A, 0xFF2C, WORD_LEN}, 	// AWB_RIGHT_CCM_7                            
	{CMD_WRITE, 0xAC6C, 0x01A5, WORD_LEN}, 	// AWB_RIGHT_CCM_8                  
	{CMD_WRITE, 0xAC6E, 0x0066, WORD_LEN}, 	// AWB_RIGHT_CCM_R2BRATIO           
	{CMD_WRITE, 0xB83F, 0x00, BYTE_LEN},  	// STAT_AWB_WINDOW_POS_Y            
	{CMD_WRITE, 0xB840, 0xFF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_X           
	{CMD_WRITE, 0xB841, 0xEF, BYTE_LEN},  	// STAT_AWB_WINDOW_SIZE_Y           
	{CMD_WRITE, 0xB842, 0x0030, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_X   
	{CMD_WRITE, 0xB844, 0x002b, WORD_LEN}, 	// STAT_AWB_GRAY_CHECKER_OFFSET_Y   
	{CMD_WRITE, 0x3240, 0x0024, WORD_LEN}, 	// 34AWB_XY_SCALE                     
	{CMD_WRITE, 0x3244, 0xAA80, WORD_LEN}, 	// AWB_WEIGHT_R1                    
	{CMD_WRITE, 0x3246, 0x5580, WORD_LEN}, 	// AWB_WEIGHT_R2                    
	{CMD_WRITE, 0x3248, 0x3700, WORD_LEN}, 	// AWB_WEIGHT_R3                    
	{CMD_WRITE, 0x324A, 0x39C0, WORD_LEN}, 	// AWB_WEIGHT_R4                    
	{CMD_WRITE, 0x324C, 0x03E0, WORD_LEN}, 	// AWB_WEIGHT_R5                    
	{CMD_WRITE, 0x324E, 0x00AB, WORD_LEN}, 	// AWB_WEIGHT_R6                    
	{CMD_WRITE, 0x3250, 0x002A, WORD_LEN}, 	// AWB_WEIGHT_R7                    
	{CMD_WRITE, 0xAC97, 0x70, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_0 
	{CMD_WRITE, 0xAC98, 0x7A, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_1 
	{CMD_WRITE, 0xAC99, 0x84, BYTE_LEN}, 	// AWB_LEFT_TINT_COEF_FOR_CCM_ROW_2 
	{CMD_WRITE, 0xAC9A, 0x7A, BYTE_LEN}, 	// 80AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_0
	{CMD_WRITE, 0xAC9B, 0x80, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_1
	{CMD_WRITE, 0xAC9C, 0x7E, BYTE_LEN}, 	// AWB_RIGHT_TINT_COEF_FOR_CCM_ROW_2
	{CMD_WRITE, 0xDC33, 0x20, BYTE_LEN}, 	// SYS_FIRST_BLACK_LEVEL       
	{CMD_WRITE, 0xDC35, 0x04, BYTE_LEN}, 	// SYS_UV_COLOR_BOOST          
	{CMD_WRITE, 0x326E, 0x0006, WORD_LEN}, 	// LOW_PASS_YUV_FILTER         
	{CMD_WRITE, 0xDC37, 0x62, BYTE_LEN}, 	// SYS_BRIGHT_COLORKILL        
	{CMD_WRITE, 0x35A4, 0x0596, WORD_LEN}, 	// BRIGHT_COLOR_KILL_CONTROLS  
	{CMD_WRITE, 0x35A2, 0x0094, WORD_LEN}, 	// DARK_COLOR_KILL_CONTROLS    
	{CMD_WRITE, 0xDC36, 0x24, BYTE_LEN}, 	// SYS_DARK_COLOR_KILL 
	{CMD_WRITE, 0xBC14, 0xFFFF, WORD_LEN},     // LL_GAMMA_FADE_TO_BLACK_START_POS        
	{CMD_WRITE, 0xBC18, 0x00, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_0   
	{CMD_WRITE, 0xBC19, 0x06, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_1   
	{CMD_WRITE, 0xBC1A, 0x14, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_2   
	{CMD_WRITE, 0xBC1B, 0x31, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_3   
	{CMD_WRITE, 0xBC1C, 0x53, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_4   
	{CMD_WRITE, 0xBC1D, 0x6E, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_5   
	{CMD_WRITE, 0xBC1E, 0x84, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_6   
	{CMD_WRITE, 0xBC1F, 0x98, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_7   
	{CMD_WRITE, 0xBC20, 0xA7, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_8   
	{CMD_WRITE, 0xBC21, 0xB5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_9   
	{CMD_WRITE, 0xBC22, 0xC1, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_10  
	{CMD_WRITE, 0xBC23, 0xCB, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_11  
	{CMD_WRITE, 0xBC24, 0xD5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_12  
	{CMD_WRITE, 0xBC25, 0xDD, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_13  
	{CMD_WRITE, 0xBC26, 0xE5, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_14  
	{CMD_WRITE, 0xBC27, 0xEC, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_15  
	{CMD_WRITE, 0xBC28, 0xF3, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_16  
	{CMD_WRITE, 0xBC29, 0xF9, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_17  
	{CMD_WRITE, 0xBC2A, 0xFF, BYTE_LEN}, 	// LL_GAMMA_CONTRAST_CURVE_18  
	{CMD_WRITE, 0xBC2B, 0x00, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_0    
	{CMD_WRITE, 0xBC2C, 0x06, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_1    
	{CMD_WRITE, 0xBC2D, 0x14, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_2    
	{CMD_WRITE, 0xBC2E, 0x31, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_3    
	{CMD_WRITE, 0xBC2F, 0x53, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_4    
	{CMD_WRITE, 0xBC30, 0x6E, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_5    
	{CMD_WRITE, 0xBC31, 0x84, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_6    
	{CMD_WRITE, 0xBC32, 0x98, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_7    
	{CMD_WRITE, 0xBC33, 0xA7, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_8    
	{CMD_WRITE, 0xBC34, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_9    
	{CMD_WRITE, 0xBC35, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_10   
	{CMD_WRITE, 0xBC36, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_11   
	{CMD_WRITE, 0xBC37, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_12   
	{CMD_WRITE, 0xBC38, 0xDD, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_13   
	{CMD_WRITE, 0xBC39, 0xE5, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_14             
	{CMD_WRITE, 0xBC3A, 0xEC, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_15             
	{CMD_WRITE, 0xBC3B, 0xF3, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_16             
	{CMD_WRITE, 0xBC3C, 0xF9, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_17             
	{CMD_WRITE, 0xBC3D, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NEUTRAL_CURVE_18             
	{CMD_WRITE, 0xBC3E, 0x00, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_0                   
	{CMD_WRITE, 0xBC3F, 0x07, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_1                   
	{CMD_WRITE, 0xBC40, 0x16, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_2                   
	{CMD_WRITE, 0xBC41, 0x30, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_3                   
	{CMD_WRITE, 0xBC42, 0x4F, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_4                   
	{CMD_WRITE, 0xBC43, 0x67, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_5                   
	{CMD_WRITE, 0xBC44, 0x7A, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_6                   
	{CMD_WRITE, 0xBC45, 0x8C, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_7                   
	{CMD_WRITE, 0xBC46, 0x9B, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_8                   
	{CMD_WRITE, 0xBC47, 0xA9, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_9                   
	{CMD_WRITE, 0xBC48, 0xB5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_10                  
	{CMD_WRITE, 0xBC49, 0xC1, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_11                  
	{CMD_WRITE, 0xBC4A, 0xCB, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_12                  
	{CMD_WRITE, 0xBC4B, 0xD5, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_13                  
	{CMD_WRITE, 0xBC4C, 0xDE, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_14                  
	{CMD_WRITE, 0xBC4D, 0xE7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_15                 
	{CMD_WRITE, 0xBC4E, 0xEF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_16                  
	{CMD_WRITE, 0xBC4F, 0xF7, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_17                  
	{CMD_WRITE, 0xBC50, 0xFF, BYTE_LEN}, 	// LL_GAMMA_NR_CURVE_18                  
	{CMD_WRITE, 0xB801, 0xE0, BYTE_LEN}, 	// STAT_MODE                             
	{CMD_WRITE, 0xB862, 0x04, BYTE_LEN}, 	// STAT_BMTRACKING_SPEED                 
	{CMD_WRITE, 0xB829, 0x02, BYTE_LEN}, 	// STAT_LL_BRIGHTNESS_METRIC_DIVISOR     
	{CMD_WRITE, 0xB863, 0x02, BYTE_LEN}, 	// STAT_BM_MUL                           
	{CMD_WRITE, 0xB827, 0x0F, BYTE_LEN}, 	// STAT_AE_EV_SHIFT 
	{CMD_WRITE, 0xB82f, 0x08, BYTE_LEN}, 	// STAT_FD_SMOOTHING_FILTER_SIZE                      
	{CMD_WRITE, 0x840F, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_0_FD
	{CMD_WRITE, 0x8417, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_1_FD
	{CMD_WRITE, 0x841F, 0x01, BYTE_LEN},   // SEQ_STATE_CFG_2_FD
	{CMD_WRITE, 0xa004, 0x3c, BYTE_LEN},   // FD_EXPECTED_FLICKER_SOURCE_FREQUENCY
	{CMD_WRITE, 0xA814, 0x00b2, WORD_LEN}, 	// AE_TRACK_FLICKER_PERIOD
	{CMD_WRITE, 0xA002, 0x0001, WORD_LEN}, 	// FD_ALGO
	{CMD_WRITE, 0xA401, 0x00, BYTE_LEN}, 	// AE_RULE_MODE                          
	{CMD_WRITE, 0xA409, 0x48, BYTE_LEN},  	// AE_RULE_BASE_TARGET                   
	{CMD_WRITE, 0xA801, 0x00, BYTE_LEN},  	// AE_TRACK_MODE                         
	{CMD_WRITE, 0xA805, 0x0A, BYTE_LEN},  	// AE_TRACK_GATE                         
	{CMD_WRITE, 0xA80E, 0x1A, BYTE_LEN},  	// AE_TRACK_MAX_BLACK_LEVEL              
	{CMD_WRITE, 0xA81A, 0x0733, WORD_LEN}, 	// AE_TRACK_MAX_INT_TIME_ROWS            
	{CMD_WRITE, 0xA818, 0x0350, WORD_LEN}, 	// AE_TRACK_TARGET_INT_TIME_ROWS         
	{CMD_WRITE, 0xA81E, 0x0140, WORD_LEN}, 	// AE_TRACK_TARGET_AGAIN                 
	{CMD_WRITE, 0xA820, 0x014C, WORD_LEN}, 	// AE_TRACK_MAX_AGAIN                    
	{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, 	// AE_TRACK_MIN_DGAIN                   
	{CMD_WRITE, 0xA824, 0x0100, WORD_LEN}, 	// 1B0 AE_TRACK_MAX_DGAIN                   
	{CMD_WRITE, 0xC8E6, 0x014C, WORD_LEN}, 	// CAM_MAX_ANALOG_GAIN                  
	{CMD_WRITE, 0xC8E8, 0x0040, WORD_LEN}, 	// CAM_MIN_ANALOG_GAIN                  
	{CMD_WRITE, 0xBC52, 0x00C8, WORD_LEN}, 	// LL_START_BRIGHTNESS_METRIC           
	{CMD_WRITE, 0xBC58, 0x00FA, WORD_LEN}, 	// LL_START_GAIN_METRIC                 
	{CMD_WRITE, 0xBC5A, 0x0400, WORD_LEN}, 	// LL_END_GAIN_METRIC                   
	{CMD_WRITE, 0xBC5E, 0x00FA, WORD_LEN}, 	// LL_START_APERTURE_GAIN_BM            
	{CMD_WRITE, 0xBC60, 0x0258, WORD_LEN}, 	// LL_END_APERTURE_GAIN_BM              
	{CMD_WRITE, 0xBC66, 0x0154, WORD_LEN}, 	// LL_START_APERTURE_GM                 
	{CMD_WRITE, 0xBC68, 0x0390, WORD_LEN}, 	// LL_END_APERTURE_GM                   
	{CMD_WRITE, 0xBC86, 0x00c0, WORD_LEN}, 	// FA LL_START_FFNR_GM                     
	{CMD_WRITE, 0xBC88, 0x00a0, WORD_LEN}, 	// 390LL_END_FFNR_GM                       
	{CMD_WRITE, 0xBCCC, 0x00C8, WORD_LEN}, 	// LL_SFFB_START_MAX_GM                 
	{CMD_WRITE, 0xBCCE, 0x0390, WORD_LEN}, 	// LL_SFFB_END_MAX_GM                   
	{CMD_WRITE, 0xBC90, 0x00FA, WORD_LEN}, 	// LL_START_GRB_GM                      
	{CMD_WRITE, 0xBC92, 0x0390, WORD_LEN}, 	// LL_END_GRB_GM                        
	{CMD_WRITE, 0xBC0E, 0x0001, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_START_POS         
	{CMD_WRITE, 0xBC10, 0x00F0, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_MID_POS           
	{CMD_WRITE, 0xBC12, 0x0640, WORD_LEN}, 	// LL_GAMMA_CURVE_ADJ_END_POS           
	{CMD_WRITE, 0xBCAA, 0x03e8, WORD_LEN}, 	// LL_CDC_THR_ADJ_START_POS             
	{CMD_WRITE, 0xBCAC, 0x012c, WORD_LEN}, 	// LL_CDC_THR_ADJ_MID_POS               
	{CMD_WRITE, 0xBCAE, 0x0009, WORD_LEN}, 	// LL_CDC_THR_ADJ_END_POS               
	{CMD_WRITE, 0xBCD8, 0x0154, WORD_LEN}, 	// LL_PCR_START_BM                      
	{CMD_WRITE, 0xBCDA, 0x0A28, WORD_LEN}, 	// LL_PCR_END_BM                        
	{CMD_WRITE, 0x3380, 0x0587, WORD_LEN},  // KERNEL_CONFIG                                         
	{CMD_WRITE, 0xBCB0, 0x2C, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SLOPE
	{CMD_WRITE, 0xBCB1, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_SATUR
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0xFF, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB2, 0x20, BYTE_LEN},         // LL_CDC_DARK_CLUS_SLOPE
	{CMD_WRITE, 0xBCB3, 0x3A, BYTE_LEN},         // LL_CDC_DARK_CLUS_SATUR
	{CMD_WRITE, 0xBCB4, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB5, 0x20, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB6, 0x80, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SLOPE
	{CMD_WRITE, 0xBCB7, 0x39, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_LO_LIGHT_SATUR
	{CMD_WRITE, 0xBCB8, 0x3A, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_MID_LIGHT_SATUR
	{CMD_WRITE, 0xBCB9, 0x24, BYTE_LEN},         // LL_CDC_BRIGHT_CLUS_HI_LIGHT_SATUR                                
	{CMD_WRITE, 0xBC94, 0x12, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_0              
	{CMD_WRITE, 0xBC95, 0x0C, BYTE_LEN}, 	// LL_GB_START_THRESHOLD_1              
	{CMD_WRITE, 0xBC9C, 0x37, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_0                
	{CMD_WRITE, 0xBC9D, 0x24, BYTE_LEN}, 	// LL_GB_END_THRESHOLD_1                
	{CMD_WRITE, 0x33B0, 0x2A16, WORD_LEN}, 	// FFNR_ALPHA_BETA                      
	{CMD_WRITE, 0xBC8A, 0x20, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_Y             
	{CMD_WRITE, 0xBC8B, 0x40, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_Y               
	{CMD_WRITE, 0xBC8C, 0xb0, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_YGAIN         
	{CMD_WRITE, 0xBC8D, 0xff, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_YGAIN          
	{CMD_WRITE, 0xBC8E, 0x10, BYTE_LEN}, 	// LL_START_FF_MIX_THRESH_GAIN         
	{CMD_WRITE, 0xBC8F, 0x00, BYTE_LEN}, 	// LL_END_FF_MIX_THRESH_GAIN           
	{CMD_WRITE, 0xBCC0, 0x20, BYTE_LEN}, 	// LL_SFFB_RAMP_START                  
	{CMD_WRITE, 0xBCC1, 0x03, BYTE_LEN}, 	// LL_SFFB_RAMP_STOP                   
	{CMD_WRITE, 0xBCC2, 0x1E, BYTE_LEN}, 	// LL_SFFB_SLOPE_START                
	{CMD_WRITE, 0xBCC3, 0x0F, BYTE_LEN}, 	// LL_SFFB_SLOPE_STOP                  
	{CMD_WRITE, 0xBCC4, 0x0A, BYTE_LEN}, 	// LL_SFFB_THSTART                     
	{CMD_WRITE, 0xBCC5, 0xAF, BYTE_LEN}, 	// LL_SFFB_THSTOP                      
	{CMD_WRITE, 0xBC6A, 0x04, BYTE_LEN}, 	// LL_START_APERTURE_INTEGER_GAIN      
	{CMD_WRITE, 0xBC6B, 0x00, BYTE_LEN}, 	// LL_END_APERTURE_INTEGER_GAIN        
	{CMD_WRITE, 0x33BA, 0x001F, WORD_LEN}, 	// APEDGE_CONTROL                      
	{CMD_WRITE, 0x33BE, 0x0000, WORD_LEN}, 	// UA_KNEE_L                           
	{CMD_WRITE, 0x33C2, 0x4300, WORD_LEN}, 	// UA_WEIGHTS                          
	{CMD_WRITE, 0xBC62, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KPGAIN            
	{CMD_WRITE, 0xBC63, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KPGAIN              
	{CMD_WRITE, 0xBC64, 0x0E, BYTE_LEN}, 	// LL_START_APERTURE_KNGAIN            
	{CMD_WRITE, 0xBC65, 0x1C, BYTE_LEN}, 	// LL_END_APERTURE_KNGAIN              
	{CMD_WRITE, 0xBCE2, 0x0A, BYTE_LEN}, 	// LL_START_POS_KNEE                   
	{CMD_WRITE, 0xBCE3, 0x30, BYTE_LEN}, 	// LL_END_POS_KNEE                     
	{CMD_WRITE, 0xBCE4, 0x0A, BYTE_LEN}, 	// LL_START_NEG_KNEE                   
	{CMD_WRITE, 0xBCE5, 0x30, BYTE_LEN}, 	// LL_END_NEG_KNEE                     
	{CMD_WRITE, 0xBC56, 0x64, BYTE_LEN}, 	// LL_START_CCM_SATURATION             
	{CMD_WRITE, 0xBC57, 0x3C, BYTE_LEN}, 	// LL_END_CCM_SATURATION               
	{CMD_WRITE, 0xBCDE, 0x03, BYTE_LEN}, 	// LL_START_SYS_THRESHOLD              
	{CMD_WRITE, 0xBCDF, 0x50, BYTE_LEN}, 	// LL_STOP_SYS_THRESHOLD               
	{CMD_WRITE, 0xBCE0, 0x08, BYTE_LEN}, 	// LL_START_SYS_GAIN                   
	{CMD_WRITE, 0xBCE1, 0x03, BYTE_LEN}, 	// LL_STOP_SYS_GAIN                    
	{CMD_WRITE, 0xBCC6, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_START            
	{CMD_WRITE, 0xBCC7, 0x00, BYTE_LEN}, 	// LL_SFFB_SHARPENING_STOP             
	{CMD_WRITE, 0xBCC8, 0x2F, BYTE_LEN}, 	// LL_SFFB_FLATNESS_START              
	{CMD_WRITE, 0xBCC9, 0x40, BYTE_LEN}, 	// LL_SFFB_FLATNESS_STOP               
	{CMD_WRITE, 0xBCCA, 0x04, BYTE_LEN}, 	// LL_SFFB_TRANSITION_START            
	{CMD_WRITE, 0xBCCB, 0x00, BYTE_LEN}, 	// LL_SFFB_TRANSITION_STOP             
	{CMD_WRITE, 0xBCE6, 0x00, BYTE_LEN}, 	// LL_SFFB_ZERO_ENABLE                 
	{CMD_WRITE, 0xC8ED, 0x00, BYTE_LEN}, 	// CAM_TX_ENABLE_MODE                  
	{CMD_WRITE, 0xC8BC, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8BD, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_0_JPEG_QSCALE_1          
	{CMD_WRITE, 0xC8D2, 0x04, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_0          
	{CMD_WRITE, 0xC8D3, 0x0A, BYTE_LEN}, 	// CAM_OUTPUT_1_JPEG_QSCALE_1   
	{CMD_WRITE, 0xDC3A, 0x23, BYTE_LEN}, 	// SYS_SEPIA_CR                 
	{CMD_WRITE, 0xDC3B, 0xB2, BYTE_LEN}, 	// SYS_SEPIA_CB                 
	{CMD_WRITE, 0xBC02, 0x01FF, WORD_LEN}, 	// LL_ALGO                      
	{CMD_WRITE, 0x8419, 0x03, BYTE_LEN}, 	// SEQ_STATE_CFG_1_AF           
	{CMD_WRITE, 0xC400, 0x88, BYTE_LEN}, 	// AFM_ALGO                     
	{CMD_WRITE, 0xB002, 0x0002, WORD_LEN}, 	// AF_MODE                      
	{CMD_WRITE, 0xB004, 0x0010, WORD_LEN}, 	// AF_ALGO                      
	{CMD_WRITE, 0xB045, 0x0015, WORD_LEN}, 	// AF_MODE_EX                   
	{CMD_WRITE, 0xB02C, 0x20, BYTE_LEN}, 	// AF_HC_STEP                   
	{CMD_WRITE, 0xB02D, 0x40, BYTE_LEN}, 	// AF_HC_LONG_STEP              
	{CMD_WRITE, 0xB02E, 0x50, BYTE_LEN}, 	// AF_HC_LONG_STEP_USE_THR      
	{CMD_WRITE, 0xB048, 0x01, BYTE_LEN}, 	// AF_C_HC_PROGRESS_TH          
	{CMD_WRITE, 0xB041, 0x02, BYTE_LEN}, 	// AF_DEBUG2                    
	{CMD_WRITE, 0xC856, 0x0423, WORD_LEN}, 	// CAM_CORE_A_UNUSED1           
	{CMD_WRITE, 0xC40A, 0x0028, WORD_LEN}, 	// AFM_POS_MIN                  
	{CMD_WRITE, 0xC40C, 0x00a0, WORD_LEN}, 	// AFM_POS_MAX                  
	{CMD_WRITE, 0xC402, 0x00, BYTE_LEN},  // AFM_MODE                     
	{CMD_WRITE, 0xB018, 0x00, BYTE_LEN}, 	// AF_FS_POS_0                  
	{CMD_WRITE, 0xB019, 0x14, BYTE_LEN}, 	// AF_FS_POS_1                  
	{CMD_WRITE, 0xB01A, 0x21, BYTE_LEN}, 	// AF_FS_POS_2                  
	{CMD_WRITE, 0xB01B, 0x2e, BYTE_LEN}, 	// AF_FS_POS_3                  
	{CMD_WRITE, 0xB01C, 0x3b, BYTE_LEN}, 	// AF_FS_POS_4                  
	{CMD_WRITE, 0xB01D, 0x48, BYTE_LEN}, 	// AF_FS_POS_5                  
	{CMD_WRITE, 0xB01E, 0x55, BYTE_LEN}, 	// AF_FS_POS_6                  
	{CMD_WRITE, 0xB01F, 0x62, BYTE_LEN}, 	// AF_FS_POS_7                  
	{CMD_WRITE, 0xB020, 0x6f, BYTE_LEN}, 	// AF_FS_POS_8                  
	{CMD_WRITE, 0xB021, 0x7c, BYTE_LEN}, 	// AF_FS_POS_9  
	{CMD_WRITE, 0xB022, 0x89, BYTE_LEN},	 	// AF_FS_POS_10 
	{CMD_WRITE, 0xB023, 0x96, BYTE_LEN},	 	// AF_FS_POS_11 
	{CMD_WRITE, 0xB024, 0xa3, BYTE_LEN},	 	// AF_FS_POS_12 
	{CMD_WRITE, 0xB025, 0xb0, BYTE_LEN},	 	// AF_FS_POS_13 
	{CMD_WRITE, 0xB026, 0xbd, BYTE_LEN},	 	// AF_FS_POS_14 
	{CMD_WRITE, 0xB027, 0xd4, BYTE_LEN},	 	// AF_FS_POS_15 
	{CMD_WRITE, 0xB028, 0xe1, BYTE_LEN},	 	// AF_FS_POS_16 
	{CMD_WRITE, 0xB029, 0xee, BYTE_LEN},	 	// AF_FS_POS_17 
	{CMD_WRITE, 0xB02A, 0xfc, BYTE_LEN},	 	// AF_FS_POS_18 
	{CMD_WRITE, 0xB02B, 0xff, BYTE_LEN},	 	// AF_FS_POS_19 
	{CMD_WRITE, 0xB012, 0x14, BYTE_LEN}, 	// AF_FS_NUM_STEPS              
	{CMD_WRITE, 0xB014, 0x0B, BYTE_LEN}, 	// AF_FS_STEP_SIZE              
	{CMD_WRITE, 0xB036, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_STARTING_MOT_EVAL
	{CMD_WRITE, 0xB03C, 0x2430, WORD_LEN}, 	// AF_MD_TH_FOR_FOCUSING_AFTER_MOT               
	{CMD_WRITE, 0xB854, 0x52, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_X         
	{CMD_WRITE, 0xB855, 0x58, BYTE_LEN}, 	// STAT_SM_WINDOW_POS_Y         
	{CMD_WRITE, 0xB856, 0x5D, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_X        
	{CMD_WRITE, 0xB857, 0x5A, BYTE_LEN}, 	// STAT_SM_WINDOW_SIZE_Y
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},     // STANDBY_CONTROL_AND_STATUS 
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

struct mt9p111_register_pair const lens_check_settings_array[] =
{
	{CMD_WRITE, 0xE02A, 0x0001, WORD_LEN},
	{CMD_DELAY, 0x00, 50, 0},
};

struct mt9p111_register_pair const lens_zone0_settings_array[] =
{
	{CMD_WRITE, 0xD004, 0x04, BYTE_LEN},
	{CMD_WRITE, 0xD006, 0x0000, WORD_LEN},
	{CMD_WRITE, 0xD005, 0x00, BYTE_LEN},
	{CMD_WRITE, 0xD002, 0x8002, WORD_LEN},
	{CMD_WRITE, 0x3210, 0x49B8, WORD_LEN},
};

struct mt9p111_register_pair const lens_default_settings_array[] =
{
	//[Defalut_Lens_Correction] //golden LSC
	{CMD_WRITE, 0x3640, 0x02b0, WORD_LEN},
	{CMD_WRITE, 0x3642, 0x07ee, WORD_LEN},
	{CMD_WRITE, 0x3644, 0x3d71, WORD_LEN},
	{CMD_WRITE, 0x3646, 0xcaeb, WORD_LEN},
	{CMD_WRITE, 0x3648, 0xfe50, WORD_LEN},
	{CMD_WRITE, 0x364a, 0x02f0, WORD_LEN},
	{CMD_WRITE, 0x364c, 0xde8c, WORD_LEN},
	{CMD_WRITE, 0x364e, 0x0211, WORD_LEN},
	{CMD_WRITE, 0x3650, 0x6f4d, WORD_LEN},
	{CMD_WRITE, 0x3652, 0xf070, WORD_LEN},
	{CMD_WRITE, 0x3654, 0x03b0, WORD_LEN},
	{CMD_WRITE, 0x3656, 0x6e8d, WORD_LEN},
	{CMD_WRITE, 0x3658, 0x56d0, WORD_LEN},
	{CMD_WRITE, 0x365a, 0xf36d, WORD_LEN},
	{CMD_WRITE, 0x365c, 0x98f0, WORD_LEN},
	{CMD_WRITE, 0x365e, 0x0150, WORD_LEN},
	{CMD_WRITE, 0x3660, 0xad4d, WORD_LEN},
	{CMD_WRITE, 0x3662, 0x3f71, WORD_LEN},
	{CMD_WRITE, 0x3664, 0x016d, WORD_LEN},
	{CMD_WRITE, 0x3666, 0xfa30, WORD_LEN},
	{CMD_WRITE, 0x3680, 0x1fea, WORD_LEN},
	{CMD_WRITE, 0x3682, 0xe0cd, WORD_LEN},
	{CMD_WRITE, 0x3684, 0x20ac, WORD_LEN},
	{CMD_WRITE, 0x3686, 0x192e, WORD_LEN},
	{CMD_WRITE, 0x3688, 0x912e, WORD_LEN},
	{CMD_WRITE, 0x368a, 0x4ead, WORD_LEN},
	{CMD_WRITE, 0x368c, 0x316d, WORD_LEN},
	{CMD_WRITE, 0x368e, 0xfa09, WORD_LEN},
	{CMD_WRITE, 0x3690, 0xcb0e, WORD_LEN},
	{CMD_WRITE, 0x3692, 0xb90e, WORD_LEN},
	{CMD_WRITE, 0x3694, 0xa4ce, WORD_LEN},
	{CMD_WRITE, 0x3696, 0x598d, WORD_LEN},
	{CMD_WRITE, 0x3698, 0x60af, WORD_LEN},
	{CMD_WRITE, 0x369a, 0xf2ce, WORD_LEN},
	{CMD_WRITE, 0x369c, 0x9330, WORD_LEN},
	{CMD_WRITE, 0x369e, 0xb82d, WORD_LEN},
	{CMD_WRITE, 0x36a0, 0xa86e, WORD_LEN},
	{CMD_WRITE, 0x36a2, 0x362f, WORD_LEN},
	{CMD_WRITE, 0x36a4, 0x3d0d, WORD_LEN},
	{CMD_WRITE, 0x36a6, 0xf6ef, WORD_LEN},
	{CMD_WRITE, 0x36c0, 0x5f11, WORD_LEN},
	{CMD_WRITE, 0x36c2, 0x0a4f, WORD_LEN},
	{CMD_WRITE, 0x36c4, 0xb572, WORD_LEN},
	{CMD_WRITE, 0x36c6, 0xa16b, WORD_LEN},
	{CMD_WRITE, 0x36c8, 0x0ed3, WORD_LEN},
	{CMD_WRITE, 0x36ca, 0x25b1, WORD_LEN},
	{CMD_WRITE, 0x36cc, 0xf18e, WORD_LEN},
	{CMD_WRITE, 0x36ce, 0x90d2, WORD_LEN},
	{CMD_WRITE, 0x36d0, 0x276f, WORD_LEN},
	{CMD_WRITE, 0x36d2, 0x2313, WORD_LEN},
	{CMD_WRITE, 0x36d4, 0x2311, WORD_LEN},
	{CMD_WRITE, 0x36d6, 0x126f, WORD_LEN},
	{CMD_WRITE, 0x36d8, 0xf8d1, WORD_LEN},
	{CMD_WRITE, 0x36da, 0x870e, WORD_LEN},
	{CMD_WRITE, 0x36dc, 0x12f3, WORD_LEN},
	{CMD_WRITE, 0x36de, 0x61f1, WORD_LEN},
	{CMD_WRITE, 0x36e0, 0x92af, WORD_LEN},
	{CMD_WRITE, 0x36e2, 0xa1f2, WORD_LEN},
	{CMD_WRITE, 0x36e4, 0x1310, WORD_LEN},
	{CMD_WRITE, 0x36e6, 0x6872, WORD_LEN},
	{CMD_WRITE, 0x3700, 0x364f, WORD_LEN},
	{CMD_WRITE, 0x3702, 0xa04b, WORD_LEN},
	{CMD_WRITE, 0x3704, 0xba6d, WORD_LEN},
	{CMD_WRITE, 0x3706, 0x6bf0, WORD_LEN},
	{CMD_WRITE, 0x3708, 0x91f2, WORD_LEN},
	{CMD_WRITE, 0x370a, 0x1e0f, WORD_LEN},
	{CMD_WRITE, 0x370c, 0x954e, WORD_LEN},
	{CMD_WRITE, 0x370e, 0xdbaf, WORD_LEN},
	{CMD_WRITE, 0x3710, 0x1ef1, WORD_LEN},
	{CMD_WRITE, 0x3712, 0x8a51, WORD_LEN},
	{CMD_WRITE, 0x3714, 0xf66c, WORD_LEN},
	{CMD_WRITE, 0x3716, 0xc10e, WORD_LEN},
	{CMD_WRITE, 0x3718, 0x1bd0, WORD_LEN},
	{CMD_WRITE, 0x371a, 0x0251, WORD_LEN},
	{CMD_WRITE, 0x371c, 0xb5f2, WORD_LEN},
	{CMD_WRITE, 0x371e, 0x736e, WORD_LEN},
	{CMD_WRITE, 0x3720, 0xa4ce, WORD_LEN},
	{CMD_WRITE, 0x3722, 0x430f, WORD_LEN},
	{CMD_WRITE, 0x3724, 0x45b1, WORD_LEN},
	{CMD_WRITE, 0x3726, 0xaed2, WORD_LEN},
	{CMD_WRITE, 0x3740, 0x9a11, WORD_LEN},
	{CMD_WRITE, 0x3742, 0x9350, WORD_LEN},
	{CMD_WRITE, 0x3744, 0x8d31, WORD_LEN},
	{CMD_WRITE, 0x3746, 0xd04e, WORD_LEN},
	{CMD_WRITE, 0x3748, 0x1174, WORD_LEN},
	{CMD_WRITE, 0x374a, 0x8811, WORD_LEN},
	{CMD_WRITE, 0x374c, 0x292f, WORD_LEN},
	{CMD_WRITE, 0x374e, 0x776f, WORD_LEN},
	{CMD_WRITE, 0x3750, 0x9e71, WORD_LEN},
	{CMD_WRITE, 0x3752, 0x21f3, WORD_LEN},
	{CMD_WRITE, 0x3754, 0xb8d0, WORD_LEN},
	{CMD_WRITE, 0x3756, 0xa010, WORD_LEN},
	{CMD_WRITE, 0x3758, 0x9f0b, WORD_LEN},
	{CMD_WRITE, 0x375a, 0x7190, WORD_LEN},
	{CMD_WRITE, 0x375c, 0x47f3, WORD_LEN},
	{CMD_WRITE, 0x375e, 0x89f1, WORD_LEN},
	{CMD_WRITE, 0x3760, 0x5d0e, WORD_LEN},
	{CMD_WRITE, 0x3762, 0x8c12, WORD_LEN},
	{CMD_WRITE, 0x3764, 0xa891, WORD_LEN},
	{CMD_WRITE, 0x3766, 0x3d54, WORD_LEN},
	{CMD_WRITE, 0x3782, 0x03b4, WORD_LEN},
	{CMD_WRITE, 0x3784, 0x04f4, WORD_LEN},
};

static struct mt9p111_register_pair const preview_mode_settings_array[] = 
{
	//{CMD_WRITE, 0x3EDA, 0x6060, WORD_LEN}, // DAC_LD_14_15
	{CMD_WRITE, 0x098E, 0x843C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_5_MAX_FRAME_CNT]
	{CMD_WRITE, 0x843C, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_5_MAX_FRAME_CNT
	{CMD_WRITE, 0x8404, 0x01, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x3170, 0x2096, WORD_LEN} // ANALOG_CONTROL
};

static struct mt9p111_register_pair const snapshot_mode_settings_array[] =
{
	//{CMD_WRITE, 0xB006, 0x01, BYTE_LEN}, // AF_PROGRESS
	//{CMD_POLL, 0xB006, 0x00, 100},
	//{CMD_WRITE, 0x3EDA, 0x6060, WORD_LEN}, // DAC_LD_14_15
	{CMD_WRITE, 0x098E, 0x843C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_5_MAX_FRAME_CNT]
	{CMD_WRITE, 0x843C, 0xFF, BYTE_LEN}, // SEQ_STATE_CFG_5_MAX_FRAME_CNT
	{CMD_WRITE, 0x8404, 0x02, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x3170, 0x205C, WORD_LEN} // ANALOG_CONTROL
};

static struct mt9p111_register_pair const af_macro_mode_settings_array[] =
{
	{CMD_WRITE, 0x098E, 0x8419, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_1_AF]
	{CMD_WRITE, 0x8419, 0x05, BYTE_LEN}, // SEQ_STATE_CFG_1_AF
	{CMD_WRITE, 0xB018, 0x80 , BYTE_LEN}, // AF_FS_POS_0
	{CMD_WRITE, 0xB019, 0x98 , BYTE_LEN}, // AF_FS_POS_1
	{CMD_WRITE, 0xB01A, 0xb0 , BYTE_LEN}, // AF_FS_POS_2
	{CMD_WRITE, 0xB01B, 0xc8 , BYTE_LEN}, // AF_FS_POS_3
	{CMD_WRITE, 0xB01C, 0xe0 , BYTE_LEN}, // AF_FS_POS_4
	{CMD_WRITE, 0xB012, 0x05 , BYTE_LEN}, // AF_FS_NUM_STEPS [05step]	 //AF_postition_settings
	{CMD_WRITE, 0xB016, 0x0400 , WORD_LEN}, // AF_th_ave_max_sha 
	{CMD_WRITE, 0x8404, 0x06	, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const af_auto_mode_settings_array[] =
{
	//AF_VCM_enable
	{CMD_WRITE, 0xC400, 0x88, BYTE_LEN}, // AFM_ALGO
	{CMD_WRITE, 0x8419, 0x05, BYTE_LEN}, // SEQ_STATE_CFG_1_AF
	{CMD_WRITE, 0xC400, 0x08, BYTE_LEN}, // AFM_ALGO
	//AF_settings
	{CMD_WRITE, 0xB002, 0x0301, WORD_LEN}, // AF_MODE
	{CMD_WRITE, 0xB004, 0x0002, WORD_LEN}, // AF_ALGO
	{CMD_WRITE, 0xB045, 0x0015, WORD_LEN}, //
	//set_posMin/Max
	{CMD_WRITE, 0xC40A, 0x0028, WORD_LEN}, // AFM_POS_MIN
	{CMD_WRITE, 0xC40C, 0x00BE, WORD_LEN}, // AFM_POS_MAX

	{CMD_WRITE, 0xB018, 0x00 , BYTE_LEN}, // AF_FS_POS_0
	{CMD_WRITE, 0xB019, 0x1E , BYTE_LEN}, // AF_FS_POS_1
	{CMD_WRITE, 0xB01A, 0x2D , BYTE_LEN}, // AF_FS_POS_2
	{CMD_WRITE, 0xB01B, 0x3C , BYTE_LEN}, // AF_FS_POS_3
	{CMD_WRITE, 0xB01C, 0x4B , BYTE_LEN}, // AF_FS_POS_4
	{CMD_WRITE, 0xB01D, 0x5A , BYTE_LEN}, // AF_FS_POS_5
	{CMD_WRITE, 0xB01E, 0x69 , BYTE_LEN}, // AF_FS_POS_6
	{CMD_WRITE, 0xB01F, 0x78 , BYTE_LEN}, // AF_FS_POS_7
	{CMD_WRITE, 0xB020, 0x87 , BYTE_LEN}, // AF_FS_POS_8
	{CMD_WRITE, 0xB021, 0x96 , BYTE_LEN}, // AF_FS_POS_9
	{CMD_WRITE, 0xB022, 0xA5 , BYTE_LEN}, // AF_FS_POS_10
	{CMD_WRITE, 0xB023, 0xB4 , BYTE_LEN}, // AF_FS_POS_11
	{CMD_WRITE, 0xB024, 0xC3 , BYTE_LEN}, // AF_FS_POS_12
	{CMD_WRITE, 0xB012, 0x0D , BYTE_LEN}, // AF_FS_NUM_STEPS [13step]	 //AF_postition_settings
	{CMD_WRITE, 0xB014, 0x0B , BYTE_LEN}, // AF_FS_STEP_SIZE
	{CMD_WRITE, 0xB007, 0x00 , BYTE_LEN},

	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
//	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const af_continuous_video_mode_settings_array[] = 
{
	{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
	{CMD_WRITE, 0xB007, 0x00, BYTE_LEN}, // AF_FS_POS_0
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}	
};

static struct mt9p111_register_pair const af_continuous_camera_mode_settings_array[] = 
{
	{CMD_WRITE, 0x8419, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_1_AF
	{CMD_WRITE, 0xC400, 0x88, BYTE_LEN}, // AFM_ALGO
	{CMD_WRITE, 0xB002, 0x0002, WORD_LEN}, // AF_MODE
	{CMD_WRITE, 0xB004, 0x0010, WORD_LEN}, // AF_ALGO
	{CMD_WRITE, 0xB045, 0x0015, WORD_LEN}, 	// AF_MODE_EX                   
	{CMD_WRITE, 0xB02C, 0x20, BYTE_LEN}, // AF_HC_STEP
	{CMD_WRITE, 0xB02D, 0x40, BYTE_LEN}, // AF_HC_LONG_STEP
	{CMD_WRITE, 0xB02E, 0x50, BYTE_LEN}, // AF_HC_LONG_STEP_USE_THR	
	{CMD_WRITE, 0xB048, 0x01, BYTE_LEN}, // AF_C_HC_PROGRESS_TH	
	{CMD_WRITE, 0xB041, 0x02, BYTE_LEN}, // AF_DEBUG2
	{CMD_WRITE, 0xC856, 0x0423, WORD_LEN}, // CAM_CORE_A_UNUSED1
	{CMD_WRITE, 0xC40A, 0x0028, WORD_LEN}, // AFM_POS_MIN
	{CMD_WRITE, 0xC40C, 0x00a0, WORD_LEN}, 	// AFM_POS_MAX                  
	{CMD_WRITE, 0xC402, 0x00, BYTE_LEN}, // AFM_MODE
	{CMD_WRITE, 0xB018, 0x00, BYTE_LEN}, // AF_FS_POS_0
	{CMD_WRITE, 0xB019, 0x14, BYTE_LEN}, 	// AF_FS_POS_1                  
	{CMD_WRITE, 0xB01A, 0x21, BYTE_LEN}, 	// AF_FS_POS_2                  
	{CMD_WRITE, 0xB01B, 0x2e, BYTE_LEN}, 	// AF_FS_POS_3                  
	{CMD_WRITE, 0xB01C, 0x3b, BYTE_LEN}, 	// AF_FS_POS_4                  
	{CMD_WRITE, 0xB01D, 0x48, BYTE_LEN}, 	// AF_FS_POS_5                  
	{CMD_WRITE, 0xB01E, 0x55, BYTE_LEN}, 	// AF_FS_POS_6                  
	{CMD_WRITE, 0xB01F, 0x62, BYTE_LEN}, 	// AF_FS_POS_7                  
	{CMD_WRITE, 0xB020, 0x6f, BYTE_LEN}, 	// AF_FS_POS_8                  
	{CMD_WRITE, 0xB021, 0x7c, BYTE_LEN}, 	// AF_FS_POS_9  
	{CMD_WRITE, 0xB022, 0x89, BYTE_LEN},	 	// AF_FS_POS_10 
	{CMD_WRITE, 0xB023, 0x96, BYTE_LEN},	 	// AF_FS_POS_11 
	{CMD_WRITE, 0xB024, 0xa3, BYTE_LEN},	 	// AF_FS_POS_12 
	{CMD_WRITE, 0xB025, 0xb0, BYTE_LEN},	 	// AF_FS_POS_13 
	{CMD_WRITE, 0xB026, 0xbd, BYTE_LEN},	 	// AF_FS_POS_14 
	{CMD_WRITE, 0xB027, 0xd4, BYTE_LEN},	 	// AF_FS_POS_15 
	{CMD_WRITE, 0xB028, 0xe1, BYTE_LEN},	 	// AF_FS_POS_16 
	{CMD_WRITE, 0xB029, 0xee, BYTE_LEN},	 	// AF_FS_POS_17 
	{CMD_WRITE, 0xB02A, 0xfc, BYTE_LEN},	 	// AF_FS_POS_18 
	{CMD_WRITE, 0xB02B, 0xff, BYTE_LEN},	 	// AF_FS_POS_19 
	{CMD_WRITE, 0xB012, 0x14, BYTE_LEN}, 	// AF_FS_NUM_STEPS              
	{CMD_WRITE, 0xB014, 0x0B, BYTE_LEN}, //AF_FS_STEP_SIZE
	{CMD_WRITE, 0xB854, 0x52, BYTE_LEN}, // STAT_SM_WINDOW_POS_X
	{CMD_WRITE, 0xB855, 0x58, BYTE_LEN}, // STAT_SM_WINDOW_POS_Y
	{CMD_WRITE, 0xB856, 0x5D, BYTE_LEN}, // STAT_SM_WINDOW_SIZE_X
	{CMD_WRITE, 0xB857, 0x5A, BYTE_LEN}, // STAT_SM_WINDOW_SIZE_Y
	{CMD_WRITE, 0x0018, 0x2008, WORD_LEN},  // STANDBY_CONTROL_AND_STATUS 
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
//	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const wb_auto_mode_settings_array[] =
{
	// [MT9P111_WB_AUTO]
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const wb_cloudy_mode_settings_array[] =
{
	// [MT9P111_WB_CLOUDY]
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x7F, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0xAC04, 0x3d, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
	{CMD_WRITE, 0xAC05, 0x56, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
};

static struct mt9p111_register_pair const wb_daylight_mode_settings_array[] =
{
	// [MT9P111_WB_DAYLIGHT]
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x7F, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0xAC04, 0x3D, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
	{CMD_WRITE, 0xAC05, 0x46, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
};

static struct mt9p111_register_pair const wb_fluorescent_mode_settings_array[] =
{
	// [MT9P111_WB_FLUORESCENT]
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x50, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x50, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0xAC04, 0x4A, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
	{CMD_WRITE, 0xAC05, 0x34, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
};

static struct mt9p111_register_pair const wb_incandescent_mode_settings_array[] =
{
	// [MT9P111_WB_INCANDESCENT]
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x50, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x50, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0xAC04, 0x4c, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
	{CMD_WRITE, 0xAC05, 0x30, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
};

static struct mt9p111_register_pair const effect_off_mode_settings_array[] =
{
	// [MT9P111_EFFECT_NONE]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x00, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_gray_mode_settings_array[] =
{
	//[MT9P111_EFFECT_GRAY]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x01, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_green_mode_settings_array[] =
{
	// [MT9P111_EFFECT_GREEN]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0xC0, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0xC0, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_cool_mode_settings_array[] =
{
	// [MT9P111_EFFECT_COOL]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0xBE, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0x30, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_yellow_mode_settings_array[] =
{
	// [MT9P111_EFFECT_YELLOW]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0x20, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0x81, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_sepia_mode_settings_array[] =
{
	// [MT9P111_EFFECT_SEPIA]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0x23, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0xB2, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_purple_mode_settings_array[] =
{
	// [MT9P111_EFFECT_PURPLE]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0x40, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0x40, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_red_mode_settings_array[] =
{
	// [MT9P111_EFFECT_RED]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0x54, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0xD0, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_pink_mode_settings_array[] =
{
	// [MT9P111_EFFECT_PINK]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0x42, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0x13, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_aqua_mode_settings_array[] =
{
	// [MT9P111_EFFECT_AQUA]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x02, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC3A, 0xB7, BYTE_LEN}, // SYS_SEPIA_CR
	{CMD_WRITE, 0xDC3B, 0x39, BYTE_LEN}, // SYS_SEPIA_CB
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_negative_mode_settings_array[] =
{
	// [MT9P111_EFFECT_NEGATIVE]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x03, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const effect_solarize_mode_settings_array[] =
{
	// [MT9P111_EFFECT_SOLARIZE_1]
	{CMD_WRITE, 0x098E, 0xDC38, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_SELECT_FX]
	{CMD_WRITE, 0xDC38, 0x04, BYTE_LEN}, // SYS_SELECT_FX
	{CMD_WRITE, 0xDC02, 0x006E, WORD_LEN}, // SYS_ALGO
	{CMD_WRITE, 0xDC39, 0x00, BYTE_LEN}, // SYS_SOLARIZATION_TH
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const exposure_normal_mode_settings_array[] =
{
	// [MT9P111_EXPOSURE_NORMAL] 
	{CMD_WRITE, 0x098E, 0xB820, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS
	{CMD_WRITE, 0xB820, 0x00, BYTE_LEN}, // RESERVED_STAT_20
	{CMD_WRITE, 0xB821, 0x00, BYTE_LEN}, // RESERVED_STAT_21
	{CMD_WRITE, 0xB822, 0xFF, BYTE_LEN}, // RESERVED_STAT_22
	{CMD_WRITE, 0xB823, 0xEF, BYTE_LEN}, // RESERVED_STAT_23
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const exposure_spot_mode_settings_array[] =
{
	// [MT9P111_EXPOSURE_SPOT]
	{CMD_WRITE, 0x098E, 0xB820, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS
	{CMD_WRITE, 0xB820, 0x66, BYTE_LEN}, // RESERVED_STAT_20
	{CMD_WRITE, 0xB821, 0x66, BYTE_LEN}, // RESERVED_STAT_21
	{CMD_WRITE, 0xB822, 0x33, BYTE_LEN}, // RESERVED_STAT_22
	{CMD_WRITE, 0xB823, 0x33, BYTE_LEN}, // RESERVED_STAT_23
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const exposure_avg_mode_settings_array[] =
{
	// [MT9P111_EXPOSURE_AVERAGE]
	{CMD_WRITE, 0x098E, 0xB820, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS
	{CMD_WRITE, 0xB820, 0x14, BYTE_LEN}, // RESERVED_STAT_20
	{CMD_WRITE, 0xB821, 0x14, BYTE_LEN}, // RESERVED_STAT_21
	{CMD_WRITE, 0xB822, 0xD2, BYTE_LEN}, // RESERVED_STAT_22
	{CMD_WRITE, 0xB823, 0xD2, BYTE_LEN}, // RESERVED_STAT_23
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const scene_auto_mode_settings_array[] =
{
	// [Normal]
	{CMD_WRITE, 0xA81A, 0x07b3, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x840E, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0xBC56, 0x64, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const scene_landscape_mode_settings_array[] =
{
	// [Landscape]
	{CMD_WRITE, 0xA81A, 0x08B0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x098E, 0x840E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AE]
	{CMD_WRITE, 0x840E, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0x098E, 0xBC56, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [LL_START_CCM_SATURATION]
	{CMD_WRITE, 0xBC56, 0xA0, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const scene_sunset_mode_settings_array[] =
{
	// [Sunset]
	{CMD_WRITE, 0xA81A, 0x08B0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x098E, 0x840E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AE]
	{CMD_WRITE, 0x840E, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x01, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x7F, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x098E, 0x840E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AE]
	{CMD_WRITE, 0xAC04, 0x3D, BYTE_LEN}, // AWB_PRE_AWB_R2G_RATIO
	{CMD_WRITE, 0xAC05, 0x46, BYTE_LEN}, // AWB_PRE_AWB_B2G_RATIO
	{CMD_WRITE, 0x098E, 0xBC56, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [LL_START_CCM_SATURATION]
	{CMD_WRITE, 0xBC56, 0x80, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const scene_night_mode_settings_array[] =
{
	// [Night]
	{CMD_WRITE, 0xA81A, 0x0BC0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS 7.5fps
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x840E, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x03, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0xBC56, 0x50, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const scene_portrait_mode_settings_array[] =
{
	// [Portrait]
	{CMD_WRITE, 0xA81A, 0x08B0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x098E, 0x840E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AE]
	{CMD_WRITE, 0x840E, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0x098E, 0xBC56, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [LL_START_CCM_SATURATION]
	{CMD_WRITE, 0xBC56, 0x60, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const scene_sports_mode_settings_array[] =
{
	// [Sport]
	{CMD_WRITE, 0xA81A, 0x06B0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x098E, 0x8410, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AWB]
	{CMD_WRITE, 0x8410, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AWB
	{CMD_WRITE, 0x8418, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AWB
	{CMD_WRITE, 0x8420, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_2_AWB
	{CMD_WRITE, 0xAC44, 0x00, BYTE_LEN}, // AWB_LEFT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0xAC45, 0x7F, BYTE_LEN}, // AWB_RIGHT_CCM_POS_RANGE_LIMIT
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
	{CMD_WRITE, 0x098E, 0x840E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SEQ_STATE_CFG_0_AE]
	{CMD_WRITE, 0x840E, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_0_AE
	{CMD_WRITE, 0x8416, 0x02, BYTE_LEN}, // SEQ_STATE_CFG_1_AE
	{CMD_WRITE, 0x098E, 0xBC56, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [LL_START_CCM_SATURATION]
	{CMD_WRITE, 0xBC56, 0x80, BYTE_LEN}, // LL_START_CCM_SATURATION
	{CMD_WRITE, 0x8404, 0x05, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100},
};

static struct mt9p111_register_pair const variable_frame_settings_array[] =
{
	// [MT9P111_Variable]
	{CMD_WRITE, 0x098E, 0x485C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS
	{CMD_WRITE, 0xC85C, 0x0423, WORD_LEN}, // CAM_CORE_A_MIN_FRAME_LENGTH_LINES
	{CMD_WRITE, 0xA818, 0x0350, WORD_LEN}, // AE_TRACK_TARGET_INT_TIME_ROWS
	{CMD_WRITE, 0xA81A, 0x08B0, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}	
};

static struct mt9p111_register_pair const fixed_frame_settings_array[] =
{
	// [MT9P111_FFPS_25]
	{CMD_WRITE, 0xA81A, 0x0428, WORD_LEN}, // AE_TRACK_MAX_INT_TIME_ROWS
	{CMD_WRITE, 0xC85C, 0x0423, WORD_LEN}, // CAM_CORE_A_MIN_FRAME_LENGTH_LINES
	{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
	{CMD_POLL, 0x8404, 0x00, 100}
};

static struct mt9p111_register_pair const brightness_settings_array[][2] =
{
	{
		// [MT9P111_BRIGHT_M5]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x00AF, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_M4]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x00BF, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_M3]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x00CF, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_M2]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x00DF, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_M1]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x00EF, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_0]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0000, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_P1]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0010, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_P2]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0020, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_P3]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0030, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_P4]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0040, WORD_LEN} // SYS_RESERVED1
	},
	{
		// [MT9P111_BRIGHT_P5]
		{CMD_WRITE, 0x098E, 0x5C52, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_RESERVED1]
		{CMD_WRITE, 0xDC52, 0x0050, WORD_LEN} // SYS_RESERVED1
	}
};

static struct mt9p111_register_pair const zoom_settings_array[][2] =
{
	{
		// [1.0x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x0064, WORD_LEN} // SYS_ZOOM_FACTOR
	},
	{
		// [1.2x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x0078, WORD_LEN} // SYS_ZOOM_FACTOR
	},
	{
		// [1.4x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x008C, WORD_LEN} // SYS_ZOOM_FACTOR
	},
	{
		// [1.6x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x00A0, WORD_LEN} // SYS_ZOOM_FACTOR
	},
	{
		// [1.8x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x00B4, WORD_LEN} // SYS_ZOOM_FACTOR
	},
	{
		// [2.0x]
		{CMD_WRITE, 0x098E, 0x5C18, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [SYS_ZOOM_FACTOR]
		{CMD_WRITE, 0xDC18, 0x00C8, WORD_LEN} // SYS_ZOOM_FACTOR
	}
};

static struct mt9p111_register_pair const iso_settings_array[][8] =
{
	{
		// [Auto]
		{CMD_WRITE, 0x098E, 0x281C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81C, 0x0040, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81E, 0x00C8, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x014C, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x00A0, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	},
	{
		// [SOC ISO 50]
		{CMD_WRITE, 0x098E, 0x281C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81C, 0x0029, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81E, 0x0029, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x0029, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x0080, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	},
	{
		// [SOC ISO 100]
		{CMD_WRITE, 0x098E, 0x281C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81C, 0x0050, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81E, 0x0050, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x0050, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x0080, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	},
	{
		// [SOC ISO 200]
		{CMD_WRITE, 0x098E, 0x281C, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81C, 0x0060, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81E, 0x0060, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x0060, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x0080, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	},
	{
		// [SOC ISO 400]
		{CMD_WRITE, 0x098E, 0x281E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81E, 0x0070, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81C, 0x0070, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x0070, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0080, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x0080, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	},
	{
		// [SOC ISO 800]
		{CMD_WRITE, 0x098E, 0x281E, WORD_LEN}, // LOGICAL_ADDRESS_ACCESS [AE_TRACK_MIN_AGAIN]
		{CMD_WRITE, 0xA81E, 0x015E, WORD_LEN}, // AE_TRACK_MIN_AGAIN
		{CMD_WRITE, 0xA81C, 0x015E, WORD_LEN}, // AE_TRACK_TARGET_AGAIN
		{CMD_WRITE, 0xA820, 0x015E, WORD_LEN}, // AE_TRACK_MAX_AGAIN
		{CMD_WRITE, 0xA822, 0x0100, WORD_LEN}, // AE_TRACK_MIN_DGAIN
		{CMD_WRITE, 0xA824, 0x0100, WORD_LEN}, // AE_TRACK_MAX_DGAIN
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100},
	}
};

static struct mt9p111_register_pair const af_manual_settings_array[][4] =
{
	{
		// [manual_AF_01]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x00, BYTE_LEN}, // AF_FS_POS_0
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_02]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x1E, BYTE_LEN}, // AF_FS_POS_1
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_03]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x3A, BYTE_LEN}, // AF_FS_POS_2
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_04]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x55, BYTE_LEN}, // AF_FS_POS_3
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_05]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x66, BYTE_LEN}, // AF_FS_POS_4
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_06]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x78, BYTE_LEN}, // AF_FS_POS_5
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_07]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0x90, BYTE_LEN}, // AF_FS_POS_6
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_08]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0xA0, BYTE_LEN}, // AF_FS_POS_7
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_09]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0xB0, BYTE_LEN}, // AF_FS_POS_8
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	},
	{
		// [manual_AF_10]
		{CMD_WRITE, 0x8419, 0x01, BYTE_LEN},
		{CMD_WRITE, 0xB007, 0xC3, BYTE_LEN}, // AF_FS_POS_9
		{CMD_WRITE, 0x8404, 0x06, BYTE_LEN}, // SEQ_CMD
		{CMD_POLL, 0x8404, 0x00, 100}
	}
};

struct mt9p111_reg mt9p111_regs = 
{
	.init_settings = init_mode_settings_array,
	.init_settings_size = ARRAY_SIZE(init_mode_settings_array),

	.init2_settings = init2_mode_settings_array,
	.init2_settings_size = ARRAY_SIZE(init2_mode_settings_array),

	.prev_settings = preview_mode_settings_array,
	.prev_settings_size = ARRAY_SIZE(preview_mode_settings_array),
	
	.snap_settings = snapshot_mode_settings_array,
	.snap_settings_size = ARRAY_SIZE(snapshot_mode_settings_array),
	
	.af_macro_settings = af_macro_mode_settings_array,
	.af_macro_settings_size = ARRAY_SIZE(af_macro_mode_settings_array),
	
	.af_auto_settings = af_auto_mode_settings_array,
	.af_auto_settings_size = ARRAY_SIZE(af_auto_mode_settings_array),

	.af_continuous_video_settings = af_continuous_video_mode_settings_array,
	.af_continuous_video_settings_size = ARRAY_SIZE(af_continuous_video_mode_settings_array),

	.af_continuous_camera_settings = af_continuous_camera_mode_settings_array,
	.af_continuous_camera_settings_size = ARRAY_SIZE(af_continuous_camera_mode_settings_array),

	.wb_auto_settings = wb_auto_mode_settings_array,
	.wb_auto_settings_size = ARRAY_SIZE(wb_auto_mode_settings_array),

	.wb_cloudy_settings = wb_cloudy_mode_settings_array,
	.wb_cloudy_settings_size = ARRAY_SIZE(wb_cloudy_mode_settings_array),

	.wb_daylight_settings = wb_daylight_mode_settings_array,
	.wb_daylight_settings_size = ARRAY_SIZE(wb_daylight_mode_settings_array),

	.wb_fluorescent_settings = wb_fluorescent_mode_settings_array,
	.wb_fluorescent_settings_size = ARRAY_SIZE(wb_fluorescent_mode_settings_array),

	.wb_incandescent_settings = wb_incandescent_mode_settings_array,
	.wb_incandescent_settings_size = ARRAY_SIZE(wb_incandescent_mode_settings_array),

	.effect_off_settings = effect_off_mode_settings_array,
	.effect_off_settings_size = ARRAY_SIZE(effect_off_mode_settings_array),

	.effect_gray_settings = effect_gray_mode_settings_array,
	.effect_gray_settings_size = ARRAY_SIZE(effect_gray_mode_settings_array),

	.effect_green_settings = effect_green_mode_settings_array,
	.effect_green_settings_size = ARRAY_SIZE(effect_green_mode_settings_array),

	.effect_cool_settings = effect_cool_mode_settings_array,
	.effect_cool_settings_size = ARRAY_SIZE(effect_cool_mode_settings_array),

	.effect_yellow_settings = effect_yellow_mode_settings_array,
	.effect_yellow_settings_size = ARRAY_SIZE(effect_yellow_mode_settings_array),

	.effect_sepia_settings = effect_sepia_mode_settings_array,
	.effect_sepia_settings_size = ARRAY_SIZE(effect_sepia_mode_settings_array),

	.effect_purple_settings = effect_purple_mode_settings_array,
	.effect_purple_settings_size = ARRAY_SIZE(effect_purple_mode_settings_array),

	.effect_red_settings = effect_red_mode_settings_array,
	.effect_red_settings_size = ARRAY_SIZE(effect_red_mode_settings_array),

	.effect_pink_settings = effect_pink_mode_settings_array,
	.effect_pink_settings_size = ARRAY_SIZE(effect_pink_mode_settings_array),

	.effect_aqua_settings = effect_aqua_mode_settings_array,
	.effect_aqua_settings_size = ARRAY_SIZE(effect_aqua_mode_settings_array),

	.effect_negative_settings = effect_negative_mode_settings_array,
	.effect_negative_settings_size = ARRAY_SIZE(effect_negative_mode_settings_array),

	.effect_solarize_settings = effect_solarize_mode_settings_array,
	.effect_solarize_settings_size = ARRAY_SIZE(effect_solarize_mode_settings_array),

	.exposure_normal_settings = exposure_normal_mode_settings_array,
	.exposure_normal_settings_size = ARRAY_SIZE(exposure_normal_mode_settings_array),

	.exposure_spot_settings = exposure_spot_mode_settings_array,
	.exposure_spot_settings_size = ARRAY_SIZE(exposure_spot_mode_settings_array),

	.exposure_avg_settings = exposure_avg_mode_settings_array,
	.exposure_avg_settings_size = ARRAY_SIZE(exposure_avg_mode_settings_array),

	.scene_auto_settings = scene_auto_mode_settings_array,
	.scene_auto_settings_size = ARRAY_SIZE(scene_auto_mode_settings_array),

	.scene_landscape_settings = scene_landscape_mode_settings_array,
	.scene_landscape_settings_size = ARRAY_SIZE(scene_landscape_mode_settings_array),	

	.scene_sunset_settings = scene_sunset_mode_settings_array,
	.scene_sunset_settings_size = ARRAY_SIZE(scene_sunset_mode_settings_array),

	.scene_night_settings = scene_night_mode_settings_array,
	.scene_night_settings_size = ARRAY_SIZE(scene_night_mode_settings_array),

	.scene_portrait_settings = scene_portrait_mode_settings_array,
	.scene_portrait_settings_size = ARRAY_SIZE(scene_portrait_mode_settings_array),

	.scene_sports_settings = scene_sports_mode_settings_array,
	.scene_sports_settings_size = ARRAY_SIZE(scene_sports_mode_settings_array),
};

#endif /* #define MT9P111_REG_H */

