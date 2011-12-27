/* drivers/video/msm/src/panel/mddi/mddi_ldp_hvga.c
 *
 * Copyright (C) 2008 QUALCOMM Incorporated.
 * Copyright (c) 2008 QUALCOMM USA, INC.
 * 
 * All source code in this file is licensed under the following license
 * except where indicated.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org
 */

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define PANEL_DEBUG 0

#define LCD_CONTROL_BLOCK_BASE	0x110000
#define INTFLG		LCD_CONTROL_BLOCK_BASE|(0x18)
#define INTMSK		LCD_CONTROL_BLOCK_BASE|(0x1c)
#define VPOS		LCD_CONTROL_BLOCK_BASE|(0xc0)

//static uint32 mddi_ldp_curr_vpos;
//static boolean mddi_ldp_monitor_refresh_value = FALSE;
//static boolean mddi_ldp_report_refresh_measurements = FALSE;
static boolean is_lcd_on = -1;

/* The comment from AMSS codes:
 * Dot clock (10MHz) / pixels per row (320) = rows_per_second
 * Rows Per second, this number arrived upon empirically 
 * after observing the timing of Vsync pulses
 * XXX: TODO: change this values for INNOTEK PANEL */
static uint32 mddi_ldp_rows_per_second = 31250;
static uint32 mddi_ldp_rows_per_refresh = 480;
//static uint32 mddi_ldp_usecs_per_refresh = 15360; /* rows_per_refresh / rows_per_second */
extern boolean mddi_vsync_detect_enabled;

static msm_fb_vsync_handler_type mddi_ldp_vsync_handler = NULL;
static void *mddi_ldp_vsync_handler_arg;
static uint16 mddi_ldp_vsync_attempts;

#if defined(CONFIG_MACH_MSM7X27_GELATO) || defined(CONFIG_MACH_MSM7X27_UNIVA)
/* Define new structure named 'msm_panel_ldp_pdata' to use LCD initialization Flag (initialized)
 * 2010-04-21, minjong.gong@lge.com
 */
static struct msm_panel_ldp_pdata *mddi_ldp_pdata;
#else
static struct msm_panel_common_pdata *mddi_ldp_pdata;
#endif

static int mddi_ldp_lcd_on(struct platform_device *pdev);
static int mddi_ldp_lcd_off(struct platform_device *pdev);

static int mddi_ldp_lcd_init(void);
static void mddi_ldp_lcd_panel_poweron(void);
static void mddi_ldp_lcd_panel_poweroff(void);
static void mddi_ldp_lcd_panel_store_poweron(void);

#define DEBUG 1
#if DEBUG
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define EPRINTK(fmt, args...) do { } while (0)
#endif

#define REGISTER_DATA_LIST_32BIT
#ifdef REGISTER_DATA_LIST_32BIT
struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned val_list[256];
};
#else
struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[24];
};
#endif

struct display_table2 {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[16384];
};

#define REGFLAG_DELAY             0XFFFE
#define REGFLAG_END_OF_TABLE      0xFFFF   // END OF REGISTERS MARKER

static struct display_table mddi_ldp_position_table[] = {
	// set column address 
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},
	// set page address 
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_ldp_display_on_1st[] = {
    // Sleep Out
	{0x11, 4, {0x00, 0x00, 0x00, 0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 4, {0x00, 0x00, 0x00, 0x00}},
    {REGFLAG_DELAY, 40, {0}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct display_table mddi_ldp_display_off[] = {
	// Display off sequence
	{0x28, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 40, {}},

    // Sleep Mode In
	{0x10, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 120, {}},
        
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct display_table mddi_ldp_sleep_mode_on_data[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 40, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},

	// MCAP
	{0xb0, 1, {0x00}},

	// Low Power Mode Control
	{0xb1, 1, {0x01}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#ifdef REGISTER_DATA_LIST_32BIT
// V07_110626
static struct display_table mddi_ldp_initialize_1st[] = {
	// MCAP
	{0xb0, 1, {0x04}},

	// Set Tear On
	{0x35, 1, {0x00}},

	// Set Tear Scanline
	{0x44, 2, {0x00, 0x00}},

	// Set Address Mode
	{0x36, 1, {0x08}},

	// Set Pixel Format
	{0x3a, 1, {0x55}},    //0x55 : 16bit, 0x66 : 18bit, 0x77 : 24bit

	// Set Column Address
	{0x2a, 4, {0x00, 0x00, 0x01, 0x3f}},

	// Set Page Address
	{0x2b, 4, {0x00, 0x00, 0x01, 0xdf}},

	// Frame Memory Access and Interface Setting
	{0xb3, 4, {0x02, 0x00, 0x00, 0x00}},

	// Panel Driving Setting
	{0xc0, 8, {0x01, 0xdf, 0x40, 0x10, 0x00, 0x01, 0x00, 0x33}},
	
	// Display Timing Setting for Normal Mode
	{0xc1, 5, {0x07, 0x2d, 0x04, 0x04, 0x10}},

	// Source/Gate Driving Timing Setting
	{0xc4, 4, {0x77, 0x00, 0x03, 0x01}},
	
	// DPI Polarity Control
	{0xc6, 1, {0x00}},
	
	// Gamma Setting A Set
	{0xc8, 24,{0x00, 0x10, 0x18, 0x25, 0x31, 0x49, 0x3c, 0x2b,
		0x1a, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x25,
		0x31, 0x49, 0x3c, 0x2b, 0x1a, 0x0e, 0x06, 0x00}},
	
	// Gamma Setting B Set
	{0xc9, 24,{0x00, 0x10, 0x18, 0x25, 0x31, 0x49, 0x3c, 0x2b,
		0x1a, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x25,
		0x31, 0x49, 0x3c, 0x2b, 0x1a, 0x0e, 0x06, 0x00}},
	
	// Gamma Setting C Set
	{0xca, 24, {0x00, 0x10, 0x18, 0x25, 0x31, 0x49, 0x3c, 0x2b,
		0x1a, 0x0e, 0x06, 0x00, 0x00, 0x10, 0x18, 0x25,
		0x31, 0x49, 0x3c, 0x2b, 0x1a, 0x0e, 0x06, 0x00}},
	
	// Power Setting (Charge Pump Setting)
	{0xd0, 16, {0xa9, 0x06, 0x08, 0x20, 0x31, 0x04, 0x01, 0x00,
                0x08, 0x01, 0x00, 0x06, 0x01, 0x00, 0x00, 0x20}},

	// VCOM Setting
	{0xd1, 4, {0x02, 0x22, 0x22, 0x33}},

	// Backlight Control(1)
	{0xb8, 20, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00}},
	
	// Backlight Control(2)
	{0xb9, 4, {0x00, 0x00, 0x00, 0x00}},
	
	// Backlight Control(3)
	{0xba, 2, {0x00, 0x00}},
	
	// NVM Access Control
	{0xe0, 4, {0x00, 0x00, 0x00, 0x00}},

	// Set DDB Write Control
	{0xe1, 6, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

	// NVM Load Control
	{0xe2, 1, {0x80}},

	// Write Memory Start
	{0x2c, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {0}}
}; 
#else
static struct display_table mddi_ldp_initialize_1st[] = {
	// MCAP
	{0xb0, 4, {0x04, 0x00, 0x00, 0x00}},

	// Set Tear On
	{0x35, 4, {0x00, 0x00, 0x00, 0x00}},

	// Set Address Mode
	{0x36, 4, {0x08, 0x00, 0x00, 0x00}},

	// Set Pixel Format
	{0x3a, 4, {0x55, 0x00, 0x00, 0x00}},    //0x55 : 16bit, 0x66 : 18bit, 0x77 : 24bit

	// Set Column Address
	{0x2a, 4, {0x00, 0x00, 0x01, 0x3f}},

	// Set Page Address
	{0x2b, 4, {0x00, 0x00, 0x01, 0xdf}},

	// Frame Memory Access and Interface Setting
	{0xb3, 4, {0x02, 0x00, 0x00, 0x00}},

	// Panel Driving Setting
	//{0xc0, 8, {0x01, 0xdf, 0x40, 0x13, 0x00, 0x01, 0x00, 0x33}},
	{0xc0, 8, {0x01, 0xdf, 0x40, 0x13, 0xf0, 0x01, 0x00, 0x33}},
	
	// Display Timing Setting for Normal Mode
	//{0xc1, 8, {0x07, 0x27, 0x08, 0x08, 0x50, 0x00, 0x00, 0x00}},

	// Source/Gate Driving Timing Setting
	{0xc4, 4, {0x77, 0x00, 0x03, 0x01}},
	
	// DPI Polarity Control
	{0xc6, 4, {0x00, 0x00, 0x00, 0x00}},
	
	// Gamma Setting A Set
	{0xc8, 24, {0x00, 0x07, 0x1f, 0x23, 0x30, 0x48, 0x37, 0x25,
		0x1c, 0x16, 0x10, 0x00, 0x00, 0x07, 0x1f, 0x23,
		0x30, 0x48, 0x37, 0x25, 0x1c, 0x16, 0x10, 0x00}},
	
	// Gamma Setting B Set
	{0xc9, 24, {0x00, 0x07, 0x1f, 0x23, 0x30, 0x48, 0x37, 0x25,
		0x1c, 0x16, 0x10, 0x00, 0x00, 0x07, 0x1f, 0x23,
		0x30, 0x48, 0x37, 0x25, 0x1c, 0x16, 0x10, 0x00}},
	
	// Gamma Setting C Set
	{0xca, 24, {0x00, 0x07, 0x1f, 0x23, 0x30, 0x48, 0x37, 0x25,
		0x1c, 0x16, 0x10, 0x00, 0x00, 0x07, 0x1f, 0x23,
		0x30, 0x48, 0x37, 0x25, 0x1c, 0x16, 0x10, 0x00}},
	
	// Power Setting (Charge Pump Setting)
	{0xd0, 16, {0x95, 0x0e, 0x08, 0x20, 0x31, 0x04, 0x01, 0x00,
		0x08, 0x01, 0x00, 0x06, 0x01, 0x00, 0x00, 0x20}},

	// VCOM Setting
	{0xd1, 4, {0x02, 0x1f, 0x1f, 0x38}},
	
	// Backlight Control(1)
	{0xb8, 20, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00}},
	
	// Backlight Control(2)
	{0xb9, 4, {0x00, 0x00, 0x00, 0x00}},

	// Backlight Control(3)
	{0xba, 4, {0x00, 0x00, 0x00, 0x00}},
	
	// NVM Access Control
	{0xe0, 4, {0x00, 0x00, 0x00, 0x00}},

	// Set DDB Write Control
	{0xe1, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00}},

	// NVM Load Control
	{0xe2, 4, {0x80, 0x00, 0x00, 0x00}},

	// Write Memory Start
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {0}}
}; 
#endif

void display_table(struct display_table *table, unsigned int count)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned reg;
        reg = table[i].reg;
		
        switch (reg) {
			
            case REGFLAG_DELAY :
                msleep(table[i].count);
				EPRINTK("%s() : delay %d msec\n", __func__, table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
#ifdef REGISTER_DATA_LIST_32BIT
                mddi_host_register_cmds_write32(reg, table[i].count, table[i].val_list, 1, 0, 0);
#else				
                mddi_host_register_cmds_write8(reg, table[i].count, table[i].val_list, 1, 0, 0);
#endif
				//EPRINTK("%s: reg : %x, val : %x.\n", __func__, reg, table[i].val_list[0]);
       	}
    }
	
}

#if 0
static void compare_table(struct display_table *table, unsigned int count)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned reg;
        reg = table[i].reg;
		
        switch (reg) {
			
            case REGFLAG_DELAY :              
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
                mddi_host_register_cmds_write8(reg, table[i].count, table[i].val_list, 0, 0, 0);
//				if(table[i].val_list != temp)
					
				EPRINTK("%s: reg : %x, val : %x.\n", __func__, reg, table[i].val_list[0]);
       	}
    }	
}
#endif

static void mddi_ldp_vsync_set_handler(msm_fb_vsync_handler_type handler,	/* ISR to be executed */
					 void *arg)
{
	boolean error = FALSE;
	unsigned long flags;

	/* LGE_CHANGE [neo.kang@lge.com] 2009-11-26, change debugging api */
	printk("%s : handler = %x\n", 
			__func__, (unsigned int)handler);

	/* Disable interrupts */
	spin_lock_irqsave(&mddi_host_spin_lock, flags);
	/* INTLOCK(); */

	if (mddi_ldp_vsync_handler != NULL) {
		error = TRUE;
	} else {
		/* Register the handler for this particular GROUP interrupt source */
		mddi_ldp_vsync_handler = handler;
		mddi_ldp_vsync_handler_arg = arg;
	}
	
	/* Restore interrupts */
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	/* MDDI_INTFREE(); */
	if (error) {
		printk("MDDI: Previous Vsync handler never called\n");
	} else {
		/* Enable the vsync wakeup */
		/* mddi_queue_register_write(INTMSK, 0x0000, FALSE, 0); */
		mddi_ldp_vsync_attempts = 1;
		mddi_vsync_detect_enabled = TRUE;
	}
}

static void mddi_ldp_lcd_vsync_detected(boolean detected)
{
	/* static timetick_type start_time = 0; */
//	static struct timeval start_time;
//	static boolean first_time = TRUE;
	/* unit32 mdp_cnt_val = 0; */
	/* timetick_type elapsed_us; */
//	struct timeval now;
//	uint32 elapsed_us;
//	uint32 num_vsyncs;

/* LGE_CHANGE
  * Close below code to fix screen shaking problem
  * 2010-04-22, minjong.gong@lge.com
  */
//	mddi_queue_register_write_int(0x2C, 0);
}

static int mddi_ldp_lcd_on(struct platform_device *pdev)
{
	EPRINTK("%s: started.\n", __func__);

	if (system_state == SYSTEM_BOOTING && mddi_ldp_pdata->initialized) {
		is_lcd_on = TRUE;
		return 0;
	}

	mddi_host_client_cnt_reset();

	// LCD HW Reset
	mddi_ldp_lcd_panel_poweron();
	display_table(mddi_ldp_initialize_1st, sizeof(mddi_ldp_initialize_1st)/sizeof(struct display_table));
	display_table(mddi_ldp_display_on_1st, sizeof(mddi_ldp_display_on_1st) / sizeof(struct display_table));

	is_lcd_on = TRUE;
	return 0;
}

static int mddi_ldp_lcd_store_on(void)
{
	EPRINTK("%s: started.\n", __func__);

	if (system_state == SYSTEM_BOOTING && mddi_ldp_pdata->initialized) {
		is_lcd_on = TRUE;
		return 0;
	}

	// LCD HW Reset
	mddi_ldp_lcd_panel_store_poweron();
	display_table(mddi_ldp_initialize_1st, sizeof(mddi_ldp_initialize_1st)/sizeof(struct display_table));
	display_table(mddi_ldp_display_on_1st, sizeof(mddi_ldp_display_on_1st) / sizeof(struct display_table));

	is_lcd_on = TRUE;
	return 0;
}

static int mddi_ldp_lcd_off(struct platform_device *pdev)
{
	display_table(mddi_ldp_sleep_mode_on_data, sizeof(mddi_ldp_sleep_mode_on_data)/sizeof(struct display_table));
	mddi_ldp_lcd_panel_poweroff();
	is_lcd_on = FALSE;
	return 0;
}

ssize_t mddi_ldp_lcd_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	EPRINTK("%s : strat\n", __func__);
	return 0;
}

ssize_t mddi_ldp_lcd_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device dummy_pdev;
	int onoff; // = simple_strtol(buf, NULL, count);
	sscanf(buf, "%d", &onoff);

	EPRINTK("%s: onoff : %d\n", __func__, onoff);
	
	if(onoff) {
//		display_table(mddi_ldp_display_on, sizeof(mddi_ldp_display_on) / sizeof(struct display_table));
		mddi_ldp_lcd_store_on();
		is_lcd_on = TRUE;
	}
	else {
//		display_table(mddi_ldp_display_off, sizeof(mddi_ldp_display_off) / sizeof(struct display_table));
		mddi_ldp_lcd_off(&dummy_pdev);
		is_lcd_on = FALSE;
	}

	//return 0;
	return count;
}

int mddi_ldp_position(void)
{
	display_table(mddi_ldp_position_table, ARRAY_SIZE(mddi_ldp_position_table));
	return 0;
}
EXPORT_SYMBOL(mddi_ldp_position);

DEVICE_ATTR(lcd_onoff, 0664, mddi_ldp_lcd_show_onoff, mddi_ldp_lcd_store_onoff);

struct msm_fb_panel_data ldp_panel_data0 = {
	.on = mddi_ldp_lcd_on,
	.off = mddi_ldp_lcd_off,
	.set_backlight = NULL,
	.set_vsync_notifier = mddi_ldp_vsync_set_handler,
};

static struct platform_device this_device_0 = {
	.name   = "mddi_ldp_hvga",
	.id	= MDDI_LCD_LDP_TX08D39VM,
	.dev	= {
		.platform_data = &ldp_panel_data0,
	}
};

static int __init mddi_ldp_lcd_probe(struct platform_device *pdev)
{
	int ret;
	EPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		mddi_ldp_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver __refdata this_driver = {
	.probe  = mddi_ldp_lcd_probe,
	.driver = {
		.name   = "mddi_ldp_hvga",
	},
};

static int mddi_ldp_lcd_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	u32 id;
	id = mddi_get_client_id();

	/* TODO: Check client id */

#endif
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &ldp_panel_data0.panel_info;
		EPRINTK("%s: setting up panel info.\n", __func__);
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 16;
	
		// vsync config
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = (mddi_ldp_rows_per_second * 100) /
                        		mddi_ldp_rows_per_refresh;

/* LGE_CHANGE.
  * Change proch values to resolve LCD Tearing. Before BP:14, FP:6. After BP=FP=6.
  * The set values on LCD are both 8, but we use 6 for MDDI in order to secure timing margin.
  * 2010-08-21, minjong.gong@lge.com
  */
		pinfo->lcd.v_back_porch = 6;
		pinfo->lcd.v_front_porch = 6;
		pinfo->lcd.v_pulse_width = 4;

		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = (1 * HZ);

		pinfo->bl_max = 4;
		pinfo->bl_min = 1;

#ifdef REGISTER_DATA_LIST_32BIT
		pinfo->clk_rate = 122880000;
		pinfo->clk_min =   120000000;
		pinfo->clk_max =   130000000;
#else	
		pinfo->clk_rate = 10000000;
		pinfo->clk_min =  9000000;
		pinfo->clk_max =  11000000;
#endif		
		pinfo->fb_num = 2;

		ret = platform_device_register(&this_device_0);
		if (ret) {
			EPRINTK("%s: this_device_0 register success\n", __func__);
			platform_driver_unregister(&this_driver);
		}
	}

	if(!ret) {
		mddi_lcd.vsync_detected = mddi_ldp_lcd_vsync_detected;
	}

	return ret;
}

extern unsigned fb_width;
extern unsigned fb_height;

static void mddi_ldp_lcd_panel_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_GELATO) || defined(CONFIG_MACH_MSM7X27_UNIVA)
	struct msm_panel_ldp_pdata *pdata = mddi_ldp_pdata;
#else
	struct msm_panel_common_pdata *pdata = mddi_ldp_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 1);
		mdelay(10);
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
		gpio_set_value(pdata->gpio, 1);
		mdelay(10);
	}
}

static void mddi_ldp_lcd_panel_store_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA) \
 || defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	struct msm_panel_ldp_pdata *pdata = mddi_ldp_pdata;
#else
	struct msm_panel_ldp_pdata *pdata = mddi_ldp_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 1);
		mdelay(10);
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
		gpio_set_value(pdata->gpio, 1);
		mdelay(10);
	}
}

/* LGE_CHANGE
  * Add new function to reduce current comsumption in sleep mode.
  * In sleep mode disable LCD by assertion low on reset pin.
  * 2010-06-07, minjong.gong@lge.com
  */
static void mddi_ldp_lcd_panel_poweroff(void)
{
	struct msm_panel_ldp_pdata *pdata = mddi_ldp_pdata;

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
	}
}
module_init(mddi_ldp_lcd_init);
