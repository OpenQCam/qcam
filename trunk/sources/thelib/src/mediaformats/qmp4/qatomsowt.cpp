#include <string.h>
#include <errno.h>
#include "mediaformats/qmp4/qatomsowt.h"
#include "mediaformats/qmp4/qmp4log.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4AtomSOWT_Init(QMP4_ATOM_SOWT *atombox, QMP4_TRAK_INFO *trakInfo)
{
    int err = 0;

    unsigned char predefined1[24] = {
        0x00, 0x00, 0x00, 0x18, 0x63, 0x68, 0x61, 0x6e,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    *(unsigned short *)atombox->dataReferenceIndex = QMP4_SWAP16(1);
    *(unsigned short *)atombox->version = QMP4_SWAP16(1);
    *(unsigned short *)atombox->channelCount = QMP4_SWAP16((unsigned short)trakInfo->info.audio.channel_num);
    *(unsigned short *)atombox->sampleSize = QMP4_SWAP16(16);
    *(unsigned short *)atombox->sampleRateLo = QMP4_SWAP16((unsigned short)(0xFFFF & trakInfo->info.audio.sample_time_scale));
    *(unsigned short *)atombox->sampleRateHi = QMP4_SWAP16((unsigned short)((0xFFFF0000 & trakInfo->info.audio.sample_time_scale) >> 8));
    *(unsigned int *)atombox->samplePerPacket = QMP4_SWAP32(1);
    *(unsigned int *)atombox->bytesPerPacket = QMP4_SWAP32(2);
    *(unsigned int *)atombox->bytesPerFrame = QMP4_SWAP32(2*trakInfo->info.audio.channel_num);
    *(unsigned int *)atombox->bytesPerSample = QMP4_SWAP32(2);
    memcpy(atombox->predefined1, predefined1, sizeof(predefined1));

    qmp4_log_debug("%s() leave, atombox(0x%08x) boxtype(%c%c%c%c)", __FUNCTION__, 
                   (unsigned int)atombox, 
                   atombox->extendedBox.boxtype[0],
                   atombox->extendedBox.boxtype[1],
                   atombox->extendedBox.boxtype[2],
                   atombox->extendedBox.boxtype[3]);

    return err;
}

int QMP4AtomSOWT_Fini(QMP4_ATOM_SOWT *atombox)
{
    int err = 0;

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

int QMP4AtomSOWT_WriteBoxToFile(QMP4_ATOM_SOWT *atombox, FILE *file)
{
    int err = 0;

    if (file) {
        unsigned int toWrite = GET_FIELD_OFFSET(QMP4_ATOM_SOWT, headerAlign);
                    
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

int QMP4AtomSOWT_UpdateBoxSize(QMP4_ATOM_SOWT *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_SOWT, headerAlign);

    *(unsigned int *)atombox->extendedBox.boxsize = QMP4_SWAP32(size);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

int QMP4AtomSOWT_GetBoxAllocSize(QMP4_ATOM_SOWT *atombox)
{
    int size = 0;

    size = GET_FIELD_OFFSET(QMP4_ATOM_SOWT, headerAlign);

    qmp4_log_debug("%s() leave, size=%d", __FUNCTION__, size);

    return size;                    
}

#ifdef __cplusplus
}
#endif

