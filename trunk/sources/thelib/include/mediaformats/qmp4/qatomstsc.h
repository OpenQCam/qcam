#ifndef __QATOMSTSC_H__
#define __QATOMSTSC_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ENTRY_STSC QMP4_ENTRY_STSC;
struct _QMP4_ENTRY_STSC {
    unsigned char firstChunk[4];
    unsigned char samplesPerChunk[4];
    unsigned char sampleDescriptionIndex[4];
};

typedef struct _QMP4_ATOM_STSC QMP4_ATOM_STSC;
struct _QMP4_ATOM_STSC {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    unsigned char *dataBuff;

    unsigned int  buffSize;
    unsigned int  dataLength;

    int (*Init)(QMP4_ATOM_STSC *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STSC *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STSC *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STSC *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STSC *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_STSC *atombox, unsigned int value);
};

int QMP4AtomSTSC_Init(QMP4_ATOM_STSC *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTSC_Fini(QMP4_ATOM_STSC *atombox);
int QMP4AtomSTSC_UpdateBoxSize(QMP4_ATOM_STSC *atombox);
int QMP4AtomSTSC_GetBoxAllocSize(QMP4_ATOM_STSC *atombox);
int QMP4AtomSTSC_WriteBoxToFile(QMP4_ATOM_STSC *atombox, FILE *file);
int QMP4AtomSTSC_AddData(QMP4_ATOM_STSC *atombox, unsigned int value);

#define QMP4_ATOM_STSC_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STSC_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STSC *)calloc(1, sizeof(QMP4_ATOM_STSC));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STSC;\
        atombox->Init = QMP4AtomSTSC_Init;\
        atombox->Fini = QMP4AtomSTSC_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTSC_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTSC_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTSC_WriteBoxToFile;\
        atombox->AddData = QMP4AtomSTSC_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTSC_H__ */

