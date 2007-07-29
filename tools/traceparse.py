#!/usr/bin/env python
import struct
import sys
import subprocess
import tempfile

OBJDUMP_CMD = "arm-softfloat-linux-gnueabi-objdump"

if len(sys.argv) < 2:
	print 'Usage: %s cocoboot.trc' % sys.argv[0]
	print 'Decodes a raw cocoboot trace dump file'
	sys.exit(-1)

def decode_instr(instr):
	tmpf = tempfile.NamedTemporaryFile()
	tmpf.write(struct.pack('<I', instr))
	tmpf.flush()

	out = subprocess.Popen([OBJDUMP_CMD, "-D", "-bbinary", "-marm", "-Mreg-names-raw", tmpf.name], stdout=subprocess.PIPE).communicate()[0]
	out = out.split('\n')[6]
	print out


f = file(sys.argv[1], 'rb')
while 1:
	data = f.read(16*4)
	if not data: break

	words = struct.unpack('<16I', data)
	if (words[15] & 0xffffff00) == 0xe580c000:
		print '0x%08x = 0x%08x' % (words[1]+(words[15] & 0xff), words[13])
	elif (words[15] & 0xffffff00) == 0xe5810000:
		print '0x%08x = 0x%08x' % (words[2]+(words[15] & 0xff), words[1])
	elif (words[15] & 0xffffff00) == 0xe5812000:
		print '0x%08x = 0x%08x' % (words[2]+(words[15] & 0xff), words[3])
	elif (words[15] & 0xffffff00) == 0xe5801000:
		print '0x%08x = 0x%08x' % (words[1]+(words[15] & 0xff), words[2])
#	elif (words[15] & 0xffffff00) == 0xe583e000:
#		print '%08x = %08x' % (words[2]+(words[15] & 0xff), words[4])
	elif (words[15] & 0xffffff00) == 0xe5813000:
		print '0x%08x = 0x%08x' % (words[2]+(words[15] & 0xff), words[4])
	elif (words[15] & 0xffffff00) == 0xe5821000:
		print '0x%08x = 0x%08x' % (words[3]+(words[15] & 0xff), words[2])
	elif (words[15] & 0xffffff00) == 0xe5900000:
		print 'ldr     r0, [r0, #%d] => 0x%08x' % ((words[15] & 0xff), words[1])
	elif (words[15] & 0xffffff00) == 0xe5911000:
		print 'ldr     r1, [r1, #%d] => 0x%08x' % ((words[15] & 0xff), words[2])
	elif (words[15] & 0xffffff00) == 0xe59ee000:
		print 'ldr     r14, [r14, #%d] => ???' % ((words[15] & 0xff))
	elif (words[15] & 0xffffff00) == 0xe591e000:
		print '0x%08x => r14' % (words[2]+(words[15] & 0xff))
	else:
		for i, w in zip(range(len(words)), words):
			s = str(struct.pack('<I', w))
			if i == 0:
				reg = '   '
			elif i == 14:
				reg = 'PC '
			elif i == 15:
				reg = '   '
				decode_instr(w)
			else:
				reg = 'r%-2d' % (i-1)
			print '%s = %08x %-10d %s' % (reg, w,w, repr(s))
		print '-' * 20
		print
