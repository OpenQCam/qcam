#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomavcc.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomAVCC_Init(QMP4_ATOM_AVCC *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    atombox->configurationVersion = 0x1;
    atombox->reserved0 = 0x3f;
    atombox->reserved1 = 0x7;
    atombox->numOfSequenceParameterSets = 1;
    atombox->numOfPictureParameterSets = 1;
    atombox->lengthSizeMinusOne = 3; 

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomAVCC_Fini(QMP4_ATOM_AVCC *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomAVCC_WriteBoxToFile(QMP4_ATOM_AVCC *atombox, FILE *file)
{
    int err = 0;

    if ((0 == QMP4_SWAP16(*(unsigned short *)atombox->sequenceParameterSetLength)) ||  
        (0 == QMP4_SWAP16(*(unsigned short *)atombox->pictureParameterSetLength))) {
        err = -ENODATA;
        qmp4_log_err("%s(), incorrect sps_length(%d) or pps_length(%d)", 
                     __FUNCTION__,
                     QMP4_SWAP16(*(unsigned short *)atombox->sequenceParameterSetLength), 
                     QMP4_SWAP16(*(unsigned short *)atombox->pictureParameterSetLength));
        goto FUNC_EXIT;
    }

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_AVCC, sequenceParameterSetNALUnit);
        if (toWrite != fwrite((void *)atombox, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        toWrite = QMP4_SWAP16(*(unsigned short *)atombox->sequenceParameterSetLength);
        if (toWrite != fwrite((void *)atombox->sequenceParameterSetNALUnit, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }

        toWrite = 3; /* numOfPictureParameterSets and pictureParameterSetLength[2] */
        toWrite += QMP4_SWAP16(*(unsigned short *)atombox->pictureParameterSetLength);
        if (toWrite != fwrite((void *)&atombox->numOfPictureParameterSets, 1, toWrite, file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, toWrite);
            goto FUNC_EXIT;
        }
    }

FUNC_EXIT:

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomAVCC_UpdateSPS(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize)
{
    int err = 0;

    /* example, 
    ** profileIndication = 66 (0x42)
    ** profileCompatibility = 224 (0xe0)
    ** levelIndication = 31 (0x1f)
    ** sequenceParameterSetLength = 14 (0x000e)
    ** sequenceParameterSetNALUnit = <14 bytes> 27 42 e0 1f 8d 68 05 00 5b fe 01 e2 0f 50
    */

    atombox->profileIndication = packetBuff[1];
    atombox->profileCompatibility = packetBuff[2];
    atombox->levelIndication = packetBuff[3];
    *(unsigned short *)atombox->sequenceParameterSetLength = QMP4_SWAP16((unsigned short)packetSize);
    memcpy(atombox->sequenceParameterSetNALUnit, packetBuff, packetSize);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomAVCC_UpdatePPS(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize)
{
    int err = 0;

    /* example,
    ** pictureParameterSetLength = 4 (0x0004)
    ** pictureParameterSetNALUnit = <4 bytes> 28 ce 1f 20    
    */

    *(unsigned short *)atombox->pictureParameterSetLength = QMP4_SWAP16((unsigned short)packetSize);
    memcpy(atombox->pictureParameterSetNALUnit, packetBuff, packetSize);

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomAVCC_UpdateBoxSize(QMP4_ATOM_AVCC *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_AVCC, headerAlign);

    size -= (sizeof(atombox->sequenceParameterSetNALUnit) - QMP4_SWAP16(*(unsigned short *)atombox->sequenceParameterSetLength));
    size -= (sizeof(atombox->pictureParameterSetNALUnit) - QMP4_SWAP16(*(unsigned short *)atombox->pictureParameterSetLength));

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

int QMP4AtomAVCC_GetBoxAllocSize(QMP4_ATOM_AVCC *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_AVCC, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

#ifdef __cplusplus
}
#endif

