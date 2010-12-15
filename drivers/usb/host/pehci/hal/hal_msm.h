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

#ifndef _HAL_X86_H_
#define _HAL_X86_H_

#define DRIVER_AUTHOR	"ST-ERICSSON      "
#define DRIVER_DESC	"ISP1763 bus driver"

/* Driver tuning, per ST-ERICSSON requirements: */

#define		MEM_TO_CHECK	4096	/*bytes, must be multiple of 2 */

/* BIT defines */
#define BIT0		(1 << 0)
#define BIT1		(1 << 1)
#define BIT2		(1 << 2)
#define BIT3		(1 << 3)
#define BIT4		(1 << 4)
#define BIT5		(1 << 5)
#define BIT6		(1 << 6)
#define BIT7		(1 << 7)
#define BIT8		(1 << 8)
#define BIT9		(1 << 9)
#define BIT10		(1 << 10)
#define BIT11		(1 << 11)
#define BIT12		(1 << 12)
#define BIT13		(1 << 13)
#define BIT14		(1 << 14)
#define BIT15		(1 << 15)
#define BIT16		(1 << 16)
#define BIT17		(1 << 17)
#define BIT18		(1 << 18)
#define BIT19		(1 << 19)
#define BIT20		(1 << 20)
#define BIT21		(1 << 21)
#define BIT22		(1 << 22)
#define BIT23		(1 << 23)
#define BIT24		(1 << 24)
#define BIT25		(1 << 26)
#define BIT27		(1 << 27)
#define BIT28		(1 << 28)
#define BIT29		(1 << 29)
#define BIT30		(1 << 30)
#define BIT31		(1 << 31)

/* Definitions Related to Little Endian & Big Endian */
#define tole(x)		__constant_cpu_to_le32(x)
#define tobe(x)		__constant_cpu_to_be32(x)

/* Definitions Related to Chip Address and CPU Physical Address 
 * cpu_phy_add: CPU Physical Address , it uses 32 bit data per address
 * chip_add   : Chip Address, it uses double word(64) bit data per address
 */
#define chip_add(cpu_phy_add)	(((cpu_phy_add) - 0x400) / 8)
#define cpu_phy_add(chip_add)	((8 * (chip_add)) + 0x400)

/* for getting end add, and start add, provided we have one address with us */
/* IMPORTANT length  hex(base16) and dec(base10) works fine*/
#define end_add(start_add,length)	(start_add + (length - 4))
#define start_add(end_add,length)	(end_add - (length - 4))

/* Device Registers*/
#define		DEV_UNLOCK_REGISTER		0x7C
#define		DEV_INTERRUPT_REGISTER		0x18
#define		INT_ENABLE_REGISTER		0x14


/* The QHA Data Structure */
struct phci_qh {
	u32 hw_info1;		/* see QHA  W0, the first 32bits */
	u32 hw_info2;		/* see QHA  W1 */
	u32 hw_info3;		/* see QHA  W2 */
	u32 hw_info4;		/* see QHA  W3 */
	u32 Reserved1;		/* Reserved1..4. NOT in USE */
	u32 Reserved2;
	u32 Reserved3;
	u32 Reserved4;
} phci_qh __attribute__ ((aligned(32)));


/*
 *  We have hardware support for asynchronous DMA transfers.  We can use PIO
 *  mode as well.  The chips registers are mapped as follows:
 *
 *  Base address for pio mode
 *  Physical addresses for DMA transfers: 
 *  Note also that these are normally defined in the platform specific header
 *  files.  DON'T define them within your driver, that's extremely non-portable.
 */
typedef struct isp1763_hal {
	void *mem_base;		/* Memory base */
	u8 irq_usage;		/* NUmber of drivers using INT channel */
} isp1763_hal_t;

typedef struct {
	char *name;		/* dma stream identifier */
	char *data_buffer;	/* pointer to in memory dma buffer (virtual) */
	char *phys_data_buffer;	/* pointer to in memory dma buffer (physical) */
	char *trash_buffer;	/* don't ask... */
	char *phys_trash_buffer;
	unsigned int dma_ch;		/* DMA channel number */
	volatile u32 *drcmr;	/* the dma request channel to use, 
				not to be confused with the above */
	unsigned long dcmd;		/* DMA dcmd field */
	unsigned long dev_addr;	/* device physical address for this channel */
	unsigned long blklen;		/* block length of specific transfer */
	char output;		/* 0 for input, 1 for ouput */
} isp1763_channel_t;

typedef struct {
	isp1763_channel_t *dreq0;
	isp1763_channel_t *dreq1;
	struct semaphore sem;	/* may be handy for races... */
} dma_state_t;

#endif /*_HAL_X86_H_ */
