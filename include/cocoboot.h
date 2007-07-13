#include "shared.h"


#define CREATOR_ID 'CcBt' 
#define FEATURE_NUM 35

extern ArmStack arm_stack[];
extern ArmGlobals arm_globals;

void lprintf (const char *template, ...);
UInt32 call_arm(ArmStack *stack, UInt32 func);

UInt32 get_tt_offset();
UInt32 get_linux_mach_id();
char *get_mach_name();
void SetFieldTextFromStr(FieldPtr fldP, char *strP, Boolean redraw);
void open_console();
