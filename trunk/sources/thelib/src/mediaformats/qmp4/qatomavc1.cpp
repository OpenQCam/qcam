#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomavc1.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomAVC1_Init(QMP4_ATOM_AVC1 *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    unsigned char reserved2[14] = {
        0x00, 0x48, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };
    unsigned char reserved3[4] = {0x00, 0x18, 0xff, 0xff};

    *(unsigned short *)atombox->dataReferenceIndex = QMP4_SWAP16((unsigned short)1);
    *(unsigned short *)atombox->width = QMP4_SWAP16((unsigned short)trakInfo->info.video.width);
    *(unsigned short *)atombox->height = QMP4_SWAP16((unsigned short)trakInfo->info.video.height);
    memcpy(atombox->reserved2, reserved2, sizeof(reserved2));
    memcpy(atombox->reserved3, reserved3, sizeof(reserved3));

    err = atombox->avcc->Init(atombox->avcc, trakInfo);

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomAVC1_Fini(QMP4_ATOM_AVC1 *atombox)
{
    int err = 0;

    err = atombox->avcc->Fini(atombox->avcc);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomAVC1_WriteBoxToFile(QMP4_ATOM_AVC1 *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_AVC1, headerAlign);
                    
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = atombox->avcc->WriteBoxToFile(atombox->avcc, file);
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomAVC1_UpdateBoxSize(QMP4_ATOM_AVC1 *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_AVC1, headerAlign);

    size += atombox->avcc->UpdateBoxSize(atombox->avcc);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

int QMP4AtomAVC1_GetBoxAllocSize(QMP4_ATOM_AVC1 *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_AVC1, headerAlign);

    size += atombox->avcc->GetBoxAllocSize(atombox->avcc);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

#ifdef __cplusplus
}
#endif

