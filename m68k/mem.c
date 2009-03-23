/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include <MemoryMgr.h>
#include "cocoboot.h"
#include "mem.h"
#include "cpu.h"
#include "regs.h"

UInt32 get_ram_base()
{
	UInt32 cpu = get_cpu();
	if ((cpu & CPU_VENDOR_MASK) == CPUV_TI) {
		return 0x10000000;
	} else if ((cpu & CPU_VENDOR_MASK) == CPUV_INTEL) {
		if ((cpu & CPU_MODEL_MASK) == CPU_SA1100) {
			return 0xc0000000;
		} else {
			return 0xa0000000;
		}
	} else if ((cpu & CPU_VENDOR_MASK) == CPUV_ARM) {
		/* Motorola / Freescale i.MX */
	        if ((cpu & CPU_MODEL_MASK) == CPU_920T) {
			return 0x08000000;
		}
		return 0x0;
	}
	return 0;
}

UInt32 get_gen_ram_size()
{
	/* PalmOS seems to not report the correct RAM size, so we round
	 * to the next highest power of 2
	 */
	UInt32 reported = get_reported_ram_size() + 0x00300000; /* 3mb to cover system */
	UInt32 size = 0x100000;	/* start at 1mb */
	do {
		size <<= 1;
	} while (size < reported);
	return size;
}

UInt32 xscale_get_partition_pair_size(UInt32 mem)
{
	return ((UInt32)(1 << (8 + ((mem >> 3) & 0x3))) *
		(UInt32)(1 << (11 + ((mem >> 5) & 0x3))) *
		(UInt32)((mem & 0x1) + ((mem >> 1) & 0x1)) *
		((mem & 0x4) ? 16 : 32) ) >> (1 + !!(mem & 0x1000));
}

UInt32 get_ram_size()
{
	UInt32 mem;
	if ((get_cpu() & CPU_VENDOR_MASK) == CPUV_INTEL) {
		mem = EndianSwap32(*(UInt32 *)phys_to_virt(MDCNFG));
		return (xscale_get_partition_pair_size(mem) +
			xscale_get_partition_pair_size(mem >> 16));
	} else
		return get_gen_ram_size();
}

UInt32 get_reported_ram_size()
{
	UInt32 ram_size = 0;
	MemCardInfo(0, NULL, NULL, NULL, NULL, NULL, &ram_size, NULL);
	return ram_size;
}

/* Find out the translation table base physical address */
UInt32 get_ttb()
{
	static UInt32 ttb = 0;

	if (!ttb) {
		push_uint32(arm_stack, 2);	/* register 2 */
		push_uint32(arm_stack, 15);	/* coprocessor 15 */
		ttb = call_arm(arm_stack, ARM_read_cp) & 0xffffc000;
	}
	return ttb;
}

UInt32 get_virt_ttb()
{
	/* FIXME: Hack alert. We're making an assumption about where PalmOS maps the first part of RAM..
	 * Is there a better way of doing this? we'll need to hardcode the value for T|T2 
	 */
	return get_ttb() - get_tt_offset();
}

UInt32 virt_to_phys(UInt32 virt)
{
	UInt32 phys;
	UInt32 ttb = get_virt_ttb();
	UInt32 *fld_p = (UInt32*) (ttb + ((virt >> 20) << 2));
	UInt32 fld = EndianFix32(*fld_p);

	if ((fld & FLD_MASK) == 0) {	// invalid
		phys = 0;
	} else if ((fld & FLD_MASK) == FLD_SECTION) {	// section
		phys = (fld & 0xfff00000) | (virt & 0x000fffff);
	} else {
		phys = 0;	// TODO: small and coarse pages
	}

	return phys;
}

UInt32 phys_to_virt(UInt32 phys)
{
	UInt32 ttb = get_virt_ttb();
	UInt32 table_index, *pfld, fld;

	if (!ttb)
		return NULL;

	for (table_index = 0; table_index <= 0xfff; table_index++) {
		pfld = (UInt32 *) (ttb | (table_index << 2));
		fld = EndianFix32(*pfld);

		if ((fld & FLD_MASK) != FLD_SECTION)
			continue;	/* todo: handle other page sizes */

		if ((fld & 0xfff00000) == (phys & 0xfff00000)) {
			// found!
			return (phys & 0x000fffff) | (table_index << 20);
		}
	}

	return NULL;
}


/** 
 * Find an empty virtual address and map phys there.
 *
 * Original code from HARAET/P
 */
UInt32 map_mem(UInt32 phys)
{
	UInt32 *tt = (UInt32*) get_virt_ttb();
	UInt32 i, rec;
	UInt32 va;

	if (!tt) return NULL;

	/* search for a free virtual address */
	for (i = 0; i <= 0xfff; i++) {
		rec = EndianFix32(tt[i]);

		if ((rec & FLD_MASK) == FLD_INVALID) {
			/* found a free virtual address section */
			rec = (phys & 0xfff00000) | 0xc02;
			tt[i] = EndianFix32(rec);
			va = i << 20;
			return va | (phys & 0x000fffff);
		}

	}

	return NULL;
}

