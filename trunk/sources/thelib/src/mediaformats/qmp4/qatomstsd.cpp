#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomstsd.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomSTSD_Init(QMP4_ATOM_STSD *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    *(unsigned int *)atombox->entryCount = QMP4_SWAP32(1);

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        atombox->avc1->Init(atombox->avc1, trakInfo);
    } else {
        atombox->sowt->Init(atombox->sowt, trakInfo);
    }

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3]);


    return err;
}

int QMP4AtomSTSD_Fini(QMP4_ATOM_STSD *atombox)
{
    int err = 0;

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        atombox->avc1->Fini(atombox->avc1);
    } else {
        atombox->sowt->Fini(atombox->sowt);
    }
    
    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomSTSD_WriteBoxToFile(QMP4_ATOM_STSD *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_STSD, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
            err = atombox->avc1->WriteBoxToFile(atombox->avc1, file);
        } else {
            err = atombox->sowt->WriteBoxToFile(atombox->sowt, file);
        }
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomSTSD_UpdateBoxSize(QMP4_ATOM_STSD *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSD, headerAlign);

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->avc1->UpdateBoxSize(atombox->avc1);
    } else {
        size += atombox->sowt->UpdateBoxSize(atombox->sowt);
    }

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomSTSD_GetBoxAllocSize(QMP4_ATOM_STSD *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STSD, headerAlign);

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->avc1->GetBoxAllocSize(atombox->avc1);
    } else {
        size += atombox->sowt->GetBoxAllocSize(atombox->sowt);
    }

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

#ifdef __cplusplus
}
#endif

