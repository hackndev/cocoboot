#define EndianSwap32(n) (((((unsigned long) n) << 24) & 0xFF000000) | \
                         ((((unsigned long) n) <<  8) & 0x00FF0000) | \
                         ((((unsigned long) n) >>  8) & 0x0000FF00) | \
                         ((((unsigned long) n) >> 24) & 0x000000FF))

#define EndianSwap16(n) (((((unsigned int) n) << 8) & 0xFF00) | \
                         ((((unsigned int) n) >> 8) & 0x00FF))

/* Endian fixing when reading registers etc. */
#ifdef ARM_MODE
#define EndianFix32(x) x
#define EndianFix16(x) x
#else
#define EndianFix32(x) EndianSwap32(x)
#define EndianFix16(x) EndianSwap16(x)
#endif

/* ARM function calls */
#define ARM_read_cp 1 /* (coproc, reg) */
#define ARM_fb_test 2 /* () */

typedef UInt32 ArmStack;

typedef struct {
	UInt32 pttb;
	UInt32 vttb;

	UInt32 cpu;

	UInt32 ram_base;
	UInt32 ram_size;
} ArmGlobals;

void push_uint32(ArmStack *stack, UInt32 n);
UInt32 pop_uint32(ArmStack *stack);


