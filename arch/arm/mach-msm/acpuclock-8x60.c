/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
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
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/regulator/consumer.h>

#include <asm/cpu.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>

#include "acpuclock.h"
#include "clock-8x60.h"
#include "avs.h"

#define dprintk(msg...) \
	cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)

/* Frequency switch modes. */
#define SHOT_SWITCH		4
#define HOP_SWITCH		5
#define SIMPLE_SLEW		6
#define COMPLEX_SLEW		7

/* PLL calibration limits.
 * The PLL hardware is capable of 384MHz to 1536MHz. The L_VALs
 * used for calibration should respect these limits. */
#define L_VAL_SCPLL_CAL_MIN	0x08 /* =  432 MHz with 27MHz source */
#define L_VAL_SCPLL_CAL_MAX	0x1C /* = 1512 MHz with 27MHz source */

#define MAX_VDD_SC		1200000 /* uV */
#define MAX_VDD_MEM		1200000 /* uV */
#define MAX_VDD_DIG		1200000 /* uV */
#define MAX_AXI			262000 /* KHz */

/* SCPLL Modes. */
#define SCPLL_POWER_DOWN	0
#define SCPLL_BYPASS		1
#define SCPLL_STANDBY		2
#define SCPLL_FULL_CAL		4
#define SCPLL_HALF_CAL		5
#define SCPLL_STEP_CAL		6
#define SCPLL_NORMAL		7

#define SCPLL_DEBUG_NONE	0
#define SCPLL_DEBUG_FULL	3

/* SCPLL registers offsets. */
#define SCPLL_DEBUG_OFFSET		0x0
#define SCPLL_CTL_OFFSET		0x4
#define SCPLL_CAL_OFFSET		0x8
#define SCPLL_STATUS_OFFSET		0x10
#define SCPLL_CFG_OFFSET		0x1C
#define SCPLL_FSM_CTL_EXT_OFFSET	0x24
#define SCPLL_LUT_A_HW_MAX		(0x38 + ((L_VAL_SCPLL_CAL_MAX / 4) * 4))

/* Clock registers. */
#define SPSS0_CLK_CTL_ADDR		(MSM_ACC0_BASE + 0x04)
#define SPSS0_CLK_SEL_ADDR		(MSM_ACC0_BASE + 0x08)
#define SPSS1_CLK_CTL_ADDR		(MSM_ACC1_BASE + 0x04)
#define SPSS1_CLK_SEL_ADDR		(MSM_ACC1_BASE + 0x08)
#define SPSS_L2_CLK_SEL_ADDR		(MSM_GCC_BASE  + 0x38)
static const void * const clk_ctl_addr[] = {SPSS0_CLK_CTL_ADDR,
			SPSS1_CLK_CTL_ADDR};
static const void * const clk_sel_addr[] = {SPSS0_CLK_SEL_ADDR,
			SPSS1_CLK_SEL_ADDR, SPSS_L2_CLK_SEL_ADDR};

static struct regulator *regulator_sc[NR_CPUS];
static struct regulator *regulator_mem[NR_CPUS];
static struct regulator *regulator_dig[NR_CPUS];

enum scplls {
	CPU0 = 0,
	CPU1,
	L2,
};

static const void * const sc_pll_base[] = {
	[CPU0]	= MSM_SCPLL_BASE + 0x200,
	[CPU1]	= MSM_SCPLL_BASE + 0x300,
	[L2]	= MSM_SCPLL_BASE + 0x400,
};

enum sc_src {
	ACPU_AFAB,
	ACPU_PLL_8,
	ACPU_SCPLL,
};

static struct clock_state {
	struct clkctl_acpu_speed	*current_speed[NR_CPUS];
	struct clkctl_l2_speed		*current_l2_speed;
	struct mutex			lock;
	uint32_t			acpu_switch_time_us;
	uint32_t			vdd_switch_time_us;
	uint32_t			max_speed_delta_khz;
} drv_state;

struct clkctl_l2_speed {
	unsigned int     khz;
	unsigned int     src_sel;
	unsigned int     l_val;
	unsigned int     vdd_dig;
	unsigned int     vdd_mem;
};

static struct clkctl_l2_speed *l2_vote[NR_CPUS];

struct clkctl_acpu_speed {
	unsigned int     use_for_scaling[2]; /* One for each CPU. */
	unsigned int     acpuclk_khz;
	int              pll;
	unsigned int     acpuclk_src_sel;
	unsigned int     acpuclk_src_div;
	unsigned int     core_src_sel;
	unsigned int     l_val;
	struct clkctl_l2_speed *l2_level;
	unsigned int     vdd_sc;
	unsigned int     avsdscr_setting;
	struct clkctl_acpu_speed *up;
	struct clkctl_acpu_speed *down;
};

/* L2 frequencies = 2 * 27 MHz * L_VAL */
static struct clkctl_l2_speed l2_freq_tbl[] = {
	[0] = {MAX_AXI, 0, 0,    1000000, 1100000},
	[1] = {432000,  1, 0x08, 1000000, 1100000},
	[2] = {486000,  1, 0x09, 1100000, 1200000},
	[3] = {540000,  1, 0x0A, 1100000, 1200000},
	[4] = {594000,  1, 0x0B, 1100000, 1200000},
	[5] = {648000,  1, 0x0C, 1200000, 1200000},
	[6] = {702000,  1, 0x0D, 1200000, 1200000},
	[7] = {756000,  1, 0x0E, 1200000, 1200000},
	[8] = {810000,  1, 0x0F, 1200000, 1200000},
	[9] = {864000,  1, 0x10, 1200000, 1200000},
};
#define L2(x) (&l2_freq_tbl[(x)])

#define CAL_IDX 2 /* acpu_freq_tbl row to use when reconfiguring SC/L2 PLLs. */
 /* SCPLL frequencies = 2 * 27 MHz * L_VAL */
static struct clkctl_acpu_speed acpu_freq_tbl[] = {
  { {1, 1},  192000,  ACPU_PLL_8, 3, 1, 0, 0,    L2(1),  900000, 0x03006000},
  { {0, 0},  MAX_AXI, ACPU_AFAB,  1, 0, 0, 0,    L2(1),  925000, 0x03006000},
  { {1, 1},  384000,  ACPU_PLL_8, 3, 0, 0, 0,    L2(1),  925000, 0x03006000},
  { {0, 0},  432000,  ACPU_SCPLL, 0, 0, 1, 0x08, L2(4),  975000, 0x03006000},
  { {0, 0},  486000,  ACPU_SCPLL, 0, 0, 1, 0x09, L2(4),  975000, 0x03006000},
  { {1, 1},  540000,  ACPU_SCPLL, 0, 0, 1, 0x0A, L2(4),  975000, 0x03006000},
  { {0, 0},  594000,  ACPU_SCPLL, 0, 0, 1, 0x0B, L2(9), 1025000, 0x03006000},
  { {1, 1},  648000,  ACPU_SCPLL, 0, 0, 1, 0x0C, L2(9), 1025000, 0x03006000},
  { {0, 0},  702000,  ACPU_SCPLL, 0, 0, 1, 0x0D, L2(9), 1100000, 0x03006000},
  { {1, 1},  756000,  ACPU_SCPLL, 0, 0, 1, 0x0E, L2(9), 1100000, 0x03006000},
  { {0, 0},  810000,  ACPU_SCPLL, 0, 0, 1, 0x0F, L2(9), 1175000, 0x03006000},
  { {0, 0},  864000,  ACPU_SCPLL, 0, 0, 1, 0x10, L2(9), 1175000, 0x03006000},
  { {1, 1},  918000,  ACPU_SCPLL, 0, 0, 1, 0x11, L2(9), 1175000, 0x03006000},
  { {0, 0},  972000,  ACPU_SCPLL, 0, 0, 1, 0x12, L2(9), 1200000, 0x03006000},
  { {0, 0}, 1026000,  ACPU_SCPLL, 0, 0, 1, 0x13, L2(9), 1200000, 0x03006000},
  { {0, 0}, 1080000,  ACPU_SCPLL, 0, 0, 1, 0x14, L2(9), 1200000, 0x03006000},
  { {0, 0}, 1134000,  ACPU_SCPLL, 0, 0, 1, 0x15, L2(9), 1200000, 0x03006000},
  { {1, 1}, 1188000,  ACPU_SCPLL, 0, 0, 1, 0x16, L2(9), 1200000, 0x03006000},
  { {0, 0}, 0 },
};

unsigned long acpuclk_get_rate(int cpu)
{
	return drv_state.current_speed[cpu]->acpuclk_khz;
}

uint32_t acpuclk_get_switch_time(void)
{
	return drv_state.acpu_switch_time_us;
}

unsigned long clk_get_max_axi_khz(void)
{
	return MAX_AXI;
}
EXPORT_SYMBOL(clk_get_max_axi_khz);

#define POWER_COLLAPSE_KHZ MAX_AXI
unsigned long acpuclk_power_collapse(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), POWER_COLLAPSE_KHZ, SETRATE_PC);
	return ret;
}

#define WAIT_FOR_IRQ_KHZ MAX_AXI
unsigned long acpuclk_wait_for_irq(void)
{
	int ret = acpuclk_get_rate(smp_processor_id());
	acpuclk_set_rate(smp_processor_id(), WAIT_FOR_IRQ_KHZ, SETRATE_SWFI);
	return ret;
}

static void select_core_source(unsigned int id, unsigned int src)
{
	uint32_t regval;
	int shift;

	shift = (id == L2) ? 0 : 1;
	regval = readl(clk_sel_addr[id]);
	regval &= ~(0x3 << shift);
	regval |= (src << shift);
	writel(regval, clk_sel_addr[id]);
}

static void select_clk_source_div(unsigned int id, struct clkctl_acpu_speed *s)
{
	uint32_t reg_clksel, reg_clkctl, src_sel;

	/* Configure the PLL divider mux if we plan to use it. */
	if (s->core_src_sel == 0) {

		reg_clksel = readl(clk_sel_addr[id]);

		/* CLK_SEL_SRC1N0 (bank) bit. */
		src_sel = reg_clksel & 1;

		/* Program clock source and divider. */
		reg_clkctl = readl(clk_ctl_addr[id]);
		reg_clkctl &= ~(0xFF << (8 * src_sel));
		reg_clkctl |= s->acpuclk_src_sel << (4 + 8 * src_sel);
		reg_clkctl |= s->acpuclk_src_div << (0 + 8 * src_sel);
		writel(reg_clkctl, clk_ctl_addr[id]);

		/* Toggle clock source. */
		reg_clksel ^= 1;

		/* Program clock source selection. */
		writel(reg_clksel, clk_sel_addr[id]);
	}
}

static void scpll_enable(int sc_pll, uint32_t l_val)
{
	uint32_t regval;

	/* Power-up SCPLL into standby mode. */
	writel(SCPLL_STANDBY, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(10);

	/* Shot-switch to target frequency. */
	regval = (l_val << 3) | SHOT_SWITCH;
	writel(regval, sc_pll_base[sc_pll] + SCPLL_FSM_CTL_EXT_OFFSET);
	writel(SCPLL_NORMAL, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(20);
}

static void scpll_disable(int sc_pll)
{
	/* Power down SCPLL. */
	writel(SCPLL_POWER_DOWN, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
}

static void scpll_change_freq(int sc_pll, uint32_t l_val)
{
	uint32_t regval;
	const void *base_addr = sc_pll_base[sc_pll];

	/* Complex-slew switch to target frequency. */
	regval = (l_val << 3) | COMPLEX_SLEW;
	writel(regval, base_addr + SCPLL_FSM_CTL_EXT_OFFSET);
	writel(SCPLL_NORMAL, base_addr + SCPLL_CTL_OFFSET);

	/* Wait for frequency switch to finish. */
	while (readl(base_addr + SCPLL_STATUS_OFFSET) & 0x1)
		cpu_relax();
}

static void l2_set_speed(struct clkctl_l2_speed *tgt_s)
{
	if (tgt_s == drv_state.current_l2_speed)
		return;

	if (drv_state.current_l2_speed->src_sel == 1
				&& tgt_s->src_sel == 1)
		scpll_change_freq(L2, tgt_s->l_val);
	else {
		if (tgt_s->src_sel == 1) {
			scpll_enable(L2, tgt_s->l_val);
			mb();
			select_core_source(L2, tgt_s->src_sel);
		} else {
			select_core_source(L2, tgt_s->src_sel);
			mb();
			scpll_disable(L2);
		}
	}
	drv_state.current_l2_speed = tgt_s;
}

/* Vote for L2 speed and set L2 to the max of all votes. */
static void update_l2_speed(unsigned int voting_cpu,
			    struct clkctl_l2_speed *tgt_s)
{
	struct clkctl_l2_speed *new_s;
	int cpu;

	/* Bounds check. */
	if (tgt_s > (l2_freq_tbl + ARRAY_SIZE(l2_freq_tbl)))
		return;

	/* Update voting CPU's L2 speed vote. */
	l2_vote[voting_cpu] = tgt_s;

	/* Find max vote L2 speed vote. */
	new_s = l2_freq_tbl;
	for_each_online_cpu(cpu)
		new_s = max(new_s, l2_vote[cpu]);

	/* Set L2 speed if max vote has changed. */
	l2_set_speed(new_s);
}

/* Apply any per-cpu voltage increases. */
static int increase_vdd(int cpu, unsigned int vdd_sc, unsigned int vdd_mem,
			unsigned int vdd_dig)
{
	int rc = 0;

	/* Increase vdd_mem before vdd_dig and vdd_sc.
	 * vdd_mem should be >= both vdd_sc and vdd_dig. */
	rc = regulator_set_voltage(regulator_mem[cpu], vdd_mem, MAX_VDD_MEM);
	if (rc) {
		pr_err("%s: vdd_mem (cpu%d) increase failed (%d)\n",
			__func__, cpu, rc);
		return rc;
	}

	/* Increase vdd_dig. */
	rc = regulator_set_voltage(regulator_dig[cpu], vdd_dig, MAX_VDD_DIG);
	if (rc) {
		pr_err("%s: vdd_dig (cpu%d) increase failed (%d)\n",
			__func__, cpu, rc);
		return rc;
	}

	/* Update per-core Scorpion voltage. */
	rc = regulator_set_voltage(regulator_sc[cpu], vdd_sc, MAX_VDD_SC);
	if (rc) {
		pr_err("%s: vdd_sc (cpu%d) increase failed (%d)\n",
			__func__, cpu, rc);
		return rc;
	}

	return rc;
}

/* Apply any per-cpu voltage decreases. */
static void decrease_vdd(int cpu, unsigned int vdd_sc, unsigned int vdd_mem,
			 unsigned int vdd_dig)
{
	int ret;

	/* Update per-core Scorpion voltage. */
	ret = regulator_set_voltage(regulator_sc[cpu], vdd_sc, MAX_VDD_SC);
	if (ret) {
		pr_err("%s: vdd_sc (cpu%d) decrease failed (%d)\n",
			__func__, cpu, ret);
		return;
	}

	/* Decrease vdd_dig. */
	ret = regulator_set_voltage(regulator_dig[cpu], vdd_dig, MAX_VDD_DIG);
	if (ret) {
		pr_err("%s: vdd_dig (cpu%d) decrease failed (%d)\n",
			__func__, cpu, ret);
		return;
	}

	/* Decrease vdd_mem after vdd_dig and vdd_sc.
	 * vdd_mem should be >= both vdd_sc and vdd_dig. */
	ret = regulator_set_voltage(regulator_mem[cpu], vdd_mem, MAX_VDD_MEM);
	if (ret) {
		pr_err("%s: vdd_mem (cpu%d) decrease failed (%d)\n",
			__func__, cpu, ret);
		return;
	}
}

static void switch_sc_speed(int cpu, struct clkctl_acpu_speed *tgt_s)
{
	struct clkctl_acpu_speed *strt_s = drv_state.current_speed[cpu];

	if (strt_s->pll != ACPU_SCPLL && tgt_s->pll != ACPU_SCPLL) {
		select_clk_source_div(cpu, tgt_s);
		/* Select core source because target may be AFAB. */
		select_core_source(cpu, tgt_s->core_src_sel);
	} else if (strt_s->pll != ACPU_SCPLL && tgt_s->pll == ACPU_SCPLL) {
		scpll_enable(cpu, tgt_s->l_val);
		mb();
		select_core_source(cpu, tgt_s->core_src_sel);
	} else if (strt_s->pll == ACPU_SCPLL && tgt_s->pll != ACPU_SCPLL) {
		select_clk_source_div(cpu, tgt_s);
		select_core_source(cpu, tgt_s->core_src_sel);
		mb();
		scpll_disable(cpu);
	} else
		scpll_change_freq(cpu, tgt_s->l_val);

	/* Update the driver state with the new clock freq */
	drv_state.current_speed[cpu] = tgt_s;
}

int acpuclk_set_rate(int cpu, unsigned long rate, enum setrate_reason reason)
{
	struct clkctl_acpu_speed *cur_s, *tgt_s, *strt_s;
	unsigned int vdd_mem, vdd_dig;
	int freq_index = 0;
	int rc = 0;

	if (cpu > num_possible_cpus()) {
		rc = -EINVAL;
		goto out;
	}

	if (reason == SETRATE_CPUFREQ)
		mutex_lock(&drv_state.lock);

	strt_s = drv_state.current_speed[cpu];

	/* Return early if rate didn't change. */
	if (rate == strt_s->acpuclk_khz)
		goto out;

	/* Find target frequency. */
	for (tgt_s = acpu_freq_tbl; tgt_s->acpuclk_khz != 0; tgt_s++) {
		if (tgt_s->acpuclk_khz == rate)
			break;
		freq_index++;
	}
	if (tgt_s->acpuclk_khz == 0) {
		rc = -EINVAL;
		goto out;
	}

	/* AVS needs SAW_VCTL to be intitialized correctly, before enable,
	 * and is not initialized at acpuclk_init().
	 */
	if (reason == SETRATE_CPUFREQ)
		AVS_DISABLE(cpu);

	/* Calculate vdd_mem and vdd_dig requirements.
	 * vdd_mem must be >= vdd_sc */
	vdd_mem = max(tgt_s->vdd_sc, tgt_s->l2_level->vdd_mem);
	vdd_dig = tgt_s->l2_level->vdd_dig;

	/* Increase VDD levels if needed. */
	if ((reason == SETRATE_CPUFREQ || reason == SETRATE_INIT)
			&& (tgt_s->acpuclk_khz > strt_s->acpuclk_khz)) {
		rc = increase_vdd(cpu, tgt_s->vdd_sc, vdd_mem, vdd_dig);
		if (rc)
			goto out;
	}

	dprintk("Switching from ACPU%d rate %u KHz -> %u KHz\n",
		cpu, strt_s->acpuclk_khz, tgt_s->acpuclk_khz);

	/* Step by at most drv_state.max_speed_delta_khz at a time. */
	cur_s = strt_s;
	while (cur_s != tgt_s) {
		int d = abs((int)(cur_s->acpuclk_khz - tgt_s->acpuclk_khz));
		if (d > drv_state.max_speed_delta_khz) {
			if (tgt_s->acpuclk_khz > cur_s->acpuclk_khz)
				cur_s = cur_s->up;
			else
				cur_s = cur_s->down;
		} else
			cur_s = tgt_s;
		switch_sc_speed(cpu, cur_s);
	}

	/* Update for L2 cache speed. */
	update_l2_speed(cpu, tgt_s->l2_level);

	/* Nothing else to do for SWFI. */
	if (reason == SETRATE_SWFI)
		goto out;

	/* Nothing else to do for power collapse. */
	if (reason == SETRATE_PC)
		goto out;

	/* Drop VDD levels if we can. */
	if (tgt_s->acpuclk_khz < strt_s->acpuclk_khz)
		decrease_vdd(cpu, tgt_s->vdd_sc, vdd_mem, vdd_dig);

	dprintk("ACPU%d speed change complete\n", cpu);

	/* Re-enable AVS */
	if (reason == SETRATE_CPUFREQ)
		AVS_ENABLE(cpu, tgt_s->avsdscr_setting);

out:
	if (reason == SETRATE_CPUFREQ)
		mutex_unlock(&drv_state.lock);
	return rc;
}

static void __init scpll_init(int sc_pll)
{
	uint32_t regval;

	dprintk("Initializing SCPLL%d\n", sc_pll);

	/* Clear calibration LUT registers containing max frequency entry.
	 * LUT registers are only writeable in debug mode. */
	writel(SCPLL_DEBUG_FULL, sc_pll_base[sc_pll] + SCPLL_DEBUG_OFFSET);
	writel(0x0, sc_pll_base[sc_pll] + SCPLL_LUT_A_HW_MAX);
	writel(SCPLL_DEBUG_NONE, sc_pll_base[sc_pll] + SCPLL_DEBUG_OFFSET);

	/* Power-up SCPLL into standby mode. */
	writel(SCPLL_STANDBY, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);
	udelay(10);

	/* Calibrate the SCPLL to the maximum range supported by the h/w. We
	 * might not use the full range of calibrated frequencies, but this
	 * simplifies changes required for future increases in max CPU freq.
	 */
	regval = (L_VAL_SCPLL_CAL_MAX << 24) | (L_VAL_SCPLL_CAL_MIN << 16);
	writel(regval, sc_pll_base[sc_pll] + SCPLL_CAL_OFFSET);

	/* Start calibration */
	writel(SCPLL_FULL_CAL, sc_pll_base[sc_pll] + SCPLL_CTL_OFFSET);

	/* Wait for proof that calibration has started before checking the
	 * 'calibration done' bit in the status register. Waiting for the
	 * LUT register we cleared to contain data accomplishes this.
	 * This is required since the 'calibration done' bit takes time to
	 * transition from 'done' to 'not done' when starting a calibration.
	 */
	while (readl(sc_pll_base[sc_pll] + SCPLL_LUT_A_HW_MAX) == 0)
		cpu_relax();

	/* Wait for calibration to complete. */
	while (readl(sc_pll_base[sc_pll] + SCPLL_STATUS_OFFSET) & 0x2)
		cpu_relax();

	/* Power-down SCPLL. */
	scpll_disable(sc_pll);
}

static void __init precompute_stepping(void)
{
	int i, step_idx;

#define cur_freq acpu_freq_tbl[i].acpuclk_khz
#define step_freq acpu_freq_tbl[step_idx].acpuclk_khz
#define cur_pll acpu_freq_tbl[i].pll

	for (i = 0; acpu_freq_tbl[i].acpuclk_khz; i++) {

		/* Calculate max "up" step at each frequency. */
		step_idx = i + 1;
		while (step_freq && (step_freq - cur_freq)
					<= drv_state.max_speed_delta_khz) {
			acpu_freq_tbl[i].up = &acpu_freq_tbl[step_idx];
			step_idx++;
		}
		if (step_idx == (i + 1) && step_freq) {
			pr_crit("Delta between freqs %u KHz and %u KHz is"
				" too high!\n", cur_freq, step_freq);
			BUG();
		}

		/* Calculate max "down" step at each frequency. */
		step_idx = i - 1;
		while (step_idx >= 0 && (cur_freq - step_freq)
					<= drv_state.max_speed_delta_khz) {
			acpu_freq_tbl[i].down =	&acpu_freq_tbl[step_idx];
			step_idx--;
		}
		if (step_idx == (i - 1) && i > 0) {
			pr_crit("Delta between freqs %u KHz and %u KHz is"
				" too high!\n", cur_freq, step_freq);
			BUG();
		}
	}
}

/* Force ACPU core and L2 cache clocks to rates that don't require SCPLLs. */
static void __init unselect_scplls(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		select_clk_source_div(cpu, &acpu_freq_tbl[CAL_IDX]);
		select_core_source(cpu, 0);
		drv_state.current_speed[cpu] = &acpu_freq_tbl[CAL_IDX];
		l2_vote[cpu] = acpu_freq_tbl[CAL_IDX].l2_level;
	}

	select_core_source(L2, 0);
	drv_state.current_l2_speed = acpu_freq_tbl[CAL_IDX].l2_level;
}

/* Ensure SCPLLs use the 27MHz XO. */
static void __init scpll_set_refs(void)
{
	int cpu;
	uint32_t regval;
	int use_pxo = pxo_is_27mhz();

	/* Bit 4 = 0:PXO, 1:MXO. */
	for_each_possible_cpu(cpu) {
		regval = readl(sc_pll_base[cpu] + SCPLL_CFG_OFFSET);
		if (use_pxo)
			regval &= ~BIT(4);
		else
			regval |= BIT(4);
		writel(regval, sc_pll_base[cpu] + SCPLL_CFG_OFFSET);
	}
	regval = readl(sc_pll_base[L2] + SCPLL_CFG_OFFSET);
	if (use_pxo)
		regval &= ~BIT(4);
	else
		regval |= BIT(4);
	writel(regval, sc_pll_base[L2] + SCPLL_CFG_OFFSET);
}

/* Voltage regulator initialization. */
static void __init regulator_init(void)
{
	struct clkctl_acpu_speed **freq = drv_state.current_speed;
	const char *regulator_sc_name[] = {"8901_s0", "8901_s1"};
	int cpu, ret;

	for_each_possible_cpu(cpu) {
		/* VDD_MEM votes. */
		regulator_mem[cpu] = regulator_get(NULL, "8058_s0");
		if (IS_ERR(regulator_mem[cpu]))
			goto err;
		ret = regulator_set_voltage(regulator_mem[cpu],
				freq[cpu]->vdd_sc, MAX_VDD_MEM);
		if (ret)
			goto err;
		ret = regulator_enable(regulator_mem[cpu]);
		if (ret)
			goto err;

		/* VDD_DIG votes. */
		regulator_dig[cpu] = regulator_get(NULL, "8058_s1");
		if (IS_ERR(regulator_dig[cpu]))
			goto err;
		ret = regulator_set_voltage(regulator_dig[cpu],
				freq[cpu]->l2_level->vdd_dig, MAX_VDD_DIG);
		if (ret)
			goto err;
		ret = regulator_enable(regulator_dig[cpu]);
		if (ret)
			goto err;

		/* VDD_SC0, VDD_SC1 */
		regulator_sc[cpu] = regulator_get(NULL, regulator_sc_name[cpu]);
		if (IS_ERR(regulator_sc[cpu]))
			goto err;
		ret = regulator_set_voltage(regulator_sc[cpu],
				freq[cpu]->vdd_sc, MAX_VDD_SC);
		if (ret)
			goto err;
		ret = regulator_enable(regulator_sc[cpu]);
		if (ret)
			goto err;
	}

	return;

err:
	pr_err("%s: Failed to initialize voltage regulators\n", __func__);
	BUG();
}

#ifdef CONFIG_CPU_FREQ_MSM
static struct cpufreq_frequency_table freq_table[NR_CPUS][20];

static void __init cpufreq_table_init(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		int i, freq_cnt = 0;
		/* Construct the freq_table tables from acpu_freq_tbl. */
		for (i = 0; acpu_freq_tbl[i].acpuclk_khz != 0
				&& freq_cnt < ARRAY_SIZE(*freq_table); i++) {
			if (acpu_freq_tbl[i].use_for_scaling[cpu]) {
				freq_table[cpu][freq_cnt].index = freq_cnt;
				freq_table[cpu][freq_cnt].frequency
					= acpu_freq_tbl[i].acpuclk_khz;
				freq_cnt++;
			}
		}
		/* freq_table not big enough to store all usable freqs. */
		BUG_ON(acpu_freq_tbl[i].acpuclk_khz != 0);

		freq_table[cpu][freq_cnt].index = freq_cnt;
		freq_table[cpu][freq_cnt].frequency = CPUFREQ_TABLE_END;

		pr_info("CPU%d: %d scaling frequencies supported.\n",
			cpu, freq_cnt);

		/* Register table with CPUFreq. */
		cpufreq_frequency_table_get_attr(freq_table[cpu], cpu);
	}
}
#else
static void __init cpufreq_table_init(void) {}
#endif

void __init msm_acpu_clock_init(struct msm_acpu_clock_platform_data *clkdata)
{
	int cpu;

	mutex_init(&drv_state.lock);
	drv_state.acpu_switch_time_us = clkdata->acpu_switch_time_us;
	drv_state.vdd_switch_time_us = clkdata->vdd_switch_time_us;
	drv_state.max_speed_delta_khz = clkdata->max_speed_delta_khz;

	/* Configure hardware. */
	unselect_scplls();
	scpll_set_refs();
	for_each_possible_cpu(cpu)
		scpll_init(cpu);
	scpll_init(L2);
	regulator_init();

	precompute_stepping();

	/* Improve boot time by ramping up CPUs immediately. */
	for_each_online_cpu(cpu)
		acpuclk_set_rate(cpu, 810000, SETRATE_INIT);

	cpufreq_table_init();
}
