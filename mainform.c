#include <PalmOS.h>
#include "cocoboot.h"
#include "cocoboot_r.h"
#include "mainform.h"
#include "cpu.h"
#include "mem.h"
#include "regs.h"
#include <stdio.h>

UInt32 reg(UInt32 addr);

/* read a value from the given physical address */
UInt32 reg(UInt32 phys) {
	UInt32 *addr = (UInt32*)phys_to_virt(phys);
	UInt32 val;
	if(!addr) return 0;
	val = *addr;
	return EndianSwap32(val);
}

void lcd_info()
{
	char msg[255];
	UInt32 framebuffer = 0;

	if(!phys_to_virt(LCCR0)) {
		FrmCustomAlert(ErrorAlert, "Unable to find virtual address of"
			       " LCD registers.", " ", " ");
		return;
	}

	/* fetch location of framebuffer by LCD DMA descriptor */
	if(reg(FDADR0)) {
		framebuffer = reg( reg(FDADR0) + DMA_SRC );
	}

	sprintf(msg, "  LCCR0: 0x%08lx\n"
		"  LCCR1: 0x%08lx\n"
		"  LCCR2: 0x%08lx\n"
		"  LCCR3: 0x%08lx\n"
		"  FDADR0: 0x%08lx\n"
		"  buffer: 0x%08lx"
		,
		reg(LCCR0), 
		reg(LCCR1),
		reg(LCCR2),
		reg(LCCR3),
		reg(FDADR0),
		framebuffer);

	FrmCustomAlert(InfoAlert, "PXA LCD registers:", msg, " ");

}

void lcd_test()
{
	char msg[255];
	UInt32 ret;

	ret = call_arm(arm_stack, ARM_fb_test);

	sprintf(msg, "0x%08lx", ret);
	FrmCustomAlert(InfoAlert, "LCD test result:", msg, " ");
}


void cpu_info()
{
	char msg[255];
	UInt32 dev[2], cpu;

	dev[0] = get_dev_id();
	dev[1] = 0; /* string terminator ;-) */

	cpu = get_cpu();

	sprintf(msg, "Machine: %s\n"
		"CPU ID: 0x%08lx\n"
		"CPU: %s %s\n",
		(char*)dev,
		get_cpu_id(),
		get_cpu_vendor(cpu),
		get_cpu_name(cpu));

	FrmCustomAlert(InfoAlert, msg, " ", " ");

}

void mem_info()
{
	char msg[255];

	sprintf(msg, "  RAM base: 9x%lx\n"
		"  Size: %ldmb (0x%lx)\n"
		"  Phys TTB: 0x%lx\n"
		"  Virt TTB: 0x%lx",
		get_ram_base(),
		get_ram_size() >> 20, get_reported_ram_size(),
		get_ttb(), get_virt_ttb());

	FrmCustomAlert(InfoAlert, "Memory:", msg, " ");

}


Boolean mainform_menu_event(Int16 id)
{
	switch(id) {
	case MenuItemLCD:
		lcd_info();
		return true;
	case MenuItemLCDTest:
		lcd_test();
		return true;
	case MenuItemCPU:
		cpu_info();
		return true;
	case MenuItemMem:
		mem_info();
		return true;

	}
	return false;
}

Boolean mainform_event(EventPtr event)
{
	Boolean handled = false;
	FormPtr form = NULL;


	if (event->eType == frmOpenEvent) {
		form = FrmGetActiveForm();
		FrmDrawForm(form);
		handled = true;
		lprintf("Mapped 0x44000000 => %lx\n", map_mem(0x44000000));

	} else if (event->eType == menuEvent) {
		return mainform_menu_event(event->data.menu.itemID);
	}

	return handled;
}
