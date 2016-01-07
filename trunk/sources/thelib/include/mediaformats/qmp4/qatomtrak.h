#ifndef __QATOMTRAK_H__
#define __QATOMTRAK_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomtkhd.h"
#include "mediaformats/qmp4/qatommdia.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_TRAK QMP4_ATOM_TRAK;
struct _QMP4_ATOM_TRAK {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_TKHD *tkhd;
    QMP4_ATOM_MDIA *mdia;

    int (*Init)(QMP4_ATOM_TRAK *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_TRAK *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_TRAK *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_TRAK *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_TRAK *atombox, FILE *file);
};

int QMP4AtomTRAK_Init(QMP4_ATOM_TRAK *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomTRAK_Fini(QMP4_ATOM_TRAK *atombox);
int QMP4AtomTRAK_UpdateBoxSize(QMP4_ATOM_TRAK *atombox);
int QMP4AtomTRAK_GetBoxAllocSize(QMP4_ATOM_TRAK *atombox);
int QMP4AtomTRAK_WriteBoxToFile(QMP4_ATOM_TRAK *atombox, FILE *file);

#define QMP4_ATOM_TRAK_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_TKHD_DESTRUCT(atombox->tkhd);\
        QMP4_ATOM_MDIA_DESTRUCT(atombox->mdia);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_TRAK_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_TRAK *)calloc(1, sizeof(QMP4_ATOM_TRAK));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_TRAK;\
        atombox->Init = QMP4AtomTRAK_Init;\
        atombox->Fini = QMP4AtomTRAK_Fini;\
        atombox->UpdateBoxSize = QMP4AtomTRAK_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomTRAK_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomTRAK_WriteBoxToFile;\
        QMP4_ATOM_TKHD_CONSTRUCT(atombox->tkhd);\
        QMP4_ATOM_MDIA_CONSTRUCT(atombox->mdia);\
        if ((!atombox->tkhd) || (!atombox->mdia)) {\
            QMP4_ATOM_TKHD_DESTRUCT(atombox->tkhd);\
            QMP4_ATOM_MDIA_DESTRUCT(atombox->mdia);\
            QMP4_ATOM_TRAK_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMTRAK_H__ */

