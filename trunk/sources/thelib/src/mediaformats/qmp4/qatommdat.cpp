#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qatommdat.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomMDAT_Init(QMP4_ATOM_MDAT *atombox)
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

int QMP4AtomMDAT_Fini(QMP4_ATOM_MDAT *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomMDAT_WriteBoxToFile(QMP4_ATOM_MDAT *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_MDAT, headerAlign);
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

int QMP4AtomMDAT_UpdateBoxSize(QMP4_ATOM_MDAT *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MDAT, headerAlign);

    size += atombox->dataLength;
    
    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size; 
}

int QMP4AtomMDAT_GetBoxAllocSize(QMP4_ATOM_MDAT *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_MDAT, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);
        
    return size;    
}

int QMP4AtomMDAT_AddData(QMP4_ATOM_MDAT *atombox, unsigned int size)
{
    int err = 0;

    if (size) {
        atombox->dataLength += size;
    }

    return err;
}

#ifdef __cplusplus
}
#endif

