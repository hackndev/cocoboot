/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include <Form.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cocoboot_r.h"
#include <VFSMgr.h>
#include <ctype.h>

UInt32 getChunkSZ(UInt32 fwLen, UInt32 chunks) {
    int i=0;
    UInt32 p;
    
    p=fwLen/chunks;

    for (i=0;i<16;i++)
	if(((1<<i)<=p) && ((1<<(i+1))>=p))
	    return (1<<(i+1));
    return 0;
}

void appendBuf(char *buf, UInt32 sz, char *fileName) {
    UInt16 volRefNum;
    UInt32 volIterator=vfsIteratorStart;
    FileRef fRef;
    UInt32 rBytes;
    UInt16  err;

    while ((volIterator != vfsIteratorStop)) {
	if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone) {

	    if (VFSFileOpen(volRefNum, fileName, vfsModeWrite,&fRef) == errNone) {
		VFSFileSeek(fRef,vfsOriginBeginning,0);
                                                                                                                                                        
		err = VFSFileSeek(fRef, vfsOriginEnd, 0);
		err = VFSFileWrite (fRef, sz, (const void *)buf, &rBytes);
		VFSFileClose(fRef);

	    }
	}
    }
}

Int32 extract_mrv_wifi_helper() {
	/* DB reading stuff */
	DmOpenRef dbRef;
	MemHandle dataHandle;
	char *sptr, *eptr, *optr;
	int size = 0;
	/* VFS writing stuff */
	UInt16 volRefNum;
	UInt32 volIterator=vfsIteratorStart;
	
	/* We first open the database */
	dbRef = DmOpenDatabaseByTypeCreator('libr','Wicr', dmModeReadOnly);
	if (!dbRef) {
	    FrmCustomAlert(InfoAlert, "Error dumping helper firmware ...","","");
	    return 0;
	}

	dataHandle = DmGetResource('amdd', 0);
	sptr = MemHandleLock(dataHandle);
	eptr = sptr + MemHandleSize(dataHandle) - 2;

	while (!((sptr[0] & 0xff) == 0x03 && (sptr[1] & 0xff) == 0x00 &&
		(sptr[2] & 0xff) == 0x00 && (sptr[3] & 0xff) == 0xea))
		sptr++;

	optr = sptr;
	while (!((optr[0] & 0xff) == 0xff && (optr[129] & 0xff) == 0xff))
		optr++;

	while (!((eptr[0] & 0xff) == 0x01 && (eptr[1] & 0xff) == 0xc0))
		eptr--;
	eptr += 2;

	if (eptr <= sptr)
		return -1;

	/* Then create output file */
	while ((volIterator != vfsIteratorStop))
	    if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone)
		VFSFileCreate(volRefNum,"/libertas_cs_helper.fw");

	while (optr < eptr) {
		appendBuf(sptr, optr - sptr, "libertas_cs_helper.fw");
		size += optr - sptr;
		sptr = optr + 1;
		optr += 129;
	}
	appendBuf(sptr, eptr - sptr, "libertas_cs_helper.fw");
	size += eptr - sptr;

	MemHandleUnlock(dataHandle);
	DmCloseDatabase(dbRef);
	return size;

}

UInt32 extract_mrv_wifi_firmware() {
	/* DB reading stuff */
	DmOpenRef dbRef;
	UInt16 records,i;
	MemHandle recordHandle;
	char sz[16];
	UInt32 fwSize, chunkSize, size;
	/* VFS writing stuff */
	UInt16 volRefNum;
	UInt32 volIterator=vfsIteratorStart;
	
	/* We first open the database */
	dbRef = DmOpenDatabaseByTypeCreator('DATA','wifi', dmModeReadOnly);
	if (!dbRef) {
	    FrmCustomAlert(InfoAlert, "Error dumping firmware ...","","");
	    return 0;
	}

	/* Then create output file */
	while ((volIterator != vfsIteratorStop))
	    if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone)
		VFSFileCreate(volRefNum,"/libertas_cs.fw");

	records = DmNumRecords(dbRef);

	recordHandle = DmQueryRecord(dbRef, 1);
	memcpy(sz,MemHandleLock(recordHandle),MemHandleSize(recordHandle));
	size=fwSize=atol(sz);
	chunkSize=getChunkSZ(atol(sz),records-2);
	MemHandleUnlock(recordHandle);

	/* first two records arent interesting */
	for (i=2;i<records;i++) {
	    recordHandle = DmQueryRecord(dbRef, i);
	    appendBuf(MemHandleLock(recordHandle),(fwSize>=chunkSize)?chunkSize:fwSize,"/libertas_cs.fw");
	    fwSize-=chunkSize;
	    MemHandleUnlock(recordHandle);
	}

	DmCloseDatabase(dbRef);
	return size;

}

UInt32 extract_mrv_wifi_fw() {
	Int32 hsize, fwsize;
	char buf[1024];
	hsize = extract_mrv_wifi_helper();
	fwsize = extract_mrv_wifi_firmware();

	sprintf(buf,	"Dump complete.\n\n"
			"The following files were generated:\n");
	if (hsize > 0)
		sprintf(buf + strlen(buf), "* CARD://libertas_cs_helper.fw [%li b]\n", hsize);

	sprintf(buf + strlen(buf), "* CARD://libertas_cs.fw [%li b]\n", fwsize);

	FrmCustomAlert(InfoAlert, buf,"","");
	return 0;
}
