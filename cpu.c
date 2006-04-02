#include <PalmOS.h>
#include "cocoboot.h"
#include "cpu.h"


//fixme
//#define CPU_OMAP 

const char *get_cpu_vendor(UInt32 cpu)
{
  switch(cpu & CPU_VENDOR_MASK) {
  case CPUV_INTEL: return "Intel";
  case CPUV_TI: return "TI";
  case CPUV_ARM: return "Arm";
  }
  return "Unknown";
}

const char *get_cpu_name(UInt32 cpu)
{
  switch(cpu & CPU_MODEL_MASK) {
  case CPU_920T: return "920T";
  case CPU_922T: return "922T";
  case CPU_926E: return "926E";
  case CPU_940T: return "940T";
  case CPU_946E: return "946E";
  case CPU_1020E: return "1020E";

  case CPU_915T: return "915T";
  case CPU_925T: return "925T";
  case CPU_926T: return "926T";

  case CPU_SA1100: return "SA1100";
  case CPU_PXA25X: return "PXA25x/26x";
  case CPU_PXA27X: return "PXA27x";
  case CPU_PXA210: return "PXA210";
  }
  return "???";
}

UInt32 get_cpu()
{
  UInt32 cpu = 0;
  UInt32 id = get_cpu_id();

  if((id & CPU_VENDOR_MASK) == CPUV_INTEL) {
    cpu |= CPUV_INTEL;

    if(((id>>4) & 0xfff) == CPU_SA1100) {
      cpu |= CPU_SA1100;
    } else { 
      cpu |= (id >> 4) & 31;
    }
  } else {
    cpu |= id & CPU_VENDOR_MASK;
    cpu |= (id >> 4) & 0xFFF;
  }
  return cpu;
}

UInt32 get_cpu_id()
{
  static UInt32 cpu_id = 0;

  if(!cpu_id) {
    push_uint32(arm_stack, 0); /* register 0 */
    push_uint32(arm_stack, 15); /* coprocessor 15 */
    cpu_id = call_arm(arm_stack, ARM_read_cp);
  }
  return cpu_id;
}

UInt32 get_dev_id()
{
  UInt32 dev;
  FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &dev);
  return dev;
}
