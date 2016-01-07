#ifndef __QATOMSMHD_H__
#define __QATOMSMHD_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_SMHD QMP4_ATOM_SMHD;
struct _QMP4_ATOM_SMHD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char balance[2];
    unsigned char reserved0[2];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_SMHD *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_SMHD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_SMHD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_SMHD *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_SMHD *atombox, FILE *file);
};

int QMP4AtomSMHD_Init(QMP4_ATOM_SMHD *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSMHD_Fini(QMP4_ATOM_SMHD *atombox);
int QMP4AtomSMHD_UpdateBoxSize(QMP4_ATOM_SMHD *atombox);
int QMP4AtomSMHD_GetBoxAllocSize(QMP4_ATOM_SMHD *atombox);
int QMP4AtomSMHD_WriteBoxToFile(QMP4_ATOM_SMHD *atombox, FILE *file);

#define QMP4_ATOM_SMHD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_SMHD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_SMHD *)calloc(1, sizeof(QMP4_ATOM_SMHD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_SMHD;\
        atombox->Init = QMP4AtomSMHD_Init;\
        atombox->Fini = QMP4AtomSMHD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSMHD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSMHD_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSMHD_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSMHD_H__ */

