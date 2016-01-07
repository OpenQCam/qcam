/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

// H264FrameEnum.cpp : Defines the entry point for the console application.
//

#include "../include/qic/H264Enum.h"

void h264_frame_enum_init(h264_frame_enum_t* handle, const unsigned char* data, unsigned long data_size)
{
	handle->data = (unsigned char*)data;
	handle->data_size = data_size;
	handle->pos = (unsigned char*)data;
}

/*
 * return next H264 frame via pData and pSize.
 * return 
 *		0: success
 *		-1: not found
 */
int  h264_frame_enum_next(h264_frame_enum_t* handle, unsigned char** pData, unsigned long* pSize)
{
	static const char fsm[5][12] = 
	{
	//   0   1   2   3   4   5   6   7   8   9  10  11 
		{1,  2,  3,  3,  0,  0,  7,  8,  9,  9,  6,  6,},	// 0x00
		{0,  0,  0,  4,  0,  0,  6,  6,  6, 10,  6,  6,},	// 0x01
		{0,  0,  0,  0,  5,  0,  6,  6,  6,  6, 11,  6,},	// 0x09
		{0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6, 12,},	// 0x10 or 0x30
		{0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6,},	// other
	} ;

	unsigned char* end_pos = handle->data + handle->data_size;
	unsigned char* frame_start=0;
	int state = 0;

	while(handle->pos < end_pos)
	{
		if(state == 0)
			frame_start = handle->pos;

		switch(*handle->pos)
		{
		case 0:
			state = fsm[0][state]; break;
		case 1:
			state = fsm[1][state]; break;
		case 9:
			state = fsm[2][state]; break;
		case 0x10:
		case 0x30:
			state = fsm[3][state]; break;
		default:
			state = fsm[4][state]; break;
		}

		if(state == 12)
		{
			*pData = frame_start;
			*pSize = (unsigned long)(handle->pos - frame_start - 5);
			handle->pos -= 5;
			return 0;
		}

		handle->pos++;
	}

	return -1;
}
