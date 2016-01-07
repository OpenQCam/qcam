#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomurl.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomURL_Init(QMP4_ATOM_URL *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->fullBox.flags[2] = 0x1;

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->fullBox.extendedBox.boxtype[0],
                   atombox->fullBox.extendedBox.boxtype[1],
                   atombox->fullBox.extendedBox.boxtype[2],
                   atombox->fullBox.extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomURL_Fini(QMP4_ATOM_URL *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomURL_WriteBoxToFile(QMP4_ATOM_URL *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_URL, name);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        if (strlen((char *)atombox->name) && strlen((char *)atombox->location)) {
            toWrite = strlen((char *)atombox->name) + 1;
            if (toWrite != fwrite((void *)atombox->name, 1, toWrite, file)) {
                err = -EIO;
                qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
                goto FUNC_EXIT;
            }

            toWrite = strlen((char *)atombox->location) + 1;
            if (toWrite != fwrite((void *)atombox->location, 1, toWrite, file)) {
                err = -EIO;
                qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
                goto FUNC_EXIT;
            }
        }
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4AtomURL_UpdateBoxSize(QMP4_ATOM_URL *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_URL, name);

    if (strlen((char *)atombox->name) && strlen((char *)atombox->location)) {
        size += (strlen((char *)atombox->name) + 1);        /* +1 for null terminator */
        size += (strlen((char *)atombox->location) + 1);    /* +1 for null terminator */
    }

    *(unsigned int *)atombox->fullBox.extendedBox.boxsize = QMP4_SWAP32(size); 

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

int QMP4AtomURL_GetBoxAllocSize(QMP4_ATOM_URL *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_URL, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size; 
}

#ifdef __cplusplus
}
#endif

