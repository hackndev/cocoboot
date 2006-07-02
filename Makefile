CC-68K = m68k-palmos-gcc
CC-ARM = arm-palmos-gcc
CFLAGS = -Wall -palmos5.0R3 -O  -s #-I/opt/palmdev/sdk-5/include/Core/System/ -I/usr/share/prc-tools/include
LDFLAGS = -static -palmos5.0R3 -L/usr/m68k-palmos/lib -lPalmOSGlue -L/usr/local/share/palmdev/sdk-5r3/lib/m68k-palmos-coff/
EXECS = cocoboot cocoboot-arm
OBJS-68K = cocoboot.o mainform.o mem.o cpu.o imgloader.o
OBJS-ARM = arm.o atag.o

all:
	make cocoboot.prc

install: cocoboot.prc
	pilot-xfer -p /dev/tts/USB0 -i cocoboot.prc

cocoboot.prc: $(EXECS) gui iTbl.bin
	build-prc -n Cocoboot -c ARML $(EXECS) *.bin

cocoboot-arm: $(OBJS-ARM)
	$(CC-ARM) $(CFLAGS) -nostartfiles -o cocoboot-arm $(OBJS-ARM)

arm.o: arm.c cocoboot.h
	$(CC-ARM) $(CFLAGS) -c arm.c
	$(CC-ARM) $(CFLAGS) -S -c arm.c

atag.o: atag.c cocoboot.h
	$(CC-ARM) $(CFLAGS) -c atag.c

cocoboot: $(OBJS-68K)
	$(CC-68K) $(CFLAGS) $(LDFLAGS) $(OBJS-68K) -o cocoboot

cocoboot.o: cocoboot.c cocoboot.h
	$(CC-68K) $(CFLAGS) cocoboot.c -c -o cocoboot.o
mem.o: mem.c cocoboot.h mem.h
	$(CC-68K) $(CFLAGS) mem.c -c -o mem.o

cpu.o: cpu.c cocoboot.h cpu.h
	$(CC-68K) $(CFLAGS) cpu.c -c -o cpu.o

mainform.o: mainform.h cocoboot.h mainform.c
	$(CC-68K) $(CFLAGS) mainform.c -c -o mainform.o

imgloader.o: cocoboot.h imgloader.c
	$(CC-68K) $(CFLAGS) imgloader.c -c -o imgloader.o

iTbl.bin: #images/*
	./chunkimages.py

gui: cocoboot_r.h
	pilrc -q cocoboot.rcp

clean:
	-rm -f *.prc *.o $(EXECS) *.map *~ *.bin 
