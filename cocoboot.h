#include "shared.h"


#define CREATOR_ID 3007
#define FEATURE_NUM 35

extern ArmStack arm_stack[];

void lprintf (const char *template, ...);
UInt32 call_arm(ArmStack *stack, UInt32 func);
