/* drivers/video/msm/src/panel/mddi/mddi_hitachi_hvga.c
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

#if 0
static uint32 mddi_hitachi_curr_vpos;
static boolean mddi_hitachi_monitor_refresh_value = FALSE;
static boolean mddi_hitachi_report_refresh_measurements = FALSE;
#endif
static boolean is_lcd_on = -1;

/* The comment from AMSS codes:
 * Dot clock (10MHz) / pixels per row (320) = rows_per_second
 * Rows Per second, this number arrived upon empirically 
 * after observing the timing of Vsync pulses
 * XXX: TODO: change this values for INNOTEK PANEL */
static uint32 mddi_hitachi_rows_per_second = 31250;
static uint32 mddi_hitachi_rows_per_refresh = 480;
extern boolean mddi_vsync_detect_enabled;

static msm_fb_vsync_handler_type mddi_hitachi_vsync_handler = NULL;
static void *mddi_hitachi_vsync_handler_arg;
static uint16 mddi_hitachi_vsync_attempts;

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)
/* Define new structure named 'msm_panel_hitachi_pdata' to use LCD initialization Flag (initialized)
 * 2010-04-21, minjong.gong@lge.com
 */
static struct msm_panel_hitachi_pdata *mddi_hitachi_pdata;
#else
static struct msm_panel_common_pdata *mddi_hitachi_pdata;
#endif

static int mddi_hitachi_lcd_on(struct platform_device *pdev);
static int mddi_hitachi_lcd_off(struct platform_device *pdev);

static int mddi_hitachi_lcd_init(void);
static void mddi_hitachi_lcd_panel_poweron(void);
static void mddi_hitachi_lcd_panel_poweroff(void);
static void mddi_hitachi_lcd_panel_store_poweron(void);

#define DEBUG 1
#if DEBUG
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define EPRINTK(fmt, args...) do { } while (0)
#endif

struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[20];
};

struct display_table2 {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[16384];
};

#define REGFLAG_DELAY             0XFFFE
#define REGFLAG_END_OF_TABLE      0xFFFF   // END OF REGISTERS MARKER

static struct display_table mddi_hitachi_2c[] = {
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_hitachi_position_table[] = {
	// set column address 
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},
	// set page address 
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_hitachi_display_on_1st[] = {
	// Display on sequence
	{0x11, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 80, {}},
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x29, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_hitachi_display_on_3rd[] = {
	// Display on sequence
	{0x11, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 80, {}},
	{0x29, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct display_table2 mddi_hitachi_img[] = {
	{0x2c, 16384, {}},
};
static struct display_table mddi_hitachi_img_end[] = {
	{0x00, 0, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#if 0
static struct display_table mddi_hitachi_display_off[] = {
	// Display off sequence
	{0x28, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 40, {}},
	{0x10, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 130, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

static struct display_table mddi_hitachi_sleep_mode_on_data[] = {
	// Display off sequence
	{0x28, 4, {0x00, 0x00, 0x00, 0x00}},
	// [App 4th Table] Change from 40ms to 20ms. 2010-08-03. minjong.gong@lge.com
	{REGFLAG_DELAY, 20, {}},
	{0x10, 4, {0x00, 0x00, 0x00, 0x00}},
	// [Apply 4th Table] Change from 100ms to 40ms. 2010-08-03. minjong.gong@lge.com
	{REGFLAG_DELAY, 40, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_hitachi_initialize_1st[] = {

	// Power ON Sequence 
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xd0, 4, {0x06, 0x00, 0x00, 0x00}},

	// PWRCTL 
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x04, 0x66, 0x02, 0x04, 0x66, 0x02, 0x00, 0x00}},

	// VCMCTL 
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x01, 0x01, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	// MANPWRSEQ 
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	// change 3rd parameter from 0x1d to 0x15 to reduce flicker.
	
	// DISCTL 
	{0xf2, 20, {0x3b, 0x54, 0x0f, 0x18, 0x18, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3f, 0x18,
			    0x18, 0x18, 0x18, 0x00}},
	// 0x3b - 480 line
	// 0x3b - 60hz

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	// PGAMMACTL 
	//{0xfa, 16, {0x11, 0x13, 0x08, 0x14, 0x28, 0x2c, 0x2b, 0x0d,
	//		    0x19, 0x14, 0x1e, 0x1e, 0x0f, 0x00, 0x00, 0x00}},
	// Apply 3rd Cut gamma table. 2010-08-10. minjong.gong@lge.com
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
				0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	// NGAMMACTL 
	//{0xfb, 16, {0x11, 0x13, 0x08, 0x14, 0x28, 0x2c, 0x2b, 0x2d,
	//			0x19, 0x14, 0x1e, 0x1e, 0x0f, 0x00, 0x00, 0x00}},
	// Apply 3rd Cut gamma table. 2010-08-10. minjong.gong@lge.com
	{0xfb, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
				0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	// MADCTL 
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	// TEON 
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	// COLMOD 
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	// set column address 
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	// set page address 
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{0x2c,  4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#ifdef CONFIG_MACH_MSM7X27_THUNDERC
static struct display_table mddi_hitachi_initialize_3rd_vs660[] = {

	// Power ON Sequence 
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},

	// PWRCTL 
	// [VS660] DCN set value : 0x3F. For reducing LCD noise.
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x3f, 0x66, 0x02, 0x3f, 0x66, 0x02, 0x00, 0x00}},

	// VCMCTL 
	// Revert 6th parameter. From 0x04 to 0x00. 2010-09-02. minjong.gong@lge.com
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	// MANPWRSEQ 
	// Revert 1st parameter. From 0x03 to 0x01. 2010-09-02. minjong.gong@lge.com
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	
	// DISCTL 
	// Revert 2nd and 15th parameters. From 0x54 to 0x4d.
	// Revert 6th, 7th, 9th and 10th parameters. From 0x08 to ox00.
	// 2010-09-02. minjong.gong@lge.com
	{0xf2, 20, {0x3b, 0x4d, 0x0f, 0x08, 0x08, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x4d, 0x08,
			    0x08, 0x08, 0x08, 0x00}},

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	// PGAMMACTL 
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
			    0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	// MADCTL 
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	// TEON 
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	// COLMOD 
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	// set column address 
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	// set page address 
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

#ifdef CONFIG_MACH_MSM7X27_THUNDERG
static struct display_table mddi_hitachi_initialize_3rd_p500[] = {

	// Power ON Sequence 
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},

	// PWRCTL 
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x04, 0x66, 0x02, 0x04, 0x66, 0x02, 0x00, 0x00}},

	// VCMCTL 
	// Revert 6th parameter. From 0x04 to 0x00. 2010-09-02. minjong.gong@lge.com
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	// MANPWRSEQ 
	// Revert 1st parameter. From 0x03 to 0x01. 2010-09-02. minjong.gong@lge.com
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	
	// DISCTL 
	// Revert 2nd and 15th parameters. From 0x54 to 0x4d.
	// Revert 6th, 7th, 9th and 10th parameters. From 0x08 to ox00.
	// 2010-09-02. minjong.gong@lge.com
	{0xf2, 20, {0x3b, 0x4d, 0x0f, 0x08, 0x08, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x4d, 0x08,
			    0x08, 0x08, 0x08, 0x00}},

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	// PGAMMACTL 
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
			    0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	// MADCTL 
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	// TEON 
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	// COLMOD 
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	// set column address 
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	// set page address 
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
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
                mddi_host_register_cmds_write8(reg, table[i].count, table[i].val_list, 1, 0, 0);
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

static void mddi_hitachi_vsync_set_handler(msm_fb_vsync_handler_type handler,	/* ISR to be executed */
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

	if (mddi_hitachi_vsync_handler != NULL) {
		error = TRUE;
	} else {
		/* Register the handler for this particular GROUP interrupt source */
		mddi_hitachi_vsync_handler = handler;
		mddi_hitachi_vsync_handler_arg = arg;
	}
	
	/* Restore interrupts */
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	/* MDDI_INTFREE(); */
	if (error) {
		printk("MDDI: Previous Vsync handler never called\n");
	} else {
		/* Enable the vsync wakeup */
		/* mddi_queue_register_write(INTMSK, 0x0000, FALSE, 0); */
		mddi_hitachi_vsync_attempts = 1;
		mddi_vsync_detect_enabled = TRUE;
	}
}

/* FIXME: following function has no meaning any more
 * should be eliminated
 * 2010-11-16, cleaneye.kim@lge.com
 */
static void mddi_hitachi_lcd_vsync_detected(boolean detected)
{
#if 0
	/* static timetick_type start_time = 0; */
	static struct timeval start_time;
	static boolean first_time = TRUE;
	/* unit32 mdp_cnt_val = 0; */
	/* timetick_type elapsed_us; */
	struct timeval now;
	uint32 elapsed_us;
	uint32 num_vsyncs;
#endif

/* LGE_CHANGE
  * Close below code to fix screen shaking problem
  * 2010-04-22, minjong.gong@lge.com
  */
//	mddi_queue_register_write_int(0x2C, 0);

#if 0 /* Block temporaly till vsync implement */
	if ((detected) || (mddi_hitachi_vsync_attempts > 5)) {
		if ((detected) || (mddi_hitachi_monitor_refresh_value)) {
			/* if (start_time != 0) */
			if (!first_time) {
				jiffies_to_timeval(jiffies, &now);
				elapsed_us =
					(now.tv_sec - start_time.tv_sec) * 1000000 +
					now.tv_usec - start_time.tv_usec;
				/*
				 * LCD is configured for a refresh every usecs,
				 *  so to determine the number of vsyncs that
				 *  have occurred since the last measurement
				 *  add half that to the time difference and
				 *  divide by the refresh rate.
				 */
				num_vsyncs = (elapsed_us +
						(mddi_hitachi_rows_per_refresh >>
						 1))/
					mddi_hitachi_rows_per_refresh;
				/*
				 * LCD is configured for * hsyncs (rows) per
				 * refresh cycle. Calculate new rows_per_second
				 * value based upon these new measuerments.
				 * MDP can update with this new value.
				 */
				mddi_hitachi_rows_per_second =
					(mddi_hitachi_rows_per_refresh * 1000 *
					 num_vsyncs) / (elapsed_us / 1000);
			}
			/* start_time = timetick_get();*/
			first_time = FALSE;
			jiffies_to_timeval(jiffies, &start_time);
			if (mddi_hitachi_report_refresh_measurements) {
				(void)mddi_queue_register_read_int(VPOS,
									&mddi_hitachi_curr_vpos);
				/* mdp_cnt_val = MDP_LINE_COUNT; */
			}
		}
		/* if detected = TRUE, client initiated wakeup was detected */
		if (mddi_hitachi_vsync_handler != NULL) {
			(*mddi_hitachi_vsync_handler)
				(mddi_hitachi_vsync_handler_arg);
			mddi_hitachi_vsync_handler = NULL;
		}
		mddi_vsync_detect_enabled = FALSE;
		mddi_hitachi_vsync_attempts = 0;
		/* need to disable the interrupt wakeup */
		if (!mddi_queue_register_write_int(INTMSK, 0x0001))
			printk("Vsync interrupt disable failed!\n");
		if (!detected) {
			/* give up after 5 failed attempts but show error */
			printk("Vsync detection failed!\n");
		} else if ((mddi_hitachi_monitor_refresh_value) &&
				(mddi_hitachi_report_refresh_measurements)) {
			printk("  Last Line Counter=%d!\n",
					mddi_hitachi_curr_vpos);
			/* MDDI_MSG_NOTICE("  MDP Line Counter=%d!\n",mdp_cnt_val); */
			printk("  Lines Per Second=%d!\n",
					mddi_hitachi_rows_per_second);
		}
		/* clear the interrupt */
		if (!mddi_queue_register_write_int(INTFLG, 0x0001))
			printk("Vsync interrupt clear failed!\n");
	} else {
		/* if detected = FALSE, we woke up from hibernation, but did not
		 * detect client initiated wakeup.
		 */
		mddi_hitachi_vsync_attempts++;
	}
#endif
}

static void hitachi_workaround(void)
{
	if (lge_bd_rev <= LGE_REV_E) {
		/* Use workaround code for 1st cut LCD.
		 * 2010-04-22, minjong.gong@lge.com
		 */
		display_table(mddi_hitachi_2c,
					  sizeof(mddi_hitachi_2c) / sizeof(struct display_table));
	}
	/* Add code to prevent LCD shift.
	 * 2010-05-18, minjong.gong@lge.com
	 */
	display_table(mddi_hitachi_position_table,
				  sizeof(mddi_hitachi_position_table) / sizeof(struct display_table));
}

static int mddi_hitachi_lcd_on(struct platform_device *pdev)
{
	EPRINTK("%s: started.\n", __func__);

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)
	if (system_state == SYSTEM_BOOTING && mddi_hitachi_pdata->initialized) {
		hitachi_workaround();
		is_lcd_on = TRUE;
		return 0;
	}
#endif
	// LCD HW Reset
	mddi_hitachi_lcd_panel_poweron();
#if defined(CONFIG_MACH_MSM7X27_THUNDERG)
	if (lge_bd_rev <= LGE_REV_E) {
		EPRINTK("ThunderG ==> lge_bd_rev = %d : 1st LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		EPRINTK("ThunderG ==> lge_bd_rev = %d : 3rd LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#elif defined(CONFIG_MACH_MSM7X27_THUNDERA)
	display_table(mddi_hitachi_initialize_1st, 
			sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
	display_table(mddi_hitachi_display_on_1st,
			sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
#else
	if (lge_bd_rev <= LGE_REV_D) {
		EPRINTK("ThunderC ==> lge_bd_rev = %d : 1st LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		EPRINTK("ThunderC ==> lge_bd_rev = %d : 3rd LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_3rd_vs660, sizeof(mddi_hitachi_initialize_3rd_vs660)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#endif
	is_lcd_on = TRUE;
	return 0;
}

static int mddi_hitachi_lcd_store_on(void)
{
	EPRINTK("%s: started.\n", __func__);

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)
	if (system_state == SYSTEM_BOOTING && mddi_hitachi_pdata->initialized) {
		is_lcd_on = TRUE;
		return 0;
	}
#endif
	// LCD HW Reset
	mddi_hitachi_lcd_panel_store_poweron();
#if defined(CONFIG_MACH_MSM7X27_THUNDERG)
	if (lge_bd_rev <= LGE_REV_E) {
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#elif defined(CONFIG_MACH_MSM7X27_THUNDERA)
	display_table(mddi_hitachi_initialize_1st,
			sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
	mdelay(200);
	display_table(mddi_hitachi_display_on_1st,
			sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
#else
	if (lge_bd_rev <= LGE_REV_D) {
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		display_table(mddi_hitachi_initialize_3rd_vs660, sizeof(mddi_hitachi_initialize_3rd_vs660)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#endif
	is_lcd_on = TRUE;
	return 0;
}

static int mddi_hitachi_lcd_off(struct platform_device *pdev)
{
	display_table(mddi_hitachi_sleep_mode_on_data, sizeof(mddi_hitachi_sleep_mode_on_data)/sizeof(struct display_table));
	mddi_hitachi_lcd_panel_poweroff();
	is_lcd_on = FALSE;
	return 0;
}

ssize_t mddi_hitachi_lcd_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	EPRINTK("%s : strat\n", __func__);
	return 0;
}

ssize_t mddi_hitachi_lcd_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device dummy_pdev;
	int onoff; // = simple_strtol(buf, NULL, count);
	sscanf(buf, "%d", &onoff);

	EPRINTK("%s: onoff : %d\n", __func__, onoff);
	
	if(onoff) {
//		display_table(mddi_hitachi_display_on, sizeof(mddi_hitachi_display_on) / sizeof(struct display_table));
		mddi_hitachi_lcd_store_on();
		is_lcd_on = TRUE;
	}
	else {
//		display_table(mddi_hitachi_display_off, sizeof(mddi_hitachi_display_off) / sizeof(struct display_table));
		mddi_hitachi_lcd_off(&dummy_pdev);
		is_lcd_on = FALSE;
	}

	return 0;
}

int mddi_hitachi_position(void)
{
	display_table(mddi_hitachi_position_table, ARRAY_SIZE(mddi_hitachi_position_table));
	return 0;
}
EXPORT_SYMBOL(mddi_hitachi_position);

DEVICE_ATTR(lcd_onoff, 0666, mddi_hitachi_lcd_show_onoff, mddi_hitachi_lcd_store_onoff);

struct msm_fb_panel_data hitachi_panel_data0 = {
	.on = mddi_hitachi_lcd_on,
	.off = mddi_hitachi_lcd_off,
	.set_backlight = NULL,
	.set_vsync_notifier = mddi_hitachi_vsync_set_handler,
};

static struct platform_device this_device_0 = {
	.name   = "mddi_hitachi_hvga",
	.id	= MDDI_LCD_HITACHI_TX08D39VM,
	.dev	= {
		.platform_data = &hitachi_panel_data0,
	}
};

static int __init mddi_hitachi_lcd_probe(struct platform_device *pdev)
{
	int ret;
	EPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		mddi_hitachi_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver this_driver __refdata = {
	.probe  = mddi_hitachi_lcd_probe,
	.driver = {
		.name   = "mddi_hitachi_hvga",
	},
};

static int mddi_hitachi_lcd_init(void)
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
		pinfo = &hitachi_panel_data0.panel_info;
		EPRINTK("%s: setting up panel info.\n", __func__);
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = 0x23;//MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 16;
	
		// vsync config
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = (mddi_hitachi_rows_per_second * 100) /
                        		mddi_hitachi_rows_per_refresh;

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

		pinfo->clk_rate = 10000000;
		pinfo->clk_min =  9000000;
		pinfo->clk_max =  11000000;
		pinfo->fb_num = 2;

		ret = platform_device_register(&this_device_0);
		if (ret) {
			EPRINTK("%s: this_device_0 register success\n", __func__);
			platform_driver_unregister(&this_driver);
		}
	}

	if(!ret) {
		mddi_lcd.vsync_detected = mddi_hitachi_lcd_vsync_detected;
	}

	return ret;
}

extern unsigned fb_width;
extern unsigned fb_height;

static void mddi_hitachi_lcd_panel_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;
#else
	struct msm_panel_common_pdata *pdata = mddi_hitachi_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
	//	gpio_set_value(pdata->gpio, 1);
	//	mdelay(10);
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
		gpio_set_value(pdata->gpio, 1);
		mdelay(2);
	}
}

static void mddi_hitachi_lcd_panel_store_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;
#else
	struct msm_panel_common_pdata *pdata = mddi_hitachi_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
	//	gpio_set_value(pdata->gpio, 1);
	//	mdelay(10);
		gpio_set_value(pdata->gpio, 0);
		mdelay(50);
		gpio_set_value(pdata->gpio, 1);
		mdelay(50);
	}
}

/* LGE_CHANGE
  * Add new function to reduce current comsumption in sleep mode.
  * In sleep mode disable LCD by assertion low on reset pin.
  * 2010-06-07, minjong.gong@lge.com
  */
static void mddi_hitachi_lcd_panel_poweroff(void)
{
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 0);
		mdelay(5);
	}
}
module_init(mddi_hitachi_lcd_init);
