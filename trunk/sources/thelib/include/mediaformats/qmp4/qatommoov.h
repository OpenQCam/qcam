#ifndef __QATOMMOOV_H__
#define __QATOMMOOV_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatommvhd.h"
#include "mediaformats/qmp4/qatomtrak.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MOOV QMP4_ATOM_MOOV;
struct _QMP4_ATOM_MOOV {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    unsigned int hasAudioTrak;

    QMP4_ATOM_MVHD *mvhd;
    QMP4_ATOM_TRAK *vidtrak;
    QMP4_ATOM_TRAK *audtrak;

    int (*Init)(QMP4_ATOM_MOOV *atombox, 
                QMP4_TRAK_INFO *vidTrakInfo,
                QMP4_TRAK_INFO *audTrakInfo);
    int (*Fini)(QMP4_ATOM_MOOV *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MOOV *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MOOV *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_MOOV *atombox, FILE *file);
};

int QMP4AtomMOOV_Init(QMP4_ATOM_MOOV *atombox, 
                      QMP4_TRAK_INFO *vidTrakInfo,
                      QMP4_TRAK_INFO *audTrakInfo);
int QMP4AtomMOOV_Fini(QMP4_ATOM_MOOV *atombox);
int QMP4AtomMOOV_UpdateBoxSize(QMP4_ATOM_MOOV *atombox);
int QMP4AtomMOOV_GetBoxAllocSize(QMP4_ATOM_MOOV *atombox);
int QMP4AtomMOOV_WriteBoxToFile(QMP4_ATOM_MOOV *atombox, FILE *file);

#define QMP4_ATOM_MOOV_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_MVHD_DESTRUCT(atombox->mvhd);\
        QMP4_ATOM_TRAK_DESTRUCT(atombox->vidtrak);\
        QMP4_ATOM_TRAK_DESTRUCT(atombox->audtrak);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MOOV_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MOOV *)calloc(1, sizeof(QMP4_ATOM_MOOV));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_MOOV;\
        atombox->Init = QMP4AtomMOOV_Init;\
        atombox->Fini = QMP4AtomMOOV_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMOOV_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMOOV_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomMOOV_WriteBoxToFile;\
        QMP4_ATOM_MVHD_CONSTRUCT(atombox->mvhd);\
        QMP4_ATOM_TRAK_CONSTRUCT(atombox->vidtrak);\
        QMP4_ATOM_TRAK_CONSTRUCT(atombox->audtrak);\
        if ((!atombox->mvhd) || (!atombox->vidtrak) || (!atombox->audtrak)) {\
            QMP4_ATOM_MVHD_DESTRUCT(atombox->mvhd);\
            QMP4_ATOM_TRAK_DESTRUCT(atombox->vidtrak);\
            QMP4_ATOM_TRAK_DESTRUCT(atombox->audtrak);\
            QMP4_ATOM_MOOV_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMOOV_H__ */

