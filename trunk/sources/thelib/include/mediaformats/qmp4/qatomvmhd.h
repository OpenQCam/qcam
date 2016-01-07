#ifndef __QATOMVMHD_H__
#define __QATOMVMHD_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_VMHD QMP4_ATOM_VMHD;
struct _QMP4_ATOM_VMHD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char graphicsMode[2];
    unsigned char opColor[2*3];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_VMHD *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_VMHD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_VMHD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_VMHD *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_VMHD *atombox, FILE *file);
};

int QMP4AtomVMHD_Init(QMP4_ATOM_VMHD *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomVMHD_Fini(QMP4_ATOM_VMHD *atombox);
int QMP4AtomVMHD_UpdateBoxSize(QMP4_ATOM_VMHD *atombox);
int QMP4AtomVMHD_GetBoxAllocSize(QMP4_ATOM_VMHD *atombox);
int QMP4AtomVMHD_WriteBoxToFile(QMP4_ATOM_VMHD *atombox, FILE *file);

#define QMP4_ATOM_VMHD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_VMHD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_VMHD *)calloc(1, sizeof(QMP4_ATOM_VMHD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_VMHD;\
        atombox->Init = QMP4AtomVMHD_Init;\
        atombox->Fini = QMP4AtomVMHD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomVMHD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomVMHD_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomVMHD_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMVMHD_H__ */

