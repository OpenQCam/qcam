/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#pragma once

typedef struct
{
	unsigned char* data;
	unsigned long  data_size;
	unsigned char* pos;
   unsigned long lasttime;
}h264_frame_enum_t;

void h264_frame_enum_init(h264_frame_enum_t* handle, const unsigned char* data, unsigned long data_size);

/*
 * return next H264 frame via pData and pSize.
 * return 
 *		0: success
 *		-1: not found
 */
int  h264_frame_enum_next(h264_frame_enum_t* handle, unsigned char** pData, unsigned long* pSize);