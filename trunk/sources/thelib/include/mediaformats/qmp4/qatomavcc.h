#ifndef __QATOMAVCC_H__
#define __QATOMAVCC_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_AVCC QMP4_ATOM_AVCC;
struct _QMP4_ATOM_AVCC {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char configurationVersion; 
    unsigned char profileIndication;
    unsigned char profileCompatibility;
    unsigned char levelIndication; 
    unsigned char lengthSizeMinusOne            : 2;
    unsigned char reserved0                     : 6; 
    unsigned char numOfSequenceParameterSets    : 5;    /* in our case, always 1 */ 
    unsigned char reserved1                     : 3; 
    unsigned char sequenceParameterSetLength[2];
    unsigned char sequenceParameterSetNALUnit[128]; 
    unsigned char numOfPictureParameterSets;            /* in our case, always 1 */ 
    unsigned char pictureParameterSetLength[2];
    unsigned char pictureParameterSetNALUnit[128];
    unsigned char headerAlign;

    int (*Init)(QMP4_ATOM_AVCC *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_AVCC *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_AVCC *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_AVCC *atombox);
    int (*UpdateSPS)(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize);
    int (*UpdatePPS)(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize);
    int (*WriteBoxToFile)(QMP4_ATOM_AVCC *atombox, FILE *file);

};

int QMP4AtomAVCC_Init(QMP4_ATOM_AVCC *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomAVCC_Fini(QMP4_ATOM_AVCC *atombox);
int QMP4AtomAVCC_UpdateBoxSize(QMP4_ATOM_AVCC *atombox);
int QMP4AtomAVCC_GetBoxAllocSize(QMP4_ATOM_AVCC *atombox);
int QMP4AtomAVCC_UpdateSPS(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize);
int QMP4AtomAVCC_UpdatePPS(QMP4_ATOM_AVCC *atombox, unsigned char *packetBuff, unsigned int packetSize);
int QMP4AtomAVCC_WriteBoxToFile(QMP4_ATOM_AVCC *atombox, FILE *file);


#define QMP4_ATOM_AVCC_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_AVCC_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_AVCC *)calloc(1, sizeof(QMP4_ATOM_AVCC));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_AVCC;\
        atombox->Init = QMP4AtomAVCC_Init;\
        atombox->Fini = QMP4AtomAVCC_Fini;\
        atombox->UpdateBoxSize = QMP4AtomAVCC_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomAVCC_GetBoxAllocSize;\
        atombox->UpdateSPS = QMP4AtomAVCC_UpdateSPS;\
        atombox->UpdatePPS = QMP4AtomAVCC_UpdatePPS;\
        atombox->WriteBoxToFile = QMP4AtomAVCC_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMAVCC_H__ */

