/* linux/include/mach/rpc_hsusb.h
 *
 * Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 *
 * All source code in this file is licensed under the following license except
 * where indicated.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org
 */

#ifndef __ASM_ARCH_MSM_RPC_HSUSB_H
#define __ASM_ARCH_MSM_RPC_HSUSB_H

#include <mach/msm_rpcrouter.h>
#include <mach/msm_otg.h>
#include <mach/msm_hsusb.h>

#if defined(CONFIG_MSM_ONCRPCROUTER) && !defined(CONFIG_ARCH_MSM8X60)
int msm_hsusb_rpc_connect(void);
int msm_hsusb_phy_reset(void);
int msm_hsusb_vbus_powerup(void);
int msm_hsusb_vbus_shutdown(void);
int msm_hsusb_reset_rework_installed(void);
int msm_hsusb_enable_pmic_ulpidata0(void);
int msm_hsusb_disable_pmic_ulpidata0(void);
int msm_hsusb_rpc_close(void);

int msm_chg_rpc_connect(void);
int msm_chg_usb_charger_connected(uint32_t type);
int msm_chg_usb_i_is_available(uint32_t sample);
int msm_chg_usb_i_is_not_available(void);
int msm_chg_usb_charger_disconnected(void);
int msm_chg_rpc_close(void);

#ifdef CONFIG_USB_GADGET_MSM_72K
int hsusb_chg_init(int connect);
void hsusb_chg_vbus_draw(unsigned mA);
void hsusb_chg_connected(enum chg_type chgtype);
#endif


int msm_fsusb_rpc_init(struct msm_otg_ops *ops);
int msm_fsusb_init_phy(void);
int msm_fsusb_reset_phy(void);
int msm_fsusb_suspend_phy(void);
int msm_fsusb_resume_phy(void);
int msm_fsusb_rpc_close(void);
int msm_fsusb_remote_dev_disconnected(void);
int msm_fsusb_set_remote_wakeup(void);
void msm_fsusb_rpc_deinit(void);

/* wrapper to send pid and serial# info to bootloader */
int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum);

int msm_hsusb_send_productID(uint32_t product_id);
int msm_hsusb_send_serial_number(const char *serial_number);
int msm_hsusb_is_serial_num_null(uint32_t val);

#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)
/* ADD THUNDER feature TO USE VS740 BATT DRIVER
 * 2010-05-13, taehung.kim@lge.com
 */
/* woonghee@lge.com 2009-09-25, battery charging */
int msm_hsusb_get_charger_type(void);
#endif

/* LGE_CHANGE_S [hyunhui.park@lge.com] 2009-04-21, Detect charger type using RPC  */
#if defined(CONFIG_USB_SUPPORT_LGDRIVER_GSM) || \
	defined(CONFIG_USB_SUPPORT_LGE_GADGET_GSM)
int msm_hsusb_detect_chg_type(void);
#endif
/* LGE_CHANGE_E [hyunhui.park@lge.com] 2009-04-21 */

#if defined(CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_IMEI)
/* Type to hold UE IMEI */
struct nv_ue_imei_type {
	/* International Mobile Equipment Identity */
	u8 ue_imei[9];
} __attribute__((packed));

int msm_nv_imei_get(unsigned char *nv_imei_ptr);

enum nv_func_enum_type {
	NV_READ_F,          /* Read item */
	NV_WRITE_F,         /* Write item */
	NV_PEEK_F,          /* Peek at a location */
	NV_POKE_F,          /* Poke into a location */
	NV_FREE_F,          /* Free an nv item's memory allocation */
	NV_CHKPNT_DIS_F,    /* Disable cache checkpointing for glitch recovery */
	NV_CHKPNT_ENA_F,    /* Enable cache checkpointing for glitch recovery */
	NV_OTASP_COMMIT_F,  /* Commit (write) OTASP parameters to nv */
	NV_REPLACE_F,       /* Replace (overwrite) a dynamic pool item */
	NV_INCREMENT_F,     /* Increment the rental timer item */
	NV_FUNC_ENUM_PAD = 0x7FFF     /* Pad to 16 bits on ARM */
#ifdef FEATURE_RPC
	, NV_FUNC_ENUM_MAX = 0x7fffffff /* Pad to 32 bits */
#endif
};

enum nv_items_enum_type {
	NV_ESN_I                          = 0,
	NV_UE_IMEI_I                      = 550,
#ifdef FEATURE_NV_RPC_SUPPORT
	NV_ITEMS_ENUM_MAX           = 0x7fffffff
#endif
};

enum nv_stat_enum_type {
	NV_DONE_S,          /* Request completed okay */
	NV_BUSY_S,          /* Request is queued */
	NV_BADCMD_S,        /* Unrecognizable command field */
	NV_FULL_S,          /* The NVM is full */
	NV_FAIL_S,          /* Command failed, reason other than NVM was full */
	NV_NOTACTIVE_S,     /* Variable was not active */
	NV_BADPARM_S,       /* Bad parameter in command block */
	NV_READONLY_S,      /* Parameter is write-protected and thus read only */
	NV_BADTG_S,         /* Item not valid for Target */
	NV_NOMEM_S,         /* free memory exhausted */
	NV_NOTALLOC_S,      /* address is not a valid allocation */
	NV_STAT_ENUM_PAD = 0x7FFF,     /* Pad to 16 bits on ARM */
#ifdef FEATURE_RPC
	NV_STAT_ENUM_MAX = 0x7FFFFFFF,     /* Pad to 16 bits on ARM */
#endif /* FEATURE_RPC */
};

#endif  /* CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_IMEI */

#else
static inline int msm_hsusb_rpc_connect(void) { return 0; }
static inline int msm_hsusb_phy_reset(void) { return 0; }
static inline int msm_hsusb_vbus_powerup(void) { return 0; }
static inline int msm_hsusb_vbus_shutdown(void) { return 0; }
static inline int msm_hsusb_reset_rework_installed(void) { return 0; }
static inline int msm_hsusb_enable_pmic_ulpidata0(void) { return 0; }
static inline int msm_hsusb_disable_pmic_ulpidata0(void) { return 0; }
static inline int msm_hsusb_rpc_close(void) { return 0; }

static inline int msm_chg_rpc_connect(void) { return 0; }
static inline int msm_chg_usb_charger_connected(uint32_t type) { return 0; }
static inline int msm_chg_usb_i_is_available(uint32_t sample) { return 0; }
static inline int msm_chg_usb_i_is_not_available(void) { return 0; }
static inline int msm_chg_usb_charger_disconnected(void) { return 0; }
static inline int msm_chg_rpc_close(void) { return 0; }

#ifdef CONFIG_USB_GADGET_MSM_72K
static inline int hsusb_chg_init(int connect) { return 0; }
static inline void hsusb_chg_vbus_draw(unsigned mA) { }
static inline void hsusb_chg_connected(enum chg_type chgtype) { }
#endif

static inline int msm_fsusb_rpc_init(struct msm_otg_ops *ops) { return 0; }
static inline int msm_fsusb_init_phy(void) { return 0; }
static inline int msm_fsusb_reset_phy(void) { return 0; }
static inline int msm_fsusb_suspend_phy(void) { return 0; }
static inline int msm_fsusb_resume_phy(void) { return 0; }
static inline int msm_fsusb_rpc_close(void) { return 0; }
static inline int msm_fsusb_remote_dev_disconnected(void) { return 0; }
static inline int msm_fsusb_set_remote_wakeup(void) { return 0; }
static inline void msm_fsusb_rpc_deinit(void) { }
static inline int
usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum) { return 0; }
#endif
#endif
