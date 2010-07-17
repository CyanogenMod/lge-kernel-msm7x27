/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/crc-ccitt.h>
#include "diagchar_hdlc.h"

/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */
// enable to send more than maximum packet size limitation
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_MTC)
#include "diagchar.h" // for struct diagchar_dev
#include <mach/usbdiag.h> // for struct diag_request
#include "diagchar.h" // for POOL_TYPE_USB_STRUCT
#include "diagmem.h" // for diagmem_alloc()
#include <linux/delay.h>
#endif /*LG_FW_MTC*/
/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */

MODULE_LICENSE("GPL v2");

#define CRC_16_L_SEED           0xFFFF

#define CRC_16_L_STEP(xx_crc, xx_c) \
	crc_ccitt_byte(xx_crc, xx_c)

/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */
// enable to send more than maximum packet size limitation
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_MTC)
extern struct diag_hdlc_dest_type enc;
extern struct diagchar_dev *driver;

int lg_diag_write_overflow(void *pStart, void *pDone, void *pEnd);

static unsigned long overflow_index = 0;

unsigned int pDest_ptr = 0;
unsigned int pDest_last_ptr = 0;

void lg_diag_set_enc_param(void *pDest, void *pDest_last)
{
  pDest_ptr = (unsigned int)pDest;
  pDest_last_ptr = (unsigned int)pDest_last;
#ifdef LG_DIAG_DEBUG
  printk(KERN_INFO "LG_FW : lg_diag_set_enc_param, dest (0x%x), dest_last (0x%x)\n",\
    pDest, pDest_last);
  printk(KERN_INFO "LG_FW : lg_diag_set_enc_param, dest (0x%x), dest_last (0x%x)\n",\
    pDest_ptr, pDest_last_ptr);
#endif
}
#endif /*LG_FW_MTC*/
/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */
void diag_hdlc_encode(struct diag_send_desc_type *src_desc,
		      struct diag_hdlc_dest_type *enc)
{
	uint8_t *dest;
	uint8_t *dest_last;
	const uint8_t *src;
	const uint8_t *src_last;
	uint16_t crc;
	unsigned char src_byte = 0;
	enum diag_send_state_enum_type state;
	unsigned int used = 0;

	if (src_desc && enc) {

		/* Copy parts to local variables. */
		src = src_desc->pkt;
		src_last = src_desc->last;
		state = src_desc->state;
		dest = enc->dest;
		dest_last = enc->dest_last;

		if (state == DIAG_STATE_START) {
			crc = CRC_16_L_SEED;
			state++;
		} else {
			/* Get a local copy of the CRC */
			crc = enc->crc;
		}

		/* dest or dest_last may be NULL to trigger a
		   state transition only */
		if (dest && dest_last) {
			/* This condition needs to include the possibility
			   of 2 dest bytes for an escaped byte */
			while (src <= src_last && dest <= dest_last) {

				src_byte = *src++;

				if ((src_byte == CONTROL_CHAR) ||
				    (src_byte == ESC_CHAR)) {

					/* If the escape character is not the
					   last byte */
					if (dest != dest_last) {
						crc = CRC_16_L_STEP(crc,
								    src_byte);

						*dest++ = ESC_CHAR;
						used++;

						*dest++ = src_byte
							  ^ ESC_MASK;
						used++;
					} else {

						src--;
						break;
					}

				} else {
					crc = CRC_16_L_STEP(crc, src_byte);
					*dest++ = src_byte;
					used++;
				}
			}

			if (src > src_last) {

				if (state == DIAG_STATE_BUSY) {
					if (src_desc->terminate) {
						crc = ~crc;
						state++;
					} else {
						/* Done with fragment */
						state = DIAG_STATE_COMPLETE;
					}
				}

				while (dest <= dest_last &&
				       state >= DIAG_STATE_CRC1 &&
				       state < DIAG_STATE_TERM) {
					/* Encode a byte of the CRC next */
					src_byte = crc & 0xFF;

					if ((src_byte == CONTROL_CHAR)
					    || (src_byte == ESC_CHAR)) {

						if (dest != dest_last) {

							*dest++ = ESC_CHAR;
							used++;
							*dest++ = src_byte ^
								  ESC_MASK;
							used++;

							crc >>= 8;
						} else {

							break;
						}
					} else {

						crc >>= 8;
						*dest++ = src_byte;
						used++;
					}

					state++;
				}

				if (state == DIAG_STATE_TERM) {
					if (dest_last >= dest) {
						*dest++ = CONTROL_CHAR;
						used++;
						state++;	/* Complete */
					}
				}
			}
		}
		/* Copy local variables back into the encode structure. */

		enc->dest = dest;
		enc->dest_last = dest_last;
		enc->crc = crc;
		src_desc->pkt = src;
		src_desc->last = src_last;
		src_desc->state = state;
	}

	return;
}

/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */
// enable to send more than maximum packet size limitation
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_MTC)
void diag_hdlc_encode_mtc(struct diag_send_desc_type *src_desc,
				struct diag_hdlc_dest_type *enc_dest)

{
	uint8_t *dest;
	uint8_t *dest_last;
	const uint8_t *src;
	const uint8_t *src_last;
	uint16_t crc;
	unsigned char src_byte = 0;
	enum diag_send_state_enum_type state;
	unsigned int used = 0;
	int err;
	uint8_t *enc_dest_start;

	if (src_desc && enc_dest) {

		/* Copy parts to local variables. */
		src = src_desc->pkt;
		src_last = src_desc->last;
		state = src_desc->state;
		dest = enc_dest->dest;
		dest_last = enc_dest->dest_last;
		enc_dest_start = (uint8_t *)enc_dest->dest;
			
		if (state == DIAG_STATE_START) {
			crc = CRC_16_L_SEED;
			state++;
		} else {
			/* Get a local copy of the CRC */
			crc = enc_dest->crc;
		}

		/* dest or dest_last may be NULL to trigger a
		   state transition only */
		if (dest && dest_last) {
			/* This condition needs to include the possibility
			   of 2 dest bytes for an escaped byte */
			while (src <= src_last) {

				src_byte = *src++;

				if ((src_byte == CONTROL_CHAR) ||
				    (src_byte == ESC_CHAR)) {

					/* If there are at least 2 bytes for the escape character*/
					if (dest <= dest_last-1) {
						crc = CRC_16_L_STEP(crc,
								    src_byte);

						*dest++ = ESC_CHAR;
						used++;

						*dest++ = src_byte
							  ^ ESC_MASK;
						used++;
					} else {

						src--;
						break;
					}

				} else {
					crc = CRC_16_L_STEP(crc, src_byte);
					*dest++ = src_byte;
					used++;
				}

				/* This is to check if after HDLC encoding, we are still within the
				    limits of aggregation buffer. If not, we write out the current buffer
				    and start aggregation in a newly allocated buffer */
				if(dest >= dest_last-1) // last two characters should be reserved for ESC_CHAR and MASK
				{
					overflow_index++;
					
 					printk(KERN_INFO "LG_FW : HDLC encoding overflow, src : 0x%x, src_last : 0x%x, left : %d\n", \
  							(uint32_t)src, (uint32_t)src_last, (uint32_t)(src_last - src));
 					printk(KERN_INFO "LG_FW : HDLC encoding overflow, count : %ld\n", overflow_index);
  					printk(KERN_INFO "LG_FW : HDLC encoding overflow, dest (0x%x), dest_last (0x%x), size (0x%x)\n",\
  						(uint32_t)dest, (uint32_t)dest_last, (uint32_t)dest - (uint32_t)enc_dest_start);

					err = lg_diag_write_overflow((void*)enc_dest_start, (void*)dest, (void *)dest_last);
					if (err == -1) {
						printk(KERN_ERR "\nLG_FW : HDLC encoding overflow, diag_write error (%d)\n", err);	
					}
					enc_dest->dest = enc_dest_start;
					dest = (uint8_t *)enc_dest->dest;
					//enc_dest->dest_last = pDest_last_ptr;
					//dest_last = (uint8_t *)enc_dest->dest_last;
				}
				
			}

			if (src > src_last) {

				if (state == DIAG_STATE_BUSY) {
					if (src_desc->terminate) {
						crc = ~crc;
						state++;
					} else {
						/* Done with fragment */
						state = DIAG_STATE_COMPLETE;
					}
				}

				while (dest <= dest_last &&
				       state >= DIAG_STATE_CRC1 &&
				       state < DIAG_STATE_TERM) {
					/* Encode a byte of the CRC next */
					src_byte = crc & 0xFF;

					if ((src_byte == CONTROL_CHAR)
					    || (src_byte == ESC_CHAR)) {

						if (dest != dest_last) {

							*dest++ = ESC_CHAR;
							used++;
							*dest++ = src_byte ^
								  ESC_MASK;
							used++;

							crc >>= 8;
						} else {

							break;
						}
					} else {

						crc >>= 8;
						*dest++ = src_byte;
						used++;
					}

					state++;
				}

				if (state == DIAG_STATE_TERM) {
					if (dest_last >= dest) {
						*dest++ = CONTROL_CHAR;
						used++;
						state++;	/* Complete */
					}
				}
			}
		}
		/* Copy local variables back into the encode structure. */

		enc_dest->dest = dest;
		enc_dest->dest_last = dest_last;
		enc_dest->crc = crc;
		src_desc->pkt = src;
		src_desc->last = src_last;
		src_desc->state = state;
	}
	overflow_index = 0;

	return;
}

/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */
// enable to send more than maximum packet size limitation
int lg_diag_write_overflow(void *pStart, void *pDone, void *pEnd)
{
  int err = -1;
  
  printk(KERN_INFO "LG_FW : lg_diag_write_overflow, start (0x%x),  done (0x%x), end (0x%x)\n",\
  	(uint32_t)pStart, (uint32_t)pDone, (uint32_t)pEnd);
  printk(KERN_INFO "LG_FW : lg_diag_write_overflow, used (0x%x),  left (0x%x)\n",\
  	(uint32_t)pDone-(uint32_t)pStart, (uint32_t)pEnd-(uint32_t)pDone+1);

  driver->used = (uint32_t)pDone - (uint32_t)pStart; // last two byte are not encoded
  driver->usb_write_ptr_svc = (struct diag_request *)
  	(diagmem_alloc(driver, sizeof(struct diag_request),
  	POOL_TYPE_USB_STRUCT));
    
  driver->usb_write_ptr_svc->buf = pStart;
  driver->usb_write_ptr_svc->length = driver->used;

  err = diag_write(driver->usb_write_ptr_svc);
  if (err) {
      printk(KERN_ERR "LG_FW : lg_diag_write_overflow, diag_write error (%d)\n", err);
      /*Free the buffer right away if write failed */
      //diagmem_free(driver, buf_hdlc, POOL_TYPE_HDLC);
      //ret = -EIO;
      //goto fail_free_hdlc;
      return -1;
  }
    
  msleep(1);
    
  //buf_hdlc= NULL;
  
  driver->used = 0;

  return 0;  
}
#endif /*LG_FW_MTC*/
/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-22, LG_FW_MTC */

int diag_hdlc_decode(struct diag_hdlc_decode_type *hdlc)
{
	uint8_t *src_ptr = NULL, *dest_ptr = NULL;
	unsigned int src_length = 0, dest_length = 0;

	unsigned int len = 0;
	unsigned int i;
	uint8_t src_byte;

	int pkt_bnd = 0;

	if (hdlc && hdlc->src_ptr && hdlc->dest_ptr &&
	    (hdlc->src_size - hdlc->src_idx > 0) &&
	    (hdlc->dest_size - hdlc->dest_idx > 0)) {

		src_ptr = hdlc->src_ptr;
		src_ptr = &src_ptr[hdlc->src_idx];
		src_length = hdlc->src_size - hdlc->src_idx;

		dest_ptr = hdlc->dest_ptr;
		dest_ptr = &dest_ptr[hdlc->dest_idx];
		dest_length = hdlc->dest_size - hdlc->dest_idx;

		for (i = 0; i < src_length; i++) {

			src_byte = src_ptr[i];

			if (hdlc->escaping) {
				dest_ptr[len++] = src_byte ^ ESC_MASK;
				hdlc->escaping = 0;
			} else if (src_byte == ESC_CHAR) {
				if (i == (src_length - 1)) {
					hdlc->escaping = 1;
					i++;
					break;
				} else {
					dest_ptr[len++] = src_ptr[++i]
							  ^ ESC_MASK;
				}
			} else if (src_byte == CONTROL_CHAR) {
				dest_ptr[len++] = src_byte;
				pkt_bnd = 1;
				i++;
				break;
			} else {
				dest_ptr[len++] = src_byte;
			}

			if (len >= dest_length) {
				i++;
				break;
			}
		}

		hdlc->src_idx += i;
		hdlc->dest_idx += len;
	}

	return pkt_bnd;
}
