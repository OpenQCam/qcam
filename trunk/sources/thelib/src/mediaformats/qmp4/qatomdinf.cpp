#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomdinf.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomDINF_Init(QMP4_ATOM_DINF *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    err = atombox->dref->Init(atombox->dref, trakInfo);

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomDINF_Fini(QMP4_ATOM_DINF *atombox)
{
    int err = 0;

    err = atombox->dref->Fini(atombox->dref);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomDINF_WriteBoxToFile(QMP4_ATOM_DINF *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_DINF, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = atombox->dref->WriteBoxToFile(atombox->dref, file);
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomDINF_UpdateBoxSize(QMP4_ATOM_DINF *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_DINF, headerAlign);

    size += atombox->dref->UpdateBoxSize(atombox->dref);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);    

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

int QMP4AtomDINF_GetBoxAllocSize(QMP4_ATOM_DINF *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_DINF, headerAlign);

    size += atombox->dref->GetBoxAllocSize(atombox->dref);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;        
}

#ifdef __cplusplus
}
#endif

