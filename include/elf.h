/* elf32_hdr.e_ident bytes definition */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7
#define EI_NIDENT	16

/* ELF magic numbers - EI_MAG[0-3] */
#define	ELFMAG0		0x7f
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'

/* ELF class - EI_CLASS */
#define	ELFCLASSNONE	0
#define	ELFCLASS32	1
#define	ELFCLASS64	2

/* ELF data format - EI_DATA */
#define	ELFDATANONE	0
#define	ELFDATA2LSB	1
#define	ELFDATA2MSB	2

struct elf32_hdr {
	unsigned char	e_ident[EI_NIDENT];
	UInt16		e_type;
	UInt16		e_machine;
	UInt32		e_version;
	UInt32		e_entry;  /* Entry point */
	UInt32		e_phoff;
	UInt32		e_shoff;
	UInt32		e_flags;
	UInt16		e_ehsize;
	UInt16		e_phentsize;
	UInt16		e_phnum;
	UInt16		e_shentsize;
	UInt16		e_shnum;
	UInt16		e_shstrndx;
};

struct elf32_phdr {
	UInt32		p_type;
	UInt32		p_offset;
	UInt32		p_vaddr;
	UInt32		p_paddr;
	UInt32		p_filesz;
	UInt32		p_memsz;
	UInt32		p_flags;
	UInt32		p_align;
};

int test_elf(UInt32 *img);
void relocate_elf(UInt32 *kernel, UInt32 size);
