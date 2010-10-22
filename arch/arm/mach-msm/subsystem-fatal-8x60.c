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
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#include <mach/irqs.h>

#include "smd_private.h"
#include "modem_notifier.h"

#define MODEM_HWIO_MSS_RESET_ADDR       0x00902C48

static void modem_fatal_fn(struct work_struct *);
static irqreturn_t modem_wdog_bite_irq(int irq, void *dev_id);
static void check_modem_ahb_status(struct work_struct *work);
static int modem_notif_handler(struct notifier_block *this,
				unsigned long code,
				void *_cmd);

static DECLARE_WORK(modem_fatal_work, modem_fatal_fn);
static DECLARE_DELAYED_WORK(check_modem_ahb_work, check_modem_ahb_status);

static atomic_t modem_ahb_lockup = ATOMIC_INIT(0);
static struct notifier_block modem_notif_nb = {
	.notifier_call = modem_notif_handler,
};

static void do_soc_restart(void)
{
	printk(KERN_CRIT "subsys-restart: Rebooting SoC..\n");
	lock_kernel();
	kernel_restart(NULL);
	unlock_kernel();
}

static void check_modem_ahb_status(struct work_struct *work)
{
	if (atomic_read(&modem_ahb_lockup)) {

		disable_irq_nosync(MARM_WDOG_EXPIRED);
		panic("subsys-restart: Timeout waiting for modem to reset.\n");
	}
}

static void modem_fatal_fn(struct work_struct *work)
{
	uint32_t modem_state;

	printk(KERN_CRIT "Watchdog bite received from modem!\n");

	modem_state = smsm_get_state(SMSM_MODEM_STATE);

	if (modem_state == 0) {

		panic("subsys-restart: Unable to detect modem smsm state!");

	} else if (modem_state & SMSM_RESET) {

		panic("subsys-restart: Modem in reset mode!");

	} else if (modem_state & SMSM_SYSTEM_DOWNLOAD) {

		panic("subsys-restart: Modem in download mode!");

	} else if (modem_state & SMSM_SYSTEM_REBOOT_USR) {

		printk(KERN_CRIT
			"subsys-restart: User invoked system reboot.\n");
		do_soc_restart();

	} else if (modem_state & SMSM_SYSTEM_PWRDWN_USR) {

		printk(KERN_CRIT
			"subsys-restart: User invoked system powerdown.\n");
		do_soc_restart();
	} else {

		int ret;
		void *hwio_modem_reset_addr =
				ioremap_nocache(MODEM_HWIO_MSS_RESET_ADDR, 8);

		modem_register_notifier(&modem_notif_nb);
		atomic_set(&modem_ahb_lockup, 1);

		printk(KERN_CRIT "subsys-restart: Modem AHB locked up.\n");
		printk(KERN_CRIT "subsys-restart: Trying to free up modem!\n");

		writel(0x3, hwio_modem_reset_addr);

		ret = schedule_delayed_work(&check_modem_ahb_work,
					msecs_to_jiffies(1000));

		enable_irq(MARM_WDOG_EXPIRED);
		iounmap(hwio_modem_reset_addr);
	}
}

static irqreturn_t modem_wdog_bite_irq(int irq, void *dev_id)
{
	int ret;

	/*
	* Do not process modem watchdog bites after an attempt
	* has been made to bring the modem out of ahb lockup. Wait
	* until the modem comes back up and enters its error handler.
	*/
	if (atomic_read(&modem_ahb_lockup))
		return IRQ_HANDLED;

	ret = schedule_work(&modem_fatal_work);
	disable_irq_nosync(MARM_WDOG_EXPIRED);

	return IRQ_HANDLED;
}

static void __exit subsystem_fatal_exit(void)
{
	free_irq(MARM_WDOG_EXPIRED, NULL);
}

static int __init subsystem_fatal_init(void)
{
	int ret;

	ret = request_irq(MARM_WDOG_EXPIRED, modem_wdog_bite_irq,
			IRQF_TRIGGER_RISING, "modem_wdog", NULL);

	return ret;
}

static int modem_notif_handler(struct notifier_block *this,
				unsigned long code,
				void *_cmd)
{
	if (code == MODEM_NOTIFIER_START_RESET) {
		atomic_set(&modem_ahb_lockup, 0);
		modem_unregister_notifier(&modem_notif_nb);
	}

	return NOTIFY_DONE;
}

module_init(subsystem_fatal_init);
module_exit(subsystem_fatal_exit);
