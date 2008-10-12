
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

void setup_xscale_cpu(void)
{
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

	/*
	 * Setup resume vector for suspend on Treos. 
	 *
	 * The original bootloader will jump to the start of SDRAM (0xa0000000) on resume
	 * so we place some instructions there which will read PSPR and jump to it, which
	 * is what Linux expects.
	 */
	*((u32*) 0xa0000000) = 0xe3a00121; /* mov     r0, #0x40000008   */
	*((u32*) 0xa0000004) = 0xe280060f; /* add     r0, r0, #0xf00000 */
	*((u32*) 0xa0000008) = 0xe590f000; /* ldr     pc, [r0]          */
}
