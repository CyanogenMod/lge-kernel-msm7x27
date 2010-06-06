/* arch/arm/mach-msm/include/mach/lge_diag_test.h
 * Copyright (C) 2010 LGE Corporation.
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

#ifndef __ARCH_MSM_LGE_DIAG_TEST_H
#define __ARCH_MSM_LGE_DIAG_TEST_H

extern uint8_t if_condition_is_on_key_buffering;
extern uint8_t lgf_factor_key_test_rsp(char);
extern unsigned long int ats_mtc_log_mask;

extern int eta_execute(char *);
extern int base64_encode(char *, int, char *);

#endif
