#ifndef __QATOMFREE_H__
#define __QATOMFREE_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_FREE QMP4_ATOM_FREE;
struct _QMP4_ATOM_FREE {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    unsigned int bytesToStuffing;

    int (*Init)(QMP4_ATOM_FREE *atombox);
    int (*Fini)(QMP4_ATOM_FREE *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_FREE *atombox, unsigned int bytesToStuffing);
    int (*GetBoxAllocSize)(QMP4_ATOM_FREE *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_FREE *atombox, FILE *file);
};

int QMP4AtomFREE_Init(QMP4_ATOM_FREE *atombox);
int QMP4AtomFREE_Fini(QMP4_ATOM_FREE *atombox);
int QMP4AtomFREE_UpdateBoxSize(QMP4_ATOM_FREE *atombox, unsigned int bytesToStuffing);
int QMP4AtomFREE_GetBoxAllocSize(QMP4_ATOM_FREE *atombox);
int QMP4AtomFREE_WriteBoxToFile(QMP4_ATOM_FREE *atombox, FILE *file);

#define QMP4_ATOM_FREE_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_FREE_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_FREE *)calloc(1, sizeof(QMP4_ATOM_FREE));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_FREE;\
        atombox->Init = QMP4AtomFREE_Init;\
        atombox->Fini = QMP4AtomFREE_Fini;\
        atombox->UpdateBoxSize = QMP4AtomFREE_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomFREE_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomFREE_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMFREE_H__ */

