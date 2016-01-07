#ifndef __QATOMSTTS_H__
#define __QATOMSTTS_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ENTRY_STTS QMP4_ENTRY_STTS;
struct _QMP4_ENTRY_STTS {
    unsigned char sampleCount[4];
    unsigned char sampleDelta[4];
};

typedef struct _QMP4_ATOM_STTS QMP4_ATOM_STTS;
struct _QMP4_ATOM_STTS {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    unsigned char *dataBuff;
    unsigned int  buffSize;
    unsigned int  dataLength;

    int (*Init)(QMP4_ATOM_STTS *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STTS *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STTS *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STTS *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STTS *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_STTS *atombox, unsigned int value);
};

int QMP4AtomSTTS_Init(QMP4_ATOM_STTS *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTTS_Fini(QMP4_ATOM_STTS *atombox);
int QMP4AtomSTTS_UpdateBoxSize(QMP4_ATOM_STTS *atombox);
int QMP4AtomSTTS_GetBoxAllocSize(QMP4_ATOM_STTS *atombox);
int QMP4AtomSTTS_WriteBoxToFile(QMP4_ATOM_STTS *atombox, FILE *file);
int QMP4AtomSTTS_AddData(QMP4_ATOM_STTS *atombox, unsigned int value);

#define QMP4_ATOM_STTS_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STTS_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STTS *)calloc(1, sizeof(QMP4_ATOM_STTS));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STTS;\
        atombox->Init = QMP4AtomSTTS_Init;\
        atombox->Fini = QMP4AtomSTTS_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTTS_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTTS_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTTS_WriteBoxToFile;\
        atombox->AddData = QMP4AtomSTTS_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTTS_H__ */

