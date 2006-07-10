CC = arm-palmos-gcc
CFLAGS = -Wall -palmos5.0R3 -O -s -I../include
LDFLAGS = -nostartfiles
OBJS = arm.o atag.o boot.o

all: cocoboot.arm

cocoboot.arm: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o cocoboot.arm $(OBJS)

clean:
	rm -f $(OBJS) cocoboot.arm
