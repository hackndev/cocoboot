/* Sample kernel source
 * 
 * This simply blinks the LEDs on PalmLD.
 * */

#define	ADDR	*(unsigned volatile long *)

void kmain()
{
	ADDR 0x40e00028 |= 0x00100000;	/* clear GPIO 52 */
	ADDR 0x40e0002c |= 0x40000000;	/* clear GPIO 94 */
	for (;;) {
		if ((ADDR 0x40900000) % 2) {
			if ((ADDR 0x40e00008) & 0x40000000)
				ADDR 0x40e0002c |= 0x40000000;	/* clear GPIO 94 */
			ADDR 0x40e0001c |= 0x00100000;	/* set GPIO 52 */
		} else {
			if ((ADDR 0x40e00004) & 0x00100000)
				ADDR 0x40e00028 |= 0x00100000;	/* clear GPIO52 */
			ADDR 0x40e00020 |= 0x40000000;	/* set GPIO 94 */
		}
	}
}
