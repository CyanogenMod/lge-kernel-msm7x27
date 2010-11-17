/*
 * android vibrator driver (msm7x27, Motor IC)
 *
 * Copyright (C) 2009 LGE, Inc.
 *
 * Author: Jinkyu Choi <jinkyu@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <linux/platform_device.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <mach/board_lge.h>
#include <mach/timed_output.h>

struct android_vibrator_platform_data *vibe_data = 0;
static atomic_t vibe_gain = ATOMIC_INIT(128); /* default max gain value is 128 */

struct timed_vibrator_data {
	struct timed_output_dev dev;
	struct hrtimer timer;
	spinlock_t lock;
	unsigned 	gpio;
	int 		max_timeout;
	u8 		active_low;
};

#if defined(CONFIG_VIB_USE_WORK_QUEUE)	/* copy form  C710 (Rev.D) */
static atomic_t nForce = ATOMIC_INIT(128);
struct work_struct vib_power_set_work_queue;
#endif	/* CONFIG_VIB_USE_WORK_QUEUE */

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
struct work_struct qcoin_overdrive_off_queue;
static bool use_overdrive = false;
static atomic_t vibe_rtime = ATOMIC_INIT(20);
static atomic_t vibe_ftime = ATOMIC_INIT(20);
#endif

static int android_vibrator_intialize(void)
{
#if 0
	/* Enable Vibraror LDO Power */
	if (vibe_data->power_set(1) < 0) {
		printk(KERN_ERR "%s power set failed\n", __FUNCTION__);
		return -1;
	}
#endif
	/* request gpio pin */
	if (vibe_data->gpio_request)
		if (vibe_data->gpio_request()) {
			printk(KERN_ERR"%s: gpio request failed\n", __func__);
			return -1;
		}

	/* Disable IC  */
	if (vibe_data->ic_enable_set(0) < 0) {
		printk(KERN_ERR "%s IC enable set failed\n", __FUNCTION__);
		return -1;
	}

	/* Initializ and disable PWM */
	if (vibe_data->pwm_set(0, 0) < 0) {
		printk(KERN_ERR "%s PWM set failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
static void vib_qcoin_off_work(struct work_struct *work) {
	int rtime = atomic_read(&vibe_rtime);
	vibe_data->pwm_set(1, -125);
	if (use_overdrive)
		msleep(rtime);
	else
		msleep(5);
	vibe_data->pwm_set(0, 0);
	vibe_data->power_set(0);
	vibe_data->ic_enable_set(0);
	//printk("LGE: %s disabled\n", __FUNCTION__);
}
#endif

static int android_vibrator_force_set(int nForce)
{
#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
	static bool enabled = false;
#endif
	/* Check the Force value with Max and Min force value */
	if (nForce > 128) nForce = 128;
	if (nForce < -128) nForce = -128;

	/* TODO: control the gain of vibrator */

	if (nForce == 0) {
#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
		if (!enabled)
			return 0;
		schedule_work(&qcoin_overdrive_off_queue);	
		enabled = false;
#else 
		vibe_data->power_set(0); /* should be checked for vibrator response time */
		vibe_data->pwm_set(0, nForce);
		vibe_data->ic_enable_set(0);
#endif
	} else {
#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
		/* 
		 * overdriving the output voltage 2.9V (after 10 msec) --> 2.0V 
		 * ignore the high amp gain from Android platform for avoiding the demage of Motor.
		 * the following value is optimized to C710 model.
		 * In case of other models, the output volatge and motor ic spec. should be checked
		*/
		int time = atomic_read(&vibe_ftime);
		vibe_data->pwm_set(1, 128);
		vibe_data->power_set(1);
		msleep(time);
#if 0
		if (nForce < 115)
			vibe_data->pwm_set(1, nForce);
		else
			vibe_data->pwm_set(1, 115);
#else
		vibe_data->pwm_set(1, nForce);
		enabled = true;
#endif

#else
		vibe_data->pwm_set(1, nForce);
		vibe_data->power_set(1); /* should be checked for vibrator response time */
#endif
		vibe_data->ic_enable_set(1);
	}
	return 0;
}

#if defined(CONFIG_VIB_USE_WORK_QUEUE)
static void vib_power_set_work(struct work_struct *work)
{
	android_vibrator_force_set(atomic_read(&nForce));
}
#endif	/* CONFIG_VIB_USE_WORK_QUEUE */

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
#if defined(CONFIG_VIB_USE_WORK_QUEUE)
	atomic_set(&nForce, 0);
	schedule_work(&vib_power_set_work_queue);	/* disable vibrator */
#else	/* CONFIG_VIB_USE_WORK_QUEUE */
	android_vibrator_force_set(0);
#endif

	return HRTIMER_NORESTART;
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	struct timed_vibrator_data *data = container_of(dev, struct timed_vibrator_data, dev);

	if (hrtimer_active(&data->timer)) {
		ktime_t r = hrtimer_get_remaining(&data->timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	//struct timed_vibrator_data *data = container_of(dev, struct timed_vibrator_data, dev);
	struct timed_vibrator_data *data = (void *)NULL;
	unsigned long	flags;
#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
	int rtime;
#endif
	//int gain = atomic_read(&vibe_gain);
	int gain;

	/* Add protection code for detect null point error */	
	if (dev == (void *)NULL) {
		printk(KERN_INFO "%s:dev is null\n",__FUNCTION__);
		return;
	}

	data = container_of(dev, struct timed_vibrator_data, dev);
	if (data == (void *)NULL) {
		printk(KERN_INFO "%s:data is null\n",__FUNCTION__);
		return;
	}

	gain = atomic_read(&vibe_gain);

	//printk("LGE:%s time = %d msec\n", __FUNCTION__, value);
	spin_lock_irqsave(&data->lock, flags);

	hrtimer_cancel(&data->timer);
	if (value > 0) {
		if (value > data->max_timeout)
			value = data->max_timeout;
#if defined(CONFIG_VIB_USE_WORK_QUEUE)
		atomic_set(&nForce, gain);
		schedule_work(&vib_power_set_work_queue);	/* disable vibrator */
#else	/* CONFIG_VIB_USE_WORK_QUEUE */
		android_vibrator_force_set(gain);
#endif

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
		rtime = atomic_read(&vibe_rtime);
		if (value >= (40+rtime)) {
			use_overdrive = true;
			value -= rtime;
		} else { /* the effect under 5msec will be ignored */
			use_overdrive = false;
			value -= 5;
		}
	/*	
		printk("LGE: Overdriving Enabled, rtime = %d, ftime = %d\n", 
				rtime, atomic_read(&vibe_ftime));
	*/
		
#endif

		hrtimer_start(&data->timer, ktime_set(value / 1000, (value % 1000) * 1000000), HRTIMER_MODE_REL);
	} else if (value == -100) {
#if defined(CONFIG_VIB_USE_WORK_QUEUE)
		atomic_set(&nForce, gain);
		schedule_work(&vib_power_set_work_queue);	/* disable vibrator */
#else	/* CONFIG_VIB_USE_WORK_QUEUE */
		android_vibrator_force_set(gain);
#endif
	} else {
#if defined(CONFIG_VIB_USE_WORK_QUEUE)
		atomic_set(&nForce, 0);
		schedule_work(&vib_power_set_work_queue);	/* disable vibrator */
#else	/* CONFIG_VIB_USE_WORK_QUEUE */
		android_vibrator_force_set(0);
#endif
	}

	spin_unlock_irqrestore(&data->lock, flags);
}

/* Interface for Android Platform */
struct timed_vibrator_data android_vibrator_data = {
	.dev.name = "vibrator",
	.dev.enable = vibrator_enable,
	.dev.get_time = vibrator_get_time,
	.max_timeout = 30000, /* max time for vibrator enable 30 sec. */
};

static ssize_t vibrator_amp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int gain = atomic_read(&vibe_gain);
    return sprintf(buf, "%d\n", gain);
}

static ssize_t vibrator_amp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int gain;

    sscanf(buf, "%d", &gain);
	if (gain > 128 || gain < -128) {
		printk(KERN_ERR "%s invalid value: should be -128 ~ +128\n", __FUNCTION__);
		return -EINVAL;
	}
    atomic_set(&vibe_gain, gain);

    return size;
}
static DEVICE_ATTR(amp, S_IRUGO | S_IWUSR, vibrator_amp_show, vibrator_amp_store);

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
static ssize_t vibrator_rtime_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int time = atomic_read(&vibe_rtime);
    return sprintf(buf, "%d\n", time);
}

static ssize_t vibrator_rtime_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int time;

    sscanf(buf, "%d", &time);
	if (time > 100 || time < 0) {
		printk(KERN_ERR "%s invalid value: should be 0 ~ 100 msec\n", __FUNCTION__);
		return -EINVAL;
	}
    atomic_set(&vibe_rtime, time);

    return size;
}
static DEVICE_ATTR(rtime, S_IRUGO | S_IWUSR, vibrator_rtime_show, vibrator_rtime_store);

static ssize_t vibrator_ftime_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int time = atomic_read(&vibe_ftime);
    return sprintf(buf, "%d\n", time);
}

static ssize_t vibrator_ftime_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int time;

    sscanf(buf, "%d", &time);
	if (time > 100 || time < 0) {
		printk(KERN_ERR "%s invalid value: should be 0 ~ 100 msec\n", __FUNCTION__);
		return -EINVAL;
	}
    atomic_set(&vibe_ftime, time);

    return size;
}
static DEVICE_ATTR(ftime, S_IRUGO | S_IWUSR, vibrator_ftime_show, vibrator_ftime_store);
#endif

static int android_vibrator_probe(struct platform_device *pdev)
{
	int ret = 0;

	vibe_data = (struct android_vibrator_platform_data *)pdev->dev.platform_data;
	atomic_set(&vibe_gain,vibe_data->amp_value);

	if (android_vibrator_intialize() < 0) {
		printk(KERN_ERR "Android Vibrator Initialization was failed\n");
		return -1;
	}

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
	INIT_WORK(&qcoin_overdrive_off_queue, vib_qcoin_off_work);
#endif

#if defined(CONFIG_VIB_USE_WORK_QUEUE)
	INIT_WORK(&vib_power_set_work_queue, vib_power_set_work);

	atomic_set(&nForce, 0);
	schedule_work(&vib_power_set_work_queue);	/* disable vibrator */
#else	/* CONFIG_VIB_USE_WORK_QUEUE */
	android_vibrator_force_set(0); /* disable vibrator */
#endif

	hrtimer_init(&android_vibrator_data.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	android_vibrator_data.timer.function = vibrator_timer_func;
	spin_lock_init(&android_vibrator_data.lock);

	ret = timed_output_dev_register(&android_vibrator_data.dev);
	if (ret < 0) {
		timed_output_dev_unregister(&android_vibrator_data.dev);
		return -ENODEV;
	}

#ifdef CONFIG_VIB_USE_HIGH_VOL_OVERDRIVE
	ret = device_create_file(android_vibrator_data.dev.dev, &dev_attr_rtime);
	ret = device_create_file(android_vibrator_data.dev.dev, &dev_attr_ftime);
#endif

	ret = device_create_file(android_vibrator_data.dev.dev, &dev_attr_amp);
	if (ret < 0) {
		timed_output_dev_unregister(&android_vibrator_data.dev);
		device_remove_file(android_vibrator_data.dev.dev, &dev_attr_amp);
		return -ENODEV;
	}

	printk(KERN_INFO "LGE: Android Vibrator Initialization was done\n");
	return 0;
}

static int android_vibrator_remove(struct platform_device *dev)
{
	vibe_data->power_set(0);
	vibe_data->ic_enable_set(0);
	vibe_data->pwm_set(0, 0);
	timed_output_dev_unregister(&android_vibrator_data.dev);

	return 0;
}

#ifdef CONFIG_PM
static int android_vibrator_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk(KERN_INFO "LGE: Android Vibrator Driver Suspend\n");
	android_vibrator_force_set(0);
	return 0;
}

static int android_vibrator_resume(struct platform_device *pdev)
{
	printk(KERN_INFO "LGE: Android Vibrator Driver Resume\n");
	android_vibrator_intialize();
	return 0;
}
#endif

static void android_vibrator_shutdown(struct platform_device *pdev)
{
	android_vibrator_force_set(0);
	vibe_data->power_set(0);
}

static struct platform_driver android_vibrator_driver = {
	.probe = android_vibrator_probe,
	.remove = android_vibrator_remove,
	.shutdown = android_vibrator_shutdown,
#ifdef CONFIG_PM
	.suspend = android_vibrator_suspend,
	.resume = android_vibrator_resume,
#else
	.suspend = NULL,
	.resume = NULL,
#endif
	.driver = {
		.name = "android-vibrator",
	},
};

static int __init android_vibrator_init(void)
{
	printk(KERN_INFO "LGE: Android Vibrator Driver Init\n");
	return platform_driver_register(&android_vibrator_driver);
}

static void __exit android_vibrator_exit(void)
{
	printk(KERN_INFO "LGE: Android Vibrator Driver Exit\n");
	platform_driver_unregister(&android_vibrator_driver);
}

module_init(android_vibrator_init);
module_exit(android_vibrator_exit);

MODULE_AUTHOR("LG Electronics Inc.");
MODULE_DESCRIPTION("Android Common Vibrator Driver");
MODULE_LICENSE("GPL");

