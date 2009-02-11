/* Sample kernel source
 * 
 * This just turns on LED on PalmLD and enters endless loop.
 * */

void kmain()
{
	(*(unsigned long *)0x40e0001c) |= 0x00100000;
	for(;;);
}
