/*
 * BCMSDH Function Driver for the native SDIO/MMC driver in the Linux Kernel
 *
 * Copyright (C) 1999-2009, Broadcom Corporation
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
 * $Id: bcmsdh_sdmmc_linux.c,v 1.1.2.5.20.16 2010/05/14 04:40:41 Exp $
 */

#include <typedefs.h>
#include <bcmutils.h>
#include <sdio.h>	/* SDIO Specs */
#include <bcmsdbus.h>	/* bcmsdh to/from specific controller APIs */
#include <sdiovar.h>	/* to get msglevel bit values */

#include <linux/sched.h>	/* request_irq() */

#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif /* defined(CONFIG_HAS_EARLYSUSPEND) */

#if !defined(SDIO_VENDOR_ID_BROADCOM)
#define SDIO_VENDOR_ID_BROADCOM		0x02d0
#endif /* !defined(SDIO_DEVICE_ID_BROADCOM_4325) */

#define SDIO_DEVICE_ID_BROADCOM_DEFAULT	0x0000

#if !defined(SDIO_DEVICE_ID_BROADCOM_4325_SDGWB)
#define SDIO_DEVICE_ID_BROADCOM_4325_SDGWB	0x0492	/* BCM94325SDGWB */
#endif /* !defined(SDIO_DEVICE_ID_BROADCOM_4325_SDGWB) */
#if !defined(SDIO_DEVICE_ID_BROADCOM_4325)
#define SDIO_DEVICE_ID_BROADCOM_4325		0x0493	/* BCM94325SDAGWBM */
#endif /* !defined(SDIO_DEVICE_ID_BROADCOM_4325) */
#if !defined(SDIO_DEVICE_ID_BROADCOM_4329)
#define SDIO_DEVICE_ID_BROADCOM_4329		0x4329
#endif /* !defined(SDIO_DEVICE_ID_BROADCOM_4329) */

#include <bcmsdh_sdmmc.h>
#include <dhd_dbg.h>

#if defined(CONFIG_BRCM_GPIO_INTR) && defined(CONFIG_HAS_EARLYSUSPEND)
#include <mach/gpio.h>
#include <linux/irq.h>
#endif	/* #if defined(CONFIG_BRCM_GPIO_INTR) && defined(CONFIG_HAS_EARLYSUSPEND) */

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
#include <linux/mmc/host.h>
#include <linux/wakelock.h>

#define GPIO_WLAN_HOST_WAKE CONFIG_BCM4325_GPIO_WL_HOSTWAKEUP

struct dhd_wifisleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
};

static struct dhd_wifisleep_info *dhd_wifi_sleep;

struct wake_lock wlan_host_wakelock; 
struct wake_lock wlan_host_wakelock_resume;
int dhd_suspend_context = FALSE;

extern int del_wl_timers(void);

extern void register_mmc_card_pm(struct early_suspend *);
extern void unregister_mmc_card_pm(void);

#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
extern volatile bool dhd_mmc_suspend;
#endif

extern void sdioh_sdmmc_devintr_off(sdioh_info_t *sd);
extern void sdioh_sdmmc_devintr_on(sdioh_info_t *sd);

int sdio_function_init(void);
void sdio_function_cleanup(void);

#define DESCRIPTION "bcmsdh_sdmmc Driver"
#define AUTHOR "Broadcom Corporation"

/* module param defaults */
static int clockoverride = 0;

module_param(clockoverride, int, 0644);
MODULE_PARM_DESC(clockoverride, "SDIO card clock override");

PBCMSDH_SDMMC_INSTANCE gInstance;

/* Maximum number of bcmsdh_sdmmc devices supported by driver */
#define BCMSDH_SDMMC_MAX_DEVICES 1

extern int bcmsdh_probe(struct device *dev);
extern int bcmsdh_remove(struct device *dev);
struct device sdmmc_dev;

#if defined(CONFIG_HAS_EARLYSUSPEND)
extern int dhdsdio_bussleep(void *bus, bool sleep);
extern bool dhdsdio_dpc(void *bus);
extern int dhd_os_proto_block(void *pub);
extern int dhd_os_proto_unblock(void * pub);
extern void *dhd_es_get_dhd_pub(void);
extern void *dhd_es_get_dhd_bus_sdh(void);
static int dhd_register_early_suspend(void);
static void dhd_unregister_early_suspend(void);
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)

void dhd_early_suspend(struct early_suspend *h);
void dhd_late_resume(struct early_suspend *h);
static struct early_suspend early_suspend_data = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20,
	.suspend = dhd_early_suspend,
	.resume = dhd_late_resume
};
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
DECLARE_WAIT_QUEUE_HEAD(bussleep_wake);
typedef struct dhd_early_suspend {
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)	
	int wait_driver_load; /* waiting for driver loaded */
	bool skip;
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	bool state;
	bool drv_loaded;
	struct dhd_bus_t *bus;
} dhd_early_suspend_t;
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
dhd_early_suspend_t dhd_early_suspend_ctrl = { 0, 0, 0, 0, 0};
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
dhd_early_suspend_t dhd_early_suspend_ctrl = { 0, 0, 0};
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
#endif /* #if defined(CONFIG_HAS_EARLYSUSPEND) */

static int bcmsdh_sdmmc_probe(struct sdio_func *func,
                              const struct sdio_device_id *id)
{
	int ret = 0;
	static struct sdio_func sdio_func_0;
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));
	sd_trace(("sdio_bcmsdh: func->class=%x\n", func->class));
	sd_trace(("sdio_vendor: 0x%04x\n", func->vendor));
	sd_trace(("sdio_device: 0x%04x\n", func->device));
	sd_trace(("Function#: 0x%04x\n", func->num));

	if (func->num == 1) {
		sdio_func_0.num = 0;
		sdio_func_0.card = func->card;
		gInstance->func[0] = &sdio_func_0;
		if(func->device == 0x4) { /* 4318 */
			gInstance->func[2] = NULL;
			sd_trace(("NIC found, calling bcmsdh_probe...\n"));
			ret = bcmsdh_probe(&sdmmc_dev);
		}
	}

	gInstance->func[func->num] = func;

	if (func->num == 2) {
		sd_trace(("F2 found, calling bcmsdh_probe...\n"));
		ret = bcmsdh_probe(&sdmmc_dev);
	}

	return ret;
}

static void bcmsdh_sdmmc_remove(struct sdio_func *func)
{
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));
	sd_info(("sdio_bcmsdh: func->class=%x\n", func->class));
	sd_info(("sdio_vendor: 0x%04x\n", func->vendor));
	sd_info(("sdio_device: 0x%04x\n", func->device));
	sd_info(("Function#: 0x%04x\n", func->num));

	if (func->num == 2) {
		sd_trace(("F2 found, calling bcmsdh_probe...\n"));
		bcmsdh_remove(&sdmmc_dev);
	}
}


/* devices we support, null terminated */
static const struct sdio_device_id bcmsdh_sdmmc_ids[] = {
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_DEFAULT) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4325_SDGWB) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4325) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4329) },
	{ /* end: all zeroes */				},
};

MODULE_DEVICE_TABLE(sdio, bcmsdh_sdmmc_ids);

static struct sdio_driver bcmsdh_sdmmc_driver = {
	.probe		= bcmsdh_sdmmc_probe,
	.remove		= bcmsdh_sdmmc_remove,
	.name		= "bcmsdh_sdmmc",
	.id_table	= bcmsdh_sdmmc_ids,
	};

struct sdos_info {
	sdioh_info_t *sd;
	spinlock_t lock;
};


int
sdioh_sdmmc_osinit(sdioh_info_t *sd)
{
	struct sdos_info *sdos;

	sdos = (struct sdos_info*)MALLOC(sd->osh, sizeof(struct sdos_info));
	sd->sdos_info = (void*)sdos;
	if (sdos == NULL)
		return BCME_NOMEM;

	sdos->sd = sd;
	spin_lock_init(&sdos->lock);
	return BCME_OK;
}

void
sdioh_sdmmc_osfree(sdioh_info_t *sd)
{
	struct sdos_info *sdos;
	ASSERT(sd && sd->sdos_info);

	sdos = (struct sdos_info *)sd->sdos_info;
	MFREE(sd->osh, sdos, sizeof(struct sdos_info));
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
void
dhd_es_set_dhd_bus(void *bus)
{
	dhd_early_suspend_ctrl.bus = bus;
}

void *
dhd_es_get_dhd_bus(void)
{
	return dhd_early_suspend_ctrl.bus;
}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
static int
dhd_es_lock_dhd_bus(void)
{
/* BEGIN: 0005533 mingi.sung@lge.com 2010-03-27 */
/* MOD 0005533: [WLAN] Fixing WBT issues on Wi-Fi driver */
/* WBT Fix TD# 248350 */
	void *bus;
	bus = dhd_es_get_dhd_pub();
	if( bus )
		dhd_os_proto_block(bus);
/* END: 0005533 mingi.sung@lge.com 2010-03-27 */

	return 0;
}

static int
dhd_es_unlock_dhd_bus(void)
{
/* BEGIN: 0005533 mingi.sung@lge.com 2010-03-27 */
/* MOD 0005533: [WLAN] Fixing WBT issues on Wi-Fi driver */
/* WBT Fix TD# 248351 */
	void *bus;
	bus = dhd_es_get_dhd_pub();
	if( bus )
		dhd_os_proto_unblock(bus);
/* END: 0005533 mingi.sung@lge.com 2010-03-27 */

	return 0;
}
#endif /* defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */

bool
dhd_early_suspend_state(void)
{
	return dhd_early_suspend_ctrl.state;
}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
static void dhd_enable_sdio_irq(int enable)
{
/* BEGIN: 0004825 mingi.sung@lge.com 2010-03-09 */
/* MOD 0004825: [WLAN] Prevent insmod failed */
	struct mmc_card *card;
	struct mmc_host *host;

	if(gInstance->func[0] == NULL){
		printk("%s: gInstance->func[0] is NULL\n",__func__);	
		return;
	}
	
	card = gInstance->func[0]->card;
	host = card->host;
/* END: 0004825 mingi.sung@lge.com 2010-03-09 */

	if (enable == TRUE )
		host->ops->enable_sdio_irq(host, 1); 
	else if (enable == FALSE)
		host->ops->enable_sdio_irq(host, 0); 
}

static int dhd_suspend(void)
{
	int bus_state;
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	int max_tries = 3;
	int gpio = 0;	
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */ 	

//	printk(KERN_ERR "[dongp.kim] %s: enter\n", __FUNCTION__);
	DHD_TRACE(("%s: SUSPEND Enter\n", __FUNCTION__));
	if (NULL != dhd_early_suspend_ctrl.bus) {
		dhd_es_lock_dhd_bus();
#if !defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)		
		dhd_early_suspend_ctrl.state = TRUE;
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
		do {
			bus_state = dhdsdio_bussleep(dhd_early_suspend_ctrl.bus, TRUE);
			if (bus_state == BCME_BUSY)
			{
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
				/* 250ms timeout */
				wait_event_timeout(bussleep_wake, FALSE, HZ/4);
				DHD_TRACE(("%s in loop\n", __FUNCTION__));
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
				/* 50ms timeout */
				wait_event_timeout(bussleep_wake, FALSE, HZ/20);
				DHD_TRACE(("%s in loop\n", __FUNCTION__));
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
			}
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)			
		} while ((bus_state == BCME_BUSY) && (max_tries-- > 0));
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */		
		} while (bus_state == BCME_BUSY);
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)	
		if(max_tries <= 0)
		{
			printk(KERN_ERR "[WiFi] BUS BUSY!! Couldn't sleep.\n"); 	
			dhd_es_unlock_dhd_bus();
			/* This value should be returned to mmc_suspend*/
			return -1;
		}

		dhd_early_suspend_ctrl.state = TRUE;
		gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
		DHD_TRACE(("%s: SUSPEND Done gpio->%d\n", __FUNCTION__, gpio));
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
		DHD_TRACE(("%s: SUSPEND Done\n", __FUNCTION__));
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	} else {
		DHD_ERROR(("%s: no bus.. \n", __FUNCTION__));
	}
	return 0;
}
static int dhd_resume(void)
{
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	int gpio = 0;
/* BEGIN: 0003780 mingi.sung@lge.com 2010-02-02 */
/* DEL 0003780: [WLAN] Reduce the time to resume when using host-wakeup */
/* END: 0003780 mingi.sung@lge.com 2010-02-02 */

	dhd_enable_sdio_irq(FALSE);
	dhd_suspend_context = FALSE;
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */

//	printk(KERN_ERR "[dongp.kim] %s: enter\n", __FUNCTION__);
	DHD_TRACE(("%s: RESUME Enter\n", __FUNCTION__));
	if (NULL != dhd_early_suspend_ctrl.bus) {
		dhd_early_suspend_ctrl.state = FALSE;
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
/* LGE_CHANGE_S, [hyuksang], due to power consumption, the below line is discarded to reduce 2s delay */
/* Instead of wake_lock_timeout(), use wake_lock */
//		wake_lock_timeout(&wlan_host_wakelock_resume, 2*HZ);
		wake_lock(&wlan_host_wakelock_resume);
/* LGE_CHANGE_E, [hyuksang], due to power consumption, the below line is discarded to reduce 2s delay */
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
		dhdsdio_dpc(dhd_early_suspend_ctrl.bus);
		dhd_es_unlock_dhd_bus();
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)		
		gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
		DHD_TRACE(("%s: RESUME Done gpio->%d\n", __FUNCTION__, gpio));
/* LGE_CHANGE_S, [hyuksang], due to power consumption, the below line is discarded to reduce 2s delay */
/* Instead of wake_lock_timeout(), use wake_lock */		
		wake_unlock(&wlan_host_wakelock_resume);
/* LGE_CHANGE_E, [hyuksang], due to power consumption, the below line is discarded to reduce 2s delay */
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
		DHD_TRACE(("%s: RESUME Done\n", __FUNCTION__));
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	} else {
		DHD_ERROR(("%s: no bus.. \n", __FUNCTION__));
	}
	return 0;
}
#endif /* defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#endif /* #if defined(CONFIG_HAS_EARLYSUSPEND) */

/* Interrupt enable/disable */
SDIOH_API_RC
sdioh_interrupt_set(sdioh_info_t *sd, bool enable)
{
	ulong flags;
	struct sdos_info *sdos;

	sd_trace(("%s: %s\n", __FUNCTION__, enable ? "Enabling" : "Disabling"));

	sdos = (struct sdos_info *)sd->sdos_info;
	ASSERT(sdos);

#if !defined(OOB_INTR_ONLY)
	if (enable && !(sd->intr_handler && sd->intr_handler_arg)) {
		sd_err(("%s: no handler registered, will not enable\n", __FUNCTION__));
		return SDIOH_API_RC_FAIL;
	}
#endif /* !defined(OOB_INTR_ONLY) */

	/* Ensure atomicity for enable/disable calls */
	spin_lock_irqsave(&sdos->lock, flags);

	sd->client_intr_enabled = enable;
	if (enable) {
		sdioh_sdmmc_devintr_on(sd);
	} else {
		sdioh_sdmmc_devintr_off(sd);
	}

	spin_unlock_irqrestore(&sdos->lock, flags);

	return SDIOH_API_RC_SUCCESS;
}


#ifdef BCMSDH_MODULE
static int __init
bcmsdh_module_init(void)
{
	int error = 0;
	sdio_function_init();
	return error;
}

static void __exit
bcmsdh_module_cleanup(void)
{
	sdio_function_cleanup();
}

module_init(bcmsdh_module_init);
module_exit(bcmsdh_module_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);

#endif /* BCMSDH_MODULE */
/*
 * module init
*/
int sdio_function_init(void)
{
	int error = 0;
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));

	gInstance = kzalloc(sizeof(BCMSDH_SDMMC_INSTANCE), GFP_KERNEL);
	if (!gInstance)
		return -ENOMEM;

	bzero(&sdmmc_dev, sizeof(sdmmc_dev));
	error = sdio_register_driver(&bcmsdh_sdmmc_driver);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	if (!error) {
		dhd_register_early_suspend();
		DHD_TRACE(("%s: registered with Android PM\n", __FUNCTION__));
	}
#endif	/* defined(CONFIG_HAS_EARLYSUSPEND) */

	return error;
}

/*
 * module cleanup
*/
extern int bcmsdh_remove(struct device *dev);
void sdio_function_cleanup(void)
{
	sd_trace(("%s Enter\n", __FUNCTION__));

#if defined(CONFIG_HAS_EARLYSUSPEND)
/* BEGIN: 0004486 mingi.sung@lge.com 2010-02-28 */
/* ADD 0004486: [WLAN] Fix a bug - Reset when Wi-Fi turn on/off */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_enable_sdio_irq(FALSE);
#endif
/* END: 0004486 mingi.sung@lge.com 2010-02-28 */
	dhd_unregister_early_suspend();
/* BEGIN: 0004486 mingi.sung@lge.com 2010-02-28 */
/* ADD 0004486: [WLAN] Fix a bug - Reset when Wi-Fi turn on/off */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_suspend_context = TRUE; 
#endif
/* END: 0004486 mingi.sung@lge.com 2010-02-28 */
#endif	/* defined(CONFIG_HAS_EARLYSUSPEND) */

	sdio_unregister_driver(&bcmsdh_sdmmc_driver);

	if (gInstance)
		kfree(gInstance);
}
/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <ARP offload> */
#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
extern int dhd_config_arp_offload(void *bus, bool flag);
#endif	/* CONFIG_BRCM_LGE_WL_ARPOFFLOAD */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <ARP offload> */
/* LGE_CHANGE_s, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
extern int dhdsdio_set_dtim(void *bus, int enalbe);
#endif
/* LGE_CHANGE_E, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <Packet filter> */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_PKTFILTER)
extern int dhdsdio_enable_filters(void *bus);
extern int dhdsdio_disable_filters(void *bus);
#endif	/* defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_PKTFILTER) */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <Packet filter> */

#if defined(CONFIG_HAS_EARLYSUSPEND)
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
void dhd_early_suspend(struct early_suspend *h)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
	dhd_mmc_suspend = FALSE;
#endif

//	printk(KERN_ERR "[dongp.kim] %s: enter!!!\n", __FUNCTION__);
	DHD_TRACE(("%s: enter\n", __FUNCTION__));

	dhd_suspend_context = TRUE;
	
	/* If chip active is done, do put the device to suspend */
	del_wl_timers();
/* LGE_CHANGE_s, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	dhdsdio_set_dtim(dhd_early_suspend_ctrl.bus, TRUE);
#endif
/* LGE_CHANGE_E, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <ARP offload> */
#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	/*Enable ARP Offloading*/
	dhd_config_arp_offload(dhd_early_suspend_ctrl.bus , TRUE);
#endif	/* CONFIG_BRCM_LGE_WL_ARPOFFLOAD */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <ARP offload> */
	
/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <Packet filter> */
#if defined(CONFIG_BRCM_LGE_WL_PKTFILTER)
	dhdsdio_enable_filters(dhd_early_suspend_ctrl.bus);
#endif	/* CONFIG_BRCM_LGE_WL_PKTFILTER */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <Packet filter> */
	
	if(dhd_suspend() < 0) {
		dhd_enable_sdio_irq(TRUE); /* make sure one more for testing, later */
		DHD_ERROR(("%s: dhd_suspend() failed\n", __FUNCTION__));
		return;
	} 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)	
	dhd_mmc_suspend = TRUE;
#endif
#if defined(CONFIG_BRCM_GPIO_INTR)
	enable_irq(dhd_wifi_sleep->host_wake_irq);
#endif /* CONFIG_BRCM_GPIO_INTR */
//printk(KERN_ERR "[dongp.kim] %s: exit!!!\n", __FUNCTION__);
}

void dhd_late_resume(struct early_suspend *h)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
	dhd_mmc_suspend = FALSE;
#endif

//	printk(KERN_ERR "[dongp.kim] %s: enter\n", __FUNCTION__);
	DHD_TRACE(("%s: enter\n", __FUNCTION__));

	if(dhd_suspend_context == TRUE ){
#if defined(CONFIG_BRCM_GPIO_INTR)
	disable_irq(dhd_wifi_sleep->host_wake_irq);
#endif /* CONFIG_BRCM_GPIO_INTR */
	/*Do the resume operations*/
	dhd_resume();
	}else 
		printk("%s: Do not dhd_suspend mode setting.\n",__FUNCTION__);

/* LGE_CHANGE_s, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	dhdsdio_set_dtim(dhd_early_suspend_ctrl.bus, FALSE);
#endif
/* LGE_CHANGE_E, [jisung.yang@lge.com], 2010-08-24, <Set listen interval and dtim listen> */
/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <ARP offload> */
#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	/*DiSable ARP Offloading*/
	dhd_config_arp_offload(dhd_early_suspend_ctrl.bus, FALSE);
#endif	/* CONFIG_BRCM_LGE_WL_ARPOFFLOAD */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <ARP offload> */

/* LGE_CHANGE_S, [yoohoo@lge.com], 2010-1-13, <Packet filter> */
#if defined (CONFIG_BRCM_LGE_WL_PKTFILTER)
	dhdsdio_disable_filters(dhd_early_suspend_ctrl.bus);
#endif /* CONFIG_BRCM_LGE_WL_PKTFILTER */
/* LGE_CHANGE_E, [yoohoo@lge.com], 2010-1-13, <Packet filter> */
//	printk(KERN_ERR "[dongp.kim] %s: exit\n", __FUNCTION__);

	return;
}
EXPORT_SYMBOL(dhd_early_suspend);
EXPORT_SYMBOL(dhd_late_resume);
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Support Host Wakeup */

#if defined(CONFIG_BRCM_GPIO_INTR)

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */
#if !defined(CONFIG_LGE_BCM432X_PATCH)
#define GPIO_WLAN_HOST_WAKE 0

struct dhd_wifisleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
};

static struct dhd_wifisleep_info *dhd_wifi_sleep;
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */

/**
 * Supposed that Early Suspend/Resume is disable
 */
static int
dhd_enable_hwakeup(void)
{
	int ret;

	ret = set_irq_wake(dhd_wifi_sleep->host_wake_irq, 1);

	if (ret < 0) {
		DHD_ERROR(("Couldn't enable WLAN_HOST_WAKE as wakeup interrupt"));
		free_irq(dhd_wifi_sleep->host_wake_irq, NULL);
	}
	else 
		printk("[yoohoo] dhd_enable_hwakeup : succeed irq %d\n", dhd_wifi_sleep->host_wake_irq);

	return ret;
}

/**
 * Stops the Sleep-Mode Protocol on the Host.
 */
static void
dhd_disable_hwakeup(void)
{

	if (set_irq_wake(dhd_wifi_sleep->host_wake_irq, 0))
		DHD_ERROR(("Couldn't disable hostwake IRQ wakeup mode\n"));
}


/**
 * Schedules a tasklet to run when receiving an interrupt on the
 * <code>HOST_WAKE</code> GPIO pin.
 * @param irq Not used.
 * @param dev_id Not used.
 */
static irqreturn_t
dhd_hostwakeup_isr(int irq, void *dev_id)
{
	int gpio = 0;

	gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
	printk(KERN_ERR "[%s] HostWakeup Get GPIO %d: %d\n", 
		__func__, GPIO_WLAN_HOST_WAKE, gpio);
	gpio_set_value(GPIO_WLAN_HOST_WAKE, 0);
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */
#if !defined(CONFIG_LGE_BCM432X_PATCH)
	set_irq_type(dhd_wifi_sleep->host_wake_irq, gpio ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);
#endif /* CONFIG_LGE_BCM432X_PATCH */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */
	if (!gpio) {
		DHD_INFO(("[WiFi] complete on host-wakeup \n"));

		//dhd_late_resume(NULL); /* rypark test */
		return IRQ_HANDLED;
	}

	/* schedule a tasklet to handle the change in the host wake line */
	return IRQ_HANDLED;
}

/**
 * Initializes the module.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int
dhd_register_hwakeup(void)
{
	int ret;

	dhd_wifi_sleep = kzalloc(sizeof(struct dhd_wifisleep_info), GFP_KERNEL);
	if (!dhd_wifi_sleep)
		return -ENOMEM;

	dhd_wifi_sleep->host_wake = GPIO_WLAN_HOST_WAKE;

	printk(KERN_ERR "[yoohoo] dhd_register_hwakeup : start \n");

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Init wakelock */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	/* wake lock initialize */
   	wake_lock_init(&wlan_host_wakelock, WAKE_LOCK_SUSPEND, "WLAN_HOST_WAKE");
   	wake_lock_init(&wlan_host_wakelock_resume, WAKE_LOCK_SUSPEND, "WLAN_HOST_WAKE_RESUME");	
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Init wakelock */

	ret = gpio_request(dhd_wifi_sleep->host_wake, "wifi_hostwakeup");
	if (ret < 0) {
		DHD_ERROR(("[WiFi] Failed to get gpio_request \n"));
		gpio_free(dhd_wifi_sleep->host_wake);
		return 0;
	}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, Set gpio init value to zero */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	/* Set gpio init value to zero */
	ret = gpio_direction_output(dhd_wifi_sleep->host_wake, 0);
	if (ret < 0) {
		printk(KERN_ERR "[WiFi] Failed to get direction out\n");
	}
	//gpio_set_value(dhd_wifi_sleep->host_wake, 0);
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, Init wakelock */

	ret = gpio_direction_input(dhd_wifi_sleep->host_wake);
	if (ret < 0) {
		DHD_ERROR(("[WiFi] Failed to get direction  \n"));
		return 0;
	}

	dhd_wifi_sleep->host_wake_irq = gpio_to_irq(dhd_wifi_sleep->host_wake);

	if (dhd_wifi_sleep->host_wake_irq  < 0) {
		DHD_ERROR(("[WiFi] Failed to get irq  \n"));
		return 0;
	}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	ret = request_irq(dhd_wifi_sleep->host_wake_irq, dhd_hostwakeup_isr,
//		IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING , "wifi_hostwakeup", NULL);
		IRQF_DISABLED | IRQF_TRIGGER_RISING , "wifi_hostwakeup", NULL); //jisung.yang@lge.com
#else /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	ret = request_irq(dhd_wifi_sleep->host_wake_irq, dhd_hostwakeup_isr,
		IRQF_DISABLED | IRQF_TRIGGER_HIGH, "wifi_hostwakeup", NULL);
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-06, Support Host Wakeup */
	if (ret) {
		DHD_ERROR(("[WiFi] Failed to get HostWakeUp IRQ \n"));
		free_irq(dhd_wifi_sleep->host_wake_irq, 0);
		return ret;
		/* To do */
	}
	else {
		DHD_INFO(("[WiFi] install HostWakeup IRQ \n"));
		printk (KERN_ERR "[yoohoo] dhd_register_hwakeup : OK\n");
	}

/* LGE_CHANGE_S [yoohoo@lge.com] 2009-11-19, set_irq_type and disable_irq */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
//	set_irq_type(dhd_wifi_sleep->host_wake_irq, IRQ_TYPE_EDGE_BOTH);
	set_irq_type(dhd_wifi_sleep->host_wake_irq, IRQ_TYPE_EDGE_RISING);
#if	defined(CONFIG_BRCM_GPIO_INTR)
	disable_irq(dhd_wifi_sleep->host_wake_irq);
#endif /* CONFIG_BRCM_GPIO_INTR */
#endif /* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2009-11-19, set_irq_type and disable_irq */
	return ret;
}

static void
dhd_unregister_hwakeup(void)
{

	dhd_disable_hwakeup();
	free_irq(dhd_wifi_sleep->host_wake_irq, NULL);
	gpio_free(dhd_wifi_sleep->host_wake);
	kfree(dhd_wifi_sleep);
}
#endif /*  #ifdef CONFIG_BRCM_GPIO_INTR */

static int
dhd_register_early_suspend(void)
{
	/* LGE_CHANGE_S [yoohoo@lge.com] 2009-01-14, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_early_suspend_ctrl.drv_loaded = TRUE;

	dhd_early_suspend_ctrl.wait_driver_load = jiffies;
	register_mmc_card_pm(&early_suspend_data);

#if defined(CONFIG_BRCM_GPIO_INTR)
	/* HostWake up */
	dhd_register_hwakeup();
	dhd_enable_hwakeup();
	printk(KERN_ERR "[%s] HostWakeup Get GPIO %d: %d\n",
			__func__, GPIO_WLAN_HOST_WAKE, gpio_get_value(GPIO_WLAN_HOST_WAKE));
#endif	/* CONFIG_BRCM_GPIO_INTR */

	return 0;
#else	/* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	return 0;
#endif	/* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2010-01-14, Support Host Wakeup */
}

static void
dhd_unregister_early_suspend(void)
{
/* LGE_CHANGE_S [yoohoo@lge.com] 2009-01-14, Support Host Wakeup */
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	if (dhd_early_suspend_ctrl.drv_loaded == FALSE)
		return;
	
#if defined(CONFIG_BRCM_GPIO_INTR)
	/* HostWake up */
	dhd_unregister_hwakeup();
#endif	/* defined(CONFIG_BRCM_GPIO_INTR) */

	unregister_mmc_card_pm();

	/* Destroy the wake lock*/
	wake_lock_destroy(&wlan_host_wakelock);
	wake_lock_destroy(&wlan_host_wakelock_resume);
#else	/* CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
	return;
#endif /*  CONFIG_BRCM_LGE_WL_HOSTWAKEUP */
/* LGE_CHANGE_E [yoohoo@lge.com] 2010-01-14, Support Host Wakeup */
}
#endif	/* #if defined(CONFIG_HAS_EARLYSUSPEND) */
