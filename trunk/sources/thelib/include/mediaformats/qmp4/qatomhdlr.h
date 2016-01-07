#ifndef __QATOMHDLR_H__
#define __QATOMHDLR_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    QMP4_HDLR_TYPE_MEDIA_HANDLER = 0,
    QMP4_HDLR_TYPE_DATA_HANDLER
} QMP4_HDLR_TYPE;

typedef struct _QMP4_ATOM_HDLR QMP4_ATOM_HDLR;
struct _QMP4_ATOM_HDLR {
    QMP4_ATOM_FULL_BOX fullBox;
    unsigned char preDefined[4];
    unsigned char handlerType[4];
    unsigned char reserved0[4*3];
    unsigned char name[1];
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    int (*Init)(QMP4_ATOM_HDLR *atombox, QMP4_TRAK_INFO *trakInfo, QMP4_HDLR_TYPE handlerType);
    int (*Fini)(QMP4_ATOM_HDLR *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_HDLR *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_HDLR *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_HDLR *atombox, FILE *file);
};

int QMP4AtomHDLR_Init(QMP4_ATOM_HDLR *atombox, QMP4_TRAK_INFO *trakInfo, QMP4_HDLR_TYPE handlerType);
int QMP4AtomHDLR_Fini(QMP4_ATOM_HDLR *atombox);
int QMP4AtomHDLR_UpdateBoxSize(QMP4_ATOM_HDLR *atombox);
int QMP4AtomHDLR_GetBoxAllocSize(QMP4_ATOM_HDLR *atombox);
int QMP4AtomHDLR_WriteBoxToFile(QMP4_ATOM_HDLR *atombox, FILE *file);

#define QMP4_ATOM_HDLR_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_HDLR_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_HDLR *)calloc(1, sizeof(QMP4_ATOM_HDLR));\
    if (atombox) {\
        *((unsigned int *)atombox->fullBox.extendedBox.boxtype) = FOURCC_HDLR;\
        atombox->Init = QMP4AtomHDLR_Init;\
        atombox->Fini = QMP4AtomHDLR_Fini;\
        atombox->UpdateBoxSize = QMP4AtomHDLR_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomHDLR_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomHDLR_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMHDLR_H__ */

