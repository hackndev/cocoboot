/*
 * Tag setup code and structures were originally from the GPL'd article:
 *
 * Booting ARM Linux
 * Copyright (C) 2004 Vincent Sanders
 * http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html
 *
 */

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

/* list of possible tags */
#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009

/* structures for each atag */
struct atag_header {
		u32 size; /* length of tag in words including this header */
		u32 tag;  /* tag type */
};

struct atag_core {
		u32 flags;              /* bit 0 = read-only */
		u32 pagesize;           /* systems page size (usually 4k) */
		u32 rootdev;            /* root device number */
};

struct atag_mem {
		u32     size;   /* size of the area */
		u32     start;  /* physical start address */
};

struct atag_videotext {
		u8              x;           /* width of display */
		u8              y;           /* height of display */
		u16             video_page;
		u8              video_mode;
		u8              video_cols;
		u16             video_ega_bx;
		u8              video_lines;
		u8              video_isvga;
		u16             video_points;
};

struct atag_ramdisk {
		u32 flags;      /* bit 0 = load, bit 1 = prompt */
		u32 size;       /* decompressed ramdisk size in _kilo_ bytes */
		u32 start;      /* starting block of floppy-based RAM disk image */
};

struct atag_initrd2 {
		u32 start;      /* physical start address */
		u32 size;       /* size of compressed ramdisk image in bytes */
};

struct atag_serialnr {
		u32 low;
		u32 high;
};

struct atag_revision {
		u32 rev;
};

struct atag_videolfb {
		u16             lfb_width;
		u16             lfb_height;
		u16             lfb_depth;
		u16             lfb_linelength;
		u32             lfb_base;
		u32             lfb_size;
		u8              red_size;
		u8              red_pos;
		u8              green_size;
		u8              green_pos;
		u8              blue_size;
		u8              blue_pos;
		u8              rsvd_size;
		u8              rsvd_pos;
};

struct atag_cmdline {
	char    cmdline[1];     /* this is the minimum size */
};

struct atag {
	struct atag_header hdr;
	union {
		struct atag_core         core;
		struct atag_mem          mem;
		struct atag_videotext    videotext;
		struct atag_ramdisk      ramdisk;
		struct atag_initrd2      initrd2;
		struct atag_serialnr     serialnr;
		struct atag_revision     revision;
		struct atag_videolfb     videolfb;
		struct atag_cmdline      cmdline;
	} u;
};


#define tag_next(t)     ((struct atag *)((u32 *)(t) + (t)->hdr.size))
#define tag_size(type)  ((sizeof(struct atag_header) + sizeof(struct type)) >> 2)

static u32 strlen(const char *s)
{
	u32 i=0;
	while (s[i]) i++;
	return i;
}

static char *strcpy(char *dest, const char *src)
{
	while ((*dest = *src)) {
		dest++;
		src++;
	}
	return dest;
}

static void setup_core_tag(struct atag **params, void * address,long pagesize)
{
	*params = (struct atag *)address;         /* Initialise parameters to start at given address */

	(*params)->hdr.tag = ATAG_CORE;            /* start with the core tag */
	(*params)->hdr.size = tag_size(atag_core); /* size the tag */

	(*params)->u.core.flags = 1;               /* ensure read-only */
	(*params)->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
	(*params)->u.core.rootdev = 0;             /* zero root device (typicaly overidden from commandline )*/

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_ramdisk_tag(struct atag **params, u32 size)
{
	(*params)->hdr.tag = ATAG_RAMDISK;         /* Ramdisk tag */
	(*params)->hdr.size = tag_size(atag_ramdisk);  /* size tag */

	(*params)->u.ramdisk.flags = 0;            /* Load the ramdisk */
	(*params)->u.ramdisk.size = size;          /* Decompressed ramdisk size */
	(*params)->u.ramdisk.start = 0;            /* Unused */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_initrd2_tag(struct atag **params, u32 start, u32 size)
{
	(*params)->hdr.tag = ATAG_INITRD2;         /* Initrd2 tag */
	(*params)->hdr.size = tag_size(atag_initrd2);  /* size tag */

	(*params)->u.initrd2.start = start;        /* physical start */
	(*params)->u.initrd2.size = size;          /* compressed ramdisk size */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_mem_tag(struct atag **params, u32 start, u32 len)
{
	(*params)->hdr.tag = ATAG_MEM;             /* Memory tag */
	(*params)->hdr.size = tag_size(atag_mem);  /* size tag */

	(*params)->u.mem.start = start;            /* Start of memory area (physical address) */
	(*params)->u.mem.size = len;               /* Length of area */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_cmdline_tag(struct atag **params, const char * line)
{
	int linelen = strlen(line);

	if(!linelen)
		return;                             /* do not insert a tag for an empty commandline */

	(*params)->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
	(*params)->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

	strcpy((*params)->u.cmdline.cmdline,line); /* place commandline into tag */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_end_tag(struct atag **params)
{
	(*params)->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
	(*params)->hdr.size = 0;                   /* zero length */
}

void setup_atags(unsigned long rambase, unsigned long ramsize, const char *cmdline)
{
	struct atag *t;

	setup_core_tag(&t, (void*)(rambase + 0x100), 4096);
	setup_mem_tag(&t, rambase, ramsize);
	//setup_cmdline_tag(&t, cmdline);
	setup_end_tag(&t);
/*
	unsigned long i;
	*(unsigned long*)0x41300004 |= 0x3;
	while (1) {
		*(unsigned long*)0x41300004 ^= 0x3;
		for(i=0 ; i<40000000 ; i++);
	}
*/
}
