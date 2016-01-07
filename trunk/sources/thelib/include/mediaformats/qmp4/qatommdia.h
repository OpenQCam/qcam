#ifndef __QATOMMDIA_H__
#define __QATOMMDIA_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatommdhd.h"
#include "mediaformats/qmp4/qatomhdlr.h"
#include "mediaformats/qmp4/qatomminf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MDIA QMP4_ATOM_MDIA;
struct _QMP4_ATOM_MDIA {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_MDHD *mdhd;
    QMP4_ATOM_HDLR *hdlr;
    QMP4_ATOM_MINF *minf;

    int (*Init)(QMP4_ATOM_MDIA *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_MDIA *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MDIA *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MDIA *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_MDIA *atombox, FILE *file);
};

int QMP4AtomMDIA_Init(QMP4_ATOM_MDIA *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomMDIA_Fini(QMP4_ATOM_MDIA *atombox);
int QMP4AtomMDIA_UpdateBoxSize(QMP4_ATOM_MDIA *atombox);
int QMP4AtomMDIA_GetBoxAllocSize(QMP4_ATOM_MDIA *atombox);
int QMP4AtomMDIA_WriteBoxToFile(QMP4_ATOM_MDIA *atombox, FILE *file);

#define QMP4_ATOM_MDIA_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_MDHD_DESTRUCT(atombox->mdhd);\
        QMP4_ATOM_HDLR_DESTRUCT(atombox->hdlr);\
        QMP4_ATOM_MINF_DESTRUCT(atombox->minf);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MDIA_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MDIA *)calloc(1, sizeof(QMP4_ATOM_MDIA));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_MDIA;\
        atombox->Init = QMP4AtomMDIA_Init;\
        atombox->Fini = QMP4AtomMDIA_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMDIA_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMDIA_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomMDIA_WriteBoxToFile;\
        QMP4_ATOM_MDHD_CONSTRUCT(atombox->mdhd);\
        QMP4_ATOM_HDLR_CONSTRUCT(atombox->hdlr);\
        QMP4_ATOM_MINF_CONSTRUCT(atombox->minf);\
        if ((!atombox->mdhd) || (!atombox->hdlr) || (!atombox->minf)) {\
            QMP4_ATOM_MDHD_DESTRUCT(atombox->mdhd);\
            QMP4_ATOM_HDLR_DESTRUCT(atombox->hdlr);\
            QMP4_ATOM_MINF_DESTRUCT(atombox->minf);\
            QMP4_ATOM_MDIA_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMDIA_H__ */

