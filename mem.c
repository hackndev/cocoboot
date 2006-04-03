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

UInt32 get_ram_base()
{
  UInt32 cpu = get_cpu();
  if((cpu & CPU_VENDOR_MASK) == CPUV_TI) {
    return 0x10000000;
  } else if((cpu & CPU_VENDOR_MASK) == CPUV_INTEL) {
    if((cpu & CPU_MODEL_MASK) == CPU_SA1100) {
      return 0xc0000000;
    } else {
      return 0xa0000000;
    }
  } else if((cpu & CPU_VENDOR_MASK) == CPUV_ARM) {
    return 0x0;
  }
  return 0;
}

UInt32 get_ram_size()
{
  /* PalmOS seems to not report the correct RAM size, so we round
   * to the next highest power of 2
   */
  UInt32 reported = get_reported_ram_size();
  UInt32 size = 0x100000; /* start at 1mb */
  do {
    size <<= 1;
  } while(size < reported);
  return size;
}

UInt32 get_reported_ram_size()
{
  UInt32 ram_size=0;
  MemCardInfo(0, NULL, NULL, NULL, NULL, NULL, &ram_size, NULL);
  return ram_size;
}

/* Find out the translation table base physical address */
UInt32 get_ttb()
{
  static UInt32 ttb = 0;

  if(!ttb) {
    push_uint32(arm_stack, 2); /* register 2 */
    push_uint32(arm_stack, 15); /* coprocessor 15 */
    ttb = call_arm(arm_stack, ARM_read_cp) & 0xffffc000;
  }
  return ttb;
}

UInt32 get_virt_ttb()
{
  /* FIXME: Hack alert. We're making an assumption about where PalmOS maps the first part of RAM..
   * Is there a better way of doing this? we'll need to hardcode the value for T|T2 
   */
  return get_ttb() & 0x00FFFFFF;
}

UInt32 virt_to_phys(UInt32 virt)
{
  UInt32 phys;
  UInt32 ttb = get_virt_ttb();
  UInt32 *fld_p = ttb + ((virt >> 20) << 2);
  UInt32 fld = EndianSwap32(*fld_p);

  if((fld & 3) == 0) { // invalid
    phys = 0;
  } else if((fld & 3) == 1) { // page
    phys = (fld & 0xfff00000) | (virt & 0x000fffff);
  } else if((fld & 3) == 2) { // section
    phys = 0; // TODO
  }
  //sprintf("FLB pos %lx\n", fld);
  return phys;
}

/* find an empty virtual address and map phys there */
/* original code from HARET/P */
void *map_mem(UInt32 phys)
{
  UInt32 ttb = get_virt_ttb();
  UInt32 i = ttb;
  UInt32 entry, entry_rec, va;
  if(!ttb) return NULL;
  
  while (i <= ttb+0x3FFC) {
    // is 0x3ffc (0x4000-0x7ffc) working for all handhelds?
    entry = EndianSwap32(*((UInt32 *)i));
    if(entry & 3) {
      entry_rec = phys;
      entry_rec = (entry_rec & 0xFFF00000)+0x0C02;
      va = ((i & 0x3FFC) >> 2) << 0x14;
      entry = EndianSwap32(entry);
      *((UInt32 *)i) = entry;
      return va;
    }
    i+=4;
  }

  return NULL;
}
