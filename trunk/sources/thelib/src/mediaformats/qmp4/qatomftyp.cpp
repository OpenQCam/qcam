#include <errno.h>
#include "mediaformats/qmp4/qatomftyp.h"
#include "mediaformats/qmp4/qmp4log.h"

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomFTYP_Init(QMP4_ATOM_FTYP *atombox, QMP4_FILE_TYPE type)
{
    int err = 0;

    atombox->filetype = type;

    if (QMP4_FILE_TYPE_MOV == type) {
        *(unsigned int *)atombox->majorBrand = FOURCC_QT;
        *(unsigned int *)atombox->minorVersion = QMP4_SWAP32(0x00000200);
        //*(unsigned int *)atombox->minorVersion = QMP4_SWAP32(0x20070900);
        *(unsigned int *)atombox->brand0 = FOURCC_QT;
        *(unsigned int *)atombox->brand1 = 0;
        *(unsigned int *)atombox->brand2 = 0;
        *(unsigned int *)atombox->brand3 = 0;
    } else {
        *(unsigned int *)atombox->majorBrand = FOURCC_ISOM;
        *(unsigned int *)atombox->minorVersion = QMP4_SWAP32(0x00000200);
        *(unsigned int *)atombox->brand0 = FOURCC_MP41;
        *(unsigned int *)atombox->brand1 = FOURCC_AVC1;
        *(unsigned int *)atombox->brand2 = FOURCC_QT;
    }

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c) majorBrand(%c%c%c%c) type(%s)", 
                   __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3],
                   atombox->majorBrand[0],
                   atombox->majorBrand[1],
                   atombox->majorBrand[2],
                   atombox->majorBrand[3],
                   (QMP4_FILE_TYPE_MOV == atombox->filetype) ? "mov" : "mp4");

    return err;
}

int QMP4AtomFTYP_Fini(QMP4_ATOM_FTYP *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomFTYP_WriteBoxToFile(QMP4_ATOM_FTYP *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = QMP4_SWAP32(*(unsigned int *)atombox->extendedBox.boxsize);

        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomFTYP_UpdateBoxSize(QMP4_ATOM_FTYP *atombox)
{
    int size = 0;

    size = QMP4AtomFTYP_GetBoxAllocSize(atombox);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

int QMP4AtomFTYP_GetBoxAllocSize(QMP4_ATOM_FTYP *atombox)
{
    int size = 0;

    if (0 == *(unsigned int *)atombox->brand1) {
        size = GET_FIELD_OFFSET(QMP4_ATOM_FTYP, brand1);
    } else if (0 == *(unsigned int *)atombox->brand2) {
        size = GET_FIELD_OFFSET(QMP4_ATOM_FTYP, brand2);
    } else if (0 == *(unsigned int *)atombox->brand3) {
        size = GET_FIELD_OFFSET(QMP4_ATOM_FTYP, brand3);
    } else {
        size = GET_FIELD_OFFSET(QMP4_ATOM_FTYP, headerAlign);
    }

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;
}

#ifdef __cplusplus
}
#endif

