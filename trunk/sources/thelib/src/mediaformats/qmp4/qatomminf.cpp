#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomminf.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomMINF_Init(QMP4_ATOM_MINF *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        err = atombox->vmhd->Init(atombox->vmhd, trakInfo);
    } else {
        err = atombox->smhd->Init(atombox->smhd, trakInfo);
    }
    err = err ? err : atombox->hdlr->Init(atombox->hdlr, trakInfo, QMP4_HDLR_TYPE_DATA_HANDLER);
    err = err ? err : atombox->dinf->Init(atombox->dinf, trakInfo);
    err = err ? err : atombox->stbl->Init(atombox->stbl, trakInfo);    

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomMINF_Fini(QMP4_ATOM_MINF *atombox)
{
    int err = 0;

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        err = atombox->vmhd->Fini(atombox->vmhd);
    } else {
        err = atombox->smhd->Fini(atombox->smhd);
    }
    err = err ? err : atombox->hdlr->Fini(atombox->hdlr);
    err = err ? err : atombox->dinf->Fini(atombox->dinf);
    err = err ? err : atombox->stbl->Fini(atombox->stbl);    

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomMINF_WriteBoxToFile(QMP4_ATOM_MINF *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_MINF, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
            err = atombox->vmhd->WriteBoxToFile(atombox->vmhd, file);
        } else {
            err = atombox->smhd->WriteBoxToFile(atombox->smhd, file);
        }
        err = err ? err : atombox->hdlr->WriteBoxToFile(atombox->hdlr, file);
        err = err ? err : atombox->dinf->WriteBoxToFile(atombox->dinf, file);
        err = err ? err : atombox->stbl->WriteBoxToFile(atombox->stbl, file);    
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomMINF_UpdateBoxSize(QMP4_ATOM_MINF *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MINF, headerAlign);

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->vmhd->UpdateBoxSize(atombox->vmhd);
    } else {
        size += atombox->smhd->UpdateBoxSize(atombox->smhd);
    }
    size += atombox->hdlr->UpdateBoxSize(atombox->hdlr);
    size += atombox->dinf->UpdateBoxSize(atombox->dinf);
    size += atombox->stbl->UpdateBoxSize(atombox->stbl);    

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomMINF_GetBoxAllocSize(QMP4_ATOM_MINF *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MINF, headerAlign);

    if (QMP4_TRAK_TYPE_VIDEO_AVC == atombox->trakType) {
        size += atombox->vmhd->GetBoxAllocSize(atombox->vmhd);
    } else {
        size += atombox->smhd->GetBoxAllocSize(atombox->smhd);
    }
    size += atombox->hdlr->GetBoxAllocSize(atombox->hdlr);
    size += atombox->dinf->GetBoxAllocSize(atombox->dinf);
    size += atombox->stbl->GetBoxAllocSize(atombox->stbl);    

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

#ifdef __cplusplus
}
#endif

