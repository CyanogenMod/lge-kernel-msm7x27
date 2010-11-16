/* arch/arm/mach-msm/board-thunderg-camera.c
 * Copyright (C) 2009 LGE, Inc.
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
#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <mach/camera.h>
#include <mach/msm_iomap.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#include "board-thunderg.h"

int mclk_rate = 24000000;

DEFINE_MUTEX(camera_power_mutex);
int camera_power_state;

void camera_power_mutex_lock()
{
	mutex_lock(&camera_power_mutex);
}

void camera_power_mutex_unlock()
{
	mutex_unlock(&camera_power_mutex);
}

struct i2c_board_info i2c_devices[1] = {
#if defined (CONFIG_ISX005)
	{
		I2C_BOARD_INFO("isx005", CAM_I2C_SLAVE_ADDR),
	},
#endif
};

#if defined (CONFIG_MSM_CAMERA)
static uint32_t camera_off_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(4,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
	GPIO_CFG(5,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
	GPIO_CFG(6,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(7,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(8,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(9,  0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(10, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(11, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(12, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* PCLK */
	GPIO_CFG(13, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(GPIO_CAM_MCLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(4,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT0 */
	GPIO_CFG(5,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT1 */
	GPIO_CFG(6,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT2 */
	GPIO_CFG(7,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT3 */
	GPIO_CFG(8,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT4 */
	GPIO_CFG(9,  1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT5 */
	GPIO_CFG(10, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT6 */
	GPIO_CFG(11, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* DAT7 */
	GPIO_CFG(12, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* PCLK */
	GPIO_CFG(13, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 1, GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), /* VSYNC_IN */
	GPIO_CFG(GPIO_CAM_MCLK, 1, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_16MA), /* MCLK */
};

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

int config_camera_on_gpios(void)
{
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
	return 0;
}

void config_camera_off_gpios(void)
{
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

int camera_power_on (void)
{
	int rc;
	struct device *dev = thunderg_backlight_dev();

	camera_power_mutex_lock();
	if(lcd_bl_power_state == BL_POWER_SUSPEND)
	{
		thunderg_pwrsink_resume();
		mdelay(50);
	}

	
	/* clear RESET, PWDN to Low*/
	gpio_set_value(GPIO_CAM_RESET, 0);
	gpio_set_value(GPIO_CAM_PWDN, 0);

	/*AVDD power 2.8V*/
	if (lge_bd_rev == LGE_REV_B) {
		rc = aat28xx_ldo_set_level(dev, LDO_CAM_AF_NO, 2800);
		if (rc < 0) {
			printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_AF_NO);
			goto power_on_fail;
		}
		rc = aat28xx_ldo_enable(dev, LDO_CAM_AF_NO, 1);
		if (rc < 0) {
			printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_AF_NO);
			goto power_on_fail;
		}
	} else {	/* it is for rev.c and default */
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 2800);
		vreg_enable(vreg_mmc);
	}

  /* DVDD power 1.2V */
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_DVDD_NO, 1200);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_DVDD_NO);
		goto power_on_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_DVDD_NO, 1);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_DVDD_NO);
		goto power_on_fail;
	}

  /*IOVDD power 2.6V*/
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_IOVDD_NO, 2600);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_IOVDD_NO);
		goto power_on_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_IOVDD_NO, 1);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_IOVDD_NO);
		goto power_on_fail;
	}

	/*AVDD power  2.7V*/
	/* LGE_CHANGE 
	  * Change AVDD level from 2.7V to 2.8V in order to reduce camera noise in dard environment.
	  * 2010-08-03. minjong.gong@lge.com
	  */
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_AVDD_NO, 2800);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_AVDD_NO);
		goto power_on_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_AVDD_NO, 1);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_AVDD_NO);
		goto power_on_fail;
	}	

	mdelay(5);
	/*M Clock -24Mhz*/
	msm_camio_clk_rate_set(mclk_rate);
	mdelay(5);
	msm_camio_camif_pad_reg_reset();
	mdelay(5);

	/*reset high*/
	gpio_set_value(GPIO_CAM_RESET, 1);

	mdelay(5); 
	/*Nstandby high*/
	gpio_set_value(GPIO_CAM_PWDN, 1);
	
	mdelay(8);  // T2 

	camera_power_state = CAM_POWER_ON;

power_on_fail:
	camera_power_mutex_unlock();
	return rc;

}

int camera_power_off (void)
{
	int rc;
	struct device *dev = thunderg_backlight_dev();

	camera_power_mutex_lock();

	if (lcd_bl_power_state == BL_POWER_SUSPEND) {
		thunderg_pwrsink_resume();
		mdelay(50);
	}
	/*Nstandby low*/
	gpio_set_value(GPIO_CAM_PWDN, 0);
	mdelay(5);

	/*reset low*/
	gpio_set_value(GPIO_CAM_RESET, 0);

	/*AVDD power 2.8V*/
	if (lge_bd_rev == LGE_REV_A) {
		rc = aat28xx_ldo_set_level(dev, LDO_CAM_AF_NO, 0);
		if (rc < 0) {
			printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_AF_NO);
			goto power_off_fail;
		}
		rc = aat28xx_ldo_enable(dev, LDO_CAM_AF_NO, 0);
		if (rc < 0) {
			printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_AF_NO);
			goto power_off_fail;
		}
	} else {	/* it is for rev.c and default */
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 0);
		vreg_disable(vreg_mmc);
	}


	/*AVDD power 2.7V*/
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_AVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_AVDD_NO);
		goto power_off_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_AVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_AVDD_NO);
		goto power_off_fail;
	}

	/*IOVDD power 2.6V*/
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_IOVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_IOVDD_NO);
		goto power_off_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_IOVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_IOVDD_NO);
		goto power_off_fail;
	}

  /* DVDD power 1.2V*/
	rc = aat28xx_ldo_set_level(dev, LDO_CAM_DVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d set level error\n", __func__, LDO_CAM_DVDD_NO);
		goto power_off_fail;
	}
	rc = aat28xx_ldo_enable(dev, LDO_CAM_DVDD_NO, 0);
	if (rc < 0) {
		printk(KERN_ERR "%s: ldo %d control error\n", __func__, LDO_CAM_DVDD_NO);
		goto power_off_fail;
	}
	camera_power_state = CAM_POWER_OFF;


power_off_fail:
	camera_power_mutex_unlock();
	return rc;

}

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
	.camera_power_on = camera_power_on,
	.camera_power_off = camera_power_off,
};

#if defined (CONFIG_ISX005)
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_isx005_data = {
	.sensor_name    = "isx005",
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = GPIO_CAM_PWDN,
	.vcm_pwd        = 0,
	.vcm_enable		= 0,
	.pdata          = &msm_camera_device_data,
	.flash_data		= &flash_none,
};

static struct platform_device msm_camera_sensor_isx005 = {
	.name      = "msm_camera_isx005",
	.dev       = {
		.platform_data = &msm_camera_sensor_isx005_data,
	},
};
#endif

#endif

static struct platform_device *thunderg_camera_devices[] __initdata = {
#if defined (CONFIG_ISX005)
	&msm_camera_sensor_isx005,
#endif
};

void __init lge_add_camera_devices(void)
{
	platform_add_devices(thunderg_camera_devices, ARRAY_SIZE(thunderg_camera_devices));
}
