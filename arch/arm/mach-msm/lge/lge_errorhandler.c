/****************************************************************************
 * Created by [bluerti@lge.com]
 * 2009-07-06
 * Made this file for implementing LGE Error Hanlder 
 * *************************************************************************/
#include "lge_errorhandler.h"

#include <mach/msm_smd.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>
#include <linux/io.h>
#include <linux/syscalls.h>

#include "smd_private.h"
#include "proc_comm.h"
#include "modem_notifier.h"

#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/suspend.h>

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <mach/pmic.h>
#include <mach/board_lge.h>
#include <linux/notifier.h>

//#define IGNORE_BLUE_ERROR_HANDER	
int BLUE_ERROR_HANDLER_LEVEL =	2;		/* 0 --> Ignore Error Handler, 1 --> Support level 1 , 2 --> Support level 2*/

#define ERROR_HANDLER_NAME "errhandler"
static struct platform_device *errhandler_platform_dev;
extern void LGE_ErrorHandler_bl_on(int on);
extern suspend_state_t get_suspend_state(void);
volatile int LG_ErrorHandler_enable = 0;
static int user_keypress =0;
char android_buf[LGE_ERROR_MAX_ROW][LGE_ERROR_MAX_COLUMN];
unsigned short LG_ErrorHandler_buf[LGE_ERROR_MAX_ROW][LGE_ERROR_MAX_COLUMN];
static int lcd_suspend = 1;
char * android_errhanlder_ptr = NULL;


int LGE_ErrorHandler_Main( int crash_side, char * message)
{

#ifdef CONFIG_MACH_MSM7X27_UNIVA
	/*
	 * 2011-03-10, jinkyu.choi@lge.com
	 * add the reboot reason as chargerlogo reboot when the crash occures.
	 * if we press the volume down key and restart the system for the panic log,
	 * the chargerlogo should not work when rebooting.
	 */
	unsigned *reboot_panic;

	if (!hidden_reset_enable) {
	reboot_panic = ioremap(0x2ff40000, PAGE_SIZE);
	*reboot_panic = (unsigned)0x776655BB;
	iounmap(reboot_panic);
	}
#endif

	if (hidden_reset_enable) {
		if (crash_side == MODEM_CRASH) {
			unsigned *temp;

			printk(KERN_INFO"%s: arm9 has crashed...\n",__func__);
			printk(KERN_INFO"%s\n", message);

			atomic_notifier_call_chain(&panic_notifier_list, 0, "arm9 has crashed...\n");

			temp = lge_get_fb_copy_virt_addr();
			*temp = 0x12345678;
			printk(KERN_INFO"%s: hidden magic  %x\n",__func__, temp[0]);

			return SMSM_SYSTEM_REBOOT;
		}
		return 0;
	}

  	if(BLUE_ERROR_HANDLER_LEVEL != 0) {
		// If LCD is off,  Turn on backlight
		if (get_suspend_state() != PM_SUSPEND_ON) {
		//	LGE_ErrorHandler_bl_on(TRUE);
			lcd_suspend = 0;
		}
		
		switch(crash_side) {
			case MODEM_CRASH:
			case APPL_CRASH:
			case ANDROID_CRASH:
				if(!LG_ErrorHandler_enable) {
					LG_ErrorHandler_enable	= 1;
					raw_local_irq_enable();
					if (message != NULL)
						display_info_LCD(crash_side, message);
				}
				break;
			case ANDROID_DISPLAY_INFO :
				if (message != NULL)
					display_info_LCD(crash_side, message);
				return 0;

			default : 
				break;
				
		}
  	}

	raw_local_irq_disable();
	preempt_disable();

	smsm_reset_modem(SMSM_APPS_SHUTDOWN); //[blue.park@lge.com] It will be informed a kernel panic to Modem side.

  	if(BLUE_ERROR_HANDLER_LEVEL == 0) {		/* Just return */
		mdelay(100);
		return SMSM_SYSTEM_REBOOT;
  	}

	while(1)
	{
#if defined(CONFIG_MACH_MSM7X27_GELATO) || defined(CONFIG_MACH_MSM7X27_UNIVA)
		/*
		 * for Gelato Rev.A Board
		 * FIXME: it should be modified for the common usage.
		 * 2011-03-02, jinkyu.choi@lge.com
		 */
		gpio_set_value(33,0);
		gpio_set_value(34,1);
		gpio_set_value(35,1);
		mdelay(50);

		if(gpio_get_value(38)==0) {
			printk("Pressed Volume up key\n");
			return SMSM_SYSTEM_DOWNLOAD;
		} else if(gpio_get_value(37) ==0 ) {
			printk("Pressed Volume down key\n");
			return SMSM_SYSTEM_REBOOT;
		}
		mdelay(100);
#else
		gpio_set_value(36,0);
		gpio_set_value(32,1);
		gpio_set_value(33,1);
		
		if(gpio_get_value(38)==0) {
			printk("Pressed Volume up key\n");
			return SMSM_SYSTEM_DOWNLOAD; //volume up key is pressed
		} else if(gpio_get_value(37) ==0 ) {
			printk("Pressed Volume down key\n");
			return SMSM_SYSTEM_REBOOT; //volume down key is pressed
		}

		mdelay(200);
#endif
	
	/*	This is for C710 project
		// 1. Check Volume Down Key (GPIO81)
		if(gpio_get_value(81)==0)
			return SMSM_SYSTEM_REBOOT; //volume down key is pressed

		if(!lcd_suspend)
		//	LGE_ErrorHandler_bl_on(FALSE);
		mdelay(100);

		// 2. Check Volume Up Key (GPIO79)
		if(gpio_get_value(79)==0)
			return SMSM_SYSTEM_DOWNLOAD; //volume down key is pressed

		if(!lcd_suspend)
			//LGE_ErrorHandler_bl_on(TRUE);
		mdelay(100);
		*/
		;
	}

 }

int display_info_LCD( int crash_side, char * message)
{
	if (hidden_reset_enable)
		return 0;
	
	memset((char *)LG_ErrorHandler_buf,0x0,sizeof(LG_ErrorHandler_buf));
	expand_char_to_shrt(message,(unsigned short *)LG_ErrorHandler_buf);
	display_errorinfo_byLGE(crash_side, (unsigned short *)LG_ErrorHandler_buf,LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN );
	return 0;
}

void expand_char_to_shrt(char * message,unsigned short *buffer)
{
	char * src = message;
	unsigned char  * dst = (unsigned char *)buffer;
	int i=0;


	for(i=0;i<LGE_ERROR_MAX_ROW*LGE_ERROR_MAX_COLUMN; i++) {
		*dst++ = *src++;
		*dst++ = 0x0;
	}
	
}

void ramdump_reset_func(int key_value)
{
	printk("Ramdump_reset func , Key_value = %d\n",key_value);

	user_keypress =key_value;
	return;	
}

static long errhandler_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int rv = 0;
	int ret;
	
	if (hidden_reset_enable)
		return 0;

	switch(cmd) {
	case ERRHANDLER_IOCTL_LEVEL2 : 
		if (BLUE_ERROR_HANDLER_LEVEL <= 1) {
			break;
		}
	case ERRHANDLER_IOCTL_LEVEL1 :	
		memset(android_buf,0,sizeof(android_buf));
		if (copy_from_user(android_buf, (void __user *)arg, sizeof(android_buf))) {
	    	rv = -EFAULT;
		    goto err1;
		}
		ret = LGE_ErrorHandler_Main(ANDROID_CRASH, (char *)android_buf);
		smsm_reset_modem(ret);
		while(1)
			;
		break;

	default:
		rv = -EINVAL;
		break;
	}


err1:
	return rv;
}

static int errhandler_open(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static int errhandler_release(struct inode *inode, struct file *file)
{
	

	return 0;
}



static int __init lge_error_handler_probe(struct platform_device *pdev)
{

	return 0;
}

static int __devexit lge_error_handler_remove(struct platform_device *pdev)
{
	return 0;
}

static struct file_operations errhandler_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = errhandler_ioctl,
	.open = errhandler_open,
	.release = errhandler_release,
};

static struct miscdevice errhandler_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "errhandler",
	.fops = &errhandler_fops,
};

static struct platform_driver errhandler_driver __refdata = {
	.probe = lge_error_handler_probe,
	.remove = __devexit_p(lge_error_handler_remove),
	.driver = {
		.name = ERROR_HANDLER_NAME,
		.owner = THIS_MODULE,
	},
};

static int errhandler_add_device(void)
{
	int err;
	
	err = misc_register(&errhandler_device);
	if (err)
		goto err1;
	errhandler_platform_dev =
		platform_device_register_simple("errhandler", -1, NULL, 0);
	if (IS_ERR(errhandler_platform_dev)) {
		err = PTR_ERR(errhandler_platform_dev);
		goto err2;
	}

	return 0;


err2:
	misc_deregister(&errhandler_device);
err1:
	return err;
}
static int __init lge_error_handler_init(void)
{
	printk("[Blue Debug] lge_error_handler_init\n");
	platform_driver_register(&errhandler_driver);
	errhandler_add_device();

	return 0;
}

static void __exit lge_error_handler_exit(void)
{
	platform_driver_unregister(&errhandler_driver);
}



module_init(lge_error_handler_init);
module_exit(lge_error_handler_exit);

MODULE_DESCRIPTION("LGE error handler driver");
MODULE_AUTHOR("HyeongKoo Park <blue.park@lge.com>");
MODULE_LICENSE("GPL");

