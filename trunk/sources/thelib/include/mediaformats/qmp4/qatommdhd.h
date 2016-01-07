#ifndef __QATOMMDHD_H__
#define __QATOMMDHD_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MDHD QMP4_ATOM_MDHD;
struct _QMP4_ATOM_MDHD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char creationTime[4];      /* support version 0 only */
    unsigned char modificationTime[4];  /* support version 0 only */
    unsigned char timescale[4];
    unsigned char duration[4];          /* support version 0 only */
    unsigned char language[2];
    unsigned char preDefined[2];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    int (*Init)(QMP4_ATOM_MDHD *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_MDHD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MDHD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MDHD *atombox);
    int (*UpdateDuration)(QMP4_ATOM_MDHD *atombox, unsigned int delta);
    int (*WriteBoxToFile)(QMP4_ATOM_MDHD *atombox, FILE *file);
};

int QMP4AtomMDHD_Init(QMP4_ATOM_MDHD *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomMDHD_Fini(QMP4_ATOM_MDHD *atombox);
int QMP4AtomMDHD_UpdateBoxSize(QMP4_ATOM_MDHD *atombox);
int QMP4AtomMDHD_GetBoxAllocSize(QMP4_ATOM_MDHD *atombox);
int QMP4AtomMDHD_UpdateDuration(QMP4_ATOM_MDHD *atombox, unsigned int delta);
int QMP4AtomMDHD_WriteBoxToFile(QMP4_ATOM_MDHD *atombox, FILE *file);

#define QMP4_ATOM_MDHD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MDHD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MDHD *)calloc(1, sizeof(QMP4_ATOM_MDHD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_MDHD;\
        atombox->Init = QMP4AtomMDHD_Init;\
        atombox->Fini = QMP4AtomMDHD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMDHD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMDHD_GetBoxAllocSize;\
        atombox->UpdateDuration = QMP4AtomMDHD_UpdateDuration;\
        atombox->WriteBoxToFile = QMP4AtomMDHD_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMDHD_H__ */

