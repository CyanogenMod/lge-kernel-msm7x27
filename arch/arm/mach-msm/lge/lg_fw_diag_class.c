/*
 *   LG_FW_AUDIO_TESTMODE
 *
 *   kiwone creates this file for audio test mode, and the use of another function to send framework.
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include "lg_fw_diag_communication.h"

struct class *lg_fw_diag_class;
static atomic_t device_count;

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct diagcmd_dev *sdev = (struct diagcmd_dev *)
		dev_get_drvdata(dev);
	printk("\n%s:%d\n", __func__, sdev->state);
	return sprintf(buf, "%d\n", sdev->state);
}

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct diagcmd_dev *sdev = (struct diagcmd_dev *)
		dev_get_drvdata(dev);
	printk("\n%s:%s\n", __func__, sdev->name);
	return sprintf(buf, "%s\n", sdev->name);
}

static DEVICE_ATTR(state, S_IRUGO | S_IWUSR, state_show, NULL);
static DEVICE_ATTR(name, S_IRUGO | S_IWUSR, name_show, NULL);

void update_diagcmd_state(struct diagcmd_dev *sdev, char *cmd, int state)
{
	char name_buf[120];
	char state_buf[120];
	char *prop_buf;
	char *envp[3];
	int env_offset = 0;
	int length;

	/* 
	 * 2010-08-12, jinkyu.choi@lge.com, Do not check the state
	 * Now, each command has own state number which is the sub command number of testmode tools.
	 * The sub commands can be same though the major commands are different.
	 * It is result in not sending the commnad to Android Diag application
	 */

	//if (sdev->state != state) {
		sdev->state = state;
		sdev->name = cmd;

		prop_buf = (char *)get_zeroed_page(GFP_KERNEL);
		if (prop_buf) {
			length = name_show(sdev->dev, NULL, prop_buf);
			if (length > 0) {
				if (prop_buf[length - 1] == '\n')
					prop_buf[length - 1] = 0;
				snprintf(name_buf, sizeof(name_buf),
					"DIAG_NAME=%s", prop_buf);
				envp[env_offset++] = name_buf;
			}
			length = state_show(sdev->dev, NULL, prop_buf);
			if (length > 0) {
				if (prop_buf[length - 1] == '\n')
					prop_buf[length - 1] = 0;
				snprintf(state_buf, sizeof(state_buf),
					"DIAG_STATE=%s", prop_buf);
				envp[env_offset++] = state_buf;
			}
			envp[env_offset] = NULL;
			kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);
			free_page((unsigned long)prop_buf);
		} else {
			printk(KERN_ERR "out of memory in update_diagcmd_state\n");
			kobject_uevent(&sdev->dev->kobj, KOBJ_CHANGE);
		}
	//}
}
EXPORT_SYMBOL_GPL(update_diagcmd_state);

static int create_lg_fw_diag_class(void)
{
	if (!lg_fw_diag_class) {
		lg_fw_diag_class = class_create(THIS_MODULE, "lg_fw_diagclass");
		if (IS_ERR(lg_fw_diag_class))
			return PTR_ERR(lg_fw_diag_class);
		atomic_set(&device_count, 0);
	}

	return 0;
}

int diagcmd_dev_register(struct diagcmd_dev *sdev)
{
	int ret;

	if (!lg_fw_diag_class) {
		ret = create_lg_fw_diag_class();
		if (ret < 0)
			return ret;
	}

	sdev->index = atomic_inc_return(&device_count);
	sdev->dev = device_create(lg_fw_diag_class, NULL,
		MKDEV(0, sdev->index), NULL, sdev->name);
	if (IS_ERR(sdev->dev))
		return PTR_ERR(sdev->dev);

	ret = device_create_file(sdev->dev, &dev_attr_state);
	if (ret < 0)
		goto err_create_file_1;
	ret = device_create_file(sdev->dev, &dev_attr_name);
	if (ret < 0)
		goto err_create_file_2;

	dev_set_drvdata(sdev->dev, sdev);
	sdev->state = 0;
	return 0;

err_create_file_2:
	device_remove_file(sdev->dev, &dev_attr_state);
err_create_file_1:
	device_destroy(lg_fw_diag_class, MKDEV(0, sdev->index));
	printk(KERN_ERR "lg_fw_diagcmd: Failed to register driver %s\n", sdev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(diagcmd_dev_register);

void diagcmd_dev_unregister(struct diagcmd_dev *sdev)
{
	device_remove_file(sdev->dev, &dev_attr_name);
	device_remove_file(sdev->dev, &dev_attr_state);
	device_destroy(lg_fw_diag_class, MKDEV(0, sdev->index));
	dev_set_drvdata(sdev->dev, NULL);
}
EXPORT_SYMBOL_GPL(diagcmd_dev_unregister);

static int __init lg_fw_diag_class_init(void)
{
	return create_lg_fw_diag_class();
}

static void __exit lg_fw_diag_class_exit(void)
{
	class_destroy(lg_fw_diag_class);
}

module_init(lg_fw_diag_class_init);
module_exit(lg_fw_diag_class_exit);

MODULE_AUTHOR("kiwone.seo@lge.com");
MODULE_DESCRIPTION("lg_fw_diag class driver");
MODULE_LICENSE("GPL");
