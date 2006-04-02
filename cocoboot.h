#include "shared.h"

extern ArmStack arm_stack[];

void lprintf (const char *template, ...);
UInt32 call_arm(ArmStack *stack, UInt32 func);
