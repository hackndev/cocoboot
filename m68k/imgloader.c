#include <PalmOS.h>
#include <VFSMgr.h>
#include "cocoboot.h"
/* FIXME: This whole file is just plain ugly due to my limited PalmOS API
 * experience. But meh.. as long as it works. 
*/

typedef struct {
	Int32 start; /* (in chunks) */
	Int32 end;
	UInt32 size;
	char name[32];
} ImageTable;

/**
 * Search for a file on all volumes. Return the size and volume of the found
 * file.
 */
int search_file(char *name, UInt16 *vol_ref, UInt32 *size)
{
	UInt32 vol_iter = vfsIteratorStart;
	Err err;
	FileRef f;

	if(!name || !vol_ref) return -1;

	while (vol_iter != vfsIteratorStop) {
		if (VFSVolumeEnumerate (vol_ref, &vol_iter) != errNone)
			continue;

		err = VFSFileOpen(*vol_ref, name, vfsModeRead, &f);
		if (err != errNone)
			continue;

		if(size) VFSFileSize(f, size);
		VFSFileClose(f);
		return 1;

	}
	return 0;
}

/**
 * Load the given file into data memory. Returns the number of bytes read.
 */
Int32 load_file(char *name, UInt16 vol_ref, UInt32 size, void *buffer,
		 UInt32 offset)
{
        Err err;
        FileRef f;

	UInt32 bytes_read;

	if(!name || !buffer) return -1;

	err = VFSFileOpen(vol_ref, name, vfsModeRead, &f);
	if(err != errNone) return -2;

	VFSFileReadData(f, size, buffer, offset, &bytes_read);
	VFSFileClose(f);

	return (Int32)bytes_read;
}

/**
 * Search for a compiled in image and fallback to the filesystem.
 */
Int32 search_image(char *name, char *loc, UInt16 loc_len, UInt32 *size)
{
	UInt16 vol_ref;
	MemHandle tblh;
	ImageTable *tbl;
	int i;
	Int32 ret = -3;

	if(!loc || !name || !size) return -2;

	tblh = DmGetResource('iTbl', 0);
	if(!tblh) goto out;

	tbl = MemHandleLock(tblh);
	if(!tbl) goto out_release;

	i = 0;
	while(tbl[i].start != -1) {
		if(!StrCompare(tbl[i].name, name)) {
			
			StrCopy(loc, "Builtin");
			*size = tbl[i].size;
			ret = -1;
			goto out_unlock;
		}
		i++;
		//if(i==3) break;
	}
	/* image not found compiled-in, check filesystem */
	if(search_file(name, &vol_ref, size)) {
		VFSVolumeGetLabel(vol_ref, loc, loc_len-1);
		ret = vol_ref;
	}
	

 out_unlock:
	MemHandleUnlock(tblh);
 out_release:
	DmReleaseResource(tblh);
 out:
	return ret;
}

Int32 load_image(char *name, UInt32 size, UInt16 vol_ref, void *buffer)
{
	MemHandle tblh, pageh;
	ImageTable *tbl;
	void *page;
	UInt32 bytes, offset=0;
	Int32 ret = -1;
	int i;
	Int32 p;

	if(vol_ref == -1) { /* builtin */
		tblh = DmGetResource('iTbl', 0);
		if(!tblh) goto out;

		tbl = MemHandleLock(tblh);
		if(!tbl) goto out_release;

		for(i=0; tbl[i].start != -1; i++) {
			if(StrCompare(tbl[i].name, name)) continue;
			
			ret = 0;
			for(p = tbl[i].start; p < tbl[i].end; p++) {
				pageh = DmGetResource('imgP', p);
				page = MemHandleLock(pageh);
				
				if(page) {
					bytes = size;
					if(bytes > 32768)
						bytes = 32768;
					size -= bytes;

					DmWrite(buffer, offset, page, bytes);

					ret += bytes;
					MemHandleUnlock(pageh);
				}
				DmReleaseResource(pageh);
			}
			goto out_unlock;

		}
		
	} else { /* vfs */
		return load_file(name, vol_ref, size, buffer, 0);
	}

 out_unlock:
	MemHandleUnlock(tblh);
 out_release:
	DmReleaseResource(tblh);
 out:
	return ret;
}

/*
void list_images(char *name)
{
	char vol_name[32];
	UInt16 vol;

	VFSVolumeGetLabel (*vol_ref, vol, sizeof(vol) - 1);
	lprintf("[%s] %s: %ld bytes\n", vol, name, size);
}
*/

