/*
 *  lge_alohag_at_class.c
 *
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/lge_alohag_at.h>


struct class *atcmd_class;
static atomic_t device_count;

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct atcmd_dev *sdev = (struct atcmd_dev *)
		dev_get_drvdata(dev);
	printk("\n%s:%d\n", __func__, sdev->state);
	return sprintf(buf, "%d\n", sdev->state);
}
#if 1 
static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct atcmd_dev *sdev = (struct atcmd_dev *)
		dev_get_drvdata(dev);
	printk("\n%s:%s\n", __func__, sdev->name);
	return sprintf(buf, "%s\n", sdev->name);
//	return sprintf(buf, "%s\n", "vlc");
}
#endif
static DEVICE_ATTR(state, S_IRUGO | S_IWUSR, state_show, NULL);
static DEVICE_ATTR(name, S_IRUGO | S_IWUSR, name_show, NULL);

void update_atcmd_state(struct atcmd_dev *sdev, char *cmd, int state)
{
	char name_buf[120];
	char state_buf[120];
	char *prop_buf;
	char *envp[3];
	int env_offset = 0;
	int length;

	if (sdev->state != state) {
		sdev->state = state;
		sdev->name = cmd;

		prop_buf = (char *)get_zeroed_page(GFP_KERNEL);
		if (prop_buf) {
			length = name_show(sdev->dev, NULL, prop_buf);
			if (length > 0) {
				if (prop_buf[length - 1] == '\n')
					prop_buf[length - 1] = 0;
				snprintf(name_buf, sizeof(name_buf),
					"AT_NAME=%s", prop_buf);
//				snprintf(name_buf, sizeof(name_buf), prop_buf);
				envp[env_offset++] = name_buf;
			}

			length = state_show(sdev->dev, NULL, prop_buf);
			if (length > 0) {
				if (prop_buf[length - 1] == '\n')
					prop_buf[length - 1] = 0;
//				snprintf(state_buf, sizeof(state_buf), prop_buf);
				snprintf(state_buf, sizeof(state_buf),
					"AT_STATE=%s", prop_buf);
				envp[env_offset++] = state_buf;
			}
			envp[env_offset] = NULL;
			kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);
			free_page((unsigned long)prop_buf);
		} else {
			printk(KERN_ERR "out of memory in update_vlc_state\n");
			kobject_uevent(&sdev->dev->kobj, KOBJ_CHANGE);
		}
	}
}

EXPORT_SYMBOL_GPL(update_atcmd_state);

static int create_atcmd_class(void)
{
	if (!atcmd_class) {
		atcmd_class = class_create(THIS_MODULE, "atcmd");
		if (IS_ERR(atcmd_class))
			return PTR_ERR(atcmd_class);
		atomic_set(&device_count, 0);
	}

	return 0;
}

int atcmd_dev_register(struct atcmd_dev *sdev)
{
	int ret;

	if (!atcmd_class) {
		ret = create_atcmd_class();
		if (ret < 0)
			return ret;
	}

	sdev->index = atomic_inc_return(&device_count);
	sdev->dev = device_create(atcmd_class, NULL,
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
	device_destroy(atcmd_class, MKDEV(0, sdev->index));
	printk(KERN_ERR "atcmd: Failed to register driver %s\n", sdev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(atcmd_dev_register);

void atcmd_dev_unregister(struct atcmd_dev *sdev)
{
	device_remove_file(sdev->dev, &dev_attr_name);
	device_remove_file(sdev->dev, &dev_attr_state);
	device_destroy(atcmd_class, MKDEV(0, sdev->index));
	dev_set_drvdata(sdev->dev, NULL);
}
EXPORT_SYMBOL_GPL(atcmd_dev_unregister);

static int __init atcmd_class_init(void)
{
	return create_atcmd_class();
}

static void __exit atcmd_class_exit(void)
{
	class_destroy(atcmd_class);
}

module_init(atcmd_class_init);
module_exit(atcmd_class_exit);

MODULE_AUTHOR("kimeh@lge.com");
MODULE_DESCRIPTION("LGE_c710_atcmd class driver");
MODULE_LICENSE("GPL");

