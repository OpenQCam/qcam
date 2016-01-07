#ifndef __QATOMDREF_H__
#define __QATOMDREF_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomurl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_DREF QMP4_ATOM_DREF;
struct _QMP4_ATOM_DREF {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_URL *url;

    int (*Init)(QMP4_ATOM_DREF *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_DREF *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_DREF *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_DREF *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_DREF *atombox, FILE *file);
};

int QMP4AtomDREF_Init(QMP4_ATOM_DREF *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomDREF_Fini(QMP4_ATOM_DREF *atombox);
int QMP4AtomDREF_UpdateBoxSize(QMP4_ATOM_DREF *atombox);
int QMP4AtomDREF_GetBoxAllocSize(QMP4_ATOM_DREF *atombox);
int QMP4AtomDREF_WriteBoxToFile(QMP4_ATOM_DREF *atombox, FILE *file);

#define QMP4_ATOM_DREF_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_URL_DESTRUCT(atombox->url);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_DREF_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_DREF *)calloc(1, sizeof(QMP4_ATOM_DREF));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_DREF;\
        atombox->Init = QMP4AtomDREF_Init;\
        atombox->Fini = QMP4AtomDREF_Fini;\
        atombox->UpdateBoxSize = QMP4AtomDREF_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomDREF_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomDREF_WriteBoxToFile;\
        QMP4_ATOM_URL_CONSTRUCT(atombox->url);\
        if (!atombox->url) {\
            QMP4_ATOM_DREF_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMDREF_H__ */

