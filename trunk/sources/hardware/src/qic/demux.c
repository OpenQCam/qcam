/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/qic/demux.h"
#include "../include/qic/H264Enum.h"


#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef MARVELL_VPRO
#define memcpy arm9_memcpy
#define memset arm9_memset
#endif

static int g_TSflag=0;
static info g_sdata;
static char guid[] = {0x19, 0xA3, 0x39, 0x57, 0x4F, 0xBD, 0x4A, 0x0D, 0xB7, 0xCF, 0xE5, 0xD2, 0xC8, 0xB2, 0xDD, 0x5D};

static char sei2_guid[] = {0x01, 0x08, 0x03, 0x02, 0x01, 0x08, 0x03, 0x02, 0x01, 0x08, 0x03, 0x02, 0x01, 0x08, 0x03, 0x02};
#define Read_Byte(x) *(x)

#define Read_2Byte(x) ((*(x) << 8) + (*((x) + 1)))

#define Read_3Byte(x) ((*(x) << 16) + (*((x)+1) << 8) + (*((x)+2)))

#if 0
unsigned char Read_Byte(unsigned char *src);
unsigned int Read_2Byte(unsigned char *src);
unsigned int Read_3Byte(unsigned char *src);
#endif

int Parse_PES_Header(Packet_Source *src);
int SearchAUD(unsigned char* data, unsigned char dataSize);
int SearchTSPacket(Packet_Source *src);
int Is_TS_PKT(Packet_Source *src);
int _ts_is_pkt_continue(Packet_Source *src);
int _ts_get_header_size(Packet_Source *src);
void PES_Get_Timestamp(Packet_Source *src);

#ifdef CHECK_FRAM_ENUM
static unsigned int prov_stream_num=0;
#endif

static EncoderStreamFormat_t mode;
static unsigned char fw_api_ver;
int last_frame_size;


/*----------------------------------------------------------------------------
 * Functions
 *--------------------------------------------------------------------------*/
void demux_VP8_H264_check_bad_frame_initial(void)
{
	last_frame_size=0;
}

int demux_VP8_check_bad_frame( char **src , unsigned int *size)
{
	int ret=0,expected_size,actual_size;
	unsigned int i;
	quanta_debug_header_t *quanta_debug_header=(quanta_debug_header_t *)(*src);

	//printf("src=0x%lx\n",src);
	//printf("*src=0x%lx\n",*src);
	//printf("**src=0x%lx\n",**src);
/*
	for(i=0;i<quanta_debug_header->payload_length+8;i=i+8){
		printf("0x%x, 0x%x, 0x%x, 0x%x\n",*(*src+i),*(*src+i+1),*(*src+i+2),*(*src+i+3));
		printf("0x%x, 0x%x, 0x%x, 0x%x\n",*(*src+i+4),*(*src+i+5),*(*src+i+6),*(*src+i+7));
	}
*/
	//printf("quanta_debug_header->syncword=0x%x\n",quanta_debug_header->syncword);
	//printf("quanta_debug_header->payload_length=%d\n",quanta_debug_header->payload_length);
	if (quanta_debug_header->syncword == 0x30385056)
	{
		// Parse Data and print message
		for (i=0;i<quanta_debug_header->payload_length;i=i+8){

			switch(*((unsigned int *)(*src + sizeof(quanta_debug_header_t) + i))){

				case DBG_SEI_FRAME_SIZE:
					expected_size = *((unsigned int *)(*src + sizeof(quanta_debug_header_t) + i + 4));
					actual_size = *size - quanta_debug_header->payload_length - sizeof(quanta_debug_header_t);
					//printf("VP8 DBG_SEI_FRAME_SIZE = %d (actual %d)\n", expected_size, actual_size);

					if(!(actual_size >= expected_size && actual_size < expected_size+4)){
						printf("\nError!! VP8 DBG_SEI_FRAME_SIZE check Fail...!!\n");
						ret = 1;
					}
					break;

				case DBG_SEI_LAST_FRAME_SIZE:
					expected_size = *((unsigned int *)(*src + sizeof(quanta_debug_header_t) + i + 4));
				//	printf("VP8 DBG_SEI_LAST_FRAME_SIZE = %d (actual %d)\n", expected_size, last_frame_size);

					if(!(last_frame_size >= expected_size && last_frame_size < expected_size+4) && last_frame_size!=0){
						printf("\nError!! VP8 DBG_SEI_LAST_FRAME_SIZE check Fail...!!\n");
						ret = 1;
					}
					break;

				default:
					//printf("\nUnknown VP8 type (%d) = %d\n", *((unsigned int *)(*src + sizeof(quanta_debug_header_t) +i)),
					//							*((unsigned int *)(*src + sizeof(quanta_debug_header_t) +i + 4)));
					break;
			}
		}

		//printf("*src=0x%lx\n",*src);
		//printf("*size=%d\n",*size);
		*src += (quanta_debug_header->payload_length + sizeof(quanta_debug_header_t));
		*size -= (quanta_debug_header->payload_length  + sizeof(quanta_debug_header_t));
		last_frame_size = *size;
		//printf("*src=0x%lx\n",*src);
		//printf("*size=%d\n",*size);

	}else{
		printf("\nWarning!! VP8 Debug syncword was not found...!!\n");
		printf("Please make sure your camera firmware has support this VP8 bad frame check function!!\n\n");
		ret = 1;
	}

	return ret;
}

int demux_H264_check_bad_frame( char *src , unsigned int size)
{
	int ret=0,expected_size,actual_size;
	unsigned int i;
	char found_sei = 0;

	if(size < 16) return 1;

	/* search for guid */
    for(i=0;i<size-16;i++){
    	//printf("src[%d]=0x%x\n",i,src[i]);
    	if(src[i] == guid[0]){
            if(memcmp(&src[i], guid, sizeof(guid)) == 0){
				//printf("\nH264 SEI was found...!!\n");
                found_sei = 1;
                break;
            }
        }

        if(src[i+0] == 0x00 &&
        	src[i+1] == 0x00 &&
        	src[i+2] == 0x00 &&
        	src[i+3] == 0x01 &&
           ((src[i+4]&0x1F) == 0x05 || (src[i+4]&0x1F) == 0x01)){

        	printf("\nWarning!! H264 Debug GUID was not found...!!\n");
        	printf("Please make sure your camera firmware has support this H264 bad frame check function!!\n");
        	return 1;
        }
    }

    if(found_sei == 0){
		printf("\nError!! H264 SEI was not found...!!\n");
        return 1;
	}

    /* get payload size */
    int payload_size = src[i-1];
    //printf("payload_size=%d, i=%d\n",payload_size,i);
    if(payload_size < 16){
    	printf("\nInvalid debug sei payload\n");
        return 1;
    }
    if(payload_size > size){
    	printf("\ndebug sei payload too large\n");
        return 1;
    }

    /* decode each fields */
    unsigned char *ptr = &src[i+16];
    for(i=0;i<payload_size-16;i++){

        int type = *ptr++;
        int len = type >> 6;
        type = type & 0x3F;

        unsigned int value = 0;
        //printf("len=%d\n",len);
        switch(len)
        {
        case 3:
            value = *ptr++;
            //printf("value3=0x%x\n",value);
            i++;
            // fall through
        case 2:
            value <<=8;
            value |= *ptr++;
            //printf("value2=0x%x\n",value);
            i++;
            // fall through
        case 1:
            value <<=8;
            value |= *ptr++;
            //printf("value1=0x%x\n",value);
            i++;
            // fall through
        case 0:
            value <<=8;
            value |= *ptr++;
            //printf("value0=0x%x\n",value);
            i++;
            // fall through
        }

        switch(type){

        	case DBG_SEI_FRAME_SIZE:
                actual_size = size - payload_size - 8;
                expected_size = value;
                //printf("H264 DBG_SEI_FRAME_SIZE = %d (actual %d)\n", expected_size, actual_size);

                if(!(actual_size >= expected_size && actual_size < expected_size+4)){
                	printf("\nError!! H264 DBG_SEI_FRAME_SIZE check Fail...!!\n");
                	ret = 1;
                }
                break;

        	case DBG_SEI_LAST_FRAME_SIZE:
                expected_size = value;
                //printf("H264 DBG_SEI_LAST_FRAME_SIZE = %d (actual %d)\n", expected_size, last_frame_size);
                if(!(last_frame_size >= expected_size && last_frame_size < expected_size+4) && last_frame_size!=0){
                	printf("\nError!! H264 DBG_SEI_LAST_FRAME_SIZE check Fail...!!\n");
                	ret = 1;
                }
                break;

        	default:
        		if(!type==0x37||!type==1||!type==2||!type==3||!type==4||!type==5||!type==6){
        			printf("\nUnknown H264 type (%d) = %d\n", type, value);
        			ret = 1;
			}
        		break;
        }
    }

    last_frame_size = size - payload_size - 8;

	return ret;
}


int demux_H264_SEI2_data(char *src , unsigned int size,unsigned char* output_data, unsigned int *output_data_size)
{
    int ret=0;
    unsigned int i,j;
    char found_sei = 0;

    if(size < 16) return 1;

    /* search for guid */
    for(i=0;i<size-16;i++){
     //   printf("src[%d]=0x%x\n",i,src[i]);
        if(src[i] == sei2_guid[0]){
            if(memcmp(&src[i], sei2_guid, sizeof(sei2_guid)) == 0){
          //      printf("\nH264 SEI 2 was found...!!\n");
		//printf("i=%d,\n",i);
                found_sei = 1;
                break;
            }
        }

        if(src[i+0] == 0x00 &&
            src[i+1] == 0x00 &&
            src[i+2] == 0x00 &&
            src[i+3] == 0x01 &&
           ((src[i+4]&0x1F) == 0x05 || (src[i+4]&0x1F) == 0x01)){

            printf("\nWarning!! H264 SEI 2 GUID was not found...!!\n");
            printf("Please make sure your camera firmware has support this H264 function!!\n");
            return 1;
        }
    }

    if(found_sei == 0){
        printf("\nError!! H264 SEI 2 was not found...!!\n");
        return 1;
    }

    /* get payload size */
    int payload_size = src[i-1];
 //   printf("payload_size=%d, i=%d\n",payload_size,i);

    if(payload_size < 16){
        printf("\nInvalid debug sei payload\n");
        return 1;
    }
    if(payload_size > size){
        printf("\ndebug sei payload too large\n");
        return 1;
    }

    *output_data_size=payload_size-sizeof(sei2_guid);

    /* decode data field */
    unsigned char *ptr = &src[i+16];

    memcpy((unsigned char*)output_data,(unsigned char*)ptr,payload_size-sizeof(sei2_guid));

#if 0
     for(j=0;j<*output_data_size;j++){
       printf("out[%d]=0x%x\n",j,output_data[j]);
     } 
#endif

    return ret;
}


 EncoderStreamFormat_t GetH264format(void)
{
	return mode;
}

void SetH264format(EncoderStreamFormat_t format)
{
	mode=format;
}

/*
positive return : mean how many frame in this parseing
negative return : mean too many frame in this parseing the receiving buffer is not enough
				  return the unsuffieinet count
*/
int demux_parse_TS( info *data,unsigned char *src , int size , H264DATAFORMAT **out)
{
	int i,j,k;
	int framenum = 0;
	int expected_size;
	int findaud=0;
	/*copy un-complete data to the buffer start*/
	if(data->frame_pre_send)
		memcpy(&out[0]->data,&out[data->frame_pre_send]->data,data->frame.length);
	/*****************merge data**********************/
	memcpy(&data->usb.data[data->usb.pos] , src, size);		
	size += data->usb.pos;
	data->usb.pos = 0;
	data->usb.byteused = size;

	/* PES or TS packet ERROR 
	  find the next TS packet start
	*/
	if(data->usb.wait_start)
	{
		if(SearchTSPacket(&data->usb))
		{
			data->usb.wait_start =0;			
		}
	}
	
	for(i=data->usb.pos ; i < size ;i = data->usb.pos)
	{	
		/*check the final data size
		  if un-parse data is not enough one TS packet
		  buffer it until next handle
		*/
		if(size - i < TS_PKT_SIZE)
		{
			for(j=0;j<(size-i);j++)
			    data->usb.data[j]= data->usb.data[i+j];
			data->usb.pos = size - i;			
			break;
		}
		/***********handle TS header***************/
		/*first check  compatible with TS Packet format
		move the pos to the PES offset
		*/
		if(TS_OK == Is_TS_PKT(&data->usb) )
		{
			/*check the PID is right*/
			if(data->usb.pid == PID)
			{
				/*TS Packe is adapat field only*/
				if(data->usb.ts_adapt_len == 183)
					continue;
				/*check if is TS packe Unit start*/
				/*parse PES header*/
				if(data->usb.unit_start)
				{								
					/*PES header error*/
					if(!PES_OK == Parse_PES_Header(&data->usb))
					{				
						data->bad_count++;			
						data->usb.wait_start =1;
						data->frame.length =0;
						findaud = 0;
						continue;
					}
					/*PES start find aud*/
					else
					{
						if(SearchAUD(&data->usb.data[data->usb.pos] , data->usb.pes_payload_len) == -1 )
							findaud = 0;
						else
							findaud = 1;
					}	
		
				}		
				/*no pes header 
				  pes payload len = ts payload len
				*/
				else 
				{
					data->usb.pes_payload_len = data->usb.ts_payload_len;
					findaud =0;
				}
				/*common PES Packet*/
				if(!findaud)
				{						
					memcpy(&out[framenum]->data[data->frame.length],&data->usb.data[data->usb.pos],data->usb.pes_payload_len);
					data->frame.length += data->usb.pes_payload_len;
					data->usb.pos += data->usb.pes_payload_len;
			
				}
				/*one PES packet complete*/
				else
				{
					/*one previous complet frame */
					if(data->frame.length)
					{
						/*trim the 0*/
						while(out[framenum]->data[data->frame.length -1]==0)
							data->frame.length--;
			
						/*detect firmware additional info to do some recovery*/
						if(data->adopt_recover)
						{
							expected_size = (data->usb.data[data->usb.pos+29]<<16) + (data->usb.data[data->usb.pos+30]<<8) + data->usb.data[data->usb.pos+31];
							/*replace the 32~35 byte to 0xFF for fixed SEI error after got expected_size*/							
							for(k=32;k<36;k++)
							  data->usb.data[data->usb.pos+k]=0xff;

							/*firmware check correct*/
							
							if(data->frame.length >= expected_size && data->frame.length < expected_size + 188*2)
							{
								
								if(out[framenum]->data[5] == 0x30)//P frame
								{
									if(!data->wait_i)
									{
										out[framenum]->length = data->frame.length;
										out[framenum++]->timestamp = data->usb.timestamp;
										data->frame.length =0;
										data->frame_count++;
									}
									else
									{
										data->frame.length =0;
										data->frame_count++;
									}
								}
								else if(out[framenum]->data[5]==0x10)//I frame
								{
									out[framenum]->length = data->frame.length;
									out[framenum++]->timestamp = data->usb.timestamp;
									data->frame.length =0;
									data->frame_count++;
									data->wait_i =0;
								}
								else
								{
									data->wait_i =1;
									data->bad_count++;
									data->drop_count++;
									data->frame.length =0;
								}
					
							}
							/*check error*/
							else
							{						
								data->wait_i =1;
								data->bad_count++;
								data->drop_count++;
								data->frame.length =0;
							 }
						}
						/*no recovery */
						else
						{
							out[framenum]->length = data->frame.length;
							out[framenum++]->timestamp = data->usb.timestamp;
							data->frame.length =0;
							data->frame_count++;
						}
					}

					/*copy processed data to temp frame*/

					memcpy(&out[framenum]->data[data->frame.length] , &data->usb.data[data->usb.pos], data->usb.pes_payload_len);
					data->frame.length = data->usb.pes_payload_len;
					data->usb.pos += data->usb.pes_payload_len;;				
				}
				
			}
			/*PID is not want*/
			else
				data->usb.pos += 183;
		}
		/*TS Header error*/
		else
		{
			data->bad_count++;
			data->drop_count++;			
			data->usb.wait_start =1;
			data->frame.length =0;
			if(SearchTSPacket(&data->usb))
			{
				data->usb.wait_start =0;			
			}

		}		
	}
	if(data->usb.pos >TS_PKT_SIZE)
		data->usb.pos = 0;

	data->frame_pre_send = framenum;
	return framenum;
}

int demux_parse_ES( info *data,unsigned char *src , int size , H264DATAFORMAT **out)
{
	unsigned long ii;
	int framenum = 0;
#ifdef CHECK_FRAM_ENUM
	unsigned int expected_stream_num=0, ser_num_err=0, kk=0;
        unsigned char *frameindex;
#endif
	int expected_size;
	int j=0;
	unsigned long  usedsize=0;
	h264_frame_enum_t handle;
	unsigned long framesize;
	unsigned char *framestart;
	int flag=0;
	
	/*merge pre-data not processed & this one data */
	memcpy(&data->usb.data[data->usb.pos] , src, size);		
	/*true length processing data*/
	data->usb.byteused = size+data->usb.pos;	
	/*always start at 0 offset*/
	h264_frame_enum_init(&handle, &data->usb.data[0], data->usb.byteused );       
	while(1)
	{
		if(h264_frame_enum_next(&handle, &framestart, &framesize)==0)
		{                   
			/*all data send to up-layer*/
			usedsize+=framesize;
			/*check bad frame*/
			expected_size = ( (*(framestart+framesize+29)) <<16) + ( (*(framestart+framesize+30)) <<8) +  (*(framestart+framesize+31)) ;
		//	printf("expected_size=%u,framesize=%u\n",expected_size,framesize);


			if(GetH264format()==FORMAT_STREAM_H264_RAW_1)
			expected_size= (expected_size+3>>2)<<2;

		//	printf("%x, %x, %x, %x, %x\n",*(framestart+13),*(framestart+14),*(framestart+15),*(framestart+16),*(framestart+17));
		if(*(framestart+13)==0x3F&&*(framestart+14)==0x5B&&*(framestart+15)==0xA8&&*(framestart+16)==0x71){
			out[framenum]->timestamp = ( (*(framestart+framesize+32)) <<24) + ( (*(framestart+framesize+33)) <<16) + ( (*(framestart+framesize+34)) <<8) +  (*(framestart+framesize+35)) ;
			/*replace the 32~35 byte to 0xFF for fixed SEI error after got timestamp*/	
			for(j=32;j<36;j++)
			   (*(framestart+framesize+j))=0xff;
		}
		else{
			out[framenum]->timestamp = ( (*(framestart+framesize+33)) <<24) + ( (*(framestart+framesize+34)) <<16) + ( (*(framestart+framesize+36)) <<8) +  (*(framestart+framesize+37)) ;
		}
			
					
#ifdef CHECK_FRAM_ENUM

				frameindex=	framestart+100;	
					while((frameindex[kk]==0))
					kk++;
						
				if(frameindex[kk+1]==0x21)		
				expected_stream_num= ((frameindex[kk+2]&0x1F)<<11) +(frameindex[kk+3]<<3)+((frameindex[kk+4]&0xE0)>>5);
				else if(frameindex[kk+1]==0x25){
				expected_stream_num= ((frameindex[kk+2]&0x07)<<13) +(frameindex[kk+3]<<5)+((frameindex[kk+4]&0xF8)>>3);
					prov_stream_num=0;
				}
				
				if(expected_stream_num){
					if(++prov_stream_num==expected_stream_num){
					//	printf("%u,%u,%u\n",kk,prov_stream_num,expected_stream_num);
					}
					else
						ser_num_err=1;
				}
				else{
					prov_stream_num=0;
				//	printf("%u,%u,%u\n",kk,prov_stream_num,expected_stream_num);
				}
#endif				


			if(GetH264format()==FORMAT_STREAM_H264_RAW_1){
				if((framesize == expected_size)
#ifdef CHECK_FRAM_ENUM
			&&(!ser_num_err)
#endif
			)flag=1;
			}
			else if((GetH264format()==FORMAT_STREAM_H264_RAW_2)||(GetH264format()==FORMAT_STREAM_H264_RAW_3))
			{
				if((framesize >= expected_size && framesize < expected_size + 188*2)
#ifdef CHECK_FRAM_ENUM
			&&(!ser_num_err)
#endif
			)flag=1;
			}
			else
			{
				flag=0;
				printf("===>incorrect format!!!\n");
			}
			
			if(flag)
			{/*copy to up-layer*/
				if(framestart[5]==0x30)//P frame
				{
					if(!data->wait_i)
					{
			                  memcpy(&out[framenum]->data[0],framestart,framesize);
                                          while(out[framenum]->data[framesize-1]==0)framesize--;
			                   out[framenum++]->length = framesize;
		                         }
				}
				else if(framestart[5]==0x10)//I frame
				{
					data->wait_i =0;
			                memcpy(&out[framenum]->data[0],framestart,framesize);
                                        while(out[framenum]->data[framesize-1]==0)framesize--;
			                out[framenum++]->length = framesize;

				}
			}
			/*bad frame*/
			else
			{
				data->bad_count++;
				data->drop_count++;
				data->wait_i =1;
			}               
		}
		else
		{
			/*copy not handle data to the begin*/	
			data->usb.pos = data->usb.byteused -usedsize;

		       for(ii=0;ii<data->usb.pos;ii++)
		       {
                          data->usb.data[ii]=data->usb.data[usedsize+ii];
			   
		       }			
			break;
		}
	}
	return framenum;
			
}

int demux_check_format( info *data,unsigned char *src , int size )
{

	if(!g_TSflag){

	 	memcpy(&g_sdata,data, sizeof(info));
		/*merge pre-data not processed & this one data */
	memcpy(&g_sdata.usb.data[g_sdata.usb.pos] , src, size);		
	/*true length processing data*/
	g_sdata.usb.byteused = size+g_sdata.usb.pos;	
	/*always start at 0 offset*/

		if(SearchTSPacket(&g_sdata.usb)){
			printf("Is TSPacket? YES!!\n");
			g_TSflag=1;
		}
		else{
			printf("Is TSPacket? NO!!\n");
			g_TSflag=2;
		}
			
	}


    return g_TSflag;
			
}

int Parse_PES_Header(Packet_Source *src)
{
	
	int	packet_start_code_prefix;
	int stream_id;
	int PES_packet_length;
	int	discard_info;
	int	flags;
	int hdr_size;
	packet_start_code_prefix = Read_3Byte(&src->data[src->pos]);
	src->pos +=3;
	stream_id = Read_Byte(&src->data[src->pos]);
	src->pos++;
	PES_packet_length = Read_2Byte(&src->data[src->pos]);
	src->pos +=2;
	discard_info = Read_Byte(&src->data[src->pos]);
	src->pos++;
	flags = Read_Byte(&src->data[src->pos]);
	src->pos++;
	hdr_size = Read_Byte(&src->data[src->pos]);
	src->pos ++;
	/*to retrieve the timestamp*/
	PES_Get_Timestamp(src);
	src->pos += hdr_size;		//skip the header data
	

	if (hdr_size >= src->ts_payload_len)
	{
		return PES_ERR;
	}
	if (packet_start_code_prefix != 0x1)
	{
		return PES_ERR;
	}
	/*
	if( discard_info!=132 )
	{
		return PES_ERR;
	}
	*/
/*	if (one_zero != 0x2)
	{
		return PES_ERR;
	}
	if (PES_scrambling_control != 0)
	{
		return PES_ERR;
	}*/
	if ((flags & 0x3F) != 0)
	{
		return PES_ERR;
	}
	src->pes_payload_len = src->ts_payload_len -hdr_size -PES_PKT_HDR_LEN;

	return PES_OK;
}
#if 0
unsigned char Read_Byte(unsigned char *src)
{
	unsigned char ret;
	ret = (unsigned char )*src;
	return ret;
}

unsigned int Read_2Byte(unsigned char *src)
{
	unsigned int ret ;
	ret = Read_Byte(src);
	ret <<=8;
	ret |=Read_Byte(src+1);
	return ret;
}
unsigned int  Read_3Byte(unsigned char *src)
{
	unsigned int ret ;
	ret = Read_Byte(src);
	ret <<=8;
	ret |=Read_Byte(src+1);
	ret <<=8;
	ret |=Read_Byte(src+2);
	return ret;
}
#endif

int SearchAUD(unsigned char* data, unsigned char dataSize)
{

	/* use finite state machine to search AUD,
	 * this can reduce memory access to once per char
	*/
	unsigned char offset = 0;
	int state = 0;
	static const char fsm[4][6] = 
	{
		{1, 2, 3, 3, 0, 0},		// current char is 0
		{0, 0, 0, 4, 0, 0},		// current char is 1
		{0, 0, 0, 0, 5, 0},		// current char is 9
		{0, 0, 0, 0, 0, 0}		// current char is other
	};
		if(dataSize < 6)
		return -1;
	while(offset <= dataSize)
	{
		switch(data[offset++])
		{
		case 0:
			state = fsm[0][state];	break;
		case 1:
			state = fsm[1][state];	break;
		case 9:
			state = fsm[2][state];	break;
		default:
			state = fsm[3][state];	break;
		}
		if(state == 5)
			return offset-5;
		}
	return -1;
}

int Is_TS_PKT(Packet_Source *src)
{
	src->pid = TS_GET_PID(&src->data[src->pos]);
	src->unit_start =TS_GET_UNIT_START(&src->data[src->pos]);
	if (src->data[src->pos] != 0x47)
	{							/* Sync byte is missing. */
		return TS_ERR;
	}
	else if ((src->data[src->pos+1] & 0x80) >> 7)
	{							/* Transport error indicator is set. */
		return TS_ERR;
	}
	else if (TS_GET_SCRAMBLE (&src->data[src->pos]))
	{							/* TS packet is scrambled */
		return TS_ERR;
	}
	else if (TS_OK != _ts_is_pkt_continue (src) )
	{							/* TS packet is not continuous */
		return TS_ERR;
	}
	else if (TS_OK != _ts_get_header_size (src) )
	{							
		return TS_ERR;
	}
	else
	{	
		return TS_OK;
	}
}
int _ts_is_pkt_continue(Packet_Source *src)
{
	int cur_count;
	if(src->pid != PID)
		return TS_OK;
	cur_count = TS_GET_CONTI_CNT (&src->data[src->pos]);
	if(TS_GET_UNIT_START(&src->data[src->pos]))
	{
		src->ts_counter = cur_count;
		return TS_OK;
	}
	else if(cur_count == src->ts_counter)
	{		
		return TS_ERR;
	}
	else if (cur_count != ((src->ts_counter + 1) & 0xF))
	{	
		return TS_ERR;
	}
	else 
	{
		src->ts_counter = cur_count;
		return TS_OK;
	}
}

int _ts_get_header_size (Packet_Source *src )
{
	int             adapt_ctrl;
	unsigned char   adapt_len = src->data[src->pos+TS_PKT_HDR_LEN];

	adapt_ctrl = TS_GET_ADAPT_CTRL (&src->data[src->pos]);

	switch (adapt_ctrl)
	{
		//payload only
	case 0x01:	
		src->ts_adapt_len = 0;
		src->ts_payload_len =183;	
		src->pos += (TS_PKT_HDR_LEN+1);
		return TS_OK;
		//adaptation field only
	case 0x2:
		if (adapt_len != 183)
		{
			return TS_ERR;
		}
		else 
		{
			src->ts_payload_len =0;		
			src->ts_adapt_len = 183;
			src->pos += TS_PKT_SIZE;
			return TS_OK;
		}
		//adapt field and payload
	case 0x3:
		if (adapt_len >= 183)
		{
			return TS_ERR;
		}		
		else
		{
			src->ts_adapt_len =adapt_len;
			src->ts_payload_len = 183- adapt_len;		
			src->pos +=(adapt_len+TS_PKT_HDR_LEN+1);
			return TS_OK;
		}
	default:		
		src->ts_payload_len =0;		
		return TS_ERR;
	}	
}

int SearchTSPacket(Packet_Source *src)
{
	int state=0;
	static const char table[4][3] =
	{
		{1,1,1},
		{0,2,0},
		{0,0,3},
		{0,0,0}
	};
	while(src->pos <= src->byteused)
	{
		switch(src->data[src->pos++])
		{
		case 0x47:
			state = table[0][state];		
			break;
		case 0x40:
			state = table[1][state];
			break;
		case 0x11:
			state = table[2][state];
			break;
		default:
			state = table[3][state];			
			break;			
		}
		if(state == 3)
		{
			src->pos -=3;
			return 1;	
		}
	}
	return 0;
}

void PES_Get_Timestamp(Packet_Source *src)
{

	src->timestamp =0;
	src->timestamp = ( (src->data[src->pos]&0xF) >>1);
	src->timestamp = (src->timestamp<<8) | (src->data[src->pos+1]);
	src->timestamp = (src->timestamp<<7) | (src->data[src->pos+2]>>1);
	src->timestamp = (src->timestamp<<8) | (src->data[src->pos+3]);
	src->timestamp = (src->timestamp<<7) | (src->data[src->pos+4]>>1);
}

void demux_adopt_recover(info *data)
{
	data->adopt_recover =1;
}

void demux_set_maxbuffer(info *data, int size)
{
	data->maxbuffer = size;
}

void demux_init(info *data)
{
	memset(data,0,sizeof(info));
	data->maxbuffer = MAXBUFFERCOUNT;
}



int decode_pps_id(unsigned char* data)
{
        unsigned int buffer;
        int leading_zero;

        buffer = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | (data[3]<<0);


        /* skip first_mb_in_slice */
        leading_zero = 0;
        while(!(buffer & 0x80000000))
        {
                leading_zero++;
                buffer <<=1;

        }
        buffer <<= (1+leading_zero);

        /* skip slice_type */
        leading_zero = 0;
        while(!(buffer & 0x80000000))
        {
                leading_zero++;
                buffer <<=1;

        }
        buffer <<= (1+leading_zero);
        
        /* extract pps_id */
        leading_zero = 0;
        while(!(buffer & 0x80000000))
        {
                leading_zero++;
                buffer <<=1;
        }
        buffer <<= 1;
        if(leading_zero == 0)
                buffer = 0;
        else
                buffer >>= (32-leading_zero);
		
        return (1<<leading_zero) + buffer - 1;
}


int get_avc_stream_id(unsigned char* data, unsigned int data_size){
        /* search for slice header */
	int pps_id;	
	unsigned int i; 	
        unsigned char* tmp = data;
		
        for(i=0;(i+9)<data_size;i++)
        {

                if(data[i+0] == 0x00 &&
                   data[i+1] == 0x00 &&
                   data[i+2] == 0x00 &&
                   data[i+3] == 0x01 &&
                   ((data[i+4]&0x1F) == 0x05 || (data[i+4]&0x1F) == 0x01))
                {

                        /* decode pps id from slice header */
                        pps_id = decode_pps_id(&data[i+5]);
                        break;
                }
        }

	return pps_id;	
}


int get_stream_temporal_id(unsigned char* data, unsigned int data_size){
        /* search for slice header */
	int temporal_id=0;		
int i;
    for(i=0;i<data_size-16;i++){
    
   
        
         if(data[i+0] == 0x00 &&
                   data[i+1] == 0x00 &&
                   data[i+2] == 0x00 &&
                   data[i+3] == 0x01 &&
                   (data[i+4]&0x0F) == 0x0E &&
                   (data[i+5]&0x80) == 0x80 )
                {
                
                     temporal_id =((data[i+7]&0xE0))>>5;
                     break;
                }
    }

    
     return temporal_id;
     
     

#if 0
       if(data_size>9){
			//printf("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",data[4],data[5],data[6],data[7],data[8]);
	
                if(data[0] == 0x00 &&
                   data[1] == 0x00 &&
                   data[2] == 0x00 &&
                   data[3] == 0x01 &&
                   (data[4]&0x0F) == 0x0E &&
                   (data[5]&0x80) == 0x80 )
                {

                        /* decode temporal_id id from p-slice prefix nal header */
                        temporal_id =((data[7]&0xE0))>>5;
                        
                }else{
						temporal_id=0;
                }
        }

	return temporal_id;	
#endif
	
}


int check_for_P_frame(unsigned char* data, unsigned int data_size){
        /* search for slice header */
	int is_p_frame=0;		

int i;
char found_i=0;

  //   printf("==>0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X \n",data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);

    for(i=0;i<data_size-5;i++){
     
           if(data[i+0] == 0x00 &&
                   data[i+1] == 0x00 &&
                   data[i+2] == 0x00 &&
                   data[i+3] == 0x01 &&
                  (((data[i+4]&0x1F) == 0x05)||((data[i+4]&0x1F) == 0x07)||((data[i+4]&0x1F) == 0x08)))
                {
                     if(data[i+0] != 0x00 ||data[i+1] != 0x00 ||data[i+2] != 0x00 )
		     printf("I=0x%x,0x%x,0x%x,0x%x, i=%d, size=%d\n",data[1],data[i+1],data[i+2],data[i+4],i, data_size);	
                 //    printf("I=0x%x\n",data[i+4]);
		     is_p_frame=0;
		     found_i=1;
                     break;
               }      
     
   
            
         if(found_i==0 &&
	           data[i+0] == 0x00 &&
                   data[i+1] == 0x00 &&
                   data[i+2] == 0x00 &&
                   data[i+3] == 0x01 &&
                   (((data[i+4]&0x1F) == 0x01 )))
                {
			if(data[i+0] != 0x00 ||data[i+1] != 0x00 ||data[i+2] != 0x00 )
                 	printf("P=0x%x,0x%x,0x%x,0x%x, i=%d, size=%d\n",data[1],data[i+1],data[i+2],data[i+4],i, data_size);	
			
		//	printf("P=0x%x",data[i+4]);
                     is_p_frame=1;
                     break;
                }
    }

if(!found_i && !is_p_frame) {
    is_p_frame=2;
   printf("Unknow Type:: data_size=%d",data_size);

}


#if 0
			if(data_size>9){

                if(data[0] == 0x00 &&
                   data[1] == 0x00 &&
                   data[2] == 0x00 &&
                   data[3] == 0x01 &&
                   ((data[4]&0x0F) == 0x0E )||(data[4]==0x21))
                {
                	//	printf("P=0x%x\n",data[4]);
					is_p_frame=1;
                     
                }else{
			//		printf("not P=0x%x\n",data[4]);

                }

			}
       
#endif



    //   printf("p frame=%d\n",is_p_frame);

	return is_p_frame;	
}



