#ifndef __QATOMMINF_H__
#define __QATOMMINF_H__

#include "mediaformats/qmp4/qatombase.h"
#include "mediaformats/qmp4/qatomvmhd.h"
#include "mediaformats/qmp4/qatomsmhd.h"
#include "mediaformats/qmp4/qatomhdlr.h"
#include "mediaformats/qmp4/qatomdinf.h"
#include "mediaformats/qmp4/qatomstbl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_MINF QMP4_ATOM_MINF;
struct _QMP4_ATOM_MINF {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char headerAlign;

    QMP4_TRAK_TYPE trakType;

    QMP4_ATOM_VMHD *vmhd;
    QMP4_ATOM_SMHD *smhd;
    QMP4_ATOM_HDLR *hdlr;
    QMP4_ATOM_DINF *dinf;
    QMP4_ATOM_STBL *stbl;

    int (*Init)(QMP4_ATOM_MINF *atombox, QMP4_TRAK_INFO *trakInfo);
    int (*Fini)(QMP4_ATOM_MINF *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_MINF *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_MINF *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_MINF *atombox, FILE *file);
};

int QMP4AtomMINF_Init(QMP4_ATOM_MINF *atombox, QMP4_TRAK_INFO *trakInfo);
int QMP4AtomMINF_Fini(QMP4_ATOM_MINF *atombox);
int QMP4AtomMINF_UpdateBoxSize(QMP4_ATOM_MINF *atombox);
int QMP4AtomMINF_GetBoxAllocSize(QMP4_ATOM_MINF *atombox);
int QMP4AtomMINF_WriteBoxToFile(QMP4_ATOM_MINF *atombox, FILE *file);

#define QMP4_ATOM_MINF_DESTRUCT(atombox)\
{\
    if (atombox) {\
        QMP4_ATOM_VMHD_DESTRUCT(atombox->vmhd);\
        QMP4_ATOM_SMHD_DESTRUCT(atombox->smhd);\
        QMP4_ATOM_HDLR_DESTRUCT(atombox->hdlr);\
        QMP4_ATOM_DINF_DESTRUCT(atombox->dinf);\
        QMP4_ATOM_STBL_DESTRUCT(atombox->stbl);\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_MINF_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_MINF *)calloc(1, sizeof(QMP4_ATOM_MINF));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_MINF;\
        atombox->Init = QMP4AtomMINF_Init;\
        atombox->Fini = QMP4AtomMINF_Fini;\
        atombox->UpdateBoxSize = QMP4AtomMINF_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomMINF_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomMINF_WriteBoxToFile;\
        QMP4_ATOM_VMHD_CONSTRUCT(atombox->vmhd);\
        QMP4_ATOM_SMHD_CONSTRUCT(atombox->smhd);\
        QMP4_ATOM_HDLR_CONSTRUCT(atombox->hdlr);\
        QMP4_ATOM_DINF_CONSTRUCT(atombox->dinf);\
        QMP4_ATOM_STBL_CONSTRUCT(atombox->stbl);\
        if ((!atombox->vmhd) || (!atombox->smhd) || (!atombox->hdlr) || (!atombox->dinf) || (!atombox->stbl)) {\
            QMP4_ATOM_VMHD_DESTRUCT(atombox->vmhd);\
            QMP4_ATOM_SMHD_DESTRUCT(atombox->smhd);\
            QMP4_ATOM_HDLR_DESTRUCT(atombox->hdlr);\
            QMP4_ATOM_DINF_DESTRUCT(atombox->dinf);\
            QMP4_ATOM_STBL_DESTRUCT(atombox->stbl);\
            QMP4_ATOM_MINF_DESTRUCT(atombox);\
        }\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMMINF_H__ */

