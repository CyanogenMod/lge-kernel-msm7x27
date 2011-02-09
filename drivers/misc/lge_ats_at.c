/*
 *  lge_alohag_at.c
 *
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/lge_alohag_at.h>
#include <linux/slab.h>

#define DEBUG_AT	1

#if DEBUG_AT
#define D(fmt, args...) printk(fmt, ##args)
#else
#define D(fmt, args...) do () while(0)
#endif


struct atcmd_data {
	struct atcmd_dev sdev;
};
static struct atcmd_data *atcmd_data;

struct atcmd_dev *atcmd_get_dev(void)
{
	return &(atcmd_data->sdev);
}
EXPORT_SYMBOL(atcmd_get_dev);

static int atcmd_probe(struct platform_device *pdev)
{
	struct atcmd_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;

	if (!pdata){
		D("atcmd_probe pdata err:%s\n", pdata->name);
		return -EBUSY;
	}

	D("%s:%s\n", __func__, pdata->name);
	atcmd_data = kzalloc(sizeof(struct atcmd_data), GFP_KERNEL);
	if (!atcmd_data){
		D("atcmd_probe data err:%s\n", pdata->name);
		return -ENOMEM;
	}

	atcmd_data->sdev.name = (char *)pdata->name;
    ret = atcmd_dev_register(&atcmd_data->sdev);
	if (ret < 0)
		goto err_atcmd_dev_register;


	return 0;

err_atcmd_dev_register:
	kfree(atcmd_data);

	return ret;
}

static int __devexit atcmd_remove(struct platform_device *pdev)
{
	struct atcmd_data *atcmd = platform_get_drvdata(pdev);

    atcmd_dev_unregister(&atcmd->sdev);
	kfree(atcmd_data);

	return 0;
}

static struct platform_driver atcmd_driver = {
	.remove		= __devexit_p(atcmd_remove),
	.driver		= {
		.name	= "alohag_atcmd",
		.owner	= THIS_MODULE,
	},
};

static int __init atcmd_init(void)
{
	return platform_driver_probe(&atcmd_driver, atcmd_probe);
}

static void __exit atcmd_exit(void)
{
	platform_driver_unregister(&atcmd_driver);
}

module_init(atcmd_init);
module_exit(atcmd_exit);

MODULE_AUTHOR("kimeh@lge.com");
MODULE_DESCRIPTION("alohag_atcmd driver");
MODULE_LICENSE("GPL");
