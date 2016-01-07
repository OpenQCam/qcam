/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#ifndef _QIC_XUCTRL_H_
#define _QIC_XUCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>
#include "qic_cfg.h"
#include <linux/videodev2.h>

  // jennifer defined start
#define QIC_FIRMWARE_ADDRESS  0x240
#define QIC_FIRMWARE_DATA_SIZE  0x18
#define QIC_REVISION_CHARACTER_POSITION 16
#define QIC_REVISION_NUMBER_POSITION 19

#if (defined(__CL1830__))
#define V4L2_CID_PAN_ABSOLUTE     (V4L2_CID_CAMERA_CLASS_BASE+8)
#define V4L2_CID_TILT_ABSOLUTE      (V4L2_CID_CAMERA_CLASS_BASE+9)
#endif

  // jennifer defined end

/*Qic1802 Flash define*/
/* Flash Parameter */
#define FLASH_MAX_DATA_LEN    26

#define FLASH_PAGE_SIZE     0x100
#define FLASH_SECTOR_SIZE   0x1000
#define FLASH_MAX_SIZE      0x20000

#define FLASH_USB_ADDR      0x00000
#define FLASH_USB_MAX_SIZE    0x10000
#define FLASH_ISP_ADDR      0x10000
#define FLASH_ISP_MAX_SIZE    0x07000
#define FLASH_PARAM_ADDR    0x17000
#define FLASH_PARAM_MAX_SIZE  0x01000
#define FLASH_VERIFY_MAX_SIZE FLASH_USB_MAX_SIZE

/*Qic1816 Flash define*/
#define QIC1816_FLASH_MAX_SIZE            0x20000
#define QIC1816_FLASH_USB_ADDR            0x00000
#define QIC1816_FLASH_USB_MAX_SIZE          0x14000
#define QIC1816_FLASH_PARAM_ADDR          0x14000
#define QIC1816_FLASH_PARAM_MAX_SIZE        0x01000
#define QIC1816_FLASH_AUDIO_PARAM_ADDR        0x3D000
#define QIC1816_FLASH_AUDIO_PARAM_MAX_SIZE      0x01000
#define QIC1816_FLASH_VERIFY_MAX_SIZE QIC1816_FLASH_USB_MAX_SIZE


#ifdef QIC_SUPPORT_2ND_BL
/*Qic1822 2nd Boot Rom Flash define*/
#define QIC1822_FLASH_2ndBL_ADDR          0x00000
#define QIC1822_FLASH_2ndBL_MAX_SIZE            0x02000

/*Qic1822 Flash define*/
#define QIC1822_FLASH_MAX_SIZE            0x40000
#define QIC1822_FLASH_USB_ADDR              QIC1822_FLASH_2ndBL_MAX_SIZE
#define QIC1822_FLASH_USB_MAX_SIZE              0x36000
#define QIC1822_FLASH_PARAM_ADDR          0x3A000
#define QIC1822_FLASH_PARAM_MAX_SIZE        0x01000
#define QIC1822_FLASH_AUDIO_PARAM_ADDR        0x3D000
#define QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE      0x01000
#define QIC1822_FLASH_OSD_FONT_ADDR         0x38000
#define QIC1822_FLASH_OSD_FONT_MAX_SIZE       0x02000
#define QIC1822_FLASH_SERIAL_NUMBER_ADDR      0x3E000
#define QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE    0x50
#define QIC1822_FLASH_VERIFY_MAX_SIZE           QIC1822_FLASH_MAX_SIZE

#else
/*Qic1822 Flash define*/
#define QIC1822_FLASH_MAX_SIZE            0x40000
#define QIC1822_FLASH_USB_ADDR            0x00000
#define QIC1822_FLASH_USB_MAX_SIZE          0x38000
#define QIC1822_FLASH_PARAM_ADDR          0x3A000
#define QIC1822_FLASH_PARAM_MAX_SIZE        0x01000
#define QIC1822_FLASH_AUDIO_PARAM_ADDR        0x3D000
#define QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE      0x01000
#define QIC1822_FLASH_OSD_FONT_ADDR         0x38000
#define QIC1822_FLASH_OSD_FONT_MAX_SIZE       0x02000
#define QIC1822_FLASH_SERIAL_NUMBER_MAX_SIZE    0x50
#define QIC1822_FLASH_VERIFY_MAX_SIZE       QIC1816_FLASH_USB_MAX_SIZE


#endif

#ifdef QIC_SUPPORT_2ND_BL
#define QIC_FLASH_PID_VID_ADDR    0x2240
#define QIC_FLASH_SVN_ADDR      0x222C
#else
#define QIC_FLASH_PID_VID_ADDR      0x240
#define QIC_FLASH_SVN_ADDR      0x22C

#endif


/*Video/Audio streaming MMIO address*/
#define HSA_ADDRESS 0x40040030    //encoding stream
#define VSA_ADDRESS 0x40050030    // preview stream
#define ASA_ADDRESS 0x40060030    //aduio stream

#define QIC_IMG_HEADER_SIZE 2


/*QIC devices description*/
#define SF_ADDR_DEVICE_DESC   0x04020
#define SF_ADDR_STRING_DESC   0x02020

#define ENCODER_CAPS_MAXSIZE     255

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif

typedef enum {
  ENCODER_PROFILE_BASELINE  = 66,
  ENCODER_PROFILE_MAIN    = 77,
  ENCODER_PROFILE_EXTENDED  = 88,
  ENCODER_PROFILE_HIGH    = 100,
  ENCODER_PROFILE_HIGH_10   = 110,
  ENCODER_PROFILE_HIGH_422  = 122,
  ENCODER_PROFILE_HIGH_444  = 144,
  ENCODER_PROFILE_CAVLC_444 = 44
} EncoderProfile_t;

typedef enum {
  ENCODER_LEVEL_1       = 10,
  ENCODER_LEVEL_1B      = 19,
  ENCODER_LEVEL_1_1     = 11,
  ENCODER_LEVEL_1_2     = 12,
  ENCODER_LEVEL_1_3     = 13,
  ENCODER_LEVEL_2       = 20,
  ENCODER_LEVEL_2_1     = 21,
  ENCODER_LEVEL_2_2     = 22,
  ENCODER_LEVEL_3       = 30,
  ENCODER_LEVEL_3_1     = 31,
  ENCODER_LEVEL_3_2     = 32,
  ENCODER_LEVEL_4       = 40,
  ENCODER_LEVEL_4_1     = 41,
  ENCODER_LEVEL_4_2     = 42,
  ENCODER_LEVEL_5       = 50,
  ENCODER_LEVEL_5_1     = 51
} EncoderLevel_t;


typedef struct {
  unsigned int  uiBitrate;
  unsigned short  usWidth;
  unsigned short  usHeight;
  unsigned char ucFramerate;
  unsigned char ucSlicesize;
} EncoderParams_t;

typedef struct {
  unsigned int      uiTimeWindow; // Not implement
  unsigned char     ucOverShoot;  // Not implement
  unsigned char     ucMinQP;
  unsigned char     ucMaxQP;
} EncoderQuality_t;

typedef struct {
  unsigned short      usWidth;
  unsigned short      usHeight;
  unsigned int      uiMinBitrate;
  unsigned int      uiMaxBitrate;
  unsigned char     ucMinFramerate;
  unsigned char     ucMaxFramerate;
} EncoderCapability_t;

typedef struct {
  char      szVID[5];
  char      szPID[5];
  char      szREV[5];
} FirmwareVersion_t;

typedef struct {
  unsigned short      sVID;
  unsigned short      sPID;
  unsigned short      sREV;
  unsigned short      sSVN;
  unsigned short      sHW;
  unsigned short      sSW;
  unsigned char     cLastErr;
} SysInfo_t;

typedef struct  {
  unsigned int x;
 unsigned int y;
 unsigned int Wx;
  unsigned int Wy;
} CROPRECT;

typedef struct  {
  unsigned int x;
 unsigned int y;
 unsigned int Wx;
  unsigned int Wy;
} TRIMRECT;

/*Advanced Motor Position */
typedef struct  {
  signed short pan;  /* pan range */
  signed short tilt;  /*tilt range  */
} MotorPosition_t;

/*Advanced Motor Control Mode*/
typedef enum {
  ADV_MOTOR_STOP          = 0x00,   /*Stop for signel and Navigation mode*/
  ADV_MOTOR_SINGLE_SET        = 0x01,   /*Set position and then start Single mode*/
  ADV_MOTOR_SINGLE_GET        = 0x02,   /*Get position for Single mode*/
  ADV_MOTOR_NAVIGATE_POSITION_SET     = 0x03,   /*Set position array of Navigation mode*/
  ADV_MOTOR_NAVIGATE_SET        = 0x04,   /*Set position array of Navigation mode and then start Navigation mode*/
  ADV_MOTOR_RESET         = 0x05,   /*Reset all of positions to defalut for Navigation and Single mode*/
  ADV_MOTOR_SPEED         = 0x06,   /*Set motor speed */
} AdvMotorCtlMode_e;

/*Advanced Motor Control Position setting for Navigation*/
typedef struct  {
  unsigned char TotalPositionNumbers;  /*total number of input positions range for once UVC I/O ctl 1~5*/
 unsigned char  StartElementIndex;      /*Start index number of position arry range 1~20*/
 MotorPosition_t PosArray[5];   /* position array*/
} AdvMotorCtl_t;

/*IR ALS Mode*/
typedef enum {
  ALS_AUTO_IR_OFF         = 0x00,   /*ALS auto mode off, IR/ICR disable mode*/
  ALS_AUTO_OFF_IR_ON        = 0x01,   /*ALS auto mode off, IR/ICR enable mode*/
  ALS_AUTO_ON         = 0x02   /*ALS auto mode on mode*/
} IRALSCtlMode_e;


#ifdef QIC_OSD_API

#define OSD_COLOR_MAX 9   //10
#define OSD_FRAME_MAX 7   //8
#define OSD_CHAR_MAX 28   //32

/*OSD Status*/
typedef enum {
  OSD_DISABLE = 0x00,
  OSD_ENABLE  = 0x01
} OsdStatus_e;

/*OSD Target line id*/
typedef enum {
  LINE0   = 0x00,
  LINE1   = 0x01,
  LINE2   = 0x02,
  LINE3   = 0x03,
  LINE4   = 0x04,
  LINE5   = 0x05,
  LINE6   = 0x06,
  LINE7   = 0x07
} OsdLineId_e;

/*FG/BG Color mode:0: Normal 1: BG/image half 2: BG/FG reverse 3: Blink */
typedef enum {
  NORMAL    = 0,
  HALF    = 1,
  REVERSE   = 2,
  BLINK   = 3
} OsdColorMode_e;

typedef enum {
  GRAY=0x00,
  RED=0x01,
  GREEN=0x02,
  BLUE=0x03,
  PINK=0x04,
  PURPLE=0x05,
  SAPPHIRE=0x06,
  ORANGE=0x07,
} OsdColor_e;

/*Miscellaneous setting refer to -  OSD Miscellaneous Control:*/
typedef struct {
  unsigned char bg_alpha;         /*Background alpha, 0~15*/
  unsigned char fg_alpha;         /*Foreground alpha, 0~31*/
  unsigned char blink_on_frame_count;    /*Number of blink on frame, 0~255*/
  unsigned char blink_off_frame_count;   /*Number of blink off frame, 0~255*/
} OsdMiscell_t;

/*color setting refer to -  OSD Color Control:*/
typedef struct {
  unsigned char color_y;          /*YUV setting of foreground color*/
  unsigned char color_u;
  unsigned char color_v;
} OsdColor_t;

/*frame setting refer to -  OSD Frame Control:*/
typedef struct {
   unsigned short frame_y_start;      /*The y start address of the frame*/
   unsigned short frame_height;             /*The height of the frame*/
}OsdFrame_t;

/*line Attribute setting refer to - OSD Line Attribute Control:*/
typedef struct {
  unsigned char enabled;          /*Enable/Disable this line*/
  unsigned char char_x_repeat;      /*Resize the width of the chars in this line 0~7: 1x~8x*/
  unsigned char char_y_repeat;      /*Resize the height of the chars in this line 0~7: 1x~8x*/
  unsigned short x_start;         /*The x of (x, y), which is the top-left corner of this line*/
  unsigned short y_start;         /*The y of (x, y), which is the top-left corner of this line*/
  unsigned char char_count;       /*Number of char displayed of this line: 1~28: 1~28 char */
  unsigned char spacing;                  /*The spacing between each char of this line:0~255: 0~255 pixel*/
} OsdLineAttr_t;

/*char Attribute setting refer to - OSD Char Attribute Control: */
typedef struct {
  unsigned char updated;          /*Flag to update this char or not: 0: Do not update this char,1: Update this char*/
   unsigned char fg_color;          /*Foreground color:0~7: FG color#0~color#7 */
   unsigned char bg_color_on;       /*Background color on/off:0: BG color off1: BG color on */
   unsigned char color_mode;        /*FG/BG Color mode:0: Normal 1: BG/image half 2: BG/FG reverse 3: Blink */
} OsdCharAttr_t;

/*char Attribute setting refer to - OSD Timer Control: */
typedef struct {
  unsigned char timer_pos;        /*Assign the location of timer 7~28*/
  unsigned char initial_hour;       /*The initial bour when timer starts, 0~23*/
   unsigned char initial_min;       /*The initial minute when timer starts, 0~59*/
   unsigned char initial_sec;       /*The initial second when timer starts, 0~59*/
} OsdTimer_t;


#endif


#ifdef QIC_MD_API

typedef struct {
  unsigned char x; // in number of macro blocks.
  unsigned char y; // in number of macro blocks.
  unsigned char width; // in number of macro blocks.
  unsigned char height; // in number of macro blocks.
} rect_t;

typedef struct {
  unsigned char id; // it is an unique ID value to identify the configure/window
  unsigned char sensitivity; //0: lowest sensitive, 100: highest sensitive
// history
// in number of frames; how long an object needs to be in a region
// before it is considered to be non-moving.
  unsigned char history;
// min object size
// in number of pixels; it is the minimum area of the object.
  unsigned long min_object_size;
  rect_t region_of_intrest;
} md_region_of_interest_t;

typedef struct {
unsigned char number_of_regions;
md_region_of_interest_t region_of_interests[3];
} md_config_t;

typedef struct {
  unsigned long timestamp; // when is it detected
  unsigned char number_of_moving_objects;
  rect_t moving_objects[3];
} md_status_t;

/*MD Start/Stop*/
typedef enum {
  MD_DISABLE  = 0x00,
  MD_ENABLE = 0x01
} MdStart_e;

/*MD interrupt mode*/
typedef enum {
  NONE        = 0x00,  //host polls status by XU, so far QIC XU only support NONE mode,
  EXTERNAL_TRIGGER  = 0x01,  // external trigger mode
  AUTO_UPDATE     = 0x02   // auto update MD status on usb interrupt control
} MdInterrupt_e;




typedef struct{
  unsigned char long_term_time;  //
  unsigned char short_term_time; //
  unsigned char   sensitivity;     //range:0~100
}td_config_t;


#endif


typedef enum {
  MODE_FLASH_PROGRAM_PP = 0x00,
  MODE_FLASH_PROGAME_AAI  = 0x01
} FlashProgramMode_t;

typedef enum {
  ENCRYPT_KEY_MODE_128  = 0x00,
  ENCRYPT_KEY_MODE_192  = 0x01,
  ENCRYPT_KEY_MODE_256  = 0x02
} EncyptKeyMode_t;

typedef enum {
  CALIBRATE_NORMAL    = 0x00,
  CALIBRATE_AE_DISABLE  = 0x01,
  CALIBRATE_AWB_DISABLE = 0x02,
  CALIBRATE_AEAWB_DISABLE = 0x03
} CalibratMode_t;

typedef enum {
  EFFECT_NONE       = 0x00,
  EFFECT_BLACK_WHITE    = 0x01,
  EFFECT_FALSE_COLOR    = 0x02,
  EFFECT_NEGATIVE     = 0x03,
  EFFECT_POSTERIZATION  = 0x04,
  EFFECT_SEPIA      = 0x05,
  EFFECT_SOLARIZATION1  = 0x06,
  EFFECT_SOLARIZATION2  = 0x07,
  EFFECT_EMBOSS1      = 0x08,
  EFFECT_EMBOSS2      = 0x09,
  EFFECT_SKETCH     = 0x0A
} SpecialEffect_t;

typedef enum {
  FORMAT_STREAM_H264_RAW    = 0x00,
  FORMAT_STREAM_H264_CONTAINER  = 0x01,
  FORMAT_STREAM_H264_RAW_1  = 0x02,
  FORMAT_STREAM_H264_RAW_2  = 0x04,
  FORMAT_STREAM_H264_RAW_3  = 0x06
} EncoderStreamFormat_t;


typedef enum {
  H264_CAVLC = 0,
  H264_CABAC= 1,
} EncoderEntropy_t;

typedef enum {
  UNKNOWN=0,
  QIC1802A = 0x1802,
  QIC1806A= 0x1806,
  QIC1816A= 0x1816,
  QIC1817EVB= 0x1817,
  QIC1822A= 0x1822,
} QicChipId_t;


/*QIC IC chip model*/
typedef enum {
  QIC_HWVERSION_QIC1802 = 0,
  QIC_HWVERSION_QIC1806= 1,
  QIC_HWVERSION_QIC1822 =2
} QicHwVer_t;


/*QIC1822 IC chip version*/
typedef enum {
  QIC1822MP_VERSION=0,
  QIC1822MPW_VERSION=1,
  NONE_VERSION=2
}Qic1822HwVer_t;




typedef enum {
  FORMAT_STREAM_H264_NORMAL     = 0x00,
  FORMAT_STREAM_H264_ADD_PADDING        = 0x01
} EncoderStreamSizeFormat_t;


/*AVC Rate Control*/
typedef enum {
       VBR_MODE     =0, /*VBR (MB Based) quanta range(17~40) */ /*avc spec define range (0~51)*/
       CBR_MODE     =1, /*CBR (Reserved) not implement yet*/
       FIXED_QP_MODE  =2, /*Fixed QP range(17~40)*/
       GVBR_MODE    =3  /*Global VBR (Reserved for simulcast) not implement yet*/
}EncoderRateControlMode_t;


typedef enum {
  ADC_SNN4_MODE = 1,
  ADC_SNW4_MODE = 2,
  ADC_SNN2_MODE = 3,
  ADC_SNW2_MODE = 4,
  ADC_SNNN_MODE = 5,
  ADC_MP16_MODE = 6,
  ADC_MI2S_MODE = 7
}ADCMode_t;

#define QIC_XU1_ENCODER_BITRATE     0x01
#define QIC_XU1_ENCODER_RESOLUTION    0x02
#define QIC_XU1_ENCODER_FRAMERATE   0x04
#define QIC_XU1_ENCODER_SLICESIZE   0x08
#define QIC_XU1_ENCODER_CONFIG_ALL    (QIC_XU1_ENCODER_BITRATE | QIC_XU1_ENCODER_FRAMERATE | \
            QIC_XU1_ENCODER_RESOLUTION | QIC_XU1_ENCODER_SLICESIZE)

#define QIC_XU1_QUALITY_TIMEWINDOW    0x01
#define QIC_XU1_QUALITY_OVERSHOOT   0x02
#define QIC_XU1_QUALITY_MINQP     0x04
#define QIC_XU1_QUALITY_MAXQP     0x08
#define QIC_XU1_QUALITY_ALL     (QIC_XU1_QUALITY_TIMEWINDOW | QIC_XU1_QUALITY_OVERSHOOT | \
            QIC_XU1_QUALITY_MINQP | QIC_XU1_QUALITY_MAXQP)



/*****************************QIC1822 EU Controls for UVC1.0**************************************/

typedef enum {
  SIMULCAST_STREAM0                       = 0,
  SIMULCAST_STREAM1         = 0x400,
  SIMULCAST_STREAM2       = 0x800,
  SIMULCAST_STREAM3                   = 0xC00,
  SIMULCAST_STREAMALL             = 0x1C00,
  SIMULCAST_STREAM0_LAYER0                    = 0,
  SIMULCAST_STREAM0_LAYER1      = 0x80,
  SIMULCAST_STREAM0_LAYER2      = 0x100,
  SIMULCAST_STREAM0_LAYER3      = 0x180,
  SIMULCAST_STREAM0_LAYER_ALL           = 0x380,
  SIMULCAST_STREAM1_LAYER0      = 0x400,
  SIMULCAST_STREAM1_LAYER1      = 0x480,
  SIMULCAST_STREAM1_LAYER2      = 0x500,
  SIMULCAST_STREAM1_LAYER3      = 0x580,
  SIMULCAST_STREAM1_LAYER_ALL           = 0x780,
  SIMULCAST_STREAM2_LAYER0                      = 0x800,
  SIMULCAST_STREAM2_LAYER1      = 0x880,
  SIMULCAST_STREAM2_LAYER2      = 0x900,
  SIMULCAST_STREAM2_LAYER3      = 0x980,
  SIMULCAST_STREAM2_LAYER_ALL           = 0xB80,
  SIMULCAST_STREAM3_LAYER0      = 0xC00,
  SIMULCAST_STREAM3_LAYER1      = 0xC80,
  SIMULCAST_STREAM3_LAYER2      = 0xD00,
  SIMULCAST_STREAM3_LAYER3      = 0xD80,
  SIMULCAST_STREAM3_LAYER_ALL           = 0xF80,
  SIMULCAST_STREAMALL_LAYER0            = 0x1C00,
  SIMULCAST_STREAMALL_LAYER1            = 0x1C80,
  SIMULCAST_STREAMALL_LAYER2            = 0x1D00,
  SIMULCAST_STREAMALL_LAYER3            = 0x1D80,
  SIMULCAST_STREAMALL_LAYER_ALL                 = 0x1F80
} StreamLayerId_t;


typedef enum {
  CODEC_VP8       = 0x00,    /*VP8 single stream*/
  CODEC_VP8_SIMULCAST     = 0x01,    /*VP8 simulcast streams*/
  CODEC_H264        = 0x02,   /*h264 single stream*/
  CODEC_H264_SIMULCAST  = 0x03    /*h264 simulcast streams*/
} EuExSelectCodec_t;

/*supported H264 profile*/
typedef enum {
    BASELINE_PROFILE          =0x4240,  /*Constrained Beseline Profile*/
  MAIN_PROFILE        =0x4D00,  /*Main profile*/
  CONSTRAINED_HIGH_PROFILE    =0x640C,  /*Constrained High Profile*/
  HiGH_PROFILE                =0x6400   /* High Profile*/
}EuH264Profile_t;

/*supported H264 profile*/
typedef enum {
  CAVLC = 0x01,
  CABAC=  0x10,
}EuH264Entropy_t;

/*rate control mode*/
typedef enum {
    VBR             =1,  /*Variable Bit Rate low delay (VBR)*/
  CONSTANT_QP       =3,  /*Constant QP*/
  GVBR          =4   /*Global VBR low delay (GVBR)*/
}EuRateControlMode_t;

/*Error Resiliency features*/
typedef enum {
    RMIR              =0x01,  /*Random Macroblock Intra Refresh*/
  FRAME_LEVEL       =0x02,  /*Frame level error resiliency*/
  PARTITION_LEVEL     =0x04   /*Partition level error resiliency*/
}EuErrorResiliencyMode_t;


/*Stream Layer Control*/
typedef enum {
    LAYER_STOP            =0,  /*Stream Layer off*/
  LAYER_START         =1,  /*Stream Layer on*/
}EuStreamLayerControl_t;



#define H264_MIN_QP 10
#define H264_MAX_QP 51

#define VP8_MIN_QP 0
#define VP8_MAX_QP 127

/*****************************QIC1822 EU Controls for UVC1.0**************************************/



/* Data types for UVC control data */
#define UVC_CTRL_DATA_TYPE_RAW    0
#define UVC_CTRL_DATA_TYPE_SIGNED 1
#define UVC_CTRL_DATA_TYPE_UNSIGNED 2
#define UVC_CTRL_DATA_TYPE_BOOLEAN  3
#define UVC_CTRL_DATA_TYPE_ENUM   4
#define UVC_CTRL_DATA_TYPE_BITMASK  5

/* Control flags */
#define UVC_CONTROL_SET_CUR (1 << 0)
#define UVC_CONTROL_GET_CUR (1 << 1)
#define UVC_CONTROL_GET_MIN (1 << 2)
#define UVC_CONTROL_GET_MAX (1 << 3)
#define UVC_CONTROL_GET_RES (1 << 4)
#define UVC_CONTROL_GET_DEF (1 << 5)
/* Control should be saved at suspend and restored at resume. */
#define UVC_CONTROL_RESTORE (1 << 6)
/* Control can be updated by the camera. */
#define UVC_CONTROL_AUTO_UPDATE (1 << 7)

#define UVC_CONTROL_GET_RANGE (UVC_CONTROL_GET_CUR | UVC_CONTROL_GET_MIN | \
         UVC_CONTROL_GET_MAX | UVC_CONTROL_GET_RES | \
         UVC_CONTROL_GET_DEF)

struct uvc_xu_control_info {
  __u8 entity[16];
  __u8 index;
  __u8 selector;
  __u16 size;
  __u32 flags;
};


struct uvc_menu_info {
  __u32 value;
  __u8 name[32];
};

/*Mapping structure used from 2.6.32 - 2.6.35 kernel*/
struct uvc_xu_control_mapping {
  __u32 id;
  __u8 name[32];
  __u8 entity[16];
  __u8 selector;

  __u8 size;
  __u8 offset;
  enum v4l2_ctrl_type v4l2_type;
  __u32 data_type;
};

/*Mapping structure used for 2.6.36 - 2.6.41 kernel*/
struct uvc_xu_control_mapping_2_6_36 {
        __u32 id;
        __u8 name[32];
        __u8 entity[16];
        __u8 selector;

        __u8 size;
        __u8 offset;
        enum v4l2_ctrl_type v4l2_type;
        __u32 data_type;
  struct uvc_menu_info /*__user*/ *menu_info;
  __u32 menu_count;

  __u32 reserved[4];
};

/*Mapping structure used for kernel 2.6.42 (3.2) and greater kernels*/
struct uvc_xu_control_mapping_2_6_42 {
  __u32 id;
  __u8 name[32];
  __u8 entity[16];
  __u8 selector;

  __u8 size;
  __u8 offset;
  __u32 v4l2_type;
  __u32 data_type;

  struct uvc_menu_info *menu_info;
  __u32 menu_count;

  __u32 reserved[4];
};

struct uvc_xu_control {
  __u8 unit;
  __u8 selector;
  __u16 size;
  __u8  *data;
#ifdef ADD_FORCE
        __u8  force; //Quanta
#endif
};

struct uvc_xu_control_query {
  __u8 unit;
  __u8 selector;
  __u8 query;
  __u16 size;
  __u8 *data;
};

#define UVCIOC_CTRL_ADD   _IOW('U', 1, struct uvc_xu_control_info)
#define UVCIOC_CTRL_MAP   _IOWR('U', 2, struct uvc_xu_control_mapping)
#define UVCIOC_CTRL_GET   _IOWR('U', 3, struct uvc_xu_control)
#define UVCIOC_CTRL_SET   _IOW('U', 4, struct uvc_xu_control)


//The mapping macros are duplicated per driver difference.
#define UVCIOC_CTRL_MAP_2_6_32  _IOWR('U', 2, struct uvc_xu_control_mapping_2_6_32)
#define UVCIOC_CTRL_MAP_2_6_36  _IOWR('U', 2, struct uvc_xu_control_mapping_2_6_36)

#define UVCIOC_CTRL_MAP_2_6_42  _IOWR('u', 0x20, struct uvc_xu_control_mapping_2_6_42)
#define UVCIOC_CTRL_QUERY _IOWR('u', 0x21, struct uvc_xu_control_query)

//#define UVCIOC_CTRL_QUERY       _IOWR('U', 5, struct uvc_xu_control_query)
//Defines supported UVC control commands

int QicSetDeviceHandle (int vd);

#if defined(QIC_MPEGTS_API)
int QicEncoderGetParams (EncoderParams_t *params);
int QicEncoderSetParams (EncoderParams_t *params, unsigned char flag);
int QicEncoderSetIFrame ();
int QicEncoderGetGOP (int *gop);
int QicEncoderSetGOP (int gop);
int QicEncoderGetQuality(EncoderQuality_t *quality);
int QicEncoderSetQuality(EncoderQuality_t *quality, unsigned char flag);
int QicEncoderGetNumberOfProfiles(int *num_profiles);
int QicEncoderGetProfile(int index, int *max_level, int *profile, int *constraint_flags);
int QicEncoderGetProfileAndLevel(int *level, int *profile, int *constraint_flags);
int QicEncoderSetProfileAndLevel(int level, int profile, int constraint_flags);
int QicSetStreamFormat(EncoderStreamFormat_t format);
int QicGetStreamFormat(EncoderStreamFormat_t *format);
int QicSetEncoderFrameSizeFormat(EncoderStreamSizeFormat_t  format);
int QicGetEncoderFrameSizeFormat(EncoderStreamSizeFormat_t  *format);
int QicVideoSetRateControl(EncoderRateControlMode_t rate_control_type, unsigned char min_QP_value, unsigned char max_QP_value);
int QicVideoGetRateControl(EncoderRateControlMode_t *rate_control_type, unsigned char *min_QP_value, unsigned char *max_QP_value);

#endif

int QicMmioWrite (unsigned int addr, unsigned int value);
int QicMmioRead (unsigned int addr, unsigned int *value);

int QicFlashErase(void);
int QicFlashSectorErase (int addr);
int QicFlashCustom(char *data, int data_size);
int QicFlashWrite(int addr, unsigned char *data, int data_size, unsigned int max_flash_size);
int QicFlashWriteUSBIMG (int addr, unsigned char *data, int data_size, unsigned int max_flash_size);
int QicFlashRead(int addr, unsigned char *data, int data_size, unsigned int max_flash_size);
int QicFlashSetSpiConfig(unsigned int divider, FlashProgramMode_t mode);




int QicXuSet (unsigned int ctrlId, unsigned char* pBuf, unsigned int size);
int QicXuGet (unsigned int ctrlId, unsigned char* pBuf, unsigned int size);


int QicSetFlipMode(unsigned char flip_v, unsigned char flip_h);
int QicGetFlipMode(unsigned char *flip_v, unsigned char *flip_h);

int QicReset ();
int QicSetPll ();
int QicGetFirmwareVersion(FirmwareVersion_t *version);
int QicGetFwAPIVersion (unsigned char *version);
int QicSetSerialNumber( unsigned char *serial_number, unsigned char data_len);
int QicGetSerialNumber(char *serial_number, int data_len, int *bytes_returned);
int QicGetSvnVersion(unsigned int *version);
int QicSetLedMode(unsigned char mode);
int QicGetLedMode(unsigned char *mode);
int QicSetLEDBrightness(unsigned char brightness);
int QicGetLEDBrightness(unsigned char *brightness);


int QicSetEncoderEntropy (EncoderEntropy_t Entropy);
int QicGetEncoderEntropy (EncoderEntropy_t *Entropy);
int QicEncoderGetNumberOfCapabilities( unsigned char *count);
int QicEncoderGetStreamCaps (unsigned char index, EncoderCapability_t *capability);
int QicEncryptorSetKey(char *key, EncyptKeyMode_t key_mode);
int QicEncryptorGetKey(char *key, EncyptKeyMode_t key_mode);
int QicEncryptorSetConfig(int enable, EncyptKeyMode_t key_mode);
int QicSetCalibrateMode(CalibratMode_t mode);
int QicSetSpecialEffect(SpecialEffect_t effect);
int QicSetWBComp(unsigned int r_gain, unsigned int g_gain, unsigned int b_gain);
int QicSetExpComp(unsigned int exp_time, unsigned short exp_gain);
int QicSetAeSpeed(unsigned short time_step, unsigned char gain_step);
int QicSetEncoderOption( unsigned char encoder_option); // Not implement

int QicSetPanTilt(signed short pan, signed short tilt);
int QicGetPanTilt(signed short *pan, signed short *tilt);
int QicSetShutterMode(unsigned char mode);
int QicGetShutterMode(unsigned char *mode);

int QicSetCropped(unsigned char crop_enable, CROPRECT crop_window);
int QicSetTrimming(unsigned char trim_enable, TRIMRECT trim_window);
int QicGetTrimming(unsigned char* trim_enable, TRIMRECT* trim_window);
int QicGetALS(unsigned short *als_value);
int QicSetMotorControl(unsigned char control, unsigned char direction);
int QicGetMotorStatus(unsigned char *motor_status);

int QicSetWUSBWired(void);
int QicSetWUSBWireless(void);
int QicSetWUSBDisableAutoSwitch(void);
int QicSetIRControl(unsigned char mode);
int QicGetIRStatus(unsigned char *ir_status, unsigned char *ALS_status);
int QicSetAdvMotorControl(AdvMotorCtlMode_e mode, unsigned char set_number,AdvMotorCtl_t setting);
int QicGetAdvMotorPosition( MotorPosition_t *postion);
int QicGetSensorAEInfo(unsigned short *AE_statistic);
int QicSetLockStream(unsigned char lock);
int QicGetCheckLockStreamStatus(unsigned char *isLock, unsigned char *isStream);
int QicGetCapabilityList(unsigned char *support_number, unsigned int *support_list);

/* MJPEG QP value control */
int QicSet_MJPEG_QP_Value(unsigned char value);
int QicGet_MJPEG_QP_Value(unsigned char *value);

/*AVC Image Mirror Control*/
int QicSetMirror(unsigned char mirror);
int QicGetMirror(unsigned char *mirror);

int QicGetSysInfo(SysInfo_t *sysinfo);
int QicGetChipVer(QicChipId_t *ver);
int QicGetHWVersion(QicHwVer_t *version);
int QicGet1822HWVersion(Qic1822HwVer_t *version);
int QicSetADCMode(ADCMode_t mode);
int QicGetADCMode(ADCMode_t *mode);

int QicQueryDevice(void);

void QicChangeFD(int fd);

/* -- Audio Control API -- */
int QicSetANRStatus(unsigned char onoff);
int QicGetANRStatus(unsigned char *onoff);
int QicSetEQStatus(unsigned char onoff, unsigned char type);
int QicGetEQStatus(unsigned char *onoff, unsigned char *type);
int QicSetBFStatus(unsigned char onoff);
int QicGetBFStatus(unsigned char *onoff);



#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)
/* qic1832 OSD Control just for H.264 stream */
#ifdef QIC_OSD_API
int QicOsdStatusSet (unsigned char osd_status);
int QicOsdStatusGet (unsigned char *osd_status);
int QicOsdMiscellSet (OsdMiscell_t osd_miscell);
int QicOsdMiscellGet (OsdMiscell_t *osd_miscell);
int QicOsdColorSet (OsdColor_t osd_color[OSD_COLOR_MAX]);
int QicOsdColorGet (OsdColor_t *osd_color);
int QicOsdFrameSet (OsdFrame_t osd_frame[OSD_FRAME_MAX]);
int QicOsdFrameGet (OsdFrame_t *osd_frame);
int QicOsdLineAttrSet(unsigned char line_id, OsdLineAttr_t line_attr);
int QicOsdLineAttrGet(unsigned char line_id, OsdLineAttr_t *line_attr);
int QicOsdLineStringSet(unsigned char line_id, unsigned char start_char_index,
                  unsigned char *str, OsdCharAttr_t char_attr);
int QicOsdLineStringGet(unsigned char line_id, unsigned char start_char_index,
                  unsigned char *str, OsdCharAttr_t *char_attr);
int QicOsdLineArraySet(unsigned char line_id, unsigned char str[OSD_CHAR_MAX],
                  OsdCharAttr_t char_attr[OSD_CHAR_MAX]);
int QicOsdLineArrayGet(unsigned char line_id, unsigned char *str,
                  OsdCharAttr_t *char_attr);
int QicOsdLineClear(unsigned char line_id);
int QicOsdTimerSet(unsigned char line_id, unsigned char enable, OsdTimer_t timer);

#endif

#ifdef QIC_MD_API
/* Motion detection APIs */
int QicMDGetVersion(int *major_version, int *minor_version);
int QicMDSetEnable(unsigned char enable);
int QicMDGetEnable(unsigned char *enable);
int QicMDSetConfiguration(md_config_t* config);
int QicMDGetConfiguration(md_config_t* config);
int QicMDGetStatus(md_status_t* status);
int QicMDSetInterruptMode(unsigned char mode);
int QicMDGetInterruptMode(unsigned char* mode);
int QicMDWaitMotion(md_status_t* status);
int QicSetPeakBitrate(unsigned int dwPeakBirtate);
int QicGetPeakBitrate(unsigned int *dwPeakBirtate);

int QicTDGetVersion(int *major_version, int *minor_version);
int QicTDSetEnable(unsigned char enable);
int QicTDGetEnable(unsigned char *enable);
int QicTDSetConfiguration(td_config_t* config);
int QicTDGetConfiguration(td_config_t* config);
int QicTDGetStatus(int* status);

#endif


/* EU Codec Controls for UVC1.0 */
int QicEuSetSelectLayer(unsigned short wLayerOrViewID);
int QicEuGetSelectLayer(unsigned short *wLayerOrViewID);
int QicEuSetVideoResolution(unsigned short wWidth, unsigned short wHeight);
int QicEuGetVideoResolution(unsigned short *wWidth, unsigned short *wHeight);
int QicEuSetProfileAndToolset(unsigned short wProfile, unsigned short wConstrainedToolset, unsigned char bmSettings);
int QicEuGetProfileAndToolset(unsigned short *wProfile, unsigned short *wConstrainedToolset, unsigned char *bmSettings);
int QicEuSetMinimumFrameInterval(unsigned int dwFrameInterval);
int QicEuGetMinimumFrameInterval(unsigned int *dwFrameInterval);
int QicEuSetSliceMode(unsigned short wSliceMode, unsigned short wSliceConfigSetting);
int QicEuGetSliceMode(unsigned short *wSliceMode, unsigned short *wSliceConfigSetting);
int QicEuSetRateControlMode(unsigned char bRateControlMode);
int QicEuGetRateControlMode(unsigned char *bRateControlMode);
int QicEuSetAverageBitrateControl(unsigned int dwAverageBitRate);
int QicEuGetAverageBitrateControl(unsigned int *dwAverageBitRate);
int QicEuSetCpbSizeControl(unsigned int dwCPBsize);
int QicEuGetCpbSizeControl(unsigned int *dwCPBsize);
int QicEuSetQuantizationParameter(unsigned short wQpPrime_I, unsigned short wQpPrime_P, unsigned short wQpPrime_B);
int QicEuGetQuantizationParameter(unsigned short *wQpPrime_I, unsigned short *wQpPrime_P, unsigned short *wQpPrime_B);
int QicEuSetSynchronizationAndLongTermReferenceFrame(unsigned char bSyncFrameType, unsigned short wSyncFrameInterval, unsigned char bGradualDecoderRefresh);
int QicEuGetSynchronizationAndLongTermReferenceFrame(unsigned char *bSyncFrameType, unsigned short *wSyncFrameInterval, unsigned char *bGradualDecoderRefresh);
int QicEuSetLevelIdc(unsigned char bLevelIDC);
int QicEuGetLevelIdc(unsigned char *bLevelIDC);
int QicEuSetQpRange(unsigned char bMinQp, unsigned char bMaxQp);
int QicEuGetQpRange(unsigned char *bMinQp, unsigned char *bMaxQp);
int QicEuSetStartOrStopLayer(unsigned char bUpdate);
int QicEuGetStartOrStopLayer(unsigned char *bUpdate);
int QicEuSetErrorResiliency(unsigned short bmErrorResiliencyFeatures);
int QicEuGetErrorResiliency(unsigned short *bmErrorResiliencyFeatures);
int QicEuExSetSelectCodec(EuExSelectCodec_t bCodec);
int QicEuExGetSelectCodec(EuExSelectCodec_t *bCodec);
int QicEuExSetTsvc(unsigned char bTemporalLayer);
int QicEuExGetTsvc(unsigned char *bTemporalLayer);
#endif

#ifdef __cplusplus
}
#endif


#endif
