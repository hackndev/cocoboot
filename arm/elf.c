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
static void copy_section(UInt32 *dst, UInt32 *src,
			 UInt32 clear_size, UInt32 copy_size,
			 UInt32 pv_offset)
{
	int i;
	UInt32* dst_base = dst;

	for (i = 0 ; i < clear_size; i++)
		*(dst - pv_offset + i) = 0x00;

	dst = dst_base;
	for (i = 0 ; i < copy_size; i++)
		*(dst - pv_offset + i) = *(src + i);

}

static void boot_elf(UInt32 entry)
{
	asm volatile (	"mrs r12, cpsr;"
			"bic r12, r12, #0x1f;"
			"orr r12, r12, #0x13;"
			"msr cpsr_all, r12;"
			"mov r0, %0;"
			"mov r1, #0x30;"
			"mov r2, #0;"
			"mcr 15, 0, r1, c1, c0, 0;"
			"mcr 15, 0, r2, c8, c7, 0;"
			"mov pc, r0"
			:: "r"(entry) : "r0","r1","r2");
}

/* relocate_elf()
 *
 * Relocate the ELF file in memory ... ToComplete
 * */
void relocate_elf(UInt32 *img, UInt32 size, UInt32 mach)
{
	struct elf32_hdr *ehdr = (struct elf32_hdr *) img;
	struct elf32_phdr *phdr;
	unsigned int i;
	unsigned long pvdelta = 0, cpsr;

#define	PF_MASK	0x7
#define	PT_LOAD	0x01

	for (i = 0; i < ehdr->e_phnum; i++) {
		phdr = (struct elf32_phdr *)((char *)img +
			(ehdr->e_phoff + (ehdr->e_phentsize * i)));
		if (!pvdelta)
			pvdelta = phdr->p_vaddr - phdr->p_paddr;
		if (!(phdr->p_type == PT_LOAD && (phdr->p_flags & PF_MASK)))
			continue;
		copy_section((UInt32 *)phdr->p_vaddr, img + phdr->p_offset / 4,
				phdr->p_memsz / 4, phdr->p_filesz / 4, pvdelta / 4);
	}

	asm volatile ("mrs %0, cpsr_all" : "=r" (cpsr));
	cpsr |= 0xc0;
	asm volatile ("msr cpsr_all, %0" :: "r" (cpsr));

	/* Shove machine ID into PSPR ... ewww! */
	*(UInt32 *)0x40f00008 = mach;

	/* Jump to kernel */
	boot_elf(ehdr->e_entry);
}
