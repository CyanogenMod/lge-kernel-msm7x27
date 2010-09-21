/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
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

#include "msm_fb.h"

static int __init lcdc_qrdc_init(void)
{
	int ret;
	struct msm_panel_info pinfo;

	if (msm_fb_detect_client("lcdc_qrdc"))
		return 0;

	pinfo.xres = 1366;
	pinfo.yres = 768;
	pinfo.type = LCDC_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	pinfo.fb_num = 2;
	pinfo.clk_rate = 43192000;

	pinfo.lcdc.h_back_porch = 120;
	pinfo.lcdc.h_front_porch = 20;
	pinfo.lcdc.h_pulse_width = 40;
	pinfo.lcdc.v_back_porch = 25;
	pinfo.lcdc.v_front_porch = 1;
	pinfo.lcdc.v_pulse_width = 7;
	pinfo.lcdc.border_clr = 0;      /* blk */
	pinfo.lcdc.underflow_clr = 0xff;        /* blue */
	pinfo.lcdc.hsync_skew = 0;

	ret = lcdc_device_register(&pinfo);
	if (ret)
		printk(KERN_ERR "%s: failed to register device\n", __func__);

	return ret;
}

module_init(lcdc_qrdc_init);
