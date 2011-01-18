/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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
 *
 */

#include "msm_fb.h"

#include <linux/memory.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include "linux/proc_fs.h"

#include <linux/delay.h>

#include <mach/hardware.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define LCD_SLEEP_ENABLE

#define MSM_FB_LCDC_VREG_OP(name, op, level)			\
do { \
	vreg = vreg_get(0, name); \
	vreg_set_level(vreg, level); \
	if (vreg_##op(vreg)) \
		printk(KERN_ERR "%s: %s vreg operation failed \n", \
			(vreg_##op == vreg_enable) ? "vreg_enable" \
				: "vreg_disable", name); \
} while (0)

static char *msm_fb_vreg[] = {
	"gp1",
	"gp2",
};

// #define TOVIS_LCD_18BPP 
#define QVGA_WIDTH        240
#define QVGA_HEIGHT       320

static void *DISP_CMD_PORT;
static void *DISP_DATA_PORT;

#define EBI2_WRITE16C(x, y) outpw(x, y) 
#define EBI2_WRITE16D(x, y) outpw(x, y) 
#define EBI2_READ16(x) inpw(x) 

static boolean disp_initialized = FALSE;
/* For some reason the contrast set at init time is not good. Need to do
* it again
*/
static boolean display_on = TRUE; // FALSE;

#ifdef DISP_DEVICE_8BPP
static word convert_8_to_16_tbl[256] = {
	0x0000, 0x2000, 0x4000, 0x6000, 0x8000, 0xA000, 0xC000, 0xE000,
	0x0100, 0x2100, 0x4100, 0x6100, 0x8100, 0xA100, 0xC100, 0xE100,
	0x0200, 0x2200, 0x4200, 0x6200, 0x8200, 0xA200, 0xC200, 0xE200,
	0x0300, 0x2300, 0x4300, 0x6300, 0x8300, 0xA300, 0xC300, 0xE300,
	0x0400, 0x2400, 0x4400, 0x6400, 0x8400, 0xA400, 0xC400, 0xE400,
	0x0500, 0x2500, 0x4500, 0x6500, 0x8500, 0xA500, 0xC500, 0xE500,
	0x0600, 0x2600, 0x4600, 0x6600, 0x8600, 0xA600, 0xC600, 0xE600,
	0x0700, 0x2700, 0x4700, 0x6700, 0x8700, 0xA700, 0xC700, 0xE700,
	0x0008, 0x2008, 0x4008, 0x6008, 0x8008, 0xA008, 0xC008, 0xE008,
	0x0108, 0x2108, 0x4108, 0x6108, 0x8108, 0xA108, 0xC108, 0xE108,
	0x0208, 0x2208, 0x4208, 0x6208, 0x8208, 0xA208, 0xC208, 0xE208,
	0x0308, 0x2308, 0x4308, 0x6308, 0x8308, 0xA308, 0xC308, 0xE308,
	0x0408, 0x2408, 0x4408, 0x6408, 0x8408, 0xA408, 0xC408, 0xE408,
	0x0508, 0x2508, 0x4508, 0x6508, 0x8508, 0xA508, 0xC508, 0xE508,
	0x0608, 0x2608, 0x4608, 0x6608, 0x8608, 0xA608, 0xC608, 0xE608,
	0x0708, 0x2708, 0x4708, 0x6708, 0x8708, 0xA708, 0xC708, 0xE708,
	0x0010, 0x2010, 0x4010, 0x6010, 0x8010, 0xA010, 0xC010, 0xE010,
	0x0110, 0x2110, 0x4110, 0x6110, 0x8110, 0xA110, 0xC110, 0xE110,
	0x0210, 0x2210, 0x4210, 0x6210, 0x8210, 0xA210, 0xC210, 0xE210,
	0x0310, 0x2310, 0x4310, 0x6310, 0x8310, 0xA310, 0xC310, 0xE310,
	0x0410, 0x2410, 0x4410, 0x6410, 0x8410, 0xA410, 0xC410, 0xE410,
	0x0510, 0x2510, 0x4510, 0x6510, 0x8510, 0xA510, 0xC510, 0xE510,
	0x0610, 0x2610, 0x4610, 0x6610, 0x8610, 0xA610, 0xC610, 0xE610,
	0x0710, 0x2710, 0x4710, 0x6710, 0x8710, 0xA710, 0xC710, 0xE710,
	0x0018, 0x2018, 0x4018, 0x6018, 0x8018, 0xA018, 0xC018, 0xE018,
	0x0118, 0x2118, 0x4118, 0x6118, 0x8118, 0xA118, 0xC118, 0xE118,
	0x0218, 0x2218, 0x4218, 0x6218, 0x8218, 0xA218, 0xC218, 0xE218,
	0x0318, 0x2318, 0x4318, 0x6318, 0x8318, 0xA318, 0xC318, 0xE318,
	0x0418, 0x2418, 0x4418, 0x6418, 0x8418, 0xA418, 0xC418, 0xE418,
	0x0518, 0x2518, 0x4518, 0x6518, 0x8518, 0xA518, 0xC518, 0xE518,
	0x0618, 0x2618, 0x4618, 0x6618, 0x8618, 0xA618, 0xC618, 0xE618,
	0x0718, 0x2718, 0x4718, 0x6718, 0x8718, 0xA718, 0xC718, 0xE718
};
#endif /* DISP_DEVICE_8BPP */

static void tovis_qvga_disp_init(struct platform_device *pdev);
static void tovis_qvga_disp_set_rect(int x, int y, int xres, int yres);

#if 0
static void tovis_qvga_disp_set_contrast(void);
static void tovis_qvga_disp_set_display_area(word start_row, word end_row);
#endif
static int tovis_qvga_disp_off(struct platform_device *pdev);
static int tovis_qvga_disp_on(struct platform_device *pdev);

static void tovis_qvga_disp_init(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	if (disp_initialized)
		return;

	mfd = platform_get_drvdata(pdev);

	DISP_CMD_PORT = mfd->cmd_port;
	DISP_DATA_PORT = mfd->data_port;

	disp_initialized = TRUE;
}

static void tovis_qvga_disp_set_rect(int x, int y, int xres, int yres) // xres = width, yres - height
{
	if (!disp_initialized)
		return;

	EBI2_WRITE16C(DISP_CMD_PORT, 0x2a); // Set_Column address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xef); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0x2b); // Set_Page address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
}

static void ilitek_qvga_disp_set_rect(int x, int y, int xres, int yres) // xres = width, yres - height
{
	if (!disp_initialized)
		return;

	EBI2_WRITE16C(DISP_CMD_PORT, 0x2a); // Set_Column address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xef); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0x2b); // Set_Page address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
}

static int ebi2_power_save_on = 1;
static void msm_fb_ebi2_power_save(int on)
{
	struct vreg *vreg;
	int flag_on = !!on;

	if (ebi2_power_save_on == flag_on)
		return;

 	ebi2_power_save_on = flag_on;

	if (on) {
//		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], enable, 1800);
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], enable, 2800);
	} else {
//		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[0], disable, 0);
		MSM_FB_LCDC_VREG_OP(msm_fb_vreg[1], disable, 0);
	}
}

#ifdef LCD_SLEEP_ENABLE
static int tovis_qvga_disp_off(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (display_on) {
		// (b) -> (a)
		EBI2_WRITE16C(DISP_CMD_PORT, 0x28);  // ScreenOff
		mdelay(20);
		EBI2_WRITE16C(DISP_CMD_PORT, 0x10);  // AMP Off
		mdelay(120);

		// (a) -> (d)
		EBI2_WRITE16C(DISP_CMD_PORT, 0xb1); 
		EBI2_WRITE16D(DISP_DATA_PORT,0x01); 

		display_on = FALSE;
	}

	return 0;
}
#else
static int tovis_qvga_disp_off(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (display_on) {
		// (b) -> (a)
		EBI2_WRITE16C(DISP_CMD_PORT, 0x28); // Display Off
		mdelay(20);
		EBI2_WRITE16C(DISP_CMD_PORT, 0x10); // AMP Off
		mdelay(120);

		msm_fb_ebi2_power_save(0);
		display_on = FALSE;
	}

	return 0;
}
#endif

#ifdef LCD_SLEEP_ENABLE
static int ilitek_qvga_disp_off(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (display_on) {
		// perform lcd deep sleep instead of power off
		EBI2_WRITE16C(DISP_CMD_PORT, 0x28); 
		mdelay(50);
		EBI2_WRITE16C(DISP_CMD_PORT, 0x10); // SPLIN
		mdelay(120);
		/* TEST */ msm_fb_ebi2_power_save(0);
		display_on = FALSE;
	}

	return 0;
}
#else
static int ilitek_qvga_disp_off(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (display_on) {
		EBI2_WRITE16C(DISP_CMD_PORT, 0x28); 
		mdelay(50);
		msm_fb_ebi2_power_save(0);
		display_on = FALSE;
	}

	return 0;
}
#endif


struct msm_fb_panel_data tovis_qvga_panel_data = {
	.on = tovis_qvga_disp_on,
	.off = tovis_qvga_disp_off,
	.set_backlight = NULL,
	.set_rect = tovis_qvga_disp_set_rect,
};

static struct platform_device this_device = {
	.name   = "ebi2_tovis_qvga",
	.id	= 1,
	.dev	= {
		.platform_data = &tovis_qvga_panel_data,
	}
};

static void do_tovis_init(void) {
	EBI2_WRITE16C(DISP_CMD_PORT,0xb0); // Manufacturer command access protect
	EBI2_WRITE16D(DISP_DATA_PORT,0x0);

	EBI2_WRITE16C(DISP_CMD_PORT,0xb1); // Low power mode control
	EBI2_WRITE16D(DISP_DATA_PORT,0x0);
	
	EBI2_WRITE16C(DISP_CMD_PORT,0xb3); // frame memory & I/F setting
	EBI2_WRITE16D(DISP_DATA_PORT,0x02);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00);
	EBI2_WRITE16D(DISP_DATA_PORT,0x01);

	EBI2_WRITE16C(DISP_CMD_PORT,0xb4); // Display mode & frame memory write mode setting
	EBI2_WRITE16D(DISP_DATA_PORT,0x0);

	EBI2_WRITE16C(DISP_CMD_PORT,0xb8); // Back Light Control(1)
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x1f); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x1f); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 11 
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 15
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 16
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 17
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 18
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 19
	EBI2_WRITE16D(DISP_DATA_PORT,0xff); // 20 

	EBI2_WRITE16C(DISP_CMD_PORT,0xb9); // Back Light Control(2)
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x1d); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0xc0); // Panel driving setting
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x4f); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x33); // 8

	EBI2_WRITE16C(DISP_CMD_PORT,0xc1); // Display timing setting for normal mode
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x1c); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x06); // 5

	EBI2_WRITE16C(DISP_CMD_PORT,0xc3); // Display timing setting for idle mode
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x1c); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 5

	EBI2_WRITE16C(DISP_CMD_PORT,0xc4); // Source/Vcom/gate driving time setting
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x43); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x05); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0xc8); // Gamma Set A (Blue)
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x14); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xdd); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x09); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 11 
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0xd9); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 15
	EBI2_WRITE16D(DISP_DATA_PORT,0x14); // 16
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 17
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 18
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 19
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 20 

	EBI2_WRITE16C(DISP_CMD_PORT,0xc9); // Gamma Set B (Green)
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xde); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 11 
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0xd8); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e); // 15
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 16
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 17
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 18
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 19
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 20 

	EBI2_WRITE16C(DISP_CMD_PORT,0xca); // Gamma Set C (Red)
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xde); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 11 
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0xd8); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e); // 15
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 16
	EBI2_WRITE16D(DISP_DATA_PORT,0x0b); // 17
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 18
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 19
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 20 

	EBI2_WRITE16C(DISP_CMD_PORT,0xd0); // Power setting(common)
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0xc6); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0xdd); // 3

	EBI2_WRITE16C(DISP_CMD_PORT,0xd1); // Vcom setting
	EBI2_WRITE16D(DISP_DATA_PORT,0x69); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 3

	EBI2_WRITE16C(DISP_CMD_PORT,0xd2); // Power setting for normal mode
	EBI2_WRITE16D(DISP_DATA_PORT,0x63); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x24); // 2

	EBI2_WRITE16C(DISP_CMD_PORT,0xd4); // Power setting for idle mode
	EBI2_WRITE16D(DISP_DATA_PORT,0x63); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x24); // 2

	EBI2_WRITE16C(DISP_CMD_PORT,0xd7); // Test Register
	EBI2_WRITE16D(DISP_DATA_PORT,0x06); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x16); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x88); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0xaa); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0xca); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 9
	
	EBI2_WRITE16C(DISP_CMD_PORT,0xd8); // Sequencer Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x77); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x77); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x33); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x75); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x91); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x09); // 9

	EBI2_WRITE16C(DISP_CMD_PORT, 0xd9); // Test Register1
	EBI2_WRITE16D(DISP_DATA_PORT,0xfb); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x9f); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xe0); // NVM Access Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xe4); // Test Register2
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xe5); // Test Register3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2

	EBI2_WRITE16C(DISP_CMD_PORT,0xfa); // Test Register4
	EBI2_WRITE16D(DISP_DATA_PORT,0x04); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3

	EBI2_WRITE16C(DISP_CMD_PORT,0xfc); // Test Register5
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 7
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 11 

	EBI2_WRITE16C(DISP_CMD_PORT,0xfd); // Test Register6
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 7

	EBI2_WRITE16C(DISP_CMD_PORT,0xfe); // Test Register7
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x21); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x35); // Set_tear on
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1

	EBI2_WRITE16C(DISP_CMD_PORT,0x36); // Set_address_mode
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1

	EBI2_WRITE16C(DISP_CMD_PORT,0x3a); // Set_pixel_format
	EBI2_WRITE16D(DISP_DATA_PORT,0x55); // 1

//	EBI2_WRITE16C(DISP_CMD_PORT,0x44); // Set_tear_scanline
//	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
//	EBI2_WRITE16D(DISP_DATA_PORT,0xb8); // 2

	EBI2_WRITE16C(DISP_CMD_PORT,0x2a); // Set_column_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xef); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x2b); // Set_Page_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 4

/*-- bootlogo is displayed at oemsbl
	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
	for(y = 0; y < 320; y++) {
		int pixel = 0x0;
		for(x= 0; x < 240; x++) {
			EBI2_WRITE16D(DISP_DATA_PORT,pixel); // 1
		}
	}
*/

	EBI2_WRITE16C(DISP_CMD_PORT,0x11); // Exit_sleep_mode
	mdelay(50); //mdelay(120);

	EBI2_WRITE16C(DISP_CMD_PORT,0x29); // Display On
	mdelay(15);
}

static void do_ilitek_init(void) {

#ifdef ILITEK_ILI9340 /* for pecan */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc0);
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xc1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 1
	
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc2);
	EBI2_WRITE16D(DISP_DATA_PORT,0x22); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0xc5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x39); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x4e); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0x13);

	EBI2_WRITE16C(DISP_CMD_PORT, 0x36);
	EBI2_WRITE16D(DISP_DATA_PORT,0x48); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0x3a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x05); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x17); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb4);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x06); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x14); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb6);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x27); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0x44); // Tearing effect Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0xe2); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xe0); // Positive Gamma Correction
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x2c); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x24); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x07); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x4b); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x55); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x37); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x04); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 11
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 15

	EBI2_WRITE16C(DISP_CMD_PORT, 0xe1); // Negative Gamma Correction
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x15); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x1d); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x36); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x44); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x4a); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x06); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 11
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x2c); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x17); // 15

	EBI2_WRITE16C(DISP_CMD_PORT,0x2a); // Set_column_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xef); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x2b); // Set_Page_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0xe8); // Charge Sharing Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x89); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x79); // 3

	EBI2_WRITE16C(DISP_CMD_PORT,0x11); // Exit Sleep
		
#else	/* ILITEK_ILI9340C for muscat */

	EBI2_WRITE16C(DISP_CMD_PORT, 0xc0);
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0xc1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x11); // 1
	
	EBI2_WRITE16C(DISP_CMD_PORT, 0xc5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x38); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x50); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xcb);
	EBI2_WRITE16D(DISP_DATA_PORT,0x39); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x2c); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x34); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 5

	EBI2_WRITE16C(DISP_CMD_PORT, 0xcf);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0xaa); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0xb0); // 3

  /* Driver timing control */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe8);
	EBI2_WRITE16D(DISP_DATA_PORT,0x8a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x78); // 3

	EBI2_WRITE16C(DISP_CMD_PORT, 0xea);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2

  /* Power on sequence control */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xed);
	EBI2_WRITE16D(DISP_DATA_PORT,0x67); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x03); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x81); // 4

  /* Pump ratio control */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xf7);
	EBI2_WRITE16D(DISP_DATA_PORT,0x20); // 1

  /* Display mode setting */
  EBI2_WRITE16C(DISP_CMD_PORT, 0x13);

	EBI2_WRITE16C(DISP_CMD_PORT, 0x36);
	EBI2_WRITE16D(DISP_DATA_PORT,0x88); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0x3a);
	EBI2_WRITE16D(DISP_DATA_PORT,0x05); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x17); // 2

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb4);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb5);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x06); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x14); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0xb6);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x27); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 4

	EBI2_WRITE16C(DISP_CMD_PORT, 0x35);
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1

	EBI2_WRITE16C(DISP_CMD_PORT, 0x44); // Tearing effect Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0xab); // 2

  /* Positive Gamma Correction */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe0);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x32); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x2d); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x0e); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x05); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x4d); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x55); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x34); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x0d); // 11
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0x12); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 15

  /* Negative Gamma Correction */
	EBI2_WRITE16C(DISP_CMD_PORT, 0xe1);
	EBI2_WRITE16D(DISP_DATA_PORT,0x0a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x14); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x02); // 4
	EBI2_WRITE16D(DISP_DATA_PORT,0x0f); // 5
	EBI2_WRITE16D(DISP_DATA_PORT,0x05); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x34); // 6
	EBI2_WRITE16D(DISP_DATA_PORT,0x44); // 8
	EBI2_WRITE16D(DISP_DATA_PORT,0x4d); // 9
	EBI2_WRITE16D(DISP_DATA_PORT,0x08); // 10
	EBI2_WRITE16D(DISP_DATA_PORT,0x10); // 11
	EBI2_WRITE16D(DISP_DATA_PORT,0x0c); // 12
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f); // 13
	EBI2_WRITE16D(DISP_DATA_PORT,0x2f); // 14
	EBI2_WRITE16D(DISP_DATA_PORT,0x17); // 15

	EBI2_WRITE16C(DISP_CMD_PORT,0x2a); // Set_column_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0xef); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0x2b); // Set_Page_address
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x00); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 3
	EBI2_WRITE16D(DISP_DATA_PORT,0x3f); // 4

	EBI2_WRITE16C(DISP_CMD_PORT,0xe8); // Charge Sharing Control
	EBI2_WRITE16D(DISP_DATA_PORT,0x8a); // 1
	EBI2_WRITE16D(DISP_DATA_PORT,0x01); // 2
	EBI2_WRITE16D(DISP_DATA_PORT,0x78); // 3

	EBI2_WRITE16C(DISP_CMD_PORT,0x11); // Exit Sleep 	
#endif	
	
	mdelay(80);

/*-- bootlogo is displayed at oemsbl
	EBI2_WRITE16C(DISP_CMD_PORT,0x2c); // Write memory start
	for(y = 0; y < 320; y++) {
		int pixel = 0x0;
		for(x= 0; x < 240; x++) {
			EBI2_WRITE16D(DISP_DATA_PORT,pixel); // 1
		}
	}

	mdelay(50);
*/
	EBI2_WRITE16C(DISP_CMD_PORT,0x29); // Display On 
}

#ifdef LCD_SLEEP_ENABLE
static int tovis_qvga_disp_on(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (!display_on) {
		mdelay(10);
		gpio_set_value(102, 0);
		mdelay(1);
		gpio_set_value(102, 1);
		mdelay(5);
		display_on = TRUE;

		do_tovis_init();	
/*
		// (d) -> (a)
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		mdelay(5);
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		EBI2_WRITE16C(DISP_CMD_PORT, 0xff); // CSX Falling Edge
		mdelay(10);
*/
	}
	return 0;
}
#else
static int tovis_qvga_disp_on(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (!display_on) {
		msm_fb_ebi2_power_save(1);
		mdelay(10);
		
		gpio_set_value(102, 0);
		mdelay(1);
		gpio_set_value(102, 1);
		mdelay(5);
		display_on = TRUE;

		do_tovis_init();	
	}
	return 0;
}
#endif

#ifdef LCD_SLEEP_ENABLE
static int ilitek_qvga_disp_on(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (!display_on) {
		/* TEST */ msm_fb_ebi2_power_save(1);
		mdelay(10);
		gpio_set_value(102, 1);
		mdelay(1);
		gpio_set_value(102, 0);
		mdelay(40);
		gpio_set_value(102, 1);
		mdelay(10); //mdelay(120);
		display_on = TRUE;

		do_ilitek_init();		
	}
	return 0;
}
#else
static int ilitek_qvga_disp_on(struct platform_device *pdev)
{
	if (!disp_initialized)
		tovis_qvga_disp_init(pdev);

	if (!display_on) {
		msm_fb_ebi2_power_save(1);
		mdelay(10);
		gpio_set_value(102, 1);
		mdelay(1);
		gpio_set_value(102, 0);
		mdelay(40);
		gpio_set_value(102, 1);
		mdelay(10); //mdelay(120);
		display_on = TRUE;

		do_ilitek_init();		
	}
	return 0;
}

#endif

ssize_t tovis_qvga_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", display_on);
}

ssize_t tovis_qvga_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int onoff;
	sscanf(buf, "%d", &onoff);
	
	if (onoff) {
		tovis_qvga_panel_data.on(&this_device);
	} else {
		tovis_qvga_panel_data.off(&this_device);
	}

	return count;
}

DEVICE_ATTR(lcd_onoff, 0666, tovis_qvga_show_onoff, tovis_qvga_store_onoff);

static int __init tovis_qvga_probe(struct platform_device *pdev)
{
	int ret;
	
	if (pdev->id == 0) {
		return 0;
	}

	msm_fb_add_device(pdev);
	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);
	if (ret) {
		printk("tovis_qvga_probe device_creat_file failed!!!\n");
	}
	
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = tovis_qvga_probe,
	.driver = {
		.name   = "ebi2_tovis_qvga",
	},
};

static int __init tovis_qvga_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

	ret = platform_driver_register(&this_driver);
	if (!ret) {
		int lcd_maker_id = 1; /* 1 : BOE, 0 : LGD */
		
		/* FIXME: the dependancy of H/W should be removed */
		if (lge_bd_rev == LGE_REV_C) {
			// check
			gpio_tlmm_config(GPIO_CFG(38, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			
			gpio_set_value(32, 0);
			gpio_set_value(33, 1);
			gpio_set_value(36, 1);

			if (gpio_get_value(38) == 0) // end key
				lcd_maker_id = 0;
			else
				lcd_maker_id = 1;
		} else {
			if (gpio_get_value(101) == 0) /* LCD_MAKER_ID */
				lcd_maker_id = 0;
			else
				lcd_maker_id = 1;
		}

		if (lcd_maker_id == 0) {
			printk("kurze-LCD is Tovis (END Key Pressed!)\n");
			tovis_qvga_panel_data.on = tovis_qvga_disp_on;
			tovis_qvga_panel_data.off = tovis_qvga_disp_off;
			tovis_qvga_panel_data.set_backlight = NULL;
			tovis_qvga_panel_data.set_rect = tovis_qvga_disp_set_rect;
		} else {
			printk("kurze-LCD is Ilitek (No END Key Pressed!)\n");
		 	tovis_qvga_panel_data.on = ilitek_qvga_disp_on;
			tovis_qvga_panel_data.off = ilitek_qvga_disp_off;
			tovis_qvga_panel_data.set_backlight = NULL;
			tovis_qvga_panel_data.set_rect = ilitek_qvga_disp_set_rect;
		}

		pinfo = &tovis_qvga_panel_data.panel_info;
		pinfo->xres = 240;
		pinfo->yres = 320;
		pinfo->type = EBI2_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->wait_cycle = 0x808000;  // ebi2 write timing reduced by bongkyu.kim

#ifdef TOVIS_LCD_18BPP
		pinfo->bpp = 18;
#else
		pinfo->bpp = 16;
#endif
		pinfo->fb_num = 2;
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = 6000;
		pinfo->lcd.v_back_porch = 8;
		pinfo->lcd.v_front_porch = 4;
		pinfo->lcd.v_pulse_width = 0;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = 0;

		ret = platform_device_register(&this_device);
		if (ret)
			platform_driver_unregister(&this_driver);
	}

	return ret;
}

module_init(tovis_qvga_init);

