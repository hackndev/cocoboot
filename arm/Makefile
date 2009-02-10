CC = arm-palmos-gcc
CFLAGS = -Wall -O -s -I../include ${DEFINES}
LDFLAGS = -nostartfiles
OBJS = arm.o atag.o boot.o cpu.o debug.o elf.o

all: cocoboot.arm

cocoboot.arm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cocoboot.arm $(OBJS)

clean:
	rm -f $(OBJS) cocoboot.arm
