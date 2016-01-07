#ifndef __QATOMSTSS_H__
#define __QATOMSTSS_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ENTRY_STSS QMP4_ENTRY_STSS;
struct _QMP4_ENTRY_STSS {
    unsigned char sampleNumber[4];
};

typedef struct _QMP4_ATOM_STSS QMP4_ATOM_STSS;
struct _QMP4_ATOM_STSS {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    unsigned char *dataBuff;
    unsigned int  buffSize;
    unsigned int  dataLength;

    int (*Init)(QMP4_ATOM_STSS *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STSS *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STSS *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STSS *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STSS *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_STSS *atombox, unsigned int value);
};

int QMP4AtomSTSS_Init(QMP4_ATOM_STSS *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTSS_Fini(QMP4_ATOM_STSS *atombox);
int QMP4AtomSTSS_UpdateBoxSize(QMP4_ATOM_STSS *atombox);
int QMP4AtomSTSS_GetBoxAllocSize(QMP4_ATOM_STSS *atombox);
int QMP4AtomSTSS_WriteBoxToFile(QMP4_ATOM_STSS *atombox, FILE *file);
int QMP4AtomSTSS_AddData(QMP4_ATOM_STSS *atombox, unsigned int value);

#define QMP4_ATOM_STSS_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STSS_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STSS *)calloc(1, sizeof(QMP4_ATOM_STSS));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STSS;\
        atombox->Init = QMP4AtomSTSS_Init;\
        atombox->Fini = QMP4AtomSTSS_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTSS_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTSS_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTSS_WriteBoxToFile;\
        atombox->AddData = QMP4AtomSTSS_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTSS_H__ */

