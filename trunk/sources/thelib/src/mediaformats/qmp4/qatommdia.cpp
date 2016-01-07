#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatommdia.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomMDIA_Init(QMP4_ATOM_MDIA *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    err = atombox->mdhd->Init(atombox->mdhd, trakInfo);
    err = err ? err : atombox->hdlr->Init(atombox->hdlr, trakInfo, QMP4_HDLR_TYPE_MEDIA_HANDLER);
    err = err ? err : atombox->minf->Init(atombox->minf, trakInfo);

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomMDIA_Fini(QMP4_ATOM_MDIA *atombox)
{
    int err = 0;

    err = atombox->mdhd->Fini(atombox->mdhd);
    err = err ? err : atombox->hdlr->Fini(atombox->hdlr);
    err = err ? err : atombox->minf->Fini(atombox->minf);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomMDIA_WriteBoxToFile(QMP4_ATOM_MDIA *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_MDIA, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = atombox->mdhd->WriteBoxToFile(atombox->mdhd, file);
        err = err ? err : atombox->hdlr->WriteBoxToFile(atombox->hdlr, file);
        err = err ? err : atombox->minf->WriteBoxToFile(atombox->minf, file);
    }
                                                                               
FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomMDIA_UpdateBoxSize(QMP4_ATOM_MDIA *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MDIA, headerAlign);

    size += atombox->mdhd->UpdateBoxSize(atombox->mdhd);
    size += atombox->hdlr->UpdateBoxSize(atombox->hdlr);
    size += atombox->minf->UpdateBoxSize(atombox->minf);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomMDIA_GetBoxAllocSize(QMP4_ATOM_MDIA *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MDIA, headerAlign);

    size += atombox->mdhd->GetBoxAllocSize(atombox->mdhd);
    size += atombox->hdlr->GetBoxAllocSize(atombox->hdlr);
    size += atombox->minf->GetBoxAllocSize(atombox->minf);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

#ifdef __cplusplus
}
#endif

