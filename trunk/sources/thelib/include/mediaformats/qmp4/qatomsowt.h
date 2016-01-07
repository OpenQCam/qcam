#ifndef __QATOMSOWT_H__
#define __QATOMSOWT_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_SOWT QMP4_ATOM_SOWT;
struct _QMP4_ATOM_SOWT {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char reserved0[6];
    unsigned char dataReferenceIndex[2];
    unsigned char version[2];
    unsigned char revisionLevel[2];
    unsigned char vendor[4];
    unsigned char channelCount[2];
    unsigned char sampleSize[2];
    unsigned char predefined0[2];
    unsigned char reserved2[2];
    unsigned char sampleRateLo[2];
    unsigned char sampleRateHi[2];
    unsigned char samplePerPacket[4];
    unsigned char bytesPerPacket[4];
    unsigned char bytesPerFrame[4];
    unsigned char bytesPerSample[4];
    unsigned char predefined1[24];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_SOWT *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_SOWT *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_SOWT *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_SOWT *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_SOWT *atombox, FILE *file);

};

int QMP4AtomSOWT_Init(QMP4_ATOM_SOWT *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomSOWT_Fini(QMP4_ATOM_SOWT *atombox);
int QMP4AtomSOWT_UpdateBoxSize(QMP4_ATOM_SOWT *atombox);
int QMP4AtomSOWT_GetBoxAllocSize(QMP4_ATOM_SOWT *atombox);
int QMP4AtomSOWT_WriteBoxToFile(QMP4_ATOM_SOWT *atombox, FILE *file);


#define QMP4_ATOM_SOWT_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_SOWT_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_SOWT *)calloc(1, sizeof(QMP4_ATOM_SOWT));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_SOWT;\
        atombox->Init = QMP4AtomSOWT_Init;\
        atombox->Fini = QMP4AtomSOWT_Fini;\
        atombox->UpdateBoxSize = QMP4AtomSOWT_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomSOWT_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomSOWT_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMSOWT_H__ */

