#ifndef __QATOMTKHD_H__
#define __QATOMTKHD_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_TKHD QMP4_ATOM_TKHD;
struct _QMP4_ATOM_TKHD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char creationTime[4];      /* support version 0 only */
    unsigned char modificationTime[4];  /* support version 0 only */
    unsigned char trackID[4];
    unsigned char reserved0[4];
    unsigned char duration[4];          /* support version 0 only */
    unsigned char reserved1[4*2];
    unsigned char layer[2];
    unsigned char alternateGroup[2];
    unsigned char volume[2];
    unsigned char reserved3[2];
    unsigned char matrix[4*9];
    unsigned char width[4];
    unsigned char height[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    int (*Init)(QMP4_ATOM_TKHD *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_TKHD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_TKHD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_TKHD *atombox);
    int (*UpdateDuration)(QMP4_ATOM_TKHD *atombox, unsigned int delta);
    int (*WriteBoxToFile)(QMP4_ATOM_TKHD *atombox, FILE *file);
};

int QMP4AtomTKHD_Init(QMP4_ATOM_TKHD *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomTKHD_Fini(QMP4_ATOM_TKHD *atombox);
int QMP4AtomTKHD_UpdateBoxSize(QMP4_ATOM_TKHD *atombox);
int QMP4AtomTKHD_GetBoxAllocSize(QMP4_ATOM_TKHD *atombox);
int QMP4AtomTKHD_UpdateDuration(QMP4_ATOM_TKHD *atombox, unsigned int delta);
int QMP4AtomTKHD_WriteBoxToFile(QMP4_ATOM_TKHD *atombox, FILE *file);

#define QMP4_ATOM_TKHD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_TKHD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_TKHD *)calloc(1, sizeof(QMP4_ATOM_TKHD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_TKHD;\
        atombox->Init = QMP4AtomTKHD_Init;\
        atombox->Fini = QMP4AtomTKHD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomTKHD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomTKHD_GetBoxAllocSize;\
        atombox->UpdateDuration = QMP4AtomTKHD_UpdateDuration;\
        atombox->WriteBoxToFile = QMP4AtomTKHD_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMTKHD_H__ */

