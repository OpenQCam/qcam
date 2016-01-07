#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomstsz.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomSTSZ_Init(QMP4_ATOM_STSZ *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        *(unsigned int *)atombox->sampleSize = 0;

        /* allocate data buffer */
        atombox->buffSize = sizeof(QMP4_ENTRY_STSZ)*trakInfo->info.general.max_packets;
        //atombox->dataBuff = (unsigned char *)calloc(1, atombox->buffSize);     
        atombox->dataBuff = (unsigned char *)malloc(atombox->buffSize);     

        if (!atombox->dataBuff) {
          err = -ENOMEM;
        }
    } else {
        *(unsigned int *)atombox->sampleSize = QMP4_SWAP32(1);
        atombox->buffSize = 0;
        atombox->dataBuff = NULL;     
    }

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c) err(%d)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3],
                   err);

    return err;
}

int QMP4AtomSTSZ_Fini(QMP4_ATOM_STSZ *atombox)
{
    int err = 0;

    if (atombox->dataBuff) {
        free(atombox->dataBuff);
        atombox->dataBuff = NULL;
    }    

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomSTSZ_WriteBoxToFile(QMP4_ATOM_STSZ *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_STSZ, headerAlign);
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

int QMP4AtomSTSZ_UpdateBoxSize(QMP4_ATOM_STSZ *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSZ, headerAlign);

    size += atombox->dataLength;

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomSTSZ_GetBoxAllocSize(QMP4_ATOM_STSZ *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSZ, headerAlign);

    size += atombox->buffSize;

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

int QMP4AtomSTSZ_AddData(QMP4_ATOM_STSZ *atombox, unsigned int value)
{
    int err = 0;

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        QMP4_ENTRY_STSZ *entrySTSZ;

        entrySTSZ = (QMP4_ENTRY_STSZ *)(atombox->dataBuff + atombox->dataLength);
        atombox->dataLength += sizeof(QMP4_ENTRY_STSZ);

        memset((uint8_t *)entrySTSZ, 0, sizeof(QMP4_ENTRY_STSZ));
    
        *(unsigned int *)atombox->sampleCount = QMP4_SWAP32(QMP4_SWAP32(*(unsigned int *)atombox->sampleCount)+1);

        *(unsigned int *)entrySTSZ->entrySize = QMP4_SWAP32(value);
    } else {
        *(unsigned int *)atombox->sampleCount = QMP4_SWAP32(QMP4_SWAP32(*(unsigned int *)atombox->sampleCount)+value);
    }

    qmp4_log_debug("%s() leave, sampleCount=%d", __FUNCTION__, QMP4_SWAP32(*(unsigned int *)atombox->sampleCount));

    return err; 
}

#ifdef __cplusplus
}
#endif

