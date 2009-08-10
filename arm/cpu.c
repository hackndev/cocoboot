
#define u32 unsigned long
#define io_p2v(PhAdd)      (PhAdd)
#define __REG(x)        (*((volatile u32 *)io_p2v(x)))

#define GCR             __REG(0x4050000C)  /* Global Control Register */
#define GCR_ACLINK_OFF  (1 << 3)        /* AC-link Shut Off */
#define GCR_PRIRDY_IEN  (1 << 8)        /* Primary Ready Interrupt Enable */

#define ICMR            __REG(0x40D00004)  /* Interrupt Controller Mask Register */
#define ICLR            __REG(0x40D00008)  /* Interrupt Controller Level Register */

#define GRER0           __REG(0x40E00030)  /* GPIO Rising-Edge Detect Register GPIO<31:0> */
#define GRER1           __REG(0x40E00034)  /* GPIO Rising-Edge Detect Register GPIO<63:32> */
#define GRER2           __REG(0x40E00038)  /* GPIO Rising-Edge Detect Register GPIO<80:64> */

#define GFER0           __REG(0x40E0003C)  /* GPIO Falling-Edge Detect Register GPIO<31:0> */
#define GFER1           __REG(0x40E00040)  /* GPIO Falling-Edge Detect Register GPIO<63:32> */
#define GFER2           __REG(0x40E00044)  /* GPIO Falling-Edge Detect Register GPIO<80:64> */

#define GEDR0           __REG(0x40E00048)  /* GPIO Edge Detect Status Register GPIO<31:0> */
#define GEDR1           __REG(0x40E0004C)  /* GPIO Edge Detect Status Register GPIO<63:32> */
#define GEDR2           __REG(0x40E00050)  /* GPIO Edge Detect Status Register GPIO<80:64> */

#define UDCCR           __REG(0x40600000)  /* UDC Control Register */
#define CKEN            __REG(0x41300004)  /* Clock Enable Register */

void setup_xscale_cpu(int elf)
{
	/* Interrupts off */
	asm volatile ("mrs r0, cpsr");
	asm volatile ("orr r0, r0, #0xc0");
	asm volatile ("msr cpsr, r0");

	GCR |= GCR_ACLINK_OFF;       // shut off the AC97 audio controlller
	GCR &= ~GCR_PRIRDY_IEN;      // mask an interrupt that causes sound driver to hang

	/* mask all interrupts */
	ICMR = 0;

	ICLR = 0;

	GFER0 = 0;
	GFER1 = 0;
	GFER2 = 0;
	GRER0 = 0;
	GRER1 = 0;
	GRER2 = 0;
	GEDR0 = GEDR0;
	GEDR1 = GEDR1;
	GEDR2 = GEDR2;

	UDCCR = 0;

	if (!elf)
		CKEN &= 0x580200;
}
