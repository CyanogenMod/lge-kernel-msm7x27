/*
 * arch/arm/mach-msm/lge/lge_tty_atcmd.c
 *
 * Copyright (C) 2010 LGE, Inc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/wakelock.h>

#define MAX_ATCMD_TTYS	2
#define BACK_END	0
#define FRONT_END	1
#define BUF_SIZE	SZ_4K

struct buf_fifo {
	char *buf_addr;
	int size;
	int head;
	int tail;
	int count;
	int fifo_mask;
	struct mutex *lock;
};

struct atcmd_tty_info {
	struct tty_struct *tty;
	struct atcmd_tty_info *other_tty;

	struct wake_lock wake_lock;
	struct mutex *lock;

	const char *name;
	int line_status;
	struct buf_fifo *write_buffer;
	struct buf_fifo *read_buffer;
	int open_count;

	work_func_t work_fn;
	struct delayed_work work;
};

static struct atcmd_tty_info atcmd_tty[MAX_ATCMD_TTYS];
static struct tty_driver *atcmd_tty_driver;

static struct buf_fifo down_stream_buffer;
static struct buf_fifo up_stream_buffer;

/* Shared line status between frond and back end atcmd-tty,
 * and global mutex for synchronization.
 * 2011-05-13, hyunhui.park@lge.com
 */
static int shared_line_status;
static struct mutex atcmd_status_lock;

static DEFINE_MUTEX(atcmd_fe_tty_lock);	/* front end tty mutex */
static DEFINE_MUTEX(atcmd_be_tty_lock); /* back end tty mutex */
static DEFINE_MUTEX(atcmd_us_buf_lock); /* up stream buf mutex */
static DEFINE_MUTEX(atcmd_ds_buf_lock);	/* down stream buf mutex */

/* return free space of ring buffer */
static int get_free_space(struct buf_fifo *ring_buf)
{
	int head, tail, mask, size, count;
	int remain;

	head = ring_buf->head;
	tail = ring_buf->tail;
	mask = ring_buf->fifo_mask;
	size = ring_buf->size;
	count = ring_buf->count;

	remain = size - ((head - tail) & mask);

	/* if head and tail are in same position
	 * and write count is eqaul to buffer size,
	 * it means that ring buffer is full
	 */
	if ((remain == size) && (count == size))
		remain = 0;

	return remain;
}

static void atcmd_tty_read(struct work_struct *work)
{
	struct atcmd_tty_info *info =
		container_of(work, struct atcmd_tty_info, work.work);
	struct buf_fifo *read_buffer;
	struct tty_struct *tty = info->tty;
	unsigned char *ptr;
	int n_read;
	int avail, remain, buf_size;
	char *read_addr;

	read_buffer = info->read_buffer;

	mutex_lock(read_buffer->lock);

	read_addr = read_buffer->buf_addr + read_buffer->tail;
	buf_size = read_buffer->size;


	if (!tty) {
		mutex_unlock(read_buffer->lock);
		return;
	}

	for (;;) {
		if (test_bit(TTY_THROTTLED, &tty->flags))
			break;

		n_read = buf_size - get_free_space(read_buffer);
		if (n_read == 0)
			break;

		avail = tty_prepare_flip_string(tty, &ptr, n_read);
		if (avail <= 0) {
			schedule_delayed_work(&info->work, (30 / 1000) * HZ);
			mutex_unlock(read_buffer->lock);
			return;
		}

		if (read_buffer->head > read_buffer->tail) {
			memcpy(ptr, read_addr, avail);
			read_buffer->tail += avail;
		} else {
			int read_len = avail;
			remain = buf_size - read_buffer->tail;
			if (remain < avail) {
				memcpy(ptr, read_addr, remain);
				ptr += remain;
				read_len -= remain;
				read_buffer->tail = 0;
				read_addr = read_buffer->buf_addr;
			}
			memcpy(ptr, read_addr, read_len);
			read_buffer->tail += read_len;
			read_buffer->tail %= buf_size;
		}
		read_buffer->count -= avail;

		wake_lock_timeout(&info->wake_lock, HZ / 2);
		tty_flip_buffer_push(tty);
	}

	mutex_unlock(read_buffer->lock);

	if (info->other_tty->tty)
		tty_wakeup(info->other_tty->tty);

	return;
}

static int atcmd_tty_write(struct tty_struct *tty,
		const unsigned char *buf, int len)
{
	struct atcmd_tty_info *info = tty->driver_data;
	struct atcmd_tty_info *other_tty_info;
	struct buf_fifo *write_buffer;
	int avail, remain, buf_size;
	char *write_addr;

	other_tty_info = info->other_tty;
	write_buffer = info->write_buffer;

	mutex_lock(write_buffer->lock);

	write_addr = write_buffer->buf_addr + write_buffer->head;
	buf_size = write_buffer->size;

	/* if we're writing to tty, we will
	 * never be able to write more data
	 * than there is currently space for
	 */
	avail = get_free_space(write_buffer);
	if (avail == 0) {
		mutex_unlock(write_buffer->lock);
		return 0;
	}

	if (len > avail)
		len = avail;

	/* handle ring buffer operation */
	if (write_buffer->head < write_buffer->tail) {
		memcpy(write_addr, buf, len);
		write_buffer->head += len;
	} else {
		int write_len = len;
		remain = buf_size - write_buffer->head;
		if (remain < len) {
			memcpy(write_addr, buf, remain);
			buf += remain;
			write_len -= remain;
			write_buffer->head = 0;
			write_addr = write_buffer->buf_addr;
		}
		memcpy(write_addr, buf, write_len);
		write_buffer->head += write_len;
		write_buffer->head %= buf_size;
	}
	write_buffer->count += len;

	wake_lock_timeout(&info->wake_lock, HZ / 2);

	/* invoke other tty's read operation */
	if (other_tty_info->open_count)
		schedule_delayed_work(&other_tty_info->work, 0);

	mutex_unlock(write_buffer->lock);

	return len;
}

static int atcmd_tty_write_room(struct tty_struct *tty)
{
	struct atcmd_tty_info *info = tty->driver_data;
	struct buf_fifo *write_buffer;
	int avail;

	write_buffer = info->write_buffer;

	mutex_lock(write_buffer->lock);
	avail = get_free_space(write_buffer);
	mutex_unlock(write_buffer->lock);

	return avail;
}

static int atcmd_tty_chars_in_buffer(struct tty_struct *tty)
{
	struct atcmd_tty_info *info = tty->driver_data;
	struct buf_fifo *read_buffer;
	int n_read, buf_size;

	read_buffer = info->read_buffer;
	buf_size = read_buffer->size;

	mutex_lock(read_buffer->lock);
	n_read = buf_size - get_free_space(read_buffer);
	mutex_unlock(read_buffer->lock);

	return n_read;
}

static int atcmd_tty_tiocmget(struct tty_struct *tty, struct file *file)
{
#if 0
	struct atcmd_tty_info *info = tty->driver_data;
	struct atcmd_tty_info *other_info = info->other_tty;
#endif
	int status = 0;
	int result = 0;

#if 0
	mutex_lock(info->lock);
#else
	mutex_lock(&atcmd_status_lock);
#endif

	status = shared_line_status;

	result = ((status & TIOCM_DTR)  ? TIOCM_DTR  : 0) |  /* DTR is set */
		((status & TIOCM_RTS)  ? TIOCM_RTS  : 0) |  /* RTS is set */
		((status & TIOCM_LOOP) ? TIOCM_LOOP : 0) |  /* LOOP is set */
		((status & TIOCM_CTS)  ? TIOCM_CTS  : 0) |  /* CTS is set */
		((status & TIOCM_CD)   ? TIOCM_CD  : 0) |  /* Carrier detect is set*/
		((status & TIOCM_RI)   ? TIOCM_RI   : 0) |  /* Ring Indicator is set */
		((status & TIOCM_DSR)  ? TIOCM_DSR  : 0);   /* DSR is set */

#if 0
	mutex_unlock(info->lock);
#else
	mutex_unlock(&atcmd_status_lock);
#endif

	return status;
}

static int atcmd_tty_tiocmset(struct tty_struct *tty, struct file *file,
		unsigned int set, unsigned int clear)
{
#if 0
	struct atcmd_tty_info *info = tty->driver_data;
	struct atcmd_tty_info *other_info = info->other_tty;
#endif

#if 0
	mutex_lock(info->lock);
#else
	mutex_lock(&atcmd_status_lock);
#endif

	if (set & TIOCM_RI)
		shared_line_status |= TIOCM_RI;

	if (clear & TIOCM_RI)
		shared_line_status &= ~TIOCM_RI;

	if (set & TIOCM_CD)
		shared_line_status |= TIOCM_CD;

	if (clear & TIOCM_CD)
		shared_line_status &= ~TIOCM_CD;

	/* DTR, RTS, CTS bit set/clear */
	if (set & TIOCM_DTR)
		shared_line_status |= TIOCM_DTR;

	if (clear & TIOCM_DTR)
		shared_line_status &= ~TIOCM_DTR;

	if (set & TIOCM_RTS)
		shared_line_status |= TIOCM_RTS;

	if (clear & TIOCM_RTS)
		shared_line_status &= ~TIOCM_RTS;

	if (set & TIOCM_CTS)
		shared_line_status |= TIOCM_CTS;

	if (clear & TIOCM_CTS)
		shared_line_status &= ~TIOCM_CTS;

	barrier();

#if 0
	mutex_unlock(info->lock);
#else
	mutex_unlock(&atcmd_status_lock);
#endif

	return 0;
}

static void atcmd_tty_unthrottle(struct tty_struct *tty)
{
	struct atcmd_tty_info *info = tty->driver_data;

	schedule_delayed_work(&info->work, 0);

	return;
}

static int atcmd_tty_open(struct tty_struct *tty, struct file *f)
{
	int res = 0;
	int n = tty->index;
	struct atcmd_tty_info *info;

	info = atcmd_tty + n;
	if (n == BACK_END) {
		info->lock = &atcmd_be_tty_lock;
		info->name = "atcmd-back-end-tty";
		info->work_fn = atcmd_tty_read;
		info->other_tty = atcmd_tty + 1;
		info->write_buffer = &up_stream_buffer;
		info->read_buffer = &down_stream_buffer;
	} else if (n == FRONT_END) {
		info->lock = &atcmd_fe_tty_lock;
		info->name = "atcmd-front-end-tty";
		info->work_fn = atcmd_tty_read;
		info->other_tty = atcmd_tty;
		info->write_buffer = &down_stream_buffer;
		info->read_buffer = &up_stream_buffer;
	}

	mutex_lock(info->lock);
	tty->driver_data = info;

	if (info->open_count++ == 0) {
		info->tty = tty;
		INIT_DELAYED_WORK(&info->work, info->work_fn); 
		wake_lock_init(&info->wake_lock, WAKE_LOCK_SUSPEND, info->name);
		if (info->other_tty->open_count) {
			info->line_status |= TIOCM_DTR | TIOCM_RTS;
			info->other_tty->line_status |= TIOCM_DTR | TIOCM_RTS;
			shared_line_status = info->other_tty->line_status;
			if (info->read_buffer->count)
				schedule_delayed_work(&info->work, 0);
		}
	}

	mutex_unlock(info->lock);

	return res;
}

static void atcmd_tty_close(struct tty_struct *tty, struct file *f)
{
	struct atcmd_tty_info *info = tty->driver_data;

	if (info == 0)
		return;

	mutex_lock(info->lock);
	if (--info->open_count == 0) {
		info->tty = 0;
		tty->driver_data = 0;
		cancel_delayed_work_sync(&info->work);
		wake_lock_destroy(&info->wake_lock);
		info->other_tty->line_status &= ~(TIOCM_DTR | TIOCM_RTS);
	}
	mutex_unlock(info->lock);

	return;
}

static struct tty_operations atcmd_tty_ops = {
	.open = atcmd_tty_open,
	.close = atcmd_tty_close,
	.write = atcmd_tty_write,
	.write_room = atcmd_tty_write_room,
	.chars_in_buffer = atcmd_tty_chars_in_buffer,
	.unthrottle = atcmd_tty_unthrottle,
	.tiocmget = atcmd_tty_tiocmget,
	.tiocmset = atcmd_tty_tiocmset,
};

static int __init lge_tty_atcmd_init(void)
{
	int ret;

	printk(KERN_INFO"%s: initialize atcmd-ttys\n", __func__);

	/* allocate and initialize downstream buffers */
	down_stream_buffer.buf_addr = kmalloc(BUF_SIZE, GFP_KERNEL);
	if (down_stream_buffer.buf_addr == NULL)
		goto down_stream_buf_alloc_error;
	down_stream_buffer.head = 0;
	down_stream_buffer.tail = 0;
	down_stream_buffer.fifo_mask = BUF_SIZE - 1;
	down_stream_buffer.size = BUF_SIZE;
	down_stream_buffer.count = 0;
	down_stream_buffer.lock = &atcmd_ds_buf_lock;

	/* allocate and initialize upstream buffers */
	up_stream_buffer.buf_addr = kmalloc(BUF_SIZE, GFP_KERNEL);
	if (up_stream_buffer.buf_addr == NULL)
		goto up_stream_buf_alloc_error;
	up_stream_buffer.head = 0;
	up_stream_buffer.tail = 0;
	up_stream_buffer.fifo_mask = BUF_SIZE - 1;
	up_stream_buffer.size = BUF_SIZE;
	up_stream_buffer.count = 0;
	up_stream_buffer.lock = &atcmd_us_buf_lock;

	/* allocate tty driver */
	atcmd_tty_driver = alloc_tty_driver(MAX_ATCMD_TTYS);
	if (atcmd_tty_driver == 0)
		goto tty_alloc_error;

	/* setting tty */
	atcmd_tty_driver->owner = THIS_MODULE;
	atcmd_tty_driver->driver_name = "atcmd-tty-driver";
	atcmd_tty_driver->name = "atcmd-tty";
	atcmd_tty_driver->major = 0;
	atcmd_tty_driver->minor_start = 0;
	atcmd_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	atcmd_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	atcmd_tty_driver->init_termios = tty_std_termios;
	atcmd_tty_driver->init_termios.c_iflag = 0;
	atcmd_tty_driver->init_termios.c_oflag = 0;
	atcmd_tty_driver->init_termios.c_cflag = B38400 | CS8 | CREAD;
	atcmd_tty_driver->init_termios.c_lflag = 0;
	atcmd_tty_driver->flags = TTY_DRIVER_RESET_TERMIOS |
		TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_set_operations(atcmd_tty_driver, &atcmd_tty_ops);
	ret = tty_register_driver(atcmd_tty_driver);
	if (ret)
		return ret;

	/* this should be dynamic */
	tty_register_device(atcmd_tty_driver, 0, 0);
	tty_register_device(atcmd_tty_driver, 1, 0);

	/* init line status mutex */
	mutex_init(&atcmd_status_lock);

	return 0;

tty_alloc_error:
	kfree(up_stream_buffer.buf_addr);
up_stream_buf_alloc_error:
	kfree(down_stream_buffer.buf_addr);
down_stream_buf_alloc_error:
	return -ENOMEM;
}

static void __exit lge_tty_atcmd_exit(void)
{
	return;
}

module_init(lge_tty_atcmd_init);
module_exit(lge_tty_atcmd_exit);

MODULE_DESCRIPTION("LGE tty for bypassing atcmd stream");
MODULE_AUTHOR("SungEun Kim <cleaneye.kim@lge.com>");
MODULE_LICENSE("GPL");
