#ifndef __QMP4FILE_H__
#define __QMP4FILE_H__

#include <stdio.h>
#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomftyp.h"
#include "mediaformats/qmp4/qatommoov.h"
#include "mediaformats/qmp4/qatomfree.h"
#include "mediaformats/qmp4/qatommdat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_FILE QMP4_FILE;
struct _QMP4_FILE {
    QMP4_ATOM_FTYP *ftyp;
    QMP4_ATOM_MOOV *moov;
    QMP4_ATOM_FREE *free;
    QMP4_ATOM_MDAT *mdat;

    FILE *file;
    unsigned int currFileOffset;
    unsigned int vidSampleCount;

    int (*Init)(QMP4_FILE *mp4file, 
                QMP4_TRAK_INFO *vidTrakInfo, 
                QMP4_TRAK_INFO *audTrakInfo, 
                QMP4_FILE_TYPE type, 
                const char *fileName);
    int (*UpdateSPSR)(QMP4_FILE *mp4file, 
                      unsigned char *packetSPS, 
                      unsigned int sizeSPS,   
                      unsigned char *packetPPS,
                      unsigned int sizePPS);    
    int (*AddVidData)(QMP4_FILE *mp4file, 
                      unsigned char *packet, 
                      unsigned int size,
                      unsigned int delta,
                      unsigned int isSyncFrame);
    int (*AddAudData)(QMP4_FILE *mp4file, 
                      unsigned char *packet, 
                      unsigned int size,
                      unsigned int sampleNums);
    int (*Fini)(QMP4_FILE *mp4file);
};

int QMP4File_Init(QMP4_FILE *mp4file, 
                  QMP4_TRAK_INFO *vidTrakInfo, 
                  QMP4_TRAK_INFO *audTrakInfo, 
                  QMP4_FILE_TYPE type,
                  const char *fileName);
int QMP4File_UpdateSPSR(QMP4_FILE *mp4file,
                        unsigned char *packetSPS, 
                        unsigned int sizeSPS,   
                        unsigned char *packetPPS,
                        unsigned int sizePPS);
int QMP4File_AddVidData(QMP4_FILE *mp4file, 
                        unsigned char *packet, 
                        unsigned int size,
                        unsigned int delta,
                        unsigned int isSyncFrame);
int QMP4File_AddAudData(QMP4_FILE *mp4file, 
                        unsigned char *packet, 
                        unsigned int size,
                        unsigned int sampleNums);
int QMP4File_Fini(QMP4_FILE *mp4file);

#define QMP4_FILE_DESTRUCT(mp4file)\
{\
    if (mp4file) {\
        QMP4_ATOM_FTYP_DESTRUCT(mp4file->ftyp);\
        QMP4_ATOM_MOOV_DESTRUCT(mp4file->moov);\
        QMP4_ATOM_FREE_DESTRUCT(mp4file->free);\
        QMP4_ATOM_MDAT_DESTRUCT(mp4file->mdat);\
        free(mp4file);\
        mp4file = NULL;\
    }\
}

#define QMP4_FILE_CONSTRUCT(mp4file)\
{\
    mp4file = (QMP4_FILE *)calloc(1, sizeof(QMP4_FILE));\
    if (mp4file) {\
        mp4file->Init = QMP4File_Init;\
        mp4file->UpdateSPSR = QMP4File_UpdateSPSR;\
        mp4file->AddVidData = QMP4File_AddVidData;\
        mp4file->AddAudData = QMP4File_AddAudData;\
        mp4file->Fini = QMP4File_Fini;\
        QMP4_ATOM_FTYP_CONSTRUCT(mp4file->ftyp);\
        QMP4_ATOM_MOOV_CONSTRUCT(mp4file->moov);\
        QMP4_ATOM_FREE_CONSTRUCT(mp4file->free);\
        QMP4_ATOM_MDAT_CONSTRUCT(mp4file->mdat);\
        if ((!mp4file->ftyp) || (!mp4file->moov) || (!mp4file->mdat) || (!mp4file->free)) {\
            QMP4_ATOM_FTYP_DESTRUCT(mp4file->ftyp);\
            QMP4_ATOM_MOOV_DESTRUCT(mp4file->moov);\
            QMP4_ATOM_FREE_DESTRUCT(mp4file->free);\
            QMP4_ATOM_MDAT_DESTRUCT(mp4file->mdat);\
            QMP4_FILE_DESTRUCT(mp4file);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QMP4FILE_H__ */

