#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomstsc.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomSTSC_Init(QMP4_ATOM_STSC *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;
    QMP4_ENTRY_STSC *entrySTSC;

    atombox->trakType = trakInfo->type;

    /* allocate data buffer */
    atombox->buffSize = sizeof(QMP4_ENTRY_STSC);
    atombox->dataBuff = (unsigned char *)calloc(1, atombox->buffSize);

    if (!atombox->dataBuff) {
      err = -ENOMEM;
      goto FUNC_EXIT;
    }
    
    entrySTSC = (QMP4_ENTRY_STSC *)(atombox->dataBuff + atombox->dataLength);
    atombox->dataLength += sizeof(QMP4_ENTRY_STSC);

    *(unsigned int *)atombox->entryCount = QMP4_SWAP32(1);
    *(unsigned int *)entrySTSC->firstChunk = QMP4_SWAP32(1); 
    *(unsigned int *)entrySTSC->samplesPerChunk = QMP4_SWAP32(trakInfo->info.general.samples_per_chunk);
    *(unsigned int *)entrySTSC->sampleDescriptionIndex = QMP4_SWAP32(1);    

FUNC_EXIT:

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c) err(%d)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3], 
                   err);

    return err;
}

int QMP4AtomSTSC_Fini(QMP4_ATOM_STSC *atombox)
{
    int err = 0;

    if (atombox->dataBuff) {
        free(atombox->dataBuff);
        atombox->dataBuff = NULL;
    }

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomSTSC_WriteBoxToFile(QMP4_ATOM_STSC *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_STSC, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        } 

        if (atombox->dataLength) {
            toWrite = atombox->dataLength;
            if (toWrite != fwrite((void *)atombox->dataBuff, 1, toWrite, file)) {
                err = -EIO;
                qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
                goto FUNC_EXIT;
            } 
        }
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomSTSC_UpdateBoxSize(QMP4_ATOM_STSC *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSC, headerAlign);

    size += atombox->dataLength;

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomSTSC_GetBoxAllocSize(QMP4_ATOM_STSC *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSC, headerAlign);

    size += atombox->buffSize;

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

int QMP4AtomSTSC_AddData(QMP4_ATOM_STSC *atombox, unsigned int value)
{
    int err = 0;

    qmp4_log_debug("%s() leave, entryCount=%d", __FUNCTION__, QMP4_SWAP32(*(unsigned int *)atombox->entryCount));

    return err;
}

#ifdef __cplusplus
}
#endif

