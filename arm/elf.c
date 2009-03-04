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
 * Copy a section from file to given location which is wiped before
 * */
static void copy_section(unsigned char *dst, unsigned char *src,
			 UInt32 clear_size, UInt32 copy_size,
			 UInt32 pv_offset)
{
	int i;
	unsigned char* dst_base = dst;

	for (i = 0 ; i < clear_size; i++)
		*(dst - pv_offset + i) = 0x00;

	dst = dst_base;
	for (i = 0 ; i < copy_size; i++)
		*(dst - pv_offset + i) = *(src + i);

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
	struct elf32_shdr *shdr;
	unsigned int i;
	unsigned long pvdelta = 0;

#define	PF_MASK	0x7
#define	PT_LOAD	0x01

	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = (struct elf32_phdr *)((char *)img +
			(ehdr->e_phoff + (ehdr->e_phentsize * i)));
		if (!pvdelta)
			pvdelta = phdr->p_vaddr - phdr->p_paddr;
		if (!(phdr->p_type == PT_LOAD && (phdr->p_flags & PF_MASK)))
			continue;
		copy_section((unsigned char *)phdr->p_vaddr,
				((unsigned char *)img + phdr->p_offset),
				phdr->p_memsz, phdr->p_filesz, pvdelta);
	}

	/* Jump to kernel */
	boot_elf(ehdr->e_entry);
}
