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
#include "mem.h"
#include "regs.h"
#include "cpu.h"

STANDALONE_CODE_RESOURCE_ID(0);

/* Pixel format, lifedrive = 0x10000  */
UInt32 fb_test(ArmGlobals *g);
UInt32 foobar();
UInt32 phys_to_virt(ArmGlobals *g, UInt32 phys);
UInt32 virt_to_phys(ArmGlobals *g, UInt32 virt);
UInt32 map_mem(ArmGlobals *g, UInt32 phys);
UInt32 p_reg(ArmGlobals *g, UInt32 pa);
void map(ArmGlobals *g, UInt32 phys, UInt32 virt);

#define reg(x) p_reg(g, x)

/* Note: This function needs to be kept at the top of the file */
unsigned long arm_entry(const void *emulStateP, char *userData68KP,
			Call68KFuncType * call68KFuncP)
{
	int i;
	
	ArmStack *stack = (ArmStack *) userData68KP;
	ArmGlobals *g;
	UInt32 func;
	UInt32 ret;

	if (!stack)
		return 0xbadbad;

	stack[0] = EndianSwap32(stack[0]);
	if (!stack[0]) {
		ret = 0xfeedf00d;
		goto out;
	}

	g = (ArmGlobals*)pop_uint32(stack);
	func = pop_uint32(stack);

	switch (func) {
	case ARM_read_cp:
		ret = read_cp(pop_uint32(stack), pop_uint32(stack));
		break;

	case ARM_fb_test:
		ret = fb_test(g);
		break;
	case ARM_boot_linux:

		ret = boot_linux(g, (void*)pop_uint32(stack), 
				 pop_uint32(stack), (void*)pop_uint32(stack),
				 pop_uint32(stack), (char*)pop_uint32(stack));
		break;
	case 4:
		ret = foobar();
		break;
	default:
		ret = 0xbadf0000 ^ func;
	}

      out:
	stack[0] = EndianSwap32(stack[0]);
	return ret;
}

void testfunc()
{
	while(1);
}


#if 1

//UInt16 SysFatalAlert(Char *msg);
Err SysPatchEntry(UInt32 refNum, UInt32 entryNum, void *procP, void 
		  **oldProcP);

Err HALDisplayGetAttributes(UInt16 attr, UInt32 *valueP);

UInt32 foobar()
{
	UInt32 val='wtf\0';
	void *old, *fptr;
	// depth = 16
	// videorambase = 0x50000000
	// videoramsize = 0x9600
	// videobase = 0x50000000
	//
	asm volatile ("adr %0, HackFatal" : "=r" (fptr));

	SysPatchEntry(2, 0x444/4, fptr, &old);
	//SysPatchEntry(2, 0x448/4, fptr, &old);
	//SysFatalAlert(&val);
	val = *(UInt32*)0xf0000000;
	SysPatchEntry(2, 0x444/4, old, &old);

	//HALDisplayGetAttributes(0x03, &val);
	return val;
}


UInt16 HackFatal(Char *msg) {
        // This code gets executed in privileged mode



        // This return code makes SysFatalAlert() return to the caller as if nothing happened
		      return 0xffff;
}

asm (
"SysPatchEntry:\n"
"ldr     r12, [r9, #-8]\n"
"ldr     r15, [r12, #0x8b0]\n"
"HALDisplayGetAttributes:\n"
"ldr     ip, [r9, #-4]\n"
"ldr     pc, [ip, #44]\n"
"SysFatalAlert:\n"
"ldr     ip, [r9, #-12]\n"
"ldr     pc, [ip, #1092]\n"
);

#endif

/**
 * Decode the bits per pixel from the value
 * of LCCR3 register.
 */
int decode_bpp(UInt32 lccr3) 
{
	UInt32 x = ((lccr3 >> 24) & 7) | (((lccr3 >> 29) & 1) << 3);
	switch(x) {
	case 1: return 2;
	case 2: return 4;
	case 3: return 8;
	case 4: return 16;
	case 5:
	case 6: return 18;
	case 7: 
	case 8:	return 19;
	case 9: return 24;
	case 10: return 25;
	}
	return 0;
}

/**
 * Test drawing to the frame buffer
 */
UInt32 fb_test(ArmGlobals *g) 
{
	UInt32 lccr1 = reg(LCCR1);
	UInt32 lccr2 = reg(LCCR2);
	UInt32 lccr3 = reg(LCCR3);

	UInt32 w = (lccr1 & 0x3ff) + 1;
	UInt32 h = (lccr2 & 0x3ff) + 1;
	UInt32 bpp = decode_bpp(lccr3);
	UInt32 len = w * h * bpp / 8;

	int alldead;

	/* older devices use 8-bit & palette, what do we do for them? */
	if(bpp != 16) {
		return 0xbadfb000 | bpp;
	}

	void *fb_base = (void*) reg( reg(FDADR0) + DMA_SRC);
	void *fb_end = fb_base + len;


	UInt16 i=0;
	UInt16 *j=0;
	irq_off();
	/*
	for(i=0; i<0xff; i++) {
		alldead = 1;
		for(j = fb_base; (void*)j < fb_end; j++) {
			*j = ((UInt16)(UInt32)j) + i;		
		}
		if(alldead) break;
	}*/
	*((UInt32*)phys_to_virt(g, LCCR3)) = 0x4700002;

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


/**** MMU functions ****/

UInt32 virt_to_phys(ArmGlobals *g, UInt32 virt)
{
        UInt32 phys=0x99999;
        UInt32 ttb = g->vttb;
        UInt32 *fld_p = (UInt32*) (ttb + ((virt >> 20) << 2));
        UInt32 fld = *fld_p;
	UInt32 sld;
	UInt32 *sld_p;

        if ((fld & FLD_MASK) == 0) {    /* invalid */
		phys = 0;
        } else if ((fld & FLD_MASK) == FLD_SECTION) {   /* section */
                phys = (fld & 0xfff00000) | (virt & 0x000fffff);
	} else if ((fld & FLD_MASK) == FLD_COARSE) {
		/* 2nd level, yuck. Here's hoping we can access it. */
		sld_p = (UInt32*) ( ((fld & 0xFFFFFc00) | 
				     ((virt & 0xfF000) >> 10))
				    - g->ram_base);
		sld = *sld_p; /* crash? */
		if((sld & 3) == 2) {           /* small page */
			phys = (sld & 0xFFFFF000) | (virt & 0xFFF);
		} else if((sld & 3) == 1) {    /* large page */
			phys = (sld & 0xFFFF0000) | (virt & 0xFFFF);
		} else {                       /* invalid page */
			phys = (sld & 0xFFFFF000) | (virt & 0xfff);
		}
        } else { 
		/* FIXME: THIS IS WRONG!! */
		/* 2nd level, yuck. Here's hoping we can access it. */
		sld_p = (UInt32*) ( ((fld & 0xFFFFF000) | 
				     ((virt & 0xff000) >> 10))
				    - g->ram_base);
		sld = *sld_p; /* crash? */
		if((sld & 3) == 2) {           /* small page */
			phys = (sld & 0xFFFFF000) | (virt & 0xFFF);
		} else if((sld & 3) == 1) {    /* large page */
			phys = (sld & 0xFFFF0000) | (virt & 0xFFFF);
		} else {                       /* invalid page */
			phys = (sld & 0xFFFFFC00) | (virt & 0x3ff);
		}
        }

        return phys;
}

UInt32 phys_to_virt(ArmGlobals *g, UInt32 phys)
{
        UInt32 ttb = g->vttb;
        UInt32 table_index, *pfld, fld;

        if (!ttb)
                return NULL;

        for (table_index = 0; table_index <= 0xfff; table_index++) {
                pfld = (UInt32 *) (ttb | (table_index << 2));
                fld = EndianFix32(*pfld);

                if ((fld & FLD_MASK) != FLD_SECTION)
                        continue;       /* todo: handle other page sizes */

                if ((fld & 0xfff00000) == (phys & 0xfff00000)) {
                        // found!                                               
                        return (phys & 0x000fffff) | (table_index << 20);
                }
        }

        return NULL;
}

UInt32 map_mem(ArmGlobals *g, UInt32 phys)
{
        UInt32 *tt = (UInt32*) g->vttb;
        UInt32 i, rec;
        UInt32 va;

        if (!tt) return NULL;

        /* search for a free virtual address */
        for (i = 0; i <= 0xfff; i++) {
                rec = EndianFix32(tt[i]);

                if ((rec & FLD_MASK) == FLD_INVALID) {
                        /* found a free virtual address section */
                        rec = (phys & 0xfff00000) | 0xc02;
                        tt[i] = EndianFix32(rec);
                        va = i << 20;
                        return va | (phys & 0x000fffff);
                }

        }

        return NULL;
}

void map(ArmGlobals *g, UInt32 phys, UInt32 virt)
{
	int i;
	UInt32 *tt = (UInt32*) g->vttb;
	UInt32 idx = virt >> 20;

	tt[idx] = (phys & 0xFFF00000) | 0xc1a;


	asm volatile ("mov r0, #0" : : : "r0");
#ifdef FLUSH_CACHE
	/* FIXME: this Fatal Exceptions on my LifeDrive, why?! 
	 * It works fine in Garux. :-/
	 */
	// flush v3/v4 cache          
	asm volatile ("mcr p15, 0, r0, c7, c7, 0" : : : "r0");  
#endif
	// flush v4 TLB
	asm volatile ("mcr p15, 0, r0, c8, c7, 0" : : : "r0");
	CPWAIT

	/* wait a little while for the changes to take effect */
	for(i=0; i<100000; i++);
}

UInt32 p_reg(ArmGlobals *g, UInt32 addr) {
	UInt32 *val=(UInt32*)phys_to_virt(g, addr);
	if(!val) return 0;
	return *val;
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
