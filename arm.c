/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include <PceNativeCall.h>
#include <Standalone.h>
#include "arm.h"

STANDALONE_CODE_RESOURCE_ID(0);

/* Disable IRQ and FIQ */
#define irq_off() asm volatile ("mrs r0, cpsr \n" \
                    "orr r0, r0, #0xc0 \n" \
		    "msr cpsr, r0" : : : "r0" )

/* Enable IRQ and FIQ */
#define irq_on() asm volatile ("mrs r0, cpsr \n" \
                    "and r0, r0, #0xffffff3f \n" \
		    "msr cpsr, r0" : : : "r0" )

/* Flush instruction and data caches */
#define flush_caches() asm volatile ("mov r0, #0 \n" \
			  "mcr p15, 0, r0, c7, c7, 0 \n" \
                          "mcr p15, 0, r0, c8, c7, 0" : : : "r0")




/* Note: This function needs to be kept at the top of the file */
unsigned long arm_entry(const void *emulStateP, char *userData68KP,
			Call68KFuncType * call68KFuncP)
{
	ArmStack *stack = (ArmStack *) userData68KP;
	UInt32 func;
	UInt32 ret;

	if (!stack)
		return 0xbadbad;

	stack[0] = EndianSwap32(stack[0]);
	if (!stack[0]) {
		ret = 0xfeedf00d;
		goto out;
	}

	func = pop_uint32(stack);

	switch (func) {
	case ARM_read_cp:
		ret = read_cp(pop_uint32(stack), pop_uint32(stack));
		break;

	case 2:
		hackattack();
		break;
	default:
		ret = 0xbadf0000 ^ func;
	}

      out:
	stack[0] = EndianSwap32(stack[0]);
	return ret;
}

int hackattack() {
	int w = 320;
	int h = 480;
	int bpp = 16;
	int step = 2;
	unsigned int r,g,b;
	int alldead;

	return; // hardcoded, use only on lidedrive

	UInt32 len = w*h*bpp/8 * 2;

	UInt16 i=0;
	UInt16 *j=0;
	irq_off();

	for(i=0; i<0xff; i++) {
		alldead = 1;
		for(j=(void*)0x20000; (UInt32)j < 0x20000 + len; j++) {
			b = (*j) & 0x1f;
			g = (*j>>5) & 0x3f;
			r = (*j>>11) & 0x1f;
			
			if(r||g||b) alldead =0;

			if(r) r-=step;
			if(g) g-=step;
			if(b) b-=step;
			*j = (r<<11) + (g<<5) + b;		
		}
		if(alldead) break;
	}

	irq_on();
	return 0;
}

/* Read a coprocessor register */
unsigned long read_cp(unsigned long cp, unsigned long reg)
{
	unsigned long value;
	value = 0xbabababa;
	switch (cp) {
	case 15:
		switch (reg) {
		case 0:
		      asm("mrc p15, 0, r0, c0, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 1:
		      asm("mrc p15, 0, r0, c1, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 2:
		      asm("mrc p15, 0, r0, c2, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 3:
		      asm("mrc p15, 0, r0, c3, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 4:
		      asm("mrc p15, 0, r0, c4, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 5:
		      asm("mrc p15, 0, r0, c5, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 6:
		      asm("mrc p15, 0, r0, c6, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		case 7:
		      asm("mrc p15, 0, r0, c7, c0, 0\nmov %0, r0": "=r"(value): :"r0");
			break;
		}
		break;
	}
	return value;
}

/**** Stack functions ****/

void push_uint32(ArmStack * stack, UInt32 n)
{
	stack[++stack[0]] = EndianSwap32(n);
}

UInt32 pop_uint32(ArmStack * stack)
{
	UInt32 ret = EndianSwap32(stack[stack[0]]);
	stack[0]--;
	return ret;
}
