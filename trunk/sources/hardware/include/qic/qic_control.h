/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#ifndef _QIC_CONTROL_H_
#define _QIC_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "demux.h"
#include "qic_xuctrl.h"
#include "vp8demux.h"
#include "mdparse.h"

#ifdef SKYPEAPI
#include "../include/qic/sky_xuctrl.h"
#endif

#define DEMUX_BUF_MAX 8
#define MAX_SUPPORT_DEVICES 8

/* firmware related */
#define USB_FIRMWARE 0x01
#define ISP_FIRMWARE 0x02
#define PARAM_FIRMWARE 0x04
#define AUDIO_FIRMWARE 0x08
#define OSD_FONT_FIRMWARE 0x10

/* device ID define - dirty */
#define DEV_ID_0 0x01
#define DEV_ID_1 0x02
#define DEV_ID_2 0x04
#define DEV_ID_3 0x08
#define DEV_ID_4 0x10
#define DEV_ID_5 0x20
#define DEV_ID_6 0x40
#define DEV_ID_7 0x80

/* flip define */
#define H_FLIP 0x01
#define V_FLIP 0x02

/* debug message type define */
#define DEBUG_ERROR 0x01
#define DEBUG_INFO 0x02
#define DEBUG_FRAME 0x04
#define DEBUG_DETAIL 0x08

#define PTZ_PAN_STEP    0xE10
#define PTZ_TILT_STEP   0xE10
#define PTZ_ZOOM_STEP   0x01

/*simulcast stream id define*/
#define MAX_SIMULCAST_STREAM 4
#define STREAM0 0
#define STREAM1 1
#define STREAM2 2
#define STREAM3 3




//#define USE_K26_24

#ifdef USE_K26_24

#if 1
//#define V4L2_CTRL_CLASS_USER 0x00980000
//#define V4L2_CID_BASE     (V4L2_CTRL_CLASS_USER | 0x900)
//#define V4L2_CID_USER_BASE    V4L2_CID_BASE
#define V4L2_CTRL_CLASS_CAMERA 0x009a0000 /* Camera class controls */
#define V4L2_CID_POWER_LINE_FREQUENCY (V4L2_CID_BASE+24)
enum v4l2_power_line_frequency {
  V4L2_CID_POWER_LINE_FREQUENCY_DISABLED  = 0,
  V4L2_CID_POWER_LINE_FREQUENCY_50HZ  = 1,
  V4L2_CID_POWER_LINE_FREQUENCY_60HZ  = 2,
};
#define V4L2_CID_HUE_AUTO     (V4L2_CID_BASE+25)
#define V4L2_CID_WHITE_BALANCE_TEMPERATURE  (V4L2_CID_BASE+26)
#define V4L2_CID_SHARPNESS      (V4L2_CID_BASE+27)
#define V4L2_CID_BACKLIGHT_COMPENSATION   (V4L2_CID_BASE+28)
#define V4L2_CID_CHROMA_AGC                     (V4L2_CID_BASE+29)
#define V4L2_CID_COLOR_KILLER                   (V4L2_CID_BASE+30)
/* last CID + 1 */
#define V4L2_CID_LASTP1                         (V4L2_CID_BASE+31)

/*  Camera class control IDs */
#define V4L2_CID_CAMERA_CLASS_BASE  (V4L2_CTRL_CLASS_CAMERA | 0x900)
#define V4L2_CID_CAMERA_CLASS     (V4L2_CTRL_CLASS_CAMERA | 1)

#define V4L2_CID_EXPOSURE_AUTO      (V4L2_CID_CAMERA_CLASS_BASE+1)
enum  v4l2_exposure_auto_type {
  V4L2_EXPOSURE_AUTO = 0,
  V4L2_EXPOSURE_MANUAL = 1,
  V4L2_EXPOSURE_SHUTTER_PRIORITY = 2,
  V4L2_EXPOSURE_APERTURE_PRIORITY = 3
};
#define V4L2_CID_EXPOSURE_ABSOLUTE    (V4L2_CID_CAMERA_CLASS_BASE+2)
#define V4L2_CID_EXPOSURE_AUTO_PRIORITY   (V4L2_CID_CAMERA_CLASS_BASE+3)

#define V4L2_CID_PAN_RELATIVE     (V4L2_CID_CAMERA_CLASS_BASE+4)
#define V4L2_CID_TILT_RELATIVE      (V4L2_CID_CAMERA_CLASS_BASE+5)
#define V4L2_CID_PAN_RESET      (V4L2_CID_CAMERA_CLASS_BASE+6)
#define V4L2_CID_TILT_RESET     (V4L2_CID_CAMERA_CLASS_BASE+7)

#define V4L2_CID_PAN_ABSOLUTE     (V4L2_CID_CAMERA_CLASS_BASE+8)
#define V4L2_CID_TILT_ABSOLUTE      (V4L2_CID_CAMERA_CLASS_BASE+9)

#define V4L2_CID_FOCUS_ABSOLUTE     (V4L2_CID_CAMERA_CLASS_BASE+10)
#define V4L2_CID_FOCUS_RELATIVE     (V4L2_CID_CAMERA_CLASS_BASE+11)
#define V4L2_CID_FOCUS_AUTO             (V4L2_CID_CAMERA_CLASS_BASE+12)
#endif
#define V4L2_CID_ZOOM_ABSOLUTE              (V4L2_CID_CAMERA_CLASS_BASE+13)
#endif


typedef struct {
  unsigned short width;
  unsigned short height;
  unsigned char framerate;
} SUPPORT_FORMAT;


typedef struct{
  unsigned short  width;
  unsigned short  height;
      unsigned int frame_interval;
  unsigned int bitrate;
  unsigned short key_frame_interval;
}simulcast_config_t;

typedef struct{
  simulcast_config_t configs[MAX_SIMULCAST_STREAM];
  char config_all;
}simulcast_configs_t;

typedef struct {
unsigned char bPframe;         //is P frame
unsigned char bencding_stream;   //is encoding stream
unsigned int stream_id;                //stream id of simulcast streams
unsigned long timestamp;       // timestamp
unsigned int temporal_layer_id;    // temporal layer id of steram
unsigned int frame_len;                // frame size
char * frame_data;         // frame data
#ifdef QIC_MD_API
md_status_t md_status;
#endif
} out_frame_t;


typedef struct {
  long hw_version;
  long vid;
  long pid;
  long revision;
  unsigned int svn;
  unsigned char fw_api_version;
} version_info_t;

typedef struct {
  void * start;
  size_t length;
}mmap_buffer_s;

typedef struct{
 signed int max;
 signed int min;
 signed int defuat;
 signed int now;
}sqicv4l2value;


typedef struct {
  char dev_yuv[16];
  char dev_avc[16];
}qic_dev_name_s;

typedef struct {
    sqicv4l2value Brightness;
    sqicv4l2value Contrast;
    sqicv4l2value Hue;
   sqicv4l2value BC;
    sqicv4l2value WB;
   sqicv4l2value Saturation;
   sqicv4l2value Focus;
   sqicv4l2value Sharpness;
     sqicv4l2value Zoom;
   sqicv4l2value Gamma;
  sqicv4l2value Gain;
  sqicv4l2value Plf;
   sqicv4l2value E_priority;
   sqicv4l2value Exposure;
   sqicv4l2value Pan;
   sqicv4l2value Tilt;
 }sqicV4L2;

typedef struct {

  /* device parameters*/
  char* dev_name;
  int fd;
  unsigned int dev_id; /* for dyn-config each device, generated by config_initial */
  unsigned char is_bind; /*for bind check, 2-way video*/


  /* video parameters */
  unsigned int format; /* YUV or MPEG2TS */

  unsigned int bitrate;  /* ~8000000bps */
  unsigned short width; /* ~1920 */
  unsigned short height; /* ~1080 */
  unsigned char framerate; /* fps */
  int gop; /* group of picture */
  unsigned char slicesize; /* 0 -auto, 1or up - slice height, i.e. 640 * (16*slicesize) */
            /* HD resolution force to 0 in Linux */

  unsigned char is_demux; /* on or off (need "much" CPU resource) */

  /* NRI related */
  unsigned char nri_iframe;
  unsigned char nri_sps;
  unsigned char nri_pps;
  unsigned char nri_set;

  /* internal usage */
  unsigned char is_on; /* start / stop capture */
  mmap_buffer_s* buffers; /* mmap buffer */
  unsigned int buffer_start_address; /* __not implement__ */
  unsigned int num_mmap_buffer; /* for kernel UVC buffer */
  unsigned char skype_stream;  /*for SECS2.2 api*/
  unsigned int frame_interval;
#if defined(QIC1816)&&defined(QIC_MPEGTS_API)
  demux demux_struc; /* for demuxer - H.264 only*/
  H264DATAFORMAT* out[DEMUX_BUF_MAX];
  int cur_bad_count;
  EncoderStreamFormat_t stream_format;  /*Transport stream or Elementry stream*/
    EncoderStreamSizeFormat_t stream_size_format;
        unsigned char fw_api_version;  /*F/W api version*/
#endif
  unsigned char is_encoding_video;
  EuExSelectCodec_t codec_type;  /*codec type*/
  unsigned short key_frame_interval;
  simulcast_configs_t simulcast_configs_setting;
  /* raw V4L2 Data Dump int length, char *data */
  void (*raw_dump)(int, char*);

} qic_dev;

typedef struct {

  /* for config initialization */
  unsigned int num_devices; /* max 8 devices */
  qic_dev *cam;

  /* output frame buffer: dev_id, length, data, timestamp*/
  void (*frame_output)(unsigned int, unsigned int, char *, unsigned long);
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)
  /* output simulcast frame buffer: dev_id, simulcat_frame_t*/
  void (*frame_output2)(unsigned int, out_frame_t);
#endif
  /*debug info print: int level, char* string*/
  void (*debug_print)(int, char*);
  /* debug msg type - ERROR, INFO, FRAME, */
  unsigned int debug_msg_type;

  /* __TEST__ higher prio for less frame lost */
  unsigned char high_prio;

} qic_module;

typedef enum {
  FW_PASS     = 0,   //FW upgarde success
  FW_FAIL     = 1,   //FW upgarde fail
  FW_NO_2ND_BL    = 2,   //no found 2nd Boot rom on flash
  FW_IS_STREAMING   = 3,   //qic video/audio is streaming
  FW_IS_DOWNLAODING = 4
} FWDLERRCODE_e;


void qic_get_proc_devname(qic_dev_name_s *devname );
int qic_enum_device_formats(qic_dev_name_s *name);
int qic_show_dev_format(unsigned int dev_id);

/* intial, setup, teardown */
qic_module* qic_initialize(int num_devices);
int qic_config_commit(void);
int qic_force_config(void);
int qic_release(void);

/* utility/debug functions */
int qic_get_fd_from_devid(unsigned int dev_id); /* if multiple fd, return first, -1 as not found */
char* qic_print_config_param(unsigned int dev_id);
/* __not implement
char * qic_print_video_cap_statistics(unsigned int dev_id);
*/


/* firmware related */
int qic_get_system_version (unsigned int dev_id, version_info_t *version_info);
int qic_get_image_version (char *path_info, version_info_t *version_info);
int qic_backup_firmware_to_file(unsigned int dev_id, char *update_img_loc, unsigned char update_flags, char *backup_img_loc, unsigned char backup_flags,QicHwVer_t version) ;
int qic_update_firmware_by_filename(unsigned int dev_id,
                      char *update_usb_img_loc,
                      char *update_isp_img_loc,
                      char *update_param_img_loc,
                      char *update_audio_img_loc,
                      char *update_osd_font_img_loc,
                      unsigned char update_flags,
                      QicHwVer_t version);
int qic_get_image_version_by_filename (char *usb_path_info, version_info_t *version_info);
int qic_check_firmware(unsigned int dev_id,unsigned long checksum);
int qic_check_firmware_by_file(unsigned int dev_id,char *check_img_loc);
int qic_update_audio_data_by_filename(unsigned int dev_id, char *update_audio_img_loc);
int qic_update_osd_font_by_filename(unsigned int dev_id, char *update_osd_font_img_loc);

int qic_dump_all_flash(unsigned int dev_id);
#ifdef QIC_SUPPORT_2ND_BL
int qic_update_2nd_bootrom_by_filename(unsigned int dev_id, char eraseAll, char *update_BL2nd_img_loc);
int qic_backup_2nd_Boot_Rom(unsigned int dev_id, char *backup_BL2nd_img_loc);
#endif
/*V4L2 function*/
int qic_change_V4L2_FOCUS_ABSOLUTE(unsigned int dev_id,  unsigned int Auto,signed long  ABSOLUTE);
int qic_change_V4L2_BACKLIGHT_COMPENSATION(unsigned int dev_id, signed long  BC) ;
int qic_V4L2_Control(int fd,unsigned long cmd,int Get,signed long *value,signed long invalue);
int qic_change_V4L2_flip(unsigned int dev_id, unsigned char flip) ;
int qic_change_V4L2_WHITE_BALANCE(unsigned int dev_id, unsigned int  Auto,signed long WB);
int qic_change_V4L2_BRIGHTNESS(unsigned int dev_id,signed long BRIGHTNESS);
int qic_change_V4L2_CONTRAST(unsigned int dev_id, signed long  CONTRAST);
int qic_change_V4L2_HUE(unsigned int dev_id, signed long HUE);
int qic_change_V4L2_SATURATION(unsigned int dev_id, signed long SATURATION);
int qic_change_V4L2_SHARPNESS(unsigned int dev_id, signed long SHARPNESS);
int qic_change_V4L2_GAMMA(unsigned int dev_id, signed long GAMMA);
int qic_change_V4L2_GAIN(unsigned int dev_id, signed long GAIN);
int qic_change_V4L2_POWER_LINE_FREQUENCY(unsigned int dev_id, signed long PLF);
int qic_change_V4L2_EXPOSURE(unsigned int dev_id, unsigned int Auto, signed long ABSOLUTE);
int qic_change_V4L2_EXPOSURE_AUTO_PRIORITY(unsigned int dev_id, signed long PRIORITY);
int qic_change_V4L2_ZOOM_ABSOLUTE(unsigned int dev_id, signed long ABSOLUTE);

#if 0 /*QIC not use this function*/
int qic_change_V4L2_Pan_Tilt2(unsigned int dev_id,signed short pan_value, signed short tilt_value);
#endif

/* run-time control */
int qic_start_capture(unsigned int dev_id);
int qic_stop_capture(unsigned int dev_id);

/*control APIs for AVC*/

#if defined(QIC1816)
int qic_change_resolution(unsigned int dev_id, unsigned short width, unsigned short height);
#endif

#if defined(QIC_MPEGTS_API)
int qic_change_bitrate(unsigned int dev_id, unsigned int bitrate);
int qic_change_framerate(unsigned int dev_id, unsigned char framerate);
int qic_change_slicesize(unsigned int dev_id, unsigned char slicesize);
int qic_change_gop(unsigned int dev_id, int gop);
int qic_setIframe(unsigned int dev_id);
int qic_get_number_of_profiles(unsigned int dev_id, int *num);
int qic_get_profile(unsigned int dev_id, int idx, int *max_level, int *profile, int *constraint_flags);
int qic_set_profile_and_level(unsigned int dev_id, int level, int profile, int constraint_flags);
int qic_get_profile_and_level(unsigned int dev_id, int *level, int *profile, int *constraint_flags);
int qic_change_streamformat(unsigned int dev_id, EncoderStreamFormat_t mode);
int qic_change_stream_size_format(unsigned int dev_id, EncoderStreamSizeFormat_t  mode);
int qic_change_rate_control_mode(int dev_id, EncoderRateControlMode_t rate_control_type, unsigned char min_QP_value, unsigned char max_QP_value);

char* qic_print_demux_statistics(unsigned int dev_id);

#endif

unsigned int qic_get_yuyv_bad_frame_count(void);
unsigned int qic_get_vp8_bad_frame_count(void);
unsigned int qic_get_h264_bad_frame_count(void);

int qic_getframe(unsigned int dev_id);

/* not implemented yet --- (all other camera function for tuning)
*/

/* Get H.264/VP8 Encoder Number of Capabilities*/
int qic_get_frame_number_of_encoder(unsigned char *count);
/* Get H.264/VP8 Encoder Stream Capabilities (width, height, bitrate, framerate)*/
int qic_get_descriptor_of_encoder(unsigned char index, EncoderCapability_t *capability);

/*H.264 entropy*/
int qic_change_entropy(unsigned int dev_id,EncoderEntropy_t entropy);

/* H/V flip*/
int qic_change_flip(unsigned int dev_id, unsigned char flip) ;

/*LED mode*/
int qic_change_LED_Mode( unsigned char mode);
int qic_change_LED_Brightness( unsigned char brightness);

/*pan tilt control*/
int qic_change_pan_tilt(unsigned int dev_id, signed short pan, signed short tilt);

/*shutter mode control*/
int qic_change_Shutter_Mode( unsigned char mode);

/* MJPEG QP control */
int qic_get_MJPEG_QP_Value(unsigned char *value);
int qic_change_MJPEG_QP_Value(unsigned char value);

/*mirror control*/
int qic_change_mirror_mode(int dev_id, unsigned char mode);

/*cropped image control*/
int qic_set_cropped_image(unsigned int dev_id,unsigned int crop_switch,CROPRECT crop_window);

/*trimming control*/
int qic_set_trimming(unsigned int dev_id,unsigned int trim_enable,unsigned int x, unsigned y);
int qic_set_trim_zoom(unsigned int dev_id,unsigned int x, unsigned int y, unsigned int wdith, unsigned int height, unsigned char enable);
int qic_get_trim_zoom(unsigned int dev_id,unsigned int* x, unsigned int* y, unsigned int* wdith, unsigned int* height, unsigned char* enable);


/*get ALS value*/
int qic_get_ALS(unsigned int dev_id, unsigned short *ALS);

/*motor control*/
int qic_change_motor_control( unsigned char control, unsigned char direction);
int qic_get_motor_status( unsigned char *motor_status);

/*WUSB Wired & Wireless control*/
int qic_set_WUSB_wired(void);
int qic_set_WUSB_wireless(void);
int qic_set_WUSB_disable_auto_switch(void);

/*Get support Capability list*/
int qic_get_Capability_List( unsigned char *maximun_number, unsigned int *support_list);

/*Check Lock Streaming control*/
int qic_set_lock_steam_control( unsigned char lock);
int qic_check_lock_stream_status( unsigned char *isLock, unsigned char *isStream);

/*eume controls*/
int qic_enum_controls(int dev_id);
int qic_get_control_setting(int fd, sqicV4L2 *camerav4l2 );


/*system information*/
int qic_get_sysinfo(int dev_id,SysInfo_t *sysinfo);

/*get chip id*/
int qic_get_chip_id(QicChipId_t *chip_id);

/*Audio Function control*/
int qic_change_NR_mode(unsigned char onoff);
int qic_change_EQ_mode(unsigned char onoff, unsigned char type);
int qic_change_BF_mode(unsigned char onoff);

/*advanced motor control*/
int qic_set_advance_motor_postions( unsigned char set_numbers,MotorPosition_t Pos[] );
int qic_get_advance_motor_current_postion( MotorPosition_t *Pos );
int qic_stop_advance_motor( void );
int qic_start_advance_motor( void );
int qic_reset_position_advance_motor( void ) ;
int qic_set_advance_motor_speed( unsigned short pan_speed, unsigned short tilt_speed );

/*IR control*/
int qic_change_IR_control( unsigned char mode);
int qic_get_IR_ALS_status( unsigned char *ir_status, unsigned char *ALS_status);

/*Snesor Information*/
int qic_get_Sensor_AEInfo(unsigned short *AE_statistic);

/*EU control API for QIC1822*/
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)
int qic_getframe2(unsigned int dev_id);
int qic_config_codec_EU(unsigned int dev_id,EuExSelectCodec_t tCodec );
int qic_simulcast_config_codec_EU(unsigned int dev_id,EuExSelectCodec_t tCodec,simulcast_configs_t config_setting);
int qic_set_temporal_layer_number_EU(unsigned int dev_id,unsigned short stream_id,unsigned char temp_layer_num);
int qic_change_bitrate_EU( unsigned int dev_id,unsigned short stream_id,unsigned int ubitrate);
int qic_change_frame_interval_EU(unsigned int dev_id,unsigned short stream_id,unsigned int uframe_interval);
int qic_generate_key_frame_EU(unsigned int dev_id,unsigned short stream_id,unsigned char cSyncFrameType, unsigned short sSyncFrameInterval, unsigned char cGradualDecoderRefresh);
int qic_change_resolution_EU(unsigned int dev_id,unsigned short stream_id, unsigned short width, unsigned short height);
int qic_start_stop_layer_EU(unsigned int dev_id,unsigned short stream_layer,unsigned char on);
int qic_change_ErrorResiliency_EU(unsigned int dev_id, unsigned short errorResiliency);
int qic_change_CPB_size_EU(unsigned int dev_id,unsigned short stream_layer,unsigned int CPBsize);
int qic_change_profile_toolset_EU(unsigned int dev_id,  unsigned short stream_id,unsigned short wProfile, unsigned char bmSettings);
int qic_change_slice_size_EU(unsigned int dev_id,unsigned short stream_id, unsigned short wSliceConfigSetting);
int qic_change_rete_control_mode_EU(unsigned int dev_id,unsigned short stream_id, unsigned char bRateControlMode);
int qic_change_level_EU(unsigned int dev_id , unsigned short stream_id,unsigned char bLevelIDC);
int qic_change_QP_range_EU(unsigned int dev_id ,unsigned short stream_id,unsigned char bMinQp,unsigned char bMaxQp);
int qic_change_QP_EU(unsigned int dev_id ,unsigned short stream_layer, unsigned short value);
#ifdef QIC_OSD_API
int qic_osd_change_status(unsigned int dev_id,unsigned char on);
int qic_osd_change_color(unsigned int dev_id,OsdColor_t osd_color[OSD_COLOR_MAX]);
int qic_osd_change_transparency_blink(unsigned int dev_id,OsdMiscell_t set_misc);
int qic_osd_get_transparency_blink(unsigned int dev_id,OsdMiscell_t *set_misc);
int qic_osd_get_color(unsigned int dev_id,OsdColor_t *osd_color);
int qic_osd_change_frame(unsigned int dev_id,OsdFrame_t osd_frame[OSD_FRAME_MAX]);
int qic_osd_get_frame(unsigned int dev_id,OsdFrame_t *osd_frame);
int qic_osd_change_string(unsigned int dev_id,unsigned char line_id,unsigned char start_char_index,unsigned char *str, OsdCharAttr_t char_attr);
int qic_osd_change_char_of_string(unsigned int dev_id,unsigned char line_id, unsigned char *str, OsdCharAttr_t char_attr[OSD_CHAR_MAX]);
int qic_osd_get_char_of_string(unsigned int dev_id,unsigned char line_id, unsigned char *str, OsdCharAttr_t *char_attr);
int qic_osd_change_line_attr(unsigned int dev_id,unsigned char line_id,OsdLineAttr_t line_attr);
int qic_osd_get_line_attr(unsigned int dev_id,unsigned char line_id,OsdLineAttr_t *line_attr);
int qic_osd_change_timer(unsigned int dev_id,unsigned char line_id,unsigned char enable,OsdTimer_t timer );

#endif

#ifdef QIC_MD_API
/*MD*/
int qic_md_get_version(unsigned int dev_id,int *major_version, int *minor_version);
int qic_md_start_stop(unsigned int dev_id,unsigned char on);
int qic_md_change_config(unsigned int dev_id,md_config_t* config, unsigned char interrupt_mode);
int qic_md_get_config(unsigned int dev_id,md_config_t* config, unsigned char* interrupt_mode);
int qic_md_get_status(unsigned int dev_id,md_status_t* status);// no used
int qic_md_change_Peak_Bitrate(unsigned int dev_id,unsigned int stream_id, unsigned int peakBitrate);
int qic_md_get_Peak_Bitrate(unsigned int dev_id, unsigned short stream_id, unsigned int *peakBitrate);

/*TD*/
int qic_td_get_version(unsigned int dev_id,int *major_version, int *minor_version);
int qic_td_start_stop(unsigned int dev_id,unsigned char on);
int qic_td_change_config(unsigned int dev_id,td_config_t* config);
int qic_td_get_config(unsigned int dev_id,td_config_t* config);
int qic_td_get_status(unsigned int dev_id,int* status);


#endif

#endif

/*audio ADC mode control*/
int qic_change_ADC_mode(ADCMode_t mode);

#ifdef SKYPEAPI
int sky_Generate_Key_Frame(unsigned int dev_id);
int sky_change_frame_interval(unsigned int dev_id, unsigned int frame_interval);
int sky_change_bitrate(unsigned int dev_id, unsigned int bitrate);
int qic_compose_frame(char *oldframe, char **newframe, unsigned int old_len);
int sky_config_commit(unsigned int dev_id, sid_id_t u_stream_ID ,format_id_t u_format,unsigned short u_width, unsigned short u_height,
    unsigned int u_frame_interval,unsigned int u_bitrate);
#endif

#ifdef USE_THREAD
/* for testing */
int qic_getframe_loop_start(void); /* start getframe loop thread */
int qic_getframe_loop_stop(void); /* stop the getframe loop thread  */
#endif

#ifdef __cplusplus
}
#endif

#endif

