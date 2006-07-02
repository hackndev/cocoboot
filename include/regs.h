#ifndef _REGS_H_
#define _REGS_H_

/* XScale LCD control registers */
#define LCCR0 0x44000000
#define LCCR1 0x44000004
#define LCCR2 0x44000008
#define LCCR3 0x4400000c
#define LCCR4 0x44000010
#define LCCR5 0x44000014

#define FDADR0 0x44000200

/* DMA descriptor offsets */
#define DMA_DESC 0x0 /* address of next descriptor */
#define DMA_SRC  0x4
#define DMA_TARGET 0x8
#define DMA_CMD 0xc


#endif
