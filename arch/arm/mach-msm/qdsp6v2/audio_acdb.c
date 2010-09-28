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
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/android_pmem.h>
#include "audio_acdb.h"


#define MAX_NETWORKS		9
#define NUM_ACTIVE_NETWORKS	6
#define VOCPROC_STREAM_OFFSET	NUM_ACTIVE_NETWORKS
#define VOCPROC_VOL_OFFSET	(NUM_ACTIVE_NETWORKS * 2)
#define NUM_VOCPROC_CAL_TYPES	(NUM_ACTIVE_NETWORKS * 3)
#define NUM_AUDPROC_CAL_TYPES	3

struct acdb_data {
	struct mutex		acdb_mutex;
	struct acdb_cal_block	vocproc_cal[MAX_NETWORKS];
	struct acdb_cal_block	vocproc_stream_cal[MAX_NETWORKS];
	struct acdb_cal_block	vocproc_vol_cal[MAX_NETWORKS];
	uint32_t		vocproc_cal_size;
	uint32_t		vocproc_stream_cal_size;
	uint32_t		vocproc_vol_cal_size;

	/* PMEM information */
	int			pmem_fd;
	unsigned long		paddr;
	unsigned long		kvaddr;
	unsigned long		pmem_len;
	struct file		*file;

};

static struct acdb_data		acdb_data;



void store_vocproc_cal(uint32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
			__func__, cal_blocks[i].cal_offset, acdb_data.pmem_len);
			acdb_data.vocproc_cal[i].cal_size = 0;
		} else {
			acdb_data.vocproc_cal[i].cal_size =
						cal_blocks[i].cal_size;
			acdb_data.vocproc_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset + acdb_data.kvaddr;
		}
	}
	acdb_data.vocproc_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
}

void get_vocproc_cal(struct acdb_cal_table *cal_table)
{
	pr_debug("%s\n", __func__);

	if (cal_table == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_table->idx_table_size = acdb_data.vocproc_cal_size;
	cal_table->idx_table = &acdb_data.vocproc_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_vocproc_stream_cal(uint32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
			__func__, cal_blocks[i].cal_offset, acdb_data.pmem_len);
			acdb_data.vocproc_stream_cal[i].cal_size = 0;
		} else {
			acdb_data.vocproc_stream_cal[i].cal_size =
						cal_blocks[i].cal_size;
			acdb_data.vocproc_stream_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset + acdb_data.kvaddr;
		}
	}
	acdb_data.vocproc_stream_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
}

void get_vocproc_stream_cal(struct acdb_cal_table *cal_table)
{
	pr_debug("%s\n", __func__);

	if (cal_table == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_table->idx_table_size = acdb_data.vocproc_stream_cal_size;
	cal_table->idx_table = &acdb_data.vocproc_stream_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

void store_vocproc_vol_cal(uint32_t len, struct cal_block *cal_blocks)
{
	int i;
	pr_debug("%s\n", __func__);

	mutex_lock(&acdb_data.acdb_mutex);

	for (i = 0; i < len; i++) {
		if (cal_blocks[i].cal_offset > acdb_data.pmem_len) {
			pr_err("%s: offset %d is > pmem_len %ld\n",
				__func__, cal_blocks[i].cal_offset,
				acdb_data.pmem_len);
			acdb_data.vocproc_vol_cal[i].cal_size = 0;
		} else {
			acdb_data.vocproc_vol_cal[i].cal_size =
						cal_blocks[i].cal_size;
			acdb_data.vocproc_vol_cal[i].cal_kvaddr =
				cal_blocks[i].cal_offset + acdb_data.kvaddr;
		}
	}
	acdb_data.vocproc_vol_cal_size = len;
	mutex_unlock(&acdb_data.acdb_mutex);
}

void get_vocproc_vol_cal(struct acdb_cal_table *cal_table)
{
	pr_debug("%s\n", __func__);

	if (cal_table == NULL) {
		pr_err("ACDB=> NULL pointer sent to %s\n", __func__);
		goto done;
	}

	mutex_lock(&acdb_data.acdb_mutex);

	cal_table->idx_table_size = acdb_data.vocproc_vol_cal_size;
	cal_table->idx_table = &acdb_data.vocproc_vol_cal[0];

	mutex_unlock(&acdb_data.acdb_mutex);
done:
	return;
}

static int acdb_open(struct inode *inode, struct file *f)
{
	s32 result = 0;
	pr_debug("%s\n", __func__);

	return result;
}

static int acdb_ioctl(struct inode *inode, struct file *f,
		unsigned int cmd, unsigned long arg)
{
	s32 result = 0;
	struct cal_block data[MAX_NETWORKS];
	uint32_t len;
	pr_debug("%s\n", __func__);

	switch (cmd) {
	case AUDIO_REGISTER_PMEM: {
		struct msm_audio_pmem_info info;
		pr_debug("AUDIO_REGISTER_PMEM\n");
		if (copy_from_user(&info, (void *) arg, sizeof(info)))
			return -EFAULT;

		result = get_pmem_file(info.fd, &acdb_data.paddr,
					&acdb_data.kvaddr, &acdb_data.pmem_len,
					&acdb_data.file);
		if (result == 0)
			acdb_data.pmem_fd = info.fd;

		return result;
	}
	case AUDIO_DEREGISTER_PMEM:
		if (acdb_data.pmem_fd)
			put_pmem_file(acdb_data.file);
		return result;
	}

	if (copy_from_user(&len, (void *) arg,
			sizeof(len)) || (len > MAX_NETWORKS))
		return -EFAULT;

	if (copy_from_user(data, (void *) (arg+sizeof(len)),
				len * sizeof(struct cal_block))) {
		pr_err("%s: fail to copy table size %d\n", __func__, len);
		return -EFAULT;
	}

	switch (cmd) {
	case AUDIO_SET_AUDPROC_CAL:
		break;
	case AUDIO_GET_AUDPROC_CAL:
		break;
	case AUDIO_SET_AUDPROC_STREAM_CAL:
		break;
	case AUDIO_GET_AUDPROC_STREAM_CAL:
		break;
	case AUDIO_SET_AUDPROC_VOL_CAL:
		break;
	case AUDIO_GET_AUDPROC_VOL_CAL:
		break;
	case AUDIO_SET_VOCPROC_CAL:
		store_vocproc_cal(len, data);
		break;
	case AUDIO_GET_VOCPROC_CAL:
		break;
	case AUDIO_SET_VOCPROC_STREAM_CAL:
		store_vocproc_stream_cal(len, data);
		break;
	case AUDIO_GET_VOCPROC_STREAM_CAL:
		break;
	case AUDIO_SET_VOCPROC_VOL_CAL:
		store_vocproc_vol_cal(len, data);
		break;
	case AUDIO_GET_VOCPROC_VOL_CAL:
		break;
	default:
		pr_err("ACDB=> ACDB ioctl not found!\n");
	}

	return result;
}

static int acdb_release(struct inode *inode, struct file *f)
{
	s32 result = 0;
	pr_debug("%s\n", __func__);
	return result;
}

static const struct file_operations acdb_fops = {
	.owner = THIS_MODULE,
	.open = acdb_open,
	.release = acdb_release,
	.ioctl = acdb_ioctl,
};

struct miscdevice acdb_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "msm_acdb",
	.fops	= &acdb_fops,
};

static int __init acdb_init(void)
{
	pr_debug("%s\n", __func__);
	mutex_init(&acdb_data.acdb_mutex);
	return misc_register(&acdb_misc);
}

module_init(acdb_init);

MODULE_DESCRIPTION("MSM 8x60 Audio ACDB driver");
MODULE_LICENSE("GPL v2");
