#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomtrak.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomTRAK_Init(QMP4_ATOM_TRAK *atombox, 
                      QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    atombox->tkhd->Init(atombox->tkhd, trakInfo);
    atombox->mdia->Init(atombox->mdia, trakInfo);

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomTRAK_Fini(QMP4_ATOM_TRAK *atombox)
{
    int err = 0;

    atombox->tkhd->Fini(atombox->tkhd);
    atombox->mdia->Fini(atombox->mdia);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomTRAK_WriteBoxToFile(QMP4_ATOM_TRAK *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_TRAK, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = atombox->tkhd->WriteBoxToFile(atombox->tkhd, file);
        err = err ? err : atombox->mdia->WriteBoxToFile(atombox->mdia, file);
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomTRAK_UpdateBoxSize(QMP4_ATOM_TRAK *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_TRAK, headerAlign);

    size += atombox->tkhd->UpdateBoxSize(atombox->tkhd);
    size += atombox->mdia->UpdateBoxSize(atombox->mdia);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, ret_size=%d", __FUNCTION__, size);

    return size;        
}

int QMP4AtomTRAK_GetBoxAllocSize(QMP4_ATOM_TRAK *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_TRAK, headerAlign);

    size += atombox->tkhd->GetBoxAllocSize(atombox->tkhd);
    size += atombox->mdia->GetBoxAllocSize(atombox->mdia);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

#ifdef __cplusplus
}
#endif

