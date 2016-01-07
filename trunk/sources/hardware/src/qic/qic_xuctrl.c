/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#include "../include/qic/qic_xuctrl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ctype.h>


#ifdef DMALLOC
#include <dmalloc.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))

//UVC control signals
#define UVC_SET_CUR  0x01
#define UVC_GET_CUR  0x81
#define UVC_GET_MIN  0x82
#define UVC_GET_MAX  0x83

/* Extension Unit GUID */
#define UVC_GUID_QIC_EXTENSION \
	{0x28, 0x17, 0x9e, 0x5d, 0x8d, 0xab, 0xa9, 0x4f, \
	 0x9a, 0xcb, 0xf6, 0xd5, 0x6d, 0xbc, 0x05, 0xa6}




/* Extension Unit controls */
#define XU_CONTROL_UNDEFINED            	0x00
#define XU_ENCODER_CONTROL                	0x01
#define XU_MMIO_CONTROL           	      	0x02
#define XU_FLASH_CONTROL                  	0x03
#define XU_MISC_CONTROL	            		0x04
#define XU_TEST_CONTROL                   	0x05
#define XU_AVC_ADVANCE_CONTROL          0x06
#define XU_AUDIO_CONTROL                   	0x07
#define XU_SYSINFO_CONTROL                   	0x08


#define   EU_QIC_SELECT_CODEC_CONTROL	        0x09           
#define   EU_QIC_TSVC_CONTROL			0x0A  
#define  EU_SELECT_LAYER_CONTROL   		0x0B  
#define	EU_PROFILE_TOOLSET_CONTROL		0x0C
#define	EU_VIDEO_RESOLUTION_CONTROL	0x0D	      
#define	EU_MIN_FRAME_INTERVAL_CONTROL 	0x0E		
#define	EU_SLICE_MODE_CONTROL			0x0F
#define	EU_RATE_CONTROL_MODE_CONTROL   0x10
#define	EU_AVERAGE_BITRATE_CONTROL	       0x11		
#define	EU_CPB_SIZE_CONTROL				0x12  
#define	EU_PEAK_BIT_RATE_CONTROL		0x13
#define	EU_QUANTIZATION_PARAMS_CONTROL 0x14
#define	EU_SYNC_REF_FRAME_CONTROL		 0x15		
#define	EU_LTR_BUFFER_CONTROL			0x16
#define	EU_LTR_PICTURE_CONTROL			0x17
#define	EU_LTR_VALIDATION_CONTROL		0x18
#define	EU_LEVEL_IDC_LIMIT_CONTROL		0x19	
#define	EU_SEI_PAYLOADTYPE_CONTROL		0x1A
#define	EU_QP_RANGE_CONTROL				0x1B
#define	EU_PRIORITY_CONTROL				0x1C
#define	EU_START_OR_STOP_LAYER_CONTROL  0x1D      
#define	EU_ERROR_RESILIENCY_CONTROL		0x1E	
#define XU_OSD_CONTROL                   	0x1F

#define XU_MD_CONTROL				0x13
#define XU_TD_CONTROL				0x13
#define XU_ENCODER_SIZE_DEF                	21
#define XU_MMIO_SIZE_DEF           	      	9
#define XU_FLASH_SIZE_DEF                  	27
#define XU_MISC_SIZE_DEF	            		27
#define XU_TEST_SIZE_DEF                   	27
#define XU_AVC_ADVANCE_SIZE_DEF	        27
#define XU_AUDIO_SIZE_DEF                   	27
#define XU_SYSINFO_SIZE_DEF			13
#define XU_MAX_SIZE						27

/*OSD */
#define XU_OSD_SIZE_DEF                    	31
#define XU_OSD_MAX_SIZE						31

/*MD*/
#define XU_MD_SIZE_DEF                    	31
#define XU_MD_MAX_SIZE						31

/*TD*/
#define XU_TD_SIZE_DEF                    	31 
#define XU_TD_MAX_SIZE						31

#define EU_QIC_SELECT_CODEC_SIZE		1      
#define EU_QIC_TSVC_SIZE				1 
#define EU_SELECT_LAYER_SIZE  			2    
#define	EU_PROFILE_TOOLSET_SIZE		5
#define	EU_VIDEO_RESOLUTION_SIZE		4	     	
#define	EU_MIN_FRAME_INTERVAL_SIZE 	4		
#define	EU_SLICE_MODE_SIZE			4
#define	EU_RATE_CONTROL_MODE_SIZE   1
#define	EU_AVERAGE_BITRATE_SIZE	       4		
#define	EU_CPB_SIZE_SIZE				4    
#define	EU_PEAK_BIT_RATE_SIZE			27
#define	EU_QUANTIZATION_PARAMS_SIZE 6
#define	EU_SYNC_REF_FRAME_SIZE		 4		
#define	EU_LTR_BUFFER_SIZE			27
#define	EU_LTR_PICTURE_SIZE			27
#define	EU_LTR_VALIDATION_SIZE		27
#define	EU_LEVEL_IDC_LIMIT_SIZE		1	
#define	EU_SEI_PAYLOADTYPE_SIZE		27
#define	EU_QP_RANGE_SIZE				2
#define	EU_PRIORITY_SIZE				27
#define	EU_START_OR_STOP_LAYER_SIZE  1                
#define	EU_ERROR_RESILIENCY_SIZE		2	


#define EU_MAX_SIZE					27


#define V4L2_CID_XU_ENCODER						(V4L2_CID_PRIVATE_BASE+0)
#define V4L2_CID_XU_MMIO							(V4L2_CID_PRIVATE_BASE+1)
#define V4L2_CID_XU_FLASH							(V4L2_CID_PRIVATE_BASE+2)
#define V4L2_CID_XU_MISC							(V4L2_CID_PRIVATE_BASE+3)
#define V4L2_CID_XU_TEST							(V4L2_CID_PRIVATE_BASE+4)
#define V4L2_CID_XU_AVC_ADVANCE					(V4L2_CID_PRIVATE_BASE+5)
#define V4L2_CID_XU_AUDIO							(V4L2_CID_PRIVATE_BASE+6)
#define V4L2_CID_XU_SYSINFO						(V4L2_CID_PRIVATE_BASE+7)

#define V4L2_CID_EU_QIC_SELECT_CODEC				(V4L2_CID_PRIVATE_BASE+8)
#define V4L2_CID_EU_QIC_TSVC						(V4L2_CID_PRIVATE_BASE+9)
#define V4L2_CID_EU_SELECT_LAYER					(V4L2_CID_PRIVATE_BASE+10)
#define V4L2_CID_EU_PROFILE_TOOLSET				(V4L2_CID_PRIVATE_BASE+11)
#define V4L2_CID_EU_VIDEO_RESOLUTION				(V4L2_CID_PRIVATE_BASE+12)
#define V4L2_CID_EU_MIN_FRAME_INTERVAL			(V4L2_CID_PRIVATE_BASE+13)
#define V4L2_CID_EU_SLICE_MODE					(V4L2_CID_PRIVATE_BASE+14)
#define V4L2_CID_EU_RATE_CONTROL_MODE			(V4L2_CID_PRIVATE_BASE+15)
#define V4L2_CID_EU_AVERAGE_BITRATE				(V4L2_CID_PRIVATE_BASE+16)
#define V4L2_CID_EU_CPB_SIZE						(V4L2_CID_PRIVATE_BASE+17)
#define V4L2_CID_EU_PEAK_BIT_RATE					(V4L2_CID_PRIVATE_BASE+18)
#define V4L2_CID_EU_QUANTIZATION_PARAMS			(V4L2_CID_PRIVATE_BASE+19)
#define V4L2_CID_EU_SYNC_REF_FRAME				(V4L2_CID_PRIVATE_BASE+20)
#define V4L2_CID_EU_LTR_BUFFER						(V4L2_CID_PRIVATE_BASE+21)
#define V4L2_CID_EU_LTR_PICTURE					(V4L2_CID_PRIVATE_BASE+22)
#define V4L2_CID_EU_LTR_VALIDATION				(V4L2_CID_PRIVATE_BASE+23)
#define V4L2_CID_EU_LEVEL_IDC_LIMIT				(V4L2_CID_PRIVATE_BASE+24)
#define V4L2_CID_EU_SEI_PAYLOADTYPE				(V4L2_CID_PRIVATE_BASE+25)
#define V4L2_CID_EU_QP_RANGE						(V4L2_CID_PRIVATE_BASE+26)
#define V4L2_CID_EU_PRIORITY						(V4L2_CID_PRIVATE_BASE+27)
#define V4L2_CID_EU_START_OR_STOP_LAYER			(V4L2_CID_PRIVATE_BASE+28)
#define V4L2_CID_EU_ERROR_RESILIENCY				(V4L2_CID_PRIVATE_BASE+29)
#define V4L2_CID_XU_OSD							(V4L2_CID_PRIVATE_BASE+30)
#define V4L2_CID_XU_MD							(V4L2_CID_PRIVATE_BASE+31)


#define QIC_UVC_CT_ID				0x01
#define QIC_UVC_PU_ID				0x02
#define QIC_UVC_XU_ID				0x03
#define QIC_UVC_OT1_ID				0x04
#define QIC_UVC_OT2_ID				0x05


/* Extension Unit parameter */
/* ENCODER */
#define QIC_XU1_ENCODER_BITRATE			0x01

#define QIC_XU1_ENCODER_SLICESIZE		0x08
#define QIC_XU1_ENCODER_IFRAME			0x10
#define QIC_XU1_ENCODER_GOP			0x20
#define QIC_XU1_ENCODER_QUALITY			0x40
#define QIC_XU1_ENCODER_ENTROPY 		0x40
#define QIC_XU1_ENCODER_TYPE			0x80
/* MMIO */
#define QIC_XU2_MMIO_READ			0x00
#define QIC_XU2_MMIO_WRITE			0x01
/* FLASH */
#define QIC_XU3_FLASH_WRITE			0x00
#define QIC_XU3_FLASH_WRITE_SEQ			0x01
#define QIC_XU3_FLASH_READ			0x02
#define QIC_XU3_FLASH_ERASE			0x03
#define QIC_XU3_FLASH_ERASE_SECTOR		0x04
#define QIC_XU3_FLASH_CONFIG			0x05
#define QIC_XU3_FLASH_CUSTOM			0x06
#define QIC_XU3_FLASH_GET_STATUS		0x07
/* MISC */
#define QIC_XU4_ENCRYPT_READ			0x00
#define QIC_XU4_ENCRYPT_WRITE			0x01
#define QIC_XU4_ENCRYPT_CONFIG			0x02
#define QIC_XU4_CALIBRATE_MODE			0x03
#define QIC_XU4_SPECIAL_EFFECT			0x04
#define QIC_XU4_ENCODEING_INFO			0x05
#define QIC_XU4_AEAWB_CUSTOM			0x06
#define QIC_XU4_ENCODER_OPTION			0x07
#define QIC_XU4_FLIP_MODE			0x07
/* MISC - for XU control4 - AE_Custom */
#define QIC_XU4_AEAWB_WB_COMP			0x01
#define QIC_XU4_AEAWB_EXP_COMP			0x02
#define QIC_XU4_AEAWB_AE_STEP			0x04

/* for XU control5 */
#define C5_H264_DESC				0
#define C5_LED_CONTROL				1
#define C5_STREAM_CONTROL			2
#define C5_FRAME_SIZE_CONTROL		3
#define C5_PAN_TILT_CONTROL			5
#define C5_SHUTTER_CONTROL			6
#define C5_MJPEG_QP_CONTROL			7
#define C5_CROPPED_IMAGE			8
#define C5_TRIMMING_CONTROL			9
#define C5_ALS_CONTROL			        10
#define C5_MOTOR_CONTROL			16
#define C5_WUSB_CONTROL				17
#define C5_ADVANCE_MOTOR_CONTROL	        18
#define C5_IR_CONTROL				19
#define C5_SENSOR_INFO_CONTROL		        20
#define C5_CHECK_LOCK_STREAM_CONTROL		21
#define C5_CAPABILITY_LIST_CONTROL		22
#define C5_LED_BRIGHTNESS_CONTROL               25
/* for XU control5 - Misc Control */
#define  C5_FROMAT_DESC			       0

#define C5_LED_SET_STATUS			0
#define C5_LED_GET_STATUS			1

#define C5_STREAM_SET_STATUS		0
#define C5_STREAM_GET_STATUS		1

#define C5_FRAME_SIZE_SET_STATUS	0
#define C5_FRAME_SIZE_GET_STATUS	1

#define C5_PAN_TILT_CONTROL_SET_STATUS          0
#define C5_PAN_TILT_CONTROL_GET_STATUS          1

#define C5_SHUTTER_SET_STATUS			0
#define C5_SHUTTER_GET_STATUS			1

#define C5_MJPEG_QP_CONTROL_SET_STATUS          0
#define C5_MJPEG_QP_CONTROL_GET_STATUS          1

#define C5_CROPPED_IMAGE_SET_STATUS	        0

#define C5_TRIMMING_SET_STATUS	        0
#define C5_TRIMMING_GET_STATUS	        1


#define C5_ALS_GET_STATUS	        1

#define C5_MOTOR_SET_STATUS          0
#define C5_MOTOR_GET_STATUS			1

#define C5_ADVANCE_MOTOR_SET_STATUS          0
#define C5_ADVANCE_MOTOR_GET_STATUS			1

#define C5_IR_SET_STATUS          0
#define C5_IR_GET_STATUS			1

#define C5_SENSOR_INFO_GET_STATUS			1

#define C5_WUSB_SET_WIRED	        		0
#define C5_WUSB_SET_WIRELESS	       		1
#define C5_WUSB_SET_DISABLE_AUTOSWITCH		2

#define C5_ADV_MOTOR_SET_STATUS          0
#define C5_ADV_MOTOR_GET_STATUS			1

#define C5_IR_MOTOR_SET_STATUS          0
#define C5_IR_MOTOR_GET_STATUS			1

#define C5_CHECK_LOCK_STREAM_SET_STATUS          0
#define C5_CHECK_LOCK_STREAM_GET_STATUS			1

#define C5_CAPABILITY_LIST_GET_STATUS	        	1

/*XU control 6: AVC Advance Controls*/
#define QIC_XU6_SET_STATUS			0x00
#define QIC_XU6_GET_STATUS			0x01

#define QIC_XU6_RGB_CONTROL			0x00
#define QIC_XU6_MIRROR_CONTROL                  0x01
#define QIC_XU6_MIRROR_SET_STATUS		0x00
#define QIC_XU6_MIRROR_GET_STATUS		0x01

#define QIC_XU6_TNR_CONTROL			0x02
#define QIC_XU6_RATE_CONTROL			0x03
#define QIC_XU6_DRS_CONTROL			0x06			


/* Audio */
#define QIC_XU7_ANR_CONTROL			0x00
#define QIC_XU7_ANR_SET_STATUS			0x00
#define QIC_XU7_ANR_GET_STATUS			0x01

#define QIC_XU7_EQ_CONTROL                      0x01
#define QIC_XU7_EQ_SET_STATUS			0x00
#define QIC_XU7_EQ_GET_STATUS			0x01

#define QIC_XU7_BF_CONTROL                      0x02
#define QIC_XU7_BF_SET_STATUS			0x00
#define QIC_XU7_BF_GET_STATUS			0x01

/* OSD */
#define QIC_XU31_OSD_STATUS_CONTROL		0
#define QIC_XU31_OSD_MISC_CONTROL		1
#define QIC_XU31_OSD_COLOR_CONTROL		2
#define QIC_XU31_OSD_FRAME_CONTROL		3
#define QIC_XU31_OSD_LINE_ATTRB_CONTROL	4
#define QIC_XU31_OSD_CHAR_FONT_CONTROL	5
#define QIC_XU31_OSD_CHAR_ATTRB_CONTROL 6
#define QIC_XU31_OSD_TIMER_CONTROL 		7


#define QIC_XU31_OSD_SET_SUBCMD			0x00
#define QIC_XU31_OSD_GET_SUBCMD			0x01


/*MD*/
#define QIC_XU19_MD_MODULE			0
#define QIC_XU19_MD_MODULE_1			1

#define QIC_XU19_MD_VERSION_CONTROL		0
#define QIC_XU19_MD_ENABLE_CONTROL           	1
#define QIC_XU19_MD_CONFIGURE_CONTROL		2
#define QIC_XU19_MD_STATUS_CONTROL		3
#define QIC_XU19_MD_INTERRUPT_CONTROL		4

#define QIC_XU19_MD_PEAK_BIREATE_CONTROL        0

#define QIC_XU19_MD_SET_SUBCMD			0x00
#define QIC_XU19_MD_GET_SUBCMD			0x01

/*TD*/
#define QIC_XU19_TD_MODULE			2

#define QIC_XU19_TD_VERSION_CONTROL		0
#define QIC_XU19_TD_ENABLE_CONTROL		1
#define QIC_XU19_TD_CONFIGURE_CONTROL		2
#define QIC_XU19_TD_STATUS_CONTROL		3
#define QIC_XU19_TD_SET_SUBCMD			0x00
#define QIC_XU19_TD_GET_SUBCMD			0x01


#ifdef DEBUG_LOG
/* debug output function , printf("%s\n", str); \ */
#define LOG_RET_PRINT(str, format, arg...) { \
	memset (str, 0, sizeof(str)); \
	snprintf(str, sizeof(str), format, ##arg); \
	printf("%s\n", str); \
}

#else
#define LOG_RET_PRINT(str, format, arg...)
#endif

extern char debug_xuctrl_str[384];

static struct uvc_xu_control_info xu_ctrls[] = {
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_ENCODER_CONTROL,
		.index	= XU_ENCODER_CONTROL-1,
		.size		= XU_ENCODER_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MMIO_CONTROL,
		.index	= XU_MMIO_CONTROL-1,
		.size		= XU_MMIO_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_FLASH_CONTROL,
		.index	= XU_FLASH_CONTROL-1,
		.size		= XU_FLASH_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MISC_CONTROL,
		.index	= XU_MISC_CONTROL-1,
		.size		= XU_MISC_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_TEST_CONTROL,
		.index	= XU_TEST_CONTROL-1,
		.size		= XU_TEST_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_AVC_ADVANCE_CONTROL,
		.index	= XU_AVC_ADVANCE_CONTROL-1,
		.size		= XU_AVC_ADVANCE_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_AUDIO_CONTROL,
		.index	= XU_AUDIO_CONTROL-1,
		.size		= XU_AUDIO_SIZE_DEF,
		.flags	= UVC_CONTROL_SET_CUR | UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_SYSINFO_CONTROL,
		.index	= XU_SYSINFO_CONTROL-1,
		.size		= XU_SYSINFO_SIZE_DEF,
		.flags	=   UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
/***************************************************/
/*               EU control                                                    */ 
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API) 		
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QIC_SELECT_CODEC_CONTROL,
		.index	= EU_QIC_SELECT_CODEC_CONTROL-1,
		.size		= EU_QIC_SELECT_CODEC_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QIC_TSVC_CONTROL,
		.index	= EU_QIC_TSVC_CONTROL-1,
		.size		= EU_QIC_TSVC_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},		
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SELECT_LAYER_CONTROL,
		.index	= EU_SELECT_LAYER_CONTROL-1,
		.size		= EU_SELECT_LAYER_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_PROFILE_TOOLSET_CONTROL,
		.index	= EU_PROFILE_TOOLSET_CONTROL-1,
		.size		= EU_PROFILE_TOOLSET_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},

		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_VIDEO_RESOLUTION_CONTROL,
		.index	= EU_VIDEO_RESOLUTION_CONTROL-1,
		.size		= EU_VIDEO_RESOLUTION_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
			{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_MIN_FRAME_INTERVAL_CONTROL,
		.index	= EU_MIN_FRAME_INTERVAL_CONTROL-1,
		.size		= EU_MIN_FRAME_INTERVAL_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
				{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SLICE_MODE_CONTROL,
		.index	= EU_SLICE_MODE_CONTROL-1,
		.size		= EU_SLICE_MODE_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},

{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_RATE_CONTROL_MODE_CONTROL,
		.index	= EU_RATE_CONTROL_MODE_CONTROL-1,
		.size		= EU_RATE_CONTROL_MODE_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_AVERAGE_BITRATE_CONTROL,
		.index	= EU_AVERAGE_BITRATE_CONTROL-1,
		.size		= EU_AVERAGE_BITRATE_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_CPB_SIZE_CONTROL,
		.index	= EU_CPB_SIZE_CONTROL-1,
		.size		= EU_CPB_SIZE_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},

{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QUANTIZATION_PARAMS_CONTROL,
		.index	= EU_QUANTIZATION_PARAMS_CONTROL-1,
		.size		= EU_QUANTIZATION_PARAMS_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SYNC_REF_FRAME_CONTROL,
		.index	= EU_SYNC_REF_FRAME_CONTROL-1,
		.size		= EU_SYNC_REF_FRAME_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},

	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_LEVEL_IDC_LIMIT_CONTROL,
		.index	= EU_LEVEL_IDC_LIMIT_CONTROL-1,
		.size		= EU_LEVEL_IDC_LIMIT_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},

	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QP_RANGE_CONTROL,
		.index	= EU_QP_RANGE_CONTROL-1,
		.size		= EU_QP_RANGE_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_START_OR_STOP_LAYER_CONTROL,
		.index	= EU_START_OR_STOP_LAYER_CONTROL-1,
		.size		= EU_START_OR_STOP_LAYER_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
		{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_ERROR_RESILIENCY_CONTROL,
		.index	= EU_ERROR_RESILIENCY_CONTROL-1,
		.size		= EU_ERROR_RESILIENCY_SIZE,
		.flags	=  UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
#ifdef QIC_OSD_API	
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_OSD_CONTROL,
		.index	= XU_OSD_CONTROL-1,
		.size		= XU_OSD_SIZE_DEF,
		.flags	=   UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
#endif
#ifdef QIC_MD_API	
	{
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MD_CONTROL,
		.index	= XU_MD_CONTROL-1,
		.size		= XU_MD_SIZE_DEF,
		.flags	=   UVC_CONTROL_SET_CUR| UVC_CONTROL_GET_RANGE
				| UVC_CONTROL_RESTORE,
	},
#endif

#endif
	
};

static struct uvc_xu_control_mapping xu_ctrl_mappings[] = {
	{
		.id		= V4L2_CID_XU_ENCODER,
		.name		= "ENCODER",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_ENCODER_CONTROL,
		.size		= XU_ENCODER_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER, 
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_MMIO,
		.name		= "MMIO",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MMIO_CONTROL,
		.size		= XU_MMIO_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_FLASH,
		.name		= "FLASH",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_FLASH_CONTROL,
		.size		= XU_FLASH_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_MISC,
		.name		= "MISC",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MISC_CONTROL,
		.size		= XU_MISC_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_TEST,
		.name		= "TEST",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_TEST_CONTROL,
		.size		= XU_TEST_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_AVC_ADVANCE,
		.name		= "AVC_ADVANCE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_AVC_ADVANCE_CONTROL,
		.size		= XU_AVC_ADVANCE_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_XU_AUDIO,
		.name		= "AUDIO",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_AUDIO_CONTROL,
		.size		= XU_AUDIO_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},	
	{
		.id		= V4L2_CID_XU_SYSINFO,
		.name		= "SYSINFO",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_SYSINFO_CONTROL,
		.size		= XU_SYSINFO_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
/*******************************************************/
/**********    EU control *********************************/
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API) 		
	{
		.id		= V4L2_CID_EU_QIC_SELECT_CODEC,
		.name		= "EU_QIC_SELECT_CODEC",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QIC_SELECT_CODEC_CONTROL,
		.size		= EU_QIC_SELECT_CODEC_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_QIC_TSVC,
		.name		= "EU_QIC_TSVC",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QIC_TSVC_CONTROL,
		.size		= EU_QIC_TSVC_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},

		{
		.id		= V4L2_CID_EU_SELECT_LAYER,
		.name		= "EU_SELECT_LAYER",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SELECT_LAYER_CONTROL,
		.size		= EU_SELECT_LAYER_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_PROFILE_TOOLSET,
		.name		= "EU_PROFILE_TOOLSET",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_PROFILE_TOOLSET_CONTROL,
		.size		= EU_PROFILE_TOOLSET_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_VIDEO_RESOLUTION,
		.name		= "EU_VIDEO_RESOLUTION",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_VIDEO_RESOLUTION_CONTROL,
		.size		= EU_VIDEO_RESOLUTION_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_MIN_FRAME_INTERVAL,
		.name		= "EU_MIN_FRAME_INTERVAL",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_MIN_FRAME_INTERVAL_CONTROL,
		.size		= EU_MIN_FRAME_INTERVAL_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_SLICE_MODE,
		.name		= "EU_SLICE_MODE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SLICE_MODE_CONTROL,
		.size		= EU_SLICE_MODE_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_RATE_CONTROL_MODE,
		.name		= "EU_RATE_CONTROL_MODE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_RATE_CONTROL_MODE_CONTROL,
		.size		= EU_RATE_CONTROL_MODE_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_AVERAGE_BITRATE,
		.name		= "EU_AVERAGE_BITRATE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_AVERAGE_BITRATE_CONTROL,
		.size		= EU_AVERAGE_BITRATE_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_CPB_SIZE,
		.name		= "EU_CPB_SIZE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_CPB_SIZE_CONTROL,
		.size		= EU_CPB_SIZE_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},

{
		.id		= V4L2_CID_EU_QUANTIZATION_PARAMS,
		.name		= "EU_QUANTIZATION_PARAMS",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QUANTIZATION_PARAMS_CONTROL,
		.size		= EU_QUANTIZATION_PARAMS_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_EU_SYNC_REF_FRAME,
		.name		= "EU_SYNC_REF_FRAME",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_SYNC_REF_FRAME_CONTROL,
		.size		= EU_SYNC_REF_FRAME_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_EU_LEVEL_IDC_LIMIT,
		.name		= "EU_LEVEL_IDC_LIMIT",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_LEVEL_IDC_LIMIT_CONTROL,
		.size		= EU_LEVEL_IDC_LIMIT_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
	{
		.id		= V4L2_CID_EU_QP_RANGE,
		.name		= "EU_QP_RANGE",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_QP_RANGE_CONTROL,
		.size		= EU_QP_RANGE_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},

		{
		.id		= V4L2_CID_EU_START_OR_STOP_LAYER,
		.name		= "EU_START_OR_STOP_LAYER",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_START_OR_STOP_LAYER_CONTROL,
		.size		= EU_START_OR_STOP_LAYER_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
		{
		.id		= V4L2_CID_EU_ERROR_RESILIENCY,
		.name		= "EU_ERROR_RESILIENCY",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= EU_ERROR_RESILIENCY_CONTROL,
		.size		= EU_ERROR_RESILIENCY_SIZE*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
#ifdef QIC_OSD_API
	{
		.id		= V4L2_CID_XU_OSD,
		.name		= "OSD",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_OSD_CONTROL,
		.size		= XU_OSD_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
#endif
#ifdef QIC_MD_API
	{
		.id		= V4L2_CID_XU_MD,
		.name		= "MD",
		.entity	= UVC_GUID_QIC_EXTENSION,
		.selector	= XU_MD_CONTROL,
		.size		= XU_MD_SIZE_DEF*8,
		.offset	= 0,
		.v4l2_type	= V4L2_CTRL_TYPE_INTEGER,
		.data_type	= UVC_CTRL_DATA_TYPE_RAW,
	},
#endif
#endif
		
};

unsigned int EncoderProfileTbl[] = {
	ENCODER_PROFILE_BASELINE,
	ENCODER_PROFILE_MAIN,
	ENCODER_PROFILE_EXTENDED,
	ENCODER_PROFILE_HIGH,
	ENCODER_PROFILE_HIGH_10,
	ENCODER_PROFILE_HIGH_422,
	ENCODER_PROFILE_HIGH_444,
	ENCODER_PROFILE_CAVLC_444
};

unsigned int EncoderLevelTbl[] = {
	ENCODER_LEVEL_1,
	ENCODER_LEVEL_1B,
	ENCODER_LEVEL_1_1,
	ENCODER_LEVEL_1_2,
	ENCODER_LEVEL_1_3,
	ENCODER_LEVEL_2,
	ENCODER_LEVEL_2_1,
	ENCODER_LEVEL_2_2,
	ENCODER_LEVEL_3,
	ENCODER_LEVEL_3_1,
	ENCODER_LEVEL_3_2,
	ENCODER_LEVEL_4,
	ENCODER_LEVEL_4_1,
	ENCODER_LEVEL_4_2,
	ENCODER_LEVEL_5,
	ENCODER_LEVEL_5_1
};

#if defined(QIC_MPEGTS_API)
int EncoderGetProfileIdx(unsigned int profile);
int EncoderGetLevelIdx(unsigned int level);
#endif

int m_XuEncoderSize = XU_ENCODER_SIZE_DEF;

int m_vd = 0;


void QicChangeFD(int fd) {

	m_vd = fd;

}

/*************************************************************************
QIC Initial : Register XU Controls
*************************************************************************/

int QicSetDeviceHandle (int vd)
{
	int i, ret = 0;
	unsigned int svn_version=0;
	FirmwareVersion_t fw_version;
	 struct uvc_xu_control_mapping_2_6_42 new_mapping;
	 struct uvc_xu_control_mapping   uvc_32_mapping;
     struct v4l2_capability cap;
	char str_t[512]={0};
	char str[1024*4] = {0};
	SysInfo_t  qicSysInfo;

        memset(&new_mapping,0,sizeof(new_mapping));
		
	if (m_vd != vd)
	{
		m_vd = vd;	
	

	ret= ioctl(m_vd, VIDIOC_QUERYCAP, &cap);

   if(ret){
	     if (EINVAL == errno) {
	      LOG_RET_PRINT(debug_xuctrl_str, "is no V4L2 device");
	      return ret;
	    } else {
	      LOG_RET_PRINT(debug_xuctrl_str,"%s error %d, %s\n", "VIDIOC_QUERYCAP", errno, strerror(errno));
	      return ret;
	    }
   }
   
    LOG_RET_PRINT(debug_xuctrl_str,"kernel version:  0x%x",cap.version);
        printf ("Version: %u.%u.%u\n",
	( cap.version >> 16) & 0xFF,
	( cap.version >> 8) & 0xFF,
	  cap.version & 0xFF);
 
				// register other controls		
		for (i=0; i<ARRAY_SIZE(xu_ctrls); i++)
		{				

	
		        if(cap.version<=KERNEL_VERSION(3,2,0)){			
			ret = ioctl (m_vd, UVCIOC_CTRL_ADD, &xu_ctrls[i]);
			if(ret) {
				   if(errno!=EEXIST){ //skip errorno (17)File exists 
					LOG_RET_PRINT(str_t, "\n Add XU , (%d)%s", errno, strerror(errno));
					strcat(str, str_t);
				   }
				
			} else {
				LOG_RET_PRINT(str_t, "\n  Add XU Index%d Success", xu_ctrls[i].index);
				strcat(str, str_t);
			}
                        }  

			if(cap.version>=KERNEL_VERSION(3,2,0)){	
			memcpy( &new_mapping, &xu_ctrl_mappings[i],sizeof(struct uvc_xu_control_mapping) );
			
			ret= ioctl (m_vd, UVCIOC_CTRL_MAP_2_6_42, &new_mapping);
			if(ret){
				if(errno!=EEXIST){ //skip errorno (17)File exists 
					LOG_RET_PRINT(debug_xuctrl_str,"New Map 42 error[0x%x]: %s  (%d)%s \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name,errno, strerror(errno));
				}
			}
			if (!ret) {
				LOG_RET_PRINT(debug_xuctrl_str,"Map 42[0x%x]: %s success \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name);
			}
			
			}	
			else if(cap.version<KERNEL_VERSION(3,2,0)){	
				LOG_RET_PRINT(debug_xuctrl_str,"Map 32[0x%x]: %s start \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name);
				
				// memset(&uvc_32_mapping,0,sizeof(uvc_32_mapping));	
			if (0 == ioctl (m_vd, UVCIOC_CTRL_MAP, &xu_ctrl_mappings[i])) {
				LOG_RET_PRINT(debug_xuctrl_str,"Map 32[0x%x]: %s success \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name);
			}
			else 
				{
				if(errno == EEXIST ) {
					//LOG_RET_PRINT(debug_xuctrl_str,"Map 32[0x%x]: %s  (%d)%s \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name,errno, strerror(errno));
				}
				else{

					if (0 == ioctl (m_vd, UVCIOC_CTRL_MAP_2_6_36, &xu_ctrl_mappings[i])) {
						LOG_RET_PRINT(debug_xuctrl_str,"Map 36[0x%x]: %s success \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name);
					}
					else{
						if(errno!=EEXIST) //skip errorno (17)File exists
						    LOG_RET_PRINT(debug_xuctrl_str,"Map 36 [0x%x]: %s  (%d)%s \n", xu_ctrl_mappings[i].id, xu_ctrl_mappings[i].name,errno, strerror(errno));
					}
						
				}		

			}
                  }
		
		}
		LOG_RET_PRINT(debug_xuctrl_str, "\n%s", str);

			QicGetSysInfo(&qicSysInfo);
 
		printf("\033[1;33mQIC (Current Camera Version) VID:0x%.4x PID:0x%.4x REV:0x%.4x \033[m\n", qicSysInfo.sVID, qicSysInfo.sPID,
				qicSysInfo.sREV);


		// read FW & SVN version
		ret = QicGetFirmwareVersion (&fw_version);
		if(ret) {
			LOG_RET_PRINT(str_t, "\nGet Firmware Version Error, (%d)%s", errno, strerror(errno));
			strcat(str, str_t);
		} else {
			LOG_RET_PRINT(str_t, "\nVID : %s, PID : %s, REV : %s", fw_version.szVID, fw_version.szPID, fw_version.szREV);
			strcat(str, str_t);
		}
		
		ret = QicGetSvnVersion (&svn_version);
		if(ret) {
			LOG_RET_PRINT(str_t, "\nGet SVN Version Error, (%d)%s", errno, strerror(errno));
			strcat(str, str_t);
		} else {
			LOG_RET_PRINT(str_t, "\nSVN : %d", svn_version);
			strcat(str, str_t);
		}		


	//	if(svn_version >= 1291)
		if(1)
		{
			m_XuEncoderSize = 21;	
			xu_ctrls[0].size = 21;
			xu_ctrl_mappings[0].size = 21*8;
		}
		else
		{
			m_XuEncoderSize = 14;	
			xu_ctrls[0].size = 14;
			xu_ctrl_mappings[0].size = 14*8;
		}


	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Device handle has already been set.");
	}

	return ret;
}


#if defined(QIC_MPEGTS_API)
/*************************************************************************
QIC XU1 : Encoder Control
*************************************************************************/

int QicEncoderGetParams (EncoderParams_t *params)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;

	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
   	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Param Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Param Success");
	}	
		
	params->uiBitrate = buf[1] + (buf[2] << 8) + (buf[3] << 16) + (buf[4] << 24);
	params->usWidth = buf[5] + (buf[6] << 8);
	params->usHeight = buf[7] + (buf[8] << 8);
	params->ucFramerate = buf[9];
	params->ucSlicesize = buf[10];

	return ret;
}

int QicEncoderSetParams (EncoderParams_t *params, unsigned char flag)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;

	unsigned char buf[XU_MAX_SIZE];
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE	
   	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
	
	buf[0] = flag & 0x0f; 
	buf[1] = params->uiBitrate & 0xff;
	buf[2] = (params->uiBitrate >> 8) & 0xff;
	buf[3] = (params->uiBitrate >> 16) & 0xff;
	buf[4] = (params->uiBitrate >> 24) & 0xff;
	buf[5] = params->usWidth & 0xff;
	buf[6] = (params->usWidth >> 8) & 0xff;
	buf[7] = params->usHeight & 0xff;
	buf[8] = (params->usHeight >> 8) & 0xff;
	buf[9] = params->ucFramerate & 0xff;
	buf[10] = params->ucSlicesize & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Param Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Param Success");
	}
	

	return ret;
}


int QicEncoderSetIFrame ()
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
    	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
	
	buf[0] = QIC_XU1_ENCODER_IFRAME; 
	buf[11] = 0x01;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set I Frame Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set I Frame Success");
	}

	return ret;
}

int QicEncoderGetGOP (int *gop)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
   	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get GOP Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get GOP Success");
	}
	
	*gop = buf[12] + (buf[13] << 8);

	return ret;
}

int QicEncoderSetGOP (int gop)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
  	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
	
	buf[0] = QIC_XU1_ENCODER_GOP; 
	buf[12] = gop & 0xff;
	buf[13] = (gop >> 8) & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set GOP Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set GOP Success");
	}
	return ret;
}

int QicEncoderGetQuality(EncoderQuality_t *quality)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Quality Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Quality Success");
	}
	
	quality->uiTimeWindow = 0;
	quality->ucOverShoot = buf[18];
	quality->ucMinQP = buf[19];
	quality->ucMaxQP = buf[20];

	return ret;
}

int QicEncoderSetQuality(EncoderQuality_t *quality, unsigned char flag)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;
	EncoderQuality_t _qual;	

	/* Get encoder quality */
	QicEncoderGetQuality(&_qual);

	if (flag & QIC_XU1_QUALITY_TIMEWINDOW)
	{
		_qual.uiTimeWindow = quality->uiTimeWindow; 
	}
	if (flag & QIC_XU1_QUALITY_OVERSHOOT)
	{
		_qual.ucOverShoot = quality->ucOverShoot;
	}
	if (flag & QIC_XU1_QUALITY_MINQP)
	{
		_qual.ucMinQP = quality->ucMinQP;
	}
	if (flag & QIC_XU1_QUALITY_MAXQP)
	{
		_qual.ucMaxQP = quality->ucMaxQP;
	}

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = QIC_XU1_ENCODER_QUALITY; 
	buf[14] = 0;
	buf[15] = 0;
	buf[16] = 0;
	buf[17] = 0;
	buf[18] = _qual.ucOverShoot & 0xff;
	buf[19] = _qual.ucMinQP & 0xff;
	buf[20] = _qual.ucMaxQP & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Quality Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Quality Success");
	}
	return ret;
}

/*******************************************************************
QIC XU1 : Get H.264 Encoder Number Of Profiles
*******************************************************************/
int QicEncoderGetNumberOfProfiles(int *num_profiles)
{
	*num_profiles = 1;
	
	return 0;
}

/*******************************************************************
QIC XU1 : Get H.264 Encoder Profile Infomation
*******************************************************************/
int QicEncoderGetProfile(int index, int *max_level, int *profile, int *constraint_flags)
{
	if (index == 0)
	{
		*max_level = ENCODER_LEVEL_3_1;
		*profile = EncoderProfileTbl[0];
		*constraint_flags = 0x7;
		return 0;
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Profile Error (No Profile Found)");
	}
		
	return 1;
}

/*******************************************************************
QIC XU1 : Get H.264 Encoder Profile and Level
*******************************************************************/
int QicEncoderGetProfileAndLevel(int *level, int *profile, int *constraint_flags)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	xctrl.data = buf;
	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Profile And Level Error, (%d)%s", errno, strerror(errno));
		return ret;
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Profile And Level Success");
	}

	*profile = EncoderProfileTbl[buf[14]];
	*level = EncoderLevelTbl[buf[15]];

	if (*level == ENCODER_LEVEL_1B)
		*constraint_flags = 0xf;
	else
		*constraint_flags = 0x7;

	return ret;
}

/*******************************************************************
QIC XU1 : Set H.264 Encoder Profile and Level
*******************************************************************/
int QicEncoderSetProfileAndLevel(int level, int profile, int constraint_flags)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	xctrl.data = buf;

	buf[0] = QIC_XU1_ENCODER_TYPE;
	buf[0x0E] = EncoderGetProfileIdx (profile);
	buf[0x0F] = EncoderGetLevelIdx (level);

	if ((buf[0x0E] == 0xff) || (buf[0x0F] == 0xff))
	{
		if((buf[0x0E] == 0xff) && (buf[0x0F] == 0xff)) {
			LOG_RET_PRINT(debug_xuctrl_str, "\nSet Profile And Level Error (No Level Found)\nSet Profile And Level Error (No Profile Found)");
		} else if(buf[0x0E] == 0xff) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Profile And Level Error (No Profile Found)");
		} else if(buf[0x0E] == 0xff) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Profile And Level Error (No Level Found)");
		}
		
		return (ret = 1);
	}

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Profile And Level Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Profile And Level Success");
	}

	return ret;
}


/*******************************************************************
QIC XU1 MISC : Mapped H.264 Encoder Profile Index
*******************************************************************/
int EncoderGetProfileIdx(unsigned int profile)
{
	int i;

	for (i=0; EncoderProfileTbl[i]; i++)
	{
		if (profile == EncoderProfileTbl[i])
			return i;
	}

	return -1;
}

/*******************************************************************
QIC XU1 MISC : Mapped H.264 Encoder Level Index
*******************************************************************/
int EncoderGetLevelIdx(unsigned int level)
{
	int i;

	for (i=0; EncoderLevelTbl[i]; i++)
	{
		if (level == EncoderLevelTbl[i])
			return i;
	}

	return -1;
}

int QicSetStreamFormat(EncoderStreamFormat_t format)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_STREAM_CONTROL;
	buf[1] = C5_STREAM_SET_STATUS;
	buf[2] = format& 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set stream format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set stream format Success");
	}
	return ret;
}

int QicGetStreamFormat(EncoderStreamFormat_t *format)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query =UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_STREAM_CONTROL; // GET Command
	buf[1] = C5_STREAM_GET_STATUS;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Stream format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Stream format Success");
	}
	
	/* read */
	/*try new query api first*/
	xu_q.query =UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream format Success");
	}
	
	*format = buf[2];

	return ret;
}


int QicSetEncoderFrameSizeFormat(EncoderStreamSizeFormat_t format)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_FRAME_SIZE_CONTROL;
	buf[1] = C5_FRAME_SIZE_SET_STATUS;
	buf[2] = format & 0xff;
        

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set stream size format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set stream size format Success");
	}
	return ret;
}

int QicGetEncoderFrameSizeFormat(EncoderStreamSizeFormat_t *format)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_FRAME_SIZE_CONTROL;
	buf[1] = C5_FRAME_SIZE_GET_STATUS;


	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get stream size format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get stream size format Success");
	}

	/* read */
	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);
        
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream size format Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream size format Success");
	}

        *format = buf[2];

	return ret;
}


#endif

int QicGetFwAPIVersion (unsigned char *version)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
   	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Param Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Param Success");
	}	
	
	*version=buf[0]; 

	return ret;
}


/*************************************************************************
QIC XU2 : MMIO Control
*************************************************************************/

int QicMmioWrite (unsigned int addr, unsigned int value)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MMIO_CONTROL;
	xu_q.size = XU_MMIO_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MMIO_CONTROL;
	xctrl.size = XU_MMIO_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	buf[0] = QIC_XU2_MMIO_WRITE; 
	buf[1] = addr & 0xff;
	buf[2] = (addr >> 8) & 0xff;
	buf[3] = (addr >> 16) & 0xff;
	buf[4] = (addr >> 24) & 0xff;
	buf[5] = value & 0xff;
	buf[6] = (value >> 8) & 0xff;
	buf[7] = (value >> 16) & 0xff;
	buf[8] = (value >> 24) & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Write MMIO Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Write MMIO Success");
	}	
	return ret;
}

int QicMmioRead (unsigned int addr, unsigned int *value)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MMIO_CONTROL;
	xu_q.size = XU_MMIO_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MMIO_CONTROL;
	xctrl.size = XU_MMIO_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif	
	buf[0] = QIC_XU2_MMIO_READ; 
	buf[1] = addr & 0xff;
	buf[2] = (addr >> 8) & 0xff;
	buf[3] = (addr >> 16) & 0xff;
	buf[4] = (addr >> 24) & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Read MMIO Error, UVCIOC_CTRL_SET, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Read MMIO Success, UVCIOC_CTRL_SET");
	}	

	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Read MMIO Error, UVCIOC_CTRL_GET, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Read MMIO Success, UVCIOC_CTRL_GET");
	}
	*value = buf[5] + (buf[6] << 8) + (buf[7] << 16) + (buf[8] << 24);

	return ret;
}

int QicReset ()
{
	int ret;
	QicHwVer_t version;

	 QicGetHWVersion(&version);

	if(version==QIC_HWVERSION_QIC1802){
		/* use watch-dog-timer to reset qic */
		ret = QicMmioWrite(0x02100014, 0x00);
		ret = QicMmioWrite(0x02100018, 0x80124F80); //12MHz * 700ms  = 0x80B71B00
	//	ret = QicMmioWrite(0x02100018, 0x8001D4C0); //12MHz * 10ms = 120000 = 0x1D4C0
		ret = QicMmioWrite(0x02100014, 0x08);
		ret = QicMmioWrite(0x02100014, 0x04);	
	}else{
			/* use watch-dog timer to reset qic */
		unsigned int reg;
		QicMmioRead (0x20001008, &reg);
		reg &= ~(0xc00000);

		ret = QicMmioWrite (0x20001008, reg);
		ret = QicMmioWrite (0x20001014, 0x80B71B00); //12MHz * 1s = 120000 = 0x80B71B00
//		ret = QicMmioWrite (0x20001014, 0x80124F80); //12MHz * 700ms  = 0x80124F80
//		ret = QicMmioWrite (0x20001014, 0x8001D4C0); //12MHz * 10ms = 120000 = 0x1D4C0
		ret = QicMmioWrite (0x20001008, reg | 0x800000);
		ret = QicMmioWrite (0x20001008, reg | 0x400000);
	}
	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Reset Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Reset Success");
	}
	return ret;
}

int QicSetPll()
{
	unsigned int value;
	QicHwVer_t version;
	int ret;


	 QicGetHWVersion(&version);

	if(version==QIC_HWVERSION_QIC1802){
		
		// Set PLL for QIC1802
		ret = QicMmioRead (0x02100008, &value);
		if (!(value & 0x01000000))
		{
			value = value & 0xfe000000;
			
			// Set PLL as 150MHz
			ret = QicMmioWrite (0x02100008, value | 0x0001a620);
			// Power on PLL
			ret = QicMmioWrite (0x02100008, value | 0x00012620);
			usleep (100000);
			// Set divider devide 2
			ret = QicMmioWrite (0x02100010, 0x00000000);
			// Switch to PLL clock
			ret = QicMmioWrite (0x02100008, value | 0x01012620);
		}
		

	}
	else{
			// Set PLL for QIC1806
		ret = QicMmioRead (0x20001008, &value);
		if (!(value & 0x00020000))
		{
			value = value & 0xfffc0000;
			
			// Set PLL as 300MHz
			ret = QicMmioWrite (0x20001008, value | 0x00008300);
			// Power on PLL
			ret = QicMmioWrite (0x20001008, value | 0x00000300);
			// Assert reset and wait PLL locked
			ret = QicMmioWrite (0x20001008, value | 0x00010300);
			usleep (1000);	// 1ms
			ret = QicMmioWrite (0x20001008, value | 0x00000300);
			usleep (1000);	// 1ms
			// Set divider devide 2
			ret = QicMmioWrite (0x20001010, 0x00000000);
			// Switch to PLL clock
			ret = QicMmioWrite (0x20001008, value | 0x00020300);
		}

		}

	if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Pll Error, (%d)%s", errno, strerror(errno));
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Pll Success");
		}

	
	return ret;
}

/*************************************************************************
QIC XU3 : Serial Flash Control
*************************************************************************/

int QicFlashErase (void)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	memset (buf, 0, XU_MAX_SIZE);
	
	buf[0] = (QIC_XU3_FLASH_ERASE << 5);

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Flash Erase Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Flash Erase Success");
	}
	return ret;
}

int QicFlashSectorErase (int addr)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //force erase device w/wo bad image
#endif	
	buf[0] = (QIC_XU3_FLASH_ERASE_SECTOR << 5);
	buf[1] = (addr & 0x0000ff);
	buf[2] = (addr & 0x00ff00) >> 8;
	buf[3] = (addr & 0xff0000) >> 16;
	buf[4] = (addr & 0xff000000) >> 24;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Flash Sector Erase Error, (%d)%s", errno, strerror(errno));
	} else {
	//	LOG_RET_PRINT(debug_xuctrl_str, "Flash Sector Erase Success");
	}
	return ret;
}

int QicFlashSectorErasetype2 (int addr)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //force erase device w/wo bad image
#endif
	buf[0] = (QIC_XU3_FLASH_CUSTOM << 5) | 4;
	buf[1] = 0xd8;
	buf[2] = (addr & 0xff0000) >> 16;
	buf[3] = (addr & 0x00ff00) >> 8;
	buf[4] = (addr & 0x0000ff);
	

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if (ret)
	{
		perror ("Flash Sector Erase Error");	
	}
	return ret;	
}


int QicFlashCustom(char *data, int data_size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	unsigned int len;
	char	*pdata;
	int ret = 0;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	memset (buf, 0, XU_MAX_SIZE);
	len = data_size;
	pdata = data;
	
	/* send custom request*/	
	while (len)
	{
		unsigned int	desired_len = FLASH_MAX_DATA_LEN;

		if (desired_len > len)
			desired_len = len;

		/* read file and send request */
		memcpy (&buf[1], pdata, desired_len);
		buf[0] = (QIC_XU3_FLASH_CUSTOM << 5) | desired_len;
		len -= desired_len;
		pdata += desired_len;

		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set I Frame Error, (%d)%s", errno, strerror(errno));
			return ret;
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Set I Frame Success");
		}
	}

	return ret;
}

int QicFlashWriteUSBIMG (int addr, unsigned char *data, int data_size, unsigned int max_flash_size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	unsigned int len;
	unsigned char	*pdata;
	int ret;
	unsigned int display_size=0;
	int oriaddr=addr;

	if ((oriaddr + data_size) > max_flash_size)
		return -1;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	memset (buf, 0, XU_MAX_SIZE);
	addr += 2;
	/* set header and write address */
	buf[0] = (QIC_XU3_FLASH_WRITE << 5);
	buf[1] = (addr & 0x0000ff);
	buf[2] = (addr & 0x00ff00) >> 8;
	buf[3] = (addr & 0xff0000) >> 16;
	buf[4] = (addr & 0xff000000) >> 24;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if (ret)
	{
		perror ("Flash Write Error");
		return ret;	
	}
 

	
	len = data_size-2;
	pdata = data+2;
   

	/* send write seq request*/	
	while (len)
	{
		unsigned int	desired_len = FLASH_MAX_DATA_LEN;	

		if ( ((addr + FLASH_MAX_DATA_LEN) / FLASH_PAGE_SIZE) > (addr / FLASH_PAGE_SIZE) )
			desired_len = ((addr / FLASH_PAGE_SIZE) + 1) * FLASH_PAGE_SIZE - addr;

		if (desired_len > len)
			desired_len = len;

		/* read file and send request */
		memcpy (&buf[1], pdata, desired_len);

		
		buf[0] = (QIC_XU3_FLASH_WRITE_SEQ << 5) | desired_len;
		len -= desired_len;
		addr += desired_len;
		pdata += desired_len;

		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
		if (ret)
		{
			printf ("Flash Write Error %d %d %s",len,addr,pdata);
			return ret;	
		}

		display_size=display_size+desired_len;
		printf("\r [size:%d /total:%d]",display_size,data_size);
	}

		printf("\n");

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	memset (buf, 0, XU_MAX_SIZE);

	/* set header and write address */
	buf[0] = (QIC_XU3_FLASH_WRITE << 5);
	buf[1] = (oriaddr & 0x0000ff);
	buf[2] = (oriaddr & 0x00ff00) >> 8;
	buf[3] = (oriaddr & 0xff0000) >> 16;
	buf[4] = (oriaddr & 0xff000000) >> 24;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	

	if (ret)
	{
		perror (" F Flash Write Error");
		return ret;	
	}

		buf[0] = (QIC_XU3_FLASH_WRITE_SEQ << 5) | 2;
              buf[1] =data[0];
              buf[2] =data[1];

		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
		if (ret)
		{
			printf (" Flash Write Error");
			return ret;	
		}

	
	

	return ret;	
}




int QicFlashWrite (int addr, unsigned char *data, int data_size, unsigned int max_flash_size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	unsigned int len;
	unsigned char	*pdata;
	int ret;	

	if ((addr + data_size) > max_flash_size)
		return -1;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	memset (buf, 0, XU_MAX_SIZE);

	/* set header and write address */
	buf[0] = (QIC_XU3_FLASH_WRITE << 5);
	buf[1] = (addr & 0x0000ff);
	buf[2] = (addr & 0x00ff00) >> 8;
	buf[3] = (addr & 0xff0000) >> 16;
	buf[4] = (addr & 0xff000000) >> 24;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Flash Write Error, (%d)%s", errno, strerror(errno));
		return ret;	
	} else {
	//	LOG_RET_PRINT(debug_xuctrl_str, "Flash Write Success");
	}

	len = data_size;
	pdata = data;
	
	/* send write seq request*/	
	while (len)
	{
		unsigned int	desired_len = FLASH_MAX_DATA_LEN;	

		if ( ((addr + FLASH_MAX_DATA_LEN) / FLASH_PAGE_SIZE) > (addr / FLASH_PAGE_SIZE) )
			desired_len = ((addr / FLASH_PAGE_SIZE) + 1) * FLASH_PAGE_SIZE - addr;

		if (desired_len > len)
			desired_len = len;

		/* read file and send request */
		memcpy (&buf[1], pdata, desired_len);
		buf[0] = (QIC_XU3_FLASH_WRITE_SEQ << 5) | desired_len;
		len -= desired_len;
		addr += desired_len;
		pdata += desired_len;

		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	    
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Flash Write Error, (%d)%s", errno, strerror(errno));
			return ret;
		} else {
	//		LOG_RET_PRINT(debug_xuctrl_str, "Flash Write Success, addr = %d", addr);
		}
	}

	return ret;	
}

int QicFlashRead (int addr, unsigned char *data, int data_size, unsigned int max_flash_size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	unsigned int ReadLen=0;
	int ret;	

	memset(&xctrl,0,sizeof(xctrl));
	memset(&xu_q,0,sizeof(xu_q));

	if ((addr + data_size) > max_flash_size){
		return -1;
	}
/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //force read device w/wo bad image
#endif	
	memset (buf, 0, XU_MAX_SIZE);

	/* set header and read address */
	buf[0] = (QIC_XU3_FLASH_READ << 5) | FLASH_MAX_DATA_LEN;
	buf[1] = (addr & 0x0000ff);
	buf[2] = (addr & 0x00ff00) >> 8;
	buf[3] = (addr & 0xff0000) >> 16;
	buf[4] = (addr & 0xff000000) >> 24;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Flash Read Error, (%d)%s", errno, strerror(errno));
		return ret;
	} else {
	//	LOG_RET_PRINT(debug_xuctrl_str, "Flash Read Success");
	}

	/* read */
	ReadLen = 0;
	while (ReadLen < data_size)
	{
		unsigned int	DataLen;
		
		/*try new query api first*/
		xu_q.query = UVC_GET_CUR ;
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Flash Read Error, (%d)%s", errno, strerror(errno));
			return ret;
		} else {
	//		LOG_RET_PRINT(debug_xuctrl_str, "Flash Read Success, ReadLen = %d", ReadLen);
		}

		if ( (ReadLen + FLASH_MAX_DATA_LEN) > data_size )
			DataLen = data_size - ReadLen;
		else
			DataLen = FLASH_MAX_DATA_LEN;

		memcpy ((data+ReadLen), &buf[1], DataLen);
		ReadLen += DataLen;
	}	

	return ret;	
}



/*******************************************************************
QIC XU3 : Set SPI Configuration (divider, mode)
*******************************************************************/
int QicFlashSetSpiConfig (unsigned int divider, FlashProgramMode_t mode)
{

	QicHwVer_t version;

	 QicGetHWVersion(&version);

	if(version==QIC_HWVERSION_QIC1802){

	/* Clear serial flash write protect */
		unsigned int	bit_mask, reg;

		/* Calculate bit_mask */
		bit_mask = 1L << 14;

		/* Set software control mode */
		QicMmioRead (0x02000018, &reg);
		QicMmioWrite (0x02000018, reg & ~bit_mask);

		/* Set direction to output */
		QicMmioRead (0x02000004, &reg);
		QicMmioWrite (0x02000004, reg | bit_mask);

		/* Set enable mask */
		QicMmioRead (0x02000028, &reg);
		QicMmioWrite (0x02000028, reg | bit_mask);

		/* Set new data */
		QicMmioRead (0x02000000, &reg);
		QicMmioWrite (0x02000000, reg | bit_mask);
	}

	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query =UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_FLASH_CONTROL;
	xu_q.size = XU_FLASH_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_FLASH_CONTROL;
	xctrl.size = XU_FLASH_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 1;  //Quanta
#endif
	buf[0] = (QIC_XU3_FLASH_CONFIG << 5);
	buf[1] = 0;
	buf[2] = divider & 0xff;
	buf[3] = (divider & 0xff00) >> 8;
	buf[4] = mode & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicFlashSetSpiConfig Error, (%d)%s", errno, strerror(errno));
	} else {
	//	LOG_RET_PRINT(debug_xuctrl_str, "QicFlashSetSpiConfig Success");
	}
	return ret;
}



/*******************************************************************
QIC XU : Select control ID to Set
*******************************************************************/
int QicXuSet (unsigned int ctrlId, unsigned char* pBuf, unsigned int size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	int ret;

	if(ctrlId == XU_ENCODER_CONTROL)
		size = m_XuEncoderSize;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = ctrlId;
	xu_q.size = size;//(size > XU_MAX_SIZE) ? XU_MAX_SIZE : size;
	xu_q.data = pBuf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = ctrlId;
	xctrl.size = size;//(size > XU_MAX_SIZE) ? XU_MAX_SIZE : size;
	xctrl.data = pBuf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set XU Error, (%d)%s", errno, strerror(errno));
	} else {
	//	LOG_RET_PRINT(debug_xuctrl_str, "Set XU Success");
	}

	return ret;
}

/*******************************************************************
QIC XU : Select control ID to Get
*******************************************************************/
int  QicXuGet (unsigned int ctrlId, unsigned char* pBuf, unsigned int size)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	int ret = 0;
/*
	switch(ctrlId)
	{
		case XU_ENCODER_CONTROL:
			*sizeReturned = m_XuEncoderSize;
			break;

		case XU_MMIO_CONTROL:
			*sizeReturned = XU_MMIO_SIZE_DEF;
			break;

		case XU_FLASH_CONTROL:
			*sizeReturned = XU_FLASH_SIZE_DEF;
			break;

		case XU_MISC_CONTROL:
			*sizeReturned = XU_MISC_SIZE_DEF;
			break;

		case XU_TEST_CONTROL:
			*sizeReturned = XU_TEST_SIZE_DEF;
			break;
		case XU_AVC_ADVANCE_CONTROL:
			*sizeReturned = XU_AVC_ADVANCE_SIZE_DEF;
			break;
		case XU_AUDIO_CONTROL:
			*sizeReturned = XU_AUDIO_SIZE_DEF;
			break;
		case XU_SYSINFO_CONTROL:
			*sizeReturned = XU_SYSINFO_SIZE_DEF;
			break;

			default:
				*sizeReturned=XU_MAX_SIZE;
			break;	
	}
*/

	if (pBuf)
	{
	
/*new uvc mapping*/
		xu_q.query = UVC_GET_CUR ;
		xu_q.unit = QIC_UVC_XU_ID;
		xu_q.selector = ctrlId;
		xu_q.size = size;
		xu_q.data = pBuf;
/*new uvc mapping*/

		xctrl.unit = QIC_UVC_XU_ID;
		xctrl.selector =ctrlId;
		xctrl.size = size;
		xctrl.data = pBuf;
#ifdef ADD_FORCE
                xctrl.force = 0;  //Quanta
#endif
		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);

		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Get XU Error, (%d)%s", errno, strerror(errno));
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Get XU Success");
		}
	}

	return ret;
}



int QicSetFlipMode(unsigned char flip_v, unsigned char flip_h)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = (QIC_XU4_FLIP_MODE << 5);
	buf[1] = 0x00;
	if (flip_v)
		buf[0x01] |= 0x01;
	if (flip_h)
		buf[0x01] |= 0x02;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Flip Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Flip Success");
	}
	return ret;
}

int QicGetFlipMode(unsigned char *flip_v, unsigned char *flip_h)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = (QIC_XU4_FLIP_MODE << 5) | 0x01; // GET Command
	buf[1] = 0x00;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Flip Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Flip Success");
	}
	
	/* read */
	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Flip Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Flip Success");
	}
	
	if (buf[1] & 0x01)
		*flip_v = 1;
	else
		*flip_v = 0;

	if (buf[1] & 0x02)
		*flip_h = 1;
	else
		*flip_h = 0;

	return ret;
}

/*************************************************************************
QIC : Utilities
*************************************************************************/

int QicGetFirmwareVersion(FirmwareVersion_t *version)
{
	int ret;
	char szTemp[0x18];
	unsigned int image_max_size;
	QicHwVer_t qic_hw_version;
	
	QicGetHWVersion(&qic_hw_version);

	if(qic_hw_version==QIC_HWVERSION_QIC1806){
                image_max_size=QIC1816_FLASH_MAX_SIZE;
	}
	else if(qic_hw_version==QIC_HWVERSION_QIC1822){
                image_max_size=QIC1822_FLASH_MAX_SIZE;
	}
	else{
               image_max_size=FLASH_MAX_SIZE;
	}

	ret = QicFlashRead (QIC_FLASH_PID_VID_ADDR, (unsigned char*)szTemp, 0x18,image_max_size);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Firmware Version Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Firmware Version Success");
	}	

	if (!strncmp(&szTemp[0], "VID", 3))
		strncpy((char *)&(version->szVID), &szTemp[3], 5);
	else
		strncpy((char *)&(version->szVID), "FFFF\0", 5);

	if (!strncmp(&szTemp[8], "PID", 3))
		strncpy((char *)&(version->szPID), &szTemp[11], 5);
	else
		strncpy((char *)&(version->szPID), "FFFF\0", 5);

	if (!strncmp(&szTemp[16], "REV", 3))
		strncpy((char *)&(version->szREV), &szTemp[19], 5);
	else
		strncpy((char *)&(version->szREV), "FFFF\0", 5);

	return ret;
}



int QicSetSerialNumber( unsigned char *serial_number, unsigned char data_len)
{
	int ret=0;
	int j=0;

#ifdef QIC_SUPPORT_2ND_BL	
	unsigned char szTemp[QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE];
	unsigned char BufVerify[QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE];
#else		
	unsigned char szTemp[0x50];
	unsigned char *usb_dump, *BufVerify;
	
#endif
	unsigned int iSreialNumberIndex = 0, iDescStrSize = 0, i;
	QicHwVer_t qic_hw_version;
	unsigned int image_max_size;
	unsigned int usb_dump_size;

	QicGetHWVersion(&qic_hw_version);

	if(qic_hw_version==QIC_HWVERSION_QIC1806){
                image_max_size=QIC1816_FLASH_MAX_SIZE;
		usb_dump_size=QIC1816_FLASH_USB_MAX_SIZE;
		
	}
	else if(qic_hw_version==QIC_HWVERSION_QIC1822){
                image_max_size=QIC1822_FLASH_MAX_SIZE;
		usb_dump_size=QIC1822_FLASH_USB_MAX_SIZE;
	}
	else{
               image_max_size=FLASH_MAX_SIZE;
	}


	if ((serial_number == NULL)  || (data_len < 0))
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Invalid Argument)");
		return (ret = 1);
	}

	if (data_len > QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE)
	{
		LOG_RET_PRINT(debug_xuctrl_str, "serial number out of range, maximun length is 80");
		return (ret = 1);
	}

#ifdef QIC_SUPPORT_2ND_BL

		ret= QicFlashSectorErase(QIC1822_FLASH_SERIAL_NUMBER_ADDR);
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "serial number sector erase Error (Read String ), (%d)%s", errno, strerror(errno));

			return (ret = 1);
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "serial number sector erase Success");
		}
		
		ret = QicFlashRead(QIC1822_FLASH_SERIAL_NUMBER_ADDR, BufVerify, 0x50,image_max_size);

		for (i = 0; i < QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE; i++)
			{
				if (BufVerify[i] != 0xff)
				{
					
					printf("Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
					return (ret = 1);
				}	
			}

		memset(szTemp,0,QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE);

		szTemp[0]=0x53;
		szTemp[1]=0x00;
		szTemp[2]=0x4E;
		szTemp[3]=0x00;
		szTemp[4]=0x03;
		szTemp[5]=0x00;
		szTemp[6]=(data_len*2)+2;
		szTemp[7]=0x03;
		
		 j = data_len;

		while(j >= 0){
			strncpy(&szTemp[ 8+(j*2)],&serial_number[j], 1);
			//szTemp[ 8+1+(j*2)]=0;
			j--;
		}

		ret =  QicFlashWrite(QIC1822_FLASH_SERIAL_NUMBER_ADDR, szTemp, QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE ,image_max_size);
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Serial Number Error (Read String ), (%d)%s", errno, strerror(errno));
			return (ret = 1);
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Serial Number Success");
		}

		
		return ret;

#else
	usb_dump = calloc( 1, usb_dump_size);
	BufVerify= calloc( 1, usb_dump_size);

	if(usb_dump==NULL||BufVerify==NULL){
		LOG_RET_PRINT(debug_xuctrl_str, "allocat memory failed\n");
		free(usb_dump);
		free(BufVerify);
		return (ret=1);
	}

	ret=QicFlashRead (QIC1822_FLASH_USB_ADDR,  usb_dump, usb_dump_size,image_max_size);
	if(ret){
		LOG_RET_PRINT(debug_xuctrl_str, "usb dump failed\n");
		free(usb_dump);
		free(BufVerify);
		return (ret=1);
	}
	
	for(i=QIC1822_FLASH_USB_ADDR;i<usb_dump_size;i += FLASH_SECTOR_SIZE){
		ret= QicFlashSectorErase(i);
		if(ret){
			LOG_RET_PRINT(debug_xuctrl_str, "serial number sector erase Error (Read String ), (%d)%s", errno, strerror(errno));
			free(usb_dump);
			free(BufVerify);
			return (ret = 1);
		} 
	}

	ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, usb_dump_size,image_max_size);

		for (i = 0; i < usb_dump_size; i++)
		{
			if (BufVerify[i] != 0xff)
			{			
			printf("Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
			free(usb_dump);
			free(BufVerify);
			return (ret = 1);
			}	
		}

	usleep(10000);


	/* Read Device Descriptor */
	iSreialNumberIndex = usb_dump[SF_ADDR_DEVICE_DESC+16];

	/* Read String Descriptor */
	for (i=4; i<sizeof(szTemp);i++)
	{
		if (usb_dump[SF_ADDR_STRING_DESC+i])
		{
			iDescStrSize = i;
			break;
		}
	}

	if (data_len == 0)
	{
		usb_dump[SF_ADDR_DEVICE_DESC+16] = 0;
	}
	else
	{
		if (data_len > ((iDescStrSize-2)/2))
			data_len = (iDescStrSize-2)/2;
		
		printf("iDescStrSize=%d, data_len=%d\n",iDescStrSize,data_len);


		usb_dump[SF_ADDR_DEVICE_DESC+16] = 3;

		usb_dump[ SF_ADDR_STRING_DESC + iDescStrSize*3 ] = 2 + data_len*2;
		usb_dump[ SF_ADDR_STRING_DESC + iDescStrSize*3 + 1 ] = 0x03;

		
		 j = data_len;

		while(j >= 0){
			strncpy(&usb_dump[ SF_ADDR_STRING_DESC + iDescStrSize*3 + 2 +j*2], &serial_number[j], 1);
			j--;
		}

		//memset (&usb_dump[ SF_ADDR_STRING_DESC + iDescStrSize*3 + 2 + data_len*2], 0, iDescStrSize - 2 - data_len*2);

	}

	ret =  QicFlashWriteUSBIMG(QIC1822_FLASH_USB_ADDR, usb_dump, usb_dump_size,image_max_size);
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Serial Number Error (Read String ), (%d)%s", errno, strerror(errno));
			
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Set Serial Number Success");
		}


	free(usb_dump);
	free(BufVerify);


	return ret;

#endif

}



/*******************************************************************
QIC Utility : Get Serial Number
*******************************************************************/
int QicGetSerialNumber( char *serial_number, int data_len, int *bytes_returned)
{
	int ret=0;

#ifdef QIC_SUPPORT_2ND_BL	
	char szTemp[QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE];
#else
	char szTemp[0x50];
#endif
	unsigned int iSreialNumberIndex = 0, iDescStrSize = 0, i;
	QicHwVer_t qic_hw_version;
	unsigned int image_max_size;
	
	QicGetHWVersion(&qic_hw_version);

	if(qic_hw_version==QIC_HWVERSION_QIC1806){
                image_max_size=QIC1816_FLASH_MAX_SIZE;
	}
	else if(qic_hw_version==QIC_HWVERSION_QIC1822){
                image_max_size=QIC1822_FLASH_MAX_SIZE;
	}
	else{
               image_max_size=FLASH_MAX_SIZE;
	}


	if ((serial_number == NULL) || (bytes_returned == NULL) || (data_len < 0))
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Invalid Argument)");
		return (ret = 1);
	}


#ifdef QIC_SUPPORT_2ND_BL
		ret = QicFlashRead (QIC1822_FLASH_SERIAL_NUMBER_ADDR,  (unsigned char*)szTemp, QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE ,image_max_size);
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Read String ), (%d)%s", errno, strerror(errno));
			return (ret = 1);
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Success");
		}

		if (szTemp[0] != 0)
		{

			unsigned int len = szTemp[6]-2;
			int i = 0, length;

			if(len/2 > (unsigned int)data_len)
			{
				len = data_len;
			} else {
				len = len/2; /*len must be even*/
			}

			length = len;

			while(length > 0)
			{
				strncpy(&serial_number[i], &szTemp[ 8+i*2], 1);
				i++;
				length--;

			}
			serial_number[len] = '\0';

			*bytes_returned = len;

		}
		return ret;



#else
	/* Read Device Descriptor */
	ret = QicFlashRead (SF_ADDR_DEVICE_DESC+16,  (unsigned char*)szTemp, 0x1,image_max_size);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Read Device Descriptor), (%d)%s", errno, strerror(errno));
		return (ret = 1);
	}

	iSreialNumberIndex = szTemp[0];

	if (iSreialNumberIndex == 0)
	{
		*bytes_returned = 0;
		return ret;
	}

	/* Read String Descriptor */
	ret = QicFlashRead (SF_ADDR_STRING_DESC,  (unsigned char*)szTemp, sizeof(szTemp),image_max_size);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Read String Descriptor #1), (%d)%s", errno, strerror(errno));
		return (ret = 1);
	}

	for (i=4; i<sizeof(szTemp);i++)
	{
		if (szTemp[i])
		{
			iDescStrSize = i;
			break;
		}
	}

	if (iDescStrSize != 0)
	{
		ret = QicFlashRead (SF_ADDR_STRING_DESC+(iDescStrSize*iSreialNumberIndex),  (unsigned char*)szTemp, iDescStrSize,image_max_size);
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (Read String Descriptor #2), (%d)%s", errno, strerror(errno));
			return (ret = 1);
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Success");
		}

		if (szTemp[0] != 0)
		{
			unsigned int len = szTemp[0]-2;
			int i = 0, length;

			if(len/2 > (unsigned int)data_len)
			{
				len = data_len;
			} else {
				len = len/2; /*len must be even*/
			}

			length = len;

			while(length > 0)
			{
				strncpy(&serial_number[i], &szTemp[2 + i*2], 1);
				i++;
				length--;
			}
			serial_number[len] = '\0';
			*bytes_returned = len;
#if 0
			wchar_t wstr_desc[0x50];
			if (len/2+1 > (unsigned int)data_len)
				len = (data_len-1)*2;
			if (len > sizeof(wstr_desc)-2)
				len = sizeof(wstr_desc)-2;

			memcpy (wstr_desc, (wchar_t *)&szTemp[2], len);
			wstr_desc[len/2] = '\0';
			wcstombs(serial_number, wstr_desc, len/2+1);
			*bytes_returned = len/2+1;
#endif
		}
	}
	else
	{
		*bytes_returned = 0;
		LOG_RET_PRINT(debug_xuctrl_str, "Get Serial Number Error (No Serial Number)");
		return (ret = 1);
	}

	return ret;

#endif

}


int QicGetSvnVersion(unsigned int *version)
{
	int ret;
	char szTemp[5]={0};
	QicHwVer_t qic_hw_version=0;
	unsigned int image_max_size=0;
	
	QicGetHWVersion(&qic_hw_version);

	if(qic_hw_version==QIC_HWVERSION_QIC1806){
                image_max_size=QIC1816_FLASH_MAX_SIZE;
	}
	else if(qic_hw_version==QIC_HWVERSION_QIC1822){
                image_max_size=QIC1822_FLASH_MAX_SIZE;
	}
	else{
               image_max_size=FLASH_MAX_SIZE;
	}

		
	ret = QicFlashRead (QIC_FLASH_SVN_ADDR,  (unsigned char*)szTemp, 4,image_max_size);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get SVN Version Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get SVN Version Success");
	}

	*version = atoi(szTemp);
	return ret;
}

/*******************************************************************
QIC Utility : Set LED Mode
*******************************************************************/
int QicSetLedMode(unsigned char mode)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_LED_CONTROL;
	buf[1] = C5_LED_SET_STATUS;
	buf[2] = mode;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED mode Success");
	}
	return ret;
	
}

/*******************************************************************
QIC Utility : Get LED Mode
*******************************************************************/
int QicGetLedMode(unsigned char *mode)
{
struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_LED_CONTROL; // GET Command
	buf[1] = C5_LED_GET_STATUS;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED mode Success");
	}
	
	/* read */
	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get LED mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get LED mode Success");
	}
	
	*mode = buf[2];

	return ret;
	
}


/*******************************************************************
QIC Utility : Set LED Brightness
*******************************************************************/
int QicSetLEDBrightness(unsigned char brightness)
{

	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_LED_BRIGHTNESS_CONTROL;
	buf[1] = C5_LED_SET_STATUS;
	buf[2] = brightness;
     
	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED brightness Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED brightness Success");
	}
	return ret;
}
 
/*******************************************************************
QIC Utility : Get LED Brightness
*******************************************************************/
int QicGetLEDBrightness(unsigned char *brightness)
{

struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_LED_BRIGHTNESS_CONTROL; // GET Command
	buf[1] = C5_LED_GET_STATUS;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED brightness Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set LED brightness Success");
	}
	
	/* read */
	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get LED brightness Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get LED brightness Success");
	}
	
	*brightness = buf[2];

	return ret;
}


int QicSetPanTilt(signed short pan, signed short tilt)
{
        struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
        unsigned char buf[XU_MAX_SIZE];
        int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

        xctrl.unit = QIC_UVC_XU_ID;
        xctrl.selector = XU_TEST_CONTROL;
        xctrl.size = XU_TEST_SIZE_DEF;
        xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;
#endif
        buf[0] = C5_PAN_TILT_CONTROL;
        buf[1] = C5_PAN_TILT_CONTROL_SET_STATUS;
        buf[2] = pan & 0xff;
        buf[3] = (pan >> 8) & 0xff;
        buf[4] = tilt & 0xff;
        buf[5] = (tilt >> 8) & 0xff;

        /*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
        ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
        if(ret) {
                LOG_RET_PRINT(debug_xuctrl_str, "Set pan tilt Error, (%d)%s", errno, strerror(errno));
        } else {
                LOG_RET_PRINT(debug_xuctrl_str, "Set pan tilt Success");
        }
        return ret;
}

int QicGetPanTilt(signed short *pan, signed short *tilt)
{
        struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
        unsigned char buf[XU_MAX_SIZE];
        int ret;

/*new uvc mapping*/
	//xu_q.query = UVC_GET_CUR ;
	xu_q.query = UVC_SET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

        xctrl.unit = QIC_UVC_XU_ID;
        xctrl.selector = XU_TEST_CONTROL;
        xctrl.size = XU_TEST_SIZE_DEF;
        xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;
#endif
        buf[0] = C5_PAN_TILT_CONTROL;
        buf[1] = C5_PAN_TILT_CONTROL_GET_STATUS;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
        ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);
        
        if(ret) {
                LOG_RET_PRINT(debug_xuctrl_str, "Set pan tilt Error, (%d)%s", errno, strerror(errno));
        } else {
                LOG_RET_PRINT(debug_xuctrl_str, "Set pan tilt Success");
        }

/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
        ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);
        
        if(ret) {
                LOG_RET_PRINT(debug_xuctrl_str, "Get pan tilt Error, (%d)%s", errno, strerror(errno));
        } else {
                LOG_RET_PRINT(debug_xuctrl_str, "Get pan tilt Success");
        }
        *pan = buf[2] + (buf[3]<<8);
        *tilt = buf[4] + (buf[5]<<8);

        return ret;
}


/*******************************************************************
QIC Utility : Set Shutter Mode
*******************************************************************/
int QicSetShutterMode(unsigned char mode)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_SHUTTER_CONTROL;
	buf[1] = C5_SHUTTER_SET_STATUS;
	buf[2] = mode;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Shutter mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Shutter mode Success");
	}
	return ret;
	
}

/*******************************************************************
QIC Utility : Get Shutter Mode
*******************************************************************/
int QicGetShutterMode(unsigned char *mode)
{
struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_TEST_CONTROL;
	xu_q.size = XU_TEST_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_TEST_CONTROL;
	xctrl.size = XU_TEST_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
	xctrl.force = 0;  //Quanta
#endif
	buf[0] = C5_SHUTTER_CONTROL; // GET Command
	buf[1] = C5_SHUTTER_GET_STATUS;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Shutter mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Shutter mode Success");
	}
	
	/* read */
	/*try new query api first*/
	xu_q.query = UVC_GET_CUR ;
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);

	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Shutter mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Shutter mode Success");
	}
	
	*mode = buf[2];

	return ret;
	
}


/*******************************************************************
QIC Utility : Set MJPEG QP Factor Value
*******************************************************************/
int QicSet_MJPEG_QP_Value(unsigned char value)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = C5_MJPEG_QP_CONTROL;
	buf[1] = C5_MJPEG_QP_CONTROL_SET_STATUS;
	buf[2] = value;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set MJPEG QP Value Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set MJPEG QP Value Success");
	}
	return ret;

}

/*******************************************************************
QIC Utility : Get MJPEG QP Factor Value
*******************************************************************/
int QicGet_MJPEG_QP_Value(unsigned char *value)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = C5_MJPEG_QP_CONTROL;
	buf[1] = C5_MJPEG_QP_CONTROL_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set MJPEG QP Value Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set MJPEG QP Value Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get MJPEG QP Value Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get MJPEG QP Value Success");
	}

        *value = buf[2];

	return ret;

}

/*******************************************************************
QIC Utility XU5: Set Cropped image
*******************************************************************/
int QicSetCropped(unsigned char crop_enable, CROPRECT crop_window)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = C5_CROPPED_IMAGE;
	buf[1] = C5_CROPPED_IMAGE_SET_STATUS;
	buf[2] = crop_enable;
	buf[3] = crop_window.x&0xFF;
	buf[4] = (crop_window.x >>8)&0xFF;
	buf[5] = crop_window.y&0xFF;
	buf[6] = (crop_window.y >>8)&0xFF;
	buf[7] = crop_window.Wx&0xFF; 
	buf[8] = (crop_window.Wx >>8)&0xFF;
	buf[9] = crop_window.Wy&0xFF; 
	buf[10] = (crop_window.Wy >>8)&0xFF;

	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Cropped Image Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Cropped Image Success");
	}

	return ret;
}

/*******************************************************************
QIC Utility XU5: Set Trimming
*******************************************************************/
int QicSetTrimming(unsigned char trim_enable, TRIMRECT trim_window)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = C5_TRIMMING_CONTROL;
	buf[1] = C5_TRIMMING_SET_STATUS;
	buf[2] = trim_enable;
	buf[3] = trim_window.x&0xFF;
	buf[4] = (trim_window.x >>8)&0xFF;
	buf[5] = trim_window.y&0xFF;
	buf[6] = (trim_window.y >>8)&0xFF;
	buf[7] = trim_window.Wx&0xFF; 
	buf[8] = (trim_window.Wx >>8)&0xFF;
	buf[9] = trim_window.Wy&0xFF; 
	buf[10] = (trim_window.Wy >>8)&0xFF;

	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Trimming  Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Trimming Success");
	}

	return ret;
}


/*******************************************************************
QIC Utility XU5: Get Trimming
*******************************************************************/
int QicGetTrimming(unsigned char* trim_enable, TRIMRECT* trim_window)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = C5_TRIMMING_CONTROL;
	buf[1] = C5_TRIMMING_GET_STATUS;


	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Trimming  Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Trimming Success");
	}

	      ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Trimming Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Trimming Success");
	}


	*trim_enable= buf[2];
	
	trim_window->x= buf[3]+(buf[4]<<8);
	trim_window->y=buf[5]+(buf[6]<<8);
	trim_window->Wx=buf[7]+(buf[8]<<8);
	trim_window->Wy=buf[9]+(buf[10]<<8);


	return ret;
}


/*******************************************************************
QIC Utility XU5: Get ALS
*******************************************************************/
int QicGetALS(unsigned short *als_value)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_ALS_CONTROL;
	buf[1] = C5_ALS_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set ALS Value control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set ALS Value control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get ALS Value Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get ALS Value Success");
	}

        *als_value = buf[2] + (buf[3]<<8);


	return ret;

}


/*******************************************************************
QIC Utility XU5: Set Motor Control
*******************************************************************/
int QicSetMotorControl(unsigned char control, unsigned char direction)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_MOTOR_CONTROL;
	buf[1] = C5_MOTOR_SET_STATUS;
    buf[2] = control; 				/*motor control. 0 :stopping motor, 1: rotating motor*/
	buf[3] = direction; 			/*motor rotating directoin. 0: move to opening, 1: move to closeing */
	buf[4] = 1; 					/* Auto stop or not, 1 for yes, 0 for not. */ 

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Motor control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Motor control Success");
	}

   

	return ret;

}

/*******************************************************************
QIC Utility XU5: Get Motor Current Status
*******************************************************************/
int QicGetMotorStatus(unsigned char *motor_status)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_MOTOR_CONTROL;
	buf[1] = C5_MOTOR_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Motor control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Motor control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Motor status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Motor status Success");
	}

        *motor_status = buf[2];


	return ret;

}


/*******************************************************************
QIC Utility XU5: Set Advanced Motor Control
*******************************************************************/
int QicSetAdvMotorControl(AdvMotorCtlMode_e mode, unsigned char set_number,AdvMotorCtl_t setting)
{

	unsigned char buf[XU_MAX_SIZE];
	int i=0;
	int j=0;
	int ret;	

	memset(buf, 0, XU_MAX_SIZE);
		
	buf[0] = C5_ADVANCE_MOTOR_CONTROL;
	buf[1] = C5_ADVANCE_MOTOR_SET_STATUS;
        buf[2] = mode; 							
	buf[3] = setting.TotalPositionNumbers; 			
	buf[4] = setting.StartElementIndex;

	if(set_number){
		for(i=0;i<set_number;i++){
			
			buf[5+j]=setting.PosArray[i].pan;
			buf[6+j]=(setting.PosArray[i].pan>>8)&0xFF;
			buf[7+j]=setting.PosArray[i].tilt;
			buf[8+j]=(setting.PosArray[i].tilt>>8)&0xFF;
			j=j+4;	
	
		}
	}

//	printf(" Set %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d.%d,%d,%d \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10],
//		 buf[11], buf[12], buf[13], buf[14], buf[15], buf[16], buf[17], buf[18], buf[19]);

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Advance Motor control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Advance Motor control Success");
	}

   

	return ret;

}

/*******************************************************************
QIC Utility XU5: Get Advanced Motor Current Position
*******************************************************************/
int QicGetAdvMotorPosition(MotorPosition_t *postion)
{

	unsigned char buf[XU_MAX_SIZE];

	int ret;	

	memset(buf, 0, XU_MAX_SIZE);
	
	buf[0] = C5_ADVANCE_MOTOR_CONTROL;
	buf[1] = C5_ADVANCE_MOTOR_GET_STATUS;
	buf[2] = ADV_MOTOR_SINGLE_GET;
        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Advance Motor Positions Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Advance Motor Positions Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Advance Motor Positions Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Advance Motor Positions Success");
	}
	
		postion->pan=buf[5]+(buf[6]<<8);
		postion->tilt=buf[7]+(buf[8]<<8);	

printf(" Get %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d.%d,%d,%d \n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10],
		 buf[11], buf[12], buf[13], buf[14], buf[15], buf[16], buf[17], buf[18], buf[19]);

//printf(" Get %d,%d \n", buf[6]<<8, buf[8]<<8);


	return ret;

}


/*******************************************************************
QIC Utility XU5: Set IR Control
*******************************************************************/
int QicSetIRControl(unsigned char mode)
{
/*
//IR ALS Mode
typedef enum {
	ALS_AUTO_IR_OFF					= 0x00,   //ALS auto mode off, IR/ICR disable mode
	ALS_AUTO_OFF_IR_ON				= 0x01,   //ALS auto mode off, IR/ICR enable mode
	ALS_AUTO_ON					= 0x02   //ALS auto mode on mode
} IRALSCtlMode_e;
*/
	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_IR_CONTROL;
	buf[1] = C5_IR_SET_STATUS;
        buf[2] = mode; 				


        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set IR control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set IR control Success");
	}

   

	return ret;

}

/*******************************************************************
QIC Utility XU5: Get IR control Current Status
*******************************************************************/
int QicGetIRStatus(unsigned char *ir_status, unsigned char *ALS_status)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_IR_CONTROL;
	buf[1] = C5_IR_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set IR control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set IR control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get IR status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get IR status Success");
	}

        *ir_status = buf[2]&0x01;
        *ALS_status = (buf[2]>>1)&0x01;

	return ret;

}


/*******************************************************************
QIC Utility XU5: Get Support Capability List Control
*******************************************************************/
int QicGetCapabilityList(unsigned char *support_number, unsigned int *support_list)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_CAPABILITY_LIST_CONTROL;
	buf[1] = C5_CAPABILITY_LIST_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Capability list control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Capability list control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Capability list Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Capability list Success");
	}

        *support_number = buf[2];
	
	*support_list = buf[3] + (buf[4] << 8)+(buf[5] << 16)+(buf[6] << 24);


	return ret;

}


/*******************************************************************
QIC Utility XU5: Get Sensor Information
*******************************************************************/
int QicGetSensorAEInfo(unsigned short *AE_statistic)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_SENSOR_INFO_CONTROL;
	buf[1] = C5_SENSOR_INFO_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Sensor AE info control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set ensor AE info control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get ensor AE info Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get ensor AE info Success");
	}

        *AE_statistic = buf[2]+(buf[3]<<8);

	return ret;

}

/*******************************************************************
QIC Utility XU5: Set check lock streaming Control
*******************************************************************/
int QicSetLockStream(unsigned char lock)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_CHECK_LOCK_STREAM_CONTROL;
	buf[1] = C5_CHECK_LOCK_STREAM_SET_STATUS;
        buf[2] = lock; 				/* control. 0 :disable Lock, 1: enable Lock streaming*/


        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Lock control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Lock control Success");
	}

   

	return ret;

}

/*******************************************************************
QIC Utility XU5: Get check lock streaming control Current Status
*******************************************************************/
int QicGetCheckLockStreamStatus(unsigned char *isLock, unsigned char *isStream)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	
	
	buf[0] = C5_CHECK_LOCK_STREAM_CONTROL;
	buf[1] = C5_CHECK_LOCK_STREAM_GET_STATUS;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Lock control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Lock control Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Lock Stream status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Lock Stream status Success");
	}

        *isLock = buf[2];            /*0: unlock, 1:lock */
	 *isStream=buf[3];        /*0: QIC is not steraming, 1: QIC is streaming*/

	return ret;

}



/*******************************************************************
QIC XU2 : Set H.264 Encoder Entropy 
*******************************************************************/
int QicSetEncoderEntropy (EncoderEntropy_t Entropy){
struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;
/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/
#ifdef ADD_FORCE
  	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
	
	buf[0] = QIC_XU1_ENCODER_ENTROPY; 
	buf[0x11] = Entropy & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Entropy Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Entropy Success");
	}
	return ret;
}

/*******************************************************************
QIC XU2 : Get H.264 Encoder Entropy 
*******************************************************************/
int QicGetEncoderEntropy (EncoderEntropy_t *Entropy)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_GET_CUR ;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_ENCODER_CONTROL;
	xu_q.size = m_XuEncoderSize;
	xu_q.data = buf;
/*new uvc mapping*/

#ifdef ADD_FORCE
   	xctrl.force = 0;  //Quanta
#endif
	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_ENCODER_CONTROL;
	xctrl.size = m_XuEncoderSize;
	xctrl.data = buf;
	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Entropy Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Entropy Success");
	}
	
	*Entropy = buf[0x11];

	return ret;
}


/*******************************************************************
QIC Utility : Set Calibrate Mode
*******************************************************************/
int QicSetCalibrateMode(CalibratMode_t mode)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	buf[0] = QIC_XU4_CALIBRATE_MODE << 5;
	if (mode == CALIBRATE_NORMAL)
	{
		buf[1] = 1;	// AE
		buf[2] = 1;	// AWB
	}
	else if (mode == CALIBRATE_AE_DISABLE)
	{
		buf[1] = 0;	// AE
		buf[2] = 1;	// AWB
	}
	else if (mode == CALIBRATE_AWB_DISABLE)
	{
		buf[1] = 1;	// AE
		buf[2] = 0;	// AWB
	}
	else if (mode == CALIBRATE_AEAWB_DISABLE)
	{
		buf[1] = 0;	// AE
		buf[2] = 0;	// AWB
	}
	else
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Set Calibrate Mode Error (Invalid Argument)");
		return (ret = 1);
	}

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Calibrate Mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Calibrate Mode Success");
	}
	return ret;
	
}

/*******************************************************************
QIC Utility : Set Special Effect
*******************************************************************/
int QicSetSpecialEffect(SpecialEffect_t effect)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	if (effect > EFFECT_SKETCH)
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Set Special Effect Error (Invalid Argument)");
		return (ret = 1);
	}

	buf[0] = QIC_XU4_SPECIAL_EFFECT << 5;
	buf[1] = effect & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Special Effect Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Special Effect Success");
	}
	return ret;
}

/*******************************************************************
QIC Utility : Set White Balance Component
*******************************************************************/
int QicSetWBComp(unsigned int r_gain, unsigned int g_gain, unsigned int b_gain)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	buf[0x00] = (QIC_XU4_AEAWB_CUSTOM << 5) | QIC_XU4_AEAWB_WB_COMP;
	buf[0x01] = r_gain & 0xff;
	buf[0x02] = (r_gain >> 8) & 0xff;
	buf[0x03] = g_gain & 0xff;
	buf[0x04] = (g_gain >> 8) & 0xff;
	buf[0x05] = b_gain & 0xff;
	buf[0x06] = (b_gain >> 8) & 0xff;
	buf[0x07] = ((r_gain >> 16) & 0x03) | (((g_gain >> 16) & 0x03) << 2) | (((b_gain >> 16) & 0x03) << 4);
	buf[0x08] = 0;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WB Comp Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WB Comp Success");
	}
	return ret;
}

/*******************************************************************
QIC Utility : Set Exposure Component
*******************************************************************/
int QicSetExpComp(unsigned int exp_time, unsigned short exp_gain)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	buf[0x00] = (QIC_XU4_AEAWB_CUSTOM << 5) | QIC_XU4_AEAWB_EXP_COMP;
	buf[0x09] = exp_time & 0xff;
	buf[0x0A] = (exp_time >> 8) & 0xff;
	buf[0x0B] = (exp_time >> 16) & 0xff;
	buf[0x0C] = (exp_time >> 24) & 0xff;
	buf[0x0D] = exp_gain & 0xff;
	buf[0x0E] = (exp_gain >> 8) & 0xff;
	
	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Exp Comp Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Exp Comp Success");
	}
	return ret;
}

/*******************************************************************
QIC Utility : Set AE Speed
*******************************************************************/
int QicSetAeSpeed(unsigned short time_step, unsigned char gain_step)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	buf[0x00] = (QIC_XU4_AEAWB_CUSTOM << 5) | QIC_XU4_AEAWB_AE_STEP;
	buf[0x0f] = time_step & 0xff;
	buf[0x10] = (time_step >> 8) & 0xff;
	buf[0x11] = gain_step & 0xff;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Exp Comp Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Exp Comp Success");
	}
	return ret;
}

/*******************************************************************
QIC Utility : Set AE Speed
*******************************************************************/
int QicSetEncoderOption( unsigned char encoder_option)
{
#if 1
	LOG_RET_PRINT(debug_xuctrl_str, "Set Encoder Option Error (Function not Implemented)");
	return 1;

#endif
}

/*************************************************************************
QIC XU4 : MISC Control
*************************************************************************/
/*******************************************************************
QIC XU4 : Set AES Encryptor Key
*******************************************************************/
int QicEncryptorSetKey(char *key, EncyptKeyMode_t key_mode)
{
	unsigned int len;
	unsigned int buf_offset = 0;
	unsigned int written_size, key_size = 0;

	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret = 0;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	switch (key_mode)
	{
		case ENCRYPT_KEY_MODE_256:
			buf_offset = 0;
			key_size = 32;
			break;
		case ENCRYPT_KEY_MODE_192:
			buf_offset = 8;
			key_size = 24;
			break;
		case ENCRYPT_KEY_MODE_128:
			buf_offset = 16;
			key_size = 16;
			break;
	}

	len = key_size;

	/* set key*/
	written_size = 0;
	while (len)
	{
		unsigned int desired_len;

		if (len > 25)
			desired_len = 25;
		else
			desired_len = len;

		/* read file and send request */
		memcpy (&buf[2], key+written_size, desired_len);
		buf[0] = (QIC_XU4_ENCRYPT_WRITE << 5) | desired_len;
		buf[1] = buf_offset;

		len -= desired_len;
		buf_offset += desired_len;
		written_size += desired_len;

		/*try new query api first*/
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Set AES Encryptor Key Error, (%d)%s", errno, strerror(errno));
			return ret;
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Set AES Encryptor Key Success");
		}
	}

	return ret;
}

/*******************************************************************
QIC XU4 : Get AES Encryptor Key
*******************************************************************/
int QicEncryptorGetKey(char *key, EncyptKeyMode_t key_mode)
{
	unsigned int len;
	unsigned int offset = 0;
	unsigned int gotten_size, key_size;

	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

	if(key == NULL) {
		LOG_RET_PRINT(debug_xuctrl_str, "Encryptor Get Key Error (Invalid Argument)");
		return (ret = 1);
	}

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	switch (key_mode)
	{
		case ENCRYPT_KEY_MODE_256:
			offset = 0;
			key_size = 32;
			break;
		case ENCRYPT_KEY_MODE_192:
			offset = 8;
			key_size = 24;
			break;
		case ENCRYPT_KEY_MODE_128:
			offset = 16;
			key_size = 16;
			break;
	}

	/* set header and read address */
	buf[0] = (QIC_XU4_ENCRYPT_READ << 5) | FLASH_MAX_DATA_LEN;
	buf[1] = offset;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get AES Encryptor Key Error (Set header), (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get AES Encryptor Key Success (Set header)");
	}

	/* read */
	len = 0;
	gotten_size = 0;
	do
	{
		/*try new query api first*/
		xu_q.query = UVC_GET_CUR ;
		ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
		if(ret)
		ret = ioctl (m_vd, UVCIOC_CTRL_GET, &xctrl);


		if(ret) {
			LOG_RET_PRINT(debug_xuctrl_str, "Get AES Encryptor Key Error, (%d)%s", errno, strerror(errno));
			return ret;
		} else {
			LOG_RET_PRINT(debug_xuctrl_str, "Get AES Encryptor Key Success");
		}

		len = buf[0] & 0x1F;

		memcpy (key+gotten_size, &buf[1], len);
		gotten_size += len;

	} while (len == 26);


	return ret;
}

/*******************************************************************
QIC XU4 : Set AES Encryptor Key Mode
*******************************************************************/
int QicEncryptorSetConfig(int enable, EncyptKeyMode_t key_mode)
{
	struct uvc_xu_control xctrl;
	struct uvc_xu_control_query xu_q;
	unsigned char buf[XU_MAX_SIZE];
	int ret;

/*new uvc mapping*/
	xu_q.query = UVC_SET_CUR;
	xu_q.unit = QIC_UVC_XU_ID;
	xu_q.selector = XU_MISC_CONTROL;
	xu_q.size = XU_MISC_SIZE_DEF;
	xu_q.data = buf;
/*new uvc mapping*/

	xctrl.unit = QIC_UVC_XU_ID;
	xctrl.selector = XU_MISC_CONTROL;
	xctrl.size = XU_MISC_SIZE_DEF;
	xctrl.data = buf;
#ifdef ADD_FORCE
        xctrl.force = 0;  //Quanta
#endif
	if (enable)
		enable = 1;

	/* set encrypt config */
	buf[0] = (QIC_XU4_ENCRYPT_CONFIG << 5);
	buf[1] = enable;
	buf[2] = key_mode;

	/*try new query api first*/
	ret = ioctl(m_vd, UVCIOC_CTRL_QUERY, &xu_q);
	
	if(ret)
	ret = ioctl (m_vd, UVCIOC_CTRL_SET, &xctrl);

	
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set AES Encryptor Key Mode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set AES Encryptor Key Mode Success");
	}
	return ret;
}


/*******************************************************************
QIC Utility XU5: Get Encoding stream(H264/VP8) Descriptor frame number
*******************************************************************/
int QicEncoderGetNumberOfCapabilities(unsigned char *count)
{

	unsigned char buf[XU_MAX_SIZE];
	int index=0;
	int ret;	
	
	buf[0] = C5_H264_DESC;
	buf[1] = C5_FROMAT_DESC;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set FrameNumber Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set FrameNumber Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get FrameNumber Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get FrameNumber Success");
	}

	if (buf[2] == 0x01)	{
		*count = buf[3];
	} else {
		*count = 0;
		LOG_RET_PRINT(debug_xuctrl_str, "Get Number Of Capabilities Error (Generic Failure)");
		return (ret = 1);
	}

	return ret;

}


/*******************************************************************
QIC Utility XU5: Get Encoding stream(H264/VP8) Descriptor
*******************************************************************/
int QicEncoderGetStreamCaps(unsigned char index, EncoderCapability_t *capability)
{

	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	if ((index < 0) || (index > ENCODER_CAPS_MAXSIZE-1))
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get Stream Capabilities Error (Out of Range)");
		return (ret = 1);
	}

	if (capability == NULL)
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get Stream Capabilities Error (Invalid Argument)");
		return (ret = 1);
	}
	
	buf[0] = C5_H264_DESC;
	buf[1] = index+1;

        ret = QicXuSet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set  Stream Capabilities Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set  Stream Capabilities Success");
	}

        ret = QicXuGet(XU_TEST_CONTROL, buf, XU_TEST_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream Capabilities Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get  Stream Capabilities Success");
	}

     	if (buf[2] == 0x02)
	{
		capability->usWidth = buf[3] + (buf[4] << 8);
		capability->usHeight = buf[5] + (buf[6] << 8);
		capability->uiMinBitrate = buf[7] + (buf[8] << 8) + (buf[9] << 16) + (buf[10] << 24);
		capability->uiMaxBitrate = buf[11] + (buf[12] << 8) + (buf[13] << 16) + (buf[14] << 24);
		capability->ucMinFramerate = buf[15];
		capability->ucMaxFramerate = buf[16];
	}
	else
	{
		capability->usWidth = 0;
		capability->usHeight = 0;
		capability->uiMinBitrate = 0;
		capability->uiMaxBitrate = 0;
		capability->ucMinFramerate = 0;
		capability->ucMaxFramerate = 0;

		LOG_RET_PRINT(debug_xuctrl_str, "Get Stream Capabilities Error (Generic Failure)");
		return (ret = 1);
	}

	return ret;

}


/*******************************************************************
QIC Utility XU5: Set WUSB Control
*******************************************************************/
int QicSetWUSBWired(void)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;

	buf[0] = C5_WUSB_CONTROL;
	buf[1] = C5_WUSB_SET_WIRED;

	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Wired  Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Wired Success");
	}

	return ret;
}

int QicSetWUSBWireless(void)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;

	buf[0] = C5_WUSB_CONTROL;
	buf[1] = C5_WUSB_SET_WIRELESS;

	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Wireless  Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Wireless Success");
	}

	return ret;
}

int QicSetWUSBDisableAutoSwitch(void)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;

	buf[0] = C5_WUSB_CONTROL;
	buf[1] = C5_WUSB_SET_DISABLE_AUTOSWITCH;

	ret = QicXuSet(XU_TEST_CONTROL, buf,XU_TEST_SIZE_DEF);
	if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Disable Auto Switch Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set WUSB Disable Auto Switch Success");
	}

	return ret;
}

/*******************************************************************/
/*XU control 6: AVC Advance Controls*/
/*AVC Image Mirror Control setting*/
/* setting 1/0 to on/off*/
/*******************************************************************/
int QicSetMirror(unsigned char mirror)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = QIC_XU6_MIRROR_CONTROL;
	buf[1] = QIC_XU6_MIRROR_SET_STATUS;
	buf[2] = mirror;

 	ret = QicXuSet(XU_AVC_ADVANCE_CONTROL, buf,XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set mirror Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set mirror Status Success");
	}
	return ret;
	
}


/*******************************************************************/
/*XU control 6: AVC Advance Controls*/
/*AVC Image Mirror Control Getting*/
/*return value:*/
/*Bit 0 : Preview image mirror*/
/*Bit 1 : Encoded image mirror*/

/*******************************************************************/
int QicGetMirror(unsigned char *mirror)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = QIC_XU6_MIRROR_CONTROL; // GET Command
	buf[1] = QIC_XU6_MIRROR_GET_STATUS;

       ret = QicXuSet(XU_AVC_ADVANCE_CONTROL, buf,XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set mirror Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set mirror Status Success");
	}

        ret = QicXuGet(XU_AVC_ADVANCE_CONTROL, buf, XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get mirror Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get mirror Status Success");
	}


	*mirror = buf[2];

	return ret;
	
}


/*******************************************************************/
/*XU control 6: AVC Advance Controls*/
/*AVC Rate Control setting*/
/*******************************************************************/
int QicVideoSetRateControl(EncoderRateControlMode_t rate_control_type, unsigned char min_QP_value, unsigned char max_QP_value)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = QIC_XU6_RATE_CONTROL;
	buf[1] = QIC_XU6_SET_STATUS;
	buf[2] = rate_control_type;
        buf[3] = min_QP_value;
        buf[4] = max_QP_value;

 	ret = QicXuSet(XU_AVC_ADVANCE_CONTROL, buf,XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Rate Control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Rate Control  Success");
	}
	return ret;
	
}


/*******************************************************************/
/*XU control 6: AVC Advance Controls*/
/*AVC Rate Control Getting*/
/*******************************************************************/
int QicVideoGetRateControl(EncoderRateControlMode_t *rate_control_type, unsigned char *min_QP_value, unsigned char *max_QP_value)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = QIC_XU6_RATE_CONTROL; // GET Command
	buf[1] = QIC_XU6_GET_STATUS;

       ret = QicXuSet(XU_AVC_ADVANCE_CONTROL, buf,XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Rate Control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Rate Control Success");
	}

        ret = QicXuGet(XU_AVC_ADVANCE_CONTROL, buf, XU_AVC_ADVANCE_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Rate Control Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Rate Control Success");
	}


	*rate_control_type = buf[2];
        *min_QP_value 	   = buf[3];
        *max_QP_value      = buf[4];  

	return ret;
	
}





/*XU 8 control getting system information*/
int QicGetSysInfo(SysInfo_t *sysinfo)
{

	int ret;
	unsigned char buf[XU_SYSINFO_SIZE_DEF]={0};	
	
	memset(&buf[0],0,sizeof(buf));
	ret=QicXuGet (XU_SYSINFO_CONTROL, &buf[0] , XU_SYSINFO_SIZE_DEF);
	
	if(ret)
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicGetSysInfo Error (%d)%s", errno, strerror(errno));	
	}
	else{
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicGetSysInfo Success");
		
		sysinfo->sHW= buf[0] + (buf[1]<<8);
		sysinfo->sSW= buf[2] + (buf[3]<<8);
		sysinfo->sVID= buf[4] + (buf[5]<<8);
		sysinfo->sPID= buf[6] + (buf[7]<<8);
		sysinfo->sREV= buf[8] + (buf[9]<<8);
		sysinfo->sSVN= buf[10] + (buf[11]<<8);
		sysinfo->cLastErr= buf[12];
	}

	return ret;
}

int QicGetChipVer(QicChipId_t *ver)
{
	int ret;
	unsigned char buf[XU_SYSINFO_SIZE_DEF]={0};	
	memset(&buf[0],0,sizeof(buf));
	ret=QicXuGet (XU_SYSINFO_CONTROL, &buf[0] , XU_SYSINFO_SIZE_DEF);
	
	if(ret)
	{
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicGetChipVer Error (%d)%s", errno, strerror(errno));
		*ver=0;
	}
	else{
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicGetChipVer Success");
		
		*ver= buf[0] + (buf[1]<<8);
	}

	return ret;
}



int QicQueryDevice(void)
{
	int ret;
unsigned int value;
	
  	ret=QicMmioRead (0x02000018, &value);
  	
	return ret;
}


/* ----------- Audio Control API Functions --------------------------------*/
/*******************************************************************
QIC Utility : Set Acoustic Noise Reduction Status
*******************************************************************/
int QicSetANRStatus(unsigned char onoff)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = QIC_XU7_ANR_CONTROL;
	buf[1] = QIC_XU7_ANR_SET_STATUS;
	buf[2] = onoff;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Acoustic Noise Reduction Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Acoustic Noise Reduction Status Success");
	}
	return ret;

}

/*******************************************************************
QIC Utility : Get Acoustic Noise Reduction Status
*******************************************************************/
int QicGetANRStatus(unsigned char *onoff)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = QIC_XU7_ANR_CONTROL;
	buf[1] = QIC_XU7_ANR_GET_STATUS;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Acoustic Noise Reduction Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Acoustic Noise Reduction Status Success");
	}

        ret = QicXuGet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Acoustic Noise Reduction Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Acoustic Noise Reduction Status Success");
	}

        *onoff = buf[2];

	return ret;

}

/*******************************************************************
QIC Utility : Set Equalizer Status
*******************************************************************/
int QicSetEQStatus(unsigned char onoff, unsigned char type)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret;	

	buf[0] = QIC_XU7_EQ_CONTROL;
	buf[1] = QIC_XU7_EQ_SET_STATUS;
	buf[2] = onoff;
        buf[3] = type;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Equalizer Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Equalizer Status Success");
	}
	return ret;

}

/*******************************************************************
QIC Utility : Get Equalizer Status
*******************************************************************/
int QicGetEQStatus(unsigned char *onoff, unsigned char *type)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = QIC_XU7_EQ_CONTROL;
	buf[1] = QIC_XU7_EQ_GET_STATUS;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Equalizer Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Equalizer Status Success");
	}

        ret = QicXuGet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Equalizer Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Equalizer Status Success");
	}

        *onoff = buf[2];
        *type = buf[3];

	return ret;

}

/*******************************************************************
QIC Utility : Set Beam Forming Status
*******************************************************************/
int QicSetBFStatus(unsigned char onoff)
{
	unsigned char buf[XU_MAX_SIZE];
	int ret=0;	

	buf[0] = QIC_XU7_BF_CONTROL;
	buf[1] = QIC_XU7_BF_SET_STATUS;
	buf[2] = onoff;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Beam Forming Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Beam Forming Status Success");
	}
	return ret;

}

/*******************************************************************
QIC Utility : Get Beam Forming Status
*******************************************************************/
int QicGetBFStatus(unsigned char *onoff)
{
	unsigned char buf[XU_MAX_SIZE];	
	int ret;

	buf[0] = QIC_XU7_BF_CONTROL;
	buf[1] = QIC_XU7_BF_GET_STATUS;

        ret = QicXuSet(XU_AUDIO_CONTROL, buf, XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Beam Forming Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set Beam Forming Status Success");
	}

        ret = QicXuGet(XU_AUDIO_CONTROL, buf,XU_AUDIO_SIZE_DEF);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Beam Forming Status Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get Beam Forming Status Success");
	}

        *onoff = buf[2];

	return ret;

}

/* ----------- Get Qic18xx H/W version Function --------------------------------*/

int QicGetHWVersion(QicHwVer_t *version)
{
/* Read QIC MMIO to distinguish QIC1802, QIC1806, QIC1822*/
	unsigned int   value = 0;
    Qic1822HwVer_t qic1822_version;
	int ret;


    if(QicGet1822HWVersion(&qic1822_version)==0&&qic1822_version!=NONE_VERSION)
    {
       LOG_RET_PRINT(debug_xuctrl_str, "QIC HW version is 1822");
		*version= QIC_HWVERSION_QIC1822;
		return 0;
    }

	ret = QicMmioRead (0x06000000, &value);
	 if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get HW Version Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get HW Version Success");
	}

	LOG_RET_PRINT(debug_xuctrl_str, "Get HW Version  value=%x",value );

	if (value == 0x0a020801){
		LOG_RET_PRINT(debug_xuctrl_str, "QIC HW version is 1802");
		*version= QIC_HWVERSION_QIC1802;
	}
	else{
		LOG_RET_PRINT(debug_xuctrl_str, "QIC HW version is 1806");
		*version= QIC_HWVERSION_QIC1806;
	}



	return ret;
}


int QicGet1822HWVersion(Qic1822HwVer_t *version)
{
	/* Read QIC1822 MMIO to distinguish IC chip MP, MPW version*/ 
	int ret;
	unsigned int   value = 0;
	
	ret = QicMmioRead (0x40050000, &value);

	 if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get qic1822 hw Version Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get qic1822 hw Version Success");
	}

	LOG_RET_PRINT(debug_xuctrl_str, "Get HW Version  value=%x",value );

	if (value == 0x22292278){
		LOG_RET_PRINT(debug_xuctrl_str, "1822 MP version");
		*version= QIC1822MP_VERSION;
	}
	else if (value == 0x14321552){
		LOG_RET_PRINT(debug_xuctrl_str, "1822 MPW version");
		*version= QIC1822MPW_VERSION;
	}else{
		LOG_RET_PRINT(debug_xuctrl_str, "Not 1822 version");
		*version= NONE_VERSION;
		}


	return ret;
}

/*change audio ADC mode*/
int QicSetADCMode(ADCMode_t mode)
{
	int ret;
	unsigned int   value = 0;
	
	value=mode;
	ret = QicMmioWrite(0xC0010000, value);

	 if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicSetADCMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicSetADCMode Success");
	}

	

	return ret;
}

/*get current audio ADC mode*/
int QicGetADCMode(ADCMode_t *mode)
{
	int ret;
	unsigned int   value = 0;
	
	ret = QicMmioRead (0xC0020003, &value);

	 if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicGetADCMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicGetADCMode Success");
	}

	LOG_RET_PRINT(debug_xuctrl_str, "QicGetADCMode value=%x",value );

	*mode=value;


	return ret;
}



#if defined(QIC1822)&& defined(QIC_SIMULCAST_API) 

/*1822  EU Codec Controls for UVC1.0 */
int QicEuSetSelectLayer(unsigned short wLayerOrViewID)
{
	int ret;
	unsigned char buf[EU_SELECT_LAYER_SIZE];	

	memset(buf, 0, EU_SELECT_LAYER_SIZE);

	buf[0] =	wLayerOrViewID & 0xFF;
	buf[1] = (wLayerOrViewID>> 8)& 0xFF;

        ret = QicXuSet(EU_SELECT_LAYER_CONTROL, buf, EU_SELECT_LAYER_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSelectLayer Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSelectLayer Success");
	}

	return ret;
}

int QicEuGetSelectLayer(unsigned short *wLayerOrViewID)
{
	int ret=0;
	unsigned char buf[EU_SELECT_LAYER_SIZE];	
	memset(buf, 0, EU_SELECT_LAYER_SIZE);


 	ret = QicXuGet(EU_SELECT_LAYER_CONTROL, buf,EU_SELECT_LAYER_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSelectLayer, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSelectLayer Success");
	}

	*wLayerOrViewID = buf[0] + (buf[1] << 8);


	return ret;
}

int QicEuSetVideoResolution(unsigned short wWidth, unsigned short wHeight)
{
	int ret;
	unsigned char buf[EU_VIDEO_RESOLUTION_SIZE];	

	memset(buf, 0, EU_VIDEO_RESOLUTION_SIZE);

	buf[0] =	wWidth & 0xFF;
	buf[1] = (wWidth >> 8)& 0xFF;
	buf[2] =	wHeight & 0xFF;
	buf[3] = (wHeight  >> 8)& 0xFF;
	
        ret = QicXuSet(EU_VIDEO_RESOLUTION_CONTROL, buf, EU_VIDEO_RESOLUTION_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetVideoResolution Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetVideoResolution Success");
	}

	return ret;
}


int QicEuGetVideoResolution(unsigned short *wWidth, unsigned short *wHeight)
{
	int ret=0;
	unsigned char buf[EU_VIDEO_RESOLUTION_SIZE];	
	memset(buf, 0, EU_VIDEO_RESOLUTION_SIZE);


 	ret = QicXuGet(EU_VIDEO_RESOLUTION_CONTROL, buf,EU_VIDEO_RESOLUTION_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetVideoResolution, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetVideoResolution Success");
	}

	*wWidth = buf[0] + (buf[1] << 8);
	*wHeight = buf[2] +(buf[3] << 8);

	return ret;
}

int QicEuSetProfileAndToolset(unsigned short wProfile, unsigned short wConstrainedToolset, unsigned char bmSettings)
{
	int ret;
	unsigned char buf[EU_PROFILE_TOOLSET_SIZE];	

	memset(buf, 0, EU_PROFILE_TOOLSET_SIZE);

	buf[0] =	wProfile & 0xFF;
	buf[1] = (wProfile >> 8)& 0xFF;
	buf[2] =	wConstrainedToolset & 0xFF;
	buf[3] = (wConstrainedToolset  >> 8)& 0xFF;
	buf[4] =	bmSettings & 0xFF;

	
        ret = QicXuSet(EU_PROFILE_TOOLSET_CONTROL, buf, EU_PROFILE_TOOLSET_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetProfileAndToolset Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetProfileAndToolset Success");
	}

	return ret;
}

int QicEuGetProfileAndToolset(unsigned short *wProfile, unsigned short *wConstrainedToolset, unsigned char *bmSettings)
{
	int ret=0;
	unsigned char buf[EU_PROFILE_TOOLSET_SIZE];	
	memset(buf, 0, EU_PROFILE_TOOLSET_SIZE);


 	ret = QicXuGet(EU_PROFILE_TOOLSET_CONTROL, buf,EU_PROFILE_TOOLSET_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetProfileAndToolset, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetProfileAndToolset Success");
	}

	*wProfile = buf[0] + (buf[1] << 8);
	*wConstrainedToolset = buf[2] +(buf[3] << 8);
	*bmSettings = buf[4];
	
	return ret;
}

int QicEuSetMinimumFrameInterval(unsigned int dwFrameInterval)
{
	int ret;
	unsigned char buf[EU_MIN_FRAME_INTERVAL_SIZE];	

	memset(buf, 0, EU_MIN_FRAME_INTERVAL_SIZE);

	buf[0] =	dwFrameInterval & 0xFF;
	buf[1] = (dwFrameInterval >> 8)& 0xFF;
	buf[2] = (dwFrameInterval >> 16)& 0xFF;
	buf[3] = (dwFrameInterval >> 24)& 0xFF;
	
        ret = QicXuSet(EU_MIN_FRAME_INTERVAL_CONTROL, buf, EU_MIN_FRAME_INTERVAL_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetMinimumFrameInterval Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetMinimumFrameInterval Success");
	}

	return ret;
}

int QicEuGetMinimumFrameInterval(unsigned int *dwFrameInterval)
{
	int ret=0;
	unsigned char buf[EU_MIN_FRAME_INTERVAL_SIZE];	
	memset(buf, 0, EU_MIN_FRAME_INTERVAL_SIZE);


 	ret = QicXuGet(EU_MIN_FRAME_INTERVAL_CONTROL, buf,EU_MIN_FRAME_INTERVAL_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetMinimumFrameInterval, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetMinimumFrameInterval Success");
	}

	*dwFrameInterval = buf[0] + (buf[1] << 8)+(buf[2] << 16)+(buf[3] << 24);
	
	return ret;
}
int QicEuSetSliceMode(unsigned short wSliceMode, unsigned short wSliceConfigSetting)
{
	int ret;
	unsigned char buf[EU_SLICE_MODE_SIZE];	

	memset(buf, 0, EU_SLICE_MODE_SIZE);

	buf[0] =	wSliceMode & 0xFF;
	buf[1] = (wSliceMode >> 8)& 0xFF;
	buf[2] =	wSliceConfigSetting & 0xFF;        //0~3
	buf[3] = (wSliceConfigSetting  >> 8)& 0xFF;
	
        ret = QicXuSet(EU_SLICE_MODE_CONTROL, buf, EU_SLICE_MODE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSliceMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSliceMode Success");
	}

	return ret;
}
int QicEuGetSliceMode(unsigned short *wSliceMode, unsigned short *wSliceConfigSetting)
{
	int ret=0;
	unsigned char buf[EU_SLICE_MODE_SIZE];	
	memset(buf, 0, EU_SLICE_MODE_SIZE);


 	ret = QicXuGet(EU_SLICE_MODE_CONTROL, buf,EU_SLICE_MODE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSliceMode, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSliceMode Success");
	}

	*wSliceMode = buf[0] + (buf[1] << 8);
	*wSliceConfigSetting= buf[2]+(buf[3] << 8);
	
	return ret;
}

int QicEuSetRateControlMode(unsigned char bRateControlMode)
{
	int ret;
	unsigned char buf[EU_RATE_CONTROL_MODE_SIZE];	

	memset(buf, 0, EU_RATE_CONTROL_MODE_SIZE);

	buf[0] =	bRateControlMode & 0xFF;

	
        ret = QicXuSet(EU_RATE_CONTROL_MODE_CONTROL, buf, EU_RATE_CONTROL_MODE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetRateControlMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetRateControlMode Success");
	}

	return ret;
}

int QicEuGetRateControlMode(unsigned char *bRateControlMode)
{
	int ret=0;
	unsigned char buf[EU_RATE_CONTROL_MODE_SIZE];	
	memset(buf, 0, EU_RATE_CONTROL_MODE_SIZE);


 	ret = QicXuGet(EU_RATE_CONTROL_MODE_CONTROL, buf,EU_RATE_CONTROL_MODE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetRateControlMode, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetRateControlMode Success");
	}

	*bRateControlMode = buf[0];
	
	return ret;
}

int QicEuSetAverageBitrateControl(unsigned int dwAverageBitRate)
{
	int ret;
	unsigned char buf[EU_AVERAGE_BITRATE_SIZE];	

	memset(buf, 0, EU_AVERAGE_BITRATE_SIZE);

	buf[0] =	dwAverageBitRate & 0xFF;
	buf[1] = (dwAverageBitRate >> 8)& 0xFF;
	buf[2] = (dwAverageBitRate  >> 16)& 0xFF;
	buf[3] = (dwAverageBitRate  >> 24)& 0xFF;
	
        ret = QicXuSet(EU_AVERAGE_BITRATE_CONTROL, buf, EU_AVERAGE_BITRATE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetAverageBitrateControl Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetAverageBitrateControl Success");
	}

	return ret;
}

int QicEuGetAverageBitrateControl(unsigned int *dwAverageBitRate)
{
	int ret=0;
	unsigned char buf[EU_AVERAGE_BITRATE_SIZE];	
	memset(buf, 0, EU_AVERAGE_BITRATE_SIZE);


 	ret = QicXuGet(EU_AVERAGE_BITRATE_CONTROL, buf,EU_AVERAGE_BITRATE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetAverageBitrateControl, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetAverageBitrateControl Success");
	}

	*dwAverageBitRate = buf[0]+(buf[1] << 8)+(buf[2] << 16)+(buf[3] << 24);
	
	return ret;
}

int QicEuSetCpbSizeControl(unsigned int dwCPBsize)
{
	int ret;
	unsigned char buf[EU_CPB_SIZE_SIZE];	

	memset(buf, 0, EU_CPB_SIZE_SIZE);

	buf[0] =	dwCPBsize & 0xFF;
	buf[1] = (dwCPBsize >> 8)& 0xFF;
	buf[2] = (dwCPBsize  >> 16)& 0xFF;
	buf[3] = (dwCPBsize  >> 24)& 0xFF;
	
        ret = QicXuSet(EU_CPB_SIZE_CONTROL, buf, EU_CPB_SIZE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetCpbSizeControl Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetCpbSizeControl Success");
	}

	return ret;
}
int QicEuGetCpbSizeControl(unsigned int *dwCPBsize)
{
	int ret=0;
	unsigned char buf[EU_CPB_SIZE_SIZE];	
	memset(buf, 0, EU_CPB_SIZE_SIZE);


 	ret = QicXuGet(EU_CPB_SIZE_CONTROL, buf,EU_CPB_SIZE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetCpbSizeControl, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetCpbSizeControl Success");
	}

	*dwCPBsize = buf[0]+(buf[1] << 8)+(buf[2] << 16)+(buf[3] << 24);
	
	return ret;
}
int QicEuSetQuantizationParameter(unsigned short wQpPrime_I, unsigned short wQpPrime_P, unsigned short wQpPrime_B)
{
	int ret;
	unsigned char buf[EU_QUANTIZATION_PARAMS_SIZE];	

	memset(buf, 0, EU_QUANTIZATION_PARAMS_SIZE);

	buf[0] =	wQpPrime_I & 0xFF;
	buf[1] = (wQpPrime_I >> 8)& 0xFF;
	buf[2] = wQpPrime_P& 0xFF;
	buf[3] = (wQpPrime_P  >> 8)& 0xFF;
	buf[4] = wQpPrime_B& 0xFF;
	buf[5] = (wQpPrime_B  >> 8)& 0xFF;
	
        ret = QicXuSet(EU_QUANTIZATION_PARAMS_CONTROL, buf, EU_QUANTIZATION_PARAMS_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetQuantizationParameter Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetQuantizationParameter Success");
	}

	return ret;
}

int QicEuGetQuantizationParameter(unsigned short *wQpPrime_I, unsigned short *wQpPrime_P, unsigned short *wQpPrime_B)
{
	int ret=0;
	unsigned char buf[EU_QUANTIZATION_PARAMS_SIZE];	
	memset(buf, 0, EU_QUANTIZATION_PARAMS_SIZE);


 	ret = QicXuGet(EU_QUANTIZATION_PARAMS_CONTROL, buf,EU_QUANTIZATION_PARAMS_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetQuantizationParameter, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetQuantizationParameter Success");
	}

	*wQpPrime_I = buf[0]+(buf[1] << 8);
	*wQpPrime_P= buf[2]+(buf[3] << 8);
	*wQpPrime_B= buf[4]+(buf[5] << 8);
	
	return ret;
}

int QicEuSetSynchronizationAndLongTermReferenceFrame(unsigned char bSyncFrameType, unsigned short wSyncFrameInterval, unsigned char bGradualDecoderRefresh)
{
	int ret;
	unsigned char buf[EU_SYNC_REF_FRAME_SIZE];	

	memset(buf, 0, EU_SYNC_REF_FRAME_SIZE);

	buf[0] =	bSyncFrameType & 0xFF;
	buf[1] =	wSyncFrameInterval & 0xFF;
	buf[2] = (wSyncFrameInterval >> 8)& 0xFF;
	buf[3] = bGradualDecoderRefresh& 0xFF;

	
        ret = QicXuSet(EU_SYNC_REF_FRAME_CONTROL, buf, EU_SYNC_REF_FRAME_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSynchronizationAndLongTermReferenceFrame Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetSynchronizationAndLongTermReferenceFrame Success");
	}

	return ret;
}

int QicEuGetSynchronizationAndLongTermReferenceFrame(unsigned char *bSyncFrameType, unsigned short *wSyncFrameInterval, unsigned char *bGradualDecoderRefresh)
{
	int ret=0;
	unsigned char buf[EU_SYNC_REF_FRAME_SIZE];	
	memset(buf, 0, EU_SYNC_REF_FRAME_SIZE);


 	ret = QicXuGet(EU_SYNC_REF_FRAME_CONTROL, buf,EU_SYNC_REF_FRAME_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSynchronizationAndLongTermReferenceFrame, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetSynchronizationAndLongTermReferenceFrame Success");
	}

	*bSyncFrameType = buf[0];
	*wSyncFrameInterval= buf[1]+(buf[2] << 8);
	*bGradualDecoderRefresh= buf[3];
	
	return ret;
}
int QicEuSetLevelIdc(unsigned char bLevelIDC)
{
	int ret;
	unsigned char buf[EU_LEVEL_IDC_LIMIT_SIZE];	

	memset(buf, 0, EU_LEVEL_IDC_LIMIT_SIZE);

	buf[0] =	bLevelIDC & 0xFF;
	
        ret = QicXuSet(EU_LEVEL_IDC_LIMIT_CONTROL, buf, EU_LEVEL_IDC_LIMIT_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetLevelIdc Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetLevelIdc Success");
	}

	return ret;
}

int QicEuGetLevelIdc(unsigned char *bLevelIDC)
{
	int ret=0;
	unsigned char buf[EU_LEVEL_IDC_LIMIT_SIZE];	
	memset(buf, 0, EU_LEVEL_IDC_LIMIT_SIZE);


 	ret = QicXuGet(EU_LEVEL_IDC_LIMIT_CONTROL, buf,EU_LEVEL_IDC_LIMIT_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetLevelIdc, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetLevelIdc Success");
	}

	*bLevelIDC = buf[0];

	
	return ret;
}

int QicEuSetQpRange(unsigned char bMinQp, unsigned char bMaxQp)
{
	int ret;
	unsigned char buf[EU_QP_RANGE_SIZE];	

	memset(buf, 0, EU_QP_RANGE_SIZE);

	buf[0] =	bMinQp & 0xFF;
	buf[1] =	bMaxQp & 0xFF;
	
        ret = QicXuSet(EU_QP_RANGE_CONTROL, buf, EU_QP_RANGE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetQpRange Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetQpRange Success");
	}

	return ret;
}

int QicEuGetQpRange(unsigned char *bMinQp, unsigned char *bMaxQp)
{
	int ret=0;
	unsigned char buf[EU_QP_RANGE_SIZE];	
	memset(buf, 0, EU_QP_RANGE_SIZE);


 	ret = QicXuGet(EU_QP_RANGE_CONTROL, buf,EU_QP_RANGE_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetQpRange, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetQpRange Success");
	}

	*bMinQp = buf[0];
	*bMaxQp=buf[1];

	
	return ret;
}

int QicEuSetStartOrStopLayer(unsigned char bUpdate)
{
	int ret;
	unsigned char buf[EU_START_OR_STOP_LAYER_SIZE];	

	memset(buf, 0, EU_START_OR_STOP_LAYER_SIZE);

	buf[0] =	bUpdate & 0xFF;
	
        ret = QicXuSet(EU_START_OR_STOP_LAYER_CONTROL, buf, EU_START_OR_STOP_LAYER_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetStartOrStopLayer Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetStartOrStopLayer Success");
	}

	return ret;
}

int QicEuGetStartOrStopLayer(unsigned char *bUpdate)
{
	int ret=0;
	unsigned char buf[EU_START_OR_STOP_LAYER_SIZE];	
	memset(buf, 0, EU_START_OR_STOP_LAYER_SIZE);


 	ret = QicXuGet(EU_START_OR_STOP_LAYER_CONTROL, buf,EU_START_OR_STOP_LAYER_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetStartOrStopLayer, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetStartOrStopLayer Success");
	}

	*bUpdate = buf[0];

	
	return ret;
}

int QicEuSetErrorResiliency(unsigned short bmErrorResiliencyFeatures)
{
	int ret;
	unsigned char buf[EU_ERROR_RESILIENCY_SIZE];	

	memset(buf, 0, EU_ERROR_RESILIENCY_SIZE);

	buf[0] =	bmErrorResiliencyFeatures & 0xFF;
	buf[1] =	(bmErrorResiliencyFeatures>>8) & 0xFF;
	
        ret = QicXuSet(EU_ERROR_RESILIENCY_CONTROL, buf, EU_ERROR_RESILIENCY_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetErrorResiliency Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuSetErrorResiliency Success");
	}

	return ret;
}

int QicEuGetErrorResiliency(unsigned short *bmErrorResiliencyFeatures)
{
	int ret=0;
	unsigned char buf[EU_ERROR_RESILIENCY_SIZE];	
	memset(buf, 0, EU_ERROR_RESILIENCY_SIZE);


 	ret = QicXuGet(EU_ERROR_RESILIENCY_CONTROL, buf,EU_ERROR_RESILIENCY_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetErrorResiliency, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuGetErrorResiliency Success");
	}

	*bmErrorResiliencyFeatures = buf[0]+ (buf[1]<<8);

	
	return ret;
}

int QicEuExSetSelectCodec(EuExSelectCodec_t bCodec)
{
	int ret;
	unsigned char buf[EU_QIC_SELECT_CODEC_SIZE];	

	memset(buf, 0, EU_QIC_SELECT_CODEC_SIZE);

	buf[0] =	bCodec& 0xFF;
	
        ret = QicXuSet(EU_QIC_SELECT_CODEC_CONTROL, buf, EU_QIC_SELECT_CODEC_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExSetSelectCodec Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExSetSelectCodec Success");
	}

	return ret;
}

int QicEuExGetSelectCodec(EuExSelectCodec_t *bCodec)
{
	int ret=0;
	unsigned char buf[EU_QIC_SELECT_CODEC_SIZE];	
	memset(buf, 0, EU_QIC_SELECT_CODEC_SIZE);


 	ret = QicXuGet(EU_QIC_SELECT_CODEC_CONTROL, buf,EU_QIC_SELECT_CODEC_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExGetSelectCodec, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExGetSelectCodec Success");
	}

	*bCodec = buf[0];

	
	return ret;
}

int QicEuExSetTsvc(unsigned char bTemporalLayer)
{
	int ret;
	unsigned char buf[EU_QIC_TSVC_SIZE];	

	memset(buf, 0, EU_QIC_TSVC_SIZE);

	buf[0] =	bTemporalLayer;
	
        ret = QicXuSet(EU_QIC_TSVC_CONTROL, buf, EU_QIC_TSVC_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExSetTsvc Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExSetTsvc Success");
	}

	return ret;
}

int QicEuExGetTsvc(unsigned char *bTemporalLayer)
{
	int ret=0;
	unsigned char buf[EU_QIC_TSVC_SIZE];	
	memset(buf, 0, EU_QIC_TSVC_SIZE);


 	ret = QicXuGet(EU_QIC_TSVC_CONTROL, buf,EU_QIC_TSVC_SIZE);
        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExGetTsvc, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicEuExGetTsvc Success");
	}

	*bTemporalLayer = buf[0];

	
	return ret;
}



/* qic1832 OSD Control just for encoded stream */
#ifdef QIC_OSD_API
int QicOsdStatusSet (unsigned char osd_status)  /*working for before stream on*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_STATUS_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = osd_status;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdStatusSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdStatusSet Success");
	}

	return ret;
}

int QicOsdStatusGet (unsigned char *osd_status) /*working for before stream on*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_STATUS_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdStatusGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdStatusGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdStatusGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdStatusGet Success");
	}

    *osd_status = buf[2];
	return ret;
}

int QicOsdMiscellSet (OsdMiscell_t osd_miscell) /*working for run time*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_MISC_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = osd_miscell.bg_alpha;
	buf[3] = osd_miscell.fg_alpha;
	buf[4] = osd_miscell.blink_on_frame_count;
	buf[5] = osd_miscell.blink_off_frame_count;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdMiscellSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdMiscellSet Success");
	}

	return ret;
}

int QicOsdMiscellGet (OsdMiscell_t *osd_miscell) /*working for run time*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_MISC_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdMiscellGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdMiscellGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdMiscellGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdMiscellGet Success");
	}

    osd_miscell->bg_alpha = buf[2];
    osd_miscell->fg_alpha = buf[3];
    osd_miscell->blink_on_frame_count = buf[4];
    osd_miscell->blink_off_frame_count = buf[5];
	return ret;
}

int QicOsdColorSet (OsdColor_t osd_color[OSD_COLOR_MAX])  /*working for run time*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_COLOR_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	memcpy(&buf[2], (unsigned char *)osd_color, OSD_COLOR_MAX*sizeof(OsdColor_t));

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdColorSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdColorSet Success");
	}

	return ret;
}

int QicOsdColorGet (OsdColor_t *osd_color) /*working for run time*/
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_COLOR_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdColorGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdColorGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdColorGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdColorGet Success");
	}

    memcpy((unsigned char *)osd_color, &buf[2], 10*sizeof(OsdColor_t));
	return ret;
}

int QicOsdFrameSet (OsdFrame_t osd_frame[OSD_FRAME_MAX])
{
	int ret;
	int i=0;
	int j=0;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_FRAME_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;

	for(i=0;i<OSD_FRAME_MAX;i++){
		
		buf[2+j]=osd_frame[i].frame_y_start;
		buf[3+j]=(osd_frame[i].frame_y_start>>8)&0xFF;
		buf[4+j]=osd_frame[i].frame_height;
		buf[5+j]=(osd_frame[i].frame_height>>8)&0xFF;
		j=j+4;	
		
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdFrameSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdFrameSet Success");
	}

	return ret;
}

int QicOsdFrameGet (OsdFrame_t *osd_frame)
{
	int ret;
	int i=0;
	int j=0;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_FRAME_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdFrameGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdFrameGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdFrameGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdFrameGet Success");
	}

    	for(i=0;i<OSD_FRAME_MAX;i++){
		osd_frame[i].frame_y_start=buf[2+j]+(buf[3+j]<<8);
		osd_frame[i].frame_height=buf[4+j]+(buf[5+j]<<8);
		j=j+4;		
	}

	return ret;
}

int QicOsdLineAttrSet(unsigned char line_id, OsdLineAttr_t line_attr)
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_LINE_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;
	buf[3] = line_attr.enabled;
	buf[4] = line_attr.char_x_repeat;
	buf[5] = line_attr.char_y_repeat;
	buf[6] = line_attr.x_start & 0xFF;
	buf[7] = (line_attr.x_start >> 8)& 0xFF;
	buf[8] = line_attr.y_start & 0xFF;
	buf[9] = (line_attr.y_start  >> 8)& 0xFF;
	buf[10] =line_attr.char_count;
	buf[11] =line_attr.spacing;


    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineAttrSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineAttrSet Success");
	}

	return ret;
}

int QicOsdLineAttrGet(unsigned char line_id, OsdLineAttr_t *line_attr)
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);

	buf[0] = QIC_XU31_OSD_LINE_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;
	buf[2] = line_id;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineAttrGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineAttrGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineAttrGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineAttrGet Success");
	}

	line_attr->enabled=buf[3];
	line_attr->char_x_repeat=buf[4];
	line_attr->char_y_repeat=buf[5];
	line_attr->x_start=buf[6]+(buf[7] << 8);
	line_attr->y_start=buf[8]+(buf[9] << 8);
	line_attr->char_count=buf[10];
	line_attr->spacing=buf[11];

	return ret;
}

int QicOsdLineStringSet(unsigned char line_id, unsigned char start_char_index, unsigned char *str, OsdCharAttr_t char_attr)
{
	int ret, i,j;
	unsigned char buf[XU_OSD_SIZE_DEF];

	/* Set String for Display */
	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_FONT_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;

	j=0;
	for(i=start_char_index;i<OSD_CHAR_MAX;i++){
		if(str[j]!=0){
			buf[i+3] = (str[j]<<1) +1;
			
			j++;
		}else{
			break;
		}
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringSet Success");
	}

    /* Set String Attribute */
    memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;

	j=0;
	for(i=start_char_index;i<OSD_CHAR_MAX;i++){
		if(str[j]!=0){
			buf[i+3] = (char_attr.fg_color<<1) + (char_attr.bg_color_on<<4) +
							(char_attr.color_mode<<5) + (char_attr.updated&0x01);
			j++;
		}else{
			break;
		}
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringSet Success");
	}

	return ret;
}

int QicOsdLineStringGet(unsigned char line_id, unsigned char start_char_index, unsigned char *str, OsdCharAttr_t *char_attr)
{
	int ret, i,j;
	unsigned char buf[XU_OSD_SIZE_DEF];

	/* Get String for Display */
	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_FONT_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;
	buf[2] = line_id;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineStringGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineStringGet Success");
	}

	j=0;
	for(i=start_char_index;i<OSD_CHAR_MAX;i++){
		if((buf[i+3]&0x01)!=0){
			str[j]=buf[i+3]>>1;
			j++;
		}else{
			str[j]=0x00;
			break;
		}
	}

    /* Get String Attribute */
    memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;
	buf[2] = line_id;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineStringGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineStringGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineStringGet Success");
	}

	char_attr->updated     = buf[3+start_char_index]&0x01;
	char_attr->fg_color    = (buf[3+start_char_index]>>1)&0x07;
	char_attr->bg_color_on = (buf[3+start_char_index]>>4)&0x01;
	char_attr->color_mode  = (buf[3+start_char_index]>>5)&0x03;

	return ret;
}

int QicOsdLineArraySet(unsigned char line_id, unsigned char str[OSD_CHAR_MAX], OsdCharAttr_t char_attr[OSD_CHAR_MAX])
{
	int ret, i;
	unsigned char buf[XU_OSD_SIZE_DEF];

	/* Set String for Display */
	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_FONT_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;

	for(i=0;i<OSD_CHAR_MAX;i++){
		buf[i+3] = (str[i]<<1) + char_attr[i].updated;
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArraySet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArraySet Success");
	}

    /* Set String Attribute */
    memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;

	for(i=0;i<OSD_CHAR_MAX;i++){
		buf[i+3] = (char_attr[i].fg_color<<1) + (char_attr[i].bg_color_on<<4) +
							(char_attr[i].color_mode<<5) + (char_attr[i].updated&0x01);
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArraySet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArraySet Success");
	}

	return ret;
}

int QicOsdLineArrayGet(unsigned char line_id, unsigned char *str, OsdCharAttr_t *char_attr)
{
	int ret, i;
	unsigned char buf[XU_OSD_SIZE_DEF];

	/* Get String for Display */
	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_FONT_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;
	buf[2] = line_id;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArrayGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArrayGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineArrayGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineArrayGet Success");
	}

	for(i=0;i<OSD_CHAR_MAX;i++){
		str[i]=buf[i+3]>>1;
	}

    /* Get String Attribute */
    memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_ATTRB_CONTROL;
	buf[1] = QIC_XU31_OSD_GET_SUBCMD;
	buf[2] = line_id;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArrayGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineArrayGet Success");
	}

    ret = QicXuGet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineArrayGet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicOsdLineArrayGet Success");
	}

    for(i=0;i<OSD_CHAR_MAX;i++){
    	char_attr[i].updated     = buf[3+i]&0x01;
    	char_attr[i].fg_color    = (buf[3+i]>>1)&0x07;
    	char_attr[i].bg_color_on = (buf[3+i]>>4)&0x01;
    	char_attr[i].color_mode  = (buf[3+i]>>5)&0x03;
    }

	return ret;
}

int QicOsdLineClear(unsigned char line_id)
{
	int ret, i;
	unsigned char buf[XU_OSD_SIZE_DEF];

	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_CHAR_FONT_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;

	for(i=0;i<OSD_CHAR_MAX;i++){
		buf[i+3] = 0x41; //space "0x20"+updated "1"
	}

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineClear Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdLineClear Success");
	}

    return ret;
}

int QicOsdTimerSet(unsigned char line_id, unsigned char enable, OsdTimer_t timer)
{
	int ret;
	unsigned char buf[XU_OSD_SIZE_DEF];

	/* Set String for Display */
	memset(buf, 0, XU_OSD_SIZE_DEF);
	buf[0] = QIC_XU31_OSD_TIMER_CONTROL;
	buf[1] = QIC_XU31_OSD_SET_SUBCMD;
	buf[2] = line_id;
	buf[3] = enable;
	buf[4] = timer.timer_pos;
	buf[5] = timer.initial_hour;
	buf[6] = timer.initial_min;
    buf[7] = timer.initial_sec;

    ret = QicXuSet(XU_OSD_CONTROL, buf, XU_OSD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdTimerSet Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicOsdTimerSet Success");
	}

 

	return ret;
}



#endif


#ifdef QIC_MD_API

int QicMDGetVersion(int *major_version, int *minor_version)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);



	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_VERSION_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;

    ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetVersion Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetVersion Success");
	}

    ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetVersion Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetVersion Success");
	}


	*major_version = buf[4];
	*minor_version = buf[5];

	return ret;
}

int QicMDSetEnable(unsigned char enable)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Command part
	buf[0] = QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] =  QIC_XU19_MD_ENABLE_CONTROL;
	buf[3] = QIC_XU19_MD_SET_SUBCMD;
	// Data part
	buf[4] = enable;

	 ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetEnable Success");
	}

	return ret;
	
}


int QicMDGetEnable(unsigned char *enable)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] =  QIC_XU19_MD_ENABLE_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;

    ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetEnable Success");
	}

    ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetEnable Success");
	}


	*enable = buf[4];

	printf("MD enable\%d\n",buf[4]);

	return ret;

}


int QicMDSetConfiguration(md_config_t* config)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];
	int i;
	memset(buf, 0, XU_MD_SIZE_DEF);

	// Set data
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_CONFIGURE_CONTROL;
	buf[3] = QIC_XU19_MD_SET_SUBCMD;
	// Data part
	buf[4] = config->number_of_regions;
	for( i=0; i<3; i++){
		unsigned int chuckSize = 8;
		buf[5 + chuckSize*i] = config->region_of_interests[i].id;
		buf[6 + chuckSize*i] = config->region_of_interests[i].sensitivity;
		buf[7 + chuckSize*i] = config->region_of_interests[i].history;
		buf[8 + chuckSize*i] = config->region_of_interests[i].min_object_size;
		buf[9 + chuckSize*i] = config->region_of_interests[i].region_of_intrest.x;
		buf[10 + chuckSize*i] = config->region_of_interests[i].region_of_intrest.y;
		buf[11 + chuckSize*i] = config->region_of_interests[i].region_of_intrest.width;
		buf[12 + chuckSize*i] = config->region_of_interests[i].region_of_intrest.height ;
	}

	ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetConfiguration Success");
	}

	return ret;
	
}


int QicMDGetConfiguration(md_config_t* config)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];
	int i;
	memset(buf, 0, XU_MD_SIZE_DEF);

/*
#define QIC_XU19_MD_MODULE			0

#define QIC_XU19_MD_VERSION_CONTROL		0
#define QIC_XU19_MD_ENABLE_CONTROL           	1
#define QIC_XU19_MD_CONFIGURE_CONTROL		2
#define QIC_XU19_MD_STATUS_CONTROL		3
#define QIC_XU19_MD_INTERRUPT_CONTROL		4
*/

	// Set data
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_CONFIGURE_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;
	// Data part
	// None

	   ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetConfiguration Success");
	}

    ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetConfiguration Success");
	}

	// Get data buffer
	// Data part
	config->number_of_regions = buf[4];
	for( i=0; i<3; i++){
		unsigned char chuckSize = 8;
		config->region_of_interests[i].id = buf[5 + chuckSize*i];
		config->region_of_interests[i].sensitivity = buf[6 + chuckSize*i];
		config->region_of_interests[i].history = buf[7 + chuckSize*i];
		config->region_of_interests[i].min_object_size = buf[8 + chuckSize*i];
		config->region_of_interests[i].region_of_intrest.x = buf[9 + chuckSize*i];
		config->region_of_interests[i].region_of_intrest.y = buf[10 + chuckSize*i];
		config->region_of_interests[i].region_of_intrest.width = buf[11 + chuckSize*i];
		config->region_of_interests[i].region_of_intrest.height = buf[12 + chuckSize*i];
	}

	return ret;
}


int QicMDGetStatus(md_status_t* status)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Set data buffer
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_STATUS_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;
	// Data part
	// None

	ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDGetStatus Success");
	}

    ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Success");
	}

	// Get data buffer
	// Data part
	/************************************************************************************************
	Offset	Field				Size	Value	Description
		4	dwTimestamp			4		Number	The timestamp of the motion detection result in milliseconds.
		8	bNumberOfObjects	1		Number	The number of moving object windows.
		9	bLeft0				1		Number	Left position of the moving object in number of macro blocks.
		10	bTop0				1		Number	Top position of the moving object in number of macro blocks.
		11	bWidth0				1		Number	Width of the moving object in number of macro blocks.
		12	bHeight0			1		Number	Height of the moving object in number of macro blocks.
		13	bLeft1				1		Number	Left position of the moving object in number of macro blocks.
		14	bTop1				1		Number	Top position of the moving object in number of macro blocks.
		15	bWidth1				1		Number	Width of the moving object in number of macro blocks.
		16	bHeight1			1		Number	Height of the moving object in number of macro blocks.
		17	bLeft2				1		Number	Left position of the moving object in number of macro blocks.
		18	bTop2				1		Number	Top position of the moving object in number of macro blocks.
		19	bWidth2				1		Number	Width of the moving object in number of macro blocks.
		20	bHeight2			1		Number	Height of the moving object in number of macro blocks.

	************************************************************************************************/
#define DATA_FROM_OFFSET(type,offset,pdata) (*((type*)(((unsigned char*)pdata)+offset)))
	status->timestamp =			DATA_FROM_OFFSET(unsigned int, 4, buf);
	status->number_of_moving_objects =	DATA_FROM_OFFSET(unsigned char,  8, buf);

	// macro-blocks x block-size(16).
	status->moving_objects[0].x = DATA_FROM_OFFSET(unsigned char,  9, buf); 
	status->moving_objects[0].y = DATA_FROM_OFFSET(unsigned char, 10, buf); 
	status->moving_objects[0].width = DATA_FROM_OFFSET(unsigned char, 11, buf); 
	status->moving_objects[0].height = DATA_FROM_OFFSET(unsigned char, 12, buf);

	status->moving_objects[1].x = DATA_FROM_OFFSET(unsigned char,  13, buf); 
	status->moving_objects[1].y = DATA_FROM_OFFSET(unsigned char, 14, buf); 
	status->moving_objects[1].width = DATA_FROM_OFFSET(unsigned char, 15, buf); 
	status->moving_objects[1].height = DATA_FROM_OFFSET(unsigned char, 16, buf);

	status->moving_objects[2].x = DATA_FROM_OFFSET(unsigned char,  17, buf); 
	status->moving_objects[2].y = DATA_FROM_OFFSET(unsigned char, 18, buf); 
	status->moving_objects[2].width = DATA_FROM_OFFSET(unsigned char, 19, buf); 
	status->moving_objects[2].height = DATA_FROM_OFFSET(unsigned char, 20, buf);
#undef DATA_FROM_OFFSET

	return ret;
}


int QicMDSetInterruptMode(unsigned char mode)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Set data buffer
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_INTERRUPT_CONTROL;
	buf[3] = QIC_XU19_MD_SET_SUBCMD;
	// Data part
	buf[4] = mode;

	ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetInterruptMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetInterruptMode Success");
	}

	return ret;
}


int QicMDGetInterruptMode(unsigned char* mode)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Set data buffer
	// Command part
	buf[0] =  QIC_XU19_MD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_MD_INTERRUPT_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;

	ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetInterruptMode Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicMDSetInterruptMode Success");
	}

	  ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Success");
	}

	*mode = buf[4];

	return ret;
}

int QicMDWaitMotion(md_status_t* status)
{
	//TODO
	return 0;
}



int QicSetPeakBitrate(unsigned int dwPeakBirtate)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Command part
	buf[0] = QIC_XU19_MD_MODULE_1 & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE_1 >> 8) & 0xFF;
	buf[2] =  QIC_XU19_MD_PEAK_BIREATE_CONTROL;
	buf[3] = QIC_XU19_MD_SET_SUBCMD;
	// Data part
	buf[4] = dwPeakBirtate & 0xff;
	buf[5] = (dwPeakBirtate >> 8) & 0xff;
	buf[6] = (dwPeakBirtate >> 16) & 0xff;
	buf[7] = (dwPeakBirtate >> 24) & 0xff;


	 ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicMDSetPeakBitrate Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicMDSetPeakBitrate Success");
	}

	return ret;
	
}



int QicGetPeakBitrate(unsigned int *dwPeakBirtate)
{
	int ret;
	unsigned char buf[XU_MD_SIZE_DEF];

	memset(buf, 0, XU_MD_SIZE_DEF);

	// Command part
	buf[0] = QIC_XU19_MD_MODULE_1 & 0xFF;
	buf[1] = (QIC_XU19_MD_MODULE_1 >> 8) & 0xFF;
	buf[2] =  QIC_XU19_MD_PEAK_BIREATE_CONTROL;
	buf[3] = QIC_XU19_MD_GET_SUBCMD;


	 ret = QicXuSet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);

        if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "QicMDGetPeakBitrate, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "QicMDGetPeakBitrate Success");
	}
	
		  ret = QicXuGet(XU_MD_CONTROL, buf, XU_MD_SIZE_DEF);
       if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicMDGetStatus Success");
	}


	*dwPeakBirtate = buf[4]+(buf[5] << 8)+(buf[6] << 16)+(buf[7] << 24);
	
	return ret;


	return ret;
	
}



int QicTDGetVersion(int *major_version, int *minor_version)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];

	memset(buf, 0, XU_TD_SIZE_DEF);



	buf[0] =  QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_TD_VERSION_CONTROL;
	buf[3] = QIC_XU19_TD_GET_SUBCMD;

    ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetVersion Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetVersion Success");
	}

    ret = QicXuGet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetVersion Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetVersion Success");
	}


	*major_version = buf[4];
	*minor_version = buf[5];

	return ret;
}

int QicTDSetEnable(unsigned char enable)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];

	memset(buf, 0, XU_TD_SIZE_DEF);

	// Command part
	buf[0] = QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] =  QIC_XU19_TD_ENABLE_CONTROL;
	buf[3] = QIC_XU19_TD_SET_SUBCMD;
	// Data part
	buf[4] = enable;

	printf("set TD enable=%d\n",enable);
	 ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDSetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDSetEnable Success");
	}

	return ret;
	
}


int QicTDGetEnable(unsigned char *enable)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];

	memset(buf, 0, XU_TD_SIZE_DEF);

	
	// Command part
	buf[0] =  QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] =  QIC_XU19_TD_ENABLE_CONTROL;
	buf[3] = QIC_XU19_TD_GET_SUBCMD;

    ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetEnable Success");
	}

    ret = QicXuGet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetEnable Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetEnable Success");
	}


	*enable = buf[4];

	printf("TD enable:%d\n",buf[4]);

	return ret;

}


int QicTDSetConfiguration(td_config_t* config)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];
	int i;
	memset(buf, 0, XU_TD_SIZE_DEF);

	// Set data
	// Command part
	buf[0] =  QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_TD_CONFIGURE_CONTROL;
	buf[3] = QIC_XU19_TD_SET_SUBCMD;
	// Data part
	buf[4] = config->long_term_time;
	buf[5] = config->short_term_time;
	buf[6] = config->sensitivity;

	ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDSetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDSetConfiguration Success");
	}

	return ret;
	
}


int QicTDGetConfiguration(td_config_t* config)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];
	int i;
	memset(buf, 0, XU_TD_SIZE_DEF);

	// Set data
	// Command part
	buf[0] =  QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_TD_CONFIGURE_CONTROL;
	buf[3] = QIC_XU19_TD_GET_SUBCMD;
	// Data part
	// None

	   ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetConfiguration Success");
	}

    ret = QicXuGet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetConfiguration Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetConfiguration Success");
	}

   	config->long_term_time = buf[4];
	config->short_term_time = buf[5];
	config->sensitivity = buf[6];

	return ret;
}


int QicTDGetStatus(int* status)
{
	int ret;
	unsigned char buf[XU_TD_SIZE_DEF];

	memset(buf, 0, XU_TD_SIZE_DEF);

	// Set data buffer
	// Command part
	buf[0] = QIC_XU19_TD_MODULE & 0xFF;
	buf[1] = (QIC_XU19_TD_MODULE >> 8) & 0xFF;
	buf[2] = QIC_XU19_TD_STATUS_CONTROL;
	buf[3] = QIC_XU19_TD_GET_SUBCMD;
	// Data part
	// None

	ret = QicXuSet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Set QicTDGetStatus Success");
	}

    ret = QicXuGet(XU_TD_CONTROL, buf, XU_TD_SIZE_DEF);
    if(ret) {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetStatus Error, (%d)%s", errno, strerror(errno));
	} else {
		LOG_RET_PRINT(debug_xuctrl_str, "Get QicTDGetStatus Success");
	}
	
	*status = buf[4];  //return type of detected tempering 

	return ret;
}


#endif


#endif






