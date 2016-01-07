#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomstbl.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomSTBL_Init(QMP4_ATOM_STBL *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    err = atombox->stsd->Init(atombox->stsd, trakInfo);
    err = err ? err : atombox->stts->Init(atombox->stts, trakInfo);
    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        err = err ? err : atombox->stss->Init(atombox->stss, trakInfo);
    }
    err = err ? err : atombox->stsc->Init(atombox->stsc, trakInfo);
    err = err ? err : atombox->stsz->Init(atombox->stsz, trakInfo);
    err = err ? err : atombox->stco->Init(atombox->stco, trakInfo);

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomSTBL_Fini(QMP4_ATOM_STBL *atombox)
{
    int err = 0;

    err = atombox->stsd->Fini(atombox->stsd);
    err = err ? err : atombox->stts->Fini(atombox->stts);
    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        err = err ? err : atombox->stss->Fini(atombox->stss);
    }
    err = err ? err : atombox->stsc->Fini(atombox->stsc);
    err = err ? err : atombox->stsz->Fini(atombox->stsz);
    err = err ? err : atombox->stco->Fini(atombox->stco);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomSTBL_WriteBoxToFile(QMP4_ATOM_STBL *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_STBL, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = atombox->stsd->WriteBoxToFile(atombox->stsd, file);
        err = err ? err : atombox->stts->WriteBoxToFile(atombox->stts, file);
        if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
            err = err ? err : atombox->stss->WriteBoxToFile(atombox->stss, file);
        }
        err = err ? err : atombox->stsc->WriteBoxToFile(atombox->stsc, file);
        err = err ? err : atombox->stsz->WriteBoxToFile(atombox->stsz, file);
        err = err ? err : atombox->stco->WriteBoxToFile(atombox->stco, file);
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomSTBL_UpdateBoxSize(QMP4_ATOM_STBL *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STBL, headerAlign);

    size += atombox->stsd->UpdateBoxSize(atombox->stsd);
    size += atombox->stts->UpdateBoxSize(atombox->stts);
    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->stss->UpdateBoxSize(atombox->stss);
    }
    size += atombox->stsc->UpdateBoxSize(atombox->stsc);
    size += atombox->stsz->UpdateBoxSize(atombox->stsz);
    size += atombox->stco->UpdateBoxSize(atombox->stco);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomSTBL_GetBoxAllocSize(QMP4_ATOM_STBL *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_STBL, headerAlign);

    size += atombox->stsd->GetBoxAllocSize(atombox->stsd);
    size += atombox->stts->GetBoxAllocSize(atombox->stts);
    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->stss->GetBoxAllocSize(atombox->stss);
    }
    size += atombox->stsc->GetBoxAllocSize(atombox->stsc);
    size += atombox->stsz->GetBoxAllocSize(atombox->stsz);
    size += atombox->stco->GetBoxAllocSize(atombox->stco);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;        
}

#ifdef __cplusplus
}
#endif

