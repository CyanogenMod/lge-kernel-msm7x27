/* 
* Copyright (C) ST-Ericsson AP Pte Ltd 2010 
*
* ISP1763 Linux HCD Controller driver : hal
* 
* This program is free software; you can redistribute it and/or modify it under the terms of 
* the GNU General Public License as published by the Free Software Foundation; version 
* 2 of the License. 
* 
* This program is distributed in the hope that it will be useful, but WITHOUT ANY  
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more  
* details. 
* 
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. 
* 
* This is the main hardware abstraction layer file. Hardware initialization, interupt
* processing and read/write routines are handled here.
* 
* Author : wired support <wired.support@stericsson.com>
*
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <asm/dma.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>



/*--------------------------------------------------------------*
 *               linux system include files
 *--------------------------------------------------------------*/
#include "hal_msm.h"
#include "../hal/hal_intf.h"
#include "../hal/isp1763.h"




/*--------------------------------------------------------------*
 *               Local variable Definitions
 *--------------------------------------------------------------*/
struct isp1763_dev isp1763_loc_dev[ISP1763_LAST_DEV];
static struct isp1763_hal hal_data;
static u32 pci_io_base = 0;
void *iobase = 0;
int iolength = 0;
static u32 pci_mem_phy0 = 0;
static u32 pci_mem_len = 0x20000;	
static int isp1763_pci_latency;



/*--------------------------------------------------------------*
 *               Local # Definitions
 *--------------------------------------------------------------*/
#define         PCI_ACCESS_RETRY_COUNT  20
#define         ISP1763_DRIVER_NAME     "1763-pci"

/*--------------------------------------------------------------*
 *               Local Function
 *--------------------------------------------------------------*/

static void __devexit isp1763_pci_remove(struct pci_dev *dev);
static int __devinit isp1763_pci_probe(struct pci_dev *dev,
	const struct pci_device_id *id);


/*--------------------------------------------------------------*
 *               PCI Driver Interface Functions
 *--------------------------------------------------------------*/

static const struct pci_device_id __devinitdata isp1763_pci_ids[] = {
	{
	 /* handle PCI BRIDE  manufactured by PLX */
		class:((PCI_CLASS_BRIDGE_OTHER << 8) | (0x06 << 16)),
		class_mask:~0,
	/* no matter who makes it */
		vendor:		/*0x10B5, */ PCI_ANY_ID,
		device:		/*0x5406, */ PCI_ANY_ID,
		subvendor:PCI_ANY_ID,
		subdevice:PCI_ANY_ID,
	},
	{ /* end: all zeroes */ }
};

MODULE_DEVICE_TABLE(pci, isp1763_pci_ids);

/* Pci driver interface functions */
static struct pci_driver isp1763_pci_driver = {
	name:"isp1763-hal",
	id_table:&isp1763_pci_ids[0],
	probe:isp1763_pci_probe,
	remove:isp1763_pci_remove,
};


/*--------------------------------------------------------------*
 *               ISP1763 Read write routine
 *--------------------------------------------------------------*/

/* Write a 32 bit Register of isp1763 */
void
isp1763_reg_write32(struct isp1763_dev *dev, u16 reg, u32 data)
{
	/* Write the 32bit to the register address given to us */

#ifdef DATABUS_WIDTH_16
	writew((u16) data, dev->baseaddress + ((reg)));
	writew((u16) (data >> 16), dev->baseaddress + (((reg + 2))));
#else
	writeb((u8) data, dev->baseaddress + (reg));
	writeb((u8) (data >> 8), dev->baseaddress + ((reg + 1)));
	writeb((u8) (data >> 16), dev->baseaddress + ((reg + 2)));
	writeb((u8) (data >> 24), dev->baseaddress + ((reg + 3)));
#endif

}
EXPORT_SYMBOL(isp1763_reg_write32);


/* Read a 32 bit Register of isp1763 */
u32
isp1763_reg_read32(struct isp1763_dev *dev, u16 reg, u32 data)
{

	data = 0;
#ifdef DATABUS_WIDTH_16
	u16 wvalue1, wvalue2;
	wvalue1 = readw(dev->baseaddress + ((reg)));
	wvalue2 = readw(dev->baseaddress + (((reg + 2))));
	data |= wvalue2;
	data <<= 16;
	data |= wvalue1;
#else
	u8 bval1, bval2, bval3, bval4;

	bval1 = readb(dev->baseaddress + (reg));
	bval2 = readb(dev->baseaddress + (reg + 1));
	bval3 = readb(dev->baseaddress + (reg + 2));
	bval4 = readb(dev->baseaddress + (reg + 3));
	data = 0;
	data |= bval4;
	data <<= 8;
	data |= bval3;
	data <<= 8;
	data |= bval2;
	data <<= 8;
	data |= bval1;

#endif

	return data;
}
EXPORT_SYMBOL(isp1763_reg_read32);


/* Read a 16 bit Register of isp1763 */
u16
isp1763_reg_read16(struct isp1763_dev * dev, u16 reg, u16 data)
{
#ifdef DATABUS_WIDTH_16
	data = readw(dev->baseaddress + ((reg)));
#else
	u8 bval1, bval2;
	bval1 = readb(dev->baseaddress + (reg));
	if (reg == HC_DATA_REG){
		bval2 = readb(dev->baseaddress + (reg));
	} else {
		bval2 = readb(dev->baseaddress + ((reg + 1)));
	}
	data = 0;
	data |= bval2;
	data <<= 8;
	data |= bval1;

#endif
	return data;
}
EXPORT_SYMBOL(isp1763_reg_read16);

/* Write a 16 bit Register of isp1763 */
void
isp1763_reg_write16(struct isp1763_dev *dev, u16 reg, u16 data)
{
#ifdef DATABUS_WIDTH_16
	writew(data, dev->baseaddress + ((reg)));
#else
	writeb((u8) data, dev->baseaddress + (reg));
	if (reg == HC_DATA_REG){
		writeb((u8) (data >> 8), dev->baseaddress + (reg));
	}else{
		writeb((u8) (data >> 8), dev->baseaddress + ((reg + 1)));
	}

#endif
}
EXPORT_SYMBOL(isp1763_reg_write16);

/* Read a 8 bit Register of isp1763 */
u8
isp1763_reg_read8(struct isp1763_dev *dev, u16 reg, u8 data)
{
	data = readb((dev->baseaddress + (reg)));
	return data;
}
EXPORT_SYMBOL(isp1763_reg_read8);

/* Write a 8 bit Register of isp1763 */
void
isp1763_reg_write8(struct isp1763_dev *dev, u16 reg, u8 data)
{
	writeb(data, (dev->baseaddress + (reg)));
}
EXPORT_SYMBOL(isp1763_reg_write8);


/*--------------------------------------------------------------*
 *
 * Module dtatils: isp1763_mem_read
 *
 * Memory read using PIO method.
 *
 *  Input: struct isp1763_driver *drv  -->  Driver structure.
 *                      u32 start_add     --> Starting address of memory
 *              u32 end_add     ---> End address
 *
 *              u32 * buffer      --> Buffer pointer.
 *              u32 length       ---> Length
 *              u16 dir          ---> Direction ( Inc or Dec)
 *
 *  Output     int Length  ----> Number of bytes read
 *
 *  Called by: system function
 *
 *
 *--------------------------------------------------------------*/
/* Memory read function PIO */

int
isp1763_mem_read(struct isp1763_dev *dev, u32 start_add,
	u32 end_add, u32 * buffer, u32 length, u16 dir)
{
	u8 *one = (u8 *) buffer;
	u16 *two = (u16 *) buffer;
	u32 a = (u32) length;
	u32 w;
	u32 w2;
	u8 bvalue;
	u16 wvalue;
	unsigned long ulFlags;

	static int iShowTS = 0;

	if (buffer == 0) {
		printk("Buffer address zero\n");
		return 0;
	}


	isp1763_reg_write16(dev, HC_MEM_READ_REG, start_add);

last:
	w = isp1763_reg_read16(dev, HC_DATA_REG, w);
	w2 = isp1763_reg_read16(dev, HC_DATA_REG, w);
	w2 <<= 16;
	w = w | w2;
	if (a == 1) {
		*one = (u8) w;
		return 0;
	}
	if (a == 2) {
		*two = (u16) w;
		return 0;
	}

	if (a == 3) {
		*two = (u16) w;
		two += 1;
		w >>= 16;
		*two = (u8) (w);
		return 0;

	}
	while (a > 0) {
		*buffer = w;
		a -= 4;
		if (a <= 0) {
			break;
		}
		if (a < 4) {
			buffer += 1;
			one = (u8 *) buffer;
			two = (u16 *) buffer;
			goto last;
		}
		buffer += 1;
		w = isp1763_reg_read16(dev, HC_DATA_REG, w);
		w2 = isp1763_reg_read16(dev, HC_DATA_REG, w);
		w2 <<= 16;
		w = w | w2;
	}
	return ((a < 0) || (a == 0)) ? 0 : (-1);

}
EXPORT_SYMBOL(isp1763_mem_read);


/*--------------------------------------------------------------*
 *
 * Module dtatils: isp1763_mem_write
 *
 * Memory write using PIO method.
 *
 *  Input: struct isp1763_driver *drv  -->  Driver structure.
 *                      u32 start_add     --> Starting address of memory
 *              u32 end_add     ---> End address
 *
 *              u32 * buffer      --> Buffer pointer.
 *              u32 length       ---> Length
 *              u16 dir          ---> Direction ( Inc or Dec)
 *
 *  Output     int Length  ----> Number of bytes read
 *
 *  Called by: system function
 *
 *
 *--------------------------------------------------------------*/

/* Memory read function IO */

int
isp1763_mem_write(struct isp1763_dev *dev,
	u32 start_add, u32 end_add, u32 * buffer, u32 length, u16 dir)
{
	int a = length;
	u8 *temp = (u8 *) buffer;
	u16 *temp16 = (u8 *) buffer;
	u8 one = (u8) (*buffer);
	u16 two = (u16) (*buffer);
	static int iCount = 0;
	int fJump = 0;
	unsigned long ulFlags;

	if (buffer == NULL || length == 0) {
		printk("Wrong length or buffer pointer\n");
		return -1;
	}

	isp1763_reg_write16(dev, HC_MEM_READ_REG, start_add);
	a = (int) (length);
      last_write:

	if (a == 1) {
		isp1763_reg_write16(dev, HC_DATA_REG, one);
		iCount++;
		return 0;
	}
	if (a == 2) {
		isp1763_reg_write16(dev, HC_DATA_REG, two);
		iCount++;
		return 0;
	}


	while (a > 0) {
		isp1763_reg_write16(dev, HC_DATA_REG, (*(u16 *) temp));
		a -= 2;
		if (a <= 2) {
			if (temp16 != temp){
				printk("Offset is wrong!!" 
					"temp=0x%X temp16=0x%X\n", temp,temp16);
			}
			temp += 2;
			temp16++;
			two = *temp16;
			one = *temp;
			fJump = 1;
			goto last_write;
		}
		temp += 2;
		temp16++;

	}

	return ((a < 0) || (a == 0)) ? 0 : (-1);

}
EXPORT_SYMBOL(isp1763_mem_write);


/*--------------------------------------------------------------*
 *
 * Module dtatils: isp1763_register_driver
 *
 * This function is used by top driver (OTG, HCD, DCD) to register
 * their communication functions (probe, remove, suspend, resume) using
 * the drv data structure.
 * This function will call the probe function of the driver if the ISP1763
 * corresponding to the driver is enabled
 *
 *  Input: struct isp1763_driver *drv  --> Driver structure.
 *  Output result
 *         0= complete
 *         1= error.
 *
 *  Called by: system function module_init
 *
 *
 *--------------------------------------------------------------*/

int
isp1763_register_driver(struct isp1763_driver *drv)
{
	struct isp1763_dev *dev;
	int result;
	isp1763_id *id;

	hal_entry("%s: Entered\n", __FUNCTION__);
	info("isp1763_register_driver(drv=%p) \n", drv);

	if (!drv) {
		return -EINVAL;
	}
	dev = &isp1763_loc_dev[drv->index];

	dev->active = 1;	/* set the driver as active*/

	if (drv->probe) {
		result = drv->probe(dev, drv->id);
	} else {
		printk("%s no probe function for indes %d \n", __FUNCTION__,
			drv->index);
	}

	if (result >= 0) {
		printk(KERN_INFO __FILE__ ": Registered Driver %s\n",
			drv->name);
		dev->driver = drv;
	}
	hal_entry("%s: Exit\n", __FUNCTION__);
	return result;
}				/* End of isp1763_register_driver */
EXPORT_SYMBOL(isp1763_register_driver);


/*--------------------------------------------------------------*
 *
 * Module dtatils: isp1763_unregister_driver
 *
 * This function is used by top driver (OTG, HCD, DCD) to de-register
 * their communication functions (probe, remove, suspend, resume) using
 * the drv data structure.
 * This function will check whether the driver is registered or not and
 * call the remove function of the driver if registered
 *
 *  Input: struct isp1763_driver *drv  --> Driver structure.
 *  Output result
 *         0= complete
 *         1= error.
 *
 *  Called by: system function module_init
 *
 *
 *--------------------------------------------------------------*/

void
isp1763_unregister_driver(struct isp1763_driver *drv)
{
	struct isp1763_dev *dev;
	hal_entry("%s: Entered\n", __FUNCTION__);

	info("isp1763_unregister_driver(drv=%p)\n", drv);
	dev = &isp1763_loc_dev[drv->index];
	if (dev->driver == drv) {
		/* driver registered is same as the requestig driver */
		drv->remove(dev);
		dev->driver = NULL;
		info(": De-registered Driver %s\n", drv->name);
		return;
	}
	hal_entry("%s: Exit\n", __FUNCTION__);
}				/* End of isp1763_unregister_driver */
EXPORT_SYMBOL(isp1763_unregister_driver);


/*--------------------------------------------------------------*
 *               ISP1763 PCI driver interface routine.
 *--------------------------------------------------------------*/


/*--------------------------------------------------------------*
 *
 *  Module dtatils: isp1763_pci_module_init
 *
 *  This  is the module initialization function. It registers to
 *  PCI driver for a PLX PCI bridge device. And also resets the
 *  internal data structures before registering to PCI driver.
 *
 *  Input: void
 *  Output result
 *         0= complete
 *         1= error.
 *
 *  Called by: system function module_init
 *
 *
 *
 -------------------------------------------------------------------*/
static int __init
isp1763_pci_module_init(void)
{
	int result = 0;
	hal_entry("%s: Entered\n", __FUNCTION__);
	printk(KERN_NOTICE "+isp1763_pci_module_init \n");
	memset(isp1763_loc_dev, 0, sizeof(isp1763_loc_dev));

	if ((result = pci_register_driver(&isp1763_pci_driver)) < 0) {
		printk("PCI Iinitialization Fail(error = %d)\n", result);
		return result;
	} else{
		info(":%s PCI Initialization Success \n", ISP1763_DRIVER_NAME);
	}

	printk(KERN_NOTICE "-isp1763_pci_module_init \n");
	hal_entry("%s: Exit\n", __FUNCTION__);
	return result;
}

/*--------------------------------------------------------------*
 *
 *  Module dtatils: isp1763_pci_module_cleanup
 *
 * This  is the module cleanup function. It de-registers from
 * PCI driver and resets the internal data structures.
 *
 *  Input: void
 *  Output void
 *
 *  Called by: system function module_cleanup
 *
 *
 *
 --------------------------------------------------------------*/

static void __exit
isp1763_pci_module_cleanup(void)
{
	u32 intcsr;
	struct isp1763_dev *dev = &isp1763_loc_dev[0];
	printk("Hal Module Cleanup\n");
	intcsr = readl(iobase + 0x68);
	intcsr &= ~0x900;
	writel(intcsr, iobase + 0x68);
	pci_unregister_driver(&isp1763_pci_driver);

	memset(isp1763_loc_dev, 0, sizeof(isp1763_loc_dev));
}


/*--------------------------------------------------------------*
 *
 *  Module dtatils: isp1763_pci_probe
 *
 * PCI probe function of ISP1763
 * This function is called from PCI Driver as an initialization function
 * when it founds the PCI device. This functions initializes the information
 * for the 3 Controllers with the assigned resources and tests the register
 * access to these controllers and do a software reset and makes them ready
 * for the drivers to play with them.
 *
 *  Input:
 *              struct pci_dev *dev                     ----> PCI Devie data structure
 *      const struct pci_device_id *id  ----> PCI Device ID
 *  Output void
 *
 *  Called by: system function module_cleanup
 *
 *
 *
 --------------------------------------------------------------**/

static int __devinit
isp1763_pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	u8 latency, limit;
	u32 reg_data = 0;
	int retry_count;
	struct isp1763_dev *loc_dev;
	void *address = 0;
	int length = 0;
	int status = 1;
	u32 ul_busregion_descr = 0;
	u32 hwmodectrl = 0;
	u16 us_reset_hc = 0;
	u32 chipid = 0;
	u32 ureadVal = 0;
	hal_entry("%s: Entered\n", __FUNCTION__);

	hal_init(("isp1763_pci_probe(dev=%p)\n", dev));
	printk(KERN_NOTICE "+isp1763_pci_probe \n");
	if (pci_enable_device(dev) < 0) {
		err("failed in enabing the device\n");
		return -ENODEV;
	}
	if (!dev->irq) {
		err("found ISP1763 device with no IRQ assigned.");
		err("check BIOS settings!");
		return -ENODEV;
	}
	/* Grab the PLX PCI mem maped port start address we need  */
	pci_io_base = pci_resource_start(dev, 0);
	printk("isp1763 pci IO Base= 0x%X\n", pci_io_base);

	iolength = pci_resource_len(dev, 0);
	printk(KERN_NOTICE "isp1763 pci io length 0x%X\n", iolength);
	hal_init(("isp1763 pci io length 0x%X\n", iolength));
	printk("Before calling request_mem_region for pci_io_base\n");

	if (!request_mem_region(pci_io_base, iolength, "ISP1763 IO MEM")) {
		err("host controller already in use1\n");
		return -EBUSY;
	}
	iobase = ioremap_nocache(pci_io_base, iolength);
	if (!iobase) {
		err("can not map io memory to system memory\n");
		release_mem_region(pci_io_base, iolength);
		return -ENOMEM;
	}
	/* Grab the PLX PCI shared memory of the isp1763 we need  */
	printk("Before calling pci_resource_start for 3\n");
	pci_mem_phy0 = pci_resource_start(dev, 3);
	printk("isp1763 pci base address = %x\n", pci_mem_phy0);

	/* Get the Host Controller IO and INT resources
	 */
	loc_dev = &(isp1763_loc_dev[ISP1763_HC]);
	loc_dev->irq = dev->irq;
	loc_dev->io_base = pci_mem_phy0;
	loc_dev->start = pci_mem_phy0;
	loc_dev->length = pci_mem_len;
	loc_dev->io_len = pci_mem_len;	/*64K */
	loc_dev->index = ISP1763_HC;	/*zero */

	length = pci_resource_len(dev, 3);
	printk(KERN_NOTICE "isp1763 pci resource length %x\n", length);
	if (length < pci_mem_len) {
		err("memory length for this resource is less than required\n");
		release_mem_region(pci_io_base, iolength);
		iounmap(iobase);
		return -ENOMEM;

	}
	loc_dev->io_len = length;
	if (check_mem_region(loc_dev->io_base, length) < 0) {
		err("host controller already in use\n");
		release_mem_region(pci_io_base, iolength);
		iounmap(iobase);
		return -EBUSY;
	}
	if (!request_mem_region(loc_dev->io_base, length,ISP1763_DRIVER_NAME)){
		err("host controller already in use\n");
		release_mem_region(pci_io_base, iolength);
		iounmap(iobase);
		return -EBUSY;

	}

	/*map available memory */
	address = ioremap_nocache(pci_mem_phy0, length);
	if (address == NULL) {
		err("memory map problem\n");
		release_mem_region(pci_io_base, iolength);
		iounmap(iobase);
		release_mem_region(loc_dev->io_base, length);
		return -ENOMEM;
	}

	loc_dev->baseaddress = (u8 *) address;
	loc_dev->dmabase = (u8 *) iobase;

	hal_init(("isp1763 HC MEM Base= %p irq = %d\n",
		loc_dev->baseaddress, loc_dev->irq));


	chipid = isp1763_reg_read32(loc_dev, DC_CHIPID, chipid);
	printk("START: chip id:%x \n", chipid);

	u16 us_fpga_conf;


#ifdef DATABUS_WIDTH_16
	/* Change from 0xF8 to 0x18 to align with WinCE kit */
	ul_busregion_descr = readl(iobase + 0x18);
	printk(KERN_NOTICE "setting plx bus width to 16:BusRegionDesc %x \n",
		ul_busregion_descr);
	ul_busregion_descr &= 0xFFFFFFFC;
	ul_busregion_descr |= 0x00000001;
	writel(ul_busregion_descr, iobase + 0x18);
	ul_busregion_descr = readl(iobase + 0x18);
	printk(KERN_NOTICE "BusRegionDesc %x\n", ul_busregion_descr);


#else /* we are in 8-bit mode*/
	ul_busregion_descr = readl(iobase + 0xF8);
	printk(KERN_NOTICE "setting plx bus width to 8:BusRegionDesc %x !!\n",
		ul_busregion_descr);
	ul_busregion_descr &= 0xFFFFFFFC;
	writel(ul_busregion_descr, iobase + 0xF8);
	printk(KERN_NOTICE "BusRegionDesc %x \n", ul_busregion_descr);
#endif /*DATABUS_WIDTH_16*/


	chipid = isp1763_reg_read32(loc_dev, DC_CHIPID, chipid);
	printk("After setting plx, chip id:%x \n", chipid);


#ifdef DATABUS_WIDTH_16		/*FPGA settings*/

	isp1763_reg_write16(loc_dev, FPGA_CONFIG_REG, 0xBF);
	us_fpga_conf =
		isp1763_reg_read16(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 1 %x \n", us_fpga_conf);

	isp1763_reg_write16(loc_dev, FPGA_CONFIG_REG, 0x3F);
	us_fpga_conf =
		isp1763_reg_read16(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 2 %x \n", us_fpga_conf);
	mdelay(5);

	isp1763_reg_write16(loc_dev, FPGA_CONFIG_REG, 0xFF);
	us_fpga_conf =
		isp1763_reg_read16(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 3 %x \n", us_fpga_conf);
	mdelay(1);


#else

	us_fpga_conf =
		isp1763_reg_read8(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "INIT FPGA CONF REG 1 %x \n", us_fpga_conf);
	isp1763_reg_write8(loc_dev, FPGA_CONFIG_REG, 0xB7);
	us_fpga_conf =
		isp1763_reg_read8(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 1 %x \n", us_fpga_conf);

	isp1763_reg_write8(loc_dev, FPGA_CONFIG_REG, 0x37);
	us_fpga_conf =
		isp1763_reg_read8(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 2 %x \n", us_fpga_conf);
	mdelay(5);

	isp1763_reg_write8(loc_dev, FPGA_CONFIG_REG, 0xF7);
	us_fpga_conf =
		isp1763_reg_read8(loc_dev, FPGA_CONFIG_REG, us_fpga_conf);
	printk(KERN_NOTICE "FPGA CONF REG 3 %x \n", us_fpga_conf);
	mdelay(1);

#endif

	chipid = isp1763_reg_read32(loc_dev, DC_CHIPID, chipid);
	printk("After setting fpga , chip id:%x \n", chipid);

	/*reset the host controller  */
	printk("RESETTING\n");
	us_reset_hc |= 0x1;
	isp1763_reg_write16(loc_dev, 0xB8, us_reset_hc);
	mdelay(20);
	us_reset_hc = 0;
	us_reset_hc |= 0x2;
	isp1763_reg_write16(loc_dev, 0xB8, us_reset_hc);

	chipid = isp1763_reg_read32(loc_dev, DC_CHIPID, chipid);
	printk("after HC reset, chipid:%x \n", chipid);

	mdelay(20);
	hwmodectrl = isp1763_reg_read16(loc_dev, HC_HW_MODE_REG, hwmodectrl);
	printk("Mode Ctrl Value b4 setting buswidth: %x\n", hwmodectrl);
#ifdef DATABUS_WIDTH_16
	hwmodectrl &= 0xFFEF;	/*enable the 16 bit bus */
#else
	printk("Setting 8-BIT mode \n");
	hwmodectrl |= 0x0010;	/*enable the 8 bit bus */
#endif
	isp1763_reg_write16(loc_dev, HC_HW_MODE_REG, hwmodectrl);
	printk("writing 0x%x to hw mode reg \n", hwmodectrl);

	hwmodectrl = isp1763_reg_read16(loc_dev, HC_HW_MODE_REG, hwmodectrl);
	mdelay(100);

	printk("Mode Ctrl Value after setting buswidth: %x\n", hwmodectrl);


	chipid = isp1763_reg_read32(loc_dev, DC_CHIPID, chipid);
	printk("after setting HW MODE to 8bit, chipid:%x \n", chipid);



	hal_init(("isp1763 DC MEM Base= %lx irq = %d\n",
		loc_dev->io_base, loc_dev->irq));
	reg_data = isp1763_reg_read16(loc_dev, HC_SCRATCH_REG, reg_data);
	printk("Scratch register is 0x%x \n", reg_data);
	reg_data = 0xABCD;
	isp1763_reg_write16(loc_dev, HC_SCRATCH_REG, reg_data);
	reg_data = isp1763_reg_read16(loc_dev, HC_SCRATCH_REG, reg_data);
	printk("After write, Scratch register is 0x%x \n", reg_data);

	/* bad pci latencies can contribute to overruns */
	pci_read_config_byte(dev, PCI_LATENCY_TIMER, &latency);
	if (latency) {
		pci_read_config_byte(dev, PCI_MAX_LAT, &limit);
		if (limit && limit < latency) {
			dbg("PCI latency reduced to max %d", limit);
			pci_write_config_byte(dev, PCI_LATENCY_TIMER, limit);
			isp1763_pci_latency = limit;
		} else {
			/* it might already have been reduced */
			isp1763_pci_latency = latency;
		}
	}

	/* Try to check whether we can access Scratch Register of
	 * Host Controller or not. The initial PCI access is retried until
	 * local init for the PCI bridge is completed
	 */

	loc_dev = &(isp1763_loc_dev[ISP1763_HC]);
	retry_count = PCI_ACCESS_RETRY_COUNT;

	reg_data = 0;

	while (reg_data < 0xFFFF) {
		isp1763_reg_write16(loc_dev, HC_SCRATCH_REG, reg_data);	
		udelay(1);
		chipid = isp1763_reg_read16(loc_dev, DC_CHIPID, chipid);
		if (chipid != 0x6310){
			hal_init(("CHIP ID WRONG:%x \n", chipid));
		}
		ureadVal =
			isp1763_reg_read16(loc_dev, HC_SCRATCH_REG, ureadVal);
		if (reg_data != ureadVal){
			hal_init(("MisMatch Scratch Value %x ActVal %x\n",
				  ureadVal, reg_data));
		}
		reg_data++;
	}

	memcpy(loc_dev->name, ISP1763_DRIVER_NAME, sizeof(ISP1763_DRIVER_NAME));
	loc_dev->name[sizeof(ISP1763_DRIVER_NAME)] = 0;

	info("controller address %p\n", &dev->dev);
	/*keep a copy of pcidevice */
	loc_dev->pcidev = dev;

	pci_set_master(dev);
	hal_data.irq_usage = 0;
	pci_set_drvdata(dev, loc_dev);

	/* PLX DMA Test */

/*initialize tasklet*/
/*register for interrupts*/
	/* Interrupt will be registered by HCD directly due to 2.6.28 kernel nature*/

	printk(KERN_NOTICE "-isp1763_pci_probe\n");
	hal_entry("%s: Exit\n", __FUNCTION__);
	return 1;

	clean:
	release_mem_region(pci_io_base, iolength);
	iounmap(iobase);
	release_mem_region(loc_dev->io_base, loc_dev->io_len);
	iounmap(loc_dev->baseaddress);
	hal_entry("%s: Exit\n", __FUNCTION__);
	return status;
}				/* End of isp1763_pci_probe */


/*--------------------------------------------------------------*
 *
 *  Module dtatils: isp1763_pci_remove
 *
 * PCI cleanup function of ISP1763
 * This function is called from PCI Driver as an removal function
 * in the absence of PCI device or a de-registration of driver.
 * This functions checks the registerd drivers (HCD, DCD, OTG) and calls
 * the corresponding removal functions. Also initializes the local variables
 * to zero.
 *
 *  Input:
 *              struct pci_dev *dev                     ----> PCI Devie data structure
 *
 *  Output void
 *
 *  Called by: system function module_cleanup
 *
 *
 *
 --------------------------------------------------------------*/
static void __devexit
isp1763_pci_remove(struct pci_dev *dev)
{
	struct isp1763_dev *loc_dev;
	hal_init(("isp1763_pci_remove(dev=%p)\n", dev));


	/*Lets handle the host first */
	loc_dev = &isp1763_loc_dev[ISP1763_HC];
	/*free the memory occupied by host */
	release_mem_region(loc_dev->io_base, loc_dev->io_len);
	release_mem_region(pci_io_base, iolength);
	/*unmap the occupied memory resources */
	iounmap(loc_dev->baseaddress);
	/* unmap the occupied io resources */
	iounmap(iobase);
	return;
}				/* End of isp1763_pci_remove */


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_init(isp1763_pci_module_init);
module_exit(isp1763_pci_module_cleanup);
