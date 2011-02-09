#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>

#include "mt9t113.h"

#define BUF_SIZE	(128 * 1024)

struct mt9t113_reg mt9t113_regs;

static struct mt9t113_register_address_value_pair
	init_tmp_reg_settings_array[10];
static struct mt9t113_register_address_value_pair
	tuning_tmp_reg_settings_array[1200];
static struct mt9t113_register_address_value_pair
	prev_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	snap_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	effect_off_tmp_reg_settings_array[10];
static struct mt9t113_register_address_value_pair
	effect_mono_tmp_reg_settings_array[10];
static struct mt9t113_register_address_value_pair
	effect_negative_tmp_reg_settings_array[10];
static struct mt9t113_register_address_value_pair
	effect_solarize_tmp_reg_settings_array[10];
static struct mt9t113_register_address_value_pair
	effect_sepia_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	effect_aqua_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	wb_auto_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	wb_incandescent_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	wb_fluorescent_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	wb_daylight_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	wb_cloudy_tmp_reg_settings_array[20];
static struct mt9t113_register_address_value_pair
	iso_auto_tmp_reg_settings_array[30];
static struct mt9t113_register_address_value_pair
	iso_100_tmp_reg_settings_array[30];
static struct mt9t113_register_address_value_pair
	iso_200_tmp_reg_settings_array[30];
static struct mt9t113_register_address_value_pair
	iso_400_tmp_reg_settings_array[30];
static struct mt9t113_register_address_value_pair
	brightness_tmp_reg_settings_array[30];
// 2010-11-24 add framerate mode
static struct mt9t113_register_address_value_pair
 auto_tmp_framerate_mode_reg_settings_array[30];
static struct mt9t113_register_address_value_pair
	fixed_tmp_framerate_mode_reg_settings_array[30];

static void parsing_register(char* buf, int buf_size)
{
	int i = 0;
	int addr, value;
	int rc;

	int init_cnt = 0;
 int tuning_cnt = 0;
 int prev_cnt = 0;
 int snap_cnt = 0;
 int effect_off_cnt = 0;
 int effect_mono_cnt = 0;
 int effect_negative_cnt = 0;
 int effect_solarize_cnt = 0;
 int effect_sepia_cnt = 0;
 int effect_aqua_cnt = 0;
 int wb_auto_cnt = 0;
 int wb_incandescent_cnt = 0;
 int wb_fluorescent_cnt = 0;
 int wb_daylight_cnt = 0;
 int wb_cloudy_cnt = 0;
 int iso_auto_cnt = 0;
 int iso_100_cnt = 0;
 int iso_200_cnt = 0;
 int iso_400_cnt = 0;
 int brightness_cnt = 0;
 int auto_fps_cnt = 0;
 int fixed_fps_cnt = 0;

	char scan_buf[16];
	int scan_buf_len;
	char subject;

	while (i < buf_size) {
	 // select subject
	 if (buf[i] == '<') {
	  subject = buf[++i];
	  while(buf[++i] != '>');
	 }

	 // code delude
		if (buf[i] == '{') {
			scan_buf_len = 0;
			while(buf[i] != '}') {
				if (buf[i] < 33 || 126 < buf[i]) {
					++i;
					continue;
				} else
					scan_buf[scan_buf_len++] = buf[i++];
			}
			scan_buf[scan_buf_len++] = buf[i];
			scan_buf[scan_buf_len] = 0;

			rc = sscanf(scan_buf, "{%x,%x}", &addr, &value);

			if (rc != 2) {
				printk(KERN_ERR "file format error. rc = %d\n", rc);
				return;
			}

			switch (subject) {
			 case 'A' : {
 			 init_tmp_reg_settings_array[init_cnt].register_address = addr;
  			init_tmp_reg_settings_array[init_cnt].register_value = value;
  			init_tmp_reg_settings_array[init_cnt].register_length= WORD_LEN;
  			++init_cnt;
  			break;
			 }
			 case 'B' : {
 			 tuning_tmp_reg_settings_array[tuning_cnt].register_address = addr;
  			tuning_tmp_reg_settings_array[tuning_cnt].register_value = value;
  			tuning_tmp_reg_settings_array[tuning_cnt].register_length= WORD_LEN;
  			++tuning_cnt;
  			break;
  		}
			 case 'C' : {
 			 prev_tmp_reg_settings_array[prev_cnt].register_address = addr;
  			prev_tmp_reg_settings_array[prev_cnt].register_value = value;
  			prev_tmp_reg_settings_array[prev_cnt].register_length= WORD_LEN;
  			++prev_cnt;
  			break;
			 }
			 case 'D' : {
 			 snap_tmp_reg_settings_array[snap_cnt].register_address = addr;
  			snap_tmp_reg_settings_array[snap_cnt].register_value = value;
  			snap_tmp_reg_settings_array[snap_cnt].register_length= WORD_LEN;
  			++snap_cnt;
  			break;
			 }
			 case 'E' : {
 			 effect_off_tmp_reg_settings_array[effect_off_cnt].register_address = addr;
  			effect_off_tmp_reg_settings_array[effect_off_cnt].register_value = value;
  			effect_off_tmp_reg_settings_array[effect_off_cnt].register_length= WORD_LEN;
  			++effect_off_cnt;
  			break;
			 }
			 case 'F' : {
 			 effect_mono_tmp_reg_settings_array[effect_mono_cnt].register_address = addr;
  			effect_mono_tmp_reg_settings_array[effect_mono_cnt].register_value = value;
  			effect_mono_tmp_reg_settings_array[effect_mono_cnt].register_length= WORD_LEN;
  			++effect_mono_cnt;
  			break;
			 }
			 case 'G' : {
 			 effect_negative_tmp_reg_settings_array[effect_negative_cnt].register_address = addr;
  			effect_negative_tmp_reg_settings_array[effect_negative_cnt].register_value = value;
  			effect_negative_tmp_reg_settings_array[effect_negative_cnt].register_length= WORD_LEN;
  			++effect_negative_cnt;
  			break;
			 }
			 case 'H' : {
 			 effect_solarize_tmp_reg_settings_array[effect_solarize_cnt].register_address = addr;
  			effect_solarize_tmp_reg_settings_array[effect_solarize_cnt].register_value = value;
  			effect_solarize_tmp_reg_settings_array[effect_solarize_cnt].register_length= WORD_LEN;
  			++effect_solarize_cnt;
  			break;
			 }
			 case 'I' : {
 			 effect_sepia_tmp_reg_settings_array[effect_sepia_cnt].register_address = addr;
  			effect_sepia_tmp_reg_settings_array[effect_sepia_cnt].register_value = value;
  			effect_sepia_tmp_reg_settings_array[effect_sepia_cnt].register_length= WORD_LEN;
  			++effect_sepia_cnt;
  			break;
			 }
			 case 'J' : {
 			 effect_aqua_tmp_reg_settings_array[effect_aqua_cnt].register_address = addr;
  			effect_aqua_tmp_reg_settings_array[effect_aqua_cnt].register_value = value;
  			effect_aqua_tmp_reg_settings_array[effect_aqua_cnt].register_length= WORD_LEN;
  			++effect_aqua_cnt;
  			break;
			 }
			 case 'K' : {
 			 wb_auto_tmp_reg_settings_array[wb_auto_cnt].register_address = addr;
  			wb_auto_tmp_reg_settings_array[wb_auto_cnt].register_value = value;
  			wb_auto_tmp_reg_settings_array[wb_auto_cnt].register_length= WORD_LEN;
  			++wb_auto_cnt;
  			break;
			 }
			 case 'L' : {
 			 wb_incandescent_tmp_reg_settings_array[wb_incandescent_cnt].register_address = addr;
  			wb_incandescent_tmp_reg_settings_array[wb_incandescent_cnt].register_value = value;
  			wb_incandescent_tmp_reg_settings_array[wb_incandescent_cnt].register_length= WORD_LEN;
  			++wb_incandescent_cnt;
  			break;
			 }
			 case 'M' : {
 			 wb_fluorescent_tmp_reg_settings_array[wb_fluorescent_cnt].register_address = addr;
  			wb_fluorescent_tmp_reg_settings_array[wb_fluorescent_cnt].register_value = value;
  			wb_fluorescent_tmp_reg_settings_array[wb_fluorescent_cnt].register_length= WORD_LEN;
  			++wb_fluorescent_cnt;
  			break;
			 }
			 case 'N' : {
 			 wb_daylight_tmp_reg_settings_array[wb_daylight_cnt].register_address = addr;
  			wb_daylight_tmp_reg_settings_array[wb_daylight_cnt].register_value = value;
  			wb_daylight_tmp_reg_settings_array[wb_daylight_cnt].register_length= WORD_LEN;
  			++wb_daylight_cnt;
  			break;
			 }
			 case 'O' : {
 			 wb_cloudy_tmp_reg_settings_array[wb_cloudy_cnt].register_address = addr;
  			wb_cloudy_tmp_reg_settings_array[wb_cloudy_cnt].register_value = value;
  			wb_cloudy_tmp_reg_settings_array[wb_cloudy_cnt].register_length= WORD_LEN;
  			++wb_cloudy_cnt;
  			break;
			 }
			 case 'P' : {
 			 iso_auto_tmp_reg_settings_array[iso_auto_cnt].register_address = addr;
  			iso_auto_tmp_reg_settings_array[iso_auto_cnt].register_value = value;
  			iso_auto_tmp_reg_settings_array[iso_auto_cnt].register_length= WORD_LEN;
  			++iso_auto_cnt;
  			break;
			 }
			 case 'Q' : {
 			 iso_100_tmp_reg_settings_array[iso_100_cnt].register_address = addr;
  			iso_100_tmp_reg_settings_array[iso_100_cnt].register_value = value;
  			iso_100_tmp_reg_settings_array[iso_100_cnt].register_length= WORD_LEN;
  			++iso_100_cnt;
  			break;
			 }
			 case 'R' : {
 			 iso_200_tmp_reg_settings_array[iso_200_cnt].register_address = addr;
  			iso_200_tmp_reg_settings_array[iso_200_cnt].register_value = value;
  			iso_200_tmp_reg_settings_array[iso_200_cnt].register_length= WORD_LEN;
  			++iso_200_cnt;
  			break;
			 }
			 case 'S' : {
 			 iso_400_tmp_reg_settings_array[iso_400_cnt].register_address = addr;
  			iso_400_tmp_reg_settings_array[iso_400_cnt].register_value = value;
  			iso_400_tmp_reg_settings_array[iso_400_cnt].register_length= WORD_LEN;
  			++iso_400_cnt;
  			break;
			 }
			 case 'T' : {
 			 brightness_tmp_reg_settings_array[brightness_cnt].register_address = addr;
  			brightness_tmp_reg_settings_array[brightness_cnt].register_value = value;
  			brightness_tmp_reg_settings_array[brightness_cnt].register_length= WORD_LEN;
  			++brightness_cnt;
  			break;
			 }
			 case 'U' : {
 			 auto_tmp_framerate_mode_reg_settings_array[auto_fps_cnt].register_address = addr;
  			auto_tmp_framerate_mode_reg_settings_array[auto_fps_cnt].register_value = value;
  			auto_tmp_framerate_mode_reg_settings_array[auto_fps_cnt].register_length= WORD_LEN;
  			++auto_fps_cnt;
  			break;
			 }			 
			 case 'V' : {
 			 fixed_tmp_framerate_mode_reg_settings_array[fixed_fps_cnt].register_address = addr;
  			fixed_tmp_framerate_mode_reg_settings_array[fixed_fps_cnt].register_value = value;
  			fixed_tmp_framerate_mode_reg_settings_array[fixed_fps_cnt].register_length= WORD_LEN;
  			++fixed_fps_cnt;
  			break;
			 }			 			 
			 default :
			  break;
			}
		}
		++i;
	}
 //pll
 mt9t113_regs.init_reg_settings = init_tmp_reg_settings_array;
 mt9t113_regs.init_reg_settings_size = init_cnt;

 //init
 mt9t113_regs.tuning_reg_settings = tuning_tmp_reg_settings_array;
 mt9t113_regs.tuning_reg_settings_size = tuning_cnt;

 // preview
 mt9t113_regs.prev_reg_settings = prev_tmp_reg_settings_array;
 mt9t113_regs.prev_reg_settings_size = prev_cnt;

 // capture
 mt9t113_regs.snap_reg_settings = snap_tmp_reg_settings_array;
 mt9t113_regs.snap_reg_settings_size = snap_cnt;

 // effect : off
 mt9t113_regs.effect_off_reg_settings = effect_off_tmp_reg_settings_array;
 mt9t113_regs.effect_off_reg_settings_size = effect_off_cnt;

 // effect : mono
 mt9t113_regs.effect_mono_reg_settings = effect_mono_tmp_reg_settings_array;
 mt9t113_regs.effect_mono_reg_settings_size = effect_mono_cnt;

 // effect : negative
 mt9t113_regs.effect_negative_reg_settings = effect_negative_tmp_reg_settings_array;
 mt9t113_regs.effect_negative_reg_settings_size = effect_negative_cnt;

 // effect : solarize
 mt9t113_regs.effect_solarize_reg_settings = effect_solarize_tmp_reg_settings_array;
 mt9t113_regs.effect_solarize_reg_settings_size = effect_solarize_cnt;

 // effect : sepia
 mt9t113_regs.effect_sepia_reg_settings = effect_sepia_tmp_reg_settings_array;
 mt9t113_regs.effect_sepia_reg_settings_size = effect_sepia_cnt;

 // effect : aqua
 mt9t113_regs.effect_aqua_reg_settings = effect_aqua_tmp_reg_settings_array;
 mt9t113_regs.effect_aqua_reg_settings_size = effect_aqua_cnt;

 // WB : auto
 mt9t113_regs.wb_auto_reg_settings = wb_auto_tmp_reg_settings_array;
 mt9t113_regs.wb_auto_reg_settings_size = wb_auto_cnt;

 // WB : incandescent
 mt9t113_regs.wb_incandescent_reg_settings = wb_incandescent_tmp_reg_settings_array;
 mt9t113_regs.wb_incandescent_reg_settings_size = wb_incandescent_cnt;

 // WB : fluorescent
 mt9t113_regs.wb_fluorescent_reg_settings = wb_fluorescent_tmp_reg_settings_array;
 mt9t113_regs.wb_fluorescent_reg_settings_size = wb_fluorescent_cnt;

 // WB : daylight
 mt9t113_regs.wb_daylight_reg_settings = wb_daylight_tmp_reg_settings_array;
 mt9t113_regs.wb_daylight_reg_settings_size = wb_daylight_cnt;

 // WB : cloudy
 mt9t113_regs.wb_cloudy_reg_settings = wb_cloudy_tmp_reg_settings_array;
 mt9t113_regs.wb_cloudy_reg_settings_size = wb_cloudy_cnt;

 // ISO : auto
 mt9t113_regs.iso_auto_reg_settings = iso_auto_tmp_reg_settings_array;
 mt9t113_regs.iso_auto_reg_settings_size = iso_auto_cnt;

 // ISO : 100
 mt9t113_regs.iso_100_reg_settings = iso_100_tmp_reg_settings_array;
 mt9t113_regs.iso_100_reg_settings_size = iso_100_cnt;

 // ISO : 200
 mt9t113_regs.iso_200_reg_settings = iso_200_tmp_reg_settings_array;
 mt9t113_regs.iso_200_reg_settings_size = iso_200_cnt;

 // ISO : 400
 mt9t113_regs.iso_400_reg_settings = iso_400_tmp_reg_settings_array;
 mt9t113_regs.iso_400_reg_settings_size = iso_400_cnt;

 // Brightness
 mt9t113_regs.brightness_reg_settings = brightness_tmp_reg_settings_array;
 mt9t113_regs.brightness_reg_settings_size = brightness_cnt;
 
 // auto framerate mode
	mt9t113_regs.auto_framerate_reg_settings = auto_tmp_framerate_mode_reg_settings_array;
	mt9t113_regs.auto_framerate_reg_settings_size = auto_fps_cnt;

 // fixed framerate mode
	mt9t113_regs.fixed_framerate_reg_settings = fixed_tmp_framerate_mode_reg_settings_array;
	mt9t113_regs.fixed_framerate_reg_settings_size = fixed_fps_cnt; 
}

static void mt9t113_read_register_from_file(void)
{
	int fd;
	mm_segment_t oldfs;
	char* buf;
	int read_size;

	oldfs = get_fs();
	set_fs(get_ds());

	fd = sys_open("/sdcard/mt9t113_tuning", O_RDONLY, 0644);
	//fd = sys_open("/data/local/register", O_RDONLY, 0644);
		
	if (fd < 0) {
		printk(KERN_ERR "File open fail\n");
		return;
	}

	buf = kmalloc(BUF_SIZE, GFP_KERNEL);
	if (!buf) {
		printk(KERN_ERR "Memory alloc fail\n");
		return;
	}

 /*test*/
 printk(KERN_ERR "[tun] memory allocation m ");

	read_size = sys_read(fd, buf, BUF_SIZE);
	if (read_size < 0) {
		printk(KERN_ERR "File read fail: read_size = %d\n", read_size);
		return;
	}

	parsing_register(buf, read_size);

	kfree(buf);
	sys_close(fd);
	set_fs(oldfs);
}


