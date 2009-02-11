/* Sample kernel source
 * 
 * This simply blinks the LEDs on PalmLD.
 * */

#define delay(i)	for(i = 0; i < 0x200000; i++)

void kmain()
{
	int i;
	(*(unsigned long *)0x40e00020) &= ~0x40000000;
	(*(unsigned long *)0x40e0001c) &= ~0x00100000;
	for (;;) {
		(*(unsigned long *)0x40e00020) &= ~0x40000000;
		(*(unsigned long *)0x40e0001c) |= 0x00100000;
		delay(i);
		(*(unsigned long *)0x40e0001c) &= ~0x00100000;
		(*(unsigned long *)0x40e00020) |= 0x40000000;
		delay(i);
	}
}
