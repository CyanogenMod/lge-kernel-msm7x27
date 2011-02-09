/* drivers/misc/lge_ats_uevent.c
 *
 * Copyright (C) 2010 LGE.
 * Author: Munyoung Hwang <munyoung.hwang@lge.com>
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

#include <linux/platform_device.h>

static void ats_uevent_send(struct device *dev, char *cmd)
{
	char *str;
	char *envp[3];

	if(cmd) {
		str = strchr(cmd, ' ');
		if(!str || !(str+1)) {
			printk(KERN_ERR"%s: cmd format is invalid\n", __func__);
			return;
		}
		*str = '\0';
		envp[0] = cmd;
		envp[1] = str + 1;
		envp[2] = NULL;
		kobject_uevent_env(&dev->kobj, KOBJ_CHANGE, envp);
	}
}

static ssize_t ats_uevent_cmd_store(struct device *dev, struct device_attribute *attr,
						 const char *buf, size_t size)
{
	char *cmd;
	int len;

	cmd = kstrdup(buf, size);
	if(cmd) {
		printk(KERN_INFO"%s: send cmd: %s\n", __func__, cmd);
		len = strlen(cmd);
		if(cmd[len-1] == '\n')
			cmd[len-1] = '\0';
		ats_uevent_send(dev, cmd);
		kfree(cmd);
	}
	return size;
}
static DEVICE_ATTR(cmd, S_IRUSR | S_IWUSR, NULL, ats_uevent_cmd_store);

static int ats_uevent_probe(struct platform_device *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_cmd);
	if(ret < 0) {
		printk(KERN_ERR "%s: Error while creating device file\n", __func__);
		return ret;
	}
	return 0;
}

static int ats_uevent_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_cmd);
	return 0;
}

static struct platform_driver ats_uevent_driver = {
	.remove = ats_uevent_remove,
	.driver = {
		.name = "ats_uevent",
	},
};

static int __init ats_uevent_init(void)
{
	printk(KERN_INFO "%s: ats uevent driver\n", __func__);
	return platform_driver_probe(&ats_uevent_driver, ats_uevent_probe);
}

static void __exit ats_uevent_exit(void)
{
	platform_driver_unregister(&ats_uevent_driver);
}

module_init(ats_uevent_init);
module_exit(ats_uevent_exit);

MODULE_AUTHOR("LG Electronics Inc.");
MODULE_DESCRIPTION("ATS uevent device driver");
MODULE_LICENSE("GPL");

