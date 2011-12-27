/*
 *
 *
 *
 */

#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <mach/board_lge.h>

extern int lge_erase_block(int ebnum);
extern int lge_write_block(int ebnum, unsigned char *buf, size_t size);
extern int lge_read_block(int ebnum, unsigned char *buf);
extern int init_mtd_access(int partition, int block);

static int tolk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long magic_number = 0;
	unsigned *vir_addr;

	printk("%s:factory reset magic num from android=%s\n",__func__,buf);
	magic_number = simple_strtoul(buf,NULL,16);
	printk("magic_number = %lu\n",magic_number);
#if defined (CONFIG_MACH_MSM7X27_MUSCAT) || defined (CONFIG_MACH_MSM7X27_JUMP)
	vir_addr = ioremap(0xffff000, PAGE_SIZE);
#else
	vir_addr = ioremap(0x2ffff000, PAGE_SIZE);
#endif
	(*(unsigned int *)vir_addr) = magic_number;

	return count;
}
static DEVICE_ATTR(tolk, 0664, NULL, tolk_store);

static unsigned int lcdbe = 1;
static int lcdbe_mode(char *test)
{
	if(!strncmp("off", test, 3))
		lcdbe = 0;
	else
		lcdbe = 1;
	return 0;
}
__setup("lge.lcd=", lcdbe_mode);

static int lcdbe_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", lcdbe);
}
static DEVICE_ATTR(lcdis, 0664, lcdbe_show, NULL);

static unsigned int test_result = 0;
static int result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int a, b, c, d;
	unsigned char *buf11;
	
	buf11 = (unsigned char*)kmalloc(2048,GFP_KERNEL);
	if(buf11==NULL)
		return -1;
	
	init_mtd_access(4, 7);
	lge_read_block(7, buf11);

	a = buf11[0] & 0x000000ff;
	b = (buf11[1] << 8) & 0x0000ff00;
	c = (buf11[2] << 16) & 0x00ff0000;
	d = (buf11[3] << 24) & 0xff000000;

	test_result = a + b + c + d;
	kfree(buf11);

	return sprintf(buf, "%d\n", test_result);
}

static int result_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned char *buf11;

	buf11 = (unsigned char*)kmalloc(2048,GFP_KERNEL);
	if(buf11==NULL)
		return -1;

	memset((void *)buf11, 0, sizeof(buf11));

	sscanf(buf, "%d\n", &test_result);

	buf11[0] = (unsigned char)(0x000000ff & test_result);
	buf11[1] = (unsigned char)(0x000000ff & (test_result>>8));
	buf11[2] = (unsigned char)(0x000000ff & (test_result>>16));
	buf11[3] = (unsigned char)(0x000000ff & (test_result>>24));

	init_mtd_access(4, 7);
	lge_erase_block(7);
	lge_write_block(7, buf11, 2048);
	kfree(buf11);
	return count;
}
static DEVICE_ATTR(result, 0664, result_show, result_store);

static unsigned int g_flight = 0;
static int flight_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	return sprintf(buf, "%d\n", g_flight);
}

static int flight_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int test_result=0;
	
	sscanf(buf, "%d\n", &test_result);
	g_flight = test_result;	
	
	return count;
}
static DEVICE_ATTR(flight, 0664, flight_show, flight_store);

static int get_qem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned value;

	value = lge_get_nv_qem();
	printk("%s: qem=%d\n",__func__,value);
	
	return sprintf(buf, "%d\n", value);
}
static DEVICE_ATTR(qem, 0664, get_qem_show, NULL);

static int __init lge_tempdevice_probe(struct platform_device *pdev)
{
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_result);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);
	
	err = device_create_file(&pdev->dev, &dev_attr_lcdis);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);

	err = device_create_file(&pdev->dev, &dev_attr_tolk);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);

	err = device_create_file(&pdev->dev, &dev_attr_flight);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);
	
	err = device_create_file(&pdev->dev, &dev_attr_qem);
	if (err < 0) 
		printk("%s : Cannot create the sysfs\n", __func__);
	return err;
}

static struct platform_device lgetemp_device = {
	.name = "autoall",
	.id		= -1,
};

static struct platform_driver this_driver __refdata = {
	.probe = lge_tempdevice_probe,
	.driver = {
		.name = "autoall",
	},
};

int __init lge_tempdevice_init(void)
{
	printk("%s\n", __func__);
	platform_device_register(&lgetemp_device);

	return platform_driver_register(&this_driver);
}

module_init(lge_tempdevice_init);
MODULE_DESCRIPTION("Just temporal code for PV");
MODULE_AUTHOR("MoonCheol Kang <neo.kang@lge.com>");
MODULE_LICENSE("GPL");
