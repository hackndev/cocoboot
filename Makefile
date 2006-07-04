EXECS = m68k/cocoboot.m68k arm/cocoboot.arm

all: cocoboot.prc

install: cocoboot.prc
	pilot-xfer -p /dev/tts/USB0 -i cocoboot.prc

cocoboot.prc: arm-objs m68k-objs gui iTbl.bin
	build-prc -n Cocoboot -c CcBt $(EXECS) *.bin

arm-objs:
	make -C arm

m68k-objs:
	make -C m68k

iTbl.bin: #images/*
	tools/chunkimages.py

gui: include/cocoboot_r.h
	pilrc -q -I include include/cocoboot.rcp 
clean:
	rm -f *.prc *.map *~ *.bin 
	make -C arm clean
	make -C m68k clean
