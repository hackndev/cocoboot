
void setup_atags()
{
	unsigned long i;
	*(unsigned long*)0x41300004 |= 0x3;
	while (1) {
		*(unsigned long*)0x41300004 ^= 0x3;
		for(i=0 ; i<40000000 ; i++);
	}

}
