/* arch/arm/mach-msm/proc_comm.c
 *
 * Copyright (C) 2007-2008 Google, Inc.
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>

#include "proc_comm.h"
#include "smd_private.h"
#ifdef CONFIG_LGE_BLUE_ERROR_HANDLER
/* LGE_CHANGE_S [bluerti@lge.com] 2009-07-06 <For Error Handler > */
#include "lge/lge_errorhandler.h"
/* LGE_CHANGE_E [bluerti@lge.com] 2009-07-06 <For Error Handler > */
#endif

#if defined(CONFIG_ARCH_MSM7X30)
#define MSM_TRIG_A2M_PC_INT (writel(1 << 6, MSM_GCC_BASE + 0x8))
#elif defined(CONFIG_ARCH_MSM8X60)
#define MSM_TRIG_A2M_PC_INT (writel(1 << 5, MSM_GCC_BASE + 0x8))
#else
#define MSM_TRIG_A2M_PC_INT (writel(1, MSM_CSR_BASE + 0x400 + (6) * 4))
#endif

static inline void notify_other_proc_comm(void)
{
	MSM_TRIG_A2M_PC_INT;
}

#define APP_COMMAND 0x00
#define APP_STATUS  0x04
#define APP_DATA1   0x08
#define APP_DATA2   0x0C

#define MDM_COMMAND 0x10
#define MDM_STATUS  0x14
#define MDM_DATA1   0x18
#define MDM_DATA2   0x1C

static DEFINE_SPINLOCK(proc_comm_lock);

/* Poll for a state change, checking for possible
 * modem crashes along the way (so we don't wait
 * forever while the ARM9 is blowing up.
 *
 * Return an error in the event of a modem crash and
 * restart so the msm_proc_comm() routine can restart
 * the operation from the beginning.
 */
static int proc_comm_wait_for(unsigned addr, unsigned value)
{
	while (1) {
		if (readl(addr) == value)
			return 0;

		if (smsm_check_for_modem_crash())
			return -EAGAIN;

		udelay(5);
	}
}

void msm_proc_comm_reset_modem_now(void)
{
	unsigned base = (unsigned)MSM_SHARED_RAM_BASE;
	unsigned long flags;

	spin_lock_irqsave(&proc_comm_lock, flags);

again:
	if (proc_comm_wait_for(base + MDM_STATUS, PCOM_READY))
		goto again;

	writel(PCOM_RESET_MODEM, base + APP_COMMAND);
	writel(0, base + APP_DATA1);
	writel(0, base + APP_DATA2);

	spin_unlock_irqrestore(&proc_comm_lock, flags);

	/* Make sure the writes complete before notifying the other side */
	dsb();

	notify_other_proc_comm();

	return;
}
EXPORT_SYMBOL(msm_proc_comm_reset_modem_now);

int msm_proc_comm(unsigned cmd, unsigned *data1, unsigned *data2)
{
	unsigned base = (unsigned)MSM_SHARED_RAM_BASE;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&proc_comm_lock, flags);
#ifdef CONFIG_LGE_BLUE_ERROR_HANDLER
	if (proc_comm_wait_for(base + MDM_STATUS, PCOM_READY))
		goto crash;
#else
again:
	if (proc_comm_wait_for(base + MDM_STATUS, PCOM_READY))
		goto again;
#endif
	writel(cmd, base + APP_COMMAND);
	writel(data1 ? *data1 : 0, base + APP_DATA1);
	writel(data2 ? *data2 : 0, base + APP_DATA2);

	/* Make sure the writes complete before notifying the other side */
	dsb();

	notify_other_proc_comm();

	if (proc_comm_wait_for(base + APP_COMMAND, PCOM_CMD_DONE))
#ifdef CONFIG_LGE_BLUE_ERROR_HANDLER
		goto crash;
#else
		goto again;
#endif

	if (readl(base + APP_STATUS) == PCOM_CMD_SUCCESS) {
		if (data1)
			*data1 = readl(base + APP_DATA1);
		if (data2)
			*data2 = readl(base + APP_DATA2);
		ret = 0;
	} else {
		ret = -EIO;
	}

	writel(PCOM_CMD_IDLE, base + APP_COMMAND);

	/* Make sure the writes complete before returning */
	dsb();

	spin_unlock_irqrestore(&proc_comm_lock, flags);
	return ret;
#ifdef CONFIG_LGE_BLUE_ERROR_HANDLER
/* LGE_CHANGE_S [bluerti@lge.com] 2009-07-06 <For Error Handler > */
crash:
	{
		extern char * error_modem_message ;
		extern int LG_ErrorHandler_enable;
	//	extern int get_status_hidden_reset();
		int ret;
		spin_unlock_irqrestore(&proc_comm_lock, flags);
	
		if (LG_ErrorHandler_enable) // check using proc_comm after arm9 crash 
			return 0;
		
	//	if(get_status_hidden_reset()==0 ) {
			ret = LGE_ErrorHandler_Main(MODEM_CRASH, error_modem_message);
			smsm_reset_modem(ret);
	//	} else {
	//		smsm_reset_modem(SMSM_SYSTEM_REBOOT);
	//	}
		while(1) 
			;
	}
/* LGE_CHANGE_E [bluerti@lge.com] 2009-07-06 <For Error Handler > */
#endif
}
EXPORT_SYMBOL(msm_proc_comm);
