#ifndef __QATOMSTBL_H__
#define __QATOMSTBL_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomstsd.h"
#include "mediaformats/qmp4/qatomstsc.h"
#include "mediaformats/qmp4/qatomstco.h"
#include "mediaformats/qmp4/qatomstts.h"
#include "mediaformats/qmp4/qatomstss.h"
#include "mediaformats/qmp4/qatomstsz.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_STBL QMP4_ATOM_STBL;
struct _QMP4_ATOM_STBL {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_STSD *stsd;
    QMP4_ATOM_STTS *stts;
    QMP4_ATOM_STSS *stss;
    QMP4_ATOM_STSC *stsc;
    QMP4_ATOM_STSZ *stsz;
    QMP4_ATOM_STCO *stco;

    int (*Init)(QMP4_ATOM_STBL *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STBL *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STBL *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STBL *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STBL *atombox, FILE *file);
};

int QMP4AtomSTBL_Init(QMP4_ATOM_STBL *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTBL_Fini(QMP4_ATOM_STBL *atombox);
int QMP4AtomSTBL_UpdateBoxSize(QMP4_ATOM_STBL *atombox);
int QMP4AtomSTBL_GetBoxAllocSize(QMP4_ATOM_STBL *atombox);
int QMP4AtomSTBL_WriteBoxToFile(QMP4_ATOM_STBL *atombox, FILE *file);

#define QMP4_ATOM_STBL_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_STSD_DESTRUCT(atombox->stsd);\
        QMP4_ATOM_STTS_DESTRUCT(atombox->stts);\
        QMP4_ATOM_STSS_DESTRUCT(atombox->stss);\
        QMP4_ATOM_STSC_DESTRUCT(atombox->stsc);\
        QMP4_ATOM_STSZ_DESTRUCT(atombox->stsz);\
        QMP4_ATOM_STCO_DESTRUCT(atombox->stco);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STBL_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STBL *)calloc(1, sizeof(QMP4_ATOM_STBL));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_STBL;\
        atombox->Init = QMP4AtomSTBL_Init;\
        atombox->Fini = QMP4AtomSTBL_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTBL_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTBL_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTBL_WriteBoxToFile;\
        QMP4_ATOM_STSD_CONSTRUCT(atombox->stsd);\
        QMP4_ATOM_STTS_CONSTRUCT(atombox->stts);\
        QMP4_ATOM_STSS_CONSTRUCT(atombox->stss);\
        QMP4_ATOM_STSC_CONSTRUCT(atombox->stsc);\
        QMP4_ATOM_STSZ_CONSTRUCT(atombox->stsz);\
        QMP4_ATOM_STCO_CONSTRUCT(atombox->stco);\
        if ((!atombox->stsd) || (!atombox->stsc) || (!atombox->stco) || (!atombox->stts) ||\
            (!atombox->stss) || (!atombox->stsz)) {\
            QMP4_ATOM_STSD_DESTRUCT(atombox->stsd);\
            QMP4_ATOM_STTS_DESTRUCT(atombox->stts);\
            QMP4_ATOM_STSS_DESTRUCT(atombox->stss);\
            QMP4_ATOM_STSC_DESTRUCT(atombox->stsc);\
            QMP4_ATOM_STSZ_DESTRUCT(atombox->stsz);\
            QMP4_ATOM_STCO_DESTRUCT(atombox->stco);\
            QMP4_ATOM_STBL_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTBL_H__ */

