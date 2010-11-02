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
 *
 */
/*
 * Qualcomm MSM Sleep Stats Interface for Userspace
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/cpu.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/cpufreq.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "cpuidle.h"

struct sleep_data {
	int cpu;
	atomic_t idle_microsec;
	atomic_t busy_microsec;
	atomic_t timer_val_ms;
	atomic_t timer_expired;
	atomic_t policy_changed;
	struct hrtimer timer;
	struct attribute_group *attr_group;
	struct kobject *kobj;
	struct notifier_block nb;
	struct wait_to_notify *wait;
	struct work_struct work;
};

DEFINE_PER_CPU(struct sleep_data, core_sleep_info);

static void idle_enter(int cpu, unsigned int microsec)
{
	struct sleep_data *sleep_info = &per_cpu(core_sleep_info, cpu);

	if (!sleep_info || (sleep_info && (sleep_info->cpu < 0)))
		return;

	/* cumulative atomic counter, reset after reading */
	atomic_add(microsec, &sleep_info->busy_microsec);
	hrtimer_cancel(&sleep_info->timer);
}

static void idle_exit(int cpu, unsigned int microsec)
{
	struct sleep_data *sleep_info = &per_cpu(core_sleep_info, cpu);

	if (!sleep_info || (sleep_info && (sleep_info->cpu < 0)))
		return;

	/* cumulative atomic counter, reset after reading */
	atomic_add(microsec, &sleep_info->idle_microsec);
	if (atomic_read(&sleep_info->timer_val_ms) != INT_MAX &&
		atomic_read(&sleep_info->timer_val_ms))
		hrtimer_start(&sleep_info->timer,
			ktime_set(0,
			atomic_read(&sleep_info->timer_val_ms) * NSEC_PER_MSEC),
			HRTIMER_MODE_REL);
}

static void notify_uspace_work_fn(struct work_struct *work)
{
	struct sleep_data *sleep_info = container_of(work, struct sleep_data,
			work);

	/* Notify polling threads on change of value */
	sysfs_notify(sleep_info->kobj, NULL, "timer_expired");
}

static enum hrtimer_restart timer_func(struct hrtimer *handle)
{
	struct sleep_data *sleep_info = container_of(handle, struct sleep_data,
			timer);

	if (atomic_read(&sleep_info->timer_expired))
		pr_info("msm_sleep_stats: Missed timer interrupt on cpu %d\n",
				sleep_info->cpu);

	atomic_set(&sleep_info->timer_val_ms, 0);
	atomic_set(&sleep_info->timer_expired, 1);

	schedule_work_on(sleep_info->cpu, &sleep_info->work);

	return HRTIMER_NORESTART;
}

static ssize_t show_idle_ms(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	val = atomic_read(&sleep_info->idle_microsec);
	atomic_sub(val, &sleep_info->idle_microsec);

	return sprintf(buf, "%d\n", val/1000);
}

static ssize_t show_busy_ms(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	val = atomic_read(&sleep_info->busy_microsec);
	atomic_sub(val, &sleep_info->busy_microsec);

	return sprintf(buf, "%d\n", val/1000);
}

static ssize_t show_timer_val_ms(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	val = atomic_read(&sleep_info->timer_val_ms);
	atomic_sub(val, &sleep_info->timer_val_ms);

	return sprintf(buf, "%d\n", val);
}

static ssize_t store_timer_val_ms(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	hrtimer_cancel(&sleep_info->timer);
	sscanf(buf, "%du", &val);
	atomic_set(&sleep_info->timer_val_ms, val);
	if (atomic_read(&sleep_info->timer_val_ms) != INT_MAX &&
		atomic_read(&sleep_info->timer_val_ms))
		hrtimer_start(&sleep_info->timer,
			ktime_set(0,
			atomic_read(&sleep_info->timer_val_ms) * NSEC_PER_MSEC),
			HRTIMER_MODE_REL);

	return count;
}
static ssize_t show_timer_expired(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	val = atomic_read(&sleep_info->timer_expired);
	atomic_set(&sleep_info->timer_expired, 0);

	return sprintf(buf, "%d\n", val);
}

static ssize_t show_policy_changed(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	int val = 0;
	int cpu = 0;
	struct sleep_data *sleep_info = NULL;

	sscanf(kobj->parent->name, "cpu%d", &cpu);
	sleep_info = &per_cpu(core_sleep_info, cpu);
	val = atomic_read(&sleep_info->policy_changed);
	atomic_set(&sleep_info->policy_changed, 0);

	return sprintf(buf, "%d\n", val);
}

static int policy_change_notifier(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct sleep_data *sleep_info = container_of(nb, struct sleep_data, nb);
	struct cpufreq_policy *policy = (struct cpufreq_policy *)data;

	if (event == CPUFREQ_ADJUST && sleep_info->cpu == policy->cpu) {
		atomic_set(&sleep_info->policy_changed, 1);
		sysfs_notify(sleep_info->kobj, NULL, "policy_changed");
	}

	return 0;
}

static int add_sysfs_objects(struct sleep_data *sleep_info)
{
	int err = 0;

	struct kobj_attribute *idle_attrib = NULL;
	struct kobj_attribute *busy_attrib = NULL;
	struct kobj_attribute *timer_val_attrib = NULL;
	struct kobj_attribute *timer_exp_attrib = NULL;
	struct kobj_attribute *policy_chg_attrib = NULL;
	struct attribute **attribs = NULL;

	atomic_set(&sleep_info->idle_microsec, 0);
	atomic_set(&sleep_info->busy_microsec, 0);
	atomic_set(&sleep_info->timer_expired, 0);
	atomic_set(&sleep_info->policy_changed, 0);
	atomic_set(&sleep_info->timer_val_ms, INT_MAX);

	idle_attrib = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!idle_attrib)
		goto rel;
	idle_attrib->attr.name = "idle_ms";
	idle_attrib->attr.mode = 0444;
	idle_attrib->show = show_idle_ms;
	idle_attrib->store = NULL;

	busy_attrib = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!busy_attrib)
		goto rel;
	busy_attrib->attr.name = "busy_ms";
	busy_attrib->attr.mode = 0444;
	busy_attrib->show = show_busy_ms;
	busy_attrib->store = NULL;

	timer_val_attrib = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!timer_val_attrib)
		goto rel;
	timer_val_attrib->attr.name = "timer_val_ms";
	timer_val_attrib->attr.mode = 0666;
	timer_val_attrib->show = show_timer_val_ms;
	timer_val_attrib->store = store_timer_val_ms;

	/* pollable attributes */
	timer_exp_attrib = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!timer_exp_attrib)
		goto rel;
	timer_exp_attrib->attr.name = "timer_expired";
	timer_exp_attrib->attr.mode = 0444;
	timer_exp_attrib->show = show_timer_expired;
	timer_exp_attrib->store = NULL;

	policy_chg_attrib = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
	if (!policy_chg_attrib)
		goto rel;
	policy_chg_attrib->attr.name = "policy_changed";
	policy_chg_attrib->attr.mode = 0444;
	policy_chg_attrib->show = show_policy_changed;
	policy_chg_attrib->store = NULL;

	attribs = kzalloc(sizeof(struct attribute *) * 6, GFP_KERNEL);
	if (!attribs)
		goto rel;
	attribs[0] = &idle_attrib->attr;
	attribs[1] = &busy_attrib->attr;
	attribs[2] = &timer_val_attrib->attr;
	attribs[3] = &timer_exp_attrib->attr;
	attribs[4] = &policy_chg_attrib->attr;
	attribs[5] = NULL;

	sleep_info->attr_group = kzalloc(sizeof(struct attribute_group),
			GFP_KERNEL);
	if (!sleep_info->attr_group)
		goto rel;
	sleep_info->attr_group->attrs = attribs;

	/* Create /sys/devices/system/cpu/cpuX/sleep-stats/... */
	sleep_info->kobj = kobject_create_and_add("sleep-stats",
			&get_cpu_sysdev(sleep_info->cpu)->kobj);
	if (!sleep_info->kobj)
		goto rel;

	err = sysfs_create_group(sleep_info->kobj, sleep_info->attr_group);
	if (err)
		kobject_put(sleep_info->kobj);
	else
		kobject_uevent(sleep_info->kobj, KOBJ_ADD);

	if (!err)
		return err;

rel:
	kfree(idle_attrib);
	kfree(busy_attrib);
	kfree(timer_val_attrib);
	kfree(timer_exp_attrib);
	kfree(policy_chg_attrib);
	kfree(attribs);
	kfree(sleep_info->attr_group);
	kfree(sleep_info->kobj);

	return -ENOMEM;
}

static void remove_sysfs_objects(struct sleep_data *sleep_info)
{
	if (sleep_info->kobj)
		sysfs_remove_group(sleep_info->kobj, sleep_info->attr_group);

	kfree(sleep_info->attr_group);
	kfree(sleep_info->kobj);
}

static int __init msm_sleep_info_init(void)
{
	int err = 0;
	int cpu;
	struct sleep_data *sleep_info = NULL;

	/* Register callback from idle for all cpus */
	msm_idle_register_cb(idle_enter, idle_exit);

	for_each_possible_cpu(cpu) {
		printk(KERN_INFO "msm_sleep_stats: Initializing sleep stats "
				"for CPU[%d]\n", cpu);
		sleep_info = &per_cpu(core_sleep_info, cpu);
		sleep_info->cpu = cpu;
		INIT_WORK(&sleep_info->work, notify_uspace_work_fn);

		/* Initialize high resolution timer */
		hrtimer_init(&sleep_info->timer, CLOCK_MONOTONIC,
				HRTIMER_MODE_REL);
		sleep_info->timer.function = timer_func;

		/* Register for cpufreq policy changes */
		sleep_info->nb.notifier_call = policy_change_notifier;
		err = cpufreq_register_notifier(&sleep_info->nb,
					CPUFREQ_POLICY_NOTIFIER);
		if (err)
			goto cleanup;

		/* Create sysfs object */
		err = add_sysfs_objects(sleep_info);
		if (err)
			goto cleanup;

		continue;
cleanup:
		printk(KERN_INFO "msm_sleep_stats: Failed to initialize sleep "
				"stats for CPU[%d]\n", cpu);
		sleep_info->cpu = -1;
		cpufreq_unregister_notifier(&sleep_info->nb,
				CPUFREQ_POLICY_NOTIFIER);
		remove_sysfs_objects(sleep_info);
	}

	return 0;
}
late_initcall(msm_sleep_info_init);

