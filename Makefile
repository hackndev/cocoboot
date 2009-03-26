EXECS = m68k/cocoboot.m68k arm/cocoboot.arm
VERSION="0.6.1"

# For Treo650 support, until I make it run-time conditional,
# uncomment this:
#DEFINES=-DTREO650
#DEFINES=-DMOVE_FRAMEBUFFER

default:
	${MAKE} clean
	${MAKE} cocoboot.prc

all: cocoboot.prc elf-kernel

install: cocoboot.prc
	pilot-xfer -p /dev/ttyUSB1 -i cocoboot.prc

cocoboot.prc: date-prep arm-objs m68k-objs gui iTbl.bin
	build-prc -n Cocoboot -c CcBt $(EXECS) *.bin

date-prep:
	$(shell sed "s/{\$$BUILD}/Build:\ `date +"%T %d.%m.%Y"\
	`\\\n\\\n/;s/{\$$VERSION}/${VERSION}/g;s/{\$$CYEAR}/`date +"%Y"\
	`/g" include/cocoboot.rcp > include/cocoboot.rcp.tmp)

arm-objs:
	make -C arm DEFINES=${DEFINES}

m68k-objs:
	make -C m68k DEFINES=${DEFINES}

elf-kernel:
	make -C elfkernel

iTbl.bin: #images/*
	tools/chunkimages.py

gui: include/cocoboot_r.h
	pilrc -q -I include include/cocoboot.rcp.tmp

clean:
	rm -f *.prc *.map *~ *.bin include/*.tmp
	make -C arm clean
	make -C m68k clean

elf-clean:
	make -C elfkernel clean
