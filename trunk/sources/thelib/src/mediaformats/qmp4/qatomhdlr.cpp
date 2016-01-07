#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomhdlr.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomHDLR_Init(QMP4_ATOM_HDLR *atombox, QMP4_TRAK_INFO *trakInfo, QMP4_HDLR_TYPE handlerType)
{
    int err = 0;

    atombox->trakType = trakInfo->type;

    if (QMP4_HDLR_TYPE_MEDIA_HANDLER == handlerType) { 
        *(unsigned int *)atombox->preDefined = MAKE_FOURCC('m', 'h', 'l', 'r');
        if (QMP4_TRAK_TYPE_AUDIO_PCM == atombox->trakType) {
            *(unsigned int *)atombox->handlerType = MAKE_FOURCC('s', 'o', 'u', 'n');
        } else {
            *(unsigned int *)atombox->handlerType = MAKE_FOURCC('v', 'i', 'd', 'e');
        }
    } else {
        *(unsigned int *)atombox->preDefined = MAKE_FOURCC('d', 'h', 'l', 'r');
        *(unsigned int *)atombox->handlerType = MAKE_FOURCC('a', 'l', 'i', 's');
    }

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomHDLR_Fini(QMP4_ATOM_HDLR *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomHDLR_WriteBoxToFile(QMP4_ATOM_HDLR *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_HDLR, headerAlign);
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

int QMP4AtomHDLR_UpdateBoxSize(QMP4_ATOM_HDLR *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_HDLR, headerAlign);

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size);    

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

int QMP4AtomHDLR_GetBoxAllocSize(QMP4_ATOM_HDLR *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_HDLR, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

#ifdef __cplusplus
}
#endif

