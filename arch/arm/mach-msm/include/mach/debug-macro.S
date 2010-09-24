/* arch/arm/mach-msm7200/include/mach/debug-macro.S
 *
 * Copyright (C) 2007 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */



#include <mach/hardware.h>
#include <mach/msm_iomap.h>

#ifdef CONFIG_MSM_DEBUG_UART
	.macro	addruart,rx,tmp
	@ see if the MMU is enabled and select appropriate base address
	mrc	p15, 0, \rx, c1, c0
	tst	\rx, #1
	ldreq	\rx, =MSM_DEBUG_UART_PHYS
	ldrne	\rx, =MSM_DEBUG_UART_BASE
	.endm

	.macro	senduart,rd,rx
#ifdef CONFIG_SERIAL_MSM_HSL
	mov 	r12, #0x1
	str	r12, [\rx, #0x40]
	str	\rd, [\rx, #0x70]
#else
	teq     \rx, #0
	strne   \rd, [\rx, #0x0C]
#endif
	.endm

	.macro	waituart,rd,rx
	@ wait for TX_READY
#ifdef CONFIG_SERIAL_MSM_HSL
	ldr	\rd, [\rx, #0x08]
	tst	\rd, #0x08
	bne	1002f
1001:	ldreq	\rd, [\rx, #0x14]
	tst	\rd, #0x80
	beq 	1001b
#else
	1001:   ldr \rd, [\rx, #0x08]
	tst     \rd, #0x04
	beq     1001b
#endif
1002:
	.endm

#else
	.macro	addruart,rx,tmp
	.endm

	.macro	senduart,rd,rx
	.endm

	.macro	waituart,rd,rx
	.endm
#endif

	.macro	busyuart,rd,rx
	.endm
