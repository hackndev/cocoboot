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

/* WIP ... this will go into fn below */
int mytest(UInt32 *img) {
	struct elf32_hdr *ehdr = img;
	struct elf32_phdr *phdr;
	struct elf32_shdr *shdr;

	/* phdr contains 0th entry from Program Header table
	 * shdr contains 2nd entry from Section Header table
	 * */
	phdr = img + ((ehdr->e_phoff + (ehdr->e_phentsize * 0)) >> 2);
	shdr = img + ((ehdr->e_shoff + (ehdr->e_shentsize * 2)) >> 2);

	return shdr->sh_addr;
}

/* relocate_elf()
 *
 * Relocate the ELF file in memory ... ToDo
 * */
void relocate_elf(UInt32 *img, UInt32 size)
{
	struct elf32_hdr *ehdr = img;

	for(;;);
}
