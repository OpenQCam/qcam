#ifndef __QATOMDINF_H__
#define __QATOMDINF_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomdref.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_DINF QMP4_ATOM_DINF;
struct _QMP4_ATOM_DINF {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_DREF *dref;

    int (*Init)(QMP4_ATOM_DINF *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_DINF *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_DINF *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_DINF *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_DINF *atombox, FILE *file);
};

int QMP4AtomDINF_Init(QMP4_ATOM_DINF *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomDINF_Fini(QMP4_ATOM_DINF *atombox);
int QMP4AtomDINF_UpdateBoxSize(QMP4_ATOM_DINF *atombox);
int QMP4AtomDINF_GetBoxAllocSize(QMP4_ATOM_DINF *atombox);
int QMP4AtomDINF_WriteBoxToFile(QMP4_ATOM_DINF *atombox, FILE *file);

#define QMP4_ATOM_DINF_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_DREF_DESTRUCT(atombox->dref);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_DINF_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_DINF *)calloc(1, sizeof(QMP4_ATOM_DINF));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_DINF;\
        atombox->Init = QMP4AtomDINF_Init;\
        atombox->Fini = QMP4AtomDINF_Fini;\
        atombox->UpdateBoxSize = QMP4AtomDINF_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomDINF_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomDINF_WriteBoxToFile;\
        QMP4_ATOM_DREF_CONSTRUCT(atombox->dref);\
        if (!atombox->dref) {\
            QMP4_ATOM_DINF_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMDINF_H__ */

