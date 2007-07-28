#!/usr/bin/env python
import struct
import sys
import string

if len(sys.argv) < 2:
	print 'Usage: %s cocoboot.trc' % sys.argv[0]
	print 'Decodes a raw cocoboot trace dump file to serial'
	sys.exit(-1)

write_pc = 0x730b5256
read_pc = 0x730b4bb6

olddir = ''
f = file(sys.argv[1], 'rb')
while 1:
	data = f.read(16*4)
	if not data: break

	words = struct.unpack('<16I', data)
	pc = words[14]
	
	if pc == read_pc:
		dir = 'Read '
		val = words[2]
	elif pc == write_pc:
		dir = 'Write'
		val = words[1]
	else:
		dir = 'Unkn'
		val = words[1]

	if dir != olddir:
		sys.stdout.write('\n' + dir + ': ')
		olddir = dir

	s = struct.pack('<I', val)
	out = []
	for c in s:
		if c == '\r': continue
		if c not in string.printable:
			out.append('.')
		else:
			out.append(c)
	
	sys.stdout.write(''.join(out))
	
	"""
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
	"""
	#print '-' * 20
	#print
