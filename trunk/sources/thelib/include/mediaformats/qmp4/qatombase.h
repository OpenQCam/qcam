#ifndef __QATOMBASE_H__
#define __QATOMBASE_H__

#include <stdio.h>
#include <stdlib.h>
#include "mediaformats/qmp4/qatomtype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _QMP4_ATOM_EXTENDED_BOX QMP4_ATOM_EXTENDED_BOX;
struct _QMP4_ATOM_EXTENDED_BOX {
    unsigned char boxsize[4];
    unsigned char boxtype[4];
};

typedef struct _QMP4_ATOM_FULL_BOX QMP4_ATOM_FULL_BOX;
struct _QMP4_ATOM_FULL_BOX {
    QMP4_ATOM_EXTENDED_BOX extendedBox;
    unsigned char version;
    unsigned char flags[3];
};

typedef enum {
    QMP4_TRAK_TYPE_VIDEO_AVC = 0,
    QMP4_TRAK_TYPE_AUDIO_PCM
} QMP4_TRAK_TYPE;

typedef struct _QMP4_TRAK_INFO {
    QMP4_TRAK_TYPE type;
    union {
        struct {
            unsigned int creation_sec_time_since_1904;
            unsigned int sample_time_scale;
            unsigned int max_packets;
            unsigned int track_id;
            unsigned int samples_per_chunk;
            unsigned int reserved0;
            unsigned int reserved1;
            unsigned int reserved2;
        } general;
        struct {
            unsigned int creation_sec_time_since_1904;
            unsigned int sample_time_scale;
            unsigned int max_packets;
            unsigned int track_id;
            unsigned int samples_per_chunk;
            unsigned int width;
            unsigned int height;
            unsigned int reserved2;
        } video;
        struct {
            unsigned int creation_sec_time_since_1904;
            unsigned int sample_time_scale;
            unsigned int max_packets;
            unsigned int track_id;
            unsigned int sample_per_chunk;
            unsigned int channel_num;
            unsigned int reserved1;
            unsigned int reserved2;
        } audio;
    } info;
} QMP4_TRAK_INFO;


#ifdef __cplusplus
}
#endif

#endif /* __QATOMBASE_H__ */

