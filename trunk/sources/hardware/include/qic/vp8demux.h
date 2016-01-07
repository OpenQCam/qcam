#ifndef __VP8_DEMUX_H__
#define __VP8_DEMUX_H__

#include <stdint.h>


#define BYTE    unsigned char
#define WORD   unsigned short
#define DWORD  unsigned int


#define MAX_FRAME_SIZE	(512*1024)
#define MAX_STREAM_NUM	4

typedef struct{
	// fields in payload header
	DWORD stream_type;
	WORD  width;
	WORD  height;
	WORD  frame_rate_nominator;
	WORD  frame_rate_denominator;
	WORD  delay;
	DWORD presentation_timestamp;

	// vp8 data
	BYTE  data[MAX_FRAME_SIZE];
	DWORD size;
}vp8_frame_t;

typedef struct{
	vp8_frame_t frames[MAX_STREAM_NUM];
}vp8_frames_t;

/**
 * vp8demux - get the vp8 frames embedded in JPEG
 *
 * Parameters
 *		src			the pointer to the JPEG data. 
 *		size		the size of the JPEG data in bytes.
 *		frames		vp8demux set all frames to this parameter.
 *
 * Return Value
 *		The number of frames found is returned. 
 *		-1 is returned if data incorrect.
 */
int vp8demux(BYTE* src, int size, vp8_frames_t* frames);

typedef struct{
    // fields in payload header
    DWORD stream_type;
    WORD  width;
    WORD  height;
    WORD  frame_rate_nominator;
    WORD  frame_rate_denominator;
    WORD  delay;
    DWORD presentation_timestamp;

    // vp8 data
    BYTE* data;
    DWORD size;
}vp8_frame_ex_t;

typedef struct{
    vp8_frame_ex_t frames[MAX_STREAM_NUM];
}vp8_frames_ex_t;

/**
 * vp8demux_ex - get the vp8 frames embedded in JPEG
 *
 * Parameters
 *		src			the pointer to the JPEG data.
 *		size		the size of the JPEG data in bytes.
 *		frames		vp8demux set all frames to this parameter.
 *
 * Return Value
 *		The number of frames found is returned.
 *		-1 is returned if data incorrect.
 *
 * Note
 *      vp8demux_ex() optimize the performance to reduce CPU
 * usage. For the specification limitation, when frame size >
 * 64k bytes, the frame must be split into several APP4 markers.
 * vp8demux merge these data by memcpy.
 *      vp8demux_ex() does following steps to reduce CPU usage.
 * 1. return the payload of first APP4 block to skip first memcpy.
 * 2. copy all following blocks to the tail of APP4 block, note that
 *    this overwrites the following MJPG marker structures.
 */
int vp8demux_ex(BYTE* src, int size, vp8_frames_ex_t* frames);

#endif