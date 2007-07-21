/*
 * Some parts based on GNU Haret which is:
 * (C) Copyright 2006 Kevin O'Connor <kevin@koconnor.net>
 *  
 *  This file may be distributed under the terms of the GNU GPL license.
 *   
 */
#include <PalmOS.h>
#include "arm.h"
#include "mem.h"
#include "regs.h"
#include "cpu.h"

/* location of the pointer to the IRQ handler */
#define IRQH (*(void**)0x38)
#define DATAH (*(void**)0x30)

void irqhandler();
void datahandler();

// Set the IBCR0 software debug register
static inline void  set_IBCR0(UInt32 val) {
	asm volatile("mcr p15, 0, %0, c14, c8, 0" : : "r"(val));
}
// Set the IBCR1 software debug register
static inline void  set_IBCR1(UInt32 val) {
	asm volatile("mcr p15, 0, %0, c14, c9, 0" : : "r"(val));
}
// Set the EVTSEL performance monitoring register
static inline void  set_EVTSEL(UInt32 val) {
	asm volatile("mcr p14, 0, %0, c8, c1, 0" : : "r"(val));
}
// Set the INTEN performance monitoring register
static inline void  set_INTEN(UInt32 val) {
	asm volatile("mcr p14, 0, %0, c4, c1, 0" : : "r"(val));
}
// Set the PMNC performance monitoring register
static inline void  set_PMNC(UInt32 val) {
	asm volatile("mcr p14, 0, %0, c0, c1, 0" : : "r"(val));
}
// Set the DBCON software debug register
static inline void  set_DBCON(UInt32 val) {
	asm volatile("mcr p15, 0, %0, c14, c4, 0" : : "r"(val));
}
// Set the DBR0 software debug register
static inline void  set_DBR0(UInt32 val) {
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(val));
}
// Set the DBR1 software debug register
static inline void  set_DBR1(UInt32 val) {
	asm volatile("mcr p15, 0, %0, c14, c3, 0" : : "r"(val));
}
// Set the DCSR software debug register
static inline void  set_DCSR(UInt32 val) {
	asm volatile("mcr p14, 0, %0, c10, c0, 0" : : "r"(val));
}


UInt32 install_irqhandler(ArmGlobals *g, UInt32 *buf)
{
	UInt32 *src = 0;
	UInt32 *end = 0;
	UInt32 *dest = buf;
	UInt32 *dataoffset;

	/* new value for the domain access control, this disables memory 
	 * protection allowing us to write directly into the storage heap.
	 */
	UInt32 domain_ac=0xFFFFFFFF;
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(domain_ac));

	asm volatile ("adr %0, IrqHandlerStart" :"=r"(src) );
	asm volatile ("adr %0, HandlersEnd" :"=r"(end) );
	
	asm volatile ("adr %0, DataAbortHandler" :"=r"(dataoffset) );

	dataoffset = (UInt32*)((UInt32)dataoffset - (UInt32)src + (UInt32)buf);

	/* copy our new IRQ handler into the buffer */
	while (src < end)
		*dest++ = *src++;

	/* replace PalmOS handler with our own */
	IRQH = buf;
	DATAH = dataoffset;
	
	set_DBCON(0); /* disable debugging */
	set_DBR0(0x90100000);
	set_DBR1(0xffffffff);
	set_DCSR(1<<31); /* global enable */
	set_DBCON(2); /* enable DBR0, both load and store */

//	return *(volatile UInt32*)0x90100000;
	return (UInt32)dataoffset;
}

void irqhandler() {
	asm volatile ("IrqHandlerStart:");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	
	/* Backup register values */
#if 0	
	asm volatile ("str r0, REG0");
	asm volatile ("str r1, REG1");

	/* Let PalmOS' irq handler deal with the event. */
	asm volatile ("ChainHandler:");
	asm volatile ("ldr r1, REG1");		/* restore regs from backup */
	asm volatile ("ldr r0, REG0");
#endif
	
	asm volatile ("ldr pc, [pc, #-4]"); 	/* jump to POS' irq handler */
	asm volatile ("OLDHANDLER: .word 0x200a2b94");
	
	asm volatile ("REG0: .word 0xdeadbeef");
	asm volatile ("REG1: .word 0xdeadbeef");


	asm volatile ("INTH:");

}

void datahandler()
{
	asm volatile ("DataAbortHandler:");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	
#if 0
	asm volatile ("stmdb	sp!, {r14}"); 		/* save r14 */
	asm volatile ("mrc	p15, 0, r14, c10, c0, 0");/* DCSR */
	asm volatile ("and	r14, r14, #28");	/* get method of entry */
	asm volatile ("cmp	r14, #8");		/* data breakpoint? */
	asm volatile ("bne	ChainDataAbort");	/* no? lets get of out of here! */
	

	/* I guess we should start by restoring r14 */
	asm volatile ("ldmia   sp!, {r14}");
#endif
	
	/* okay so we've got a data breakpoint, now what do we do? */
	/* Now let's save all our registers */
	asm volatile ("stmdb   sp!, {r0-r12}");

	/* Store the values of r0 and r1 so we can get them
	 * without messing with the stack.
	 */
	asm volatile ("str r0, [pc, #(Reg0 - . - 8)]");
	asm volatile ("str r1, [pc, #(Reg1 - . - 8)]");
//	asm volatile ("mov r1, #0");
	
	/* grab the next free spot in the buffer */
//	asm volatile ("ldr r1, [pc, #(DumpOffset - . - 8)]");
#if 0
	asm volatile ("pcgrab: add r1, r1, pc");
	asm volatile ("add r1, r1, #(HandlersEnd - pcgrab + 8)");
	
	/* store "1" to indicate this is a data breakpoint */
	asm volatile ("mov r0, #1"); 
	asm volatile ("str r0, [r1]");
	asm volatile ("add r1, r1, #4");

	/* lets save the address of the instruction we broken, as well
	 * as the instruction itself.
	 */
	asm volatile ("sub r0, r14, #8");
	asm volatile ("str r0, [r1]");
	asm volatile ("ldr r0, [r0]");
	asm volatile ("str r0, [r1, #4]");
	asm volatile ("add r1, r1, #8");
	
	/* now lets start saving registers, first the tricky ones */
	asm volatile ("ldr r0, [pc, #(Reg0 - . - 8)]");
	asm volatile ("str r0, [r1]");
	asm volatile ("ldr r0, [pc, #(Reg1 - . - 8)]");
	asm volatile ("str r0, [r1, #4]");
	asm volatile ("add r1, r1, #8");
	
	/* now the rest */
	asm volatile ("str r2, [r1]");
	asm volatile ("str r3, [r1, #4]");
	asm volatile ("str r4, [r1, #8]");
	asm volatile ("str r5, [r1, #0xc]");
	asm volatile ("str r6, [r1, #0x10]");
	asm volatile ("str r7, [r1, #0x14]");
	asm volatile ("str r8, [r1, #0x18]");
	asm volatile ("str r9, [r1, #0x1c]");
	asm volatile ("str r10, [r1, #0x20]");
	asm volatile ("str r11, [r1, #0x24]");
	asm volatile ("str r12, [r1, #0x28]");
	asm volatile ("add r1, r1, #0x2c");
	
	/* save our buffer pointer */
	asm volatile ("str r1, [pc, #(DumpOffset - . - 8)]");
#endif	
	/* Restore everything we clobbered */
	asm volatile ("ldmia   sp!, {r0-r12}");
	
	/* return to aborted process */
	asm volatile ("subs pc,r14,#4");

#if 0
	
	asm volatile ("ChainDataAbort:");
	asm volatile ("ldmia   sp!, {r14}");	/* restore r14 */
	asm volatile ("ldr pc, [pc, #-4]"); 	/* jump to POS' handler */
	asm volatile ("OldDataAbort: .word 0x200a3370");
#endif

	asm volatile ("Reg0: .word 0");
	asm volatile ("Reg1: .word 0");
	asm volatile ("DumpOffset: .word 0");
	asm volatile ("HandlersEnd: .word 0");
}
