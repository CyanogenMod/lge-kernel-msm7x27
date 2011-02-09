/*
 *   LG_FW_AUDIO_TESTMODE
 *
 *   kiwone creates this file for audio test mode, and the use of another function to send framework.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include "lg_fw_diag_communication.h"


#define DEBUG_DIAG 1

#if DEBUG_DIAG
#define D(fmt, args...) printk(fmt, ##args)
#else
#define D(fmt, args...) do () while(0)
#endif

struct diagcmd_data {
	struct diagcmd_dev sdev;
};
static struct diagcmd_data *diagcmd_data;

struct diagcmd_dev *diagcmd_get_dev(void)
{
	return &(diagcmd_data->sdev);
}
EXPORT_SYMBOL(diagcmd_get_dev);

static int diagcmd_probe(struct platform_device *pdev)
{
	struct diagcmd_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;

	if (!pdata){
		D("diagcmd_probe pdata err:%s\n", pdata->name);
		return -EBUSY;
	}

	D("%s:%s\n", __func__, pdata->name);
	diagcmd_data = kzalloc(sizeof(struct diagcmd_data), GFP_KERNEL);
	if (!diagcmd_data){
		D("diagcmd_probe data err:%s\n", pdata->name);
		return -ENOMEM;
	}

	diagcmd_data->sdev.name = pdata->name;
    ret = diagcmd_dev_register(&diagcmd_data->sdev);
	if (ret < 0)
		goto err_diagcmd_dev_register;


	return 0;

err_diagcmd_dev_register:
	kfree(diagcmd_data);

	return ret;
}

static int __devexit diagcmd_remove(struct platform_device *pdev)
{
	struct diagcmd_data *atcmd = platform_get_drvdata(pdev);

    diagcmd_dev_unregister(&atcmd->sdev);
	kfree(diagcmd_data);

	return 0;
}

static struct platform_driver diagcmd_driver = {
	.remove		= __devexit_p(diagcmd_remove),
	.driver		= {
		.name	= "lg_fw_diagcmd",
		.owner	= THIS_MODULE,
	},
};

static int __init diagcmd_init(void)
{
	return platform_driver_probe(&diagcmd_driver, diagcmd_probe);
}

static void __exit diagcmd_exit(void)
{
	platform_driver_unregister(&diagcmd_driver);
}

module_init(diagcmd_init);
module_exit(diagcmd_exit);

MODULE_AUTHOR("kiwone.seo@lge.com");
MODULE_DESCRIPTION("lg_fw_diagcmd driver");
MODULE_LICENSE("GPL");

