/****************************************************************************
 * Created by [bluerti@lge.com]
 * 2009-07-06
 * Made this file for implementing LGE Error Hanlder 
 * *************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fb.h>
#include <linux/vt_kern.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <linux/irq.h>
#include <asm/system.h>
#include "../smd_private.h"

#define MODEM_CRASH				0
#define APPL_CRASH				1
#define ANDROID_CRASH			2
#define ANDROID_DISPLAY_INFO	3

#define ERRHANDLER_IOCTL_LEVEL1		1
#define ERRHANDLER_IOCTL_LEVEL2		2

#define	LGE_ERROR_MAX_ROW				50	//30
#define	LGE_ERROR_MAX_COLUMN			80	//40

#define LG_ERROR_FILE_ARM9			"/sdcard/arm9_error_info.txt"
#define LG_ERROR_FILE_ARM11		"/sdcard/arm11_error_info.txt"
#define LG_ERROR_DEFAULT_FILE		"/data/default_error_info.txt"

#define LGE_ERR_MESSAGE_BUF_LEN   (LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN +8)
#define TRUE		1
#define FALSE		0

#define LED_COLOR_RED	1
#define LED_COLOR_BLUE	2
#define LED_COLOR_GREEN	3

int LGE_ErrorHandler_Main( int crash_side, char * message);
int display_info_LCD( int crash_side, char * message);
int store_info_file(int crash_side, char * message);
void expand_char_to_shrt(char * message,unsigned short * buffer);
extern void display_errorinfo_byLGE(int crash_side, unsigned short * buf, int count);
void ramdump_reset_func(int key_value);
