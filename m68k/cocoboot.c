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
#include <string.h>
#include <VFSMgr.h>

#include "cocoboot.h"
#include "cocoboot_r.h"
#include "mainform.h"

#include "mem.h"
#include "cpu.h"

ArmStack arm_stack[20];
ArmGlobals arm_globals;
FormPtr mainform;
UInt16 usb_port;
char console_buffer[128];
int console_buffer_fill=0;

/* A couple of these functions are originally from GaruxNG:
 * GaruxNG is Copyright (C) 2006 SCL (scl@hexview.com)
 *
 * FIXME: But doesn't this leave memory leaks?
 */
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
	arm_globals.mach_num = EndianFix32(835);
	arm_globals.tt_offset = get_tt_offset();
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

void sendf(const char *template, ...)
{
	char buf[2048];
	Err err;

	va_list ap;

	va_start(ap, template);
	vsprintf(buf, template, ap);
	va_end(ap);

	if (usb_port)
		SrmSend(usb_port, buf, strlen(buf), &err);
}


void open_console(void) {
	char *ready_text = "[Cocoboot ready]\r\n";
	Err err;

	/* open USB serial port */
	usb_port = 0;
	console_buffer_fill = 0;
	SrmOpen(serPortCradleUSBPort, 115200, &usb_port);
	if (usb_port) {
		SrmSend(usb_port, ready_text, strlen(ready_text), &err);
		sendf("Cocoboot> ");
		SrmSendFlush(usb_port);
	}
}

void close_console()
{
	Err err;
	char *exit_text = "[Cocoboot exiting]\r\n";
	if (usb_port) {
		SrmSend(usb_port, exit_text, strlen(exit_text), &err),
		SrmClose(usb_port);
		usb_port = 0;
	}
}

void console_help(void)
{
	sendf("Available commands:\r\n");
	sendf("  exit            close the console\r\n");
	sendf("  help            show this help\r\n");
	sendf("  ping [text]     reply with pong text\r\n");
}

/* index(3) - locate character in string */
char *index(const char *s, int c)
{
	while (*s) {
		if(*s == c) return s;
		s++;
	}
	return NULL;
}

void handle_command(char *cmd)
{
	char *args = index(cmd, ' ');
	if (args) {
		*(args++) = 0;
	} else {
		args = cmd + strlen(cmd);
	}

	if (!strcmp(cmd, "ping")) {
		sendf("pong %s\r\n", args);
	} else if (!strcmp(cmd, "help")) {
		console_help();
	} else if (!strcmp(cmd, "exit")) {
		close_console();
	} else {
		sendf("Unknown command '%s'. Type 'help' for help.\r\n", cmd);
	}
	sendf("Cocoboot> ");
}

void handle_console(void)
{
	Err err;
	UInt32 bytes = 0;
	int i;

	SrmReceiveCheck(usb_port, &bytes);
	if (!bytes) return;

	if (bytes > sizeof(console_buffer))
		bytes = sizeof(console_buffer);

	/* new data is available, grab it and echo it! */
	bytes = SrmReceive(usb_port, console_buffer + console_buffer_fill, bytes, 1000, &err);
	SrmSend(usb_port, console_buffer + console_buffer_fill, bytes, &err);
	SrmSendFlush(usb_port);
	console_buffer_fill += bytes;

	EvtResetAutoOffTimer();

	/* do we have a full command? */
	for (i=0; i < console_buffer_fill; i++) {
		if (console_buffer[i] == '\r' || console_buffer[i] == '\n') {
			sendf("\n");
			/* yes! process it */
			console_buffer[i] = 0;
			if (i+1 < console_buffer_fill && console_buffer[i+1] == '\n')
				console_buffer[++i] = 0;
			handle_command(console_buffer);

			/* pop the command off */
			i++;
			memmove(console_buffer, console_buffer + i, console_buffer_fill - i);
			console_buffer_fill -= i;
			i = 0;
		}
	}
}

void event_loop()
{
	EventType event;
	UInt16 err;
	FormPtr form;
	Int16 form_id;
	int delay=200;

	do {
		EvtGetEvent(&event, delay);

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

		if (usb_port) {
			handle_console();
			delay = 10;
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
	close_console();
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
