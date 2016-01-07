#ifndef __QATOMMVHD_H__
#define __QATOMMVHD_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MVHD QMP4_ATOM_MVHD;
struct _QMP4_ATOM_MVHD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char creationTime[4];      /* support version 0 only */
    unsigned char modificationTime[4];  /* support version 0 only */
    unsigned char timescale[4];
    unsigned char duration[4];          /* support version 0 only */
    unsigned char rate[4];
    unsigned char volume[2];
    unsigned char reserved0[2];
    unsigned char reserved1[4*2];
    unsigned char matrix[4*9];
    unsigned char preDefined[4*6];
    unsigned char nextTrackID[4];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_MVHD *atombox,
                QMP4_TRAK_INFO *vidTrakInfo,
                QMP4_TRAK_INFO *audTrakInfo);
    int (*Fini)(QMP4_ATOM_MVHD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MVHD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MVHD *atombox);
    int (*UpdateDuration)(QMP4_ATOM_MVHD *atombox, unsigned int delta);
    int (*WriteBoxToFile)(QMP4_ATOM_MVHD *atombox, FILE *file);
};

int QMP4AtomMVHD_Init(QMP4_ATOM_MVHD *atombox,
                      QMP4_TRAK_INFO *vidTrakInfo,
                      QMP4_TRAK_INFO *audTrakInfo);
int QMP4AtomMVHD_Fini(QMP4_ATOM_MVHD *atombox);
int QMP4AtomMVHD_UpdateBoxSize(QMP4_ATOM_MVHD *atombox);
int QMP4AtomMVHD_GetBoxAllocSize(QMP4_ATOM_MVHD *atombox);
int QMP4AtomMVHD_UpdateDuration(QMP4_ATOM_MVHD *atombox, unsigned int delta);
int QMP4AtomMVHD_WriteBoxToFile(QMP4_ATOM_MVHD *atombox, FILE *file);

#define QMP4_ATOM_MVHD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MVHD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MVHD *)calloc(1, sizeof(QMP4_ATOM_MVHD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_MVHD;\
        atombox->Init = QMP4AtomMVHD_Init;\
        atombox->Fini = QMP4AtomMVHD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMVHD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMVHD_GetBoxAllocSize;\
        atombox->UpdateDuration = QMP4AtomMVHD_UpdateDuration;\
        atombox->WriteBoxToFile = QMP4AtomMVHD_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMVHD_H__ */

