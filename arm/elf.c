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

#if 0

/* USAGE: loaded(0xa0200000, 0xe3a0005c) */
#define loaded(addr, insn) \
	if ( *((unsigned long *)addr) == insn) \
		*((unsigned long *)0x40e00020) |= 0x40000000; \
	else \
		*((unsigned long *)0x40e0001c) |= 0x00100000;

/* WIP ... this will go into fn below */
int mytest(UInt32 *img) {
	struct elf32_hdr *ehdr = (struct elf32_hdr *) img;
	struct elf32_phdr *phdr;
	struct elf32_shdr *shdr;

	/* phdr contains 0th entry from Program Header table
	 * shdr contains 2nd entry from Section Header table
	 * */
	phdr = (struct elf32_phdr *)(img + ((ehdr->e_phoff +
		(ehdr->e_phentsize * 0)) >> 2));
	shdr = (struct elf32_shdr *)(img + ((ehdr->e_shoff +
		(ehdr->e_shentsize * 2)) >> 2));

	return shdr->sh_addr;
}
#endif

/* copy_section()
 *
 * Copy a section from file to given location
 * */
static void copy_section(UInt32 *img, struct elf32_phdr *phdr)
{
	UInt32 addr = phdr->p_paddr;		/* destination address */
	UInt32 offset = phdr->p_offset / 4;	/* offset in file */
	UInt32 len = phdr->p_filesz / 4;	/* size of data being loaded */

	/* Copy the section in place */
	while(len--) {
		*((unsigned long *)addr) = *(img + offset);
		addr += 4;
		offset++;
	}
}

static void boot_elf(UInt32 entry)
{
	asm volatile (	"mov pc, %0" :: "r"(entry) : "r0");
}

/* relocate_elf()
 *
 * Relocate the ELF file in memory ... ToComplete
 * */
void relocate_elf(UInt32 *img, UInt32 size)
{
	struct elf32_hdr *ehdr = (struct elf32_hdr *) img;
	struct elf32_phdr *phdr;
	unsigned int i;

	/* For each Program header, relocate it's section */
	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = (struct elf32_phdr *)(img + ((ehdr->e_phoff +
			(ehdr->e_phentsize * i)) >> 2));
		copy_section(img, phdr);
	}

	/* Jump to kernel */
	boot_elf(ehdr->e_entry);
}
