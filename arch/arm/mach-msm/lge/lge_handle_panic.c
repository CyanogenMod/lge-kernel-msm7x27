/* 
 * arch/arm/mach-msm/lge/lge_handle_panic.c
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

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/console.h>
#include <linux/kbd_kern.h>
#include <linux/vt_kern.h>
#include <linux/fb.h>
#include <linux/reboot.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <asm/setup.h>
#include <mach/board_lge.h>

#define PANIC_HANDLER_NAME "panic-handler"
#define PANIC_DUMP_CONSOLE 0
#define PANIC_MAGIC_KEY	0x12345678
#define CRASH_ARM9		0x87654321

/* following data structure is duplicate of drivers/video/console/fbcon.h */
struct fbcon_ops {
	void (*bmove)(struct vc_data *vc, struct fb_info *info, int sy,
			int sx, int dy, int dx, int height, int width);
	void (*clear)(struct vc_data *vc, struct fb_info *info, int sy,
			int sx, int height, int width);
	void (*putcs)(struct vc_data *vc, struct fb_info *info,
			const unsigned short *s, int count, int yy, int xx,
			int fg, int bg);
	void (*clear_margins)(struct vc_data *vc, struct fb_info *info,
			int bottom_only);
	void (*cursor)(struct vc_data *vc, struct fb_info *info, int mode,
			int softback_lines, int fg, int bg);
	int  (*update_start)(struct fb_info *info);
	int  (*rotate_font)(struct fb_info *info, struct vc_data *vc);
	struct fb_var_screeninfo var;  /* copy of the current fb_var_screeninfo */
	struct timer_list cursor_timer; /* Cursor timer */
	struct fb_cursor cursor_state;
	struct display *p;
	int    currcon;                 /* Current VC. */
	int    cursor_flash;
	int    cursor_reset;
	int    blank_state;
	int    graphics;
	int    flags;
	int    rotate;
	int    cur_rotate;
	char  *cursor_data;
	u8    *fontbuffer;
	u8    *fontdata;
	u8    *cursor_src;
	u32    cursor_size;
	u32    fd_size;
};

/* following data structure is duplicate of drivers/staging/android/ram_console.c */
struct ram_console_buffer {
	uint32_t    sig;
	uint32_t    start;
	uint32_t    size;
	uint8_t     data[0];
};

struct panic_log_dump {
	unsigned int magic_key;
	unsigned int size;
	unsigned char buffer[0];
};

#ifndef CONFIG_LGE_HIDDEN_RESET_PATCH
static struct panic_log_dump *panic_dump_log;
static int (*reboot_key_detect)(void) = NULL;
static char *panic_init_strings[] = {
	"K e r n e l   p a n i c   h a s   b e e n   g e n e r a t e d . . . ",
	"F o l l o w i n g   m e s s a g e s   s h o w   c p u   c o n t e x t ",
	"a n d   b a c k t r a c e s   o f   f u n c t i o n   c a l l ",
	"  ",
	"  ",
	"  ",
	"I f   y o u   w a n t   t o   r e b o o t   p h o n e ,   p r e s s   a n y   k e y ",
	"  ",
	"  ",
	"  ",
};

static DEFINE_SPINLOCK(lge_panic_lock);
#else
static int (*reboot_key_detect)(void) = NULL;
#endif

static int dummy_arg;
static int gen_panic(const char *val, struct kernel_param *kp)
{
	BUG();

	return 0;
}
module_param_call(gen_panic, gen_panic, param_get_bool, &dummy_arg, S_IWUSR | S_IRUGO);

static int display_lk_enable = 0;
module_param_named(
		display_lk_enable, display_lk_enable,
		int, S_IRUGO | S_IWUSR | S_IWGRP);

static int display_kernel_enable = 0;
module_param_named(
		display_kernel_enable, display_kernel_enable,
		int, S_IRUGO | S_IWUSR | S_IWGRP);

#ifdef CONFIG_LGE_BLUE_ERROR_HANDLER
int hidden_reset_enable = 0;
module_param_named(
		hidden_reset_enable, hidden_reset_enable,
		int, S_IRUGO | S_IWUSR | S_IWGRP);

int on_hidden_reset = 0;
module_param_named(
		on_hidden_reset, on_hidden_reset,
		int, S_IRUGO | S_IWUSR | S_IWGRP);

static int __init check_hidden_reset(char *reset_mode)
{
	if (!strncmp(reset_mode, "on", 2)) {
		on_hidden_reset = 1;
		printk(KERN_INFO"reboot mode: hidden reset %s\n", "on");
	}
	
	return 1;
}
__setup("lge.hreset=", check_hidden_reset);
#endif

#ifndef CONFIG_LGE_HIDDEN_RESET_PATCH
static struct ram_console_buffer *ram_console_buffer = 0;

static int get_panic_report_start(uint32_t start, uint32_t size, uint8_t *data)
{
	int report_start;
	uint8_t buffer[8];
	int i;

	report_start = -1;

	for (i = 0; i < size; i++) {
		if (data[i] == '>') {
			if (!strncmp(&data[i], ">>>>>", 5)) {
				report_start = i;
				break;
			}
		}
	}

	/* because ram cosole is ring buffer */
	for (i = 0; i < 4; i++) {
		buffer[i] = data[(size - 4) + i];
		buffer[i + 4] = data[i];
	}
	for (i = 0; i < 4; i++) {
		if (buffer[i] == '>') {
			if (!strncmp(&buffer[i], ">>>>>", 5)) {
				report_start = (size - 4) + i;
				break;
			}
		}
	}

	return report_start;
}

static void display_line(unsigned short *line_buffer, int line_num)
{
	struct vc_data *vc;
	struct fb_info *info = registered_fb[0];
	struct fbcon_ops *ops = info->fbcon_par;

	vc = vc_cons[PANIC_DUMP_CONSOLE].d;
	vc->vc_mode = KD_TEXT;
	ops->graphics = 0;
	ops->putcs(vc, info, (unsigned short *)line_buffer, 
			vc->vc_cols, line_num, 0, 1, 0);
}

static void display_update(void)
{
	struct fb_info *info = registered_fb[0];
	struct fbcon_ops *ops = info->fbcon_par;

	ops->update_start(info);
}

void msm_pm_flush_console(void);
#endif

#ifdef CONFIG_LGE_HIDDEN_RESET_PATCH
static int copy_frame_buffer(struct notifier_block *this, unsigned long event,
		void *ptr)
{
	void *fb_addr;
	void *copy_addr;
	void *copy_phys_addr;
	int fb_size = HIDDEN_RESET_FB_SIZE;

	copy_addr = lge_get_fb_copy_virt_addr();
	fb_addr = lge_get_fb_addr();
	printk(KERN_INFO"%s: copy %x\n",__func__, (unsigned)copy_addr);
	printk(KERN_INFO"%s: fbad %x\n",__func__, (unsigned)fb_addr);

	memcpy(copy_addr, fb_addr, fb_size);

	copy_phys_addr = lge_get_fb_copy_phys_addr();
	lge_set_reboot_reason((unsigned int)copy_phys_addr);

	*((unsigned *)copy_addr) = 0x12345678;
	printk(KERN_INFO"%s: hidden magic  %x\n",__func__, *((unsigned *)copy_addr));

	return NOTIFY_DONE;
}
#else
static int display_panic_reason(struct notifier_block *this, unsigned long event,
		void *ptr)
{
	struct vc_data *vc;
	uint32_t start;
	uint32_t size;
	uint8_t *data;
	int report_start;
	unsigned char *display_buffer;
	unsigned char *store_buffer;
	int	display_size;
	int index;
	int line_num = 0;
	int time_count;
	unsigned long flags;
	struct membank *bank = &meminfo.bank[0];

	/* if direct display in kernel or lk is not enabled, return immediately */
	if (!display_lk_enable && !display_kernel_enable) {
		/* don't display and return */
		return NOTIFY_DONE;
	}

	msm_pm_flush_console();
	
	if (display_kernel_enable)
		mdelay(500);
	spin_lock_irqsave(&lge_panic_lock, flags);
	bust_spinlocks(1);

	/* initial variables */
	ram_console_buffer = get_ram_console_buffer();

	start = ram_console_buffer->start;
	size = ram_console_buffer->size;
	data = ram_console_buffer->data;
	report_start = get_panic_report_start(start, size, data);
	panic_dump_log = (struct panic_log_dump *)(data - sizeof(struct ram_console_buffer)
					+ (SZ_1K * 128));

	if (report_start < start)
		display_size = (start - report_start);
	else 
		display_size = (size - report_start) + start;

	/* store kernel log to buffer which will be used by lk loader */
	if (display_lk_enable) {
		panic_dump_log->magic_key = PANIC_MAGIC_KEY;
		panic_dump_log->size = display_size;
		store_buffer = panic_dump_log->buffer;
		memset(store_buffer, 0x00, display_size * 2);
		lge_set_reboot_reason(bank->size);

		if ((unsigned int)ptr == CRASH_ARM9) /* arm9 has crashed */
			panic_dump_log->magic_key = CRASH_ARM9;

		if (report_start < start) {
			memcpy(store_buffer, &data[report_start], display_size);
		} else {
			memcpy(store_buffer, &data[report_start], size - report_start);
			memcpy(store_buffer, &data[0], start);
		}
	}

	/* if direct display in kernel is not enabled, return immediately */
	if (!display_kernel_enable) {
		/* don't display and return */
		bust_spinlocks(0);
		spin_unlock_irqrestore(&lge_panic_lock, flags);
		//mdelay(1500);
		
		return NOTIFY_DONE;
	}

	/* display in kernel start */
	vc = vc_cons[PANIC_DUMP_CONSOLE].d;
	display_buffer =
		(unsigned char *)kzalloc(vc->vc_cols * sizeof(unsigned short), GFP_ATOMIC);

	/* blank screen */
	for (index = 0; index < vc->vc_rows; index++)
		display_line((unsigned short *)display_buffer, index);

	for (index = 0; index < ARRAY_SIZE(panic_init_strings); index++) {
		memset(display_buffer, 0x00, vc->vc_cols * sizeof(unsigned short));
		memcpy(display_buffer, panic_init_strings[index], 
				strlen(panic_init_strings[index]));
		display_line((unsigned short *)display_buffer, line_num);
		line_num++;
	}

	if (report_start < start) {
		unsigned char *line_buffer = display_buffer;

		for (index = 0; index < display_size; index++) {
			if (data[report_start + index] == '\n') {
				display_line((unsigned short *)line_buffer, line_num);
				line_num++;
				display_buffer = line_buffer;
				memset(display_buffer, 0x00, vc->vc_cols * sizeof(unsigned short));
			} else {
				*display_buffer++ = data[report_start + index];
				*display_buffer++ = 0x0;
			}

			if (line_num == vc->vc_rows)
				goto reboot_system;
		}
	} else {
		unsigned char *line_buffer = display_buffer;
		
		for (index = 0; index < (size - report_start); index++) {
			if (data[report_start + index] == '\n') {
				display_line((unsigned short *)line_buffer, line_num);
				line_num++;
				display_buffer = line_buffer;
				memset(display_buffer, 0x00, vc->vc_cols * sizeof(unsigned short));
			} else {
				*display_buffer++ = data[report_start + index];
				*display_buffer++ = 0x0;
			}

			if (line_num == vc->vc_rows)
				goto reboot_system;
		}
	
		display_buffer = line_buffer;
		for (index = 0; index < start; index++) {
			if (data[report_start + index] == '\n') {
				display_line((unsigned short *)line_buffer, line_num);
				line_num++;
				display_buffer = line_buffer;
				memset(display_buffer, 0x00, vc->vc_cols * sizeof(unsigned short));
			} else {
				*display_buffer++ = data[index];
				*display_buffer++ = 0x0;
			}
			
			if (line_num == vc->vc_rows)
				goto reboot_system;
		}
	}

reboot_system:
	display_update();
	
	bust_spinlocks(0);
	spin_unlock_irqrestore(&lge_panic_lock, flags);
	
	/* wait reboot key pushed
	 * if 20 second is elapsed after panic in not pushed,
	 * reboot automatically */
	time_count = 40;
	while(1) {
		mdelay(500);
		if (reboot_key_detect)
			if (reboot_key_detect() == REBOOT_KEY_PRESS)
				break;
		time_count--;
		if (time_count == 0)
			break;
	}

	return NOTIFY_DONE;
}
#endif

static struct notifier_block panic_handler_block = {
#ifdef CONFIG_LGE_HIDDEN_RESET_PATCH
	.notifier_call  = copy_frame_buffer,
#else
	.notifier_call  = display_panic_reason,
#endif
};

static int __init lge_panic_handler_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct lge_panic_handler_platform_data *pdata = pdev->dev.platform_data;
	
	reboot_key_detect = pdata->reboot_key_detect;

	/* Setup panic notifier */
	atomic_notifier_chain_register(&panic_notifier_list, &panic_handler_block);

	return ret;
}

static int __devexit lge_panic_handler_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver __refdata panic_handler_driver = {
	.remove = __devexit_p(lge_panic_handler_remove),
	.driver = {
		.name = PANIC_HANDLER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init lge_panic_handler_init(void)
{
	return platform_driver_probe(&panic_handler_driver, lge_panic_handler_probe);
}

static void __exit lge_panic_handler_exit(void)
{
	platform_driver_unregister(&panic_handler_driver);
}

module_init(lge_panic_handler_init);
module_exit(lge_panic_handler_exit);

MODULE_DESCRIPTION("LGE panic handler driver");
MODULE_AUTHOR("SungEun Kim <cleaneye.kim@lge.com>");
MODULE_LICENSE("GPL");
