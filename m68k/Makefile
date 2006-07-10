CC = m68k-palmos-gcc
CFLAGS = -Wall -O -s -I../include #-I/opt/palmdev/sdk-5/include/Core/System/ -I/usr/share/prc-tools/include
LDFLAGS = -static -L/usr/m68k-palmos/lib -lPalmOSGlue -L/usr/local/share/palmdev/sdk-5r3/lib/m68k-palmos-coff/
OBJS = cocoboot.o mainform.o mem.o cpu.o imgloader.o mach.o


cocoboot.m68k: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o cocoboot.m68k

clean:
	rm -f $(OBJS) cocoboot.m68k
