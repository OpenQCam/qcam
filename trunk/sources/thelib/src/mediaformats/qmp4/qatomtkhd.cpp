#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomtkhd.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomTKHD_Init(QMP4_ATOM_TKHD *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;
    unsigned char matrix[4*9] = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00
    };

    atombox->trakType = trakInfo->type;

    if (QMP4_TRAK_TYPE_AUDIO_PCM == atombox->trakType) {
        atombox->volume[0] = (unsigned char)1; 
    } else {
        *(unsigned short *)atombox->width = QMP4_SWAP16(trakInfo->info.video.width); 
        *(unsigned short *)atombox->height = QMP4_SWAP16(trakInfo->info.video.height); 
    }
                                                        
    atombox->fullBox.flags[2] = 0x3;
    *(unsigned int *)atombox->creationTime = QMP4_SWAP32(trakInfo->info.general.creation_sec_time_since_1904);    
    *(unsigned int *)atombox->modificationTime = QMP4_SWAP32(trakInfo->info.general.creation_sec_time_since_1904);    
    *(unsigned int *)atombox->trackID = QMP4_SWAP32(trakInfo->info.general.track_id); 
    memcpy(atombox->matrix, matrix, sizeof(matrix));

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c) err(%d)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3],
                   err);

    return err;
}

int QMP4AtomTKHD_Fini(QMP4_ATOM_TKHD *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomTKHD_WriteBoxToFile(QMP4_ATOM_TKHD *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_TKHD, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomTKHD_UpdateDuration(QMP4_ATOM_TKHD *atombox, unsigned int delta)
{
    int err = 0;

    *(unsigned int *)atombox->duration = QMP4_SWAP32(QMP4_SWAP32(*(unsigned int *)atombox->duration)+delta);
    
    qmp4_log_debug("%s() leave, duration=%d err=%d", __FUNCTION__, QMP4_SWAP32(*(unsigned int *)atombox->duration), err); 

    return err;
}


int QMP4AtomTKHD_UpdateBoxSize(QMP4_ATOM_TKHD *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_TKHD, headerAlign);

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);    

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;        
}

int QMP4AtomTKHD_GetBoxAllocSize(QMP4_ATOM_TKHD *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_TKHD, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;        
}

#ifdef __cplusplus
}
#endif

