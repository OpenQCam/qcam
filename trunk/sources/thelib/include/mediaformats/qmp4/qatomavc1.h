#ifndef __QATOMAVC1_H__
#define __QATOMAVC1_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomavcc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_AVC1 QMP4_ATOM_AVC1;
struct _QMP4_ATOM_AVC1 {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char reserved0[6];             /* 00 00 00 00 00 00 */
    unsigned char dataReferenceIndex[2];
    unsigned char reserved1[16];            /* 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 */
    unsigned char width[2];
    unsigned char height[2];
    unsigned char reserved2[14];            /* 00 48 00 00 00 48 00 00 00 00 00 00 00 01 */
    unsigned char compressorName[32];
    unsigned char reserved3[4];             /* 00 18 ff ff */
    unsigned char headerAlign;

    QMP4_ATOM_AVCC *avcc;

    int (*Init)(QMP4_ATOM_AVC1 *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_AVC1 *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_AVC1 *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_AVC1 *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_AVC1 *atombox, FILE *file);
};

int QMP4AtomAVC1_Init(QMP4_ATOM_AVC1 *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomAVC1_Fini(QMP4_ATOM_AVC1 *atombox);
int QMP4AtomAVC1_UpdateBoxSize(QMP4_ATOM_AVC1 *atombox);
int QMP4AtomAVC1_GetBoxAllocSize(QMP4_ATOM_AVC1 *atombox);
int QMP4AtomAVC1_WriteBoxToFile(QMP4_ATOM_AVC1 *atombox, FILE *file);

#define QMP4_ATOM_AVC1_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_AVCC_DESTRUCT(atombox->avcc);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_AVC1_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_AVC1 *)calloc(1, sizeof(QMP4_ATOM_AVC1));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_AVC1;\
        atombox->Init = QMP4AtomAVC1_Init;\
        atombox->Fini = QMP4AtomAVC1_Fini;\
        atombox->UpdateBoxSize = QMP4AtomAVC1_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomAVC1_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomAVC1_WriteBoxToFile;\
        QMP4_ATOM_AVCC_CONSTRUCT(atombox->avcc);\
        if (!atombox->avcc) {\
            QMP4_ATOM_AVC1_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMAVC1_H__ */

