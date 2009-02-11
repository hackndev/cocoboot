PREFIX=arm-angstrom-linux-gnueabi-
CFLAGS=-Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs -fno-builtin -I. -mcpu=arm7tdmi

all: kernel.bin

# test dummy kernel in ELF format

kernel.o: kernel.c
	$(PREFIX)gcc -o kernel.o -c kernel.c $(CFLAGS)

kernel.bin: kernel.o loader.s
	$(PREFIX)as -o loader.o loader.s
	$(PREFIX)ld -T linker.ld -o kernel.bin loader.o kernel.o

clean:
	rm -rf *.o kernel.bin
