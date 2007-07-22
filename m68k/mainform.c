#include <PalmOS.h>
#include "cocoboot.h"
#include "cocoboot_r.h"
#include "mainform.h"
#include "cpu.h"
#include "mem.h"
#include "regs.h"
#include "imgloader.h"
#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <DataMgr.h>
#include <VFSMgr.h>

UInt32 reg(UInt32 addr);
int use_initrd;
int kernel_ok;

/* read a value from the given physical address */
UInt32 reg(UInt32 phys) {
	UInt32 *addr = (UInt32*)phys_to_virt(phys);
	UInt32 val;
	if(!addr) return 0;
	val = *addr;
	return EndianSwap32(val);
}

/* write a value to the given physical address */
void set_reg(UInt32 phys, UInt32 val) {
	UInt32 *addr = (UInt32*)phys_to_virt(phys);
	if(!addr) return;
	*addr = EndianSwap32(val);
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

void usb_console(void)
{
	open_console();
	FrmCustomAlert(InfoAlert, "USB Console is now enabled.", "Connect to /dev/ttyUSBn on PC.", " ");
}

#define LCCR0_LDM (1<<3)
#define LCCR0_DIS (1<<10)

void lcd_test()
{
	char msg[255];
	UInt32 ret;
	//UInt32 i, lccr0;
	
	//lccr0 = reg(LCCR0);
	
	//set_reg(LCSR, 0xffffffff); /* clear status */
	//set_reg(LCCR0, lccr0 & ~LCCR0_LDM); /* enable LCD disable done */
	//set_reg(LCCR0, (lccr0 & ~LCCR0_LDM) | LCCR0_DIS); /* disable LCD */
	
	/* wait a little */
	//for (i=0; i<100000; i++);
	
	//set_reg(LCCR3, 0x4700004);
	
	/* wait a little more */
	//for (i=0; i<100000; i++);

	/* re-enable LCD controller */
	//set_reg(LCCR0, lccr0);
	
	
	ret = call_arm(arm_stack, ARM_fb_test);

	sprintf(msg, "0x%08lx", ret);
	FrmCustomAlert(InfoAlert, "LCD test result:", msg, " ");
	
}

void dump_mmu()
{
	UInt16 volref; 
	UInt32 voliter = vfsIteratorStart; 
	FileRef file;
	Err err = VFSVolumeEnumerate(&volref, &voliter);
	char msg[255];
	UInt32 addr, phys, lastphys=0xffff, lastvirt=0xffff, startphys=0xffff, startvirt=0xffff;

	if (err != errNone) {
		FrmCustomAlert(InfoAlert, "Memory card not found.", " ", " ");
		return;
	}

	if (VFSFileOpen(volref, "/memorymap.txt", vfsModeWrite | vfsModeCreate, &file) != errNone) {
		FrmCustomAlert(InfoAlert, "Can't open memorymap.txt for writing", " ", " ");
		return;
	}

	addr = 0;
	while (1) {
		phys = virt_to_phys(addr);
		if (phys != lastphys + 0x00100000) {
			if (startphys && startphys!=0xffff) {
				sprintf(msg, "%08lx-%08lx -> %08lx-%08lx\n", startvirt, lastvirt, startphys, lastphys);
				VFSFileWrite(file, StrLen(msg), msg, NULL);
			}
			startphys = phys;
			startvirt = addr;
		}
		lastphys = phys;
		lastvirt = addr;
		addr += 0x00100000;
		if (addr >= 0xff000000) break;
	}
	sprintf(msg, "%08lx-%08lx -> %08lx-%08lx\n", startvirt, lastvirt, startphys, lastphys);
	VFSFileWrite(file, StrLen(msg), msg, NULL);
	VFSFileClose(file);	
	
	FrmCustomAlert(InfoAlert, "/memorymap.txt created", " ", " ");
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

	sprintf(msg, "  RAM base: 0x%lx\n"
		"  Size: %ldmb (0x%lx)\n"
		"  Phys TTB: 0x%lx\n"
		"  Virt TTB: 0x%lx",
		get_ram_base(),
		get_ram_size() >> 20, get_reported_ram_size(),
		get_ttb(), get_virt_ttb());

	FrmCustomAlert(InfoAlert, "Memory:", msg, " ");

}

void start_irq_trace()
{
	Err err=0;
	UInt32 size = 1024L * 100; /* 100k should be plenty */
	UInt32 ret;
	void *buffer = NULL;
	char msg[100];

	if((err=FtrPtrNew (CREATOR_ID, FEATURE_NUM, size, &buffer))) {
		sprintf(msg, "Error: %d", err);
		FrmCustomAlert(InfoAlert, "Unable to allocate trace buffer.", msg, " ");
		return;
	}
	
	push_uint32(arm_stack, (UInt32)buffer);
	ret = call_arm(arm_stack, ARM_install_irqhandler);
	
	if (!ret) {
		FrmCustomAlert(InfoAlert, "Trace started.", " ", " ");
	} else {
		sprintf(msg, "Error: 0x%lx", ret);
		FrmCustomAlert(InfoAlert, msg, " ", " ");
	}
}

/* find some random volume to save to */
UInt16 find_some_vol(void)
{
	UInt32 it;
	UInt16 vn = 0;
	it = vfsIteratorStart;
	VFSVolumeEnumerate (&vn, &it);
	return vn;
}

/* dump our memory trace log to the memory card
 * TODO: make this safe when no trace has been run.
 */
void dump_trace_log(void)
{
	FileRef f = 0;
	UInt16 vn = find_some_vol();
	UInt32 *log = (UInt32*)EndianFix32(*(UInt32*)(0x68));
	UInt32 *end = (UInt32*)EndianFix32(*(UInt32*)(0x64));
	UInt32 val;
	Err err;
	char buf[256];
	UInt32 bigbuf[100];
	UInt32 i;
	
	sprintf(buf, "Log: log=%lx end=%lx blocks=%lx", log, end, end-log);
	FrmCustomAlert(InfoAlert, "Dumping...", buf, " ");
	err = VFSFileOpen (vn, "/cocoboot.trc", vfsModeWrite | vfsModeCreate, &f);
	if (err != errNone) goto error1; 

	while (log < end) {
		/* lets do a bunch at a time for speed */
		for (i=0; i < sizeof(bigbuf)/sizeof(UInt32) && log < end; i++) {
			bigbuf[i] = *log;
			log++;
		}
		err = VFSFileWrite(f, i*sizeof(UInt32), bigbuf, NULL);
		if (err != errNone) goto error2;
	}

	VFSFileClose (f);
	FrmCustomAlert(InfoAlert, "Trace log dumped to /cocoboot.trc.", " ", " ");
	return;

error2:
	VFSFileClose (f);
error1:
	FrmCustomAlert(InfoAlert, "Error writing to /cocoboot.memtrace.", " ", " ");
}

void show_gsm_code(void)
{
	char *buf = 0;
	UInt16 len;
	char buf2[256];
	int i;
	SysGetROMToken(0, 'GoUc', &buf, &len);
	if (!buf || len == 0xffff) {
		FrmCustomAlert(InfoAlert, "Code not found.", "This option only works on unlocked GSM phones. It does not allow you to unlock a locked phone.", " ");
		return;
	}
	strncpy(buf2, buf, len < 256 ? len : 256);
	buf2[len] = 0;
	for (i=0; i<len; i++)
		buf2[i] -= 13;
	FrmCustomAlert(InfoAlert, "GSM unlock code:", buf2, " ");
}

UInt32 load_parts(int n, char *name, void **image)
{
	/* more ugly code... */
	Err err=0;
	char loc[32];
	UInt32 size = 1000;
	UInt32 ftr_size;
	Int32 vol, bytes;

	lprintf("Loading %s... ", name);
	vol = search_image(name, loc, sizeof(loc), &size);
	if(vol < -1) goto out;

	ftr_size = size;
	while (ftr_size) {
		if(!(err=FtrPtrNew (CREATOR_ID, FEATURE_NUM + n, ftr_size, image))) {
			break;
		}

		if (ftr_size == size) {
			lprintf("alloc error, trying overwrite ram. ");
		}

		if (ftr_size < 102400)
			ftr_size = 0;
		else
			ftr_size -= 102400;
	}

	if (!ftr_size) {
		lprintf("Gave up. Boot ");
		goto out;
	}
	
	bytes = load_image(name, size, (UInt16)vol, *image);
	lprintf("%ld b ", bytes);
	
	if(bytes == size) {
		lprintf("OK.\n");
		return size;
	}

	FtrPtrFree(CREATOR_ID, FEATURE_NUM + n);

 out:
	lprintf("failed. (%d/%d)\n", vol, err);
	return 0;
}

UInt32 read_mach_id()
{
	FormPtr form = FrmGetActiveForm();
	UInt32 num;
	FieldPtr id_p = FrmGetObjectPtr(form, FrmGetObjectIndex(form, MachIdField));
	MemHandle id_h = FldGetTextHandle(id_p);
	char *id = MemHandleLock(id_h);
	num = atoi(id);
	MemHandleUnlock(id_h);
	return num;
}

char *read_command_line()
{
	FormPtr form = FrmGetActiveForm();
	FieldPtr id_p = FrmGetObjectPtr(form, FrmGetObjectIndex(form, CommandLine));
	MemHandle id_h = FldGetTextHandle(id_p);
	char *cmdline;
	if (!id_h) return "root=bug1";
	cmdline = MemHandleLock(id_h);
	if (!cmdline) return "root=bug2";
	return cmdline;
}

void boot_linux()
{
	void *kernel=NULL, *initrd=NULL;
	UInt32 kernel_size=0, initrd_size=0;
	UInt32 ret;
	char *cmdline;

	log_clear();	
	kernel_size = load_parts(0, get_option("kernel"), &kernel);
	if(kernel_size) {
		if(use_initrd) {
			initrd_size = load_parts(1, get_option("initrd"), &initrd);
		}

		if(!use_initrd || initrd_size) {
			cmdline = read_command_line();

			arm_globals.mach_num = EndianFix32(read_mach_id());

			push_uint32(arm_stack, (UInt32)cmdline);
			push_uint32(arm_stack, initrd_size);
			push_uint32(arm_stack, (UInt32)initrd);
			push_uint32(arm_stack, kernel_size);
			push_uint32(arm_stack, (UInt32)kernel);

			ret = call_arm(arm_stack, ARM_boot_linux);

			/* we're back?! Boot must have failed. */
			lprintf("Returned: %lx\n", ret);

		}
		if (initrd_size) 
			FtrPtrFree(CREATOR_ID, FEATURE_NUM + 1);
	}

	if (kernel_size) 
		FtrPtrFree(CREATOR_ID, FEATURE_NUM);
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
	case MenuItemDumpMMU:
		dump_mmu();
		return true;
	case MenuItemConsole:
		usb_console();
		return true;
	case MenuItemStartIrqTrace:
 		start_irq_trace();
 		return true; 
	case MenuItemDumpTraceLog:
 		dump_trace_log();
 		return true;
	case MenuItemGsmCode:
 		show_gsm_code();
 		return true; 

	}
	return false;
}

int check_image(char *name)
{
	char loc[32];
	UInt32 size = 0;
	Int32 vol;

	vol = search_image(name, loc, sizeof(loc), &size);
	if(vol>=-1) {
		lprintf("[%s] %s: %ld bytes\n", loc, name, size);
		return 1;
	} else {
		lprintf("%s not found! (%ld)\n", name, vol);
	}
	return 0;
}

void display_mach(FormPtr form)
{
	char id[8];
	FieldPtr id_p = FrmGetObjectPtr(form, FrmGetObjectIndex(form, MachIdField));
	FieldPtr name_p = FrmGetObjectPtr(form, FrmGetObjectIndex(form, MachNameField));

	sprintf(id, "%ld", get_linux_mach_id());
	SetFieldTextFromStr(id_p, id, true);
	SetFieldTextFromStr(name_p, get_mach_name(), true);
}



Boolean mainform_event(EventPtr event)
{
	Boolean handled = false;
	FormPtr form = NULL;
	FieldPtr cmdline_p;
	MemHandle cmdline_th;
	char *cmdline_tp;
	UInt16 size;

	if (event->eType == frmOpenEvent) {
		form = FrmGetActiveForm();

		/* setup command line buffer */
		size = 256;
		cmdline_p = FrmGetObjectPtr(form, FrmGetObjectIndex(form, CommandLine));
	        cmdline_th = MemHandleNew(size);
	        cmdline_tp = MemHandleLock(cmdline_th);
		StrCopy(cmdline_tp, get_option("cmdline")); /* default value */
		//PrefGetAppPreferences ('CcBt', 1, cmdline_tp, &size, true);
		MemHandleUnlock(cmdline_th);
		FldSetTextHandle(cmdline_p, cmdline_th);
		
		display_mach(form);
		FrmDrawForm(form);
		handled = true;
		
		kernel_ok = check_image(get_option("kernel"));
		use_initrd = check_image(get_option("initrd"));
		
		/* boot immediately if in noprompt mode */
		if (atoi(get_option("noprompt")) != 0 && kernel_ok)
			boot_linux();

	} else if (event->eType == menuEvent) {
		return mainform_menu_event(event->data.menu.itemID);
	}

	if (event->eType == ctlSelectEvent) {
		if (event->data.ctlSelect.controlID == LinuxButton) {
#ifdef WARNING
			if (FrmAlert (StartupAlert) == 1) {
				FrmCloseAllForms ();
				return 0;
			}
#endif
			boot_linux();
			return 0;
		}
	}


	return handled;
}
