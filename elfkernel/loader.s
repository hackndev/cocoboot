.global loader
loader:
	mov r0, #0x5c
	lsl r0, #0x08
	orr r0, #0x04
	lsl r0, #0x10	@; r0 = 0x5c040000 [SRAM bank 3]

	mov sp, r0	@; set Stack pointer into SRAM

	b kmain		@; actually start the kernel
