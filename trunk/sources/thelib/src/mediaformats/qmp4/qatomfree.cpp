#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomfree.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomFREE_Init(QMP4_ATOM_FREE *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomFREE_Fini(QMP4_ATOM_FREE *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomFREE_WriteBoxToFile(QMP4_ATOM_FREE *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_FREE, headerAlign);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        err = fseek(file, atombox->bytesToStuffing, SEEK_CUR);
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, bytesToStuffing=%d err=%d", __FUNCTION__, atombox->bytesToStuffing, err);

    return err;
}

int QMP4AtomFREE_UpdateBoxSize(QMP4_ATOM_FREE *atombox, unsigned int bytesToStuffing)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_FREE, headerAlign);

    size += bytesToStuffing;

    atombox->bytesToStuffing = bytesToStuffing;

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomFREE_GetBoxAllocSize(QMP4_ATOM_FREE *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_FREE, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

#ifdef __cplusplus
}
#endif

