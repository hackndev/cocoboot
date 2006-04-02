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
UInt32 get_tlb()
{
  static UInt32 tlb = 0;

  if(!tlb) {
    push_uint32(arm_stack, 2); /* register 2 */
    push_uint32(arm_stack, 15); /* coprocessor 15 */
    tlb = call_arm(arm_stack, ARM_read_cp) & 0xffffc000;
  }
  return tlb;
}

void *map_mem(UInt32 phys)
{
  UInt32 tlb = get_tlb();
  
  if(!tlb) return NULL;

  return NULL;
}
