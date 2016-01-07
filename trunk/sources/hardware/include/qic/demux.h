/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#ifndef _QICDEMUX_H_
#define _QICDEMUX_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "qic_xuctrl.h"

#define TS_GET_ERR_INDI(x)          (((*(x+1)) & 0x80) >> 7)
#define TS_GET_UNIT_START(x)        (((*(x+1)) & 0x40) >> 6)
#define TS_GET_PID(x)               ((((*(x+1)) & 0x1F) << 8) + (*(x+2)))
#define TS_GET_SCRAMBLE(x)          (((*(x+3)) & 0xC0) >> 6)
#define TS_GET_ADAPT_CTRL(x)        (((*(x+3)) & 0x30) >> 4)
#define TS_GET_CONTI_CNT(x)         ((*(x+3)) & (0xf))
#define TS_OK                       0
#define TS_ERR                      -1
#define PES_OK                      0
#define PES_ERR                     -1
#define PID							0x11
#define TS_PKT_SIZE					188
#define TS_PKT_HDR_LEN				(4)
#define PES_PKT_HDR_LEN				(9)
#define MAXBUFFERCOUNT				5
#define MAXFRAMESIZE				128*1024

#define SYNCWORD	0x30385056

typedef struct
{
	unsigned char data[MAXFRAMESIZE];		//store the raw H.264 bitstream
	int length;						//indicate the data length
	unsigned long  timestamp;
}H264DATAFORMAT;
typedef struct
{
	int length;						//indicate the data length
	unsigned long  timestamp;
}H264DATAFORMAT_IN;;
typedef struct
{
	int ts_counter;	
	int ts_payload_len;
	int pes_payload_len;
	int ts_adapt_len;
	int wait_start;
	int pid;
	int unit_start;
	int pos;
	int byteused;
	unsigned long timestamp;
	unsigned char data[MAXFRAMESIZE];
}Packet_Source;
typedef struct
{
	H264DATAFORMAT_IN  frame;
	int frame_count;		
	int frame_pre_send;
	int bad_count;
	int drop_count;
	int wait_i;
	int not_complete;
	int adopt_recover;
	int maxbuffer;
	Packet_Source usb;	
}info;

//typedef int(*fun_parse) (info* data, unsigned char *src, int size, H264DATAFORMAT **out) ;
//typedef void(*fun_init)(info *data);
typedef struct
{
	info data;
//	fun_init	init;
//	fun_parse parse;	
}demux;

typedef enum _quanta_enum_debug_type_t
{
    DBG_SEI_FRAME_SIZE          = 14,
    DBG_SEI_LAST_FRAME_SIZE     = 47
}quanta_enum_debug_type_t;

typedef struct{
    unsigned int syncword;
    unsigned int payload_length;
}quanta_debug_header_t;


void demux_init(info *data);
void demux_adopt_recover(info *data);
void demux_set_maxbuffer(info *data, int size);
int demux_parse_TS(info *data,unsigned char *src , int size , H264DATAFORMAT **out);
int demux_parse_ES(info *data,unsigned char *src , int size , H264DATAFORMAT **out);
void SetH264format(EncoderStreamFormat_t format);
int demux_check_format( info *data,unsigned char *src , int size);
void demux_VP8_H264_check_bad_frame_initial(void);
int demux_VP8_check_bad_frame( char **src , unsigned int *size);
int demux_H264_check_bad_frame( char *src , unsigned int size);
int demux_H264_SEI2_data(char *src , unsigned int size,unsigned char* output_data, unsigned int *output_data_size);

/*api for h264 simulcast stream*/
int get_avc_stream_id(unsigned char* data, unsigned int data_size);
int get_stream_temporal_id(unsigned char* data, unsigned int data_size);
int check_for_P_frame(unsigned char* data, unsigned int data_size);

#ifdef __cplusplus
}
#endif 

#endif

