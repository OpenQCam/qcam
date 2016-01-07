#ifndef __QATOMURL_H__
#define __QATOMURL_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_URL QMP4_ATOM_URL;
struct _QMP4_ATOM_URL {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char name[256];
    unsigned char location[1024];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_URL *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_URL *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_URL *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_URL *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_URL *atombox, FILE *file);
};

int QMP4AtomURL_Init(QMP4_ATOM_URL *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomURL_Fini(QMP4_ATOM_URL *atombox);
int QMP4AtomURL_UpdateBoxSize(QMP4_ATOM_URL *atombox);
int QMP4AtomURL_GetBoxAllocSize(QMP4_ATOM_URL *atombox);
int QMP4AtomURL_WriteBoxToFile(QMP4_ATOM_URL *atombox, FILE *file);

#define QMP4_ATOM_URL_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_URL_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_URL *)calloc(1, sizeof(QMP4_ATOM_URL));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_URL;\
        atombox->Init = QMP4AtomURL_Init;\
        atombox->Fini = QMP4AtomURL_Fini;\
        atombox->UpdateBoxSize = QMP4AtomURL_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomURL_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomURL_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMURL_H__ */

