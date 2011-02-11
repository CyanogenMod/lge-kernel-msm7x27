/*
 * Power sink driver
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * Author: Munyoung Hwang <munyoung.hwang@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/platform_device.h>
#include <linux/pm.h>

int lge_pwrsink_suspend_noirq(struct device *dev)
{
	struct dev_pm_ops *pm_ops = (struct dev_pm_ops*)dev_get_drvdata(dev);

	if(pm_ops && pm_ops->suspend_noirq)
		return pm_ops->suspend_noirq(dev);

	printk(KERN_ERR"%s: pm_ops or suspend_noirq func ptr is null\n", __func__); 
	return 0;
}

int lge_pwrsink_resume_noirq(struct device *dev)
{
	struct dev_pm_ops *pm_ops = (struct dev_pm_ops*)dev_get_drvdata(dev);

	if(pm_ops && pm_ops->resume_noirq)
		return pm_ops->resume_noirq(dev);

	printk(KERN_ERR"%s: pm_ops or resume_noirq func ptr is null\n", __func__); 
	return 0;
}

static int lge_pwrsink_probe(struct platform_device *pdev)
{
	printk(KERN_INFO"%s: setting driver data\n", __func__);
	dev_set_drvdata(&pdev->dev, pdev->dev.platform_data);
	return 0;
}

static int lge_pwrsink_remove(struct platform_device *dev)
{
	printk(KERN_INFO"%s\n", __func__);
	return 0;
}

static struct dev_pm_ops lge_pwrsink_pm_ops = {
	.suspend_noirq = lge_pwrsink_suspend_noirq,
	.resume_noirq = lge_pwrsink_resume_noirq,
};

static struct platform_driver lge_pwrsink_driver = {
	.remove = lge_pwrsink_remove,
	.driver = {
		.name = "lge-pwrsink",
		.pm = &lge_pwrsink_pm_ops,
	},
};

static int __init lge_pwrsink_init(void)
{
	printk(KERN_INFO "LGE Power Sink Driver Init\n");
	return platform_driver_probe(&lge_pwrsink_driver, lge_pwrsink_probe);
}

static void __exit lge_pwrsink_exit(void)
{
	printk(KERN_INFO "LGE Power Sink Driver Exit\n");
	platform_driver_unregister(&lge_pwrsink_driver);
}

module_init(lge_pwrsink_init);
module_exit(lge_pwrsink_exit);

MODULE_AUTHOR("LG Electronics Inc.");
MODULE_DESCRIPTION("LGE Power Sink Driver");
MODULE_LICENSE("GPL");

