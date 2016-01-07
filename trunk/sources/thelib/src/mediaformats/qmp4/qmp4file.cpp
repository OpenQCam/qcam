#include <stdio.h>
#include <errno.h>
#include "mediaformats/qmp4/qmp4file.h"
#include "mediaformats/qmp4/qmp4log.h"
#include "mediaformats/qmp4/qatommoov.h"
#include "mediaformats/qmp4/qatomtrak.h"
#include "mediaformats/qmp4/qatommdia.h"
#include "mediaformats/qmp4/qatomminf.h"
#include "mediaformats/qmp4/qatomstbl.h"
#include "mediaformats/qmp4/qatomstsd.h"
#include "mediaformats/qmp4/qatomavc1.h"
#include "mediaformats/qmp4/qatomavcc.h"
#include "mediaformats/qmp4/qatomstsc.h"
#include "mediaformats/qmp4/qatomstco.h"
#include "mediaformats/qmp4/qatomstts.h"
#include "mediaformats/qmp4/qatomstss.h"
#include "mediaformats/qmp4/qatomstsz.h"

#include "mediaformats/qmp4/qatommvhd.h"
#include "mediaformats/qmp4/qatomtkhd.h"
#include "mediaformats/qmp4/qatommdhd.h"

#ifdef __cplusplus
extern "C" {
#endif

int QMP4File_Init(QMP4_FILE *mp4file, 
                  QMP4_TRAK_INFO *vidTrakInfo,
                  QMP4_TRAK_INFO *audTrakInfo,
                  QMP4_FILE_TYPE type,
                  const char *fileName)
{
    int err = 0;
    int offsetToData = 0;

    qmp4_log_debug("%s() enter", __FUNCTION__);

    mp4file->file = fopen(fileName, "w");
    if (!mp4file->file) {
        err = -EIO;
        qmp4_log_err("%s(), failed to open %s", __FUNCTION__, fileName);
        goto FUNC_EXIT;
    }

    mp4file->ftyp->Init(mp4file->ftyp, type);
    mp4file->moov->Init(mp4file->moov, vidTrakInfo, audTrakInfo);
    mp4file->free->Init(mp4file->free);
    mp4file->mdat->Init(mp4file->mdat);

    offsetToData = mp4file->ftyp->GetBoxAllocSize(mp4file->ftyp);
    offsetToData += mp4file->moov->GetBoxAllocSize(mp4file->moov);
    offsetToData += mp4file->free->GetBoxAllocSize(mp4file->free);
    offsetToData += mp4file->mdat->GetBoxAllocSize(mp4file->mdat);

    err = fseek(mp4file->file, offsetToData, SEEK_SET);
    mp4file->currFileOffset = offsetToData;

FUNC_EXIT:

    qmp4_log_debug("%s() leave, err=%d", __FUNCTION__, err);

    return err;
}

int QMP4File_UpdateSPSR(QMP4_FILE *mp4file, 
                        unsigned char *packetSPS, 
                        unsigned int sizeSPS, 
                        unsigned char *packetPPS, 
                        unsigned int sizePPS) 
{
    int err = 0;

    QMP4_ATOM_AVCC *avcc = mp4file->moov->vidtrak->mdia->minf->stbl->stsd->avc1->avcc;

    err = avcc->UpdateSPS(avcc,
                          packetSPS, 
                          sizeSPS);

    err = err ? err : avcc->UpdatePPS(avcc,
                                      packetPPS, 
                                      sizePPS);

    qmp4_log_debug("%s() leave, sizeSPS=%d sizePPS=%d err=%d", __FUNCTION__, sizeSPS, sizePPS, err);

    return err;
}

int QMP4File_AddVidData(QMP4_FILE *mp4file, 
                        unsigned char *packet, 
                        unsigned int size,
                        unsigned int delta,
                        unsigned int isSyncFrame)
{
    int err = 0;

    QMP4_ATOM_STTS *stts = mp4file->moov->vidtrak->mdia->minf->stbl->stts;
    QMP4_ATOM_STSS *stss = mp4file->moov->vidtrak->mdia->minf->stbl->stss;
    QMP4_ATOM_STSZ *stsz = mp4file->moov->vidtrak->mdia->minf->stbl->stsz;
    QMP4_ATOM_STCO *stco = mp4file->moov->vidtrak->mdia->minf->stbl->stco;

    QMP4_ATOM_MVHD *mvhd = mp4file->moov->mvhd;
    QMP4_ATOM_TKHD *vidtkhd = mp4file->moov->vidtrak->tkhd;
    QMP4_ATOM_TKHD *audtkhd = mp4file->moov->audtrak->tkhd;
    QMP4_ATOM_MDHD *mdhd = mp4file->moov->vidtrak->mdia->mdhd;

    if (mp4file->file && packet && size) {

        /* update mdat size */
        err = mp4file->mdat->AddData(mp4file->mdat, size);

        /* update stts */
        err = err ? err : stts->AddData(stts, delta); 

        /* update stss */
        if (isSyncFrame) {
            err = err ? err : stss->AddData(stss, mp4file->vidSampleCount+1);
        }

        /* update stsz */
        err = err ? err : stsz->AddData(stsz, size);

        /* update stco */
        err = err ? err : stco->AddData(stco, mp4file->currFileOffset);

        /* update duration in mvhd, tkhd and mdhd */
        err = err ? err : mvhd->UpdateDuration(mvhd, delta); 
        err = err ? err : vidtkhd->UpdateDuration(vidtkhd, delta); 
        err = err ? err : audtkhd->UpdateDuration(audtkhd, delta); 
        err = err ? err : mdhd->UpdateDuration(mdhd, delta); 

        if (err) {
            qmp4_log_err("%s(), failed in update atombox information", __FUNCTION__);
            goto FUNC_EXIT;
        }

        /* write mdat data to file */
        if (size != fwrite((void *)packet, 1, size, mp4file->file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, size);
            goto FUNC_EXIT;
        }

        mp4file->currFileOffset += size;
        mp4file->vidSampleCount += 1;
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, size=%d err=%d", __FUNCTION__, size, err);

    return err;
}

int QMP4File_AddAudData(QMP4_FILE *mp4file, 
                        unsigned char *packet, 
                        unsigned int size,
                        unsigned int sampleNums)
{
    int err = 0;

    QMP4_ATOM_STTS *stts = mp4file->moov->audtrak->mdia->minf->stbl->stts;
    QMP4_ATOM_STSC *stsc = mp4file->moov->audtrak->mdia->minf->stbl->stsc;
    QMP4_ATOM_STSZ *stsz = mp4file->moov->audtrak->mdia->minf->stbl->stsz;
    QMP4_ATOM_STCO *stco = mp4file->moov->audtrak->mdia->minf->stbl->stco;

    QMP4_ATOM_MDHD *mdhd = mp4file->moov->audtrak->mdia->mdhd;

    if (mp4file->file && packet && size) {

        /* update mdat size */
        err = mp4file->mdat->AddData(mp4file->mdat, size);

        /* update stts */
        err = err ? err : stts->AddData(stts, sampleNums);

        /* update stsc */
        err = err ? err : stsc->AddData(stsc, sampleNums);

        /* update stsz */
        err = err ? err : stsz->AddData(stsz, sampleNums);

        /* update stco */
        err = err ? err : stco->AddData(stco, mp4file->currFileOffset);

        /* update duration in mdhd */
        err = err ? err : mdhd->UpdateDuration(mdhd, sampleNums); 

        if (err) {
            qmp4_log_err("%s(), failed in update atombox information", __FUNCTION__);
            goto FUNC_EXIT;
        }

        /* write mdat data to file */
        if (size != fwrite((void *)packet, 1, size, mp4file->file)) {
            err = -EIO;
            qmp4_log_err("%s(), failed in fwrite(%d)", __FUNCTION__, size);
            goto FUNC_EXIT;
        }

        mp4file->currFileOffset += size;
        //mp4file->vidSampleCount += 1;
    }

FUNC_EXIT:    

    qmp4_log_debug("%s() leave, size=%d sampleNums=%d err=%d", __FUNCTION__, size, sampleNums, err);

    return err;
}

int QMP4File_Fini(QMP4_FILE *mp4file)
{
    int err = 0;
    unsigned int moovRealSize;
    unsigned int moovAllocSize;
    unsigned int toStuffingFree;

    mp4file->ftyp->UpdateBoxSize(mp4file->ftyp);
    moovRealSize = mp4file->moov->UpdateBoxSize(mp4file->moov);
    moovAllocSize = mp4file->moov->GetBoxAllocSize(mp4file->moov); 
    toStuffingFree = moovAllocSize - moovRealSize;
    if (toStuffingFree) {
        moovRealSize = mp4file->free->UpdateBoxSize(mp4file->free, toStuffingFree);
    }
    mp4file->mdat->UpdateBoxSize(mp4file->mdat);

    err = fseek(mp4file->file, 0, SEEK_SET);
    mp4file->currFileOffset = 0;

    mp4file->ftyp->WriteBoxToFile(mp4file->ftyp, mp4file->file);
    mp4file->moov->WriteBoxToFile(mp4file->moov, mp4file->file);
    mp4file->free->WriteBoxToFile(mp4file->free, mp4file->file);
    mp4file->mdat->WriteBoxToFile(mp4file->mdat, mp4file->file);

    mp4file->ftyp->Fini(mp4file->ftyp);
    mp4file->moov->Fini(mp4file->moov);
    mp4file->free->Fini(mp4file->free);
    mp4file->mdat->Fini(mp4file->mdat);

    if (mp4file->file) {
        fclose(mp4file->file); 
        mp4file->file = NULL;
    }

    qmp4_log_debug("%s() leave", __FUNCTION__);

    return err;
}

#ifdef __cplusplus
}
#endif

