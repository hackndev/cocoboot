CC = m68k-palmos-gcc
CFLAGS = -Wall -O -s -I../include ${DEFINES}
LDFLAGS = -static 
LIBS=-L/software/apps/prc-tools/sdks/sdk-5r3/lib/m68k-palmos-coff/ -lPalmOSGlue 
OBJS = cocoboot.o mainform.o mem.o cpu.o imgloader.o mach.o options.o


cocoboot.m68k: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o cocoboot.m68k ${LIBS}

clean:
	rm -f $(OBJS) cocoboot.m68k
