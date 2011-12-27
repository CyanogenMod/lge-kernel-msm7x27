/* arch/arm/mach-msm/board-univa-camera.c
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
#include "board-univa.h"
#if defined(UNIVA_EVB)
#include <../drivers/video/backlight/lp8720.h>
#endif

#if defined(UNIVA_EVB)
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
extern void lp8720_write_reg(u8 reg, u8 data);
#endif

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
#if defined (CONFIG_MT9T113)
	{
		I2C_BOARD_INFO("mt9t113", CAM_I2C_SLAVE_ADDR),
	},
#elif defined (CONFIG_S5K5CAGA)
	{
		I2C_BOARD_INFO("s5k5caga", CAM_I2C_SLAVE_ADDR),
	},
//LGE_DEV_PORTING UNIVA_S : Camera (MT9P111)
#elif defined (CONFIG_MT9P111)
	{
		I2C_BOARD_INFO("mt9p111", CAM_I2C_SLAVE_ADDR),
	},
//LGE_DEV_PORTING UNIVA_E
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
//	int rc;

	camera_power_mutex_lock();
	//if(lcd_bl_power_state == BL_POWER_SUSPEND)
	//{
	//	u370_pwrsink_resume();
	//	mdelay(50);
	//}
#if defined(CONFIG_MT9P111)
#if defined(UNIVA_EVB)
/*AF power 2.8V*/
	{
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 2800);
		vreg_enable(vreg_mmc);
	}

	/* LDO5 : DVDD power 1.8V */
	/* LDO4 : IOVDD power 2.8V*/
	/* LDO2 : AVDD power  2.8V*/	
	lp8720_write_reg(LP8720_LDO4_SETTING, LP8720_STARTUP_DELAY_3TS | 0x1E); // 2.8v - CAM_IOVDD_2.8V
	lp8720_write_reg(LP8720_LDO2_SETTING, LP8720_STARTUP_DELAY_3TS | 0x19); // 2.8v - CAM_AVDD_2.8V
	lp8720_write_reg(LP8720_LDO5_SETTING, LP8720_STARTUP_DELAY_3TS | 0x0C); // 1.8v - CAM_DVDD_1.8V
	mdelay(5);
	gpio_direction_output(LP8720_ENABLE, 1);

	subpm_set_output(LDO4,1);
	subpm_set_output(LDO2,1);
	subpm_set_output(LDO5,1);
	subpm_output_enable();
	mdelay(5); 	
#else
	// CAM_AF_2.8V
	{
		gpio_tlmm_config(GPIO_CFG(26, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(26, 1);
	}

	// CAM_DVDD_1.8V
	{
		struct vreg *vreg_rftx = vreg_get(0, "rftx");
		vreg_set_level(vreg_rftx, 1800);
		vreg_enable(vreg_rftx);
	}
	
	// CAM_IOVDD_2.8V
	{
		struct vreg *vreg_rfrx2 = vreg_get(0, "rfrx2");
		vreg_set_level(vreg_rfrx2, 2800);
		vreg_enable(vreg_rfrx2);
	}

	// CAM_AVDD_2.8V
	{
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 2800);
		vreg_enable(vreg_mmc);
	}
#endif	
#endif

	camera_power_state = CAM_POWER_ON;

//power_on_fail:
	camera_power_mutex_unlock();
//	return rc;
	return 0;
}

int camera_power_off (void)
{
//	int rc;

	camera_power_mutex_lock();

#if defined(CONFIG_MT9P111)
#if defined(UNIVA_EVB)
	{ /* it is for rev.c and default */
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 0);
		vreg_disable(vreg_mmc);
	}

	/* LDO5 : DVDD power 1.8V */
	/* LDO4 : IOVDD power 2.8V*/
	/* LDO2 : AVDD power  2.8V*/
	subpm_set_output(LDO5,0);
	subpm_set_output(LDO4,0);
	subpm_set_output(LDO2,0);
	subpm_output_enable();
	gpio_direction_output(LP8720_ENABLE, 0);
#else
	// CAM_AF_2.8V
	{
		gpio_tlmm_config(GPIO_CFG(26, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_set_value(26, 0);
	}

	// CAM_AVDD_2.8V
	{
		struct vreg *vreg_mmc = vreg_get(0, "mmc");
		vreg_set_level(vreg_mmc, 0);
		vreg_disable(vreg_mmc);
	}

	// CAM_DVDD_1.8V
	{
		struct vreg *vreg_rftx = vreg_get(0, "rftx");
		vreg_set_level(vreg_rftx, 0);
		vreg_disable(vreg_rftx);
	}
	
	// CAM_IOVDD_2.8V
	{
		struct vreg *vreg_rfrx2 = vreg_get(0, "rfrx2");
		vreg_set_level(vreg_rfrx2, 0);
		vreg_disable(vreg_rfrx2);
	}
#endif	
#endif

	camera_power_state = CAM_POWER_OFF;

//power_off_fail:
	camera_power_mutex_unlock();
//	return rc;
	return 0;
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

#if defined (CONFIG_MT9T113) 
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9t113_data = {
	.sensor_name    = "mt9t113",
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = GPIO_CAM_PWDN,
	.vcm_pwd        = 0,
	.vcm_enable		= 0,
	.pdata          = &msm_camera_device_data,
	.flash_data		= &flash_none,
};

static struct platform_device msm_camera_sensor_mt9t113 = {
	.name      = "msm_camera_mt9t113",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9t113_data,
	},
};
#elif defined (CONFIG_S5K5CAGA)
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_s5k5caga_data = {
	.sensor_name    = "s5k5caga",
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = GPIO_CAM_PWDN,
	.vcm_pwd        = 0,
	.vcm_enable		= 0,
	.pdata          = &msm_camera_device_data,
	.flash_data		= &flash_none,
};

static struct platform_device msm_camera_sensor_s5k5caga = {
	.name      = "msm_camera_s5k5caga",
	.dev       = {
		.platform_data = &msm_camera_sensor_s5k5caga_data,
	},
};
#elif defined (CONFIG_MT9P111)
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_mt9p111_data = {
	.sensor_name    = "mt9p111",
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = GPIO_CAM_PWDN,
	.vcm_pwd        = 0,
	.vcm_enable		= 0,
	.pdata          = &msm_camera_device_data,
	.flash_data		= &flash_none,
};

static struct platform_device msm_camera_sensor_mt9p111 = {
	.name      = "msm_camera_mt9p111",
	.dev       = {
		.platform_data = &msm_camera_sensor_mt9p111_data,
	},
};
#endif

#endif

static struct platform_device *univa_camera_devices[] __initdata = {
#if defined (CONFIG_MT9T113)
	&msm_camera_sensor_mt9t113,
#elif defined (CONFIG_S5K5CAGA)
	&msm_camera_sensor_s5k5caga,
#elif defined (CONFIG_MT9P111)
	&msm_camera_sensor_mt9p111,
#endif
};

void __init lge_add_camera_devices(void)
{
	platform_add_devices(univa_camera_devices, ARRAY_SIZE(univa_camera_devices));
}
