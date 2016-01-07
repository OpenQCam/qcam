#ifndef __QATOMSTCO_H__
#define __QATOMSTCO_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ENTRY_STCO QMP4_ENTRY_STCO;
struct _QMP4_ENTRY_STCO {
    unsigned char chunkOffset[4];
};

typedef struct _QMP4_ATOM_STCO QMP4_ATOM_STCO;
struct _QMP4_ATOM_STCO {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char entryCount[4];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    unsigned char *dataBuff;
    unsigned int  buffSize;
    unsigned int  dataLength;    

    int (*Init)(QMP4_ATOM_STCO *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_STCO *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_STCO *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_STCO *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_STCO *atombox, FILE *file);
    int (*AddData)(QMP4_ATOM_STCO *atombox, unsigned int value);
};

int QMP4AtomSTCO_Init(QMP4_ATOM_STCO *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSTCO_Fini(QMP4_ATOM_STCO *atombox);
int QMP4AtomSTCO_UpdateBoxSize(QMP4_ATOM_STCO *atombox);
int QMP4AtomSTCO_GetBoxAllocSize(QMP4_ATOM_STCO *atombox);
int QMP4AtomSTCO_WriteBoxToFile(QMP4_ATOM_STCO *atombox, FILE *file);
int QMP4AtomSTCO_AddData(QMP4_ATOM_STCO *atombox, unsigned int value);

#define QMP4_ATOM_STCO_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_STCO_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_STCO *)calloc(1, sizeof(QMP4_ATOM_STCO));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_STCO;\
        atombox->Init = QMP4AtomSTCO_Init;\
        atombox->Fini = QMP4AtomSTCO_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSTCO_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSTCO_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSTCO_WriteBoxToFile;\
        atombox->AddData = QMP4AtomSTCO_AddData;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSTCO_H__ */

