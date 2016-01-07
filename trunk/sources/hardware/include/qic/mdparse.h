#ifndef __MDPARSE_H__
#define __MDPARSE_H__

#include <stdint.h>

#include "qic_xuctrl.h"

#ifdef QIC_MD_API

#define DBG_SEI_MD_TIMESTAMP            1
#define DBG_SEI_MD_NUM_OF_MOVING_OBJS   2
#define DBG_SEI_MD_X                    3
#define DBG_SEI_MD_Y                    4
#define DBG_SEI_MD_WIDTH                5
#define DBG_SEI_MD_HEIGHT               6
#define DBG_SEI_CODE_PAGE            0x3F

unsigned int AnalyzeSEIMessages(unsigned char* buffer,unsigned int size,unsigned int* sei_begin_offset,md_status_t* md_status);

#endif
#endif


