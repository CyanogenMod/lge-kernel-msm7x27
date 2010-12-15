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


#ifdef ENABLE_PLX_DMA
u32 plx9054_reg_read(u32 reg);
void plx9054_reg_write(u32 reg, u32 data);

u8 *g_pDMA_Write_Buf = 0;
u8 *g_pDMA_Read_Buf = 0;

#endif


/*--------------------------------------------------------------*
 *               Local variable Definitions
 *--------------------------------------------------------------*/
char IsrName[] = "isp1763 HAL";
struct isp1763_dev isp1763_loc_dev[ISP1763_LAST_DEV];
static struct isp1763_hal hal_data;
static u32 pci_io_base = 0;
void *iobase = 0;
int iolength = 0;
static u32 pci_mem_phy0 = 0;
static u32 pci_mem_len = 0x20000;	
static int isp1763_pci_latency;
struct tasklet_struct irq_tasklet;
spinlock_t hal_lock;
spinlock_t rw_lock;



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

static irqreturn_t isp1763_pci_isr(int irq, void *dev_id, 
	struct pt_regs *regs);


/*--------------------------------------------------------------*
 *               ISP1763 Interrupt Service Routines
 *--------------------------------------------------------------*/
/*tasklet handler*/
static void
hal_handle_int(unsigned long pass)

{
	unsigned long ulFlags;
	struct isp1763_dev *dev;

	spin_lock_irqsave(&hal_lock, ulFlags);

	dev = &isp1763_loc_dev[ISP1763_HC];
	if (dev->active) {
		if (dev->int_reg & ~HC_OTG_INTERRUPT) {
			if (dev->handler) {
#ifdef LINUX_2620
				dev->handler(dev, dev->isr_data, NULL);
#else
				dev->handler(dev, dev->isr_data);
#endif
			}

		}
	}

	dev = &isp1763_loc_dev[ISP1763_DC];
	if (dev->active) {
		if (dev->int_reg) {

			if (dev->handler) {
#ifdef LINUX_2620
				dev->handler(dev, dev->isr_data, NULL);
#else
				dev->handler(dev, dev->isr_data);
#endif
			}
		}

	}

	dev = &isp1763_loc_dev[ISP1763_OTG];
	if (dev->active) {
		printk("%s Error: OTG is active \n", __FUNCTION__);

	}

	spin_unlock_irqrestore(&hal_lock, ulFlags);

}



/* Interrupt Service Routine of isp1763
 * Reads the source of interrupt and calls the corresponding driver's ISR.
 * Before calling the driver's ISR clears the source of interrupt.
 * The drivers can get the source of interrupt from the dev->int_reg field
 */
irqreturn_t
isp1763_pci_isr(int irq, void *__data, struct pt_regs *r)
{
	u32 irq_mask = 0;
	struct isp1763_dev *dev;

	u32 int_enable;
	u32 int_status = IRQ_NONE;

	/* Process the Host Controller Driver */
	dev = &isp1763_loc_dev[ISP1763_HC];

	if (dev->active) {
		/* Get the source of interrupts for Host Controller */
		dev->int_reg =
			isp1763_reg_read16(dev,HC_INTERRUPT_REG, dev->int_reg);

		if (dev->int_reg != 0) {

			if (dev->int_reg & HC_ATL_INTERRUPT) {
				dev->atl_donemap_reg |=
					isp1763_reg_read16(dev,
						HC_ATL_PTD_DONEMAP_REG,
						dev->atl_donemap_reg);
			}
			if (dev->int_reg & HC_INTL_INTERRUPT) {
				dev->intl_donemap_reg |=
					isp1763_reg_read16(dev,
						HC_INT_PTD_DONEMAP_REG,
						dev->intl_donemap_reg);
			}
			if (dev->int_reg & HC_ISO_INTERRUPT) {
				dev->iso_donemap_reg |=
					isp1763_reg_read16(dev,
						HC_ISO_PTD_DONEMAP_REG,
						dev->iso_donemap_reg);
			}

			isp1763_reg_write16(dev, HC_INTERRUPT_REG,
				dev->int_reg);

			irq_mask =
				isp1763_reg_read16(dev, HC_INTENABLE_REG,
					irq_mask);
			dev->int_reg &= irq_mask;	/*shared irq */
			int_status = IRQ_HANDLED;
		}
	}

	/*Process the Device Controller */
	dev = &isp1763_loc_dev[ISP1763_DC];

	if (dev->active) {
		/*unblock the device interrupt */
		isp1763_reg_write16(dev, DEV_UNLOCK_REGISTER, 0xaa37);
		dev->int_reg =
			isp1763_reg_read32(dev, DEV_INTERRUPT_REGISTER,
				dev->int_reg);
		if (dev->int_reg != 0) {
			int_enable =
				isp1763_reg_read32(dev, INT_ENABLE_REGISTER,
					int_enable);
			hal_int("isp1763_pci_dc_isr:INTERRUPT_REGISTER 0x%x\n",
				dev->int_reg);

			/*clear the interrupt source */
			isp1763_reg_write32(dev, DEV_INTERRUPT_REGISTER,
				dev->int_reg);
			int_status = IRQ_HANDLED;
		}

	}

	/*Process the OTG Controller */
	dev = &isp1763_loc_dev[ISP1763_OTG];

	if (dev->active) {
		printk("%s Error: OTG is active \n", __FUNCTION__);
	}

/* schedule the tasklet*/
	if (int_status != IRQ_NONE) {
		tasklet_schedule(&irq_tasklet);
	}


	return int_status;
}				/* End of isp1763_pci_isr */

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

/* Access PLX9054 Register */
void
plx9054_reg_write(u32 reg, u32 data)
{
	writel(data, iobase + (reg));
}
EXPORT_SYMBOL(plx9054_reg_write);


void
plx9054_reg_writeb(u32 reg, u32 data)
{
	writeb(data, iobase + (reg));
}
EXPORT_SYMBOL(plx9054_reg_writeb);

u32
plx9054_reg_read(u32 reg)
{
	u32 uData;
	u32 uDataLow, uDataHigh;

	uData = readl(iobase + (reg));

	return uData;
}
EXPORT_SYMBOL(plx9054_reg_read);


u8
plx9054_reg_readb(u32 reg)
{
	u8 bData;

	bData = readb(iobase + (reg));

	return bData;
}
EXPORT_SYMBOL(plx9054_reg_readb);

#ifdef ENABLE_PLX_DMA

int 
isp1763_mem_read_dma(struct isp1763_dev *dev, u32 start_add,
	u32 end_add, u32 * buffer, u32 length, u16 dir)
{
	u8 *pDMABuffer = 0;
	u32 uPhyAddress = 0;


	/* Set start memory location for write*/
	isp1763_reg_write16(dev, HC_MEM_READ_REG, start_add);
	udelay(1);

	/* Malloc DMA safe buffer and convert to PHY Address*/
	pDMABuffer = g_pDMA_Read_Buf;

	if (pDMABuffer == NULL) {
		printk("Cannnot allocate DMA safe memory for DMA read\n");
		return -1;
	}
	uPhyAddress = virt_to_phys(pDMABuffer);

	/* Program DMA transfer*/

	/*DMA CHANNEL 1 PCI ADDRESS */
	plx9054_reg_write(0x98, uPhyAddress);

	/*DMA CHANNEL 1 LOCAL ADDRESS */
	plx9054_reg_write(0x9C, 0x40);

	/*DMA CHANNEL 1 TRANSFER SIZE */
	plx9054_reg_write(0xA0, length);

	/*DMA CHANNEL 1 DESCRIPTOR POINTER */
	plx9054_reg_write(0xA4, 0x08);

	/*DMA THRESHOLD */
	plx9054_reg_write(0xB0, 0x77220000);

	/*DMA CHANNEL 1 COMMAND STATUS */
	plx9054_reg_writeb(0xA9, 0x03);


	u32 ulDmaCmdStatus, ulDmaIntrStatus, fDone = 0;

	do {
		ulDmaCmdStatus = plx9054_reg_read(0xA8);	

		if ((ulDmaCmdStatus & 0x00001000)) {
			ulDmaCmdStatus |= 0x00000800;
			plx9054_reg_write(0xA8, ulDmaCmdStatus);
			fDone = 1;
		} else {
			
			fDone = 0;
		}
	} while (fDone == 0);

	/* Copy DMA buffer to upper layer buffer*/
	memcpy(buffer, pDMABuffer, length);

	
	return 0;
}

int
isp1763_mem_write_dma(struct isp1763_dev *dev,
	u32 start_add, u32 end_add, u32 * buffer, u32 length, u16 dir)
{
	u8 *pDMABuffer = 0;
	u8 bDmaCmdStatus = 0;
	u32 uPhyAddress;

	isp1763_reg_write16(dev, HC_MEM_READ_REG, start_add);
	udelay(1);		

	/* Malloc DMA safe buffer and convert to PHY Address*/
	pDMABuffer = g_pDMA_Write_Buf;

	if (pDMABuffer == NULL) {
		printk("Cannnot allocate DMA safe memory for DMA write\n");
		return -1;
	}
	/* Copy content to DMA safe buffer*/
	memcpy(pDMABuffer, buffer, length);	
	uPhyAddress = virt_to_phys(pDMABuffer);


	/*DMA CHANNEL 1 PCI ADDRESS */
	plx9054_reg_write(0x98, uPhyAddress);

	/*DMA CHANNEL 1 LOCAL ADDRESS */
	plx9054_reg_write(0x9C, 0x40);

	/*DMA CHANNEL 1 TRANSFER SIZE */
	plx9054_reg_write(0xA0, length);

	/*DMA CHANNEL 1 DESCRIPTOR POINTER */
	plx9054_reg_write(0xA4, 0x00);

	/*DMA THRESHOLD */
	plx9054_reg_write(0xB0, 0x77220000);

	/*DMA CHANNEL 1 COMMAND STATUS */
	bDmaCmdStatus = plx9054_reg_readb(0xA9);
	bDmaCmdStatus |= 0x03;
	plx9054_reg_writeb(0xA9, bDmaCmdStatus);


	u32 ulDmaCmdStatus, ulDmaIntrStatus, fDone = 0;

	do {
		ulDmaCmdStatus = plx9054_reg_read(0xA8);	

		if ((ulDmaCmdStatus & 0x00001000)){
			ulDmaCmdStatus |= 0x00000800;
			plx9054_reg_write(0xA8, ulDmaCmdStatus);
			fDone = 1;
		} else {
			fDone = 0;
		}
	} while (fDone == 0);

	return 0;
}

#endif

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


#ifdef ENABLE_PLX_DMA

	isp1763_mem_read_dma(dev, start_add, end_add, buffer, length, dir);
	a = 0;

#else
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
#endif
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
#ifdef ENABLE_PLX_DMA

	isp1763_mem_write_dma(dev, start_add, end_add, buffer, length, dir);
	a = 0;

#else

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
#endif

	return ((a < 0) || (a == 0)) ? 0 : (-1);

}
EXPORT_SYMBOL(isp1763_mem_write);



/*--------------------------------------------------------------*
 *
 * Module dtatils: isp1763_request_irq
 *
 * This function registers the ISR of driver with this driver.
 * Since there is only one interrupt line, when the first driver
 * is registerd, will call the system function request_irq. The PLX
 * bridge needs enabling of interrupt in the interrupt control register to
 * pass the local interrupts to the PCI (cpu).
 * For later registrations will just update the variables. On ISR, this driver
 * will look for registered handlers and calls the corresponding driver's
 * ISR "handler" function with "isr_data" as parameter.
 *
 *  Input: struct
 *              (void (*handler)(struct isp1763_dev *, void *)-->handler.
 *               isp1763_driver *drv  --> Driver structure.
 *  Output result
 *         0= complete
 *         1= error.
 *
 *  Called by: system function module_init
 *
 *
 *--------------------------------------------------------------*/
#ifdef LINUX_2620
int
isp1763_request_irq(void (*handler)     
	(struct isp1763_dev * dev, void *isr_data),
	 struct isp1763_dev *dev, void *isr_data)
#else
int
isp1763_request_irq(void (*handler) (struct isp1763_dev *, void *),
	struct isp1763_dev *dev, void *isr_data)
#endif
{
	int result = 0;
	u32 intcsr = 0;
	u32 ul_busregion_descr = 0;
	hal_entry("%s: Entered\n", __FUNCTION__);
	hal_int("isp1763_request_irq: dev->index %x\n", dev->index);

	printk(KERN_NOTICE "+isp1763_request_irq: dev->index %x\n", dev->index);

	/*Interrupt handler routine */
	if (dev->active) {
		dev->handler = handler;
		dev->isr_data = isr_data;
		printk(KERN_NOTICE "-isp1763_request_irq: dev->index %x\n",
			dev->index);
		hal_int("isp1763_request_irq: dev->handler %s\n", dev->handler);
		hal_int("isp1763_request_irq: dev->isr_data %x\n",
			dev->isr_data);
	} else {
		printk("%s : error - driver %d is not active \n", __FUNCTION__,
			dev->index);
	}
	hal_entry("%s: Exit\n", __FUNCTION__);
	return result;
}				/* End of isp1763_request_irq */

EXPORT_SYMBOL(isp1763_request_irq);



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
	/*free the irq*/
#ifdef LINUX_2620
	free_irq(dev->irq, &isp1763_loc_dev);
#endif
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
	printk(KERN_NOTICE "BusRegionDesc %x \n", ul_busregion_descr);

#ifdef ENABLE_PLX_DMA
	u32 ulDmaModeCh1;

	ulDmaModeCh1 = plx9054_reg_read(0x94);

	/* Set as 16 bit mode */
	ulDmaModeCh1 &= 0xFFFFFFFC;
	ulDmaModeCh1 |= 0x00020401;

	ulDmaModeCh1 |= 0x00000800;	/*Holds local address bus constant*/

	plx9054_reg_write(0x94, ulDmaModeCh1);


#endif /*ENABLE_PLX_DMA*/

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
#ifdef ENABLE_PLX_DMA
	int i;
	g_pDMA_Read_Buf = kmalloc(DMA_BUF_SIZE, GFP_KERNEL | GFP_DMA);
	g_pDMA_Write_Buf = kmalloc(DMA_BUF_SIZE, GFP_KERNEL | GFP_DMA);

	if (g_pDMA_Read_Buf == NULL || g_pDMA_Write_Buf == NULL) {
		printk("Cannot allocate memory for DMA operations!\n");
		return -2;
	}
#endif

/*initialize tasklet*/
	tasklet_init(&irq_tasklet, (void (*)(unsigned long)) hal_handle_int,
		&isp1763_loc_dev);
/*register for interrupts*/
#ifdef LINUX_2620
	status = request_irq(dev->irq, isp1763_pci_isr, SA_SHIRQ, &IsrName,
		&isp1763_loc_dev);
#else /*Linux 2.6.28*/
	/* Interrupt will be registered by HCD directly due to 2.6.28 kernel nature*/

#endif

	spin_lock_init(&hal_lock);
	spin_lock_init(&rw_lock);
	printk(KERN_NOTICE "-isp1763_pci_probe \n");
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

#ifdef ENABLE_PLX_DMA
	if (g_pDMA_Read_Buf != NULL){
		kfree(g_pDMA_Read_Buf);
	}
	if (g_pDMA_Write_Buf != NULL){
		kfree(g_pDMA_Write_Buf);
	}
#endif

	/*Lets handle the host first */
	loc_dev = &isp1763_loc_dev[ISP1763_HC];
	/*free the memory occupied by host */
	release_mem_region(loc_dev->io_base, loc_dev->io_len);
	release_mem_region(pci_io_base, iolength);
	/*unmap the occupied memory resources */
	iounmap(loc_dev->baseaddress);
	/* unmap the occupied io resources */
	iounmap(iobase);
	/*kill the tasklet*/
	tasklet_kill(&irq_tasklet);
	return;
}				/* End of isp1763_pci_remove */


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_init(isp1763_pci_module_init);
module_exit(isp1763_pci_module_cleanup);
