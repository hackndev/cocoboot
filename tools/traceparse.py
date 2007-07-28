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

	out = subprocess.Popen([OBJDUMP_CMD, "-D", "-bbinary", "-marm", tmpf.name], stdout=subprocess.PIPE).communicate()[0]
	print out


f = file(sys.argv[1], 'rb')
while 1:
	data = f.read(16*4)
	if not data: break

	words = struct.unpack('<16I', data)
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
