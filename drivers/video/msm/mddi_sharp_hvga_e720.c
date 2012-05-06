/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <asm/gpio.h>

static uint32 mddi_sharp_curr_vpos;
static boolean mddi_sharp_monitor_refresh_value = FALSE;
static boolean mddi_sharp_report_refresh_measurements = FALSE;

/* The comment from AMSS codes:
 * Dot clock (10MHz) / pixels per row (320) = rows_per_second
 * Rows Per second, this number arrived upon empirically 
 * after observing the timing of Vsync pulses
 * XXX: TODO: change this values for SHARP PANEL */
static uint32 mddi_sharp_rows_per_second = 31250;
static uint32 mddi_sharp_rows_per_refresh = 480;
static uint32 mddi_sharp_usecs_per_refresh = 15360; /* rows_per_refresh / rows_per_second */

static boolean is_lcd_on = -1;
extern boolean mddi_vsync_detect_enabled;


static msm_fb_vsync_handler_type mddi_sharp_vsync_handler = NULL;
static void *mddi_sharp_vsync_handler_arg;
static uint16 mddi_sharp_vsync_attempts;

static struct msm_panel_common_pdata *mddi_sharp_pdata;

static int mddi_sharp_lcd_on(struct platform_device *pdev);
static int mddi_sharp_lcd_off(struct platform_device *pdev);

static int mddi_sharp_lcd_init_e720(void);
static void mddi_sharp_lcd_panel_poweron(void);

#define DEBUG 0
#if DEBUG
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define EPRINTK(fmt, args...) do { } while (0)
#endif

struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned val_list[256];
};

#define REGFLAG_DELAY               0XFFFE
#define REGFLAG_END_OF_TABLE        0xFFFF   // END OF REGISTERS MARKER

#define GPIO_LCD_RESET_N        	102

#define GPIO_HIGH_VALUE 1
#define GPIO_LOW_VALUE  0

#define USE_TENTATIVE_COMMAND 1
static struct display_table mddi_sharp_display_on[] = {
    // Display on sequence
    {0x3900, 1, {0x0000}}, // Set Idle Mode on
#if defined(USE_TENTATIVE_COMMAND)
    {0x1100, 1, {0x0000}}, // sleep out
#endif
    {REGFLAG_DELAY, 100, {}},
    {0xF300, 1, {0x00AA}}, // unlock cmd2
    {0xF280, 1, {0x0002}}, // check cmd status
    {0x0280, 1, {0x0011}}, // power ctrl
    {0x0380, 1, {0x0000}}, // power ctrl
    {0x0480, 1, {0x0056}}, // set LTPS timing : 150 clks
    {0x0580, 1, {0x0056}}, // set LTPS timing : 150 clks
    {0x0680, 1, {0x0056}}, // set LTPS timing : 150 clks
    {0x0780, 1, {0x0000}}, // power ctrl 5->0
    {0x0880, 1, {0x0033}}, // power ctrl
    {0x0980, 1, {0x0043}}, // power ctrl
    {0x0A80, 1, {0x0030}}, // power ctrl
    {0x0B80, 1, {0x0044}}, // power ctrl
    {0x0C80, 1, {0x0054}}, // power ctrl
    {0x0D80, 1, {0x0030}}, // power ctrl
    {0x0E80, 1, {0x0033}}, // power ctrl
    {0x0F80, 1, {0x0043}}, // power ctrl
    {0x1080, 1, {0x0030}}, // power ctrl
    {0x1180, 1, {0x0000}}, // power ctrl
    {0x1280, 1, {0x000C}}, // VDDGR
    {0x1380, 1, {0x0004}}, // VG Control
    {0x1480, 1, {0x0058}}, // Set GVDD=5.0V
    {0x1680, 1, {0x0070}}, // Set VCOMDC1=2.1V
    {0x1780, 1, {0x00CC}}, // VCOM Control
    {0x1880, 1, {0x0080}}, // VCOM Control
    {0x1980, 1, {0x0000}}, // VCOM Mode=0x00 for DC VCOM Mode 1
    {0x1A80, 1, {0x0078}}, // VCOM Control
    {0x1B80, 1, {0x0050}}, // Set VCOMMH=3.5V
    {0x1C80, 1, {0x0080}}, // VCOM Control
    {0x9480, 1, {0x0017}}, // Set LTPS timing : 23 clks
    {0x9580, 1, {0x0021}}, // Set LTPS timing : 33 clks
    {0x9680, 1, {0x0005}}, // Set LTPS timing : 5 clks
    {0x9780, 1, {0x000C}}, // Set LTPS timing : 12 clks
    {0x9880, 1, {0x0072}}, // Set LTPS timing : 114 clks
    {0x9980, 1, {0x0012}}, // Set LTPS timing : 18 clks added
    {0x9A80, 1, {0x0088}}, // Set LTPS timing : 136 clks
    {0x9B80, 1, {0x0001}}, // Set LTPS timing : 1 clks
    {0x9C80, 1, {0x0005}}, // Set LTPS timing : 5 clks
    {0x9D80, 1, {0x0016}}, // Set LTPS timing : 22 clks
    {0x9E80, 1, {0x0000}}, // Set LTPS timing
    {0x9F80, 1, {0x0000}}, // Set LTPS timing
    {0xA380, 1, {0x00F8}}, // Set LTPS timing
    {0xA480, 1, {0x003F}}, // Set LTPS timing
    {0xA680, 1, {0x0008}}, // Set LTPS timing
    //{0x3600, 1, {0x0008}}, // Set RGB
    {0x2880, 1, {0x0009}}, // Set Gamma R
    {0x2980, 1, {0x001E}}, // Set Gamma R
    {0x2A80, 1, {0x0045}}, // Set Gamma R
    {0x2B80, 1, {0x0063}}, // Set Gamma R
    {0x2C80, 1, {0x000D}}, // Set Gamma R
    {0x2D80, 1, {0x002E}}, // Set Gamma R
    {0x2E80, 1, {0x0061}}, // Set Gamma R
    {0x2F80, 1, {0x0063}}, // Set Gamma R
    {0x3080, 1, {0x0020}}, // Set Gamma R
    {0x3180, 1, {0x0026}}, // Set Gamma R
    {0x3280, 1, {0x00A5}}, // Set Gamma R
    {0x3380, 1, {0x001E}}, // Set Gamma R
    {0x3480, 1, {0x0048}}, // Set Gamma R
    {0x3580, 1, {0x0067}}, // Set Gamma R
    {0x3680, 1, {0x0078}}, // Set Gamma R
    {0x3780, 1, {0x0088}}, // Set Gamma R
    {0x3880, 1, {0x0025}}, // Set Gamma R
    {0x3890, 1, {0x0053}}, // Set Gamma R
    {0x3A80, 1, {0x0009}}, // Set Gamma R
    {0x3B80, 1, {0x0037}}, // Set Gamma R
    {0x3C80, 1, {0x0056}}, // Set Gamma R
    {0x3D80, 1, {0x0068}}, // Set Gamma R
    {0x3E80, 1, {0x0018}}, // Set Gamma R
    {0x3F80, 1, {0x0037}}, // Set Gamma R
    {0x4080, 1, {0x0061}}, // Set Gamma R
    {0x4180, 1, {0x0040}}, // Set Gamma R
    {0x4280, 1, {0x0019}}, // Set Gamma R
    {0x4380, 1, {0x001F}}, // Set Gamma R
    {0x4480, 1, {0x0085}}, // Set Gamma R
    {0x4580, 1, {0x001E}}, // Set Gamma R
    {0x4680, 1, {0x0051}}, // Set Gamma R
    {0x4780, 1, {0x0072}}, // Set Gamma R
    {0x4880, 1, {0x0087}}, // Set Gamma R
    {0x4980, 1, {0x00A6}}, // Set Gamma R
    {0x4A80, 1, {0x004D}}, // Set Gamma R
    {0x4B80, 1, {0x0062}}, // Set Gamma R
    {0x4C80, 1, {0x003D}}, // Set Gamma G
    {0x4D80, 1, {0x0050}}, // Set Gamma G
    {0x4E80, 1, {0x006E}}, // Set Gamma G
    {0x4F80, 1, {0x007E}}, // Set Gamma G
    {0x5080, 1, {0x0009}}, // Set Gamma G
    {0x5180, 1, {0x0028}}, // Set Gamma G
    {0x5280, 1, {0x005C}}, // Set Gamma G
    {0x5380, 1, {0x006A}},
    {0x5480, 1, {0x001F}},
    {0x5580, 1, {0x0026}},
    {0x5680, 1, {0x00AA}},
    {0x5780, 1, {0x001D}},
    {0x5880, 1, {0x0048}},
    {0x5980, 1, {0x0065}}, //value changed from thunder
    {0x5A80, 1, {0x007A}},
    {0x5B80, 1, {0x008A}},
    {0x5C80, 1, {0x0026}},
    {0x5D80, 1, {0x0053}},
    {0x5E80, 1, {0x0009}},
    {0x5F80, 1, {0x0036}},
    {0x6080, 1, {0x0053}},
    {0x6180, 1, {0x0066}},
    {0x6280, 1, {0x001A}},
    {0x6380, 1, {0x0037}},
    {0x6480, 1, {0x0062}},
    {0x6580, 1, {0x003B}},
    {0x6680, 1, {0x0019}},
    {0x6780, 1, {0x0020}},
    {0x6880, 1, {0x007E}},
    {0x6980, 1, {0x0023}},
    {0x6A80, 1, {0x0057}},
    {0x6B80, 1, {0x0076}},
    {0x6C80, 1, {0x006C}},
    {0x6D80, 1, {0x007C}},
    {0x6E80, 1, {0x001A}},
    {0x6F80, 1, {0x002D}},
    {0x7080, 1, {0x0009}}, // Set Gamma B
    {0x7180, 1, {0x0023}},
    {0x7280, 1, {0x004F}},
    {0x7380, 1, {0x0069}},
    {0x7480, 1, {0x0015}},
    {0x7580, 1, {0x003E}},
    {0x7680, 1, {0x0069}},
    {0x7780, 1, {0x0074}},
    {0x7880, 1, {0x0020}},
    {0x7980, 1, {0x0026}},
    {0x7A80, 1, {0x00AD}},
    {0x7B80, 1, {0x001E}},
    {0x7C80, 1, {0x004E}},
    {0x7D80, 1, {0x0067}},
    {0x7E80, 1, {0x0079}},
    {0x7F80, 1, {0x0086}},
    {0x8080, 1, {0x0028}},
    {0x8180, 1, {0x0053}},
    {0x8280, 1, {0x0009}},
    {0x8380, 1, {0x0034}},
    {0x8480, 1, {0x0058}},
    {0x8580, 1, {0x0067}},
    {0x8680, 1, {0x0018}},
    {0x8780, 1, {0x0031}},
    {0x8880, 1, {0x0061}},
    {0x8980, 1, {0x0038}},
    {0x8A80, 1, {0x0019}},
    {0x8B80, 1, {0x001F}},
    {0x8C80, 1, {0x0074}},
    {0x8D80, 1, {0x0016}},
    {0x8E80, 1, {0x0041}},
    {0x8F80, 1, {0x006A}},
    {0x9080, 1, {0x0081}},
    {0x9180, 1, {0x009B}},
    {0x9280, 1, {0x0048}},
    {0x9380, 1, {0x0062}},
#if defined(USE_TENTATIVE_COMMAND)
    {0x2780, 1, {0x0033}},
#endif
    {0x1580, 1, {0x00AA}}, // Lock CMD2
    {0xF200, 1, {0x0001}}, // Cehck CMD status
#if defined(USE_TENTATIVE_COMMAND)
    //{0x1100, 1, {0x0000}}, // sleep out
    //{REGFLAG_DELAY, 100, {}},
#endif
    {0x3B00, 1, {0x0043}}, // RGB Setup
    {0x3B01, 1, {0x0004}},
    {0x3B02, 1, {0x0004}},
    {0x3B03, 1, {0x0008}},
    {0x3B04, 1, {0x0007}},
	/*  [james.jang] 2010-06-18, off LEDPWM(7Fh -> 00h) */
	//{0x5100, 1, {0x007F}}, // Output LEDPWM=50% Duty
    {0x5100, 1, {0x0000}}, // Output LEDPWM=0% Duty
    {0x5300, 1, {0x002C}}, // Output LEDPWM=50% Duty
    // set horizontal address 
    {0x2a00, 1, {0x0000}}, // XSA
    {0x2a01, 1, {0x0000}}, // XSA
    {0x2a02, 1, {0x0000}}, // XEA
    {0x2a03, 1, {0x013f}}, // XEA, 320-1
    // set vertical address 
    {0x2b00, 1, {0x0000}}, // YSA
    {0x2b01, 1, {0x0000}}, // YSA
    {0x2b02, 1, {0x0000}}, // YEA
    {0x2b03, 1, {0x01df}}, // YEA, 480-1
    {0x3600, 1, {0x0008}}, // Set RGB
    {0x3800, 1, {0x0000}}, // Set Idle Mode Off
    {0x3A00, 1, {0x0055}}, // Set RGB565
    {0x3500, 1, {0x0000}}, // TE On
    {0x2900, 1, {0x0000}}, // Display On
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_sharp_display_off[] = {
	{0x3900, 1, {0x0000}},
	{0x2800, 1, {0x0000}},
	{REGFLAG_DELAY, 20, {}},
	{0x1000, 4, {0x0000}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0, {}}
};

static struct display_table mddi_sharp_sleep_mode_on[] = {
};

static struct display_table mddi_sharp_sleep_mode_off[] = {
};

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
                mddi_host_register_cmds_write32(reg, table[i].count, table[i].val_list, 0, 0, 0);
       	}
    }
	
	
}


static void mddi_sharp_vsync_set_handler(msm_fb_vsync_handler_type handler,	/* ISR to be executed */
					 void *arg)
{
	boolean error = FALSE;
	unsigned long flags;

	/* Disable interrupts */
	spin_lock_irqsave(&mddi_host_spin_lock, flags);

	if (mddi_sharp_vsync_handler != NULL) {
		error = TRUE;
	} else {
		/* Register the handler for this particular GROUP interrupt source */
		mddi_sharp_vsync_handler = handler;
		mddi_sharp_vsync_handler_arg = arg;
	}

	/* Restore interrupts */
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	// MDDI_INTFREE();
	if (error) {
		MDDI_MSG_ERR("MDDI: Previous Vsync handler never called\n");
		printk("MDDI: Previous Vsync handler never called\n");
	} else {
		/* TODO: Enable the vsync wakeup */
		mddi_sharp_vsync_attempts = 1;
		mddi_vsync_detect_enabled = TRUE;
	}
}

static void mddi_sharp_lcd_vsync_detected(boolean detected)
{
	static struct timeval start_time;
	static boolean first_time = TRUE;
	struct timeval now;
	uint32 elapsed_us;
	uint32 num_vsyncs;

	mddi_vsync_detect_enabled = TRUE;;

	#if 0
	if ((detected) || (mddi_sharp_vsync_attempts > 5)) {
		if ((detected) && (mddi_sharp_monitor_refresh_value)) {
			if (!first_time) {
				jiffies_to_timeval(jiffies, &now);
				elapsed_us =
				    (now.tv_sec - start_time.tv_sec) * 1000000 +
				    now.tv_usec - start_time.tv_usec;
				/* LCD is configured for a refresh every * usecs, so to
				 * determine the number of vsyncs that have occurred
				 * since the last measurement add half that to the
				 * time difference and divide by the refresh rate. */
				num_vsyncs = (elapsed_us +
					      (mddi_sharp_usecs_per_refresh >>
					       1)) /
				    mddi_sharp_usecs_per_refresh;
				/* LCD is configured for * hsyncs (rows) per refresh cycle.
				 * Calculate new rows_per_second value based upon these
				 * new measurements. MDP can update with this new value. */
				mddi_sharp_rows_per_second =
				    (mddi_sharp_rows_per_refresh * 1000 *
				     num_vsyncs) / (elapsed_us / 1000);
			}
			first_time = FALSE;
			jiffies_to_timeval(jiffies, &start_time);
			if (mddi_sharp_report_refresh_measurements) {
			}
		}
		/* if detected = TRUE, client initiated wakeup was detected */
		if (mddi_sharp_vsync_handler != NULL) {
			(*mddi_sharp_vsync_handler)
			    (mddi_sharp_vsync_handler_arg);
			mddi_sharp_vsync_handler = NULL;
		}
		mddi_vsync_detect_enabled = FALSE;
		mddi_sharp_vsync_attempts = 0;
		/* need to disable the interrupt wakeup */
		/* TODO: how to? */
		if (!detected) {
			/* give up after 5 failed attempts but show error */
			MDDI_MSG_NOTICE("Vsync detection failed!\n");
			printk("Vsync detection failed!\n");
		} else if ((mddi_sharp_monitor_refresh_value) &&
			   (mddi_sharp_report_refresh_measurements)) {
			MDDI_MSG_NOTICE("  Last Line Counter=%d!\n",
					mddi_sharp_curr_vpos);
			printk("  Last Line Counter=%d!\n",
				mddi_sharp_curr_vpos);
			// MDDI_MSG_NOTICE("  MDP Line Counter=%d!\n",mdp_cnt_val);
			MDDI_MSG_NOTICE("  Lines Per Second=%d!\n",
					mddi_sharp_rows_per_second);
			printk("  Lines Per Second=%d!\n",
				mddi_sharp_rows_per_second);
		}
		/* clear the interrupt */
		/* XXX: we do not have to */
	} else {
		/* if detected = FALSE, we woke up from hibernation, but did not
		 * detect client initiated wakeup.
		 */
		mddi_sharp_vsync_attempts++;
	}
#endif
}


static int mddi_sharp_lcd_on(struct platform_device *pdev)
{
	if(is_lcd_on == -1) {
		is_lcd_on = TRUE;
		return 0;
	}
	EPRINTK("%s: started.\n", __func__);

	if(is_lcd_on == FALSE)
	{
		mddi_sharp_lcd_panel_poweron();
		display_table(mddi_sharp_display_on,
					sizeof(mddi_sharp_display_on) / sizeof(struct display_table));
		is_lcd_on = TRUE;	
	}	
	return 0;
}

static int mddi_sharp_lcd_off(struct platform_device *pdev)
{
	if(is_lcd_on != FALSE)
	{
		display_table(mddi_sharp_display_off,
						sizeof(mddi_sharp_display_off) / sizeof(struct display_table));

		gpio_set_value(GPIO_LCD_RESET_N, 0);

		gpio_tlmm_config(GPIO_CFG(GPIO_LCD_RESET_N, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		gpio_direction_output(GPIO_LCD_RESET_N, 0);

		is_lcd_on = FALSE;	
	}
	return 0;
}

ssize_t mddi_sharp_lcd_show_onoff(struct platform_device *pdev)
{
	EPRINTK("%s : strat\n", __func__);
	return 0;
}

ssize_t mddi_sharp_lcd_store_onoff(struct platform_device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
	int onoff;
	sscanf(buf, "%d", &onoff);

	EPRINTK("%s: buf %s, onoff : %s\n", __func__, onoff);
	
	if(onoff) {
		is_lcd_on = TRUE;
		display_table(mddi_sharp_display_on, sizeof(mddi_sharp_display_on) / sizeof(struct display_table));
	}
	else {
		is_lcd_on = FALSE;
		display_table(mddi_sharp_display_off, sizeof(mddi_sharp_display_off) / sizeof(struct display_table));
	}

	return 0;
}
DEVICE_ATTR(lcd_onoff, 0666, mddi_sharp_lcd_show_onoff, mddi_sharp_lcd_store_onoff);

struct msm_fb_panel_data sharp_panel_data0 = {
	.on = mddi_sharp_lcd_on,
	.off = mddi_sharp_lcd_off,
	.set_backlight = NULL,
	.set_vsync_notifier = mddi_sharp_vsync_set_handler,
};

static struct platform_device this_device_0 = {
	.name   = "mddi_sharp_hvga",
	.id	= MDDI_LCD_SHARP_NT35451,
	.dev	= {
		.platform_data = &sharp_panel_data0,
	}
};

static int __init mddi_sharp_lcd_probe(struct platform_device *pdev)
{
	int ret;
	EPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		mddi_sharp_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);
	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mddi_sharp_lcd_probe,
	.driver = {
		.name   = "mddi_sharp_hvga",
	},
};

static int mddi_sharp_lcd_init_e720(void)
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
		pinfo = &sharp_panel_data0.panel_info;
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
		pinfo->lcd.refx100 = 
                        (mddi_sharp_rows_per_second * 100) /
                        mddi_sharp_rows_per_refresh;
		pinfo->lcd.v_back_porch = 200;
		pinfo->lcd.v_front_porch = 200;
		pinfo->lcd.v_pulse_width = 30;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = (1 * HZ);

		pinfo->bl_max = 4;
		pinfo->bl_min = 1;
		pinfo->clk_rate = 122880000;
		pinfo->clk_min =   120000000;
		pinfo->clk_max =   130000000;
		pinfo->fb_num = 2;

		ret = platform_device_register(&this_device_0);
		if (ret)
			platform_driver_unregister(&this_driver);
	}

	if(!ret) {
		mddi_lcd.vsync_detected = mddi_sharp_lcd_vsync_detected;
	}

	return ret;
}

extern unsigned fb_width;
extern unsigned fb_height;

static void mddi_sharp_lcd_panel_poweron(void)
{
	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;


	gpio_set_value(GPIO_LCD_RESET_N, 1);
	mdelay(10);
	gpio_set_value(GPIO_LCD_RESET_N, 0);
	mdelay(10);
	gpio_set_value(GPIO_LCD_RESET_N, 1);
	mdelay(20);
}
module_init(mddi_sharp_lcd_init_e720);
