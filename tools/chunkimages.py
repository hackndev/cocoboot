#!/usr/bin/env python
"""
Splits up all the files in the images directory into 32kb chunks suitable
for adding to a prc. Also writes an image table.
"""

import os, struct

DIR = 'images'
CHUNKSIZE = 32*1024
RSCNAME = 'imgP'

def chunk(fn, i):
    f = file(fn, 'rb')
    size = 0
    while 1:
        data = f.read(CHUNKSIZE)
        if not data: break
        file(RSCNAME + ('%04x' % i) + '.bin', 'wb').write(data)
        size += len(data)
        i+=1
    
    return i, size

def main():
    files = os.listdir(DIR)
    table = file('iTbl0000.bin', 'wb')
    i = 0
    num = 0
    for fn in files:
    	if not os.path.isfile(os.path.join(DIR, fn)): continue
        end, size = chunk(os.path.join(DIR, fn), i)
	print i,end, fn
        table.write(struct.pack('>iiI1s31s', i, end, size, '/', fn))
        num += 1
        i = end

    table.write(struct.pack('>iiI1s31s', -1, -1, 0, '/', 'www.hackndev.com'))
        

if __name__ == '__main__': main()

