#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatommvhd.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomMVHD_Init(QMP4_ATOM_MVHD *atombox,
                      QMP4_TRAK_INFO *vidTrakInfo,
                      QMP4_TRAK_INFO *audTrakInfo)
{
    int err = 0;

    unsigned char rate[4] = {0x00, 0x01, 0x00, 0x00};
    unsigned char volume[2] = {0x01, 0x00};
    unsigned char matrix[4*9] = { 
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x40, 0x00, 0x00, 0x00
    };
#if 0
    static const unsigned long secsThrough1904To1970 = 2082844800;
    time_t currTime;

    currTime = time(NULL);
    currTime += secsThrough1904To1970;
                
    *(unsigned int *)atombox->creationTime = QMP4_SWAP32(currTime);
    *(unsigned int *)atombox->modificationTime = QMP4_SWAP32(currTime);
#endif
    *(unsigned int *)atombox->creationTime = QMP4_SWAP32(vidTrakInfo->info.general.creation_sec_time_since_1904);
    *(unsigned int *)atombox->modificationTime = QMP4_SWAP32(vidTrakInfo->info.general.creation_sec_time_since_1904);    
    *(unsigned int *)atombox->timescale = QMP4_SWAP32(vidTrakInfo->info.general.sample_time_scale); 
    memcpy(atombox->rate, rate, sizeof(rate));
    memcpy(atombox->volume, volume, sizeof(volume));
    memcpy(atombox->matrix, matrix, sizeof(matrix));
    *(unsigned int *)atombox->nextTrackID = QMP4_SWAP32((audTrakInfo ? 3 : 2)); 
                                                
    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomMVHD_Fini(QMP4_ATOM_MVHD *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomMVHD_WriteBoxToFile(QMP4_ATOM_MVHD *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_MVHD, headerAlign);
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

int QMP4AtomMVHD_UpdateDuration(QMP4_ATOM_MVHD *atombox, unsigned int delta)
{
    int err = 0;

    *(unsigned int *)atombox->duration = QMP4_SWAP32(QMP4_SWAP32(*(unsigned int *)atombox->duration)+delta);
    
    qmp4_log_debug("%s() leave, duration=%d err=%d", __FUNCTION__, QMP4_SWAP32(*(unsigned int *)atombox->duration), err);    

    return err;
}

int QMP4AtomMVHD_UpdateBoxSize(QMP4_ATOM_MVHD *atombox)
{
    int size = 0;

    size = QMP4AtomMVHD_GetBoxAllocSize(atombox);

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);
   
    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size;    
}

int QMP4AtomMVHD_GetBoxAllocSize(QMP4_ATOM_MVHD *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MVHD, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size;    
}

#ifdef __cplusplus
}
#endif

