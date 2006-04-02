#include <PalmOS.h>
#include "cocoboot.h"
#include "mainform.h"
#include "cpu.h"
#include "mem.h"

Boolean mainform_event(EventPtr event)
{
  Boolean handled = false;
  FormPtr form = NULL;
  UInt32 cpu;
  UInt32 dev[2];

  if(event->eType == frmOpenEvent) {
    form = FrmGetActiveForm();
    FrmDrawForm(form);
    
    dev[0] = get_dev_id();
    dev[1] = 0;
    lprintf("Machine: %s\n", dev);

    lprintf("RAM base: 0x%lx\n", get_ram_base());
    lprintf("RAM size: %ldmb (0x%lx)\n", get_ram_size()>>20, get_reported_ram_size());
    lprintf("TLB loc: 0x%lx\n", get_tlb());

    cpu = get_cpu();
    lprintf("CPU ID: %lx\n", get_cpu_id());
    lprintf("CPU: %s %s\n", get_cpu_vendor(cpu), get_cpu_name(cpu));

    handled = true;
  }

  return handled;
}
