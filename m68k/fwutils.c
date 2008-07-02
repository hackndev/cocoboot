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

void appendBuf(char *buf, UInt32 sz) {
    UInt16 volRefNum;
    UInt32 volIterator=vfsIteratorStart;
    FileRef fRef;
    UInt32 rBytes;
    UInt16  err;

    while ((volIterator != vfsIteratorStop)) {
	if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone) {

	    if (VFSFileOpen(volRefNum,"/libertas_cs_onestage.fw", vfsModeWrite,&fRef) == errNone) {
		VFSFileSeek(fRef,vfsOriginBeginning,0);
                                                                                                                                                        
		err = VFSFileSeek(fRef, vfsOriginEnd, 0);
		err = VFSFileWrite (fRef, sz, (const void *)buf, &rBytes);
		VFSFileClose(fRef);

	    }
	}
    }
}


UInt32 extract_mrv_wifi_fw() {
	/* DB reading stuff */
	DmOpenRef dbRef;
	UInt16 records,i;
	MemHandle recordHandle;
	char buf[256];
	char sz[16];
	UInt32 fwSize, chunkSize;
	/* VFS writing stuff */
	UInt16 volRefNum;
	UInt32 volIterator=vfsIteratorStart;
	
	/* We first open the database */
	dbRef = DmOpenDatabaseByTypeCreator('DATA','wifi', dmModeReadOnly);
	if (!dbRef) {
	    FrmCustomAlert(InfoAlert, "Asi nejakej problem ...","","");
	    return 0;
	}

	/* Then create output file */
	while ((volIterator != vfsIteratorStop))
	    if (VFSVolumeEnumerate(&volRefNum, &volIterator) == errNone)
		VFSFileCreate(volRefNum,"/libertas_cs_onestage.fw");

	records = DmNumRecords(dbRef);

	recordHandle = DmQueryRecord(dbRef, 1);
	memcpy(sz,MemHandleLock(recordHandle),MemHandleSize(recordHandle));
	fwSize=atol(sz);
	chunkSize=getChunkSZ(atol(sz),records-2);
	sprintf(buf, "FW: %li b\nChunk: %li b\n\nPress OK and wait.",fwSize,chunkSize);
	FrmCustomAlert(InfoAlert, buf,"","");
	MemHandleUnlock(recordHandle);

	/* first two records arent interesting */
	for (i=2;i<records;i++) {
	    recordHandle = DmQueryRecord(dbRef, i);
	    appendBuf(MemHandleLock(recordHandle),(fwSize>=chunkSize)?chunkSize:fwSize);
	    fwSize-=chunkSize;
	    MemHandleUnlock(recordHandle);
	}

	FrmCustomAlert(InfoAlert, "Dump complete :-)","See CARD://libertas_cs_onestage.fw","");

	DmCloseDatabase(dbRef);
	return records;

}

UInt32 extract_asus_wifi_fw() {
    return 0;
}
