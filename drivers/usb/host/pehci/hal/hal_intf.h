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
* This is a hardware abstraction layer header file.
* 
* Author : wired support <wired.support@stericsson.com>
*
*/

#ifndef	__HAL_INTF_H__
#define	__HAL_INTF_H__

//#define	LINUX_2620


/* Host controller conditional defines*/
/*#define MSEC_INT_BASED  */	/* define to run host on SOF interrupt */


#ifdef LINUX_2620
#else
/* For Iso support */
#ifdef MSEC_INT_BASED		
/* Comment out to remove isochronous transfer support */

/* #define CONFIG_ISO_SUPPORT */
#endif

/* Comment out to remove isochronous transfer support */
#define CONFIG_ISO_SUPPORT 

#ifdef CONFIG_ISO_SUPPORT

#define ISO_DBG_ENTRY 0
#define ISO_DBG_EXIT  0
#define ISO_DBG_ADDR 0
#define ISO_DBG_DATA 0
#define ISO_DBG_ERR  0
#define ISO_DBG_INFO 0

/*#define ISO_DEBUG_ENABLE*/

#ifdef ISO_DEBUG_ENABLE
#define iso_dbg(category, format, arg...) \
do \
{ \
	if(category) { \
        	printk(format, ## arg); \
    	} \
} while(0)

#else

#define iso_dbg(category, format, arg...) do { } while(0)

#endif

#endif /* CONFIG_ISO_SUPPORT */
#endif /*LINUX_2620*/

#ifndef	DATABUS_WIDTH_16
#define	DATABUS_WIDTH_16	/*comment out this line to enable 8-bit mode*/
#endif

#ifdef	DATABUS_WIDTH_16
/*DMA SUPPORT */
//#define	ENABLE_PLX_DMA
#endif

#define	DMA_BUF_SIZE	(4096 * 2)

/* Values for id_flags filed of isp1763_driver_t */
#define ISP1763_HC				0	/* Host Controller Driver */
#define ISP1763_DC				1	/* Device Controller Driver */
#define ISP1763_OTG				2	/* Otg Controller Driver */
#define ISP1763_LAST_DEV			(ISP1763_OTG + 1)
#define ISP1763_1ST_DEV				(ISP1763_HC)

#define HC_SPARAMS_REG				0x04	/* Structural Parameters Register */
#define HC_CPARAMS_REG				0x08	/* Capability Parameters Register */

#define HC_USBCMD_REG				0x8C	/* USB Command Register */
#define HC_USBSTS_REG				0x90	/* USB Status Register */
#define HC_INTERRUPT_REG_EHCI			0x94	/* Interrupt Enable Register */
#define HC_FRINDEX_REG				0x98	/* Frame Index Register */

#define HC_CONFIGFLAG_REG			0x9C	/* Conigured Flag  Register */
#define HC_PORTSC1_REG				0xA0	/* Port Status Control for Port1 */


/*ISO Transfer Registers */
#define HC_ISO_PTD_DONEMAP_REG			0xA4	/* ISO PTD Done Map Register */
#define HC_ISO_PTD_SKIPMAP_REG			0xA6	/* ISO PTD Skip Map Register */
#define HC_ISO_PTD_LASTPTD_REG			0xA8	/* ISO PTD Last PTD Register */

/*INT Transfer Registers */
#define HC_INT_PTD_DONEMAP_REG			0xAA	/* INT PTD Done Map Register */
#define HC_INT_PTD_SKIPMAP_REG			0xAC	/* INT PTD Skip Map Register */
#define HC_INT_PTD_LASTPTD_REG			0xAE	/* INT PTD Last PTD Register  */

/*ATL Transfer Registers */
#define HC_ATL_PTD_DONEMAP_REG			0xB0	/* ATL PTD Last PTD Register  */
#define HC_ATL_PTD_SKIPMAP_REG			0xB2	/* ATL PTD Last PTD Register  */
#define HC_ATL_PTD_LASTPTD_REG			0xB4	/* ATL PTD Last PTD Register  */

/*General Purpose Registers */
#define HC_HW_MODE_REG				0xB6	/* H/W Mode Register  */
#define HC_CHIP_ID_REG				0x70	/* Chip ID Register */
#define HC_SCRATCH_REG				0x78	/* Scratch Register */
#define HC_RESET_REG				0xB8	/* HC Reset Register */

/* Interrupt Registers */
#define HC_INTERRUPT_REG			0xD4	/* Interrupt Register */
#define HC_INTENABLE_REG			0xD6	/* Interrupt enable Register */
#define HC_ISO_IRQ_MASK_OR_REG			0xD8	/* ISO Mask OR Register */
#define HC_INT_IRQ_MASK_OR_REG			0xDA	/* INT Mask OR Register */
#define HC_ATL_IRQ_MASK_OR_REG			0xDC	/* ATL Mask OR Register */
#define HC_ISO_IRQ_MASK_AND_REG			0xDE	/* ISO Mask AND Register */
#define HC_INT_IRQ_MASK_AND_REG			0xE0	/* INT Mask AND Register */
#define HC_ATL_IRQ_MASK_AND_REG			0xE2	/* ATL Mask AND Register */
/*power control reg */
#define HC_POWER_DOWN_CONTROL_REG		0xD0


/*RAM Registers */
#define HC_DMACONFIG_REG			0xBC	/* DMA Config Register */
#define HC_MEM_READ_REG				0xC4	/* Memory Register */
#define HC_DATA_REG				0xC6	/* Data Register */

#define OTG_CTRL_SET_REG			0xE4
#define OTG_CTRL_CLEAR_REG			0xE6

/*interrupt count and buffer status register*/

#define HC_BUFFER_STATUS_REG			0xBA
#define HC_INT_THRESHOLD_REG			0xC8
#define HC_OTG_INTERRUPT			0x400

#define HC_ATL_INTERRUPT			0x100
#define HC_INTL_INTERRUPT			0x080
#define HC_ISO_INTERRUPT			0x200
#define DC_CHIPID				0x70

#define FPGA_CONFIG_REG				0x100


struct isp1763_driver;
typedef struct _isp1763_id {
	u16 idVendor;
	u16 idProduct;
	unsigned long driver_info;
} isp1763_id;

typedef struct isp1763_dev {
	/*added for pci device */
	struct pci_dev *pcidev;
	struct device *dev;		/* corresponding device structure */
	struct isp1763_driver *driver;	/* which driver has allocated this device */
	void *driver_data;	/* data private to the host controller driver */
	void *otg_driver_data;	/*data private for otg controler */
	unsigned char index;	/* local controller (HC/DC/OTG) */
	unsigned int irq;	/*Interrupt Channel allocated for this device */
#ifdef LINUX_2620
	void (*handler) (struct isp1763_dev * dev, void *isr_data, struct pt_regs *);	/* Interrupt Serrvice Routine */
#else
	void (*handler) (struct isp1763_dev * dev, void *isr_data);	/* Interrupt Serrvice Routine */
#endif
	void *isr_data;		/* isr data of the driver */
	unsigned long int_reg;	/* Interrupt register */
	unsigned long alt_int_reg;	/* Interrupt register 2 */
	unsigned short atl_donemap_reg;	/* atl done map register */
	unsigned short intl_donemap_reg;	/* intl done map register */
	unsigned short iso_donemap_reg;	/* iso done map register */
	unsigned long start;
	unsigned long length;
	struct resource *mem_res;
	unsigned long io_base;	/* Start Io address space for this device */
	unsigned long io_len;	/* IO address space length for this device */

	unsigned long chip_id;	/* Chip Id */

	char name[80];		/* device name */
	int active;		/* device status */

	unsigned long dma;
	u8 *baseaddress;	/*base address for i/o ops */
	u8 *dmabase;
} isp1763_dev_t;



typedef struct isp1763_driver {
	char *name;
	unsigned long index;	/* HC or DC or OTG */
	isp1763_id *id;		/*device ids */
	int (*probe) (struct isp1763_dev * dev, isp1763_id * id);	/* New device inserted */
	void (*remove) (struct isp1763_dev * dev);	/* Device removed (NULL if not a hot-plug capable driver) */

	void (*suspend) (struct isp1763_dev * dev);	/* Device suspended */
	void (*resume) (struct isp1763_dev * dev);	/* Device woken up */

} isp_1763_driver_t;

int isp1763_register_driver(struct isp1763_driver *drv);
void isp1763_unregister_driver(struct isp1763_driver *drv);
#ifdef LINUX_2620

int isp1763_request_irq(void(*handler)
	(struct isp1763_dev * dev, void *isr_data),
	struct isp1763_dev *dev, void *isr_data);

#else
int isp1763_request_irq(void (*handler)	       
	(struct isp1763_dev * dev, void *isr_data),
	struct isp1763_dev *dev, void *isr_data);
#endif

u32 isp1763_reg_read32(isp1763_dev_t * dev, u16 reg, u32 data);
u16 isp1763_reg_read16(isp1763_dev_t * dev, u16 reg, u16 data);
void isp1763_reg_write32(isp1763_dev_t * dev, u16 reg, u32 data);
void isp1763_reg_write16(isp1763_dev_t * dev, u16 reg, u16 data);
int isp1763_mem_read(isp1763_dev_t * dev, u32 start_add,
		     u32 end_add, u32 * buffer, u32 length, u16 dir);
int isp1763_mem_write(isp1763_dev_t * dev, u32 start_add,
		      u32 end_add, u32 * buffer, u32 length, u16 dir);
#endif /* __HAL_INTF_H__ */
