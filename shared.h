#define EndianSwap32(n) (((((unsigned long) n) << 24) & 0xFF000000) | \
                         ((((unsigned long) n) <<  8) & 0x00FF0000) | \
                         ((((unsigned long) n) >>  8) & 0x0000FF00) | \
                         ((((unsigned long) n) >> 24) & 0x000000FF))

#define EndianSwap16(n) (((((unsigned int) n) << 8) & 0xFF00) | \
                         ((((unsigned int) n) >> 8) & 0x00FF))

/* ARM function calls */
#define ARM_read_cp 1 /* (coproc, reg) */

typedef UInt32 ArmStack;

void push_uint32(ArmStack *stack, UInt32 n);
UInt32 pop_uint32(ArmStack *stack);


