/* linux/arch/arm/mach-msm/rpc_hsusb.c
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

#include <linux/err.h>
#include <mach/rpc_hsusb.h>
#include <asm/mach-types.h>

static struct msm_rpc_endpoint *usb_ep;
static struct msm_rpc_endpoint *chg_ep;

#define MSM_RPC_CHG_PROG 0x3000001a

struct msm_chg_rpc_ids {
	unsigned long	vers_comp;
	unsigned	chg_usb_charger_connected_proc;
	unsigned	chg_usb_charger_disconnected_proc;
	unsigned	chg_usb_i_is_available_proc;
	unsigned	chg_usb_i_is_not_available_proc;
};

struct msm_hsusb_rpc_ids {
	unsigned long	prog;
	unsigned long	vers_comp;
	unsigned long	init_phy;
	unsigned long	vbus_pwr_up;
	unsigned long	vbus_pwr_down;
	unsigned long	update_product_id;
	unsigned long	update_serial_num;
	unsigned long	update_is_serial_num_null;
	unsigned long	reset_rework_installed;
	unsigned long	enable_pmic_ulpi_data0;
	unsigned long	disable_pmic_ulpi_data0;
};

static struct msm_hsusb_rpc_ids usb_rpc_ids;
static struct msm_chg_rpc_ids chg_rpc_ids;

static int msm_hsusb_init_rpc_ids(unsigned long vers)
{
	if (vers == 0x00010001) {
		usb_rpc_ids.prog			= 0x30000064;
		usb_rpc_ids.vers_comp			= 0x00010001;
		usb_rpc_ids.init_phy			= 2;
		usb_rpc_ids.vbus_pwr_up			= 6;
		usb_rpc_ids.vbus_pwr_down		= 7;
		usb_rpc_ids.update_product_id		= 8;
		usb_rpc_ids.update_serial_num		= 9;
		usb_rpc_ids.update_is_serial_num_null	= 10;
		usb_rpc_ids.reset_rework_installed	= 17;
		usb_rpc_ids.enable_pmic_ulpi_data0	= 18;
		usb_rpc_ids.disable_pmic_ulpi_data0	= 19;
		return 0;
	} else if (vers == 0x00010002) {
		usb_rpc_ids.prog			= 0x30000064;
		usb_rpc_ids.vers_comp			= 0x00010002;
		usb_rpc_ids.init_phy			= 2;
		usb_rpc_ids.vbus_pwr_up			= 6;
		usb_rpc_ids.vbus_pwr_down		= 7;
		usb_rpc_ids.update_product_id		= 8;
		usb_rpc_ids.update_serial_num		= 9;
		usb_rpc_ids.update_is_serial_num_null	= 10;
		usb_rpc_ids.reset_rework_installed	= 17;
		usb_rpc_ids.enable_pmic_ulpi_data0	= 18;
		usb_rpc_ids.disable_pmic_ulpi_data0	= 19;
		return 0;
	} else {
		pr_err("%s: no matches found for version\n",
			__func__);
		return -ENODATA;
	}
}

static int msm_chg_init_rpc(unsigned long vers)
{
	if (((vers & RPC_VERSION_MAJOR_MASK) == 0x00010000) ||
	    ((vers & RPC_VERSION_MAJOR_MASK) == 0x00020000) ||
	    ((vers & RPC_VERSION_MAJOR_MASK) == 0x00030000) ||
	    ((vers & RPC_VERSION_MAJOR_MASK) == 0x00040000)) {
		chg_ep = msm_rpc_connect_compatible(MSM_RPC_CHG_PROG, vers,
						     MSM_RPC_UNINTERRUPTIBLE);
		if (IS_ERR(chg_ep))
			return -ENODATA;
		chg_rpc_ids.vers_comp				= vers;
		chg_rpc_ids.chg_usb_charger_connected_proc 	= 7;
		chg_rpc_ids.chg_usb_charger_disconnected_proc 	= 8;
		chg_rpc_ids.chg_usb_i_is_available_proc 	= 9;
		chg_rpc_ids.chg_usb_i_is_not_available_proc 	= 10;
		return 0;
	} else
		return -ENODATA;
}

/* rpc connect for hsusb */
int msm_hsusb_rpc_connect(void)
{

	if (usb_ep && !IS_ERR(usb_ep)) {
		pr_debug("%s: usb_ep already connected\n", __func__);
		return 0;
	}

	/* Initialize rpc ids */
	if (msm_hsusb_init_rpc_ids(0x00010001)) {
		pr_err("%s: rpc ids initialization failed\n"
			, __func__);
		return -ENODATA;
	}

	usb_ep = msm_rpc_connect_compatible(usb_rpc_ids.prog,
					usb_rpc_ids.vers_comp,
					MSM_RPC_UNINTERRUPTIBLE);

	if (IS_ERR(usb_ep)) {
		pr_err("%s: connect compatible failed vers = %lx\n",
			 __func__, usb_rpc_ids.vers_comp);

		/* Initialize rpc ids */
		if (msm_hsusb_init_rpc_ids(0x00010002)) {
			pr_err("%s: rpc ids initialization failed\n",
				__func__);
			return -ENODATA;
		}
		usb_ep = msm_rpc_connect_compatible(usb_rpc_ids.prog,
					usb_rpc_ids.vers_comp,
					MSM_RPC_UNINTERRUPTIBLE);
	}

	if (IS_ERR(usb_ep)) {
		pr_err("%s: connect compatible failed vers = %lx\n",
				__func__, usb_rpc_ids.vers_comp);
		return -EAGAIN;
	} else
		pr_info("%s: rpc connect success vers = %lx\n",
				__func__, usb_rpc_ids.vers_comp);

	return 0;
}
EXPORT_SYMBOL(msm_hsusb_rpc_connect);

/* rpc connect for charging */
int msm_chg_rpc_connect(void)
{
	uint32_t chg_vers;

#if !defined(CONFIG_MACH_LGE)
	if (machine_is_msm7201a_surf() || machine_is_msm7x27_surf() ||
	    machine_is_qsd8x50_surf() || machine_is_msm7x25_surf() ||
	    machine_is_qsd8x50a_surf())
		return -ENOTSUPP;
#endif

	if (chg_ep && !IS_ERR(chg_ep)) {
		pr_debug("%s: chg_ep already connected\n", __func__);
		return 0;
	}

	chg_vers = 0x00040001;
	if (!msm_chg_init_rpc(chg_vers))
		goto chg_found;

	chg_vers = 0x00030001;
	if (!msm_chg_init_rpc(chg_vers))
		goto chg_found;

	chg_vers = 0x00020001;
	if (!msm_chg_init_rpc(chg_vers))
		goto chg_found;

	chg_vers = 0x00010001;
	if (!msm_chg_init_rpc(chg_vers))
		goto chg_found;

	pr_err("%s: connect compatible failed \n",
			__func__);
	return -EAGAIN;

chg_found:
	pr_debug("%s: connected to rpc vers = %x\n",
			__func__, chg_vers);
	return 0;
}
EXPORT_SYMBOL(msm_chg_rpc_connect);

/* rpc call for phy_reset */
int msm_hsusb_phy_reset(void)
{
	int rc = 0;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: phy_reset rpc failed before call,"
			"rc = %ld\n", __func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_call(usb_ep, usb_rpc_ids.init_phy,
				&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: phy_reset rpc failed! rc = %d\n",
			__func__, rc);
	} else
		pr_info("msm_hsusb_phy_reset\n");

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_phy_reset);

/* rpc call for vbus powerup */
int msm_hsusb_vbus_powerup(void)
{
	int rc = 0;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: vbus_powerup rpc failed before call,"
			"rc = %ld\n", __func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_call(usb_ep, usb_rpc_ids.vbus_pwr_up,
		&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: vbus_powerup failed! rc = %d\n",
			__func__, rc);
	} else
		pr_debug("msm_hsusb_vbus_powerup\n");

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_vbus_powerup);

/* rpc call for vbus shutdown */
int msm_hsusb_vbus_shutdown(void)
{
	int rc = 0;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: vbus_shutdown rpc failed before call,"
			"rc = %ld\n", __func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_call(usb_ep, usb_rpc_ids.vbus_pwr_down,
		&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: vbus_shutdown failed! rc = %d\n",
			__func__, rc);
	} else
		pr_debug("msm_hsusb_vbus_shutdown\n");

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_vbus_shutdown);

int msm_hsusb_send_productID(uint32_t product_id)
{
	int rc = 0;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
		uint32_t product_id;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: rpc connect failed: rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	req.product_id = cpu_to_be32(product_id);
	rc = msm_rpc_call(usb_ep, usb_rpc_ids.update_product_id,
				&req, sizeof(req),
				5 * HZ);
	if (rc < 0)
		pr_err("%s: rpc call failed! error: %d\n",
			__func__, rc);
	else
		pr_debug("%s: rpc call success\n" , __func__);

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_send_productID);

int msm_hsusb_send_serial_number(const char *serial_number)
{
	int rc = 0, serial_len;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
		uint32_t length;
#if defined(CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_MEID)
		/* MEID is constitued of 14 characters */
		char serial_num[15];
#else	
		char serial_num[20];
#endif
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: rpc connect failed: rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

#if defined(CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_MEID)
	memset(req.serial_num, 0, 15);
	serial_len  = strlen(serial_number);
	strncpy(req.serial_num, serial_number, serial_len);
	serial_len++;
#else
	serial_len  = strlen(serial_number)+1;
	strncpy(req.serial_num, serial_number, 20);
#endif
	req.length = cpu_to_be32(serial_len);
	rc = msm_rpc_call(usb_ep, usb_rpc_ids.update_serial_num,
				&req, sizeof(req),
				5 * HZ);
	if (rc < 0)
		pr_err("%s: rpc call failed! error: %d\n",
			__func__, rc);
	else
		pr_debug("%s: rpc call success\n", __func__);

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_send_serial_number);

int msm_hsusb_is_serial_num_null(uint32_t val)
{
	int rc = 0;
	struct hsusb_phy_start_req {
			struct rpc_request_hdr hdr;
			uint32_t value;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: rpc connect failed: rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}
	if (!usb_rpc_ids.update_is_serial_num_null) {
		pr_err("%s: proc id not supported \n", __func__);
		return -ENODATA;
	}

	req.value = cpu_to_be32(val);
	rc = msm_rpc_call(usb_ep, usb_rpc_ids.update_is_serial_num_null,
				&req, sizeof(req),
				5 * HZ);
	if (rc < 0)
		pr_err("%s: rpc call failed! error: %d\n" ,
			__func__, rc);
	else
		pr_debug("%s: rpc call success\n", __func__);

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_is_serial_num_null);

#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)
/* ADD THUNDER feature TO USE VS740 BATT DRIVER IN THUNDERC
 * 2010-05-13, taehung.kim@lge.com
 */
/* woonghee@lge.com	2009-09-25, battery charging */
static int charger_type;
#endif

int msm_chg_usb_charger_connected(uint32_t device)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
		uint32_t otg_dev;
	} req;

#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)
	/* ADD THUNDER feature TO USE VS740 BATT DRIVER IN THUNDERC
	 * 2010-05-13, taehung.kim@lge.com
	 */
	/* woonghee@lge.com	2009-09-25, battery charging */
	charger_type = device;
#endif

	if (!chg_ep || IS_ERR(chg_ep))
		return -EAGAIN;
	req.otg_dev = cpu_to_be32(device);
	rc = msm_rpc_call(chg_ep, chg_rpc_ids.chg_usb_charger_connected_proc,
			&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: charger_connected failed! rc = %d\n",
			__func__, rc);
	} else
		pr_debug("msm_chg_usb_charger_connected\n");

	return rc;
}
EXPORT_SYMBOL(msm_chg_usb_charger_connected);

int msm_chg_usb_i_is_available(uint32_t sample)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
		uint32_t i_ma;
	} req;

	if (!chg_ep || IS_ERR(chg_ep))
		return -EAGAIN;
	req.i_ma = cpu_to_be32(sample);
	rc = msm_rpc_call(chg_ep, chg_rpc_ids.chg_usb_i_is_available_proc,
			&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: charger_i_available failed! rc = %d\n",
			__func__, rc);
	} else
		pr_debug("msm_chg_usb_i_is_available(%u)\n", sample);

	return rc;
}
EXPORT_SYMBOL(msm_chg_usb_i_is_available);

int msm_chg_usb_i_is_not_available(void)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
	} req;

#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)
/* LGE_CHANGE
 * ADD THUNDER feature TO USE VS740 BATT DRIVER IN THUNDERC
 * 2010-05-13, taehung.kim@lge.com
 */
	/* LGE_CHANGES_S [woonghee@lge.com] 2009-09-25, battery charging */
	charger_type = 3;	/* CHG_UNDEFINDED */
#endif

	if (!chg_ep || IS_ERR(chg_ep))
		return -EAGAIN;
	rc = msm_rpc_call(chg_ep, chg_rpc_ids.chg_usb_i_is_not_available_proc,
			&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: charger_i_not_available failed! rc ="
			"%d \n", __func__, rc);
	} else
		pr_debug("msm_chg_usb_i_is_not_available\n");

	return rc;
}
EXPORT_SYMBOL(msm_chg_usb_i_is_not_available);

int msm_chg_usb_charger_disconnected(void)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
	} req;

	if (!chg_ep || IS_ERR(chg_ep))
		return -EAGAIN;
	rc = msm_rpc_call(chg_ep, chg_rpc_ids.chg_usb_charger_disconnected_proc,
			&req, sizeof(req), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: charger_disconnected failed! rc = %d\n",
			__func__, rc);
	} else
		pr_debug("msm_chg_usb_charger_disconnected\n");

	return rc;
}
EXPORT_SYMBOL(msm_chg_usb_charger_disconnected);

/* rpc call to close connection */
int msm_hsusb_rpc_close(void)
{
	int rc = 0;

	if (IS_ERR(usb_ep)) {
		pr_err("%s: rpc_close failed before call, rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_close(usb_ep);
	usb_ep = NULL;

	if (rc < 0) {
		pr_err("%s: close rpc failed! rc = %d\n",
			__func__, rc);
		return -EAGAIN;
	} else
		pr_debug("rpc close success\n");

	return rc;
}
EXPORT_SYMBOL(msm_hsusb_rpc_close);

/* rpc call to close charging connection */
int msm_chg_rpc_close(void)
{
	int rc = 0;

	if (IS_ERR(chg_ep)) {
		pr_err("%s: rpc_close failed before call, rc = %ld\n",
			__func__, PTR_ERR(chg_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_close(chg_ep);
	chg_ep = NULL;

	if (rc < 0) {
		pr_err("%s: close rpc failed! rc = %d\n",
			__func__, rc);
		return -EAGAIN;
	} else
		pr_debug("rpc close success\n");

	return rc;
}
EXPORT_SYMBOL(msm_chg_rpc_close);

int msm_hsusb_reset_rework_installed(void)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
	} req;
	struct hsusb_rpc_rep {
		struct rpc_reply_hdr hdr;
		uint32_t rework;
	} rep;

	memset(&rep, 0, sizeof(rep));

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: hsusb rpc connection not initialized, rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	rc = msm_rpc_call_reply(usb_ep, usb_rpc_ids.reset_rework_installed,
				&req, sizeof(req),
				&rep, sizeof(rep), 5 * HZ);

	if (rc < 0) {
		pr_err("%s: rpc call failed! error: (%d)"
				"proc id: (%lx)\n",
				__func__, rc,
				usb_rpc_ids.reset_rework_installed);
		return rc;
	}

	pr_info("%s: rework: (%d)\n", __func__, rep.rework);
	return be32_to_cpu(rep.rework);
}
EXPORT_SYMBOL(msm_hsusb_reset_rework_installed);

static int msm_hsusb_pmic_ulpidata0_config(int enable)
{
	int rc = 0;
	struct hsusb_start_req {
		struct rpc_request_hdr hdr;
	} req;

	if (!usb_ep || IS_ERR(usb_ep)) {
		pr_err("%s: hsusb rpc connection not initialized, rc = %ld\n",
			__func__, PTR_ERR(usb_ep));
		return -EAGAIN;
	}

	if (enable)
		rc = msm_rpc_call(usb_ep, usb_rpc_ids.enable_pmic_ulpi_data0,
					&req, sizeof(req), 5 * HZ);
	else
		rc = msm_rpc_call(usb_ep, usb_rpc_ids.disable_pmic_ulpi_data0,
					&req, sizeof(req), 5 * HZ);

	if (rc < 0)
		pr_err("%s: rpc call failed! error: %d\n",
				__func__, rc);
	return rc;
}

int msm_hsusb_enable_pmic_ulpidata0(void)
{
	return msm_hsusb_pmic_ulpidata0_config(1);
}
EXPORT_SYMBOL(msm_hsusb_enable_pmic_ulpidata0);

int msm_hsusb_disable_pmic_ulpidata0(void)
{
	return msm_hsusb_pmic_ulpidata0_config(0);
}
EXPORT_SYMBOL(msm_hsusb_disable_pmic_ulpidata0);


/* wrapper for sending pid and serial# info to bootloader */
int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	int ret;

	ret = msm_hsusb_send_productID(pid);
	if (ret)
		return ret;

	if (!snum) {
		ret = msm_hsusb_is_serial_num_null(1);
		if (ret)
			return ret;
	}

	ret = msm_hsusb_is_serial_num_null(0);
	if (ret)
		return ret;
	ret = msm_hsusb_send_serial_number(snum);
	if (ret)
		return ret;

	return 0;
}


#ifdef CONFIG_USB_GADGET_MSM_72K
/* charger api wrappers */
int hsusb_chg_init(int connect)
{
	if (connect)
		return msm_chg_rpc_connect();
	else
		return msm_chg_rpc_close();
}
EXPORT_SYMBOL(hsusb_chg_init);

void hsusb_chg_vbus_draw(unsigned mA)
{
	msm_chg_usb_i_is_available(mA);
}
EXPORT_SYMBOL(hsusb_chg_vbus_draw);

void hsusb_chg_connected(enum chg_type chgtype)
{
	char *chg_types[] = {"STD DOWNSTREAM PORT",
			"CARKIT",
			"DEDICATED CHARGER",
			"INVALID"};

	if (chgtype == USB_CHG_TYPE__INVALID) {
		msm_chg_usb_i_is_not_available();
		msm_chg_usb_charger_disconnected();
		return;
	}

	pr_info("\nCharger Type: %s\n", chg_types[chgtype]);

	msm_chg_usb_charger_connected(chgtype);
}
EXPORT_SYMBOL(hsusb_chg_connected);
#endif

#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)
/* LGE_CHANGE
 * ADD THUNDER feature TO USE VS740 BATT DRIVER IN THUNDERC
 * 2010-05-13, taehung.kim@lge.com
 */

/* LGE_CHANGES_S [woonghee@lge.com] 2009-09-25, battery charging */
int msm_hsusb_get_charger_type(void)
{
	return charger_type;
}
EXPORT_SYMBOL(msm_hsusb_get_charger_type);
#endif

#if defined(CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_IMEI)
/* Get IMEI from arm9 (GSM class) */

#define ONCRPC_NV_CMD_REMOTE_PROC   9
#define NV_IMEI_GET_PROG            0x3000000e
/* NOTE: version of RPC depends on AMSS's setting(below is alohag's). */
#define NV_IMEI_GET_VER             0x00060001
#define NV_UE_IMEI_SIZE             9
#define MAX_IMEI_SIZE               ((NV_UE_IMEI_SIZE - 1) * 2)

int msm_nv_imei_get(unsigned char *nv_imei_ptr)
{
	static struct msm_rpc_endpoint *nv_imei_get_ep;
	int rc = 0;
	uint32_t nv_result;
	uint32_t dummy1, dummy2;
	struct nv_ue_imei_type imea_data;
	int i;

	struct msm_nv_imem_get_req {
		struct rpc_request_hdr hdr;
		enum nv_func_enum_type cmd;
		enum nv_items_enum_type item;
		uint32_t more_data;
		enum nv_items_enum_type disc;
		struct nv_ue_imei_type imea_data;;
	} req;

	struct hsusb_rpc_rep {
		struct rpc_reply_hdr hdr;
		enum nv_stat_enum_type result_item;
		uint32_t rep_more_data;
		enum nv_items_enum_type rep_disc;
		struct nv_ue_imei_type imea_data;;
	} rep;

	nv_imei_get_ep = msm_rpc_connect_compatible(NV_IMEI_GET_PROG,
			NV_IMEI_GET_VER, MSM_RPC_UNINTERRUPTIBLE);

	if (IS_ERR(nv_imei_get_ep)) {
		printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
				__func__, PTR_ERR(nv_imei_get_ep));
		return -EINVAL;
	}

	/* init imea_data struct */
	memset(&imea_data, 0, sizeof(imea_data));

	req.cmd = cpu_to_be32(NV_READ_F);
	req.item = cpu_to_be32(NV_UE_IMEI_I);
	req.more_data = cpu_to_be32(1);
	req.disc = cpu_to_be32(NV_UE_IMEI_I);
	req.imea_data = imea_data;

	rc = msm_rpc_call_reply(nv_imei_get_ep,
			ONCRPC_NV_CMD_REMOTE_PROC,
			&req, sizeof(req),
			&rep, sizeof(rep),
			5 * HZ);

	if (rc < 0) {
		printk(KERN_ERR "%s: msm_rpc_call failed! rc = %d\n", __func__, rc);
		return -EINVAL;
	}

	nv_result = be32_to_cpu(rep.result_item);
	dummy1 = be32_to_cpu(rep.rep_more_data);
	dummy2 = be32_to_cpu(rep.rep_disc);

	for (i = 0; i < NV_UE_IMEI_SIZE; i++) {
		if ((rep.imea_data.ue_imei[i] & 0x0F) >= 0xA)
			*(nv_imei_ptr + i*2) = (rep.imea_data.ue_imei[i] & 0x0F) + 55;
		else
			*(nv_imei_ptr + i*2) = (rep.imea_data.ue_imei[i] & 0x0F) + 48;
		if (((rep.imea_data.ue_imei[i] & 0xF0) >> 4) >= 0xA)
			*(nv_imei_ptr + i*2 + 1) = ((rep.imea_data.ue_imei[i] & 0xF0) >> 4) + 55;
		else
			*(nv_imei_ptr + i*2 + 1) = ((rep.imea_data.ue_imei[i] & 0xF0) >> 4) + 48;
	}
	*(nv_imei_ptr + MAX_IMEI_SIZE + 2) = '\0';
	msm_rpc_close(nv_imei_get_ep);

	pr_info("%s: msm_rpc_call success. ver = 0x%x\n",
			__func__, NV_IMEI_GET_VER);
	return rc;
}
EXPORT_SYMBOL(msm_nv_imei_get);

/* LGE_CHANGE_S [hyunhui.park@lge.com] 2009-04-21, Detect charger type using RPC  */
#if defined(CONFIG_USB_SUPPORT_LGDRIVER_GSM) || \
	defined(CONFIG_USB_SUPPORT_LGE_GADGET_GSM)

#define ONRPC_CHG_GET_GENERAL_STATUS_PROC	12

enum charger_hw_type {
	USB_CHARGER_TYPE_NONE,
	USB_CHARGER_TYPE_WALL,
	USB_CHARGER_TYPE_USB_PC,
	USB_CHARGER_TYPE_USB_WALL,
	USB_CHARGER_TYPE_USB_CARKIT,
	USB_CHARGER_TYPE_INVALID
};

struct hsusb_rep_chg_type {
	struct rpc_reply_hdr hdr;
	u32 more_data;

	u32 charger_status;
	u32 charger_type;
	u32 battery_status;
	u32 battery_level;
	u32 battery_voltage;
	u32 battery_temp;
	u32 charge_counter;
};

static struct hsusb_rep_chg_type rep;

int msm_hsusb_detect_chg_type(void)
{
	int rc, ret = 0;
	struct hsusb_req_chg_type {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req;

	if (!chg_ep || IS_ERR(chg_ep)) {
		pr_err("%s: hsusb rpc connection not initialized, rc = %ld\n",
				__func__, PTR_ERR(chg_ep));
		return -EAGAIN;
	}

	req.more_data = __constant_cpu_to_be32(1);

	memset(&rep, 0, sizeof(struct hsusb_rep_chg_type));

	rc = msm_rpc_call_reply(chg_ep, ONRPC_CHG_GET_GENERAL_STATUS_PROC,
			&req, sizeof(struct hsusb_req_chg_type),
			&rep, sizeof(struct hsusb_rep_chg_type),
			msecs_to_jiffies(5000));

	if (rc < 0) {
		printk(KERN_ERR "%s: rpc call failed !  rc = %d\n",
				__func__, rc);
		return rc;
	}

	rep.charger_type = be32_to_cpu(rep.charger_type);

	/* ret value is matched to charger type in msm_hsusb.c */
	switch (rep.charger_type) {
	case USB_CHARGER_TYPE_WALL:
	case USB_CHARGER_TYPE_USB_WALL:
	case USB_CHARGER_TYPE_USB_CARKIT:
		ret = 2; /* WALL CHARGER */
		break;
	case USB_CHARGER_TYPE_USB_PC:
		ret = 0; /* HOST PC */
		break;
	case USB_CHARGER_TYPE_NONE:
	case USB_CHARGER_TYPE_INVALID:
	default:
		ret = 3; /* INVALID */
		break;
	}

	return ret;
}
EXPORT_SYMBOL(msm_hsusb_detect_chg_type);

#endif
/* LGE_CHANGE_E [hyunhui.park@lge.com] 2009-04-21 */

#endif
