/*
 * BT-AMP support routines
 *
 * $Copyright Broadcom Corporation$
 *
 * $Id: dhd_bta.h,v 1.2 2009/02/26 22:35:56 jqliu Exp $
 */
#ifndef __dhd_bta_h__
#define __dhd_bta_h__

struct dhd_pub;

extern int dhd_bta_docmd(struct dhd_pub *pub, void *cmd_buf, uint cmd_len);

extern void dhd_bta_doevt(struct dhd_pub *pub, void *data_buf, uint data_len);

extern int dhd_bta_tx_hcidata(struct dhd_pub *pub, void *data_buf, uint data_len);
extern void dhd_bta_tx_hcidata_complete(struct dhd_pub *dhdp, void *txp, bool success);

#ifdef BCMDBG
struct amp_hci_event;
struct amp_hci_ACL_data;

extern void dhd_bta_hcidump_evt(struct dhd_pub *pub, struct amp_hci_event *event);
extern void dhd_bta_hcidump_ACL_data(struct dhd_pub *pub, struct amp_hci_ACL_data *ACL_data,
                                     bool tx);
#endif

#endif /* __dhd_bta_h__ */
