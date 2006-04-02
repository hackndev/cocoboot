#include <PceNativeCall.h>
#include <Standalone.h>
#include "arm.h"

STANDALONE_CODE_RESOURCE_ID (0);

unsigned long read_cp(unsigned long cp, unsigned long reg);

/* Note: This function needs to be at the top of the file */
unsigned long arm_entry(const void *emulStateP, char *userData68KP, Call68KFuncType *call68KFuncP)
{
  ArmStack *stack = (ArmStack*)userData68KP;
  UInt32 func;
  UInt32 ret;

  if(!stack) return 0xbadbad;

  stack[0] = EndianSwap32(stack[0]);
  if(!stack[0]) {
    ret = 0xfeedf00d;
    goto out;
  }

  func = pop_uint32(stack);

  switch(func) {
  case ARM_read_cp:
    ret = read_cp(pop_uint32(stack), pop_uint32(stack));
    break;
  default:
    ret = 0xbadf0000 ^ func;
  }

 out:
  stack[0] = EndianSwap32(stack[0]);
  return ret;
}

/* Read a coprocessor register */
unsigned long read_cp(unsigned long cp, unsigned long reg)
{
  unsigned long value;
  value = 0xbabababa;
  switch (cp) {
  case 15:
    switch(reg) {
    case 0: asm("mrc p15, 0, r0, c0, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 1: asm("mrc p15, 0, r0, c1, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 2: asm("mrc p15, 0, r0, c2, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 3: asm("mrc p15, 0, r0, c3, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 4: asm("mrc p15, 0, r0, c4, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 5: asm("mrc p15, 0, r0, c5, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 6: asm("mrc p15, 0, r0, c6, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    case 7: asm("mrc p15, 0, r0, c7, c0, 0\nmov %0, r0": "=r"(value) : : "r0");  break;
    }
    break;
  }
  return value;
}

/**** Stack functions ****/

void push_uint32(ArmStack *stack, UInt32 n)
{
  stack[++stack[0]] = EndianSwap32(n);
}

UInt32 pop_uint32(ArmStack *stack)
{
  UInt32 ret = EndianSwap32(stack[stack[0]]);
  stack[0]--;
  return ret;
}

