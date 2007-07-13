#ifndef _ARM_H_
#define _ARM_H_

#define ARM_MODE
#include "shared.h"

/* arm.c */
unsigned long read_cp(unsigned long cp, unsigned long reg);
UInt32 virt_to_phys(ArmGlobals *g, UInt32 virt);
void map(ArmGlobals *g, UInt32 phys, UInt32 virt);

/*cpu.c*/
void setup_xscale_cpu(void);

/* boot.c */
UInt32 boot_linux(ArmGlobals *g, void *kernel, UInt32 kernel_size,
		  void *initrd, UInt32 initrd_size, char *cmdline);

/* atag.c */
void setup_atags(UInt32 tag_base, UInt32 ram_base, UInt32 ram_size, const char *cmd_line, 
		UInt32 initrd_base, UInt32 initrd_size);


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

#define CPWAIT asm ("mrc p15, 0, r10, c2, c0, 0");asm ("mov r0, r0");\
               asm ("sub pc, pc, #4");

#endif
