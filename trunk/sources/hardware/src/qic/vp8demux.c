#include "../include/qic/vp8demux.h"
#include <stdio.h>
#include <string.h>

struct jpeg_parser{
	BYTE* data;
	int size;
};

static int jpeg_parser_get_marker(struct jpeg_parser* parser, BYTE** marker, int* marker_length)
{
	int size;

	*marker = 0;
	*marker_length = 0;

	if(parser->size < 4)
	{
		printf("ERROR: JPEG size < 4 bytes");
		return 0;
	}

	/* check if it is marker */
	if(parser->data[0] != 0xFF || parser->data[1]== 0x00 || parser->data[1]== 0xFF)
		return 0;

	/* SOI and EOI don't have length */
	if(parser->data[0] == 0xFF && (parser->data[1]== 0xD8 || parser->data[1]== 0xD9))
	{
		*marker = &parser->data[0];
		*marker_length = 2;
		parser->data += 2;
		parser->size -= 2;
		return 1;
	}

	/* return marker */
	size = (parser->data[2]<<8) | (parser->data[3]<<0);
	if(size + 2 > parser->size)
	{
		printf("ERROR: marker size invalid");
		return 0;
	}
	*marker = parser->data;
	*marker_length = size + 2;

	/* update parser */
	parser->data += size + 2;
	parser->size -= size + 2;

	return 1;
}

static int  jpeg_parser_get_APP4_marker(struct jpeg_parser* handle, BYTE** marker, int* marker_length)
{

	while(jpeg_parser_get_marker(handle, marker, marker_length))
	{
		if((*marker)[0] == 0xFF && (*marker)[1] == 0xE4)
		{
			/* extract payload */
			(*marker) += 4;
			(*marker_length) -= 4;
			return 1;
		}
	}

	return 0;
}

static int  vp8_paylod_parser(struct jpeg_parser* handle, BYTE* out_buffer, BYTE** header)
{
	BYTE* marker;
	int marker_length;

	/* get first APP4 marker */
	if(jpeg_parser_get_APP4_marker(handle, &marker, &marker_length) == 0)
	{
		printf("ERROR: jpeg_parser_get_APP4_marker");
		return -1;
	}
	if(marker_length < 26)
	{
		printf("ERROR: APP4 marker size shall > 26");
		return -1;
	}
	*header = marker;

	/* parse payload header to get size */
	int version = (marker[0]<<8) | marker[1];
	int header_size = (marker[2]<<8) | marker[3];
	int payload_size = (marker[header_size]<<24) | (marker[header_size+1]<<16) | (marker[header_size+2]<<8) | (marker[header_size+3]<<0);

	if(version != 0x0100)
	{
		/* old format */
		return -1;
	}
	if(payload_size > MAX_FRAME_SIZE)
	{
		printf("ERROR: frame too large");
		return -1;
	}

	/* memcpy first block */
	int received_size = marker_length - header_size - 4;
	memcpy(out_buffer, &marker[header_size+4], received_size);

	while(received_size < payload_size)
	{
		/* get next APP4 marker */
		if(jpeg_parser_get_APP4_marker(handle, &marker, &marker_length) == 0)
		{
			printf("ERROR: received data size(%d) < expected size(%d)", received_size, payload_size);
			return -1;
		}

		/* memcpy following block */
		memcpy(out_buffer + received_size, marker, marker_length);
		received_size += marker_length;
	}

	if(received_size != payload_size)
	{
		printf("ERROR: received data size(%d) != expected size(%d)", received_size, payload_size);
		return -1;
	}

	return payload_size;
}

int vp8demux(BYTE* src, int size, vp8_frames_t* frames)
{
	int i;
	struct jpeg_parser parser = {src, size};

	memset(frames, 0, sizeof(vp8_frames_t));

	for(i=0;i<MAX_STREAM_NUM;i++)
	{
		BYTE* header;

		if(vp8_paylod_parser(&parser, frames->frames[i].data, &header) < 0){
			return i;
		}
		frames->frames[i].stream_type = (header[4]<<24)|(header[5]<<16)|(header[6]<<8)|(header[7]<<0);
		frames->frames[i].width = (header[8]<<8)|(header[9]<<0);
		frames->frames[i].height = (header[10]<<8)|(header[11]<<0);
		frames->frames[i].frame_rate_nominator = (header[12]<<8)|(header[13]<<0);
		frames->frames[i].frame_rate_denominator = (header[14]<<8)|(header[15]<<0);
		frames->frames[i].delay = (header[16]<<8)|(header[17]<<0);
		frames->frames[i].presentation_timestamp = (header[18]<<24)|(header[19]<<16)|(header[20]<<8)|(header[21]<<0);
		frames->frames[i].size = (header[22]<<24)|(header[23]<<16)|(header[24]<<8)|(header[25]<<0);

	}

	return i;
}

static int  vp8_paylod_parser_ex(struct jpeg_parser* handle, BYTE** out_buffer, BYTE** header)
{
    BYTE* marker;
    int marker_length;

    /* get first APP4 marker */
    if(jpeg_parser_get_APP4_marker(handle, &marker, &marker_length) == 0)
    {
        printf("ERROR: jpeg_parser_get_APP4_marker");
        return -1;
    }
    if(marker_length < 26)
    {
        printf("ERROR: APP4 marker size shall > 26");
        return -1;
    }
    *header = marker;

    /* parse payload header to get size */
    int version = (marker[0]<<8) | marker[1];
    int header_size = (marker[2]<<8) | marker[3];
    int payload_size = (marker[header_size]<<24) | (marker[header_size+1]<<16) | (marker[header_size+2]<<8) | (marker[header_size+3]<<0);

    if(version != 0x0100)
    {
        /* old format */
        return -1;
    }
    if(payload_size > MAX_FRAME_SIZE)
    {
        printf("ERROR: frame too large");
        return -1;
    }

    /* setup the return value of out_buffer */
    *out_buffer = &marker[header_size+4];

    /* return first block and copy the following APP4 blocks*/
    int received_size = marker_length - header_size - 4;

    while(received_size < payload_size)
    {
        /* get next APP4 marker */
        if(jpeg_parser_get_APP4_marker(handle, &marker, &marker_length) == 0)
        {
            printf("ERROR: received data size(%d) < expected size(%d)", received_size, payload_size);
            return -1;
        }

        /* connect first block with all following blocks */
        memmove(*out_buffer + received_size, marker, marker_length);
        received_size += marker_length;
    }

    if(received_size != payload_size)
    {
        printf("ERROR: received data size(%d) != expected size(%d)", received_size, payload_size);
        return -1;
    }

    return payload_size;
}

int vp8demux_ex(BYTE* src, int size, vp8_frames_ex_t* frames)
{
    int i;
    struct jpeg_parser parser = {src, size};

    memset(frames, 0, sizeof(vp8_frames_ex_t));

    for(i=0;i<MAX_STREAM_NUM;i++)
    {
        BYTE* header;

        if(vp8_paylod_parser_ex(&parser, &frames->frames[i].data, &header) < 0){
            return i;
        }
        frames->frames[i].stream_type = (header[4]<<24)|(header[5]<<16)|(header[6]<<8)|(header[7]<<0);
        frames->frames[i].width = (header[8]<<8)|(header[9]<<0);
        frames->frames[i].height = (header[10]<<8)|(header[11]<<0);
        frames->frames[i].frame_rate_nominator = (header[12]<<8)|(header[13]<<0);
        frames->frames[i].frame_rate_denominator = (header[14]<<8)|(header[15]<<0);
        frames->frames[i].delay = (header[16]<<8)|(header[17]<<0);
        frames->frames[i].presentation_timestamp = (header[18]<<24)|(header[19]<<16)|(header[20]<<8)|(header[21]<<0);
        frames->frames[i].size = (header[22]<<24)|(header[23]<<16)|(header[24]<<8)|(header[25]<<0);

    }

    return i;
}


