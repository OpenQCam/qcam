#ifndef __QATOMMDAT_H__
#define __QATOMMDAT_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MDAT QMP4_ATOM_MDAT;
struct _QMP4_ATOM_MDAT {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    unsigned int dataLength;

    int (*Init)(QMP4_ATOM_MDAT *atombox);
    int (*Fini)(QMP4_ATOM_MDAT *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MDAT *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MDAT *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_MDAT *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_MDAT *atombox, unsigned int size);
};

int QMP4AtomMDAT_Init(QMP4_ATOM_MDAT *atombox);
int QMP4AtomMDAT_Fini(QMP4_ATOM_MDAT *atombox);
int QMP4AtomMDAT_UpdateBoxSize(QMP4_ATOM_MDAT *atombox);
int QMP4AtomMDAT_GetBoxAllocSize(QMP4_ATOM_MDAT *atombox);
int QMP4AtomMDAT_WriteBoxToFile(QMP4_ATOM_MDAT *atombox, FILE *file);
int QMP4AtomMDAT_AddData(QMP4_ATOM_MDAT *atombox, unsigned int size);

#define QMP4_ATOM_MDAT_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MDAT_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MDAT *)calloc(1, sizeof(QMP4_ATOM_MDAT));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_MDAT;\
        atombox->Init = QMP4AtomMDAT_Init;\
        atombox->Fini = QMP4AtomMDAT_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMDAT_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMDAT_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomMDAT_WriteBoxToFile;\
        atombox->AddData = QMP4AtomMDAT_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMDAT_H__ */

