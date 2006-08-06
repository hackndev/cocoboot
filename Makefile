EXECS = m68k/cocoboot.m68k arm/cocoboot.arm

# For Treo650 support, until I make it run-time conditional,
# uncomment this:
#DEFINES=-DTREO650
#DEFINES=-DMOVE_FRAMEBUFFER

default:
	${MAKE} clean
	${MAKE} cocoboot.prc

all: cocoboot.prc

install: cocoboot.prc
	pilot-xfer -p /dev/tts/USB0 -i cocoboot.prc

cocoboot.prc: arm-objs m68k-objs gui iTbl.bin
	build-prc -n Cocoboot -c CcBt $(EXECS) *.bin

arm-objs:
	make -C arm DEFINES=${DEFINES}

m68k-objs:
	make -C m68k DEFINES=${DEFINES}

iTbl.bin: #images/*
	tools/chunkimages.py

gui: include/cocoboot_r.h
	pilrc -q -I include include/cocoboot.rcp 
clean:
	rm -f *.prc *.map *~ *.bin 
	make -C arm clean
	make -C m68k clean
