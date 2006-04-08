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
#include <PceNativeCall.h>
#include <MemGlue.h>
#include <MemoryMgr.h>
//#include <string.h>
#include <stdio.h>
#include <VFSMgr.h>

#include "cocoboot.h"
#include "cocoboot_r.h"
#include "mainform.h"

#include "mem.h"
#include "cpu.h"

ArmStack arm_stack[20];
ArmGlobals arm_globals;
FormPtr mainform;

void SetFieldTextFromStr(FieldPtr fldP, char *strP, Boolean redraw)
{
	MemHandle oldTxtH;
	MemHandle txtH;
	MemPtr txtP;
	Boolean fieldWasEditable;
	FieldAttrType attrs;

	if (fldP == NULL)
		return;

	// get some space in which to stash the string.
	txtH = MemHandleNew(StrLen(strP) + 1);
	txtP = MemHandleLock(txtH);
	// copy the string.
	StrCopy((char *) txtP, strP);
	MemHandleUnlock(txtH);

	FldGetAttributes(fldP, &attrs);
	fieldWasEditable = attrs.editable;
	attrs.editable = true;
	FldSetAttributes(fldP, &attrs);

	oldTxtH = FldGetTextHandle(fldP);
	FldSetTextHandle(fldP, txtH);

	attrs.editable = fieldWasEditable;
	FldSetAttributes(fldP, &attrs);

	if (oldTxtH != NULL)
		MemHandleFree(oldTxtH);

	if (redraw)
		FldDrawField(fldP);
}

void log_write(char *str)
{
	char buf[2048];
	FormPtr frmP = mainform;//FrmGetActiveForm();
	FieldPtr fldP =
	    FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, LogField));
	if (FldGetTextPtr(fldP)) {
		StrCopy(buf, FldGetTextPtr(fldP));
		StrCat(buf, str);
		SetFieldTextFromStr(fldP, buf, true);
	} else {
		SetFieldTextFromStr(fldP, str, true);
	}
}

void lprintf(const char *template, ...)
{
	char buf[2048];

	va_list ap;

	va_start(ap, template);
	vsprintf(buf, template, ap);
	va_end(ap);

	log_write(buf);
}

/* ARM side communication stuff */
inline void push_uint32(ArmStack * stack, UInt32 n)
{
	stack[++stack[0]] = n;
}

inline UInt32 pop_uint32(ArmStack * stack)
{
	return stack[stack[0]--];
}

/**
 * Collect some basic information that the ARM-side is
 * going to need a lot.
 */
void setup_arm_globals()
{
	arm_globals.pttb = EndianFix32(get_ttb());
	arm_globals.vttb = EndianFix32(get_virt_ttb());
	arm_globals.cpu = EndianFix32(get_cpu());
	arm_globals.ram_base = EndianFix32(get_ram_base());
	arm_globals.ram_size = EndianFix32(get_ram_size());
}

/**
 * Call an ARM side function
 */
UInt32 call_arm(ArmStack * stack, UInt32 func)
{
	MemHandle arm_code_handle;
	void *arm_code = NULL;
	UInt32 ret = 3;

	arm_code_handle = DmGetResource('armc', 0);
	if (!arm_code_handle) {
		lprintf("Error: Unable to get handle on ARM code.\n");
		return 1;
	}

	arm_code = MemHandleLock(arm_code_handle);
	if (!arm_code_handle) {
		lprintf("Error: Unable to lock ARM code.\n");
		return 2;
	}

	push_uint32(stack, func);
	push_uint32(stack, (UInt32)&arm_globals);

	ret = PceNativeCall(arm_code, stack);
	MemHandleUnlock(arm_code_handle);

	return ret;
}

void event_loop()
{
	EventType event;
	UInt16 err;
	FormPtr form;
	Int16 form_id;

	do {
		EvtGetEvent(&event, 200);

		if (SysHandleEvent(&event))
			continue;
		if (MenuHandleEvent(NULL, &event, &err))
			continue;

		if (event.eType == frmLoadEvent) {
			form_id = event.data.frmLoad.formID;
			mainform = form = FrmInitForm(form_id);
			FrmSetActiveForm(form);

			if (form_id == MainForm) {
				FrmSetEventHandler(form, mainform_event);
			}
		}
		if (event.eType == frmOpenEvent) {
			FrmDrawForm(FrmGetActiveForm());
		}

		FrmDispatchEvent(&event);
	} while (event.eType != appStopEvent);
}

UInt16 start_app()
{
	arm_stack[0] = 0;
	FrmGotoForm(MainForm);

	setup_arm_globals();
	return 0;
}

void stop_app()
{
}

UInt32 PilotMain(UInt16 launch_code, MemPtr cmd_PBP, UInt16 launch_flags)
{
	UInt16 err;
	if (launch_code == sysAppLaunchCmdNormalLaunch) {
		err = start_app();
		if (err) {
			return err;
		}

		event_loop();

		stop_app();

	}
	return 0;
}
