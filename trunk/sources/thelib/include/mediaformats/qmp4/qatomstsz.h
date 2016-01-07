#ifndef __QATOMSTSZ_H__
#define __QATOMSTSZ_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ENTRY_STSZ QMP4_ENTRY_STSZ;
struct _QMP4_ENTRY_STSZ {
    unsigned char entrySize[4];
};

typedef struct _QMP4_ATOM_STSZ QMP4_ATOM_STSZ;
struct _QMP4_ATOM_STSZ {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char sampleSize[4];
    unsigned char sampleCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    unsigned char *dataBuff;
    unsigned int  buffSize;
    unsigned int  dataLength;    

    int (*Init)(QMP4_ATOM_STSZ *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STSZ *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STSZ *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STSZ *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STSZ *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_STSZ *atombox, unsigned int value);
};

int QMP4AtomSTSZ_Init(QMP4_ATOM_STSZ *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTSZ_Fini(QMP4_ATOM_STSZ *atombox);
int QMP4AtomSTSZ_UpdateBoxSize(QMP4_ATOM_STSZ *atombox);
int QMP4AtomSTSZ_GetBoxAllocSize(QMP4_ATOM_STSZ *atombox);
int QMP4AtomSTSZ_WriteBoxToFile(QMP4_ATOM_STSZ *atombox, FILE *file);
int QMP4AtomSTSZ_AddData(QMP4_ATOM_STSZ *atombox, unsigned int value);

#define QMP4_ATOM_STSZ_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STSZ_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STSZ *)calloc(1, sizeof(QMP4_ATOM_STSZ));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STSZ;\
        atombox->Init = QMP4AtomSTSZ_Init;\
        atombox->Fini = QMP4AtomSTSZ_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTSZ_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTSZ_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTSZ_WriteBoxToFile;\
        atombox->AddData = QMP4AtomSTSZ_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTSZ_H__ */

