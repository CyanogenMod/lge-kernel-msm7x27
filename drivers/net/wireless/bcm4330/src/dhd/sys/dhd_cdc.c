/*
 * DHD Protocol Module for CDC and BDC.
 *
 * Copyright (C) 1999-2011, Broadcom Corporation
 * 
 *         Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: dhd_cdc.c,v 1.51.6.28.4.1 2011/02/01 19:36:23 Exp $
 *
 * BDC is like CDC, except it includes a header for data packets to convey
 * packet priority over the bus, and flags (e.g. to indicate checksum status
 * for dongle offload.)
 */

#include <typedefs.h>
#include <osl.h>

#include <bcmutils.h>
#include <bcmcdc.h>
#include <bcmendian.h>

#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_proto.h>
#include <dhd_bus.h>
#include <dhd_dbg.h>



/* Packet alignment for most efficient SDIO (can change based on platform) */
#ifndef DHD_SDALIGN
#define DHD_SDALIGN	32
#endif
#if !ISPOWEROF2(DHD_SDALIGN)
#error DHD_SDALIGN is not a power of 2!
#endif

#define RETRIES 2		/* # of retries to retrieve matching ioctl response */
#define BUS_HEADER_LEN	(16+DHD_SDALIGN)	/* Must be at least SDPCM_RESERVE
				 * defined in dhd_sdio.c (amount of header tha might be added)
				 * plus any space that might be needed for alignment padding.
				 */
#define ROUND_UP_MARGIN	2048 	/* Biggest SDIO block size possible for
				 * round off at the end of buffer
				 */

typedef struct dhd_prot {
	uint16 reqid;
	uint8 pending;
	uint32 lastcmd;
	uint8 bus_header[BUS_HEADER_LEN];
	cdc_ioctl_t msg;
	unsigned char buf[WLC_IOCTL_MAXLEN + ROUND_UP_MARGIN];
} dhd_prot_t;

static int
dhdcdc_msg(dhd_pub_t *dhd)
{
	int err = 0;
	dhd_prot_t *prot = dhd->prot;
	int len = ltoh32(prot->msg.len) + sizeof(cdc_ioctl_t);

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	DHD_OS_WAKE_LOCK(dhd);

	/* NOTE : cdc->msg.len holds the desired length of the buffer to be
	 *        returned. Only up to CDC_MAX_MSG_SIZE of this buffer area
	 *	  is actually sent to the dongle
	 */
	if (len > CDC_MAX_MSG_SIZE)
		len = CDC_MAX_MSG_SIZE;

	/* Send request */
	err = dhd_bus_txctl(dhd->bus, (uchar*)&prot->msg, len);

	DHD_OS_WAKE_UNLOCK(dhd);
	return err;
}

static int
dhdcdc_cmplt(dhd_pub_t *dhd, uint32 id, uint32 len)
{
	int ret;
	int cdc_len = len+sizeof(cdc_ioctl_t);
	dhd_prot_t *prot = dhd->prot;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	do {
		ret = dhd_bus_rxctl(dhd->bus, (uchar*)&prot->msg, cdc_len);
		if (ret < 0)
			break;
	} while (CDC_IOC_ID(ltoh32(prot->msg.flags)) != id);

	return ret;
}

static int
dhdcdc_query_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd, void *buf, uint len, uint8 action)
{
	dhd_prot_t *prot = dhd->prot;
	cdc_ioctl_t *msg = &prot->msg;
	void *info;
	int ret = 0, retries = 0;
	uint32 id, flags = 0;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));
	DHD_CTL(("%s: cmd %d len %d\n", __FUNCTION__, cmd, len));


	/* Respond "bcmerror" and "bcmerrorstr" with local cache */
	if (cmd == WLC_GET_VAR && buf)
	{
		if (!strcmp((char *)buf, "bcmerrorstr"))
		{
			strncpy((char *)buf, bcmerrorstr(dhd->dongle_error), BCME_STRLEN);
			goto done;
		}
		else if (!strcmp((char *)buf, "bcmerror"))
		{
			*(int *)buf = dhd->dongle_error;
			goto done;
		}
	}

	memset(msg, 0, sizeof(cdc_ioctl_t));

	msg->cmd = htol32(cmd);
	msg->len = htol32(len);
	msg->flags = (++prot->reqid << CDCF_IOC_ID_SHIFT);
	CDC_SET_IF_IDX(msg, ifidx);
	/* add additional action bits */
	action &= WL_IOCTL_ACTION_MASK;
	msg->flags |= (action << CDCF_IOC_ACTION_SHIFT);
	msg->flags = htol32(msg->flags);

	if (buf)
		//memcpy((void *)(&msg[1]), buf, len);
		memcpy(prot->buf, buf, len);	// 20110324_WBT : ID 1044, 1045

	if ((ret = dhdcdc_msg(dhd)) < 0) {
		DHD_ERROR(("dhdcdc_query_ioctl: dhdcdc_msg failed w/status %d\n", ret));
		goto done;
	}

retry:
	/* wait for interrupt and get first fragment */
	if ((ret = dhdcdc_cmplt(dhd, prot->reqid, len)) < 0)
		goto done;

	flags = ltoh32(msg->flags);
	id = (flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;

	if ((id < prot->reqid) && (++retries < RETRIES))
		goto retry;
	if (id != prot->reqid) {
		DHD_ERROR(("%s: %s: unexpected request id %d (expected %d)\n",
		           dhd_ifname(dhd, ifidx), __FUNCTION__, id, prot->reqid));
		ret = -EINVAL;
		goto done;
	}

	/* Check info buffer */
	info = (void*)&msg[1];

	/* Copy info buffer */
	if (buf)
	{
		if (ret < (int)len)
			len = ret;
		memcpy(buf, info, len);
	}

	/* Check the ERROR flag */
	if (flags & CDCF_IOC_ERROR)
	{
		ret = ltoh32(msg->status);
		/* Cache error from dongle */
		dhd->dongle_error = ret;
	}

done:
	return ret;
}

static int
dhdcdc_set_ioctl(dhd_pub_t *dhd, int ifidx, uint cmd, void *buf, uint len, uint8 action)
{
	dhd_prot_t *prot = dhd->prot;
	cdc_ioctl_t *msg = &prot->msg;
	int ret = 0;
	uint32 flags, id;

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));
	DHD_CTL(("%s: cmd %d len %d\n", __FUNCTION__, cmd, len));

	memset(msg, 0, sizeof(cdc_ioctl_t));

	msg->cmd = htol32(cmd);
	msg->len = htol32(len);
	msg->flags = (++prot->reqid << CDCF_IOC_ID_SHIFT);
	CDC_SET_IF_IDX(msg, ifidx);
	/* add additional action bits */
	action &= WL_IOCTL_ACTION_MASK;
	msg->flags |= (action << CDCF_IOC_ACTION_SHIFT) | CDCF_IOC_SET;
	msg->flags = htol32(msg->flags);

	if (buf)
		//memcpy((void *)(&msg[1]), buf, len);
		memcpy(prot->buf, buf, len);	// 20110324_WBT : ID 1121, 1122

	if ((ret = dhdcdc_msg(dhd)) < 0) {
		DHD_ERROR(("dhdcdc_set_ioctl: dhdcdc_msg failed w/status %d\n", ret));
		goto done;
	}

	if ((ret = dhdcdc_cmplt(dhd, prot->reqid, len)) < 0)
		goto done;

	flags = ltoh32(msg->flags);
	id = (flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;

	if (id != prot->reqid) {
		DHD_ERROR(("%s: %s: unexpected request id %d (expected %d)\n",
		           dhd_ifname(dhd, ifidx), __FUNCTION__, id, prot->reqid));
		ret = -EINVAL;
		goto done;
	}

	/* Check the ERROR flag */
	if (flags & CDCF_IOC_ERROR)
	{
		ret = ltoh32(msg->status);
		/* Cache error from dongle */
		dhd->dongle_error = ret;
	}

done:
	return ret;
}


int
dhd_prot_ioctl(dhd_pub_t *dhd, int ifidx, wl_ioctl_t * ioc, void * buf, int len)
{
	dhd_prot_t *prot = dhd->prot;
	int ret = -1;
	uint8 action;

	if (dhd->busstate == DHD_BUS_DOWN) {
		DHD_ERROR(("%s : bus is down. we have nothing to do\n", __FUNCTION__));
		goto done;
	}

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(len <= WLC_IOCTL_MAXLEN);

	if (len > WLC_IOCTL_MAXLEN)
		goto done;

	if (prot->pending == TRUE) {
		DHD_ERROR(("CDC packet is pending!!!! cmd=0x%x (%lu) lastcmd=0x%x (%lu)\n",
			ioc->cmd, (unsigned long)ioc->cmd, prot->lastcmd,
			(unsigned long)prot->lastcmd));
		if ((ioc->cmd == WLC_SET_VAR) || (ioc->cmd == WLC_GET_VAR)) {
			DHD_TRACE(("iovar cmd=%s\n", (char*)buf));
		}
		goto done;
	}

	prot->pending = TRUE;
	prot->lastcmd = ioc->cmd;
	action = ioc->set;
	if (action & WL_IOCTL_ACTION_SET)
		ret = dhdcdc_set_ioctl(dhd, ifidx, ioc->cmd, buf, len, action);
	else {
		ret = dhdcdc_query_ioctl(dhd, ifidx, ioc->cmd, buf, len, action);
		if (ret > 0)
			ioc->used = ret - sizeof(cdc_ioctl_t);
	}

	/* Too many programs assume ioctl() returns 0 on success */
	if (ret >= 0)
		ret = 0;
	else {
		cdc_ioctl_t *msg = &prot->msg;
		ioc->needed = ltoh32(msg->len); /* len == needed when set/query fails from dongle */
	}

	/* Intercept the wme_dp ioctl here */
	if ((!ret) && (ioc->cmd == WLC_SET_VAR) && (!strcmp(buf, "wme_dp"))) {
		int slen, val = 0;

		slen = strlen("wme_dp") + 1;
		if (len >= (int)(slen + sizeof(int)))
			bcopy(((char *)buf + slen), &val, sizeof(int));
		dhd->wme_dp = (uint8) ltoh32(val);
	}

	prot->pending = FALSE;

done:
	return ret;
}

int
dhd_prot_iovar_op(dhd_pub_t *dhdp, const char *name,
                  void *params, int plen, void *arg, int len, bool set)
{
	return BCME_UNSUPPORTED;
}


void
dhd_prot_dump(dhd_pub_t *dhdp, struct bcmstrbuf *strbuf)
{
	bcm_bprintf(strbuf, "Protocol CDC: reqid %d\n", dhdp->prot->reqid);
}

void
dhd_prot_hdrpush(dhd_pub_t *dhd, int ifidx, void *pktbuf)
{
#ifdef BDC
	struct bdc_header *h;
#endif /* BDC */

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#ifdef BDC
	/* Push BDC header used to convey priority for buses that don't */

	PKTPUSH(dhd->osh, pktbuf, BDC_HEADER_LEN);

	h = (struct bdc_header *)PKTDATA(dhd->osh, pktbuf);

	h->flags = (BDC_PROTO_VER << BDC_FLAG_VER_SHIFT);
	if (PKTSUMNEEDED(pktbuf))
		h->flags |= BDC_FLAG_SUM_NEEDED;


	h->priority = (PKTPRIO(pktbuf) & BDC_PRIORITY_MASK);
	h->flags2 = 0;
	h->dataOffset = 0;
#endif /* BDC */
	BDC_SET_IF_IDX(h, ifidx);
}

int
dhd_prot_hdrpull(dhd_pub_t *dhd, int *ifidx, void *pktbuf)
{
#ifdef BDC
	struct bdc_header *h;
#endif

	DHD_TRACE(("%s: Enter\n", __FUNCTION__));

#ifdef BDC
	/* Pop BDC header used to convey priority for buses that don't */

	if (PKTLEN(dhd->osh, pktbuf) < BDC_HEADER_LEN) {
		DHD_ERROR(("%s: rx data too short (%d < %d)\n", __FUNCTION__,
		           PKTLEN(dhd->osh, pktbuf), BDC_HEADER_LEN));
		return BCME_ERROR;
	}

	h = (struct bdc_header *)PKTDATA(dhd->osh, pktbuf);

	if ((*ifidx = BDC_GET_IF_IDX(h)) >= DHD_MAX_IFS) {
		DHD_ERROR(("%s: rx data ifnum out of range (%d)\n",
		           __FUNCTION__, *ifidx));
		return BCME_ERROR;
	}

	if (((h->flags & BDC_FLAG_VER_MASK) >> BDC_FLAG_VER_SHIFT) != BDC_PROTO_VER) {
		DHD_ERROR(("%s: non-BDC packet received, flags = 0x%x\n",
		           dhd_ifname(dhd, *ifidx), h->flags));
		if (((h->flags & BDC_FLAG_VER_MASK) >> BDC_FLAG_VER_SHIFT) == BDC_PROTO_VER_1)
			h->dataOffset = 0;
		else
			return BCME_ERROR;
	}

	if (h->flags & BDC_FLAG_SUM_GOOD) {
		DHD_INFO(("%s: BDC packet received with good rx-csum, flags 0x%x\n",
		          dhd_ifname(dhd, *ifidx), h->flags));
		PKTSETSUMGOOD(pktbuf, TRUE);
	}

	PKTSETPRIO(pktbuf, (h->priority & BDC_PRIORITY_MASK));
	PKTPULL(dhd->osh, pktbuf, BDC_HEADER_LEN);
#endif /* BDC */

	if (PKTLEN(dhd->osh, pktbuf) < (uint32) (h->dataOffset << 2)) {
		DHD_ERROR(("%s: rx data too short (%d < %d)\n", __FUNCTION__,
		           PKTLEN(dhd->osh, pktbuf), (h->dataOffset * 4)));
		return BCME_ERROR;
	}

	PKTPULL(dhd->osh, pktbuf, (h->dataOffset << 2));
	return 0;
}

int
dhd_prot_attach(dhd_pub_t *dhd)
{
	dhd_prot_t *cdc;

#ifndef DHD_USE_STATIC_BUF
	if (!(cdc = (dhd_prot_t *)MALLOC(dhd->osh, sizeof(dhd_prot_t)))) {
		DHD_ERROR(("%s: kmalloc failed\n", __FUNCTION__));
		goto fail;
	}
#else
	if (!(cdc = (dhd_prot_t *)dhd_os_prealloc(DHD_PREALLOC_PROT, sizeof(dhd_prot_t)))) {
		DHD_ERROR(("%s: kmalloc failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* DHD_USE_STATIC_BUF */
	memset(cdc, 0, sizeof(dhd_prot_t));

	/* ensure that the msg buf directly follows the cdc msg struct */
	if ((uintptr)(&cdc->msg + 1) != (uintptr)cdc->buf) {
		DHD_ERROR(("dhd_prot_t is not correctly defined\n"));
		goto fail;
	}

	dhd->prot = cdc;
#ifdef BDC
	dhd->hdrlen += BDC_HEADER_LEN;
#endif
	dhd->maxctl = WLC_IOCTL_MAXLEN + sizeof(cdc_ioctl_t) + ROUND_UP_MARGIN;
	return 0;

fail:
#ifndef DHD_USE_STATIC_BUF
	if (cdc != NULL)
		MFREE(dhd->osh, cdc, sizeof(dhd_prot_t));
#endif
	return BCME_NOMEM;
}

/* ~NOTE~ What if another thread is waiting on the semaphore?  Holding it? */
void
dhd_prot_detach(dhd_pub_t *dhd)
{
#ifndef DHD_USE_STATIC_BUF
	MFREE(dhd->osh, dhd->prot, sizeof(dhd_prot_t));
#endif
	dhd->prot = NULL;
}

void
dhd_prot_dstats(dhd_pub_t *dhd)
{
	/* No stats from dongle added yet, copy bus stats */
	dhd->dstats.tx_packets = dhd->tx_packets;
	dhd->dstats.tx_errors = dhd->tx_errors;
	dhd->dstats.rx_packets = dhd->rx_packets;
	dhd->dstats.rx_errors = dhd->rx_errors;
	dhd->dstats.rx_dropped = dhd->rx_dropped;
	dhd->dstats.multicast = dhd->rx_multicast;
	return;
}

int dhd_set_suspend(int value, dhd_pub_t *dhd)
{
	int power_mode = PM_MAX;
	wl_pkt_filter_enable_t	enable_parm;
	char iovbuf[32];
	int bcn_li_dtim = 3;

#define htod32(i) i

	if (dhd && dhd->up) {
		if (value) {
			dhd_wl_ioctl_cmd(dhd, WLC_SET_PM,
				(char *)&power_mode, sizeof(power_mode), TRUE, 0);
			/* Enable packet filter, only allow unicast packet to send up */
			enable_parm.id = htod32(100);
			enable_parm.enable = htod32(1);
			bcm_mkiovar("pkt_filter_enable", (char *)&enable_parm,
				sizeof(wl_pkt_filter_enable_t), iovbuf, sizeof(iovbuf));
			dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
			/* set bcn_li_dtim */
			bcm_mkiovar("bcn_li_dtim", (char *)&bcn_li_dtim,
				4, iovbuf, sizeof(iovbuf));
			dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
		} else {
			power_mode = PM_FAST;
			dhd_wl_ioctl_cmd(dhd, WLC_SET_PM, (char *)&power_mode,
				sizeof(power_mode), TRUE, 0);
			/* disable pkt filter */
			enable_parm.id = htod32(100);
			enable_parm.enable = htod32(0);
			bcm_mkiovar("pkt_filter_enable", (char *)&enable_parm,
				sizeof(wl_pkt_filter_enable_t), iovbuf, sizeof(iovbuf));
			dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
			/* set bcn_li_dtim */
			bcn_li_dtim = 0;
			bcm_mkiovar("bcn_li_dtim", (char *)&bcn_li_dtim,
				4, iovbuf, sizeof(iovbuf));
			dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
		}
	}

	return 0;
}

#define strtoul(nptr, endptr, base) bcm_strtoul((nptr), (endptr), (base))

/* Convert user's input in hex pattern to byte-size mask */
static int
wl_pattern_atoh(char *src, char *dst)
{
	int i;
	if (strncmp(src, "0x", 2) != 0 &&
	    strncmp(src, "0X", 2) != 0) {
		printf("Mask invalid format. Needs to start with 0x\n");
		return -1;
	}
	src = src + 2; /* Skip past 0x */
	if (strlen(src) % 2 != 0) {
		printf("Mask invalid format. Needs to be of even length\n");
		return -1;
	}
	for (i = 0; *src != '\0'; i++) {
		char num[3];
		strncpy(num, src, 2);
		num[2] = '\0';
		dst[i] = (uint8)strtoul(num, NULL, 16);
		src += 2;
	}
	return i;
}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-04-03, configs */
#if defined(CONFIG_LGE_BCM432X_PATCH)
#include <linux/fs.h>
#include <linux/ctype.h>

#if 0

CONFIG FILE FORMAT
==================

AVAILABLE PARAMETERS
~~~~~~~~~~~~~~~~~~~~
+====================+=========================================================+
| VARIABLE NAME      | DESCRIPTION                                             |
+====================+=========================================================+
| btc_mode           | BTCoexist                                               |
|                    | 0: disable, 1: enable                                   |
+--------------------+---------------------------------------------------------+
| country            | Country Code                                            |
|                    | KR, EU, US or AU ...                                    |
+--------------------+---------------------------------------------------------+
| vlan_mode          | Specifies the use of 802.1Q Tags (ON, OFF, AUTO).       |
|                    | 0: off, 1: on, -1: auto                                 |
+--------------------+---------------------------------------------------------+
| mpc                | Minimum Power Consumption                               |
|                    | 0: disable, 1: enable                                   |
+--------------------+---------------------------------------------------------+
| wme                | WME QoS                                                 |
|                    | 0: disable, 1: enable                                   |
+--------------------+---------------------------------------------------------+
| wme_apsd           | WME APSD (Advanced Power Save Delivery)                 |
|                    | 0: disable, 1: enable                                   |
+--------------------+---------------------------------------------------------+
| wme_qosinfo        | Set APSD parameters on STA.                             |
|                    | - max_sp_len = number of frames per USP: 0 (all), 2, 4, |
|                    |   or 6                                                  |
|                    | - be, bk, vi, and vo = 0 to disable, 1 to enable U-APSD |
|                    |   per AC                                                |
|                    |        <max_sp_len> <be> <bk> <vi> <vo>                 |
|                    | 0x0f =      0         1    1    1    1                  |
|                    | 0x2f =      2         1    1    1    1                  |
|                    | 0x4f =      4         1    1    1    1                  |
|                    | 0x6f =      6         1    1    1    1                  |
|                    | 0x03 =      0         0    0    1    1                  |
+--------------------+---------------------------------------------------------+
| wme_auto_trigger   | 0: disable, 1: enable                                   |
+--------------------+---------------------------------------------------------+
| wme_apsd_trigger   | in msec, 0: disable                                     |
+--------------------+---------------------------------------------------------+
| roam_off           | 0: roaming on, 1: roaming off                           |
+--------------------+---------------------------------------------------------+
| roam_scan_period   | in sec                                                  |
+--------------------+---------------------------------------------------------+
| roam_delta         | in dB                                                   |
+--------------------+---------------------------------------------------------+
| roam_trigger       | in dBm                                                  |
+--------------------+---------------------------------------------------------+
| PM                 | Power Saving Mode                                       |
|                    | 0: off, 1: max, 2: fast                                 |
+--------------------+---------------------------------------------------------+
| assoc_listen       | The Listen Interval sent in association requests        |
|                    | number of beacon                                        |
+--------------------+---------------------------------------------------------+

EXAMPLE
~~~~~~~
btc_mode=1
country=AU
vlan_mode=0
mpc=1
wme=1
wme_apsd=0
wme_qosinfo=0x00
wme_auto_trigger=1
wme_apsd_trigger=0
roam_off=0
roam_scan_period=10
roam_delta=20
roam_trigger=-70
PM=2
assoc_listen=1

#endif

#if defined (CONFIG_LGE_BCM432X_PATCH)
/* LGE Specific */
bool PM_control = TRUE;
#endif

static int dhd_preinit_proc(dhd_pub_t *dhd, int ifidx, char *name, char *value)
{
	int var_int;

	if (!strcmp(name, "country")) {
		return dhdcdc_set_ioctl(dhd, ifidx, WLC_SET_COUNTRY,
				value, WLC_CNTRY_BUF_SZ, TRUE);
	} else if (!strcmp(name, "roam_scan_period")) {
		var_int = (int)simple_strtol(value, NULL, 0);
		return dhdcdc_set_ioctl(dhd, ifidx, WLC_SET_ROAM_SCAN_PERIOD,
				&var_int, sizeof(var_int), TRUE);
	} else if (!strcmp(name, "roam_delta") || !strcmp(name, "roam_trigger")) {
		struct {
			int val;
			int band;
		} x;
		x.val = (int)simple_strtol(value, NULL, 0);
		x.band = WLC_BAND_AUTO;
		return dhdcdc_set_ioctl(dhd, ifidx, strcmp(name, "roam_delta") ?
				WLC_SET_ROAM_TRIGGER : WLC_SET_ROAM_DELTA, &x, sizeof(x), TRUE);
	} else if (!strcmp(name, "PM")) {
		var_int = (int)simple_strtol(value, NULL, 0);
		
#if defined (CONFIG_LGE_BCM432X_PATCH)
		if(var_int == 0){
			PM_control = FALSE;
			printk("%s var_int=%d don't control PM\n", __func__,var_int);
		}
		else
		{
			PM_control = TRUE;
			printk("%s var_int=%d do control PM\n", __func__,var_int);
		}
#endif		
		return dhdcdc_set_ioctl(dhd, ifidx, WLC_SET_PM,
				&var_int, sizeof(var_int), TRUE);
	} else if(!strcmp(name,"cur_etheraddr")) {
		struct ether_addr ea;
		char buf[32];
		uint iovlen;
		int ret;

		bcm_ether_atoe(value, &ea);

		ret = memcmp( &ea.octet, dhd->mac.octet, ETHER_ADDR_LEN);
		if(ret == 0){
			DHD_ERROR(("%s: Same Macaddr\n",__FUNCTION__));
			return 0;
		}

		DHD_ERROR(("%s: Change Macaddr = %02X:%02X:%02X:%02X:%02X:%02X\n",__FUNCTION__,
					ea.octet[0], ea.octet[1], ea.octet[2],
					ea.octet[3], ea.octet[4], ea.octet[5]));

		iovlen = bcm_mkiovar("cur_etheraddr", (char*)&ea, ETHER_ADDR_LEN, buf, 32);

		ret = dhdcdc_set_ioctl(dhd, ifidx, WLC_SET_VAR, buf, iovlen, TRUE);
		if (ret < 0) {
			DHD_ERROR(("%s: can't set MAC address , error=%d\n", __FUNCTION__, ret));
			return ret;
		}
		else{
			memcpy(dhd->mac.octet, (void *)&ea, ETHER_ADDR_LEN);
			return ret;
		}
	} else {
		uint iovlen;
		char iovbuf[WLC_IOCTL_SMLEN];

		/* wlu_iovar_setint */
		var_int = (int)simple_strtol(value, NULL, 0);

		/* Setup timeout bcn_timeout from dhd driver 4.217.48 */
		if(!strcmp(name, "roam_off")) {
			/* Setup timeout if Beacons are lost to report link down */
			if (var_int) {
				uint bcn_timeout = 2;
				bcm_mkiovar("bcn_timeout", (char *)&bcn_timeout, 4, iovbuf, sizeof(iovbuf));
				dhdcdc_set_ioctl(dhd, 0, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE);
			}
		}
		/* Setup timeout bcm_timeout from dhd driver 4.217.48 */

#ifdef BCMCCX								/* LGE_CHANGE_S, 2011-0226, add CCX */	//by sjpark 11-03-15
		if(var_int)
			printk(" roam_off =%d BCMCCX roam_off should be 0\n",var_int);
#endif											/* LGE_CHANGE_E, 2011-0226, add CCX */
		iovlen = bcm_mkiovar(name, (char *)&var_int, sizeof(var_int),
				iovbuf, sizeof(iovbuf));
		return dhdcdc_set_ioctl(dhd, ifidx, WLC_SET_VAR,
				iovbuf, iovlen, TRUE);
	}

	return 0;
}

static int dhd_preinit_config(dhd_pub_t *dhd, int ifidx)
{
	mm_segment_t old_fs;
	struct kstat stat;
	struct file *fp = NULL;
	unsigned int len;
	char *buf = NULL, *p, *name, *value;
	int ret = 0;

	if (!*config_path)
		return 0;

	old_fs = get_fs();
	set_fs(get_ds());
	if ((ret = vfs_stat(config_path, &stat))) {
		set_fs(old_fs);
		printk(KERN_ERR "%s: Failed to get information (%d)\n",
				config_path, ret);
		return ret;
	}
	set_fs(old_fs);

	if (!(buf = MALLOC(dhd->osh, stat.size + 1))) {
		printk(KERN_ERR "Failed to allocate memory %llu bytes\n", stat.size);
		return -ENOMEM;
	}

	if (!(fp = dhd_os_open_image(config_path)) ||
		(len = dhd_os_get_image_block(buf, stat.size, fp)) < 0)
		goto err;

	buf[stat.size] = '\0';
	for (p = buf; *p; p++) {
		if (isspace(*p))
			continue;
		for (name = p++; *p && !isspace(*p); p++) {
			if (*p == '=') {
				*p = '\0';
				p++;
				for (value = p; *p && !isspace(*p); p++);
				*p = '\0';
				if ((ret = dhd_preinit_proc(dhd, ifidx, name, value)) < 0)
					printk(KERN_ERR "%s: %s=%s\n",
							bcmerrorstr(ret), name, value);
				break;
			}
		}
	}
	ret = 0;

out:
	if (fp)
		dhd_os_close_image(fp);
	if (buf)
		MFREE(dhd->osh, buf, stat.size+1);
	return ret;

err:
	ret = -1;
	goto out;
}
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-04-03, configs */

int
dhd_preinit_ioctls(dhd_pub_t *dhd)
{
	int ret = 0;
	char eventmask[WL_EVENTING_MASK_LEN];
	char iovbuf[WL_EVENTING_MASK_LEN + 12];	/*  Room for "event_msgs" + '\0' + bitvec  */

	uint up = 0;
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-08-27, roam_off, PM */
#if !defined(CONFIG_LGE_BCM432X_PATCH)
#ifdef BCMCCX								/* LGE_CHANGE_S, 2011-0226, add CCX */	//by sjpark 11-03-15
	uint roamvar = 0;
#else									/* LGE_CHANGE_E, 2011-0226, add CCX */
#ifdef CUSTOMER_HW2
	uint roamvar = 0;
#else
	uint roamvar = 1;
#endif							/* LGE_CHANGE, 2011-0226, just add BRCM patch */
#endif
	uint power_mode = PM_FAST;
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-08-27, roam_off, PM */
	uint32 dongle_align = DHD_SDALIGN;
	uint32 glom = 0;
	int arpoe = 1;
#ifdef WLBTAMP
	int arp_ol = 0xc;
#else
	int arp_ol = 0xf;
#endif /* WLBTAMP */
#ifndef BCMCCX								/* LGE_CHANGE_S, 2011-0226, add CCX */	//by sjpark 11-03-15
	int scan_assoc_time = 40;
	int scan_unassoc_time = 80;
#endif
	const char 				*str;
	wl_pkt_filter_t		pkt_filter;
	wl_pkt_filter_t		*pkt_filterp;
	int						buf_len;
	int						str_len;
	uint32					mask_size;
	uint32					pattern_size;
	char buf[256];
#if defined(CONFIG_LGE_BCM432X_PATCH)
	uint filter_mode = 0;
#else
	uint filter_mode = 1;
#endif /* CONFIG_LGE_BCM432X_PATCH */
#ifdef AP
	uint32 mpc = 0; /* Turn MPC off for AP/APSTA mode */
	uint32 apsta = 1; /* Enable APSTA mode */
#endif

	/* Get the device MAC address */
	strcpy(iovbuf, "cur_etheraddr");
	if ((ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, iovbuf, sizeof(iovbuf), FALSE, 0)) < 0) {
		DHD_ERROR(("%s: can't get MAC address , error=%d\n", __FUNCTION__, ret));
		return BCME_NOTUP;
	}
	memcpy(dhd->mac.octet, iovbuf, ETHER_ADDR_LEN);
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-04-03, configs */
#if defined(CONFIG_LGE_BCM432X_PATCH)
	dhd_preinit_config(dhd, 0);
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-04-03, configs */

	/* Set Country code */
	if (dhd->country_code[0] != 0) {
		if (dhd_wl_ioctl_cmd(dhd, WLC_SET_COUNTRY,
			dhd->country_code, sizeof(dhd->country_code), TRUE, 0) < 0) {
			DHD_ERROR(("%s: country code setting failed\n", __FUNCTION__));
		}
	}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-08-27, already PM setup is configured */
#if !defined(CONFIG_LGE_BCM432X_PATCH)
	/* Set PowerSave mode */
	dhd_wl_ioctl_cmd(dhd, WLC_SET_PM, (char *)&power_mode, sizeof(power_mode), TRUE, 0);
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-08-27, already PM setup is configured */

	/* Match Host and Dongle rx alignment */
	bcm_mkiovar("bus:txglomalign", (char *)&dongle_align, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	/* disable glom option per default */
	bcm_mkiovar("bus:txglom", (char *)&glom, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

#ifdef AP
	/* Turn off MPC in AP mode */
	bcm_mkiovar("mpc", (char *)&mpc, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	/* Enable APSTA mode */
	bcm_mkiovar("apsta", (char *)&apsta, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
#endif

	/* Force STA UP */
	ret = dhd_wl_ioctl_cmd(dhd, WLC_UP, (char *)&up, sizeof(up), TRUE, 0);
	if (ret < 0)
		goto done;

	/* Setup event_msgs */
	bcm_mkiovar("event_msgs", eventmask, WL_EVENTING_MASK_LEN, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_GET_VAR, iovbuf, sizeof(iovbuf), FALSE, 0);
	if (ret < 0)
		goto done;
	bcopy(iovbuf, eventmask, WL_EVENTING_MASK_LEN);

	/* Setup event_msgs */
	setbit(eventmask, WLC_E_SET_SSID);
	setbit(eventmask, WLC_E_PRUNE);
	setbit(eventmask, WLC_E_AUTH);
	setbit(eventmask, WLC_E_REASSOC);
	setbit(eventmask, WLC_E_REASSOC_IND);
	setbit(eventmask, WLC_E_DEAUTH_IND);
	setbit(eventmask, WLC_E_DISASSOC_IND);
	setbit(eventmask, WLC_E_DISASSOC);
	setbit(eventmask, WLC_E_JOIN);
	setbit(eventmask, WLC_E_ASSOC_IND);
	setbit(eventmask, WLC_E_PSK_SUP);
	setbit(eventmask, WLC_E_LINK);
	setbit(eventmask, WLC_E_NDIS_LINK);
	setbit(eventmask, WLC_E_MIC_ERROR);
	setbit(eventmask, WLC_E_PMKID_CACHE);
	setbit(eventmask, WLC_E_TXFAIL);
	setbit(eventmask, WLC_E_JOIN_START);
	setbit(eventmask, WLC_E_SCAN_COMPLETE);
#ifdef WLMEDIA_HTSF
	setbit(eventmask, WLC_E_HTSFSYNC);
#endif

#if defined(BCMCCX) && defined(BCMDBG_EVENT) /* junlim */								/* LGE_CHANGE_S, 2011-0226, add CCX */		//by sjpark 11-03-15
	setbit(eventmask, WLC_E_ADDTS_IND);
	setbit(eventmask, WLC_E_DELTS_IND);
#endif /* defined(BCMCCX) && (BCMDBG_EVENT) */ /* junlim */	

	bcm_mkiovar("event_msgs", eventmask, WL_EVENTING_MASK_LEN, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
#ifndef BCMCCX								/* LGE_CHANGE_S, 2011-0226, add CCX */	//by sjpark 11-03-15
	dhd_wl_ioctl_cmd(dhd, WLC_SET_SCAN_CHANNEL_TIME, (char *)&scan_assoc_time,
		sizeof(scan_assoc_time), TRUE, 0);
	dhd_wl_ioctl_cmd(dhd, WLC_SET_SCAN_UNASSOC_TIME, (char *)&scan_unassoc_time,
		sizeof(scan_unassoc_time), TRUE, 0);
#endif									/* LGE_CHANGE_E, 2011-0226, add CCX */

	/* Set ARP offload */
	bcm_mkiovar("arpoe", (char *)&arpoe, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);
	bcm_mkiovar("arp_ol", (char *)&arp_ol, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);

	/* add a default packet filter pattern */
	str = "pkt_filter_add";
	str_len = strlen(str);
	strncpy(buf, str, str_len);
	buf[ str_len ] = '\0';
	buf_len = str_len + 1;

	pkt_filterp = (wl_pkt_filter_t *) (buf + str_len + 1);

	/* Parse packet filter id. */
	pkt_filter.id = htod32(100);

	/* Parse filter polarity. */
	pkt_filter.negate_match = htod32(0);

	/* Parse filter type. */
	pkt_filter.type = htod32(0);

	/* Parse pattern filter offset. */
	pkt_filter.u.pattern.offset = htod32(0);

#if defined(CONFIG_LGE_BCM432X_PATCH)
	/* Parse pattern filter mask. */
#if 1	//kernel panic at gelato
	mask_size =	htod32(wl_pattern_atoh("0x7f",
		(char *) pkt_filterp->u.pattern.mask_and_pattern));
#endif

	mask_size =	htod32(wl_pattern_atoh("0xff",
		(char *) pkt_filterp->u.pattern.mask_and_pattern));

	if (mask_size < 0)	{// 20110324_WBT : ID 325
		DHD_ERROR(("%s : mask_size(%d) is fail\n", __FUNCTION__, mask_size));
		return -EINVAL;
	}

	/* Parse pattern filter pattern. */
	pattern_size = htod32(wl_pattern_atoh("0xff",
		(char *) &pkt_filterp->u.pattern.mask_and_pattern[mask_size]));
#else
	/* Parse pattern filter mask. */
	mask_size =	htod32(wl_pattern_atoh("0x01",
		(char *) pkt_filterp->u.pattern.mask_and_pattern));
	if (mask_size < 0)	{// 20110324_WBT :  : ID 325
		DHD_ERROR(("%s : mask_size(%d) is fail\n", __FUNCTION__, mask_size));
		return -EINVAL;
	}
	/* Parse pattern filter pattern. */
	pattern_size = htod32(wl_pattern_atoh("0x00",
		(char *) &pkt_filterp->u.pattern.mask_and_pattern[mask_size]));
#endif /* CONFIG_LGE_BCM432X_PATCH */

	if (mask_size != pattern_size) {
		DHD_ERROR(("Mask and pattern not the same size\n"));
		return -EINVAL;
	}

	pkt_filter.u.pattern.size_bytes = mask_size;
	buf_len += WL_PKT_FILTER_FIXED_LEN;
	buf_len += (WL_PKT_FILTER_PATTERN_FIXED_LEN + 2 * mask_size);

	/* Keep-alive attributes are set in local	variable (keep_alive_pkt), and
	** then memcpy'ed into buffer (keep_alive_pktp) since there is no
	** guarantee that the buffer is properly aligned.
	*/
	memcpy((char *)pkt_filterp, &pkt_filter,
		WL_PKT_FILTER_FIXED_LEN + WL_PKT_FILTER_PATTERN_FIXED_LEN);

	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, buf, buf_len, TRUE, 0);

	/* set mode to allow pattern */
	bcm_mkiovar("pkt_filter_mode", (char *)&filter_mode, 4, iovbuf, sizeof(iovbuf));
	dhd_wl_ioctl_cmd(dhd, WLC_SET_VAR, iovbuf, sizeof(iovbuf), TRUE, 0);


done:
	return ret;
}

int
dhd_prot_init(dhd_pub_t *dhd)
{
	int ret = 0;
	wlc_rev_info_t revinfo;
	DHD_TRACE(("%s: Enter\n", __FUNCTION__));


	/* Get the device rev info */
	memset(&revinfo, 0, sizeof(revinfo));
	ret = dhd_wl_ioctl_cmd(dhd, WLC_GET_REVINFO, &revinfo, sizeof(revinfo), FALSE, 0);
	if (ret < 0)
		goto done;


	ret = dhd_preinit_ioctls(dhd);

	/* Always assumes wl for now */
	dhd->iswl = TRUE;

	goto done;

done:
	return ret;
}

void
dhd_prot_stop(dhd_pub_t *dhd)
{
	/* Nothing to do for CDC */
}

/* LGE_CHANGE_S, [yoohoo@lge.com], 2009-11-19, Use deepsleep instead of dhd_dev_reset when driver start or stop */
#if defined(CONFIG_LGE_BCM432X_PATCH) && defined(CONFIG_BRCM_USE_DEEPSLEEP)
extern dhd_pub_t * get_dhd_pub_from_dev(struct net_device *dev);
int dhd_deep_sleep(struct net_device *dev, int flag)
{
	dhd_pub_t *dhd_pub = get_dhd_pub_from_dev(dev);
    char iovbuf[20] = {0};
    uint powervar   = 0;

    DHD_TRACE(("%s: Enter Flag -> %d \n", __FUNCTION__, flag));
	if(dhd_pub == NULL)
		return 0;

    switch(flag) {
	case 1: /* DEEPSLEEP ON*/
		   printk(KERN_INFO "===== [WiFi] DEEP SLEEP ON =====\n");
	
		   /* Disable MPC */	
		   powervar = 0;
		   bcm_mkiovar("mpc", (char *)&powervar, 4, iovbuf, sizeof(iovbuf));
		   dhdcdc_set_ioctl(dhd_pub, 0, WLC_SET_VAR, iovbuf, sizeof(iovbuf));

		   /* Enable Deep Sleep */
		   powervar = 1;
		   bcm_mkiovar("deepsleep", (char *)&powervar, 4, iovbuf, sizeof(iovbuf));
		   dhdcdc_set_ioctl(dhd_pub, 0, WLC_SET_VAR, iovbuf, sizeof(iovbuf));
		   break;

	case 0: /*DEEPSLEEP OFF*/
		   printk(KERN_INFO "===== [WiFi] DEEP SLEEP OFF =====\n");

		   /* Disable Deep Sleep */	
		   powervar = 0;
		   bcm_mkiovar("deepsleep", (char *)&powervar, 4, iovbuf, sizeof(iovbuf));
		   dhdcdc_set_ioctl(dhd_pub, 0, WLC_SET_VAR, iovbuf, sizeof(iovbuf));

		   /* Enable MPC */
		   powervar = 1;
		   bcm_mkiovar("mpc", (char *)&powervar, 4, iovbuf, sizeof(iovbuf));
		   dhdcdc_set_ioctl(dhd_pub, 0, WLC_SET_VAR, iovbuf, sizeof(iovbuf));
		   break;

	default: 
		   printk(KERN_ERR "[%s] Invalid Input Flag (%d)",__FUNCTION__, flag);

    }

    return 0;

}
#endif /* CONFIG_LGE_BCM432X_PATCH && CONFIG_BRCM_USE_DEEPSLEEP */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2009-11-19, Use deepsleep instead of dhd_dev_reset when driver start or stop */
