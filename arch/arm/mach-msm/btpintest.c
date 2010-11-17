/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
 */
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include "gpiomux.h"

#define VERSION     "1.0"
struct dentry *pin_debugfs_dent;

/* UART GPIO lines for 8660 */
enum uartpins {
	UARTDM_TX = 53,
	UARTDM_RX = 54,
	UARTDM_CTS = 55,
	UARTDM_RFR = 56
};

/* Aux PCM GPIO lines for 8660 */
enum auxpcmpins {
	AUX_PCM_CLK = 114,
	AUX_PCM_SYNC = 113,
	AUX_PCM_DIN  = 112,
	AUX_PCM_DOUT = 111
};
/*Number of UART or PCM pins */
#define PIN_COUNT 4
#define PIN_TEST_CONFIG {.func = GPIOMUX_FUNC_GPIO, .drv = GPIOMUX_DRV_2MA, \
						.pull = GPIOMUX_PULL_NONE }

static struct msm_gpiomux_config pcm_test_config[]  = {
	{
		.gpio = AUX_PCM_DOUT,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = AUX_PCM_DIN,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = AUX_PCM_SYNC,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = AUX_PCM_CLK,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
};

static struct msm_gpiomux_config uart_test_config[] = {
	{
		.gpio = UARTDM_TX,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = UARTDM_RX,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = UARTDM_CTS,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
	{
		.gpio = UARTDM_RFR,
		.settings = {
			[GPIOMUX_ACTIVE]    = PIN_TEST_CONFIG,
			[GPIOMUX_SUSPENDED] = PIN_TEST_CONFIG,
		},
	},
};

static struct msm_gpiomux_config aux_pcm_config[PIN_COUNT];
static struct msm_gpiomux_config uart_config[PIN_COUNT];

static int pintest_open(struct inode *inode, struct file *file)
{
	/* non-seekable */
	file->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE);
	return 0;
}

static int pintest_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int configure_pins(struct msm_gpiomux_config *config,
				struct msm_gpiomux_config *oldconfig,
				unsigned int num_configs)
{
	int rc = 0, j, i;
	for (i = 0; i < num_configs; i++) {
		for (j = 0; j < GPIOMUX_NSETTINGS; j++) {
			(oldconfig + i)->gpio = (config + i)->gpio;
			rc = msm_gpiomux_write((config + i)->gpio,
				j,
				&(config + i)->settings[j],
				&(oldconfig + i)->settings[j]);
			if (rc < 0)
				break;
		}

	}
	return rc;
}

static ssize_t pintest_write(
	struct file *file,
	const char __user *buff,
	size_t count,
	loff_t *ppos)
{
	char mode;
	int rc = 0;

	if (count < 1)
		return -EINVAL;

	if (buff == NULL)
		return -EINVAL;

	if (copy_from_user(&mode, buff, count))
		return -EFAULT;
	mode = mode - '0';

	if (mode) {
		/* Configure all AuX PCM gpios for the custom settings */
		rc = configure_pins(pcm_test_config, aux_pcm_config,
					ARRAY_SIZE(pcm_test_config));
		if (rc < 0)
			return rc;

		/* Configure all UART gpios for the custom settings */
		rc = configure_pins(uart_test_config, uart_config,
					ARRAY_SIZE(uart_test_config));
	} else {
		/* Configure all AuX PCM gpios for the original settings */
		rc = configure_pins(aux_pcm_config, pcm_test_config,
					ARRAY_SIZE(pcm_test_config));
		if (rc < 0)
			return rc;

		/* Configure all UART gpios for the original settings */
		rc = configure_pins(uart_config, uart_test_config,
					ARRAY_SIZE(uart_test_config));
	}
	return rc;
}

static const struct file_operations pintest_debugfs_fops = {
	.open = pintest_open,
	.release = pintest_release,
	.write = pintest_write,
};

static int __init bluepintest_init(void)
{
	pin_debugfs_dent = debugfs_create_dir("btpintest", NULL);

	if (IS_ERR(pin_debugfs_dent)) {
		printk(KERN_ERR "%s(%d): debugfs_create_dir fail, error %ld\n",
			__FILE__, __LINE__, PTR_ERR(pin_debugfs_dent));
		return -ENOMEM;
	}

	if (debugfs_create_file("enable", 0644, pin_debugfs_dent,
					0, &pintest_debugfs_fops) == NULL) {
		printk(KERN_ERR "%s(%d): debugfs_create_file: index fail\n",
							__FILE__, __LINE__);
		return -ENOMEM;
	}
	return 0;
}

static void __exit bluepintest_exit(void)
{
	debugfs_remove_recursive(pin_debugfs_dent);
}

module_init(bluepintest_init);
module_exit(bluepintest_exit);

MODULE_DESCRIPTION("Bluetooth Pin Connectivty Test Driver ver %s " VERSION);
MODULE_LICENSE("GPL v2");
