#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatommoov.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomMOOV_Init(QMP4_ATOM_MOOV *atombox, 
                      QMP4_TRAK_INFO *vidTrakInfo,
                      QMP4_TRAK_INFO *audTrakInfo)
{
    int err = 0;

    if (audTrakInfo) {
        atombox->hasAudioTrak = 1;
    }

    err = atombox->mvhd->Init(atombox->mvhd, vidTrakInfo, audTrakInfo);
    err = err ? err : atombox->vidtrak->Init(atombox->vidtrak, vidTrakInfo);
    if (atombox->hasAudioTrak) {
        err = err ? err : atombox->audtrak->Init(atombox->audtrak, audTrakInfo);
    }

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomMOOV_Fini(QMP4_ATOM_MOOV *atombox)
{
    int err = 0;

    err = atombox->mvhd->Fini(atombox->mvhd);
    err = err ? err : atombox->vidtrak->Fini(atombox->vidtrak);
    if (atombox->hasAudioTrak) {
        err = err ? err : atombox->audtrak->Fini(atombox->audtrak);
    }

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomMOOV_WriteBoxToFile(QMP4_ATOM_MOOV *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_MOOV, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }    

        err = atombox->mvhd->WriteBoxToFile(atombox->mvhd, file);
        err = err ? err : atombox->vidtrak->WriteBoxToFile(atombox->vidtrak, file);
        if (atombox->hasAudioTrak) {
            err = err ? err : atombox->audtrak->WriteBoxToFile(atombox->audtrak, file);
        }
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomMOOV_UpdateBoxSize(QMP4_ATOM_MOOV *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MOOV, headerAlign);

    size += atombox->mvhd->UpdateBoxSize(atombox->mvhd);
    size += atombox->vidtrak->UpdateBoxSize(atombox->vidtrak);
    if (atombox->hasAudioTrak) {
        size += atombox->audtrak->UpdateBoxSize(atombox->audtrak);
    }

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);
    
    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size;    
}

int QMP4AtomMOOV_GetBoxAllocSize(QMP4_ATOM_MOOV *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MOOV, headerAlign);

    size += atombox->mvhd->GetBoxAllocSize(atombox->mvhd);
    size += atombox->vidtrak->GetBoxAllocSize(atombox->vidtrak);
    if (atombox->hasAudioTrak) {
        size += atombox->audtrak->GetBoxAllocSize(atombox->audtrak);
    }

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size;
}

#ifdef __cplusplus
}
#endif

