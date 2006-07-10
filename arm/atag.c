/*
 * Tag setup code and structures were originally from the GPL'd article:
 *
 * Booting ARM Linux
 * Copyright (C) 2004 Vincent Sanders
 * http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html
 *
 */

#include <PalmOS.h>

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
		UInt32 size; /* length of tag in words including this header */
		UInt32 tag;  /* tag type */
};

struct atag_core {
		UInt32 flags;              /* bit 0 = read-only */
		UInt32 pagesize;           /* systems page size (usually 4k) */
		UInt32 rootdev;            /* root device number */
};

struct atag_mem {
		UInt32     size;   /* size of the area */
		UInt32     start;  /* physical start address */
};

struct atag_videotext {
		UInt8              x;           /* width of display */
		UInt8              y;           /* height of display */
		UInt16             video_page;
		UInt8              video_mode;
		UInt8              video_cols;
		UInt16             video_ega_bx;
		UInt8              video_lines;
		UInt8              video_isvga;
		UInt16             video_points;
};

struct atag_ramdisk {
		UInt32 flags;      /* bit 0 = load, bit 1 = prompt */
		UInt32 size;       /* decompressed ramdisk size in _kilo_ bytes */
		UInt32 start;      /* starting block of floppy-based RAM disk image */
};

struct atag_initrd2 {
		UInt32 start;      /* physical start address */
		UInt32 size;       /* size of compressed ramdisk image in bytes */
};

struct atag_serialnr {
		UInt32 low;
		UInt32 high;
};

struct atag_revision {
		UInt32 rev;
};

struct atag_videolfb {
		UInt16             lfb_width;
		UInt16             lfb_height;
		UInt16             lfb_depth;
		UInt16             lfb_linelength;
		UInt32             lfb_base;
		UInt32             lfb_size;
		UInt8              red_size;
		UInt8              red_pos;
		UInt8              green_size;
		UInt8              green_pos;
		UInt8              blue_size;
		UInt8              blue_pos;
		UInt8              rsvd_size;
		UInt8              rsvd_pos;
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


#define tag_next(t)     ((struct atag *)((UInt32 *)(t) + (t)->hdr.size))
#define tag_size(type)  ((sizeof(struct atag_header) + sizeof(struct type)) >> 2)

static UInt32 strlen(const char *s)
{
	UInt32 i=0;
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
	(*params)->u.core.rootdev = 0x100;      /* zero root device (typicaly overidden from commandline )*/

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_ramdisk_tag(struct atag **params, UInt32 size)
{
	(*params)->hdr.tag = ATAG_RAMDISK;         /* Ramdisk tag */
	(*params)->hdr.size = tag_size(atag_ramdisk);  /* size tag */

	(*params)->u.ramdisk.flags = 0;            /* Load the ramdisk */
	(*params)->u.ramdisk.size = size;          /* Decompressed ramdisk size */
	(*params)->u.ramdisk.start = 0;            /* Unused */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_initrd2_tag(struct atag **params, UInt32 start, UInt32 size)
{
	(*params)->hdr.tag = ATAG_INITRD2;         /* Initrd2 tag */
	(*params)->hdr.size = tag_size(atag_initrd2);  /* size tag */

	(*params)->u.initrd2.start = start;        /* physical start */
	(*params)->u.initrd2.size = size;          /* compressed ramdisk size */

	*params = tag_next(*params);              /* move pointer to next tag */
}

static void
setup_mem_tag(struct atag **params, UInt32 start, UInt32 len)
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

void setup_atags(UInt32 tag_base, UInt32 ram_base, UInt32 ram_size, const char *cmd_line, 
		UInt32 initrd_base, UInt32 initrd_size)
{
	struct atag *t;

	setup_core_tag(&t, (void*)(tag_base), 4096);
	setup_mem_tag(&t, ram_base, ram_size);
	setup_cmdline_tag(&t, cmd_line);

	if (initrd_size) {
		setup_initrd2_tag(&t, initrd_base, initrd_size);
	}
	
	setup_end_tag(&t);
}
