#ifndef __QATOMFTYP_H__
#define __QATOMFTYP_H__

#include "mediaformats/qmp4/qatombase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ftyp */
typedef struct _QMP4_ATOM_FTYP QMP4_ATOM_FTYP;
struct _QMP4_ATOM_FTYP {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    char majorBrand[4];
    char minorVersion[4];
    char brand0[4];
    char brand1[4];
    char brand2[4];
    char brand3[4];
    unsigned char headerAlign;

    QMP4_FILE_TYPE filetype;

    int (*Init)(QMP4_ATOM_FTYP *atombox, QMP4_FILE_TYPE type);
    int (*Fini)(QMP4_ATOM_FTYP *atombox);
    int (*UpdateBoxSize)(QMP4_ATOM_FTYP *atombox);
    int (*GetBoxAllocSize)(QMP4_ATOM_FTYP *atombox);
    int (*WriteBoxToFile)(QMP4_ATOM_FTYP *atombox, FILE *file);
};

int QMP4AtomFTYP_Init(QMP4_ATOM_FTYP *atombox, QMP4_FILE_TYPE type);
int QMP4AtomFTYP_Fini(QMP4_ATOM_FTYP *atombox);
int QMP4AtomFTYP_UpdateBoxSize(QMP4_ATOM_FTYP *atombox);
int QMP4AtomFTYP_GetBoxAllocSize(QMP4_ATOM_FTYP *atombox);
int QMP4AtomFTYP_WriteBoxToFile(QMP4_ATOM_FTYP *atombox, FILE *file);

#define QMP4_ATOM_FTYP_DESTRUCT(atombox)\
{\
    if (atombox) {\
        free(atombox);\
        atombox = NULL;\
    }\
}

#define QMP4_ATOM_FTYP_CONSTRUCT(atombox)\
{\
    atombox = (QMP4_ATOM_FTYP *)calloc(1, sizeof(QMP4_ATOM_FTYP));\
    if (atombox) {\
        *((unsigned int *)atombox->extendedBox.boxtype) = FOURCC_FTYP;\
        atombox->Init = QMP4AtomFTYP_Init;\
        atombox->Fini = QMP4AtomFTYP_Fini;\
        atombox->UpdateBoxSize = QMP4AtomFTYP_UpdateBoxSize;\
        atombox->GetBoxAllocSize = QMP4AtomFTYP_GetBoxAllocSize;\
        atombox->WriteBoxToFile = QMP4AtomFTYP_WriteBoxToFile;\
    }\
}

#ifdef __cplusplus
}
#endif

#endif /* __QATOMFTYP_H__ */

