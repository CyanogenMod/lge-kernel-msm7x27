/*
 *  drivers/input/keyboard/kbd_pp2016.c
 *
 *  Copyright (c) 2010 LGE.
 *  
 *  All source code in this file is licensed under the following license
 *  except where indicated.
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, you can find it at http://www.fsf.org
 */

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <mach/board_lge.h>
#if defined(CONFIG_LGE_DIAGTEST)
#include <mach/lge_diag_test.h>
#endif
#include <linux/lge_alohag_at.h>

#define DRIVER_VERSION "v1.0"
#define KEY_DRIVER_NAME "kbd_pp2106"

static const char *kbd_name  = "pp2106"; 

#define PP2106_EARLY_SUSPEND


#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_MACH_MSM7X27_MUSCAT
#include <linux/earlysuspend.h>

static struct early_suspend kbd_early_suspend;
static void pp2106_early_suspend(struct early_suspend *h);
static void pp2106_late_resume(struct early_suspend *h);
#endif
#endif


/* add extern value for AT_CMD GKPD
 * 2010-05-02 younchan.kim@lge.com
 */
extern int gkpd_status ;
extern int at_key_logging(int scancode,unsigned char keystate);
extern unsigned long int ats_mtc_log_mask;

enum kbd_inevents {
	PP2106_IN_KEYPRESS        = 1,
	PP2106_IN_KEYRELEASE      = 0,
};

enum {
	GPIO_LOW_VALUE  = 0,
	GPIO_HIGH_VALUE = 1
};

enum {
	QWERTY_START_BIT = 0,
	QWERTY_1ST_BIT7,
	QWERTY_1ST_BIT6,
	QWERTY_1ST_BIT5,
	QWERTY_1ST_BIT4,
	QWERTY_1ST_BIT3,
	QWERTY_1ST_BIT2,
	QWERTY_1ST_BIT1,
	QWERTY_1ST_BIT0,
	QWERTY_ACK_BIT,
	QWERTY_MAX_BIT
} ;

#define KEY_SCL_PIN		(pp2106_pdata->scl_pin)
#define KEY_SDA_PIN		(pp2106_pdata->sda_pin)

#define KEY_IRQ_PIN		(pp2106_pdata->irq_pin)
#define KEY_RST_PIN		(pp2106_pdata->reset_pin)

#define QWERTY_SDA_OUTPUT()	{ gpio_direction_output(KEY_SDA_PIN, 1); udelay(25);}
#define QWERTY_SDA_HIGH()	{ gpio_set_value(KEY_SDA_PIN,GPIO_HIGH_VALUE); udelay(25); }
#define QWERTY_SDA_LOW()	{ gpio_set_value(KEY_SDA_PIN,GPIO_LOW_VALUE); udelay(25); }
#define QWERTY_SDA_INPUT()	{ gpio_direction_input(KEY_SDA_PIN); udelay(25);}
#define QWERTY_SDA_READ()	gpio_get_value(KEY_SDA_PIN)
#define QWERTY_SCL_OUTPUT()	{ gpio_direction_output(KEY_SCL_PIN, 1); udelay(25); }
#define QWERTY_SCL_HIGH()	{ gpio_set_value(KEY_SCL_PIN,GPIO_HIGH_VALUE); udelay(25); }
#define QWERTY_SCL_LOW()	{ gpio_set_value(KEY_SCL_PIN,GPIO_LOW_VALUE); udelay(25); }

#define QWERTY_IRQ_LOW()	{ gpio_set_value(KEY_IRQ_PIN,GPIO_LOW_VALUE); udelay(25); }
#define QWERTY_IRQ_HIGH()	{ gpio_set_value(KEY_IRQ_PIN,GPIO_HIGH_VALUE); udelay(25); }
#define QWERTY_RST_LOW()	{ gpio_set_value(KEY_RST_PIN,GPIO_LOW_VALUE); udelay(25); }

/*
 * The qwerty_kbd_record structure consolates all the data/variables
 * specific to managing the single instance of the keyboard.
 */

static struct input_dev *pp2106_kbd_dev;
static struct pp2106_platform_data *pp2106_pdata;
static struct work_struct pp2106_irqwork;

static __inline void pp2106_send_ack(void)
{
	QWERTY_SDA_OUTPUT();
	QWERTY_SDA_LOW();

	QWERTY_SCL_LOW();
	gpio_direction_input(KEY_SDA_PIN);
	QWERTY_SCL_HIGH();
}

static __inline int pp2106_get_data(uint32_t *p_data)
{
	int trigger_count;
	int first_bit_count = 0;

	QWERTY_SDA_OUTPUT();
	QWERTY_SCL_HIGH();
	QWERTY_SDA_LOW();

	for (trigger_count=QWERTY_START_BIT;
			trigger_count < QWERTY_MAX_BIT; trigger_count++) {
		if (trigger_count == QWERTY_START_BIT) {
			QWERTY_SCL_LOW();
			QWERTY_SDA_INPUT();	
			QWERTY_SCL_HIGH();
		} 
		else if (trigger_count >= QWERTY_1ST_BIT7 &&
				trigger_count <= QWERTY_1ST_BIT0) {
			QWERTY_SCL_LOW();
			if(QWERTY_SDA_READ()) {
				*p_data |= 0x80 >> (first_bit_count);
			}
			first_bit_count++;
			QWERTY_SCL_HIGH();
		}
		else if (trigger_count == QWERTY_ACK_BIT) {
			pp2106_send_ack();
		} 
	}

	return 0;
}

static irqreturn_t pp2106_irqhandler(int irq, void *dev_id)
{
	schedule_work(&pp2106_irqwork);

	return IRQ_HANDLED;
}

static int pp2106_config_gpio(void)
{
	int rc = 0;

	rc = gpio_request(pp2106_pdata->irq_pin, "pp2106_irq");
	if (rc) 
		printk(KERN_ERR"gpio_request failed on pin %d (rc=%d)\n",
				pp2106_pdata->irq_pin, rc);

	rc = gpio_direction_input(pp2106_pdata->irq_pin);
	if (rc) 
		printk(KERN_ERR"gpio_direction_input failed on "
				"pin %d (rc=%d)\n", pp2106_pdata->irq_pin, rc);

	rc = gpio_request(pp2106_pdata->reset_pin, "pp2106_reset");
	if (rc) 
		printk(KERN_ERR"gpio_request failed on pin %d (rc=%d)\n",
				pp2106_pdata->reset_pin, rc);

	rc = gpio_direction_output(pp2106_pdata->reset_pin, 1);
	if (rc) 
		printk(KERN_ERR"gpio_direction_output failed on "
				"pin %d (rc=%d)\n", pp2106_pdata->reset_pin, rc);
	
	rc = gpio_request(pp2106_pdata->sda_pin, "pp2106_sda");
	if (rc) 
		printk(KERN_ERR"gpio_request failed on pin %d (rc=%d)\n",
				pp2106_pdata->sda_pin, rc);

	rc = gpio_direction_input(pp2106_pdata->sda_pin);
	if (rc) 
		printk(KERN_ERR"gpio_direction_input failed on "
				"pin %d (rc=%d)\n", pp2106_pdata->sda_pin, rc);
	
	rc = gpio_request(pp2106_pdata->scl_pin, "pp2106_scl");
	if (rc) 
		printk(KERN_ERR"gpio_request failed on pin %d (rc=%d)\n",
				pp2106_pdata->scl_pin, rc);

	rc = gpio_direction_output(pp2106_pdata->scl_pin, 1);
	if (rc) 
		printk(KERN_ERR"gpio_direction_output failed on "
				"pin %d (rc=%d)\n", pp2106_pdata->scl_pin, rc);

	return rc;
}

static void pp2106_hwreset(void)
{
	gpio_set_value(pp2106_pdata->reset_pin, 1);
	msleep(25);
	gpio_set_value(pp2106_pdata->reset_pin, 0);
	msleep(25);
	gpio_set_value(pp2106_pdata->reset_pin, 1);
	msleep(25);
}

#if defined(CONFIG_LGE_DIAGTEST) || defined(CONFIG_LGE_TOUCH_RECORDING)
static void pp2106_ats_eta_key_logging(int scancode, unsigned char keystate)
{
	unsigned char *eta_cmd_buf = NULL;
	unsigned char *eta_cmd_buf_encoded = NULL;
	int index =0;
	int lenb64 = 0;
	int exec_result = 0;

	/* c710 unused lgf_factor_key_test_rsp funtion (vs740 use)
	 * if(if_condition_is_on_key_buffering == 1 && keystate == PP2106_IN_KEYPRESS)
	 *	lgf_factor_key_test_rsp((u8)scancode);
	 */
	/* 23 bytes for key log -> will be encoded to 32 byte (base64)
	 * CMD_CODE 		1 (0xF0)
	 * SUB_CMD 			1 (0x08)
	 * LOG_ID  			1 (1 key, 2 touch)
	 * LOG_LEN   		2 (data length in bytes)
	 * LOG_DATA 		LOG_LEN = 18
	 * 		- TIME		8 (timestamp in milliseconds)
	 * 		- HOLD		1 (Press or release)
	 * 		- KEYCODE	1
	 * 		- ACTIVE_UIID 8 (Activated UI ID)
	 */
	if((ats_mtc_log_mask&0x00000001) != 0) { /* ETA_LOGITEM_KEY */
		eta_cmd_buf = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
		eta_cmd_buf_encoded = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
		memset(eta_cmd_buf,0x00, 50);
		memset(eta_cmd_buf_encoded,0x00, 50);

		index = 0;
		eta_cmd_buf[index++] = (unsigned char)0xF0;	/* MTC_CMD_CODE */
		eta_cmd_buf[index++] = (unsigned char)0x08;	/* MTC_LOG_REQ_CMD */
		eta_cmd_buf[index++] = (unsigned char)1; 	/* LOG_ID, 1 key, 2 touch */
		eta_cmd_buf[index++] = (unsigned char)18;	/* LOG_LEN */
		eta_cmd_buf[index++] = (unsigned char)0;	/* LOG_LEN */
		
		/* LOG_DATA 18bytes */
		for(index = 5; index<23; index++)
			eta_cmd_buf[index] = 0;

		index = 13;
		eta_cmd_buf[index++] = (unsigned char)keystate; /* hold */
		eta_cmd_buf[index++] = (unsigned char)scancode; /* key code */

		printk(KERN_INFO "[ETA] key code 0x%X, hold : %d \n", scancode, keystate);

		lenb64 = base64_encode((char *)eta_cmd_buf, index, (char *)eta_cmd_buf_encoded);

		exec_result = eta_execute(eta_cmd_buf_encoded);
		printk(KERN_INFO "[ETA]AT+MTC exec_result %d\n",exec_result);

		kfree(eta_cmd_buf);
		kfree(eta_cmd_buf_encoded);
	}
}
#endif

static void pp2106_fetchkeys(struct work_struct *work)
{
	u32 scancode = 0;
	u32 buf = 0;
	u8 keystate = 0;  /* press = 1 , release = 0 */
	u8 key_col, key_row;

	pp2106_get_data(&buf);
	keystate = (buf & 0x80) ? PP2106_IN_KEYRELEASE : PP2106_IN_KEYPRESS;

	key_col = key_row = buf;
	
	/* hazel : fixed col, row by bongkyu.kim@lge.com
	 * old driver has problem that col, row are reversed
	 */
	key_col &= 0x0f;
	if (key_col)
		key_col -= 1;

	key_row >>= 4; 
	key_row &= 0x07;

	scancode = (unsigned int)pp2106_pdata->keycode[2 * (key_row * pp2106_pdata->keypad_col + key_col)];

	printk(KERN_INFO"Keypad : row <0x%x>, column <0x%x>, keycode <<%d>>\n", 
			key_row, key_col, scancode);

	if (scancode) {
		input_report_key(pp2106_kbd_dev, scancode, keystate);
		input_sync(pp2106_kbd_dev);
		/* add func at_key_logging for AT_CMD GKPD
		 * 2010-05-02 younchan.kim@lge.com
		 */
		/* hazel temp block by bongkyu.kim */
		//if(gkpd_status)
		//	at_key_logging(scancode, keystate);
#if defined(CONFIG_LGE_DIAGTEST) || defined(CONFIG_LGE_TOUCH_RECORDING)
		pp2106_ats_eta_key_logging(scancode, keystate);
#endif
	}
}

struct input_dev *qwerty_get_input_dev(void)
{
	return pp2106_kbd_dev;
}
EXPORT_SYMBOL(qwerty_get_input_dev);

static int pp2106_suspend(struct platform_device *pdev, pm_message_t state)
{
	disable_irq_nosync(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin));
		
	pp2106_pdata->power(0);

	QWERTY_SDA_LOW();
	QWERTY_SCL_LOW();
	QWERTY_IRQ_LOW();
	QWERTY_RST_LOW();
	printk("[pp2106] %s() ............ \n",__FUNCTION__);
	return 0;
}

static int pp2106_resume(struct platform_device *pdev)
{
	pp2106_pdata->power(1);

	/* future capability*/
	QWERTY_SDA_HIGH();
	QWERTY_SCL_HIGH();
	QWERTY_IRQ_HIGH();

	pp2106_hwreset();

	enable_irq(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin));
	printk("[pp2106] %s() ............ \n",__FUNCTION__);

	return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_MACH_MSM7X27_MUSCAT
static void pp2106_early_suspend(struct early_suspend * h)
{

	if(lge_bd_rev < LGE_REV_C)
		return;
		

	disable_irq_nosync(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin));

	pp2106_pdata->power(0);

	QWERTY_SDA_LOW();
	QWERTY_SCL_LOW();
	QWERTY_IRQ_LOW();
	QWERTY_RST_LOW();

	printk("[pp2106] %s() ............ \n",__FUNCTION__);
}

static void pp2106_late_resume(struct early_suspend * h)
{

	if(lge_bd_rev < LGE_REV_C)
		return;
	
	pp2106_pdata->power(1);

	/* future capability*/
	QWERTY_SDA_HIGH();
	QWERTY_SCL_HIGH();
	QWERTY_IRQ_HIGH();

	pp2106_hwreset();

	enable_irq(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin));
	printk("[pp2106] %s() ............ \n",__FUNCTION__);

}
#endif
#endif



static int  __init pp2106_probe(struct platform_device *pdev)
{
	int rc = 0;
	int key_idx;
	unsigned keycode = KEY_UNKNOWN;

	if (!pdev || !pdev->dev.platform_data) {
		printk(KERN_ERR"%s : pdev or platform data is null\n", __func__);
		return -ENODEV;
	}
	pp2106_pdata = pdev->dev.platform_data;

	if (!pp2106_pdata->reset_pin 	|| !pp2106_pdata->irq_pin		||
		!pp2106_pdata->sda_pin 		|| !pp2106_pdata->scl_pin		||
		!pp2106_pdata->keypad_row	|| !pp2106_pdata->keypad_col	||
		!pp2106_pdata->keycode) {
		printk(KERN_ERR"%s : platform data is invalid\n", __func__);
		return -ENODEV;
	}

	pp2106_kbd_dev = input_allocate_device();
	if (!pp2106_kbd_dev) {
		printk(KERN_ERR "%s: not enough memory for input device\n", __func__);
		return -ENOMEM;
	}

	pp2106_kbd_dev->name = KEY_DRIVER_NAME;
	pp2106_kbd_dev->phys = "pp2106/input1";
	pp2106_kbd_dev->id.bustype = BUS_HOST;
	pp2106_kbd_dev->id.vendor = 0x0001;
	pp2106_kbd_dev->id.product = 0x0001;
	pp2106_kbd_dev->id.version = 0x0100;
	pp2106_kbd_dev->dev.parent = &pdev->dev;
	pp2106_kbd_dev->evbit[0] = BIT_MASK(EV_KEY);

	pp2106_kbd_dev->keycode = pp2106_pdata->keycode;
	pp2106_kbd_dev->keycodesize = sizeof(unsigned short);
	pp2106_kbd_dev->keycodemax = pp2106_pdata->keypad_row * pp2106_pdata->keypad_col;
	pp2106_kbd_dev->mscbit[0] = 0;
	
	for (key_idx = 0; key_idx <= pp2106_kbd_dev->keycodemax; key_idx++) {
		keycode = pp2106_pdata->keycode[2 * key_idx];
		if (keycode != KEY_UNKNOWN)
				set_bit(keycode, pp2106_kbd_dev->keybit);
	}
	
	pp2106_pdata->power(1);

	rc = pp2106_config_gpio();
	if (rc) {
		printk(KERN_ERR"%s : gpio setting failed\n", __func__);
		return rc;
	}

	pp2106_hwreset();

	INIT_WORK(&pp2106_irqwork, pp2106_fetchkeys);
	
	rc = request_irq(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin), &pp2106_irqhandler,
			IRQF_TRIGGER_FALLING, kbd_name, NULL);
	if (rc < 0) {
		printk(KERN_ERR
				"Could not register for  %s interrupt "
				"(rc = %d)\n", kbd_name, rc);
		rc = -EIO;
	}
	//set_irq_wake(MSM_GPIO_TO_INT(pp2106_pdata->irq_pin), 1);

	rc = input_register_device(pp2106_kbd_dev);
	if (rc)
		printk(KERN_ERR"%s : input_register_device failed\n", __func__);

#ifdef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_MACH_MSM7X27_MUSCAT
	if(lge_bd_rev >= LGE_REV_C)
	{
		kbd_early_suspend.suspend = pp2106_early_suspend;
		kbd_early_suspend.resume = pp2106_late_resume;
		kbd_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1 ;
		register_early_suspend(&kbd_early_suspend);
	}
#endif	
#endif

	return rc;
}

static struct platform_driver __refdata qwerty_kbd_driver = {
	.driver = {
		.name = KEY_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.suspend = pp2106_suspend,
	.resume  = pp2106_resume,
};

static int __init pp2106_init(void)
{
	return platform_driver_probe(&qwerty_kbd_driver, pp2106_probe);
}


static void __exit pp2106_exit(void)
{
	platform_driver_unregister(&qwerty_kbd_driver);
}

module_init(pp2106_init);
module_exit(pp2106_exit);

MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION("PP2106 QWERTY keyboard driver");
MODULE_LICENSE("GPL v2");
