/* arch/arm/mach-msm/include/mach/board_lge.h
 * Copyright (C) 2010 LGE Corporation.
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
#ifndef __ASM_ARCH_MSM_BOARD_LGE_H
#define __ASM_ARCH_MSM_BOARD_LGE_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/rfkill.h>
#include <linux/platform_device.h>
#include <asm/setup.h>

#if __GNUC__
#define __WEAK __attribute__((weak))
#endif

#ifdef CONFIG_ANDROID_RAM_CONSOLE
/* allocate 128K * 2 instead of ram_console's original size 128K
 * this is for storing kernel panic log which is used by lk loader
 * 2010-03-03, cleaneye.kim@lge.com
 */
#define MSM7X27_EBI1_CS0_BASE	PHYS_OFFSET
#define LGE_RAM_CONSOLE_SIZE    (128 * SZ_1K * 2)
#endif

/* define PMEM address size */
#define MSM_PMEM_MDP_SIZE      0x1C91000
#define MSM_PMEM_ADSP_SIZE     0xAE4000
#define MSM_PMEM_AUDIO_SIZE    0x121000
#define MSM_FB_SIZE            0x177000
#define MSM_GPU_PHYS_SIZE      SZ_2M
#define PMEM_KERNEL_EBI1_SIZE  0x64000

/* TA charger */
#define GISELE_TA_CHG_CURRENT	600
#define GISELE_USB_CHG_CURRENT	400

/* board revision information */
enum {
	EVB         = 0,
	LGE_REV_A,
	LGE_REV_B,
	LGE_REV_C,
	LGE_REV_D,
	LGE_REV_E,
	LGE_REV_10,
	LGE_REV_TOT_NUM,
};

extern int lge_bd_rev;

/* define gpio pin number of i2c-gpio */
struct gpio_i2c_pin {
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int reset_pin;
	unsigned int irq_pin;
};

/* touch screen platform data */
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int (*power)(unsigned char onoff);
	int irq;
	int scl;
	int sda;
};

/* pp2106 qwerty platform data */
struct pp2106_platform_data {
	unsigned int reset_pin;
	unsigned int irq_pin;
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int keypad_row;
	unsigned int keypad_col;
	unsigned char *keycode;
};

/* atcmd virtual keyboard platform data */
struct atcmd_virtual_platform_data {
	unsigned int keypad_row;
	unsigned int keypad_col;
	unsigned char *keycode;
};

/* bu52031 hall ic platform data */
struct bu52031_platform_data {
	unsigned int irq_pin;
	unsigned int prohibit_time;
};

/* gpio switch platform data */
struct lge_gpio_switch_platform_data {
	const char *name;
	unsigned *gpios;
	size_t num_gpios;
	unsigned long irqflags;
	unsigned int wakeup_flag;
	int (*work_func)(void);
	char *(*print_state)(int state);
	int (*sysfs_store)(const char *buf, size_t size);
	int (*additional_init)(void);
};

/* proximity platform data */
struct proximity_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
	int methods;
	int operation_mode;
	int debounce;
	u8 cycle;
};

/* acceleration platform data */
struct acceleration_platform_data {
	int irq_num;
	int (*power)(unsigned char onoff);
};

/* kr3dh acceleration platform data */
struct kr3dh_platform_data {
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*kr_init)(void);
	void (*kr_exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);
	int (*gpio_config)(int config);
};

/* ecompass platform data */
struct ecom_platform_data {
	int pin_int;
	int pin_rst;
	int (*power)(unsigned char onoff);
	char accelerator_name[20];
	int fdata_sign_x;
        int fdata_sign_y;
        int fdata_sign_z;
	int fdata_order0;
	int fdata_order1;
	int fdata_order2;
	int sensitivity1g;
	s16 *h_layout;
	s16 *a_layout;
};

/* gyro(ami602) platform data */
struct gyro_platform_data {
        int pin_rst;
        int pin_busy;
        int pin_trg;
        int (*power)(unsigned char onoff);
};

/* msm pmic leds platform data */
struct msm_pmic_leds_pdata {
	struct led_classdev *custom_leds;
	int (*register_custom_leds)(struct platform_device *pdev);
	void (*unregister_custom_leds)(void);
	void (*suspend_custom_leds)(void);
	void (*resume_custom_leds)(void);
	int (*msm_keypad_led_set)(unsigned char value);
};

/* aat1270 flash platform data */
struct aat1270_flash_platform_data {
	int gpio_flen;
	int gpio_en_set;
	int gpio_inh;
};

/* android vibrator platform data */
struct android_vibrator_platform_data {
	int enable_status;
	int (*power_set)(int enable); 		/* LDO Power Set Function */
	int (*pwn_set)(int enable, int gain); 		/* PWM Set Function */
	int (*ic_enable_set)(int enable); 	/* Motor IC Set Function */
};

/* bd6084gu backight, pmic */
#define BACKLIGHT_NORMAL_MODE  0
#define BACKLIGHT_ALC_MODE     1
struct backlight_platform_data {
	void (*platform_init)(void);
	int gpio;
	unsigned int mode;		     /* initial mode */
	int max_current;			 /* led max current(0-7F) */
	int init_on_boot;			 /* flag which initialize on system boot */
};
int bd6084gu_ldo_enable(struct device *dev, unsigned num, unsigned enable);
int bd6084gu_ldo_set_level(struct device *dev, unsigned num, unsigned vol);

struct aat28xx_platform_data {
	void (*platform_init)(void);
	int gpio;
	unsigned int mode;		     /* initial mode */
	int max_current;			 /* led max current(0-7F) */
	int init_on_boot;			 /* flag which initialize on system boot */
	int version;				 /* Chip version number */
};
int aat28xx_ldo_enable(struct device *dev, unsigned num, unsigned enable);
int aat28xx_ldo_set_level(struct device *dev, unsigned num, unsigned vol);

/* rt9393 backlight */
struct rt9393_platform_data {
	int gpio_en;
};

/* LCD panel */
struct msm_panel_lgit_pdata {
	int gpio;
	int (*backlight_level)(int level, int max, int min);
	int (*pmic_backlight)(int level);
	int (*panel_num)(void);
	void (*panel_config_gpio)(int);
	int *gpio_num;
	int initialized;
};

/* Define new structure named 'msm_panel_hitachi_pdata' */
struct msm_panel_hitachi_pdata {
	int gpio;
	int (*backlight_level)(int level, int max, int min);
	int (*pmic_backlight)(int level);
	int (*panel_num)(void);
	void (*panel_config_gpio)(int);
	int *gpio_num;
	int initialized;
};

/* tsc2007 platform data */
struct tsc2007_platform_data {
	u16	model;				/* 2007. */
	u16	x_plate_ohms;

	int	(*get_pendown_state)(void);
	void	(*clear_penirq)(void);		/* If needed, clear 2nd level
						   interrupt source */
	int	(*init_platform_hw)(void);
	void	(*exit_platform_hw)(void);
};

struct bluetooth_platform_data {
	int (*bluetooth_power)(int on);
	int (*bluetooth_toggle_radio)(void *data, enum rfkill_state state);
};

struct bluesleep_platform_data {
	int bluetooth_port_num;
};

struct gpio_h2w_platform_data {
	int gpio_detect;
	int gpio_button_detect;
};

enum {
	REBOOT_KEY_PRESS = 0,
	REBOOT_KEY_NOT_PRESS,
};

struct lge_panic_handler_platform_data {
	int (*reboot_key_detect)(void);
};

struct ram_console_buffer *get_ram_console_buffer(void);
void lge_set_reboot_reason(unsigned int reason);
int lge_gpio_switch_pass_event(char *sdev_name, int state);
unsigned lge_get_pif_info(void);
unsigned lge_get_lpm_info(void);

typedef void (gpio_i2c_init_func_t)(int bus_num);
int __init init_gpio_i2c_pin(struct i2c_gpio_platform_data *i2c_adap_pdata,
		struct gpio_i2c_pin gpio_i2c_pin,
		struct i2c_board_info *i2c_board_info_data);

void __init msm_msm7x2x_allocate_memory_regions(void);
void __init msm_add_fb_device(void);
void __init msm_add_pmem_devices(void);
void __init msm_add_kgsl_device(void);
void __init msm_add_usb_devices(void);
void __init msm_device_i2c_init(void);

void __init lge_add_ramconsole_devices(void);
void __init lge_add_panic_handler_devices(void);
void __init lge_add_ers_devices(void);

void __init lge_add_camera_devices(void);
void __init lge_add_input_devices(void);
void __init lge_add_lcd_devices(void);
void __init lge_add_btpower_devices(void);
void __init lge_add_mmc_devices(void);
void __init lge_add_misc_devices(void);
void __init lge_add_gpio_i2c_device(gpio_i2c_init_func_t *init_func);
void __init lge_add_gpio_i2c_devices(void);
int __init lge_get_uart_mode(void);
#endif
