#ifndef __QATOMTYPE_H__
#define __QATOMTYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GET_FIELD_OFFSET
#define GET_FIELD_OFFSET(type, field)   ((long)&(((type *)0)->field))
#endif 

#ifndef GET_FIELD_SIZE
#define GET_FIELD_SIZE(type, field)     ((long)sizeof(((type *)0)->field))
#endif 

#define QMP4_SWAP32(ch) \
    ((((unsigned int)(ch) & 0xFF) << 24) | \
     (((unsigned int)(ch) & 0xFF00) << 8) | \
     (((unsigned int)(ch) & 0xFF0000) >> 8) | \
     (((unsigned int)(ch) & 0xFF000000) >> 24))

#define QMP4_SWAP16(ch) \
    ((((unsigned short)(ch) & 0xFF) << 8) | \
     (((unsigned short)(ch) & 0xFF00) >> 8))

#define MAKE_FOURCC(ch0, ch1, ch2, ch3) \
    ((unsigned int)(unsigned char)(ch0) | \
    ((unsigned int)(unsigned char)(ch1) << 8) | \
    ((unsigned int)(unsigned char)(ch2) << 16) | \
    ((unsigned int)(unsigned char)(ch3) << 24))

#define FOURCC_QT   MAKE_FOURCC('q', 't', ' ', ' ')
#define FOURCC_CAEP MAKE_FOURCC('C', 'A', 'E', 'P')
#define FOURCC_ISOM MAKE_FOURCC('i', 's', 'o', 'm')
#define FOURCC_ISO2 MAKE_FOURCC('i', 's', 'o', '2')
#define FOURCC_AVC1 MAKE_FOURCC('a', 'v', 'c', '1')
#define FOURCC_MP41 MAKE_FOURCC('m', 'p', '4', '1')

#define FOURCC_AVCC MAKE_FOURCC('a', 'v', 'c', 'C')
#define FOURCC_URL  MAKE_FOURCC('u', 'r', 'l', ' ')

#define FOURCC_SMHD MAKE_FOURCC('s', 'm', 'h', 'd')
#define FOURCC_SOWT MAKE_FOURCC('s', 'o', 'w', 't')
#define FOURCC_CHAN MAKE_FOURCC('c', 'h', 'a', 'n')

#define FOURCC_FTYP MAKE_FOURCC('f', 't', 'y', 'p')
#define FOURCC_MOOV MAKE_FOURCC('m', 'o', 'o', 'v')
#define FOURCC_MVHD MAKE_FOURCC('m', 'v', 'h', 'd')
#define FOURCC_TRAK MAKE_FOURCC('t', 'r', 'a', 'k')
#define FOURCC_TKHD MAKE_FOURCC('t', 'k', 'h', 'd')
#define FOURCC_MDIA MAKE_FOURCC('m', 'd', 'i', 'a')
#define FOURCC_MDHD MAKE_FOURCC('m', 'd', 'h', 'd')
#define FOURCC_HDLR MAKE_FOURCC('h', 'd', 'l', 'r')
#define FOURCC_MINF MAKE_FOURCC('m', 'i', 'n', 'f')
#define FOURCC_VMHD MAKE_FOURCC('v', 'm', 'h', 'd')
#define FOURCC_DINF MAKE_FOURCC('d', 'i', 'n', 'f')
#define FOURCC_DREF MAKE_FOURCC('d', 'r', 'e', 'f')
#define FOURCC_URL  MAKE_FOURCC('u', 'r', 'l', ' ')
#define FOURCC_STBL MAKE_FOURCC('s', 't', 'b', 'l')
#define FOURCC_STSD MAKE_FOURCC('s', 't', 's', 'd')
#define FOURCC_STSC MAKE_FOURCC('s', 't', 's', 'c')
#define FOURCC_STCO MAKE_FOURCC('s', 't', 'c', 'o')
#define FOURCC_STTS MAKE_FOURCC('s', 't', 't', 's')
#define FOURCC_STSS MAKE_FOURCC('s', 't', 's', 's')
#define FOURCC_STSZ MAKE_FOURCC('s', 't', 's', 'z')
#define FOURCC_FREE MAKE_FOURCC('f', 'r', 'e', 'e')
#define FOURCC_MDAT MAKE_FOURCC('m', 'd', 'a', 't')

typedef enum {
    QMP4_FILE_TYPE_MP4 = 0,
    QMP4_FILE_TYPE_MOV
} QMP4_FILE_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* __QATOMTYPE_H__ */

