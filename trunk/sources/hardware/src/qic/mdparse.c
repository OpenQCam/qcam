
#include "../include/qic/mdparse.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//#define false 0
//#define true 1
#ifdef QIC_MD_API
static const unsigned char guid[] = {0x19, 0xA3, 0x39, 0x57, 0x4F, 0xBD, 0x4A, 0x0D, 0xB7, 0xCF, 0xE5, 0xD2, 0xC8, 0xB2, 0xDD, 0x5D};



unsigned int AnalyzeSEIMessages(unsigned char* buffer, unsigned int size, unsigned int* sei_begin_offset, md_status_t* md_status)
{
    unsigned int i;
    bool foundSEI = false;
    unsigned char* sei;
    unsigned int sei_size;
    int top = 0;
	

    /*DEBUG("0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x 0x%02x",
         buffer[0], buffer[1], buffer[2], buffer[3],
            buffer[4], buffer[5], buffer[6], buffer[7],
            buffer[8], buffer[9], buffer[10], buffer[11],
            buffer[12], buffer[13], buffer[14], buffer[15]
            );*/

    /* search for the SEI uuid */
    for(i=0;i<size-16;i++)
    {
        if(buffer[i] == guid[0])
        {
            if(memcmp(&buffer[i], guid, sizeof(guid)) == 0)
            {
                foundSEI = true;
                *sei_begin_offset = i - 7;
                sei = &buffer[i+16];
                sei_size =buffer[i-1] - 16;
					
                break;
            }
        }
    }
  //  printf("foundSEI=%d\n", foundSEI);
    if(foundSEI == false)
        return 0;

    memset(md_status, 0, sizeof(md_status_t));

    /* parse key/value */
    int codepage = 0;
    for(i=0;i<sei_size;i++)
    {
        int type = sei[i];
        int len = type >> 6;
        int value = 0;

        type = type & 0x3F;
   //      printf("type=0x%x\n", foundSEI);
        unsigned char* ptr = &sei[i+1];
        switch(len)
        {
        case 3:
                value = *ptr++;
                i++;
                // fall through
        case 2:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        case 1:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        case 0:
                value <<=8;
                value |= *ptr++;
                i++;
                // fall through
        }

        if(type == DBG_SEI_CODE_PAGE)
        {
            codepage = value;
        }
        else if(codepage == 1)
        {
            switch(type)
            {
            case DBG_SEI_MD_TIMESTAMP:
                md_status->timestamp = value;
	//			printf("timestamp=%d\n",value);
                break;
            case DBG_SEI_MD_NUM_OF_MOVING_OBJS:
                md_status->number_of_moving_objects = value;
	//			printf("OBJ=%d\n",value);
                break;
            case DBG_SEI_MD_X:
                md_status->moving_objects[top].x = value;
	//			printf("X=%d\n",value);
                break;
            case DBG_SEI_MD_Y:
                md_status->moving_objects[top].y = value;
	//			printf("Y=%d\n",value);
                break;
            case DBG_SEI_MD_WIDTH:
                md_status->moving_objects[top].width = value;
	//			printf("W=%d\n",value);
                break;
            case DBG_SEI_MD_HEIGHT:
                md_status->moving_objects[top].height = value;
	//			printf("H=%d\n",value);
                top++;
                break;
            default:
                printf("got unknown entry (%d, %d, %d)", codepage, type, value);
            }
        }
    }

	//printf("BBBB\n");

    unsigned int sei_end_offset = (unsigned int)(sei - buffer) + sei_size + 1;
    return sei_end_offset;
}

#endif


