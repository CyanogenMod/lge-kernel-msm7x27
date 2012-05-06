// File name: kr3dm.c //

/************************* kr3dm *************************************

Application description       : kr3dm Linux driver

                              : STMicroelectronics

Date                          : 08/12/2009

Revision                      : 1-0-0

Changed Features              : First Release

Bug fixes                     : First Release

H/W platform                  : OMAP3530 

MEMS platform                 : digital output KR3DM_ACC

S/W platform                  : gcc 4.2.1

Application Details           : kr3dm Linux driver
                                

Copyright (c) 2009 STMicroelectronics.

THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.

THIS DOCUMENT CONTAINS PROPRIETARY AND CONFIDENTIAL INFORMATION OF THE
STMICROELECTRONICS GROUP.
INFORMATION FURNISHED IS BELIEVED TO BE ACCURATE AND RELIABLE. HOWEVER, 
STMICROELECTRONICS ASSUMES NO RESPONSIBILITY FOR THE CONSEQUENCES OF USE
OF SUCH INFORMATION.
SPECIFICATIONS MENTIONED IN THIS PUBLICATION ARE SUBJECT TO CHANGE WITHOUT NOTICE.
THIS PUBLICATION SUPERSEDES AND REPLACES ALL INFORMATION PREVIOUSLY SUPPLIED.
STMICROELECTRONICS PRODUCTS ARE NOT AUTHORIZED FOR USE AS CRITICAL COMPONENTS IN LIFE
SUPPORT DEVICES OR SYSTEMS WITHOUT EXPRESS WRITTEN APPROVAL OF STMICROELECTRONICS.

STMicroelectronics GROUP OF COMPANIES

Australia - Belgium - Brazil - Canada - China - France - Germany - Italy - Japan - Korea -
Malaysia - Malta - Morocco - The Netherlands - Singapore - Spain - Sweden - Switzerland -
Taiwan - Thailand - United Kingdom - U.S.A.
STMicroelectronics Limited is a member of the STMicroelectronics Group.
********************************************************************************
Version History.

Revision 1-0-0 19/11/09
First Release

Revision 1-0-0 08/12/2009
First Release for KR3DM based on LIS331DLH
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
//#include <linux/init.h>
//#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/uaccess.h>
//#include <linux/unistd.h>
#include <linux/input.h>

#define KR3DM_MAJOR   100
#define KR3DM_MINOR   4

/* KR3DM I2C Slave Address */
#define KR3DM_I2C_ADDR    0x10

#define KR3DM_IOCTL_BASE 'a'
/** The following define the IOCTL command values via the ioctl macros */
#define KR3DM_IOCTL_SET_DELAY       _IOW(KR3DM_IOCTL_BASE, 0, int)
#define KR3DM_IOCTL_GET_DELAY       _IOR(KR3DM_IOCTL_BASE, 1, int)
#define KR3DM_IOCTL_SET_ENABLE      _IOW(KR3DM_IOCTL_BASE, 2, int)
#define KR3DM_IOCTL_GET_ENABLE      _IOR(KR3DM_IOCTL_BASE, 3, int)
#define KR3DM_IOCTL_SET_G_RANGE     _IOW(KR3DM_IOCTL_BASE, 4, int)
#define KR3DM_IOCTL_SET_REPORT     	_IOW(KR3DM_IOCTL_BASE, 5, int)
#define KR3DM_IOCTL_GET_TAPPING    	_IOW(KR3DM_IOCTL_BASE, 6, int)
#define KR3DM_IOCTL_READ_ACCEL_XYZ  _IOW(KR3DM_IOCTL_BASE, 8, int)

//kr3dm registers
#define WHO_AM_I    0x0F
/* ctrl 1: pm2 pm1 pm0 dr1 dr0 zenable yenable zenable */
#define CTRL_REG1       0x20    /* power control reg */
#define CTRL_REG2       0x21    /* power control reg */
#define CTRL_REG3       0x22    /* power control reg */
#define CTRL_REG4       0x23    /* interrupt control reg */
#define CTRL_REG5       0x24    /* interrupt control reg */
#define AXISDATA_REG    0x29

#define OUT_X		0x29
#define OUT_Y		0x2B
#define OUT_Z		0x2D

#define KR3DM_G_2G    0x00
#define KR3DM_G_4G    0x10
#define KR3DM_G_8G    0x30

#define PM_OFF            0x00
#define PM_NORMAL         0x20
#define ENABLE_ALL_AXES   0x07

#define ODRHALF           0x40  /* 0.5Hz output data rate */
#define ODR1              0x60  /* 1Hz output data rate */
#define ODR2              0x80  /* 2Hz output data rate */
#define ODR5              0xA0  /* 5Hz output data rate */
#define ODR10             0xC0  /* 10Hz output data rate */
#define ODR50             0x00  /* 50Hz output data rate */
#define ODR100            0x08  /* 100Hz output data rate */
#define ODR400            0x10  /* 400Hz output data rate */

#define DEBUG 1

/** KR3DM acceleration data 
  \brief Structure containing acceleration values for x,y and z-axis in signed short

*/

typedef struct  
{
  int	x, /**< holds x-axis acceleration data sign extended. Range -128 to +127. */
		y, /**< holds y-axis acceleration data sign extended. Range -128 to +127. */
		z; /**< holds z-axis acceleration data sign extended. Range -128 to +127. */

} kr3dm_acc_t;

static struct i2c_client *kr3dm_client = NULL;

struct kr3dm_data{
  struct i2c_client client;
};


static struct class *kr3d_dev_class;
static struct input_dev *kr3d_dev_input = NULL;
static int kr3d_sidetapping_enable = 0;


static char kr3dm_i2c_write(unsigned char reg_addr, unsigned char *data, unsigned char len);
static char kr3dm_i2c_read(unsigned char reg_addr, unsigned char *data, unsigned char len);

#undef CONFIG_HAS_EARLYSUSPEND
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>

struct early_suspend kr3dm_sensor_early_suspend;

static void kr3dm_early_suspend(struct early_suspend *h);
static void kr3dm_late_resume(struct early_suspend *h);
#endif

static ssize_t show_sidetap_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	int enable;

	enable = kr3d_sidetapping_enable;
	return sprintf(buf, "%d\n", enable);	
}

static ssize_t store_sidetap_value(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int enable = 0;
	sscanf(buf, "%d", &enable);	
 	//device_init(enable);
 	kr3d_sidetapping_enable = enable;
	return count;
}	

static DEVICE_ATTR(sidetap, 0666, show_sidetap_value, store_sidetap_value );

// set kr3dm bandwidth
int kr3dm_set_bandwidth(char bw) 
{
	int res = 0;
	unsigned char data;

//	res = i2c_smbus_read_word_data(kr3dm_client, CTRL_REG1);
	res = i2c_smbus_read_byte_data(kr3dm_client, CTRL_REG1);
	if (res>=0){         
		data = res & 0x00e7;
	}
	data = data + bw;

	res = kr3dm_i2c_write(CTRL_REG1, &data, 1);
	return res;
}

// read selected bandwidth from kr3dm 
int kr3dm_get_bandwidth(unsigned char *bw) {
  int res = 1;
  //TO DO 
  return res;

}
int kr3dm_set_enable(char mode)
{
	int res = 0;
	unsigned char data;

//	res = i2c_smbus_read_word_data(kr3dm_client, CTRL_REG1);
	res = i2c_smbus_read_byte_data(kr3dm_client, CTRL_REG1);
	if (res>=0){         
		data = res & 0x001f;
	}
	data = mode + data;
	res = kr3dm_i2c_write(CTRL_REG1, &data, 1);
	return res;
}

/** X,Y and Z-axis acceleration data readout 
  \param *acc pointer to \ref kr3dm_acc_t structure for x,y,z data readout
  \note data will be read by multi-byte protocol into a 6 byte structure 
*/
int kr3dm_read_accel_xyz(kr3dm_acc_t * acc)
{
	int res;
	
#if 0
	unsigned char acc_data[6];
	res = kr3dm_i2c_read(AXISDATA_REG,&acc_data[0], 6);

	/*   
	acc->x = (short) (((acc_data[1]) << 8) | acc_data[0]);
	acc->y = (short) (((acc_data[3]) << 8) | acc_data[2]);
	acc->z = (short) (((acc_data[5]) << 8) | acc_data[4]);

	acc->x = acc->x/16;
	acc->y = acc->y/16;
	acc->z = acc->z/16;

	*/

	acc->x= acc_data[1];
	acc->y= acc_data[3];
	acc->z= acc_data[5];	
#else
	unsigned char acc_data[3];

	res = kr3dm_i2c_read(OUT_X,&acc_data[0], 1);
	res = kr3dm_i2c_read(OUT_Y,&acc_data[1], 1);
	res = kr3dm_i2c_read(OUT_Z,&acc_data[2], 1);

	acc->x= (int)((signed char)acc_data[0]);
	acc->y= (int)((signed char)acc_data[1]);
	acc->z= (int)((signed char)acc_data[2]);
#endif
	//printk(KERN_INFO"[accel_xyz] x=[%d], y=[%d], z=[%d]\n", acc->x,acc->y,acc->z);	
	return res;

}


/* Device Initialization  */
int device_init(void/*kr3dm_t *pLis*/) 
{
	int res;
	unsigned char buf[5];
	buf[0]=0x37;
	buf[1]=0x00;
	buf[2]=0x00;
	buf[3]=0x00;
	buf[4]=0x00;
	res = kr3dm_i2c_write(CTRL_REG1, &buf[0], 5);
	return res;
}

int kr3dm_set_range(char range) 
{     
	int err = 0;
	unsigned char buf[2];

	buf[0] = range;
	err = kr3dm_i2c_write(CTRL_REG4, buf, 1);
	return err;
}


/*  i2c write routine for kr3dm */
static char kr3dm_i2c_write(unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	int dummy;
	int i; 
	//printk(KERN_INFO"%s\n", __FUNCTION__);
	if( kr3dm_client == NULL )  /*  No global client pointer? */
		return -1;
	for(i=0;i<len;i++)
	{ 
		dummy = i2c_smbus_write_byte_data(kr3dm_client, reg_addr++, data[i]);
		if(dummy)
		{
			printk(KERN_INFO"[kr3dm] i2c write error\n");
			return dummy; 
		}
	}
	return 0;
}

/*  i2c read routine for kr3dm  */
static char kr3dm_i2c_read(unsigned char reg_addr, unsigned char *data, unsigned char len) 
{
	int dummy=0;
	int i=0;
	//printk(KERN_INFO"%s\n", __FUNCTION__);
	if( kr3dm_client == NULL )  /*  No global client pointer? */
		return -1;
	while(i<len)
	{        
//		dummy = i2c_smbus_read_word_data(kr3dm_client, reg_addr++);
		dummy = i2c_smbus_read_byte_data(kr3dm_client,reg_addr);
		if (dummy>=0)
		{         
			data[i] = dummy & 0x00ff;
			i++;
		} 
		else
		{
			printk(KERN_INFO"[kr3dm] i2c read error\n "); 
			return dummy;
		}
		dummy = len;
	}
	return dummy;
}

/*  read command for KR3DM device file  */
static ssize_t kr3dm_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
#if DEBUG 
	kr3dm_acc_t acc;  
#endif
	if( kr3dm_client == NULL )
		return -1;
#if DEBUG
	kr3dm_read_accel_xyz(&acc);
	printk("X axis: %d\n" , acc.x);
	printk("Y axis: %d\n" , acc.y); 
	printk("Z axis: %d\n" , acc.z);  
#endif
return 0;
}

/*  write command for KR3DM device file */
static ssize_t kr3dm_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	if( kr3dm_client == NULL )
		return -1;
#if DEBUG
	printk("KR3DM should be accessed with ioctl command\n");
#endif
	return 0;
}

/*  open command for KR3DM device file  */
static int kr3dm_open(struct inode *inode, struct file *file)
{
	if( kr3dm_client == NULL)
	{
#if DEBUG
		printk("I2C driver not install\n"); 
#endif
		return -1;
	}
	device_init();

#if DEBUG
	printk("KR3DM has been opened\n");
#endif
	return 0;
}

/*  release command for KR3DM device file */
static int kr3dm_close(struct inode *inode, struct file *file)
{
#if DEBUG 
	printk("KR3DM has been closed\n");  
#endif
	return 0;
}


/*  ioctl command for KR3DM device file */
static int kr3dm_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	char data[6];
	int data_acc[3];
#if DEBUG 
	//printk("kr3dm_ioctl\n");  
#endif

	// check kr3dm_client 
	if( kr3dm_client == NULL)
	{
#if DEBUG
		printk("I2C driver not install\n"); 
#endif
		return -EFAULT;
	}

	/* cmd mapping */

	switch(cmd)
	{

	/*case KR3DM_SELFTEST:
	//TO DO
	return err;*/

	case KR3DM_IOCTL_SET_G_RANGE:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0)
		{
#if DEBUG     
			printk("copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = kr3dm_set_range(*data);
		return err;

	case KR3DM_IOCTL_SET_ENABLE:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0){
#if DEBUG
			printk("copy_to_user error\n");
#endif
			return -EFAULT;
		}
		err = kr3dm_set_enable(*data);
		return err;

	case KR3DM_IOCTL_SET_DELAY:
		if(copy_from_user(data,(unsigned char*)arg,1)!=0)
		{
#if DEBUG
			printk("copy_from_user error\n");
#endif
			return -EFAULT;
		}
		err = kr3dm_set_bandwidth(*data);
		return err;

	case KR3DM_IOCTL_GET_DELAY:
		err = kr3dm_get_bandwidth(data);
		if(copy_to_user((unsigned char*)arg,data,1)!=0)
		{
#if DEBUG
			printk("copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return err;

	case KR3DM_IOCTL_SET_REPORT:
		printk("Click Event Detected!!\n");
		
		input_report_key(kr3d_dev_input, KEY_LEFT, 1);	// Left key Pressed
		input_sync(kr3d_dev_input);
		
		input_report_key(kr3d_dev_input, KEY_LEFT, 0);	// Left key Released
		input_sync(kr3d_dev_input);
		return 0;

	case KR3DM_IOCTL_GET_TAPPING :
		if(copy_to_user((unsigned char*)arg,&kr3d_sidetapping_enable,sizeof(int))!=0)
		{
#if DEBUG
			printk("copy_to_user error\n");
#endif
			return -EFAULT;
		}
		return 0;		

	case KR3DM_IOCTL_READ_ACCEL_XYZ:
		err = kr3dm_read_accel_xyz((kr3dm_acc_t*)data_acc);
		if(copy_to_user(arg, data_acc,sizeof(int)*3) !=0)
		{
#if DEBUG
			printk("copy_to error\n");
#endif
			return -EFAULT;
		}
		return err;

		default:
			return 0;
	}
}


static const struct file_operations kr3dm_fops = {
	.owner = THIS_MODULE,
	.read = kr3dm_read,
	.write = kr3dm_write,
	.open = kr3dm_open,
	.release = kr3dm_close,
	.ioctl = kr3dm_ioctl,
};

int kr3dm_probe(struct i2c_client *client, const struct i2c_device_id * devid)
{
	struct kr3dm_data *data;
	int err = 0;
	int tempvalue;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		goto exit;

	/* OK. For now, we presume we have a valid client. We now create the
	client structure, even though we cannot fill it completely yet. */
	if (!(data = kmalloc(sizeof(struct kr3dm_data), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	} 
	
	memset(data, 0, sizeof(struct kr3dm_data));
	i2c_set_clientdata(client, data);
	kr3dm_client = client;

#if defined(CONFIG_HAS_EARLYSUSPEND)
	kr3dm_sensor_early_suspend.suspend = kr3dm_early_suspend;
	kr3dm_sensor_early_suspend.resume = kr3dm_late_resume;
	register_early_suspend(&kr3dm_sensor_early_suspend);
#endif		
	

//	if (i2c_smbus_read_byte(client) < 0)
//	{
//		printk("i2c_smbus_read_byte error!!\n");
//		goto exit_kfree;
//	}
//	else
//	{
//		printk("KR3DM Device detected!\n");
//	}

	err = device_create_file(&client->dev, &dev_attr_sidetap);

	/* read chip id */
	tempvalue = i2c_smbus_read_byte_data(client,WHO_AM_I);
//	tempvalue = i2c_smbus_read_word_data(client,WHO_AM_I);
	//  if((tempvalue&0x00FF) == 0x0032) 
	if((tempvalue&0x00FF) == 0x0012)  // changed for KR3DM.
	{
		printk("I2C driver registered!\n");
	}
	else
	{
		printk("I2C driver not registered 0x%x!\n", tempvalue);
		kr3dm_client = NULL;
		goto exit_kfree;
	}

	return 0;

	exit_kfree:
	kfree(data);
	exit:
	return err;
}

static int kr3dm_remove(struct i2c_client *client)
{
//int err;
	struct kr3dm_data *lis = i2c_get_clientdata(client);

#if DEBUG
	printk(KERN_INFO "KR3DM driver removing\n");
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
	unregister_early_suspend(&kr3dm_sensor_early_suspend);
#endif	

	device_remove_file(&client->dev, &dev_attr_sidetap);

	kfree(lis);
	kr3dm_client = NULL;
	return 0;
}
#ifdef CONFIG_PM
static int kr3dm_suspend(struct device *device)
{
	int err;
	u8 buf  = PM_OFF;	
	
#if DEBUG
	printk("%s()\n",__FUNCTION__);
#endif
	err = kr3dm_i2c_write(CTRL_REG1, &buf, 1);
	if (err < 0)
		printk("I2C driver ctrl_reg1 write error!\n");

	return 0;
}

static int kr3dm_resume(struct device *device)
{

#if DEBUG
	printk("%s()\n",__FUNCTION__);
#endif
	//TO DO
	device_init();
	return 0;
}
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void kr3dm_early_suspend(struct early_suspend *h)
{
	int err;
	u8 buf	= PM_OFF;	
	
#if DEBUG
	printk("%s()\n",__FUNCTION__);
#endif
	err = kr3dm_i2c_write(CTRL_REG1, &buf, 1);
	if (err < 0)
		printk("I2C driver ctrl_reg1 write error!\n");

	return 0;
}



static void kr3dm_late_resume(struct early_suspend *h)
{

#if DEBUG
	printk("%s()\n",__FUNCTION__);
#endif
	//TO DO
	device_init();
	return 0;
}

#endif

static const struct i2c_device_id kr3dm_id[] = {
        { "KR3DM", 0 },
        { },
};

MODULE_DEVICE_TABLE(i2c, kr3dm_id);

static struct dev_pm_ops kr3dm_pm_ops = {
       .suspend = kr3dm_suspend,
       .resume = kr3dm_resume,
};

static struct i2c_driver kr3dm_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = kr3dm_probe,
	.remove =__devexit_p(kr3dm_remove),
	.id_table = kr3dm_id,
	.driver = {
	.owner = THIS_MODULE,
	.name = "KR3DM",
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
		.pm	= &kr3dm_pm_ops,
#endif	
	},
  //.detect = kr3dm_detect,
  //.address_data = &addr_data,   //Car Ji please check
};

static int __init kr3dm_init(void)
{
	int res;
	struct device *dev;
	// register a char dev  
	res = register_chrdev(KR3DM_MAJOR, "KR3DM", &kr3dm_fops);
	if (res)
		goto out;
	// create lis-dev device class 
	kr3d_dev_class = class_create(THIS_MODULE, "KR3D-dev");
	if (IS_ERR(kr3d_dev_class)) {
		res = PTR_ERR(kr3d_dev_class);
	goto out_unreg_chrdev;
	}

	kr3d_dev_input = input_allocate_device();
	if (kr3d_dev_input == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		res = -ENOMEM;
		goto out_unreg_input;
	}
	
	kr3d_dev_input->name = "KR3DM";

	set_bit(EV_SYN, 	 kr3d_dev_input->evbit);
	set_bit(EV_KEY, 	 kr3d_dev_input->evbit);
	set_bit(KEY_LEFT, kr3d_dev_input->keybit);

	res = input_register_device(kr3d_dev_input);
	if (res < 0) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto err_input_allocate;
	}
	
	// add i2c driver for kr3dm 
	res = i2c_add_driver(&kr3dm_driver);
	if (res)
		goto out_unreg_class;
	// create device node for kr3dm 
	dev = device_create(kr3d_dev_class, NULL,
							MKDEV(KR3DM_MAJOR, 0),
							NULL,
							"KR3DM");
	if (IS_ERR(dev)) {
		res = PTR_ERR(dev);
	goto error_destroy;
	}
	printk(KERN_INFO "KR3DM device created successfully\n");

	return 0;

	error_destroy:
	i2c_del_driver(&kr3dm_driver);
	out_unreg_input:
	input_unregister_device(kr3d_dev_input);
	err_input_allocate :
	input_free_device(kr3d_dev_input);
	kr3d_dev_input = NULL;
	out_unreg_class:
	class_destroy(kr3d_dev_class);
	out_unreg_chrdev:
	unregister_chrdev(KR3DM_MAJOR, "KR3DM");
	out:
	printk(KERN_ERR "%s: Driver Initialization failed\n", __FILE__);
	return res;
	
	res = i2c_add_driver(&kr3dm_driver);
	printk(KERN_INFO "KR3DM device created successfully\n");
	return res;
}

static void __exit kr3dm_exit(void)
{
	i2c_del_driver(&kr3dm_driver);
	input_unregister_device(kr3d_dev_input);
	input_free_device(kr3d_dev_input);
	kr3d_dev_input = NULL;
	class_destroy(kr3d_dev_class);
	unregister_chrdev(KR3DM_MAJOR,"KR3DM");
	printk(KERN_INFO "KR3DM exit\n");
}

module_init(kr3dm_init);
module_exit(kr3dm_exit);

MODULE_DESCRIPTION("kr3dm accelerometer driver");
MODULE_AUTHOR("STMicroelectronics");
MODULE_LICENSE("GPL");

