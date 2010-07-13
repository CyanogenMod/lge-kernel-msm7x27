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

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/clk.h>
#include <mach/msm_iomap.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_bus.h>
#include "clock.h"
#include "footswitch.h"

#define REG(off) (MSM_MMSS_CLK_CTL_BASE + (off))
#define GEMINI_GFS_CTL_REG	REG(0x01A0)
#define GFX2D0_GFS_CTL_REG	REG(0x0180)
#define GFX2D1_GFS_CTL_REG	REG(0x0184)
#define GFX3D_GFS_CTL_REG	REG(0x0188)
#define MDP_GFS_CTL_REG		REG(0x0190)
#define ROT_GFS_CTL_REG		REG(0x018C)
#define VED_GFS_CTL_REG		REG(0x0194)
#define VFE_GFS_CTL_REG		REG(0x0198)
#define VPE_GFS_CTL_REG		REG(0x019C)
#define SW_RESET_CORE_REG	REG(0x0210)

#define CLAMP_BIT		BIT(5)
#define ENABLE_BIT		BIT(8)
#define RETENTION_BIT		BIT(9)

#define MAX_FS_CLOCKS 5
struct footswitch {
	struct regulator_dev	*rdev;
	struct regulator_desc	desc;
	void			*gfs_ctl_reg;
	const char		*clk_names[MAX_FS_CLOCKS];
	struct clk		*clk[MAX_FS_CLOCKS];
	int			bus_port1, bus_port2;
	int			core_reset_mask;
	int			is_enabled;
	unsigned int		gfs_delay_cnt:5;
};

static DEFINE_SPINLOCK(reset_reg_lock);
static struct clk *gfx3d_clk;

static int footswitch_is_enabled(struct regulator_dev *rdev)
{
	struct footswitch *fs = rdev_get_drvdata(rdev);

	return fs->is_enabled;
}

static int footswitch_enable(struct regulator_dev *rdev)
{
	struct footswitch *fs = rdev_get_drvdata(rdev);
	int clk_enabled[MAX_FS_CLOCKS];
	int gfx3d_rate = 0;
	uint32_t regval, i, rc = 0;

	if (fs->is_enabled)
		return 0;

	/* Turn on all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_enabled[i] = !clk_enable(fs->clk[i]);

	/* Un-halt all bus ports in the power domain */
	if (fs->bus_port1) {
		rc = msm_bus_axi_portunhalt(fs->bus_port1);
		if (rc) {
			pr_err("%s: Port 1 unhalt failed.\n", __func__);
			goto out;
		}
	}
	if (fs->bus_port2) {
		rc = msm_bus_axi_portunhalt(fs->bus_port2);
		if (rc) {
			pr_err("%s: Port 2 unhalt failed.\n", __func__);
			goto out;
		}
	}

	/* gfx3d_clk must run at XO speed while being reset. */
	if (fs->desc.id == FS_GFX3D) {
		int ret;
		gfx3d_rate = clk_get_rate(gfx3d_clk);
		ret = clk_set_rate(gfx3d_clk, 27000000);
		if (ret)
			pr_err("%s: Failed to slow gfx3d_clk.\n", __func__);
	}

	/* Assert resets for all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_reset(fs->clk[i], CLK_RESET_ASSERT);

	/* Wait for any synchronous resets to propagate (a few
	 * clock cycles of slowest clock in the power domain). */
	udelay(5);

	/* Enable the power rail at the footswitch. */
	regval = readl(fs->gfs_ctl_reg);
	regval |= ENABLE_BIT;
	writel(regval, fs->gfs_ctl_reg);

	/* Wait 2us for the rail to fully charge. */
	udelay(2);

	/* Force-off all clock in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_output_disable(fs->clk[i]);

	/* Un-clamp the I/O ports. */
	regval &= ~CLAMP_BIT;
	writel(regval, fs->gfs_ctl_reg);

	/* Wait for the clamps to clear and signals to settle. */
	udelay(5);

	/* Force-on all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_output_enable(fs->clk[i]);

	/* Toggle core reset and wait for it to propogate. */
	spin_lock(&reset_reg_lock);
	regval = readl(SW_RESET_CORE_REG);
	regval |= fs->core_reset_mask;
	writel(regval, SW_RESET_CORE_REG);
	udelay(5);
	regval &= ~(fs->core_reset_mask);
	writel(regval, SW_RESET_CORE_REG);
	spin_unlock(&reset_reg_lock);
	udelay(5);

	/* Deassert resets for all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_reset(fs->clk[i], CLK_RESET_DEASSERT);

	/* gfx3d_clk was slowed down for reset, restore the speed. */
	if (fs->desc.id == FS_GFX3D) {
		int ret = clk_set_rate(gfx3d_clk, gfx3d_rate);
		if (ret)
			pr_err("%s: Failed to restore gfx3d_clk.\n", __func__);
	}

	/* Return clocks to their state before this function. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		if (clk_enabled[i])
			clk_disable(fs->clk[i]);

	fs->is_enabled = 1;

out:
	return rc;
}

static int footswitch_disable(struct regulator_dev *rdev)
{

	struct footswitch *fs = rdev_get_drvdata(rdev);
	int clk_enabled[MAX_FS_CLOCKS];
	uint32_t regval, i, rc = 0;

	if (!fs->is_enabled)
		return 0;

	/* Turn on all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_enabled[i] = !clk_enable(fs->clk[i]);

	/* Halt all bus ports in the power domain */
	if (fs->bus_port1) {
		rc = msm_bus_axi_porthalt(fs->bus_port1);
		if (rc) {
			pr_err("%s: Port 1 halt failed.\n", __func__);
			goto err_port1_halt;
		}
	}
	if (fs->bus_port2) {
		rc = msm_bus_axi_porthalt(fs->bus_port2);
		if (rc) {
			pr_err("%s: Port 1 halt failed.\n", __func__);
			goto err_port2_halt;
		}
	}

	/* Assert core reset. */
	spin_lock(&reset_reg_lock);
	regval = readl(SW_RESET_CORE_REG);
	regval |= fs->core_reset_mask;
	writel(regval, SW_RESET_CORE_REG);
	spin_unlock(&reset_reg_lock);

	/* Assert resets for all clocks in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_reset(fs->clk[i], CLK_RESET_ASSERT);

	/* Wait for any synchronous resets to propagate (a few
	 * clock cycles of slowest clock in the power domain) */
	udelay(5);

	/* Force-off all clock in the power domain. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++)
		clk_output_disable(fs->clk[i]);

	/* Clamp the I/O ports of the core to ensure the values
	 * remain fixed while the core is collapsed. */
	regval = readl(fs->gfs_ctl_reg);
	regval |= CLAMP_BIT;
	writel(regval, fs->gfs_ctl_reg);

	/* Collapse the power rail at the footswitch. */
	regval &= ~ENABLE_BIT;
	writel(regval, fs->gfs_ctl_reg);

	/* Return clocks to their state before this function. */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i]; i++) {
		clk_output_enable(fs->clk[i]);
		if (clk_enabled[i])
			clk_disable(fs->clk[i]);
	}

	fs->is_enabled = 0;

	return rc;

err_port2_halt:
	msm_bus_axi_portunhalt(fs->bus_port1);
err_port1_halt:
	return rc;
}

static struct regulator_ops footswitch_ops = {
	.is_enabled = footswitch_is_enabled,
	.enable = footswitch_enable,
	.disable = footswitch_disable,
};

#define FOOTSWITCH(_id, _name, _gfs_ctl_reg, _dc, _bp1, _bp2, _cr, ...) \
	[(_id)] = { \
		.desc = { \
			.id = (_id), \
			.name = (_name), \
			.ops = &footswitch_ops, \
			.type = REGULATOR_VOLTAGE, \
			.owner = THIS_MODULE, \
		}, \
		.gfs_ctl_reg = (_gfs_ctl_reg), \
		.gfs_delay_cnt = (_dc), \
		.bus_port1 = (_bp1), \
		.bus_port2 = (_bp2), \
		.core_reset_mask = (_cr), \
		.clk_names = { __VA_ARGS__ } \
	}
static struct footswitch footswitches[] = {
	FOOTSWITCH(FS_GFX2D0, "fs_gfx2d0",
		GFX2D0_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_GRAPHICS_2D_CORE0, 0, BIT(14),
		"gfx2d0_clk", "gfx2d0_pclk"),
	FOOTSWITCH(FS_GFX3D, "fs_gfx3d",
		GFX3D_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_GRAPHICS_3D, 0, BIT(12),
		"gfx3d_clk", "gfx3d_pclk"),
	FOOTSWITCH(FS_IJPEG, "fs_ijpeg",
		GEMINI_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_JPEG_ENC, 0, BIT(9),
		"ijpeg_clk", "ijpeg_pclk", "ijpeg_axi_clk"),
	FOOTSWITCH(FS_MDP, "fs_mdp",
		MDP_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_MDP_PORT0,
		MSM_BUS_MMSS_MASTER_MDP_PORT1, BIT(21),
		"mdp_clk", "mdp_pclk", "mdp_axi_clk"),
	FOOTSWITCH(FS_ROT, "fs_rot",
		ROT_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_ROTATOR, 0, BIT(2),
		"rot_clk", "rotator_pclk", "rot_axi_clk"),
	FOOTSWITCH(FS_VED, "fs_ved",
		VED_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_HD_CODEC_PORT0,
		MSM_BUS_MMSS_MASTER_HD_CODEC_PORT1, BIT(6),
		"vcodec_clk", "vcodec_pclk", "vcodec_axi_clk"),
	FOOTSWITCH(FS_VFE, "fs_vfe",
		VFE_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_VFE, 0, BIT(15),
		"vfe_clk", "vfe_pclk", "vfe_axi_clk"),
	FOOTSWITCH(FS_VPE, "fs_vpe",
		VPE_GFS_CTL_REG, 31,
		MSM_BUS_MMSS_MASTER_VPE, 0, BIT(17),
		"vpe_clk", "vpe_pclk", "vpe_axi_clk"),
};

static int footswitch_probe(struct platform_device *pdev)
{
	struct footswitch *fs;
	struct regulator_init_data *init_data;
	uint32_t regval, i, rc = 0;

	if (pdev == NULL)
		return -EINVAL;

	if (pdev->id >= MAX_FS)
		return -ENODEV;

	fs = &footswitches[pdev->id];
	init_data = pdev->dev.platform_data;

	/* Set up clocks.  If a required clock is rate-settable but has not
	 * yet had its rate set, default the clock rate to its lowest (but
	 * leave the clock off). */
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk_names[i]; i++) {
		fs->clk[i] = clk_get(NULL, fs->clk_names[i]);
		if (IS_ERR(fs->clk[i])) {
			pr_err("%s: clk_get(\"%s\") failed\n",
				__func__, fs->clk_names[i]);
			rc = PTR_ERR(fs->clk[i]);
			goto err;
		}
		if (clk_get_rate(fs->clk[i]) == 0)
			clk_set_min_rate(fs->clk[i], 0);
	}
	/* gfx3d_clk requires additional control. */
	if (pdev->id == FS_GFX3D) {
		gfx3d_clk = clk_get(NULL, "gfx3d_clk");
		if (IS_ERR(gfx3d_clk)) {
			pr_err("%s: clk_get(\"gfx3d_clk\") failed\n", __func__);
			goto err;
		}
	}

	/* Set number of AHB_CLK cycles to delay the assertion of gfs_en_all
	 * after enabling the footswitch.  Also ensure the retention bit is
	 * clear so disabling the footswitch will power-collapse the core. */
	regval = readl(fs->gfs_ctl_reg);
	regval |= fs->gfs_delay_cnt;
	regval &= ~RETENTION_BIT;
	writel(regval, fs->gfs_ctl_reg);

	fs->rdev = regulator_register(&fs->desc, &pdev->dev, init_data, fs);
	if (IS_ERR(footswitches[pdev->id].rdev)) {
		pr_err("%s: regulator_register(\"%s\") failed\n",
			__func__, fs->desc.name);
		rc = PTR_ERR(footswitches[pdev->id].rdev);
		goto err;
	}

	return 0;

err:
	for (i = 0; i < MAX_FS_CLOCKS && fs->clk[i] && !IS_ERR(fs->clk[i]); i++)
		clk_put(fs->clk[i]);
	return rc;
}

static int __devexit footswitch_remove(struct platform_device *pdev)
{
	struct footswitch *fs = &footswitches[pdev->id];
	int i;

	for (i = 0; i < MAX_FS_CLOCKS; i++)
		if (fs->clk[i] != NULL)
			clk_put(fs->clk[i]);
	regulator_unregister(fs->rdev);

	return 0;
}

static struct platform_driver footswitch_driver = {
	.probe		= footswitch_probe,
	.remove		= __devexit_p(footswitch_remove),
	.driver		= {
		.name		= "footswitch-msm8x60",
		.owner		= THIS_MODULE,
	},
};

static int __init footswitch_init(void)
{
	return platform_driver_register(&footswitch_driver);
}
subsys_initcall(footswitch_init);

static void __exit footswitch_exit(void)
{
	platform_driver_unregister(&footswitch_driver);
}
module_exit(footswitch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MSM8x60 rail footswitch");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:footswitch-msm8x60");
