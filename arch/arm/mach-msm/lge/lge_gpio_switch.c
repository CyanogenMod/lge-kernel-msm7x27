/*
 * arch/arm/mach-msm/lge/lge_gpio_switch.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <mach/board_lge.h>

static LIST_HEAD(switchs);

struct lge_gpio_switch_data {
	int dev_id;
	struct list_head list;
	struct switch_dev sdev;
	unsigned *gpios;
	size_t num_gpios;
	unsigned long irqflags;
	unsigned int wakeup_flag;
	int *irqs;
	struct work_struct work;
	int (*work_func)(void);
	char *(*print_state)(int state);
	int (*sysfs_store)(const char *buf, size_t size);
};

struct lge_gpio_switch_data *lge_switch_data;

static void gpio_switch_work(struct work_struct *work)
{
	int state;
	struct lge_gpio_switch_data	*data =
		container_of(work, struct lge_gpio_switch_data, work);
	
	state = data->work_func();
	switch_set_state(&data->sdev, state);
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	struct lge_gpio_switch_data *switch_data =
	    (struct lge_gpio_switch_data *)dev_id;
	
	schedule_work(&switch_data->work);
	return IRQ_HANDLED;
}

static ssize_t switch_gpio_print_state(struct switch_dev *sdev, char *buf)
{
	struct lge_gpio_switch_data	*switch_data =
		container_of(sdev, struct lge_gpio_switch_data, sdev);
	const char *state;
	int cur_state;

	cur_state = switch_get_state(sdev);
	if (switch_data->print_state)
		state = switch_data->print_state(cur_state);
	else
		return -1;

	if (state)
		return sprintf(buf, "%s\n", state);

	return -1;
}

static ssize_t sysfs_mode_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int state;
	struct lge_gpio_switch_data *switch_data;
	struct lge_gpio_switch_platform_data *pdata;

	pdata = (struct lge_gpio_switch_platform_data *)dev->platform_data;

	list_for_each_entry(switch_data, &switchs, list) 
		if (!strcmp(switch_data->sdev.name,	pdata->name)) 
			goto found_it;

	printk(KERN_INFO"%s: not found %s\n",__func__, pdata->name);
	return size;

found_it:			
	state = switch_data->sysfs_store(buf, size);
	switch_set_state(&switch_data->sdev, state);
	
	return size;
}

static DEVICE_ATTR(report, S_IRUGO | S_IWUSR, NULL , sysfs_mode_store);

int lge_gpio_switch_pass_event(char *sdev_name, int state)
{
	struct lge_gpio_switch_data *switch_data;

	list_for_each_entry(switch_data, &switchs, list) 
		if (!strcmp(switch_data->sdev.name,	sdev_name)) 
			goto found_it;

	printk(KERN_INFO"%s: not found %s\n",__func__, sdev_name);
	return -1;

found_it:			
	switch_set_state(&switch_data->sdev, state);
	
	return 0;
}


static int lge_gpio_switch_probe(struct platform_device *pdev)
{
	struct lge_gpio_switch_platform_data *pdata = pdev->dev.platform_data;
	struct lge_gpio_switch_data *switch_data;
	int ret = 0;
	int index;

	if (!pdata)
		return -EBUSY;

	switch_data = kzalloc(sizeof(struct lge_gpio_switch_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;

	switch_data->dev_id = pdev->id;
	switch_data->sdev.name = pdata->name;
	switch_data->gpios = pdata->gpios;
	switch_data->num_gpios = pdata->num_gpios;
	switch_data->irqflags = pdata->irqflags;
	switch_data->wakeup_flag = pdata->wakeup_flag;
	switch_data->work_func = pdata->work_func;
	switch_data->print_state = pdata->print_state;
	switch_data->sysfs_store = pdata->sysfs_store;
	switch_data->sdev.print_state = switch_gpio_print_state;
	switch_data->irqs = kzalloc(sizeof(int) * pdata->num_gpios, GFP_KERNEL);

	list_add_tail(&switch_data->list, &switchs);

    ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0)
		goto err_switch_dev_register;

	for(index = 0; index < switch_data->num_gpios; index++) {
		gpio_tlmm_config(GPIO_CFG(switch_data->gpios[index], 0, 
					GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
					GPIO_CFG_2MA), GPIO_CFG_ENABLE);

		ret = gpio_request(switch_data->gpios[index], pdev->name);
		if (ret < 0)
			goto err_request_gpio;

		ret = gpio_direction_input(switch_data->gpios[index]);
		if (ret < 0)
			goto err_request_gpio;
	}
	
	INIT_WORK(&switch_data->work, gpio_switch_work);

	for(index = 0; index < switch_data->num_gpios; index++) {
		switch_data->irqs[index] = gpio_to_irq(switch_data->gpios[index]);
		if (switch_data->irqs[index] < 0) {
			ret = switch_data->irqs[index];
			goto err_request_gpio;
		}

		ret = request_irq(switch_data->irqs[index], gpio_irq_handler,
				switch_data->irqflags,
				pdata->name, switch_data);
		if (ret < 0)
			goto err_request_gpio;
		
		if (switch_data->wakeup_flag)
			set_irq_wake(switch_data->irqs[index], 1);
	}	

	if (switch_data->sysfs_store) {
		ret = device_create_file(&pdev->dev, &dev_attr_report);
		if (ret) {
			device_remove_file(&pdev->dev, &dev_attr_report);
			goto err_request_gpio;
		}
	}

	/* additional init for each board */
	if (pdata->additional_init)
		pdata->additional_init();

	/* Perform initial detection */
	gpio_switch_work(&switch_data->work);

	return 0;

err_request_gpio:
    switch_dev_unregister(&switch_data->sdev);
err_switch_dev_register:
	kfree(switch_data);

	return ret;
}

static int __devexit lge_gpio_switch_remove(struct platform_device *pdev)
{
	struct lge_gpio_switch_data *switch_data = platform_get_drvdata(pdev);

	cancel_work_sync(&switch_data->work);
    switch_dev_unregister(&switch_data->sdev);

	return 0;
}

static struct platform_driver lge_gpio_switch_driver = {
	.remove		= __devexit_p(lge_gpio_switch_remove),
	.driver		= {
		.name	= "lge-switch-gpio",
		.owner	= THIS_MODULE,
	},
};

static int __init lge_gpio_switch_init(void)
{
	return platform_driver_probe(&lge_gpio_switch_driver, lge_gpio_switch_probe);
}

static void __exit lge_gpio_switch_exit(void)
{
	platform_driver_unregister(&lge_gpio_switch_driver);
}

module_init(lge_gpio_switch_init);
module_exit(lge_gpio_switch_exit);

MODULE_DESCRIPTION("LGE GPIO Switch driver");
MODULE_LICENSE("GPL");
