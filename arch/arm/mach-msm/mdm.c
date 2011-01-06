/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
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

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/ioctl.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>
#include "mdm.h"
#include "mdm_ioctls.h"

#define AP2MDM_STATUS	136
#define MDM2AP_STATUS	134
#define MDM2AP_WAKEUP	40
#define MDM2AP_ERRFATAL	133
#define AP2MDM_ERRFATAL	93

#define AP2MDM_PMIC_RESET_N	131
#define AP2MDM_KPDPWR_N	132
#define AP2PMIC_TMPNI_CKEN	141

/*
 * If defined, when MDM2AP_ERRFATAL goes high, an IRQ will occur which will
 * trigger a panic. This is here for testing purposes in cases where a
 * panic/restart is not desireable.
 */
#define CHARM_IRQ_FATAL

static void (*power_on_charm)(void);
static void (*power_down_charm)(void);

static long charm_modem_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{

	int ret = 0;

	if (_IOC_TYPE(cmd) != CHARM_CODE) {
		pr_err("%s: invalid ioctl code\n", __func__);
		return -EINVAL;
	}

	gpio_request(MDM2AP_STATUS, "MDM2AP_STATUS");
	gpio_direction_input(MDM2AP_STATUS);
#ifndef CHARM_IRQ_FATAL
	gpio_request(MDM2AP_ERRFATAL, "MDM2AP_ERRFATAL");
	gpio_direction_input(MDM2AP_STATUS);
#endif

	switch (cmd) {
	case WAKE_CHARM:
		/* turn the charm on */
		power_on_charm();
		break;

	case RESET_CHARM:
		/* put the charm back in reset */
		power_down_charm();
		break;

	case CHECK_FOR_BOOT:
		if (gpio_get_value(MDM2AP_STATUS) == 0)
			put_user(1, (unsigned long __user *) arg);
		else
			put_user(0, (unsigned long __user *) arg);
		break;

	case WAIT_FOR_BOOT:
		/* wait for status to be high */
		while (gpio_get_value(MDM2AP_STATUS) == 0)
			;
		break;

#ifndef CHARM_IRQ_FATAL
	case WAIT_FOR_EVENT:
		/* wait for error fatal to be high */
		while (gpio_get_value(MDM2AP_ERRFATAL) == 0)
			;
		break;
#endif

	default:
		ret = -EINVAL;
	}

#ifndef CHARM_IRQ_FATAL
	gpio_free(MDM2AP_ERRFATAL);
#endif
	gpio_free(MDM2AP_STATUS);

	return ret;
}

static int charm_modem_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations charm_modem_fops = {
	.owner		= THIS_MODULE,
	.open		= charm_modem_open,
	.unlocked_ioctl	= charm_modem_ioctl,
};


struct miscdevice charm_modem_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "mdm",
	.fops	= &charm_modem_fops
};

#ifdef CHARM_IRQ_FATAL

static void mdm_fatal_fn(struct work_struct *work)
{
	pr_debug("%s: Got an error fatal!\n", __func__);
}

static DECLARE_WORK(mdm_fatal_work, mdm_fatal_fn);

static int irqc;

static irqreturn_t errfatal(int irq, void *dev_id)
{
	pr_debug("charm got errfatal! Scheduling work to panic now...\n");
	irqc++;
	schedule_work(&mdm_fatal_work);
	disable_irq_nosync(MSM_GPIO_TO_INT(MDM2AP_ERRFATAL));
	return IRQ_HANDLED;
}
#endif

static int __init charm_modem_probe(struct platform_device *pdev)
{
#ifdef CHARM_IRQ_FATAL
	int ret, irq;
#endif
	struct charm_platform_data *d = pdev->dev.platform_data;

	gpio_request(AP2MDM_STATUS, "AP2MDM_STATUS");
	gpio_request(AP2MDM_ERRFATAL, "AP2MDM_ERRFATAL");

	gpio_direction_output(AP2MDM_STATUS, 1);
	gpio_direction_output(AP2MDM_ERRFATAL, 0);

	power_on_charm = d->charm_modem_on;
	power_down_charm = d->charm_modem_off;

#ifdef CHARM_IRQ_FATAL
	gpio_request(MDM2AP_ERRFATAL, "MDM2AP_ERRFATAL");
	gpio_direction_input(MDM2AP_ERRFATAL);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("%s: could not get MDM2AP_ERRFATAL IRQ resource. \
			error=%d No IRQ will be generated on errfatal.",
			__func__, irq);
		goto out;
	}

	ret = request_irq(irq, errfatal,
		IRQF_TRIGGER_HIGH , "charm errfatal", NULL);

	if (ret < 0) {
		pr_err("%s: MDM2AP_ERRFATAL IRQ#%d request failed with error=%d\
			. No IRQ will be generated on errfatal.",
			__func__, irq, ret);
	}

out:
#endif
	pr_info("%s: Registering charm modem\n", __func__);

	return misc_register(&charm_modem_misc);
}


static int __devexit charm_modem_remove(struct platform_device *pdev)
{

	return misc_deregister(&charm_modem_misc);
}

static struct platform_driver charm_modem_driver = {
	.remove         = charm_modem_remove,
	.driver         = {
		.name = "charm_modem",
		.owner = THIS_MODULE
	},
};

static int __init charm_modem_init(void)
{
	return platform_driver_probe(&charm_modem_driver, charm_modem_probe);
}

static void __exit charm_modem_exit(void)
{
	platform_driver_unregister(&charm_modem_driver);
}

module_init(charm_modem_init);
module_exit(charm_modem_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("msm8660 charm modem driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("charm_modem")


