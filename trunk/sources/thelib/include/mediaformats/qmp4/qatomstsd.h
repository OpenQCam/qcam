#ifndef __QATOMSTSD_H__
#define __QATOMSTSD_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomavc1.h"
#include "mediaformats/qmp4/qatomsowt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_STSD QMP4_ATOM_STSD;
struct _QMP4_ATOM_STSD {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_AVC1 *avc1;
    QMP4_ATOM_SOWT *sowt;

    int (*Init)(QMP4_ATOM_STSD *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STSD *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STSD *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STSD *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STSD *atombox, FILE *file);
};

int QMP4AtomSTSD_Init(QMP4_ATOM_STSD *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTSD_Fini(QMP4_ATOM_STSD *atombox);
int QMP4AtomSTSD_UpdateBoxSize(QMP4_ATOM_STSD *atombox);
int QMP4AtomSTSD_GetBoxAllocSize(QMP4_ATOM_STSD *atombox);
int QMP4AtomSTSD_WriteBoxToFile(QMP4_ATOM_STSD *atombox, FILE *file);

#define QMP4_ATOM_STSD_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_AVC1_DESTRUCT(atombox->avc1);\
        QMP4_ATOM_SOWT_DESTRUCT(atombox->sowt);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STSD_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STSD *)calloc(1, sizeof(QMP4_ATOM_STSD));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STSD;\
        atombox->Init = QMP4AtomSTSD_Init;\
        atombox->Fini = QMP4AtomSTSD_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTSD_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTSD_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTSD_WriteBoxToFile;\
        QMP4_ATOM_AVC1_CONSTRUCT(atombox->avc1);\
        QMP4_ATOM_SOWT_CONSTRUCT(atombox->sowt);\
        if ((!atombox->avc1) || (!atombox->sowt)) {\
            QMP4_ATOM_AVC1_DESTRUCT(atombox->avc1);\
            QMP4_ATOM_SOWT_DESTRUCT(atombox->sowt);\
            QMP4_ATOM_STSD_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTSD_H__ */

