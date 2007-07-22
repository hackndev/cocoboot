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
	//IRQH = buf;
	DATAH = dataoffset;

	*((UInt32**)0x64) = dest;
	*((UInt32**)0x68) = dest;
	
	set_DBCON(0); /* disable debugging */
	set_DBR0(0x90100000); /* this is the address we want to monitor */
	set_DBR1(0x00000000); /* this is the mask (0 = compare bit, 1 = ignore bit) */
	set_DBCON(2 | 1<<8); /* enable DBR0, both load and store */
	set_DCSR(1<<31); /* global enable */

//	return *(volatile UInt32*)0x90100000;
	return dest; //(UInt32)dataoffset;
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
	//asm volatile ("stmdb	sp!, {r14}"); 		/* save r14 */
	//asm volatile ("mov	r14, #0");

	//asm volatile ("mrc	p15, 0, r14, c10, c0, 0");/* DCSR */
	//asm volatile ("and	r14, r14, #28");	/* get method of entry */
	//asm volatile ("cmp	r14, #8");		/* data breakpoint? */
	//asm volatile ("bne	runaway");		/* data breakpoint? */
	//asm volatile ("goof: b goof");		/* data breakpoint? */
	//asm volatile ("runaway:ldmia   sp!, {r14}");
	//asm volatile ("mov r0, #0x30000001");
	//asm volatile ("mov r1, #0x30000002");
	//asm volatile ("mov r2, #0x30000003");
	//asm volatile ("mov r3, #0x30000004");
	//asm volatile ("mov r4, #0x30000005");
	//asm volatile ("mov r5, #0x30000006");
	//asm volatile ("subs pc,r14,#4"); /* return to sender */
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

	asm volatile ("stmdb	sp!, {r14}"); 		/* save r14 */
	  asm volatile ("stmdb	sp!, {r12}"); 		/* save r12 */
	
	    asm volatile ("mov	r12, r14"); 		/* copy return addr */

	    asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(0xffffffff));


	    asm volatile ("mov r14, #0x00000064");
	    asm volatile ("orr r14, r14, #0x00000000");

	    asm volatile ("ldr r14, [r14]");

	    /* store some sort of start of frame marker */
	    asm volatile ("stmdb	sp!, {r12}"); 		/* save r12 */
	      asm volatile ("mov r12, #0x00034");
	      asm volatile ("orr r12, r12, #0x1200");
	      asm volatile ("str r12, [r14]");
	    asm volatile ("ldmia	sp!, {r12}"); 	/* restore r12, this gets us back out return addr */

	    /* save location of read/write instruction */
	    asm volatile ("subs r12, r12, #8");
	    asm volatile ("str r12, [r14, #0x38]");

	    /* now save the actual instruction itself */
	    //asm volatile ("ldr r12, [r12]");
	    //asm volatile ("str r12, [r14, #0x3c]");

	  asm volatile ("ldmia	sp!, {r12}"); 		/* restore r12 again, now real value */

	  /* now store all our registers */
	  asm volatile ("str r0, [r14, #0x4]");
	  asm volatile ("str r1, [r14, #0x8]");
	  asm volatile ("str r2, [r14, #0xc]");
	  asm volatile ("str r3, [r14, #0x10]");
	  asm volatile ("str r4, [r14, #0x14]");
	  asm volatile ("str r5, [r14, #0x18]");
	  asm volatile ("str r6, [r14, #0x1c]");
	  asm volatile ("str r7, [r14, #0x20]");
	  asm volatile ("str r8, [r14, #0x24]");
	  asm volatile ("str r9, [r14, #0x28]");
	  asm volatile ("str r10, [r14, #0x2c]");
	  asm volatile ("str r11, [r14, #0x30]");
	  asm volatile ("str r12, [r14, #0x34]");

	  /* increment the pointer */
	  asm volatile ("mov r0, #0x40");
	  asm volatile ("add r14, r14, r0");
	  asm volatile ("mov r0, #0x64");
	  asm volatile ("str r14, [r0]");

	asm volatile ("ldmia   sp!, {r14}");

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

	asm volatile ("Rego0: .word 0");
	asm volatile ("Rego1: .word 0");
	asm volatile ("DumpOffset: .word 0");
	asm volatile ("HandlersEnd: .word 0");
}
