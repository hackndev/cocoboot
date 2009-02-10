#include <PalmOS.h>
#include "elf.h"

/* test_elf()
 *
 * Check if supplied image is in ELF format or not.
 * */
int test_elf(UInt32* img)
{
	unsigned long elfmagic = ELFMAG0 | (ELFMAG1 << 8) |
				(ELFMAG2 << 16) | (ELFMAG3 << 24);
	return (img[0] == elfmagic);
}

/* relocate_elf()
 *
 * Relocate the ELF file in memory ... ToDo
 * */
void relocate_elf(UInt32 *img, UInt32 size)
{
	for(;;);
}
