/* drivers/power/max17040.c
 *
 * Copyright (C) 2009 LGE, Inc
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

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <asm/gpio.h>
#include <asm/system.h>

#define I2C_NO_REG 0xFF
#define MODULE_NAME    "max17040"

struct max17040_device {
	struct i2c_client *client;
	struct delayed_work work;
	int vol_level;
	int percent_level;
	int init_flag;
};

static struct max17040_device *max17040_dev = NULL;

static int max17040_write_reg(struct i2c_client *client, unsigned char reg, unsigned char val)
{
	int err;
	unsigned char buf[2];
	struct i2c_msg msg = {
		client->addr, 0, 2, buf
	};

	printk(KERN_INFO"%s: \n",__func__);
	
	buf[0] = reg;
	buf[1] = val;

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}

	return 0;
}

static int max17040_read_reg(struct i2c_client *client, unsigned char reg, unsigned char *ret)
{
	int err;

	struct i2c_msg msg[2] = {
		{ client->addr, 0, 1, &reg },
		{ client->addr, I2C_M_RD, 1, ret}
	};

	if ((err = i2c_transfer(client->adapter, msg, 2)) < 0) {
		dev_err(&client->dev, "i2c read error\n");
	}

	return 0;
}

static void max17040_init_chip(void)
{
	printk(KERN_INFO"%s: start...\n",__func__);
	
	if (!i2c_check_functionality(max17040_dev->client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "%s: need I2C_FUNC_I2C\n", __func__);
		return;
	}

	max17040_write_reg(max17040_dev->client, 0x06, 0x40); max17040_write_reg(max17040_dev->client, 0x07, 0x00);

	return;
}
static ssize_t max17040_show_vol(struct device *dev, struct device_attribute *attr, char *buf)
{

	int r;
	printk("%s()\n",__FUNCTION__);

	r = snprintf(buf, PAGE_SIZE, "%d\n", max17040_dev->vol_level);

	return r;
}

static ssize_t max17040_show_percent(struct device *dev, struct device_attribute *attr, char *buf)
{

	int r;
	printk("%s()\n",__FUNCTION__);

	r = snprintf(buf, PAGE_SIZE, "%d\n", max17040_dev->percent_level);

	return r;
}

DEVICE_ATTR(current_vol, 0444, max17040_show_vol, NULL);
DEVICE_ATTR(current_percent, 0444, max17040_show_percent, NULL);

static void max17040_work_func(struct work_struct *work)
{
	unsigned char vcell[2];
	unsigned char soc[2];
	
	if (!i2c_check_functionality(max17040_dev->client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "%s: max17040 is not working\n", __func__);
	
		max17040_dev->vol_level = 0;
		max17040_dev->percent_level = 0;

		max17040_dev->init_flag = 0;
		schedule_delayed_work(&max17040_dev->work, HZ);
	
		return ;
	}

	if (!max17040_dev->init_flag)
		max17040_init_chip();

	max17040_dev->init_flag = 1;

	max17040_read_reg(max17040_dev->client, 0x02, &vcell[0]);
	max17040_read_reg(max17040_dev->client, 0x03, &vcell[1]);

	max17040_read_reg(max17040_dev->client, 0x04, &soc[0]);
	max17040_read_reg(max17040_dev->client, 0x05, &soc[1]);

	max17040_dev->vol_level = (vcell[0] << 4) | (vcell[1] >> 4);
	max17040_dev->percent_level = soc[0];

	schedule_delayed_work(&max17040_dev->work, HZ);
	
	return;
}

static int __init max17040_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct max17040_device *dev;
	int ret = 0;

	printk(KERN_INFO"%s: \n",__func__);

	dev = kzalloc(sizeof(struct max17040_device), GFP_KERNEL);
	if(dev == NULL) {
		printk(KERN_ERR "%s: fail alloc for max17040_device\n", __func__);
		return -ENOMEM;
	}

	dev->vol_level = 0;
	dev->percent_level = 0;
	dev->client = i2c_dev;
	max17040_dev = dev;
	
	i2c_set_clientdata(i2c_dev, max17040_dev);

	INIT_DELAYED_WORK(&max17040_dev->work, max17040_work_func);
	schedule_delayed_work(&max17040_dev->work, HZ);

	ret = device_create_file(&i2c_dev->dev, &dev_attr_current_vol);
	if (ret)
		return ret;
	ret = device_create_file(&i2c_dev->dev, &dev_attr_current_percent);
	if (ret)
		return ret;

	return 0;
}

static int max17040_remove(struct i2c_client *i2c_dev)
{
	struct max17040_device *dev;

	printk(KERN_INFO"%s: start...\n",__func__);
   	
	device_remove_file(&i2c_dev->dev, &dev_attr_current_vol);
	device_remove_file(&i2c_dev->dev, &dev_attr_current_percent);
	dev = (struct max17040_device *)i2c_get_clientdata(i2c_dev);
	i2c_set_clientdata(i2c_dev, NULL);
	
	return 0;
}

static struct i2c_device_id max17040_idtable[] = {
        { "max17040", 0 },
};

MODULE_DEVICE_TABLE(i2c, max17040_idtable);

static struct i2c_driver max17040_driver = {
	.probe = max17040_probe,
	.remove = max17040_remove,
	.id_table = max17040_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init max17040_init(void)
{
	printk(KERN_INFO"%s: start\n",__func__);	

	return i2c_add_driver(&max17040_driver);
}

module_init(max17040_init);

MODULE_DESCRIPTION("MAX17040 Fuel Guage Driver");
MODULE_LICENSE("GPL");
