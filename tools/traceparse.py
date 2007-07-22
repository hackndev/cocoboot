#!/usr/bin/env python
import struct
import sys
if len(sys.argv) < 2:
	print 'Usage: %s cocoboot.trc' % sys.argv[0]
	print 'Decodes a raw cocoboot trace dump file'
	sys.exit(-1)

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
		else:
			reg = 'r%-2d' % (i-1)
		print '%s = %08x %-10d %s' % (reg, w,w, repr(s))
	print '-' * 20
	print
