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
#ifdef CONFIG_ARCH_MSM7X25
#define MSM_PMEM_MDP_SIZE   0xb21000
#define MSM_PMEM_ADSP_SIZE  0x97b000
#define MSM_PMEM_AUDIO_SIZE 0x121000
#define MSM_FB_SIZE     0x200000
#define PMEM_KERNEL_EBI1_SIZE   0x64000
#endif

#ifdef CONFIG_ARCH_MSM7X27
#define MSM_PMEM_MDP_SIZE	0x1B76000
#define MSM_PMEM_ADSP_SIZE	0xB71000
#define MSM_PMEM_AUDIO_SIZE	0x5B000
#define MSM_FB_SIZE		0x177000
#define PMEM_KERNEL_EBI1_SIZE	0x1C000

/* Using lower 1MB of OEMSBL memory for GPU_PHYS */
#define MSM_GPU_PHYS_START_ADDR	 0xD600000ul
#endif

/* Using upper 1/2MB of Apps Bootloader memory*/
#define MSM_PMEM_AUDIO_START_ADDR	0x80000ul

/* TA charger */
#define GISELE_TA_CHG_CURRENT	600
#define GISELE_USB_CHG_CURRENT	400

#ifdef CONFIG_MACH_MSM7X27_ALESSI
/* Camera LDO Enable */
#define LDO_CAM_DVDD_NO                                1               /* 1.2V */
#define LDO_CAM_IOVDD_NO                       2               /* 2.7V */
#define LDO_CAM_AF_NO                          3               /* 2.8V */
#define LDO_CAM_AVDD_NO                                4               /* 2.7V */
#endif

#ifdef CONFIG_MACH_MSM7X27_MUSCAT
/* I dont know why I allocate bigger than real lcd size in muscat , because EBI2 interface? */
#define HIDDEN_RESET_FB_SIZE 165600
#else
#define HIDDEN_RESET_FB_SIZE (320*480*2)
#endif
/* board revision information */
enum {
	EVB         = 0,
	LGE_REV_A,
	LGE_REV_B,
	LGE_REV_C,
	LGE_REV_D,
	LGE_REV_E,
	LGE_REV_F,
	LGE_REV_10,
	LGE_REV_11,
	LGE_REV_12,
	LGE_REV_13,
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
//LGE_DEV_PORTING UNIVA_S
// [LGE PATCH : START] edward1.kim@lge.com 20110214  
#if 1
struct touch_platform_data {
	int ts_x_min;
	int ts_x_max;
	int ts_y_min;
	int ts_y_max;
	int ts_y_start;
	int ts_y_scrn_max;
	int (*power)(unsigned char onoff);
	int irq;
	int gpio_int;// [LGE PATCH] edward1.kim@lge.com 20110214  
	int hw_i2c;
	int scl;
	int sda;
	int ce;
	int touch_key;
};
#else
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
#endif
// [LGE PATCH : END] edward1.kim@lge.com 20110214
//LGE_DEV_PORTING UNIVA_E  
/* pp2106 qwerty platform data */
struct pp2106_platform_data {
	unsigned int reset_pin;
	unsigned int irq_pin;
	unsigned int sda_pin;
	unsigned int scl_pin;
	unsigned int keypad_row;
	unsigned int keypad_col;
	unsigned char *keycode;
	int (*power)(unsigned char onoff);
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

/* acceleration platform data */
/* k3dh */

struct k3dh_platform_data {
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
	int (*device_id) (void);
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
	int (*pwm_set)(int enable, int gain); 		/* PWM Set Function */
	int (*ic_enable_set)(int enable); 	/* Motor IC Set Function */
	int (*gpio_request)(void);	/* gpio request */
	int amp_value;				/* PWM tuning value */
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
	int initialized;			 /* flag which initialize on system boot */
	int version;				 /* Chip version number */
};
int aat28xx_ldo_enable(struct device *dev, unsigned num, unsigned enable);
int aat28xx_ldo_set_level(struct device *dev, unsigned num, unsigned vol);
void aat28xx_power(struct device *dev, int on);

//LGE_DEV_PORTING UNIVA_S
struct lm3530_platform_data {
	void (*platform_init)(void);
	int gpio;
	unsigned int mode;		     /* initial mode */
	int max_current;			 /* led max current(0-7F) */
	int init_on_boot;			 /* flag which initialize on system boot */
	int version;				 /* Chip version number */
};
//LGE_DEV_PORTING UNIVA_E

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
#define PANEL_ID_AUO          0
#define PANEL_ID_HITACHI      1
#define PANEL_ID_TOVIS        2
#define PANEL_ID_LGDISPLAY    3
struct msm_panel_hitachi_pdata {
	int gpio;
	int (*backlight_level)(int level, int max, int min);
	int (*pmic_backlight)(int level);
	int (*panel_num)(void);
	void (*panel_config_gpio)(int);
	int *gpio_num;
	int initialized;
	int maker_id;
};



// LGE_DEV_PORTING UNIVA_S [ks82.jung@lge.com]
/* Define new structure named 'msm_panel_ldp_pdata' */
#define PANEL_ID_AUO      0
#define PANEL_ID_LDP      1
struct msm_panel_ldp_pdata {
	int gpio;
	int (*backlight_level)(int level, int max, int min);
	int (*pmic_backlight)(int level);
	int (*panel_num)(void);
	void (*panel_config_gpio)(int);
	int *gpio_num;
	int initialized;
	int maker_id;
};
// LGE_DEV_PORTING UNIVA_E [ks82.jung@lge.com]

struct msm_panel_ilitek_pdata {
	int gpio;
	int initialized;
	int maker_id;
	int (*lcd_power_save)(int);
};

struct msm_panel_novatek_pdata {
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
	int (*bluetooth_toggle_radio)(void *data, bool blocked);
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
extern int hidden_reset_enable;
extern int on_hidden_reset;
void *lge_get_fb_addr(void);
void *lge_get_fb_copy_phys_addr(void);
void *lge_get_fb_copy_virt_addr(void);

struct lge_panic_handler_platform_data {
	int (*reboot_key_detect)(void);
};

struct ram_console_buffer *get_ram_console_buffer(void);
void lge_set_reboot_reason(unsigned int reason);
int lge_gpio_switch_pass_event(char *sdev_name, int state);
unsigned lge_get_pif_info(void);
unsigned lge_get_lpm_info(void);

unsigned lge_get_batt_volt(void);
unsigned lge_get_chg_therm(void);
unsigned lge_get_pcb_version(void);
unsigned lge_get_chg_curr_volt(void);
unsigned lge_get_batt_therm(void);
unsigned lge_get_batt_volt_raw(void);
#ifdef CONFIG_MACH_MSM7X27_UNIVA
unsigned lge_get_chg_stat_reg(void);
unsigned lge_get_chg_en_reg(void);
unsigned lge_set_elt_test(void);
unsigned lge_clear_elt_test(void);
#endif
unsigned lge_get_nv_qem(void);

#define CAMERA_POWER_ON				0
#define CAMERA_POWER_OFF			1

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
void __init lge_add_pm_devices(void);
void __init lge_add_tsif_devices(void);

#endif
