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
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sched.h>
#include <assert.h>
#include <time.h>

#ifdef USE_THREAD
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#endif

#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "../include/qic/qic_control.h"



#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* some debug defines */
//#define DEBUG_LOG

//#define COMMIT_CHECK
//#define ENABLE_PRINT

/* firmware image file name default */

#define QIC1802_USB_IMAGE_NAME "QIC1802A_USB.bin"
#define QIC1802_ISP_IMAGE_NAME "QIC1802A_ISP.bin"
#define QIC1802_PARAM_IMAGE_NAME "QIC1802A_ISP_LSC_XXX.bin"
#define QIC1802_SHAING_IMAGE_NAME "QIC1802A_ISP_LSC"

#define QIC1816_USB_IMAGE_NAME "QIC1806A_USB.bin"
#define QIC1816_ISP_IMAGE_NAME "QIC1806A_ISP.bin"
#define QIC1816_PARAM_IMAGE_NAME "QIC1806A_ISP_LSC_XXX.bin"
#define QIC1816_AUDIO_IMAGE_NAME "QIC1806A_AUDIO_DATA.bin"
#define QIC1816_SHAING_IMAGE_NAME "QIC1806A_ISP_LSC"

#define QIC1822_2nd_BOOTROM_IMAGE_NAME "QIC1822A_2ndBL.bin"
#define QIC1822_USB_IMAGE_NAME "QIC1822A_USB.bin"
#define QIC1822_ISP_IMAGE_NAME "QIC1822A_ISP.bin"
#define QIC1822_PARAM_IMAGE_NAME "QIC1822A_ISP_LSC_XXX.bin"
#define QIC1822_AUDIO_IMAGE_NAME "QIC1822A_AUDIO_DATA.bin"
#define QIC1822_OSD_FONT_IMAGE_NAME "QIC1822A_OSD_FONT_DATA.bin"

#define QIC1822_SHAING_IMAGE_NAME "QIC1822A_ISP_LSC"

#define QIC_FLASH_DUMP_IMAGE_NAME "_QIC_FLASH_DUMP.bin"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define FOURCC_FORMAT   "%c%c%c%c"
#define FOURCC_ARGS(c)    (c) & 0xFF, ((c) >> 8) & 0xFF, ((c) >> 16) & 0xFF, ((c) >> 24) & 0xFF
#define TIME_DELAY(ms)    usleep(ms*1000)
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#ifdef DEBUG_LOG
/* debug output function */
#define LOG_PRINT(str, level, format, arg...) { \
  memset (str, 0, sizeof(str)); \
  snprintf(str, sizeof(str), format, ##arg); \
  (*dev_pt->debug_print)(level, str); \
}

#define LOG_XU_PRINT(str_d, str_x, ret) LOG_PRINT(str_d, !ret ? DEBUG_INFO:DEBUG_ERROR, "[xuctrl] %s\n", str_x)

char debug_str[1024]={0}; /* debug string output */
char debug_xuctrl_str[512]={0}; /* debug string for xuctrl */
/* debug level, 0:information, 1:warning, 2:critical, 3:fatal */
/*static char *debug_level[] = {"INFO", "WARN", "CRIT", "FATL", "DEAD"};*/

#else
#define LOG_PRINT(str, level, format, arg...){}
#define LOG_XU_PRINT(str_d, str_x, ret){}
#endif

SUPPORT_FORMAT YUV_bind_format[] = {  \
  {384, 216, 0}, \
  {352, 288, 0}, \
  {320, 240, 0}, \
  {320, 200, 0}, \
  {320, 180, 0}, \
  {176, 144, 0}, \
  {160, 128, 0}, \
  {160, 120, 0}, \
  {160, 90, 0}, \
  {0,0,0} \
};

SUPPORT_FORMAT YUV_format[] = {   \
  {2592,1944, 2}, {1920,1080, 6},{1280, 800, 9},{1280, 720, 9}, \
  {768, 480, 30}, {768, 480, 25}, {768, 480, 20}, {768, 480, 15}, {768, 480, 10}, {768, 480, 5}, \
  {640, 480, 30}, {640, 480, 25}, {640, 480, 20}, {640, 480, 15}, {640, 480, 10}, {640, 480, 5}, \
  {640, 400, 30}, {640, 400, 25}, {640, 400, 20}, {640, 400, 15}, {640, 400, 10}, {640, 400, 5}, \
  {640, 360, 30}, {640, 360, 25}, {640, 360, 20}, {640, 360, 15}, {640, 360, 10}, {640, 360, 5}, \
  {384, 216, 30}, {384, 216, 25}, {384, 216, 20}, {384, 216, 15}, {384, 216, 10}, {384, 216, 5}, \
  {352, 288, 30}, {352, 288, 25}, {352, 288, 20}, {352, 288, 15}, {352, 288, 10}, {352, 288, 5}, \
  {320, 240, 30}, {320, 240, 25}, {320, 240, 20}, {320, 240, 15}, {320, 240, 10}, {320, 240, 5}, \
  {320, 200, 30}, {320, 200, 25}, {320, 200, 20}, {320, 200, 15}, {320, 200, 10}, {320, 200, 5}, \
  {320, 180, 30}, {320, 180, 25}, {320, 180, 20}, {320, 180, 15}, {320, 180, 10}, {320, 180, 5}, \
  {176, 144, 30}, {176, 144, 25}, {176, 144, 20}, {176, 144, 15}, {176, 144, 10}, {176, 144, 5}, \
  {160, 128, 30}, {160, 128, 25}, {160, 128, 20}, {160, 128, 15}, {160, 128, 10}, {160, 128, 5}, \
  {160, 120, 30}, {160, 120, 25}, {160, 120, 20}, {160, 120, 15}, {160, 120, 10}, {160, 120, 5}, \
  {160, 90, 30}, {160, 90, 25}, {160, 90, 20}, {160, 90, 15}, {160, 90, 10}, {160, 90, 5}, \
  {0,0,0} \
};

SUPPORT_FORMAT MJPEG_bind_format[] = {  \
  {1920, 1080, 0},\
  {1280, 720, 0}, \
  {768, 480, 0}, \
  {640, 480, 0}, \
  {640, 400, 0},\
  {640, 360, 0}, \
  {384, 216, 0},  \
  {352, 288, 0}, \
  {320, 240, 0},  \
  {320, 200, 0}, \
  {320, 180, 0},  \
  {176, 144, 0},  \
  {160, 128, 0},  \
  {160, 120, 0}, \
  {160, 90, 0},  \
  {0,0,0} \
};

SUPPORT_FORMAT MJPEG_format[] = { \
  {1920, 1080, 30},{1920, 1080, 25}, {1920, 1080, 20}, {1920, 1080, 15}, {1920, 1080, 10}, {1920, 1080, 5}, \
  {1280, 720, 30},{1280, 720, 25}, {1280, 720, 20}, {1280, 720, 15}, {1280, 720, 10}, {1280, 720, 5}, \
  {768, 480, 30}, {768, 480, 25}, {768, 480, 20}, {768, 480, 15}, {768, 480, 10}, {768, 480, 5}, \
  {640, 480, 30}, {640, 480, 25}, {640, 480, 20}, {640, 480, 15}, {640, 480, 10}, {640, 480, 5}, \
  {640, 400, 30}, {640, 400, 25}, {640, 400, 20}, {640, 400, 15}, {640, 400, 10}, {640, 400, 5}, \
  {640, 360, 30}, {640, 360, 25}, {640, 360, 20}, {640, 360, 15}, {640, 360, 10}, {640, 360, 5}, \
  {384, 216, 30}, {384, 216, 25}, {384, 216, 20}, {384, 216, 15}, {384, 216, 10}, {384, 216, 5}, \
  {352, 288, 30}, {352, 288, 25}, {352, 288, 20}, {352, 288, 15}, {352, 288, 10}, {352, 288, 5}, \
  {320, 240, 30}, {320, 240, 25}, {320, 240, 20}, {320, 240, 15}, {320, 240, 10}, {320, 240, 5}, \
  {320, 200, 30}, {320, 200, 25}, {320, 200, 20}, {320, 200, 15}, {320, 200, 10}, {320, 200, 5}, \
  {320, 180, 30}, {320, 180, 25}, {320, 180, 20}, {320, 180, 15}, {320, 180, 10}, {320, 180, 5}, \
  {176, 144, 30}, {176, 144, 25}, {176, 144, 20}, {176, 144, 15}, {176, 144, 10}, {176, 144, 5}, \
  {160, 128, 30}, {160, 128, 25}, {160, 128, 20}, {160, 128, 15}, {160, 128, 10}, {160, 128, 5}, \
  {160, 120, 30}, {160, 120, 25}, {160, 120, 20}, {160, 120, 15}, {160, 120, 10}, {160, 120, 5}, \
  {160, 90, 30}, {160, 90, 25}, {160, 90, 20}, {160, 90, 15}, {160, 90, 10}, {160, 90, 5}, \
  {0,0,0} \
};

SUPPORT_FORMAT AVC_format[] = { \
  {1920, 1080, 30},{1920, 1080, 25}, {1920, 1080, 20}, {1920, 1080, 15}, {1920, 1080, 10}, {1920, 1080, 5}, \
  {1280, 720, 30},{1280, 720, 25},{1280, 720, 22}, {1280, 720, 20}, {1280, 720, 15}, {1280, 720, 10}, {1280, 720, 5}, \
  {768, 480, 30}, {768, 480, 25}, {768, 480, 20}, {768, 480, 15}, {768, 480, 10}, {768, 480, 5}, \
  {640, 480, 30}, {640, 480, 25}, {640, 480, 20}, {640, 480, 15}, {640, 480, 10}, {640, 480, 5}, \
  {640, 400, 30}, {640, 400, 25}, {640, 400, 20}, {640, 400, 15}, {640, 400, 10}, {640, 400, 5}, \
  {640, 360, 30}, {640, 360, 25}, {640, 360, 20}, {640, 360, 15}, {640, 360, 10}, {640, 360, 5}, \
  {352, 288, 30}, {352, 288, 25}, {352, 288, 20}, {352, 288, 15}, {352, 288, 10}, {352, 288, 5}, \
  {320, 240, 30}, {320, 240, 25}, {320, 240, 20}, {320, 240, 15}, {320, 240, 10}, {320, 240, 5}, \
  {320, 200, 30}, {320, 200, 25}, {320, 200, 20}, {320, 200, 15}, {320, 200, 10}, {320, 200, 5}, \
  {320, 180, 30}, {320, 180, 25}, {320, 180, 20}, {320, 180, 15}, {320, 180, 10}, {320, 180, 5}, \
  {176, 144, 30}, {176, 144, 25}, {176, 144, 20}, {176, 144, 15}, {176, 144, 10}, {176, 144, 5}, \
  {160, 128, 30}, {160, 128, 25}, {160, 128, 20}, {160, 128, 15}, {160, 128, 10}, {160, 128, 5}, \
   {160, 120, 30}, {160, 120, 25}, {160, 120, 20}, {160, 120, 15}, {160, 120, 10}, {160, 120, 5}, \
  {160, 90, 30}, {160, 90, 25}, {160, 90, 20}, {160, 90, 15}, {160, 90, 10}, {160, 90, 5}, \
  {0,0,0} \
};


vp8_frames_ex_t vp8_frames;
int last_frame_sizes[4];

int config_is_commit = 0; /* easy check whether config is commit */
unsigned int yuyv_data_length, yuyv_bad_frame_count, vp8_h264_bad_frame_count;
unsigned char generate_key_frame;

static qic_module *dev_pt = NULL; /* global device ref pt */

static int qic_clean_device(qic_dev *cam);
int enum_device_formats(int fd, char* format,char print);



#ifdef USE_THREAD
/* mutex related */
#define MUTEX_ON(mutex) pthread_mutex_lock( &mutex )
#define MUTEX_OFF(mutex) pthread_mutex_unlock( &mutex )

pthread_mutex_t crit_mutex = PTHREAD_MUTEX_INITIALIZER; /* loop control mutex */
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond  = PTHREAD_COND_INITIALIZER;

unsigned char pthread_onoff = 0;
unsigned char new_video_on = 0;
unsigned int syscontrol_working=0;
#else
#define MUTEX_ON(mutex)
#define MUTEX_OFF(mutex)
#endif

/* NAL related function and parameter */
#define MAX_NAL_SEG 32

typedef struct {
  unsigned char *address;
  unsigned int length;
} NAL;

typedef struct {
  NAL item[MAX_NAL_SEG]; /* max segment in one frame */
} NAL_LIST;

#ifdef SKYPEAPI

typedef struct
{
  int length;
  char gIslice[128*1024];
  int sps;
}NEWISLICE;


 const char frame_hdr[4][6]={{0x00,0x00,0x00,0x01,0x27,0x42}, //SPS
           {0x00,0x00,0x00,0x01,0x28,0xCE}, //PPS
           {0x00,0x00,0x00,0x01,0x65,0xB8},  //I-slice
           {0x00,0x00,0x00,0x01,0x21,0xE0},  //P-slice
          };

NEWISLICE new_islice;

int framecmp(const char *s1, const char *s2, int n)
{
    int ret=0;
  while(n--)
  {
  //  printf("s1=%x, s2=%x\n",*(s1),*(s2));
    if(*(s1)!=*(s2))
    {
      ret=1;
     break;
    }
    s1++;
    s2++;
  }

  return ret;
}

int qic_compose_frame(char *oldframe, char **newframe, unsigned int old_len)
{
  int new_len=0;
  if(!framecmp(&frame_hdr[3][0],oldframe,6)){
    new_len=old_len;
    *newframe=oldframe;
  }else{

    if(!framecmp(&frame_hdr[0][0],oldframe,6)){
  //    printf("new_islice=%d\n",old_len);
       memset (&new_islice, 0, sizeof (new_islice));
      new_islice.sps=1;
      new_islice.length=old_len;
      memcpy(new_islice.gIslice,oldframe,old_len);
      new_len=0;
    }else if(!framecmp(&frame_hdr[2][0],oldframe,6)&&new_islice.sps){
      new_islice.sps=0;
      memcpy(new_islice.gIslice+new_islice.length,oldframe,old_len);
      new_islice.length=new_islice.length+old_len;
      new_len=new_islice.length;
      *newframe=new_islice.gIslice;
  //    printf("new_islice.length=%d\n",new_len);
    }else{
      printf("unknow format!!\n");
      new_len=old_len;
      *newframe=oldframe;
      }
  }

  return new_len;
}
#endif


void qic_get_proc_devname(qic_dev_name_s *devname ) {
  FILE *fp ;
  char buffer[100],tmp[100] ;
  size_t bytes_read ;
  char *match ;


  fp = fopen("/proc/qicuvc", "r") ;

  if(fp!=NULL){

  bytes_read = fread(buffer, 1, sizeof(buffer), fp) ;
  fclose(fp) ;
  /* Bail if read failed or if buffer isn't big enough */
  if(bytes_read == 0 || bytes_read == sizeof(buffer))
  {
     printf("read file fail");
    return ;
  }
  /* NUL-terminate the text */
  buffer[bytes_read] = '\0' ;
  /* Locate the line that starts with "cpu MHz" */
  match = strstr(buffer, "YUV:") ;
  if(match == NULL)
  {     printf("can not found YUV \r\n");
    return ;
  }
  /* Parse the line to extract the clock speed */


  sscanf(match, "YUV:%s",tmp) ;
  sprintf(devname->dev_yuv,"/dev/%s",tmp);

  match = strstr(buffer, "AVC:") ;
  if(match == NULL)
  {     printf("can not found AVC \r\n");
    return ;
  }
  /* Parse the line to extract the clock speed */

  sscanf(match, "AVC:%s",tmp) ;
  sprintf(devname->dev_avc,"/dev/%s",tmp);

  printf("get device name %s %s \r\n",devname->dev_yuv,devname->dev_avc);

  }
  else
  {
    printf("open /proc/qicuvc error!!\n");
  }

  return ;

}

int qic_show_dev_format(unsigned int dev_id)
{
  int ret=0;
  unsigned int index=0;
  char dev_format[32];

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
        memset (dev_format, 0, sizeof(dev_format));
        ret=enum_device_formats(dev_pt->cam[index].fd,dev_format,1);
      }
    }

  return ret;

}

int qic_enum_device_formats(qic_dev_name_s *name)
{
  int ret=0;
  int fd;
  char found_video0=0;
  char found_video1=0;
  char dev_name[16];
  int i;
  char dev_format[32];


    for(i=0; i<10; i++){

  memset (dev_name, 0, sizeof(dev_name));
  snprintf(dev_name, sizeof(dev_name), "/dev/video%d", i);

  fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if(fd>0){
      QicSetDeviceHandle(fd);
      if(QicQueryDevice()==0){

        memset (dev_format, 0, sizeof(dev_format));
        ret=  enum_device_formats(fd,dev_format,0);

        if(!ret){
          if(found_video0&&(strcmp(dev_format,"MPEG2-TS")==0
              ||strcmp(dev_format,"MJPEG")==0)){
#ifdef ENABLE_PRINT
                printf("AVC is %s \n",dev_name);
#endif
            strcpy(name->dev_avc,dev_name);
            found_video1=1;
          }

          if(!found_video0&&(strcmp(dev_format,"YUV 4:2:2 (YUYV)")==0
                     ||strcmp(dev_format,"MJPEG")==0)){
#ifdef ENABLE_PRINT
                printf("YUV is %s \n",dev_name);
#endif
            strcpy(name->dev_yuv,dev_name);
            found_video0=1;
          }

        }
      }
      else{
        printf("Not supported video device\n");
        //ret =1;
        }

      close(fd);
          }
      else{
#ifdef ENABLE_PRINT
     printf("opne failed i=%d\n",i );
#endif
      }
      }

if(found_video0)
  ret=0;
else
  ret=1;

return ret;


}



static int find_shaing_image(const char *dirpath, char *search, char *imagename)
{

//  char *shaing="_ISP_LSC_";
  char *p=NULL;

  DIR * dir = opendir (dirpath);

  LOG_PRINT(debug_str, DEBUG_INFO,"dirpath=%s, search=%s\n",dirpath,search);

  if (dir == NULL){
    LOG_PRINT(debug_str, DEBUG_ERROR,"error! Unable to open the directory.\n");
    return  1;
  }

  struct dirent *d = readdir (dir);
  while (d != NULL){

    p=strstr(d->d_name,search);

    if(p){
      LOG_PRINT(debug_str, DEBUG_INFO,"\"%s\"\n", d->d_name);
      strcpy(imagename,d->d_name);
      break;
    }

    d = readdir (dir);
  }

  closedir (dir);

  if(!p){
      LOG_PRINT(debug_str, DEBUG_ERROR,"error! sharing iamge no found\n");
    return 1;
  }
  // Now, close the directory.

  return 0;
}

/* function to traverse the stream for "00 00 00 01" and change NRI value */
static void nri_change(unsigned char *base, int length, qic_dev *cam) {

  NAL_LIST nal_list;
  unsigned int index = 0;
  int offset = 0;
  int state = 0;
  static const char fsm[3][5] =
  {
    {1, 2, 3, 3, 0},    // current char is 0
    {0, 0, 0, 4, 0},    // current char is 1
    {0, 0, 0, 0, 0}   // current char is other
  };

  memset (&nal_list, 0, sizeof(nal_list));

  /* FSM test */
  while(offset <= length)
  {
    switch(base[offset++]) {
      case 0:
        state = fsm[0][state];  break;
      case 1:
        state = fsm[1][state];  break;
      default:
        state = fsm[2][state];  break;
    }

    if(state == 4) {

      /* easy for check overflow */
      assert ( index < MAX_NAL_SEG );

      /* reset state */
      state = 0;

      /*printf(".");fflush(stdout);*/
      nal_list.item[index].address = base + offset; /* omit "00 00 00 01" */

      if (index > 0)
        nal_list.item[index-1].length = (unsigned int )(nal_list.item[index].address - nal_list.item[index-1].address - 4);

      index++;

    }
  }

  /*at least include 3 type format (SPS/PPS/I-Slice)*/
     if(index>=3){
  /* back to the last element */
  index--;

  /* caculate the last length */
  if (nal_list.item[0].address != NULL)  /* yes, have item */
    nal_list.item[index].length = (unsigned int)(base + length - nal_list.item[index-1].address - nal_list.item[index-1].length -4) ;

  /*clear next item*/
  nal_list.item[index+1].length = 0;
  nal_list.item[index+1].address = NULL;

  /*change nri according to settings*/
  index = 1;
  while (nal_list.item[index].address != NULL) { /* start check beyond AUD */
    switch(nal_list.item[index].address[0]) {
      case 0x27: /* SPS*/
        nal_list.item[index].address[0] = 0x07 | ((cam->nri_sps & 0x03) << 5);
        break;

      case 0x28: /*PPS*/
        nal_list.item[index].address[0] = 0x08 | ((cam->nri_pps & 0x03) << 5);
        break;

      case 0x25: /*IDR*/
        nal_list.item[index].address[0] = 0x05 | ((cam->nri_iframe& 0x03) << 5);
        break;
    }

    index++;
  }
     }

}


static void debug_print_initial (int level, char *string) {

  printf("level=%d,string=%s\n",level,string);
}

static void frame_output_initial(unsigned int dev_id, unsigned int length, char *data, unsigned long timestamp) {
  printf("dev_id=%d,  length=%d, data=%s,  timestamp=%lu\n",dev_id,  length, data,  timestamp);
}

static int xioctl(int fd, int request, void * arg)
{
  int r;

  do r = ioctl(fd, request, arg);
  while (-1 == r && EINTR == errno);

  return r;
}



 int qic_V4L2_Control(int fd,unsigned long cmd,int Get,signed long *value,signed long invalue)
{

struct v4l2_queryctrl queryctrl;
struct v4l2_control control;
  int qic_ret=0;
      signed long* tempvalue=value;
      unsigned char h_flip2, v_flip2;
       memset (&queryctrl, 0, sizeof (queryctrl));
       queryctrl.id = cmd;

     if((cmd ==V4L2_CID_HFLIP) || (cmd ==V4L2_CID_VFLIP))
    {
           h_flip2 = 0;
           v_flip2 = 0;
            qic_ret = QicGetFlipMode(&v_flip2, &h_flip2);
         if (Get ==1){
                        if(cmd == V4L2_CID_HFLIP){
                               *tempvalue=h_flip2;
                        }
                       else if(cmd== V4L2_CID_VFLIP){
                             *tempvalue=v_flip2;
                        }
        }
          else {

                        if(cmd == V4L2_CID_HFLIP){
                             qic_ret = QicSetFlipMode(v_flip2,invalue);
                        }
                       else if(cmd== V4L2_CID_VFLIP){
                              qic_ret = QicSetFlipMode(invalue,h_flip2);
                        }
        }
       return 0;
    }

      if (-1 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
               LOG_PRINT(debug_str, DEBUG_ERROR, " %u is not supported\n",  queryctrl.id);
                return 1;
      } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
               LOG_PRINT(debug_str, DEBUG_ERROR, " %u is not supported\n",  queryctrl.id);
                return 1;
      } else {
              LOG_PRINT(debug_str,DEBUG_INFO, "%s is supported, %u\n", queryctrl.name,queryctrl.id);
      }

      memset (&control, 0, sizeof (control));
      control.id = cmd;
      if(Get==1) {
            if(0 == xioctl(fd, VIDIOC_G_CTRL, &control)) {
               *tempvalue=control.value;
            }
      }else {

           control.value=invalue;
           if(0 == xioctl(fd, VIDIOC_S_CTRL, &control)) {

              LOG_PRINT(debug_str,DEBUG_INFO, "%s set success ,value=%u\n", queryctrl.name,control.value );
            }
          else
           {
                LOG_PRINT(debug_str, DEBUG_ERROR, " %s set fail,value=%u\n",  queryctrl.name,control.value );
        return 1;
             }
      }


      return 0;
}

static int format_check(SUPPORT_FORMAT* format, unsigned short width, unsigned short height, unsigned char framerate) {

  int i = 0;
  int is_valid = 0;

  while ((format[i].width != 0) && (!is_valid)) {

    LOG_PRINT(debug_str,DEBUG_INFO, "check i[%d] %dx%d\n", i, format[i].width, format[i].height);

    if (framerate == 0) {
      if ((format[i].width == width) && (format[i].height == height)) {
        is_valid = 1;
      }
    }
    else if ((format[i].width == width) && (format[i].height == height) && (format[i].framerate == framerate)) {
      is_valid = 1;
    }

    i++;
  }

  return is_valid;

}


static void set_scheduler(void) {

  struct sched_param param;

  sched_getparam( 0, &param);

  LOG_PRINT(debug_str, DEBUG_INFO, "Process Original Priority = %d\n", param.sched_priority);

  param.sched_priority = sched_get_priority_max(SCHED_RR);

  if( sched_setscheduler( 0, SCHED_RR, &param ) == -1 )  {
    LOG_PRINT(debug_str, DEBUG_ERROR,  "sched_setscheduler error, (%d)%s\n", errno, strerror(errno));
    /*perror("sched_setscheduler");*/
  }

  sched_getparam( 0, &param);
  LOG_PRINT(debug_str, DEBUG_INFO, "Process New Priority = %d\n", param.sched_priority);

}



/**********************************************
 *  utilities functions
**********************************************/
int qic_get_fd_from_devid(unsigned int dev_id){

  unsigned int index;
  int fd = 0;

  for (index = 0; index < dev_pt->num_devices; index++) {

    if (dev_pt->cam[index].dev_id & dev_id)
    {
      fd = dev_pt->cam[index].fd;
      break;
      /*return dev_pt->cam[index].fd;*/
    }
  }

  return !fd ? -1 : fd;

}

char* qic_print_config_param(unsigned int dev_id) {

  static char report[4096];
  unsigned int index;
  int print_offset = 0;
  unsigned int buf_index;

  memset (report, 0, sizeof(report));

  for ( index = 0; index < dev_pt->num_devices; index++) {

    if (dev_pt->cam[index].dev_id & dev_id) { /* setup the report and print */
      /* name */
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\ndevice %s config state:", dev_pt->cam[index].dev_name);

      /* basic parameters */
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t dev_id: 0x%X", dev_pt->cam[index].dev_id);
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t fd: %d", dev_pt->cam[index].fd);
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t format: "FOURCC_FORMAT,FOURCC_ARGS(dev_pt->cam[index].format));
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t resolution : %dx%d", dev_pt->cam[index].width, dev_pt->cam[index].height);

      /* H.264/AVC parameters */
      if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG) { /* is encoding device */
#if defined(QIC_MPEGTS_API)
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t framerate: %dfps", dev_pt->cam[index].framerate);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t bitrate: %dbps", dev_pt->cam[index].bitrate);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t gop: %dframes", dev_pt->cam[index].gop);
        if (dev_pt->cam[index].slicesize == 0)
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t slice size: Auto");
        else
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t slice size: %d*macro block height", dev_pt->cam[index].gop);
        /* demuxer parameters */
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t demuxer: %s", dev_pt->cam[index].is_demux == 1?"On":"Off");
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t number of buffers: %d", DEMUX_BUF_MAX);
#endif
      }
      /* MJPEG parameters */
      else if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG) {
        if (dev_pt->cam[index].is_bind == 0)
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t framerate: %dfps", dev_pt->cam[index].framerate);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t is bind: %s", dev_pt->cam[index].is_bind== 1?"Yes":"No");
        if(dev_pt->cam[index].is_encoding_video==1)
        {
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t frame rate interval: %dfps", dev_pt->cam[index].frame_interval);
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t bitrate: %dbps", dev_pt->cam[index].bitrate);
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t key frame rate interval: %dfps", dev_pt->cam[index].key_frame_interval);
        }
      }
      /* YUV parameters */
      else if (dev_pt->cam[index].format == V4L2_PIX_FMT_YUYV) {
        if (dev_pt->cam[index].is_bind == 0)
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t framerate: %dfps", dev_pt->cam[index].framerate);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t is bind: %s", dev_pt->cam[index].is_bind== 1?"Yes":"No");
      }
      else /* error setting */
        continue;

      /* system parameters */
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t capturing? %s", dev_pt->cam[index].is_on == 1?"Yes":"No");
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t buffers start addr: %d", dev_pt->cam[index].buffer_start_address);
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t number of MMAP buffers: %d", dev_pt->cam[index].num_mmap_buffer);

      for (buf_index = 0; buf_index < dev_pt->cam[index].num_mmap_buffer; buf_index++) {
        if (dev_pt->cam[index].buffers[buf_index].start != NULL)
          print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t\t buffer #%d length %d at %p", \
          buf_index, \
          (unsigned int)dev_pt->cam[index].buffers[buf_index].length, \
          dev_pt->cam[index].buffers[buf_index].start);
      }
    }
  }

  print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n");
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_print_config_param - OUT\n");

  return report;
}


char * qic_print_video_cap_statistics(unsigned int dev_id) {


  static char report[] = "not implement yet\n";
  printf("dev_id=%d\n",dev_id);
  return report;
}

#if defined(QIC_MPEGTS_API)

char* qic_print_demux_statistics(unsigned int dev_id) {

  static char report[2048];
  unsigned int index;
  int print_offset = 0;

  memset (report, 0, sizeof(report));

  for ( index = 0; index < dev_pt->num_devices; index++) {


    if ((dev_pt->cam[index].dev_id & dev_id) &&(dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG)) { /* is AVC device which has demux */
#if defined(QIC1816)
      print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\ndevice %s demux statistics:", dev_pt->cam[index].dev_name);
      if (config_is_commit) { /* demux is initialized */
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t recover bad frames: %s", dev_pt->cam[index].demux_struc.data.adopt_recover==1?"On":"Off");
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t bad frames count: %d", dev_pt->cam[index].demux_struc.data.bad_count);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t drop frames count: %d", dev_pt->cam[index].demux_struc.data.drop_count);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t frames count: %d", dev_pt->cam[index].demux_struc.data.frame_count);
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t frame buffer size: %d", MAXFRAMESIZE);
        /* __not include now__
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t process frames at %p, size: %d, timestamp: %lu", \
          dev_pt->cam[index].demux_struc.data.frame.data, \
          dev_pt->cam[index].demux_struc.data.frame.length, \
          dev_pt->cam[index].demux_struc.data.frame.timestamp);
        */

      }
      else /* no data */
        print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n\t device is not initialized, call qic_config_commit first");
#endif
    }
  }
  print_offset += snprintf(report+print_offset, sizeof(report)-print_offset, "\n");

  return report;

}

#endif


int qic_change_V4L2_FOCUS_ABSOLUTE(unsigned int dev_id, unsigned int Auto,signed long  ABSOLUTE) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        if(Auto)
          ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_FOCUS_AUTO,0,0,ABSOLUTE);
        else
        {
          ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_FOCUS_AUTO,0,0,0);
                ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_FOCUS_ABSOLUTE,0, 0,ABSOLUTE);
        }

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
          if(Auto)
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_FOCUS_ABSOLUTE,1,&value,0);
          else
           qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_FOCUS_AUTO,1,&value,0);

#endif
      }

    }


  /* success */
  return ret;
}


int qic_change_V4L2_BACKLIGHT_COMPENSATION(unsigned int dev_id, signed long  BC) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

             ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_BACKLIGHT_COMPENSATION,0,0,BC);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
        signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_BACKLIGHT_COMPENSATION,1,&value,0);

#endif
      }
  }

  /* success */
  return ret;
}

int qic_change_V4L2_flip(unsigned int dev_id, unsigned char flip) {

  unsigned int index;
  int ret = 0;

  unsigned char h_flip, v_flip;

#ifdef DEBUG_LOG
  int qic_ret;
#endif

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
        h_flip = flip & H_FLIP;
        v_flip = (flip & V_FLIP)>>1;
                               ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_HFLIP,0,0,h_flip);
                               ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_VFLIP,0,0,v_flip);


#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
        h_flip = 0;
        v_flip = 0;
        qic_ret = QicGetFlipMode(&v_flip, &h_flip);
#endif
        LOG_PRINT(debug_str, DEBUG_INFO,  "QicGetFlipMode(%d) H_FLIP=%s, V_FLIP=%s\n", qic_ret,  h_flip?"on":"off", v_flip?"on":"off");


    }

  }

  /* success */
  return ret;

}

int qic_change_V4L2_WHITE_BALANCE(unsigned int dev_id, unsigned int  Auto,signed long WB) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        if(Auto)
                ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_AUTO_WHITE_BALANCE,0,0,WB);
        else
        {
          ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_AUTO_WHITE_BALANCE,0,0,0);  //disable WBT,audo
          ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_WHITE_BALANCE_TEMPERATURE,0,0,WB);
        }

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
        if(Auto)
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_AUTO_WHITE_BALANCE,1,&value,0);
                              else
        qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_WHITE_BALANCE_TEMPERATURE,1,&value,0);
#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_BRIGHTNESS(unsigned int dev_id,signed long BRIGHTNESS) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_BRIGHTNESS,0,0,BRIGHTNESS);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_BRIGHTNESS,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_CONTRAST(unsigned int dev_id,signed long  CONTRAST) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

             ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_CONTRAST,0,0,CONTRAST);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_CONTRAST,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}



int qic_change_V4L2_HUE(unsigned int dev_id, signed long HUE) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_HUE,0,0,HUE);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_HUE,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_SATURATION(unsigned int dev_id, signed long SATURATION) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_SATURATION,0,0,SATURATION);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_SATURATION,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_SHARPNESS(unsigned int dev_id,signed long SHARPNESS) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_SHARPNESS,0,0,SHARPNESS);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_SHARPNESS,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_GAMMA(unsigned int dev_id, signed long GAMMA) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_GAMMA,0,0,GAMMA);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_GAMMA,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}

int qic_change_V4L2_GAIN(unsigned int dev_id, signed long GAIN) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

             ret= qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_GAIN,0,0,GAIN);

#ifdef DEBUG_LOG

        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_GAIN,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}

int qic_change_V4L2_POWER_LINE_FREQUENCY(unsigned int dev_id, signed long PLF) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_POWER_LINE_FREQUENCY,0,0,PLF);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                                 qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_POWER_LINE_FREQUENCY,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_EXPOSURE(unsigned int dev_id, unsigned int Auto, signed long ABSOLUTE) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        if(Auto)
              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_AUTO,0,0,ABSOLUTE);
        else
        {
           ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_AUTO,0,0,1);
            ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_ABSOLUTE,0,0,ABSOLUTE);
        }
#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
                             if(Auto)
              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_AUTO,1,&value,0);
        else
        {
          ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_ABSOLUTE,1,&value,0);
        }
#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_EXPOSURE_AUTO_PRIORITY(unsigned int dev_id, signed long PRIORITY) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_AUTO_PRIORITY,0,0,PRIORITY);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_EXPOSURE_AUTO_PRIORITY,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_ZOOM_ABSOLUTE(unsigned int dev_id, signed long ABSOLUTE) {

         unsigned int index;
         int ret=0;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_ZOOM_ABSOLUTE,0,0,ABSOLUTE);

#ifdef DEBUG_LOG
        /*TIME_DELAY(1);*/
         signed long value=0;
              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_ZOOM_ABSOLUTE,1,&value,0);

#endif

    }

  }

  /* success */
  return ret;
}


int qic_change_V4L2_Pan_Tilt2(unsigned int dev_id,signed short pan_value, signed short tilt_value)
{
  int ret=0;
  signed long pos=0;
  unsigned int index;
#ifdef DEBUG_LOG
    signed long value=0;
#endif

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

/*
     if(10 <tilt_value||-10> tilt_value)
        {
            printf( " Tilt out of range!!\n");
        return -1;
        }

    if(10 <pan_value||-10> pan_value)
        {
            printf( " Pan out of range!!\n");
        return -1;
        }
*/
         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
  /* min:-36000 max:36000 step:3600  def:0 */

   pos=(tilt_value*PTZ_TILT_STEP);
  ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_TILT_ABSOLUTE,0,0,pos);

#ifdef DEBUG_LOG


              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_TILT_ABSOLUTE,1,&value,0);

#endif

     pos=(pan_value*PTZ_TILT_STEP);
  ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_PAN_ABSOLUTE,0,0,pos);

#ifdef DEBUG_LOG

              ret=qic_V4L2_Control(dev_pt->cam[index].fd,V4L2_CID_PAN_ABSOLUTE,1,&value,0);

#endif

      }
    }

    return ret;
}



/**********************************************
 *  threading qic_module functions
**********************************************/
#ifdef USE_THREAD
void* qic_loop_thread(void* ptr)  {

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    exit(0);
  }
#endif


  //init mutex
  pthread_mutex_init (&crit_mutex, NULL);
  pthread_mutex_init (&condition_mutex, NULL);
  pthread_cond_init(&condition_cond, NULL);


  unsigned int index = 0;
  int sel_ret = 0;
  unsigned int dev_id_set;
  int ret;

  pthread_onoff = 1;

  /* loop start while not_end */
  while (pthread_onoff) {

    fd_set fds;
    struct timeval tv;
    int max_fd = 0;

    FD_ZERO(&fds);

#if 0
    MUTEX_ON(crit_mutex);

    /* check for started interface */
    for (index = 0; index < dev_pt->num_devices ; index++) {
      if (dev_pt->cam[index].is_on) {
        FD_SET(dev_pt->cam[index].fd, &fds);
        max_fd = MAX(max_fd, dev_pt->cam[index].fd);
      }
    }

    MUTEX_OFF(crit_mutex);
#endif

    MUTEX_ON(crit_mutex);

    /* LOOP and wait signal if no interface is activated __NOTICE__: not code well, need re-write*/
    while (max_fd == 0) {
      MUTEX_ON(condition_mutex);
      while(new_video_on == 0)
        pthread_cond_wait( &condition_cond, &condition_mutex );
      MUTEX_OFF(condition_mutex);

      /* check again */
      for (index = 0; index < dev_pt->num_devices ; index++) {
        if (dev_pt->cam[index].is_on) {
          FD_SET(dev_pt->cam[index].fd, &fds);
          max_fd = MAX(max_fd, dev_pt->cam[index].fd);
        }
      }

      /* clear wait state */
      if (max_fd == 0)
        new_video_on = 0;

    }

    /*TIME_DELAY(10);*/

    /* run select to check */
    tv.tv_sec = 10;/* Interface Timeout, 4sec */
    tv.tv_usec = 0;


    sel_ret = select(max_fd+ 1, &fds, NULL, NULL, &tv);

    if (-1 == sel_ret) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "select error %d, %s\n", errno, strerror(errno));

      if (EINTR == errno) {
        MUTEX_OFF(crit_mutex);
        continue;
      }

      /* exit for fatal debug */
                        MUTEX_OFF(crit_mutex);
                        return(NULL);
      //exit(1);
    }

    if (0 == sel_ret) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "select timeout - interface not response?\n");
      /* exit for fatal debug */
                        MUTEX_OFF(crit_mutex);
                        return(NULL);
      //exit(1);
    }

    /* check which interface has data */
    dev_id_set = 0;
    for (index = 0; index < dev_pt->num_devices ; index++) {
      if ((dev_pt->cam[index].is_on) &&(FD_ISSET(dev_pt->cam[index].fd, &fds))) {
        dev_id_set += dev_pt->cam[index].dev_id;
      }
    }


    /* call function to get frame */
    ret = qic_getframe(dev_id_set);
    if (ret) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "qic_getframe error\n");
    }

  MUTEX_OFF(crit_mutex);
      while(syscontrol_working==1) usleep (1000);

  }
         return NULL;
  //exit(0);

}


int qic_getframe_loop_stop(void) {

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* stop the fucking thread at the next loop */
  pthread_onoff = 0;

  LOG_PRINT(debug_str, 0, "getframe_loop_thread stopped\n");

  return 0;

}


int qic_getframe_loop_start(void) {

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR,"QIC module library config is not committed\n");
    return 1;
  }
#endif

  /*thread for interface check,receive data, demux to call-back */
  pthread_t qic_thread;
  int p_ret;
#if 1
  pthread_attr_t attr;
  struct sched_param sched;

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr,32*1024);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
  pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);

  pthread_attr_setschedpolicy(&attr,SCHED_RR);
  pthread_attr_getschedparam(&attr,&sched);
  sched.sched_priority=51;
  pthread_attr_setschedparam(&attr,&sched);

  p_ret = pthread_create (&qic_thread, &attr, qic_loop_thread, NULL);
#else
  p_ret = pthread_create (&qic_thread, NULL, qic_loop_thread, NULL);
#endif

#ifdef USE_THREAD
  syscontrol_working=0;
#endif
  if(p_ret) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "getframe_loop_thread start failed, ret=%d\n", p_ret);
    return 1;

  }

  LOG_PRINT(debug_str, DEBUG_INFO, "getframe_loop_thread started\n");

  /* success */
  return 0;

}

#endif

/**********************************************
 *  firmware update __CAUTION__
**********************************************/
int qic_get_image_version_by_filename (char *usb_path_info, version_info_t *version_info) {

        char usb_img_full [512];
        int img_fd = -1;

        int image_size = -1;
        char *usb_img_array = NULL;

        char *endptr;
        int base = 16;
        QicHwVer_t version;
        int usb_image_max_size;
    unsigned int image_max_size;
        int ret;

        if (usb_path_info == NULL) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "no img directory is specified.\n");
                return 1;
        }

        QicGetHWVersion(&version);

        memset (version_info, 0, sizeof(version_info));
        memset(usb_img_full,0,sizeof(usb_img_full));
        if(version==QIC_HWVERSION_QIC1806){
                snprintf(usb_img_full, sizeof(usb_img_full), "%s", usb_path_info);
                usb_image_max_size=QIC1816_FLASH_USB_MAX_SIZE;
       image_max_size=QIC1816_FLASH_MAX_SIZE;
        }
    else if(version==QIC_HWVERSION_QIC1822){
                snprintf(usb_img_full, sizeof(usb_img_full), "%s", usb_path_info);
                usb_image_max_size=QIC1822_FLASH_USB_MAX_SIZE;
       image_max_size=QIC1822_FLASH_MAX_SIZE;
        }
        else{
                snprintf(usb_img_full, sizeof(usb_img_full), "%s", usb_path_info);
                usb_image_max_size=FLASH_USB_MAX_SIZE;
       image_max_size=FLASH_MAX_SIZE;
        }

        /* open file for reading */
        img_fd = open(usb_img_full, O_RDONLY);
        if ( -1 == img_fd) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: (%d)%s\n", usb_img_full, errno, strerror(errno));
                return 1;
        }
        else {
                LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", usb_img_full);
        }

        /* get the file size */
        image_size = lseek(img_fd, 0, SEEK_END);
        if (-1 == image_size) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: (%d)%s\n", usb_img_full, errno, strerror(errno));
                close(img_fd);
                return 1;
        }
        else {
                LOG_PRINT(debug_str, DEBUG_INFO, "file size = %d\n", image_size);
                lseek(img_fd, 0, SEEK_SET);
        }

        /* size check */
        if (image_size > usb_image_max_size) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", image_size, usb_image_max_size);
                close(img_fd);
                return 1;
        }

        /* map the image */
        usb_img_array = mmap (0, image_size, PROT_READ, MAP_SHARED, img_fd, 0);
        if (MAP_FAILED == usb_img_array) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: (%d)%s\n", errno, strerror(errno));
                close(img_fd);
                return 1;
        } else {
                LOG_PRINT(debug_str, DEBUG_INFO, "mmap success\n");
        }

  /* get the h/w version info */
    char hwTemp[5];

        memcpy (hwTemp, usb_img_array + 0x223, sizeof(hwTemp));
     version_info->hw_version= strtol(hwTemp, &endptr, base);

        /* get the f/w version info */
        char szTemp[0x18];
        memcpy (szTemp, usb_img_array + 0x240, sizeof(szTemp));

        char version_str[5];

        /*VID*/
        if (!strncmp(&szTemp[0], "VID", 3))
                strncpy(version_str, &szTemp[3], 5);
        else
                strncpy(version_str, "FFFF\0", 5);

        version_info->vid = strtol(version_str, &endptr, base);


        /*PID*/
        if (!strncmp(&szTemp[8], "PID", 3))
                strncpy(version_str, &szTemp[11], 5);
        else
                strncpy(version_str, "FFFF\0", 5);

        version_info->pid = strtol(version_str, &endptr, base);

        /*REV*/
        if (!strncmp(&szTemp[16], "REV", 3))
                strncpy(version_str, &szTemp[19], 5);
        else
                strncpy(version_str, "FFFF\0", 5);

        version_info->revision= strtol(version_str, &endptr, base);

        /*SVN - only number*/
        memset (version_str, 0, 5);
        ret = QicFlashRead (0x22C, (unsigned char*)version_str, 4,image_max_size);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, ret);
        version_info->svn = atoi(version_str);


        /* unmap the image */
        if (munmap(usb_img_array, image_size) == -1) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "munmap error: (%d)%s\n", errno, strerror(errno));
        } else {
                LOG_PRINT(debug_str, DEBUG_INFO, "munmap success\n");
        }

        close(img_fd);
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_get_image_version - OUT\n");

        return 0;

}


int qic_get_image_version (char *path_info, version_info_t *version_info) {

  char usb_img_full [512];
  int img_fd = -1;

  int image_size = -1;
  char *usb_img_array = NULL;

  char *endptr;
  int base = 16;
  QicHwVer_t version;
  int usb_image_max_size;
  unsigned int image_max_size;

  int ret;

  if (path_info == NULL) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "no img directory is specified.\n");
    return 1;
  }

  QicGetHWVersion(&version);

  memset (version_info, 0, sizeof(version_info));

  if(version==QIC_HWVERSION_QIC1806){
    snprintf(usb_img_full, sizeof(usb_img_full), "%s/%s", path_info, QIC1816_USB_IMAGE_NAME);
    usb_image_max_size=QIC1816_FLASH_USB_MAX_SIZE;
    image_max_size= QIC1816_FLASH_MAX_SIZE;
  }
  else if(version==QIC_HWVERSION_QIC1822){
    snprintf(usb_img_full, sizeof(usb_img_full), "%s/%s", path_info, QIC1822_USB_IMAGE_NAME);
    usb_image_max_size=QIC1822_FLASH_USB_MAX_SIZE;
    image_max_size =QIC1822_FLASH_MAX_SIZE;
  }
  else{
    snprintf(usb_img_full, sizeof(usb_img_full), "%s/%s", path_info, QIC1802_USB_IMAGE_NAME);
    usb_image_max_size=FLASH_USB_MAX_SIZE;
    image_max_size =FLASH_MAX_SIZE;
  }

  /* open file for reading */
  img_fd = open(usb_img_full, O_RDONLY);
  if ( -1 == img_fd) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: (%d)%s\n", usb_img_full, errno, strerror(errno));
    return 1;
  }
  else {
    LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", usb_img_full);
  }

  /* get the file size */
  image_size = lseek(img_fd, 0, SEEK_END);
  if (-1 == image_size) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: (%d)%s\n", usb_img_full, errno, strerror(errno));
    close(img_fd);
    return 1;
  }
  else {
    LOG_PRINT(debug_str, DEBUG_INFO, "file size = %d\n", image_size);
    lseek(img_fd, 0, SEEK_SET);
  }

  /* size check */
  if (image_size > usb_image_max_size) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", image_size, usb_image_max_size);
    close(img_fd);
    return 1;
  }

  /* map the image */
  usb_img_array = mmap (0, image_size, PROT_READ, MAP_SHARED, img_fd, 0);
  if (MAP_FAILED == usb_img_array) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: (%d)%s\n", errno, strerror(errno));
    close(img_fd);
    return 1;
  } else {
    LOG_PRINT(debug_str, DEBUG_INFO, "mmap success\n");
  }

  /* get the f/w version info */

  char szTemp[0x18];
  memcpy (szTemp, usb_img_array + 0x240, sizeof(szTemp));

  char version_str[5];

  /*VID*/
  if (!strncmp(&szTemp[0], "VID", 3))
    strncpy(version_str, &szTemp[3], 5);
  else
    strncpy(version_str, "FFFF\0", 5);

  version_info->vid = strtol(version_str, &endptr, base);


  /*PID*/
  if (!strncmp(&szTemp[8], "PID", 3))
    strncpy(version_str, &szTemp[11], 5);
  else
    strncpy(version_str, "FFFF\0", 5);

  version_info->pid = strtol(version_str, &endptr, base);

  /*REV*/
  if (!strncmp(&szTemp[16], "REV", 3))
    strncpy(version_str, &szTemp[19], 5);
  else
    strncpy(version_str, "FFFF\0", 5);

  version_info->revision= strtol(version_str, &endptr, base);

  /*SVN - only number*/
  memset (version_str, 0, 5);
  ret = QicFlashRead (0x22C, (unsigned char*)version_str, 4,image_max_size);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, ret);
  version_info->svn = atoi(version_str);


  /* unmap the image */
  if (munmap(usb_img_array, image_size) == -1) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "munmap error: (%d)%s\n", errno, strerror(errno));
  } else {
    LOG_PRINT(debug_str, DEBUG_INFO, "munmap success\n");
  }

  close(img_fd);
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_get_image_version - OUT\n");

  return 0;

}


int qic_get_system_version (unsigned int dev_id, version_info_t *version_info) {

  unsigned int index;
  FirmwareVersion_t fm_status;
//  Qic1822HwVer_t *qic1822_version;
  int qic_ret = 0;
  int ret = 0;

  char *endptr;
  int base = 16;

  memset (version_info, 0, sizeof(version_info));

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      /* set fd */
      qic_ret = QicSetDeviceHandle(dev_pt->cam[index].fd);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

      /* get firmware info */
      CLEAR(fm_status);

      qic_ret = QicGetFirmwareVersion(&fm_status);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if (qic_ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't get firmware version info from device %s\n", dev_pt->cam[index].dev_name);
        ret = qic_ret;
        break;
        /*return qic_ret;*/
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "%s: get firmware info from device %s\n", __func__, dev_pt->cam[index].dev_name);
      }

      version_info->pid = strtol(fm_status.szPID, &endptr, base);
      version_info->vid = strtol(fm_status.szVID, &endptr, base);
      version_info->revision = strtol(fm_status.szREV, &endptr, base);

      qic_ret = QicGetSvnVersion(&(version_info->svn));
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if (qic_ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't get svn version info from device %s\n", dev_pt->cam[index].dev_name);
        ret = qic_ret;
        break;
        /*return qic_ret;*/
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "%s: get svn info from device %s\n", __func__, dev_pt->cam[index].dev_name);
      }

      qic_ret = QicGetFwAPIVersion(&(version_info->fw_api_version));
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if (qic_ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't get fw version info from device %s\n", dev_pt->cam[index].dev_name);
        ret = qic_ret;
        break;
        /*return qic_ret;*/
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "%s: get fw version info from device %s\n", __func__, dev_pt->cam[index].dev_name);
      }

/*
            qic_ret =QicGet1822HWVersion(&qic1822_version);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if (qic_ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't get qic1822 chip version from device %s\n", dev_pt->cam[index].dev_name);
        ret = qic_ret;
        break;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "%s: get qic1822 chip version info from device %s\n", __func__, dev_pt->cam[index].dev_name);
      }
*/



      ret = 0;
      /*return 0;*/

    }
  }
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_get_system_version - OUT\n");

  return ret;
  /*return 0;*/

}

int qic_is_video_streaming(void){

int ret=0; //no streaming
unsigned int encoder_stream,preview_stream;

       QicMmioRead( HSA_ADDRESS, &encoder_stream);encoder_stream &= 0x0001;
       QicMmioRead( VSA_ADDRESS, &preview_stream);preview_stream &= 0x0001;

  printf("encoder stream=%d, preview stream=%dn",encoder_stream,preview_stream);

  if(encoder_stream||preview_stream)
    ret=1;

return ret;

}

static  unsigned long CalculateCKSum(int image_max_size)
{
        int Section[] = { 0x00000, 0x3C000, 0x3D000, 0x3000 };//bypass UVC setting section
        unsigned long Sum = 0;
        int ret=0;
        unsigned char pTemp[0x40000];
        int i,idx;

        for ( i=0; i<4; i+=2)
        {
               ret=QicFlashRead( Section[ i ], pTemp, Section[ i + 1 ],image_max_size );

                for(  idx = 0; idx < Section[ i + 1 ]; idx++ )
                Sum += (unsigned int)pTemp[ idx ];
        }

  if(ret)Sum=0;


   //    printf( "SUM=%u\n", Sum );

        return Sum;
}

static int qic_dump_flash_firmware (char *pathinfo, char *dump_image,  int dump_image_size ) {

  char cur_path[512];
  char *default_path = NULL;
  int cur_fd = -1;
  int ret = 0;
  //    struct tm *tblock;
    time_t curtime;
    curtime = time(NULL);

//    tblock = localtime(&curtime);
//    printf("Local time is: %s/n",asctime(tblock));

  /* set the backup folder */
  if (pathinfo == NULL) {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: no default path, do not backup\n", __func__);
    return 0; /* just no backup... */

  }
  else {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: use path %s for backup\n", __func__, pathinfo);
    default_path= pathinfo;
  }

  /* dump flash image */
  if (dump_image != NULL) {
      snprintf(cur_path, sizeof(cur_path), "%s/%ld%s", default_path,curtime, QIC_FLASH_DUMP_IMAGE_NAME);

    LOG_PRINT(debug_str, DEBUG_INFO, "dump flash image to %s\n", cur_path);

    cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (cur_fd != -1) {
      if ( -1 == write (cur_fd, dump_image, dump_image_size)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't write FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == fsync(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == close(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't close FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      cur_fd = -1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the FLASH image block : %s\n", strerror(errno));
      ret = 1;
    }
  }



  return ret;

}

int qic_check_firmware(unsigned int dev_id,unsigned long checksum)
{

  unsigned long sum=0;
   char* current_path=".";
  int ret = 0;
   int image_max_size;
  QicHwVer_t version;
  unsigned char *BufVerify=NULL;


     /* read data from cam */
     unsigned int index;
     int count = 0;
     for (index = 0; index < dev_pt->num_devices; index++) {
       if (dev_pt->cam[index].dev_id & dev_id) {
         count ++;
         QicChangeFD(dev_pt->cam[index].fd);
       }
     }

     QicGetHWVersion(&version);

     if(version==QIC_HWVERSION_QIC1806){
           image_max_size=QIC1816_FLASH_MAX_SIZE;
     }
     else if(version==QIC_HWVERSION_QIC1822){
           image_max_size=QIC1822_FLASH_MAX_SIZE;
     }
     else{
          image_max_size=FLASH_MAX_SIZE;
     }

          ret = QicSetPll();
     ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);

               sum=CalculateCKSum(image_max_size);
                      if(checksum==sum){
                               printf("\nchecksum verify...ok    (%lu=%lu)\n",sum,checksum);
                      }else{
                               printf("\nchecksum verify...failed    (%lu!=%lu)\n",sum,checksum);
                              printf("dump error image in current folder...\n");

        BufVerify = calloc (1, image_max_size);

        if(version==QIC_HWVERSION_QIC1806)
           ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
        else if(version==QIC_HWVERSION_QIC1822){
                                  ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
        }else
               ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);

                              if(!qic_dump_flash_firmware(current_path, (char*)BufVerify,image_max_size ))
            printf("dump error image success\n");
        else
          printf("dump error image error\n");

                             ret=1;


                        }


        if(BufVerify!=NULL)free (BufVerify);


     return ret;

}


int qic_check_firmware_by_file(unsigned int dev_id,char *check_img_loc)
{

  unsigned long sum=0;
   char* current_path=".";
  int ret = 0;
   int image_max_size;
  QicHwVer_t version;
  unsigned char *BufVerify=NULL;
        int check_img_fd = -1;
  int check_img_size = 0;
  int i=0;
  int firmware_check_fail=0;
  unsigned char *readed_check_image=NULL;

           if (check_img_loc == NULL) {
        printf("No bin file be found!!");
      return 1;
    }

     /* read data from cam */
     unsigned int index;
     int count = 0;
     for (index = 0; index < dev_pt->num_devices; index++) {
       if (dev_pt->cam[index].dev_id & dev_id) {
         count ++;
         QicChangeFD(dev_pt->cam[index].fd);
       }
     }

     QicGetHWVersion(&version);

     if(version==QIC_HWVERSION_QIC1806){
      image_max_size=QIC1816_FLASH_MAX_SIZE;
     }
     else if(version==QIC_HWVERSION_QIC1822){
      image_max_size=QIC1822_FLASH_MAX_SIZE;
     }
     else{
             image_max_size=FLASH_MAX_SIZE;
     }


          ret = QicSetPll();
     ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);


    /* open file for reading */
     check_img_fd = open(check_img_loc, O_RDONLY);

      if ( -1 == check_img_fd) {
       printf("\ncan't open file %s for reading f/w info: %s\n", check_img_loc, strerror(errno));
       return 1;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", check_img_loc);

        /* get the file size */
        check_img_size = lseek(check_img_fd, 0, SEEK_END);

        if (-1 == check_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", check_img_loc, strerror(errno));
          close(check_img_fd);
          ret = 1;
        }
        else {
          lseek(check_img_fd, 0, SEEK_SET);

          /* size check */
          if (check_img_size > image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", check_img_size, image_max_size);
            close(check_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", check_img_size);

            /* map the image */
            readed_check_image= mmap (0, check_img_size, PROT_READ, MAP_SHARED, check_img_fd, 0);
            if (MAP_FAILED == readed_check_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              munmap(readed_check_image, check_img_size);
              ret = 1;
            }
          }
        }
      }

    if(ret){
      printf( "\nRead check bin file failed\n");
             close(check_img_fd);
       return 1;
    }


        BufVerify = calloc (1, image_max_size);

        if(version==QIC_HWVERSION_QIC1806)
           ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
        else if(version==QIC_HWVERSION_QIC1822){
                                  ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
        }else
               ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);


                      LOG_PRINT(debug_str, DEBUG_INFO, "Verify USB Sector Image\n");
      for (i = 0; i < image_max_size; i++)
      {
        if (BufVerify[i] != readed_check_image[i])
        {
          ret = 1;
          firmware_check_fail=1;
          printf("Image Verify Failed at %d, file is %x, camera is %x\n", i, readed_check_image[i], BufVerify[i]);
        }

        //LOG_PRINT(debug_str, DEBUG_ERROR, "Image Verify OK at %d, %x, %x\n", i, readed_check_image[i], BufVerify[i]);
      }


        if(!firmware_check_fail){
           printf("\nfirmware  verify...ok  \n");
        }else{
           printf("\nfirmware verify...failed\n");
          printf("dump error image in current folder...\n");

             if(!qic_dump_flash_firmware(current_path, (char*)BufVerify,image_max_size ))
              printf("dump error image success\n");
          else
            printf("dump error image error\n");

                ret=1;
       }


          if(BufVerify!=NULL)free (BufVerify);

          if (readed_check_image != NULL) {
    munmap (readed_check_image, check_img_size);
    close(check_img_fd);
    }


     return ret;

}

#ifdef QIC_SUPPORT_2ND_BL
static int qic_dump_2nd_BootRom (char *pathinfo, char *dump_image,  int dump_image_size ) {

  char cur_path[512];
  char *default_path = NULL;
  int cur_fd = -1;
  int ret = 0;
  //    struct tm *tblock;
//    time_t curtime;
//    curtime = time(NULL);

//    tblock = localtime(&curtime);
//    printf("Local time is: %s/n",asctime(tblock));

  /* set the backup folder */
  if (pathinfo == NULL) {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: no default path, do not backup\n", __func__);
    return 0; /* just no backup... */

  }
  else {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: use file path %s for backup\n", __func__, pathinfo);
    default_path= pathinfo;
  }

  /* dump flash image */
  if (dump_image != NULL) {
      //snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1822_2nd_BOOTROM_IMAGE_NAME);
      snprintf(cur_path, sizeof(cur_path), "%s", default_path);

    LOG_PRINT(debug_str, DEBUG_INFO, "dump flash image to %s\n", cur_path);

    cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (cur_fd != -1) {
      if ( -1 == write (cur_fd, dump_image, dump_image_size)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't write FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == fsync(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == close(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't close FLASH image: %s\n", strerror(errno));
        ret = 1;
      }

      cur_fd = -1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the FLASH image block : %s\n", strerror(errno));
      ret = 1;
    }
  }



  return ret;

}




int qic_backup_2nd_Boot_Rom(unsigned int dev_id, char *backup_BL2nd_img_loc)
{

  //unsigned char* current_path=".";
  int ret = 0;
   int image_max_size;
  QicHwVer_t version;
  unsigned char *BufVerify=NULL;
  /* read image file */
  //char image_name[512];
  int i=0;
  int firmware_check_fail=0;


    /* set the backup folder */
  if (backup_BL2nd_img_loc == NULL) {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: no backup file path, do not backup\n", __func__);
    return 2; /* just no backup... */

  }

     /* read data from cam */
     unsigned int index;
     int count = 0;
     for (index = 0; index < dev_pt->num_devices; index++) {
       if (dev_pt->cam[index].dev_id & dev_id) {
         count ++;
         QicChangeFD(dev_pt->cam[index].fd);
       }
     }

     QicGetHWVersion(&version);

      image_max_size=QIC1822_FLASH_2ndBL_MAX_SIZE;



          ret = QicSetPll();
     ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);



        BufVerify = calloc (1, image_max_size);

                     ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, BufVerify, image_max_size,image_max_size);


         if(!qic_dump_2nd_BootRom(backup_BL2nd_img_loc, (char*)BufVerify,image_max_size )){
              printf("dump  2nd Boot Rom image success\n");
          }else{
            printf("dump  2nd Boot Rom image error\n");
                 ret=1;
          }

          if(BufVerify!=NULL)free (BufVerify);



     return ret;

}


int qic_update_2nd_bootrom_by_filename(unsigned int dev_id,char eraseAll,  char *update_BL2nd_img_loc) {


  unsigned char *orig_BL2nd_image; /* read from system */
  unsigned char *new_BL2nd_image; /*read from file */
  int ret = 0;
  int BL2nd_img_fd = -1;
  int BL2nd_img_size = 0;
        unsigned char img_header[QIC_IMG_HEADER_SIZE];
  unsigned char header_verify[QIC_IMG_HEADER_SIZE];
        unsigned char BL2nd_fail=0;
  //QicHwVer_t version;
  int BL2nd_image_max_size;
  int image_max_size;
  unsigned char *BufVerify=NULL;
  /* read data from cam */
  unsigned int index;
  int count = 0;
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      count ++;
      QicChangeFD(dev_pt->cam[index].fd);
    }
  }

  BL2nd_image_max_size=QIC1822_FLASH_2ndBL_MAX_SIZE;
  image_max_size=QIC1822_FLASH_MAX_SIZE;

  orig_BL2nd_image = calloc( 1, BL2nd_image_max_size);

  new_BL2nd_image = NULL;


  if ((orig_BL2nd_image == NULL) )  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't malloc\n", __func__);
    goto ERR_CLOSE;
  }


  ret = QicSetPll();
  ret = QicFlashSetSpiConfig(0x08, MODE_FLASH_PROGRAM_PP);

    /* read to ram */
  //    ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, orig_BL2nd_image, BL2nd_image_max_size,image_max_size);

      new_BL2nd_image = orig_BL2nd_image;
      BL2nd_img_size = BL2nd_image_max_size;


    /* read image file */
    char image_name[512];

                if (update_BL2nd_img_loc == NULL) {
      char tmp[10];
                        update_BL2nd_img_loc=tmp;
                        strcpy(update_BL2nd_img_loc,".");
    }

    /* open file for reading */
    if (1) {  /* 2nd Boot Rom image */

                        snprintf(image_name, sizeof(image_name), "%s", update_BL2nd_img_loc);

      BL2nd_img_fd = open(image_name, O_RDONLY);

      if ( -1 == BL2nd_img_fd) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
      new_BL2nd_image = orig_BL2nd_image;
      BL2nd_img_size = BL2nd_image_max_size;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

        /* get the file size */
        BL2nd_img_size = lseek(BL2nd_img_fd, 0, SEEK_END);

        if (-1 == BL2nd_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
          close(BL2nd_img_fd);
          ret = 1;
        }
        else {
          lseek(BL2nd_img_fd, 0, SEEK_SET);

          /* size check */
          if (BL2nd_img_size > BL2nd_image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", BL2nd_img_size, BL2nd_image_max_size);
            close(BL2nd_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", BL2nd_img_size);

            /* map the image */
            new_BL2nd_image= mmap (0, BL2nd_img_size, PROT_READ, MAP_SHARED, BL2nd_img_fd, 0);
            if (MAP_FAILED == new_BL2nd_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              close(BL2nd_img_fd);
              ret = 1;
            }
          }
        }
      }

    }
    else {
      new_BL2nd_image = orig_BL2nd_image;
      BL2nd_img_size =BL2nd_image_max_size;
    }

    if (ret) {
      goto ERR_CLOSE;
    }


    /* erase 2nd BL flash */
    int i;
    BufVerify = calloc (1, image_max_size);

    printf("\nFlash erasing...\n");

  /* erase */
/*

*/
  if(eraseAll){
           BufVerify = calloc (1, image_max_size);

  QicFlashErase ();
        printf("\nFlash erase all ...\n");
   usleep(2000*1000);
    /* check if erase complete */
    ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, BufVerify, image_max_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "Check Flash 2nd Boot Rom sector erase ret=%d\n",ret);


  for (i = 0; i < image_max_size; i++)
    {
      if (BufVerify[i] != 0xff)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "Flash 2nd Boot Rom erase error at sector %x %x\n", i,BufVerify[i] );
      }
    }


  }else{
    BufVerify = calloc (1, QIC1822_FLASH_2ndBL_MAX_SIZE);

    printf("\nFlash erasing ...\n");
      for (i = 0; i < QIC1822_FLASH_2ndBL_MAX_SIZE; i += FLASH_SECTOR_SIZE) {
        ret = QicFlashSectorErase(i);
        printf("\r [size:%d/total:%d]",i,image_max_size);
        usleep(50*1000);

      }
    /* check if erase complete */
    ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, BufVerify, QIC1822_FLASH_2ndBL_MAX_SIZE,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "Check Flash 2nd Boot Rom sector erase ret=%d\n",ret);


  for (i = 0; i < QIC1822_FLASH_2ndBL_MAX_SIZE; i++)
    {
      if (BufVerify[i] != 0xff)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "Flash 2nd Boot Rom erase error at sector %x %x\n", i,BufVerify[i] );
      }
    }

  }

    free(BufVerify);

    if (ret) {

      goto ERR_CLOSE_MMAP;
    }

    /* write to flash */
    if (1) {
                  printf("\n 2nd Boot Rom image flashing... \n");
      memcpy(&img_header[0],&new_BL2nd_image[0],QIC_IMG_HEADER_SIZE);

    ret = QicFlashWriteUSBIMG (QIC1822_FLASH_2ndBL_ADDR+QIC_IMG_HEADER_SIZE, new_BL2nd_image+QIC_IMG_HEADER_SIZE, BL2nd_img_size-QIC_IMG_HEADER_SIZE,image_max_size);
      //QicFlashWriteUSBIMG
    LOG_PRINT(debug_str, DEBUG_INFO, "2nd Boot Rom Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "2nd Boot Rom Sector Image Write Failed\n");
    }
        printf("2nd Boot Rom image flashing...done \n");
      }

    /*verify*/
    if (1) {
      BufVerify = calloc (1, BL2nd_img_size);
      ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, BufVerify, BL2nd_img_size,image_max_size);

      LOG_PRINT(debug_str, DEBUG_INFO, "Verify 2nd boot rom Sector Image ret=%d\n", ret);

      for (i = 0+QIC_IMG_HEADER_SIZE; i < BL2nd_img_size-QIC_IMG_HEADER_SIZE; i++)
      {
        if (BufVerify[i] != new_BL2nd_image[i])
        {
          ret = 1;
          BL2nd_fail=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "2nd boot rom Sector Image Verify Failed at %d, %x, %x\n", i, new_BL2nd_image[i], BufVerify[i]);
        }
      }

        free (BufVerify);

      if(BL2nd_fail){
          printf("\BL2nd Flash ERROR!!!!!!!!!!!!!\n");
    }


    }

      if(!BL2nd_fail){

        ret = QicFlashWrite(QIC1822_FLASH_2ndBL_ADDR, img_header, QIC_IMG_HEADER_SIZE, image_max_size);

          /*checking header*/
        ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, header_verify, QIC_IMG_HEADER_SIZE,image_max_size);

        if (header_verify[0] != 0x18&&header_verify[1] != 0x22){
          printf("QIC1822 header flash error\n");
          ret= 1;
        }

         printf("\ndownload 2nd Boot Rom ok\n");


                }else{
        printf("\ndownload 2nd Boot Rom error\n");
        ret=1;


                }


    if (ret) {

      goto ERR_CLOSE_MMAP;
    }


  /*release mem */
ERR_CLOSE_MMAP:


  if (new_BL2nd_image != orig_BL2nd_image) {
    munmap(new_BL2nd_image, BL2nd_img_size);
    close(BL2nd_img_fd);
  }


ERR_CLOSE:
  free(orig_BL2nd_image);


  return ret;

}



#endif

int qic_update_osd_font_by_filename(unsigned int dev_id, char *update_osd_font_img_loc) {


  unsigned char *orig_osd_font_image; /* read from system */
  unsigned char *new_osd_font_image; /*read from file */
  int ret = 0;
  int osd_font_img_fd = -1;
  int osd_font_img_size = 0;
        unsigned char audio_fail=0;
  //QicHwVer_t version;
  int osd_font_image_max_size;
  int image_max_size;
  unsigned char *BufVerify=NULL;
  /* read data from cam */
  unsigned int index;
  int count = 0;
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      count ++;
      QicChangeFD(dev_pt->cam[index].fd);
    }
  }

  osd_font_image_max_size=QIC1822_FLASH_OSD_FONT_MAX_SIZE;
  image_max_size=QIC1822_FLASH_MAX_SIZE;

  orig_osd_font_image = calloc( 1, osd_font_image_max_size);

  new_osd_font_image = NULL;


  if ((orig_osd_font_image == NULL) ) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't malloc\n", __func__);
    goto ERR_CLOSE;
  }


  ret = QicSetPll();
  ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);

    /* read to ram */
  ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, orig_osd_font_image, osd_font_image_max_size,image_max_size);

      new_osd_font_image = orig_osd_font_image;
      osd_font_img_size = osd_font_image_max_size;


    /* read image file */
    char image_name[512];

                if (update_osd_font_img_loc == NULL) {
      char tmp[10];
                        update_osd_font_img_loc=tmp;
                        strcpy(update_osd_font_img_loc,".");
    }

    /* open file for reading */
    if (1) {  /* osd font image */

                        snprintf(image_name, sizeof(image_name), "%s", update_osd_font_img_loc);

      osd_font_img_fd = open(image_name, O_RDONLY);

      if ( -1 == osd_font_img_fd) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
      new_osd_font_image = orig_osd_font_image;
      osd_font_img_size = osd_font_image_max_size;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

        /* get the file size */
        osd_font_img_size = lseek(osd_font_img_fd, 0, SEEK_END);

        if (-1 == osd_font_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
          close(osd_font_img_fd);
          ret = 1;
        }
        else {
          lseek(osd_font_img_fd, 0, SEEK_SET);

          /* size check */
          if (osd_font_img_size > osd_font_image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", osd_font_img_size, osd_font_image_max_size);
            close(osd_font_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", osd_font_img_size);

            /* map the image */
            new_osd_font_image= mmap (0, osd_font_img_size, PROT_READ, MAP_SHARED, osd_font_img_fd, 0);
            if (MAP_FAILED == new_osd_font_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              close(osd_font_img_fd);
              ret = 1;
            }
          }
        }
      }

    }
    else {
      new_osd_font_image = orig_osd_font_image;
      osd_font_img_size =osd_font_image_max_size;
    }


    if (ret) {
      goto ERR_CLOSE;
    }


    /* erase osd font flash */
    int i;
    BufVerify = calloc (1, osd_font_image_max_size);

    printf("\nFlash erasing...\n");

  /* erase */
        for (i = QIC1822_FLASH_OSD_FONT_ADDR; i < QIC1822_FLASH_OSD_FONT_ADDR+osd_font_image_max_size; i += FLASH_SECTOR_SIZE) {
        ret = QicFlashSectorErase(i);
        printf("\r [size:%d/total:%d]",i,osd_font_image_max_size);
        usleep(50*1000);

      }
     usleep(100*1000);


    /* check if erase complete */
    ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, BufVerify, osd_font_image_max_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "\nCheck Flash osd font sector erase ret=%d\n",ret);


  for (i = 0; i < osd_font_image_max_size; i++)
    {
      if (BufVerify[i] != 0xff)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "Flash osd font erase error at sector %x %x\n", i,BufVerify[i] );
      }
    }

    free(BufVerify);

    if (ret) {

      goto ERR_CLOSE_MMAP;
    }

    /* write to flash */
    if (1) {
                  printf("\n osd font image flashing... \n");

    ret = QicFlashWriteUSBIMG (QIC1822_FLASH_OSD_FONT_ADDR, new_osd_font_image, osd_font_img_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_INFO, "osd font Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "osd font Sector Image Write Failed\n");
    }
        printf("osd font image flashing...done \n");
      }

    /*verify*/
    if (1) {
      BufVerify = calloc (1, osd_font_img_size);
      ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, BufVerify,osd_font_img_size,image_max_size);

      LOG_PRINT(debug_str, DEBUG_INFO, "Verify osd font Sector Image ret=%d\n", ret);

      for (i = 0; i < osd_font_img_size; i++)
      {
        if (BufVerify[i] != new_osd_font_image[i])
        {
          ret = 1;
          audio_fail=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "osd font Sector Image Verify Failed at %d, %x, %x\n", i, new_osd_font_image[i], BufVerify[i]);
        }
      }

        free (BufVerify);

      if(audio_fail){
          printf("\nosd font Flash ERROR!!!!!!!!!!!!!\n");
    }


    }

      if(!audio_fail){
         printf("\ndownload osd font ok\n");


                }else{
        printf("\ndownload osd font error\n");
        ret=1;


                }


    if (ret) {

      goto ERR_CLOSE_MMAP;
    }


  /*release mem */
ERR_CLOSE_MMAP:


  if (new_osd_font_image != orig_osd_font_image) {
    munmap(new_osd_font_image, osd_font_img_size);
    close(osd_font_img_fd);
  }


ERR_CLOSE:
  free(orig_osd_font_image);


  return ret;

}



int qic_update_audio_data_by_filename(unsigned int dev_id, char *update_audio_img_loc) {


  unsigned char *orig_audio_image; /* read from system */
  unsigned char *new_audio_image; /*read from file */
  int ret = 0;
  int audio_img_fd = -1;
  int audio_img_size = 0;
        unsigned char audio_fail=0;
  //QicHwVer_t version;
  int audio_image_max_size;
  int image_max_size;
  unsigned char *BufVerify=NULL;
  /* read data from cam */
  unsigned int index;
  int count = 0;
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      count ++;
      QicChangeFD(dev_pt->cam[index].fd);
    }
  }

  audio_image_max_size=QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE;
  image_max_size=QIC1822_FLASH_MAX_SIZE;

  orig_audio_image = calloc( 1, audio_image_max_size);

  new_audio_image = NULL;


  if ((orig_audio_image == NULL) )  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't malloc\n", __func__);
    goto ERR_CLOSE;
  }


  ret = QicSetPll();
  ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);

    /* read to ram */
  ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, orig_audio_image, audio_image_max_size,image_max_size);

      new_audio_image = orig_audio_image;
      audio_img_size = audio_image_max_size;


    /* read image file */
    char image_name[512];

                if (update_audio_img_loc == NULL) {
      char tmp[10];
                        update_audio_img_loc=tmp;
                        strcpy(update_audio_img_loc,".");
    }

    /* open file for reading */
    if (1) {  /* audio image */

                        snprintf(image_name, sizeof(image_name), "%s", update_audio_img_loc);

      audio_img_fd = open(image_name, O_RDONLY);

      if ( -1 == audio_img_fd) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
      new_audio_image = orig_audio_image;
      audio_img_size = audio_image_max_size;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

        /* get the file size */
        audio_img_size = lseek(audio_img_fd, 0, SEEK_END);

        if (-1 == audio_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
          close(audio_img_fd);
          ret = 1;
        }
        else {
          lseek(audio_img_fd, 0, SEEK_SET);

          /* size check */
          if (audio_img_size > audio_image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", audio_img_size, audio_image_max_size);
            close(audio_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", audio_img_size);

            /* map the image */
            new_audio_image= mmap (0, audio_img_size, PROT_READ, MAP_SHARED, audio_img_fd, 0);
            if (MAP_FAILED == new_audio_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              close(audio_img_fd);
              ret = 1;
            }
          }
        }
      }

    }
    else {
      new_audio_image = orig_audio_image;
      audio_img_size =audio_image_max_size;
    }


    if (ret) {
      goto ERR_CLOSE;
    }


    /* erase audio flash */
    int i;
    BufVerify = calloc (1, audio_image_max_size);

    printf("\nFlash erasing...\n");

  /* erase */
    ret = QicFlashSectorErase(QIC1822_FLASH_AUDIO_PARAM_ADDR);
     usleep(100*1000);


    /* check if erase complete */
    ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, BufVerify, audio_image_max_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "Check Flash audio sector erase ret=%d\n",ret);


  for (i = 0; i < audio_image_max_size; i++)
    {
      if (BufVerify[i] != 0xff)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "Flash audio erase error at sector %x %x\n", i,BufVerify[i] );
      }
    }

    free(BufVerify);

    if (ret) {

      goto ERR_CLOSE_MMAP;
    }

    /* write to flash */
    if (1) {
                  printf("\n audio image flashing... \n");

    ret = QicFlashWriteUSBIMG (QIC1822_FLASH_AUDIO_PARAM_ADDR, new_audio_image, audio_img_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_INFO, "audio Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "audio Sector Image Write Failed\n");
    }
        printf("audio image flashing...done \n");
      }

    /*verify*/
    if (1) {
      BufVerify = calloc (1, audio_img_size);
      ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, BufVerify,audio_img_size,image_max_size);

      LOG_PRINT(debug_str, DEBUG_INFO, "Verify audio Sector Image ret=%d\n", ret);

      for (i = 0; i < audio_img_size; i++)
      {
        if (BufVerify[i] != new_audio_image[i])
        {
          ret = 1;
          audio_fail=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "audio Sector Image Verify Failed at %d, %x, %x\n", i, new_audio_image[i], BufVerify[i]);
        }
      }

        free (BufVerify);

      if(audio_fail){
          printf("\audio Flash ERROR!!!!!!!!!!!!!\n");
    }


    }

      if(!audio_fail){
         printf("\ndownload audio ok\n");


                }else{
        printf("\ndownload audio error\n");
        ret=1;


                }


    if (ret) {

      goto ERR_CLOSE_MMAP;
    }


  /*release mem */
ERR_CLOSE_MMAP:


  if (new_audio_image != orig_audio_image) {
    munmap(new_audio_image, audio_img_size);
    close(audio_img_fd);
  }


ERR_CLOSE:
  free(orig_audio_image);


  return ret;

}


int qic_dump_all_flash(unsigned int dev_id)
{

   char* current_path=".";
  int ret = 0;
   int image_max_size;
  QicHwVer_t version;
  unsigned char *BufVerify=NULL;

  int i=0;
  int firmware_check_fail=0;


     /* read data from cam */
     unsigned int index;
     int count = 0;
     for (index = 0; index < dev_pt->num_devices; index++) {
       if (dev_pt->cam[index].dev_id & dev_id) {
         count ++;
         QicChangeFD(dev_pt->cam[index].fd);
       }
     }

     QicGetHWVersion(&version);

     if(version==QIC_HWVERSION_QIC1806){
      image_max_size=QIC1816_FLASH_MAX_SIZE;
     }
     else if(version==QIC_HWVERSION_QIC1822){
      image_max_size=QIC1822_FLASH_MAX_SIZE;
     }
     else{
             image_max_size=FLASH_MAX_SIZE;
     }


          ret = QicSetPll();
     ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);



        BufVerify = calloc (1, image_max_size);

        if(version==QIC_HWVERSION_QIC1806)
           ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
        else if(version==QIC_HWVERSION_QIC1822){
#ifdef QIC_SUPPORT_2ND_BL
                     ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, BufVerify, image_max_size,image_max_size);
#else
                     ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
#endif
        }else
               ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);



         if(!qic_dump_flash_firmware(current_path, (char*)BufVerify,image_max_size )){
              printf("dump  image success\n");
          }else{
            printf("dump  image error\n");
                 ret=1;
          }

          if(BufVerify!=NULL)free (BufVerify);



     return ret;

}



/* internal usage */
static int qic_backup_firmware (char *pathinfo, char *usb_image, char *isp_image, char *parm_image, char *audio_image, char *osd_font_image,QicHwVer_t version) {

  char cur_path[512];
  char *default_path = NULL;
  int cur_fd = -1;
  int ret = 0;
  //QicHwVer_t version;
  int usb_image_max_size;
  int isp_image_max_size;
  int parm_image_max_size;
  int audio_image_max_size;
  int osd_font_image_max_size;
  /* set the backup folder */
  if (pathinfo == NULL) {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: no default path, do not backup\n", __func__);
    return 0; /* just no backup... */

  }
  else {
    LOG_PRINT(debug_str, DEBUG_INFO, "%s: use path %s for backup\n", __func__, pathinfo);
    default_path= pathinfo;
  }


  //QicGetHWVersion(&version);

  /* backup usb image */
  if (usb_image != NULL) {

    if(version==QIC_HWVERSION_QIC1806){
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1816_USB_IMAGE_NAME);
      usb_image_max_size=QIC1816_FLASH_USB_MAX_SIZE;
    }
  else if(version==QIC_HWVERSION_QIC1822){
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1822_USB_IMAGE_NAME);
      usb_image_max_size=QIC1822_FLASH_USB_MAX_SIZE;
    }
    else{
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1802_USB_IMAGE_NAME);
      usb_image_max_size=FLASH_USB_MAX_SIZE;
    }
    LOG_PRINT(debug_str, DEBUG_INFO, "backup USB image to %s\n", cur_path);

    cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (cur_fd != -1) {
      if ( -1 == write (cur_fd, usb_image, usb_image_max_size)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't write USB image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == fsync(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync USB image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == close(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't close USB image: %s\n", strerror(errno));
        ret = 1;
      }

      cur_fd = -1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the USB image block : %s\n", strerror(errno));
      ret = 1;
    }
  }

  if(version==QIC_HWVERSION_QIC1806)
  {
    LOG_PRINT(debug_str, DEBUG_INFO,"QIC1816/1806 don't need to backup ISP image\n");
  }
  if(version==QIC_HWVERSION_QIC1822)
  {
    LOG_PRINT(debug_str, DEBUG_INFO,"QIC1822 don't need to backup ISP image\n");
  }
  else{
  /* backup isp image */
      if (isp_image != NULL) {
                                isp_image_max_size=FLASH_ISP_MAX_SIZE;
        snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1802_ISP_IMAGE_NAME);
        LOG_PRINT(debug_str, DEBUG_INFO, "backup ISP image to %s\n", cur_path);

        cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
        if (cur_fd != -1) {
          if ( -1 == write (cur_fd, isp_image, isp_image_max_size)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "can't write ISP image: %s\n", strerror(errno));
            ret = 1;
          }

          if ( -1 == fsync(cur_fd)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync ISP image: %s\n", strerror(errno));
            ret = 1;
          }

          if ( -1 == close(cur_fd)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "can't close ISP image: %s\n", strerror(errno));
            ret = 1;
          }

          cur_fd = -1;
        }
        else {
          LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the ISP image block : %s\n", strerror(errno));
          ret = 1;
        }
      }
    }


/* backup parm image */
  if (parm_image != NULL) {


    if(version==QIC_HWVERSION_QIC1822){
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1822_PARAM_IMAGE_NAME);
      parm_image_max_size=QIC1822_FLASH_PARAM_MAX_SIZE;
    }else{
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1816_PARAM_IMAGE_NAME);
      parm_image_max_size=FLASH_PARAM_MAX_SIZE;
    }

    LOG_PRINT(debug_str, DEBUG_INFO, "backup PARAM image to %s\n", cur_path);

    cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (cur_fd != -1) {
      if ( -1 == write (cur_fd, parm_image, parm_image_max_size)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't write PARAM image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == fsync(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync PARAM image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == close(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't close PARAM image: %s\n", strerror(errno));
        ret = 1;
      }

      cur_fd = -1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the PARAM image block : %s\n", strerror(errno));
      ret = 1;
    }
  }


/* backup audio image */
  if (audio_image != NULL) {

     if(version==QIC_HWVERSION_QIC1822){
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1822_AUDIO_IMAGE_NAME);
      audio_image_max_size=QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE;
    }else{
      snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1816_AUDIO_IMAGE_NAME);
      audio_image_max_size=QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE;
    }

    LOG_PRINT(debug_str, DEBUG_INFO, "backup AUDIO image to %s\n", cur_path);

    cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    if (cur_fd != -1) {
      if ( -1 == write (cur_fd, audio_image, audio_image_max_size)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't write AUDIO image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == fsync(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync AUDIO image: %s\n", strerror(errno));
        ret = 1;
      }

      if ( -1 == close(cur_fd)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't close AUDIO image: %s\n", strerror(errno));
        ret = 1;
      }

      cur_fd = -1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the AUDIO image block : %s\n", strerror(errno));
      ret = 1;
    }
  }


  /* backup osd font image */
    if (osd_font_image != NULL) {

       if(version==QIC_HWVERSION_QIC1822){
        snprintf(cur_path, sizeof(cur_path), "%s/%s", default_path, QIC1822_OSD_FONT_IMAGE_NAME);
        osd_font_image_max_size=QIC1822_FLASH_OSD_FONT_MAX_SIZE;
      }


      LOG_PRINT(debug_str, DEBUG_INFO, "backup OSD Font image to %s\n", cur_path);

      cur_fd = open(cur_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
      if (cur_fd != -1) {
        if ( -1 == write(cur_fd, osd_font_image, osd_font_image_max_size)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't write OSD font image: %s\n", strerror(errno));
          ret = 1;
        }

        if ( -1 == fsync(cur_fd)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't fsync OSD font image: %s\n", strerror(errno));
          ret = 1;
        }

        if ( -1 == close(cur_fd)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't close OSD font image: %s\n", strerror(errno));
          ret = 1;
        }

        cur_fd = -1;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_ERROR, "Can't open the OSD font image block : %s\n", strerror(errno));
        ret = 1;
      }
    }

  return ret;

}


int qic_update_firmware_by_filename(unsigned int dev_id,
                      char *update_usb_img_loc,
                      char *update_isp_img_loc,
                      char *update_param_img_loc,
                      char *update_audio_img_loc,
                      char *update_osd_font_img_loc,
                      unsigned char update_flags,
                      QicHwVer_t version) {


  unsigned char *orig_usb_image, *orig_isp_image, *orig_parm_image,orig_audio_image[QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE], *orig_osd_font_image; /* read from system */
  unsigned char *new_usb_image, *new_isp_image, *new_parm_image,*new_audio_image, *new_osd_font_image; /*read from file */
  int ret = 0;
  int usb_img_fd = -1;
  int isp_img_fd = -1;
  int parm_img_fd = -1;
  int audio_img_fd = -1;
  int osd_font_img_fd = -1;

  int usb_img_size = 0;
  int isp_img_size = 0;
  int parm_img_size = 0;
  int audio_img_size = 0;
  int osd_font_img_size = 0;

        unsigned char img_header[QIC_IMG_HEADER_SIZE];
  unsigned char header_verify[QIC_IMG_HEADER_SIZE];
 //      unsigned char error_test[16]={0xa, 0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa};
        unsigned char usb_fail=0;
   unsigned char isp_fail=0;
  unsigned char parm_fail=0;
  unsigned char audio_fail=0;
  unsigned char osd_font_fail=0;
  unsigned long sum=0;
  //QicHwVer_t version;
        int image_max_size;
  int usb_image_max_size;
        int isp_image_max_size;
  int parm_image_max_size;
  int audio_image_max_size;
  int osd_font_image_max_size;

  /* read data from cam */
  unsigned int index;
  int count = 0;
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      count ++;
      QicChangeFD(dev_pt->cam[index].fd);
    }
  }

  //QicGetHWVersion(&version);

  if(version==QIC_HWVERSION_QIC1806){
        image_max_size=QIC1816_FLASH_MAX_SIZE;
    usb_image_max_size=QIC1816_FLASH_USB_MAX_SIZE;
    parm_image_max_size=QIC1816_FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=0;
    osd_font_image_max_size=0;
  }
  else if(version==QIC_HWVERSION_QIC1822){
        image_max_size=QIC1822_FLASH_MAX_SIZE;  //only erase and update USB and Parm
    usb_image_max_size=QIC1822_FLASH_USB_MAX_SIZE;
    parm_image_max_size=QIC1822_FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE;
    osd_font_image_max_size=QIC1822_FLASH_OSD_FONT_MAX_SIZE;
  }
  else{
        image_max_size=FLASH_MAX_SIZE;
    usb_image_max_size=FLASH_USB_MAX_SIZE;
        isp_image_max_size=FLASH_ISP_MAX_SIZE;
    parm_image_max_size=FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=0;
    osd_font_image_max_size=0;
  }


  if(update_osd_font_img_loc&&version!=QIC_HWVERSION_QIC1822){
      printf("OSD only support on QIC1822/1832\n");
      return 1;

  }
  if(update_audio_img_loc&&version!=QIC_HWVERSION_QIC1822){
      printf("AUDIO parm only support on QIC1822/1832\n");
      return 1;

  }

#ifdef QIC_SUPPORT_2ND_BL
  /*checking 2nd Boot Rom if available*/
      ret = QicFlashRead(QIC1822_FLASH_2ndBL_ADDR, header_verify, QIC_IMG_HEADER_SIZE,image_max_size);

      if (header_verify[0] != 0x18&&header_verify[1] != 0x22){
        printf("QIC1822 2nd Boot Rom no Found\n");
    return FW_NO_2ND_BL;
      }

#endif


  orig_usb_image = calloc( 1, usb_image_max_size);
  if(version==QIC_HWVERSION_QIC1806 ||version==QIC_HWVERSION_QIC1802)
  orig_isp_image = calloc (1, isp_image_max_size);

  orig_parm_image = calloc (1, parm_image_max_size);
  //orig_audio_image = calloc (1, audio_image_max_size);
  orig_osd_font_image = calloc (1, osd_font_image_max_size);

  new_usb_image = NULL;
  if(version==QIC_HWVERSION_QIC1806 ||version==QIC_HWVERSION_QIC1802)
  new_isp_image = NULL;
  new_parm_image = NULL;
  new_audio_image = NULL;
  new_osd_font_image = NULL;

  if ((orig_usb_image == NULL) || (orig_parm_image == NULL)|| (orig_osd_font_image == NULL)|| (orig_audio_image == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't malloc\n", __func__);
    goto ERR_CLOSE;
  }

  if((version==QIC_HWVERSION_QIC1806 ||version==QIC_HWVERSION_QIC1802)&&(orig_isp_image == NULL)){
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't ISP malloc\n", __func__);
    goto ERR_CLOSE;
  }

  ret = QicSetPll();

#ifdef QIC_SUPPORT_2ND_BL
  ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);
#else
  ret = QicFlashSetSpiConfig(0x08, MODE_FLASH_PROGRAM_PP);
#endif
    /* read to ram */
    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, orig_usb_image, usb_image_max_size,image_max_size);
    else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, orig_usb_image, usb_image_max_size,image_max_size);
    else
      ret = QicFlashRead(FLASH_USB_ADDR, orig_usb_image, usb_image_max_size,image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "USB block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, 0, "USB block read from flash\n");

      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;


    }
  if(version==QIC_HWVERSION_QIC1806){
    LOG_PRINT(debug_str,DEBUG_INFO,"QIC1816/1806 don't need to flash ISP image\n");
  }
  else if(version==QIC_HWVERSION_QIC1822){
    LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to flash ISP image\n");
  }
  else{
    ret = QicFlashRead(FLASH_ISP_ADDR, orig_isp_image, isp_image_max_size,image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "ISP block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, 0, "ISP block read from flash\n");

      new_isp_image = orig_isp_image;
      usb_img_size = isp_image_max_size;

    }
  }

        if(version==QIC_HWVERSION_QIC1806)
    ret = QicFlashRead(QIC1816_FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size,image_max_size);
   else if(version==QIC_HWVERSION_QIC1822)
    ret = QicFlashRead(QIC1822_FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size,image_max_size);
  else
    ret = QicFlashRead(FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size,image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "PARAM block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, DEBUG_INFO, "PARAM block read from flash\n");

      new_parm_image = orig_parm_image;
      usb_img_size = parm_image_max_size;

    }

    printf("audio %p\n",orig_audio_image);

    if(version==QIC_HWVERSION_QIC1822)
    ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, orig_audio_image, audio_image_max_size,image_max_size);

    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "AUDIO block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, DEBUG_INFO, "AUDIO block read from flash\n");

      new_audio_image = orig_audio_image;
      usb_img_size = audio_image_max_size;

    }


   if(version==QIC_HWVERSION_QIC1822)
    ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, orig_osd_font_image, osd_font_image_max_size,image_max_size);


    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "OSD FONT block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, DEBUG_INFO, "OSD FONT block read from flash\n");

      new_osd_font_image = orig_osd_font_image;
      usb_img_size = osd_font_image_max_size;

    }


    if (!ret) {

    }
    else {  //return and not forward
      goto ERR_CLOSE;
    }

    /* read image file */
    char image_name[512];

                if (update_usb_img_loc == NULL) {
      char tmp[10];
                        update_usb_img_loc=tmp;
                        strcpy(update_usb_img_loc,".");
    }

    /* open file for reading */
    if (update_flags & USB_FIRMWARE) {  /* USB image */

    if(version==QIC_HWVERSION_QIC1806)
                        snprintf(image_name, sizeof(image_name), "%s", update_usb_img_loc);
    else if(version==QIC_HWVERSION_QIC1822)
                        snprintf(image_name, sizeof(image_name), "%s", update_usb_img_loc);
    else
                        snprintf(image_name, sizeof(image_name), "%s", update_usb_img_loc);

      usb_img_fd = open(image_name, O_RDONLY);

      if ( -1 == usb_img_fd) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

        /* get the file size */
        usb_img_size = lseek(usb_img_fd, 0, SEEK_END);

        if (-1 == usb_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
          close(usb_img_fd);
          ret = 1;
        }
        else {
          lseek(usb_img_fd, 0, SEEK_SET);

          /* size check */
          if (usb_img_size > usb_image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", usb_img_size, usb_image_max_size);
            close(usb_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", usb_img_size);

            /* map the image */
            new_usb_image= mmap (0, usb_img_size, PROT_READ, MAP_SHARED, usb_img_fd, 0);
            if (MAP_FAILED == new_usb_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              close(usb_img_fd);
              ret = 1;
            }
          }
        }
      }

    }
    else {
      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;
    }

    if (ret) {
      goto ERR_CLOSE;
    }
    else {
      if (update_flags & ISP_FIRMWARE) {  /* ISP image */

        if(version==QIC_HWVERSION_QIC1806){
          LOG_PRINT(debug_str,DEBUG_INFO,"QIC1816/1806 don't need to flash ISP image\n");
        }
        else if(version==QIC_HWVERSION_QIC1822){
          LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to flash ISP image\n");
        }
        else{
                                                snprintf(image_name, sizeof(image_name), "%s", update_isp_img_loc);

            isp_img_fd = open(image_name, O_RDONLY);

            if ( -1 == isp_img_fd) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
            new_isp_image = orig_isp_image;
            isp_img_size = isp_image_max_size;
              //ret=1;
            }
            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

              /* get the file size */
              isp_img_size = lseek(isp_img_fd, 0, SEEK_END);
              if (-1 == isp_img_size) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
                close(isp_img_fd);
                ret = 1;
              }
              else {
                lseek(isp_img_fd, 0, SEEK_SET);

                /* size check */
                if (isp_img_size > isp_image_max_size) {
                  LOG_PRINT(debug_str, DEBUG_ERROR, "isp image size erro: too big, (%d is larger than %d bytes)\n", isp_img_size, isp_image_max_size);
                  close(isp_img_fd);
                  ret = 1;
                }
                else {
                  LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", isp_img_size);

                  /* map the image */
                  new_isp_image= mmap (0, isp_img_size, PROT_READ, MAP_SHARED, isp_img_fd, 0);
                  if (MAP_FAILED == new_isp_image) {
                    LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                    close(isp_img_fd);
                    ret = 1;
                  }
                }
              }
            }
          }
        }
      else {
        new_isp_image = orig_isp_image;
        isp_img_size = isp_image_max_size;
      }
    }



    if (ret) {
      goto ERR_CLOSE;
    }

    else {
      if (update_flags & PARAM_FIRMWARE) {  /* PARM image */

             if(version==QIC_HWVERSION_QIC1806){
              snprintf(image_name, sizeof(image_name), "%s", update_param_img_loc);
        }else if(version==QIC_HWVERSION_QIC1822){
              snprintf(image_name, sizeof(image_name), "%s", update_param_img_loc);
        }else{
              snprintf(image_name, sizeof(image_name), "%s", update_param_img_loc);
        }
        parm_img_fd = open(image_name, O_RDONLY);

        if ( -1 == parm_img_fd) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
        new_parm_image = orig_parm_image;
        parm_img_size = parm_image_max_size;
          //ret=1;
        }
        else {

          LOG_PRINT(debug_str, DEBUG_INFO, "open isp parm file %s for reading f/w version\n", image_name);

          /* get the file size */
          parm_img_size = lseek(parm_img_fd, 0, SEEK_END);
          if (-1 == parm_img_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "isp parm file %s size error: %s\n", image_name, strerror(errno));
            close(parm_img_fd);
            ret = 1;
          }
          else {
            lseek(parm_img_fd, 0, SEEK_SET);

            /* size check */
            if (parm_img_size > parm_image_max_size) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "isp parm image size erro: too big, (%d is larger than %d bytes)\n", parm_img_size, parm_image_max_size);
              close(parm_img_fd);
              ret = 1;
            }
            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "isp parm image size: %d\n", parm_img_size);

              /* map the image */
              new_parm_image= mmap (0, parm_img_size, PROT_READ, MAP_SHARED, parm_img_fd, 0);
              if (MAP_FAILED == new_parm_image) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                close(parm_img_fd);
                ret = 1;
              }
            }
          }
        }
      }
      else {
        new_parm_image = orig_parm_image;
        parm_img_size = parm_image_max_size;
      }

    }


    if (ret) {
      goto ERR_CLOSE;
    }

    else {
      if (update_flags & AUDIO_FIRMWARE) {  /* audio image */

             if(version==QIC_HWVERSION_QIC1806){
              snprintf(image_name, sizeof(image_name), "%s", update_audio_img_loc);
        }else if(version==QIC_HWVERSION_QIC1822){
              snprintf(image_name, sizeof(image_name), "%s", update_audio_img_loc);
        }else{
              snprintf(image_name, sizeof(image_name), "%s", update_audio_img_loc);
        }
        audio_img_fd = open(image_name, O_RDONLY);

        if ( -1 == parm_img_fd) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
        new_audio_image = orig_audio_image;
        audio_img_size = audio_image_max_size;
          //ret=1;
        }
        else {

          LOG_PRINT(debug_str, DEBUG_INFO, "open audio parm file %s for reading f/w version\n", image_name);

          /* get the file size */
          audio_img_size = lseek(audio_img_fd, 0, SEEK_END);
          if (-1 == audio_img_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "audio parm file %s size error: %s\n", image_name, strerror(errno));
            close(audio_img_fd);
            ret = 1;
          }
          else {
            lseek(audio_img_fd, 0, SEEK_SET);

            /* size check */
            if (audio_img_size > audio_image_max_size) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "audio parm image size erro: too big, (%d is larger than %d bytes)\n", audio_img_size, audio_image_max_size);
              close(audio_img_fd);
              ret = 1;
            }
            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "audio parm image size: %d\n", audio_img_size);

              /* map the image */
              new_audio_image= mmap (0, audio_img_size, PROT_READ, MAP_SHARED, audio_img_fd, 0);
              if (MAP_FAILED == new_audio_image) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                close(audio_img_fd);
                ret = 1;
              }
            }
          }
        }
      }
      else {
        new_audio_image = orig_audio_image;
        audio_img_size = audio_image_max_size;
      }

    }


    if (ret) {
      goto ERR_CLOSE;
    }
    else {
      if (update_flags & OSD_FONT_FIRMWARE) { /* OSD FONT image */


        if(version==QIC_HWVERSION_QIC1822){
              snprintf(image_name, sizeof(image_name), "%s", update_osd_font_img_loc);

        osd_font_img_fd = open(image_name, O_RDONLY);

        if ( -1 == osd_font_img_fd) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't open isp parm file %s for reading f/w info: %s\n", image_name, strerror(errno));
        new_osd_font_image = orig_osd_font_image;
        osd_font_img_size = osd_font_image_max_size;
          //ret=1;
        }
        else {

          LOG_PRINT(debug_str, DEBUG_INFO, "open isp parm file %s for reading f/w version\n", image_name);

          /* get the file size */
          osd_font_img_size = lseek(osd_font_img_fd, 0, SEEK_END);
          if (-1 == osd_font_img_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "isp parm file %s size error: %s\n", image_name, strerror(errno));
            close(osd_font_img_fd);
            ret = 1;
          }
          else {
            lseek(parm_img_fd, 0, SEEK_SET);

            /* size check */
            if (osd_font_img_size > osd_font_image_max_size) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "isp parm image size erro: too big, (%d is larger than %d bytes)\n", osd_font_img_size, osd_font_image_max_size);
              close(osd_font_img_fd);
              ret = 1;
            }
            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "isp parm image size: %d\n", osd_font_img_size);

              /* map the image */
              new_osd_font_image= mmap (0, osd_font_img_size, PROT_READ, MAP_SHARED, osd_font_img_fd, 0);
              if (MAP_FAILED == new_osd_font_image) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                close(osd_font_img_fd);
                ret = 1;
              }
            }
          }
        }
      }
      else {
        new_osd_font_image = orig_osd_font_image;
        osd_font_img_size = osd_font_image_max_size;
      }

    }
  }

    if (ret) {
      goto ERR_CLOSE;
    }

    /* erase flash */
    int i;
    unsigned char *BufVerify = calloc (1, image_max_size);

  printf("\nFlash erasing...\n");
  /* erase */
  if(version==QIC_HWVERSION_QIC1822){
#ifdef QIC_SUPPORT_2ND_BL
    for (i =QIC1822_FLASH_USB_ADDR; i < image_max_size; i += FLASH_SECTOR_SIZE) {
    ret = QicFlashSectorErase(i);
    printf("\r [size:%d/total:%d]",i,image_max_size);
       usleep(20*1000);
  }


  printf("\n [size:%d/total:%d]\n",image_max_size,image_max_size);
  printf("Flash erasing... done\n");
#else
      if(update_flags & USB_FIRMWARE){
     QicFlashErase ();
            usleep(2000*1000);
      }else{
        if(update_flags & USB_FIRMWARE){
      for (i =QIC1822_FLASH_USB_ADDR; i < image_max_size; i += FLASH_SECTOR_SIZE) {
      ret = QicFlashSectorErase(i);
      printf("\r [size:%d/total:%d]",i,image_max_size);
       usleep(20*1000);
      }

    }
    if(update_flags & PARAM_FIRMWARE){

      for (i =QIC1822_FLASH_PARAM_ADDR; i < QIC1822_FLASH_PARAM_ADDR+parm_image_max_size; i += FLASH_SECTOR_SIZE) {
      ret = QicFlashSectorErase(i);
      printf("\r [size:%d/total:%d]",i,parm_image_max_size);
       usleep(20*1000);
      }

    }
    if(update_flags & AUDIO_FIRMWARE){
      for (i =QIC1822_FLASH_AUDIO_PARAM_ADDR; i < QIC1822_FLASH_AUDIO_PARAM_ADDR+audio_image_max_size; i += FLASH_SECTOR_SIZE) {
      ret = QicFlashSectorErase(i);
      printf("\r [size:%d/total:%d]",i,audio_image_max_size);
       usleep(20*1000);
      }

    }
    if(update_flags & OSD_FONT_FIRMWARE){
      for (i =QIC1822_FLASH_OSD_FONT_ADDR; i < QIC1822_FLASH_OSD_FONT_ADDR+osd_font_image_max_size; i += FLASH_SECTOR_SIZE) {
      ret = QicFlashSectorErase(i);
      printf("\r [size:%d/total:%d]",i,osd_font_image_max_size);
       usleep(20*1000);
      }

    }


      }
#endif

  }else{
       QicFlashErase ();
            usleep(2000*1000);
    }

    /* check if erase complete */
    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
    else if(version==QIC_HWVERSION_QIC1822){
#ifdef QIC_SUPPORT_2ND_BL
      ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size-QIC1822_FLASH_2ndBL_MAX_SIZE,image_max_size);
#else
      ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
#endif
    }
    else
      ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "Check Flash USB sector erase ret=%d\n",ret);



  if(version==QIC_HWVERSION_QIC1822){
#ifdef QIC_SUPPORT_2ND_BL
    for (i = /*QIC1822_FLASH_USB_ADDR*/0; i < image_max_size-QIC1822_FLASH_2ndBL_MAX_SIZE; i++)
      {
        if (BufVerify[i] != 0xff)
        {
          ret = 1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
        }
      }
#else

   if(update_flags & USB_FIRMWARE){
    for (i = 0; i < image_max_size; i++)
      {
        if (BufVerify[i] != 0xff)
        {
          ret = 1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
        }
      }
   }
   else{
      if(update_flags & PARAM_FIRMWARE){
        for (i = QIC1822_FLASH_PARAM_ADDR; i < parm_image_max_size; i++)
          {
            if (BufVerify[i] != 0xff)
            {
              ret = 1;
              LOG_PRINT(debug_str, DEBUG_ERROR, "Flash PARAM erase error at sector %x %x\n", i,BufVerify[i] );
            }
          }
       }
      if(update_flags & AUDIO_FIRMWARE){
        for (i = QIC1822_FLASH_AUDIO_PARAM_ADDR; i < audio_image_max_size; i++)
          {
            if (BufVerify[i] != 0xff)
            {
              ret = 1;
              LOG_PRINT(debug_str, DEBUG_ERROR, "Flash AUDIO erase error at sector %x %x\n", i,BufVerify[i] );
            }
          }
       }
      if(update_flags & OSD_FONT_FIRMWARE){
        for (i = QIC1822_FLASH_OSD_FONT_ADDR; i < osd_font_image_max_size; i++)
          {
            if (BufVerify[i] != 0xff)
            {
              ret = 1;
              LOG_PRINT(debug_str, DEBUG_ERROR, "Flash OSD font erase error at sector %x %x\n", i,BufVerify[i] );
            }
          }
       }




   }

#endif

  }else{
       for (i = 0; i < image_max_size; i++)
      {
        if (BufVerify[i] != 0xff)
        {
          ret = 1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
        }
      }
    }

    if (ret) {

      free(BufVerify);
      goto ERR_CLOSE_MMAP;
    }

    /* write to flash */

    if (update_flags & USB_FIRMWARE) {
                  printf("\nUSB image flashing... \n");
      memcpy(&img_header[0],&new_usb_image[0],QIC_IMG_HEADER_SIZE);

      if(version==QIC_HWVERSION_QIC1806)
        ret = QicFlashWriteUSBIMG(QIC1816_FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE,image_max_size);
         else if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashWriteUSBIMG (QIC1822_FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE,image_max_size);
      else
        ret = QicFlashWriteUSBIMG(FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE,image_max_size);

    LOG_PRINT(debug_str, DEBUG_INFO, "USB Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "USB Sector Image Write Failed\n");
    }
        printf("USB image flashing...done \n");
      }


             if (update_flags & ISP_FIRMWARE)   {
    if(version==QIC_HWVERSION_QIC1806){
      LOG_PRINT(debug_str,DEBUG_INFO,"QIC1806/1816 don't need to update ISP image!!\n");
    }
    else{
      ret = QicFlashWrite (FLASH_ISP_ADDR, new_isp_image, isp_img_size,image_max_size);
      LOG_PRINT(debug_str, DEBUG_INFO, "ISP Sector Image Write to Flash\n");
      if (ret != 0)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "ISP Sector Image Write Failed\n");
      }
    }

              }


          if (update_flags & PARAM_FIRMWARE){

        printf("\nISP PARAM image flashing... \n");
                   if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashWriteUSBIMG (QIC1816_FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size,image_max_size);
       else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashWriteUSBIMG (QIC1822_FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size,image_max_size);
       else
           ret = QicFlashWriteUSBIMG (FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size,image_max_size);
    LOG_PRINT(debug_str, DEBUG_INFO, "PARAM Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "PARAM Sector Image Write Failed\n");
    }
         printf("ISP PARM image flashing...done \n");
              }


          if (update_flags & AUDIO_FIRMWARE){

        printf("\nAUDIO PARAM image flashing... \n");
            if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashWriteUSBIMG (QIC1822_FLASH_AUDIO_PARAM_ADDR, new_audio_image, audio_image_max_size,image_max_size);


    LOG_PRINT(debug_str, DEBUG_INFO, "AUDIO Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "AUDIO Sector Image Write Failed\n");
    }
         printf("AUDIO PARM image flashing...done \n");
              }


        if (update_flags & OSD_FONT_FIRMWARE){

        printf("\nOSD FONT image flashing... \n");
              if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashWriteUSBIMG (QIC1822_FLASH_OSD_FONT_ADDR, new_osd_font_image, osd_font_image_max_size,image_max_size);


    LOG_PRINT(debug_str, DEBUG_INFO, "OSD FONT Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "OSD FONT Sector Image Write Failed\n");
    }
         printf("OSD FONT image flashing...done \n");
          }



    /*verify*/
    if (update_flags & USB_FIRMWARE){


    /*for error test*//*
                        ret = QicFlashWriteUSBIMG(QIC1822_FLASH_USB_ADDR+16, error_test, 16, image_max_size);
            LOG_PRINT(debug_str, DEBUG_INFO, "write error\n");
                 */

      if(version==QIC_HWVERSION_QIC1806)
        ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, usb_img_size,image_max_size);
      else if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, usb_img_size,image_max_size);
      else
        ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, usb_img_size,image_max_size);


      LOG_PRINT(debug_str, DEBUG_INFO, "Verify USB Sector Image\n");
      for (i = 0+QIC_IMG_HEADER_SIZE; i < usb_img_size-QIC_IMG_HEADER_SIZE; i++)
      {
        if (BufVerify[i] != new_usb_image[i])
        {
          ret = 1;
          usb_fail=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "USB Sector Image Verify Failed at %d, %x, %x\n", i, new_usb_image[i], BufVerify[i]);
        }
      }

      if(usb_fail){
          printf("\nUSB Flash ERROR!!!!!!!!!!!!!\n");
    }


    }


    if (update_flags & ISP_FIRMWARE)  {
      if(version==QIC_HWVERSION_QIC1806){
        LOG_PRINT(debug_str,DEBUG_INFO,"QIC1806/1816 don't need to verify ISP image!!\n");
    }
      else if(version==QIC_HWVERSION_QIC1822){
        LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to verify ISP image!!\n");
    }
    else{
        ret = QicFlashRead(FLASH_ISP_ADDR, BufVerify, isp_img_size,image_max_size);
        LOG_PRINT(debug_str, DEBUG_INFO, "Verify ISP Sector Image\n");
        for (i = 0; i < isp_img_size; i++)
        {
          if (BufVerify[i] != new_isp_image[i])
          {
            ret = 1;
            isp_fail=1;

            LOG_PRINT(debug_str, DEBUG_ERROR, "ISP Sector Image Verify Failed at %d\n", i);
          }
        }

        if(isp_fail){
          printf("\nISP Flash ERROR!!!!!!!!!!!!!\n");
              }


      }
              }


              if (update_flags & PARAM_FIRMWARE){
                    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);
      else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);
        else
            ret = QicFlashRead(FLASH_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);
    LOG_PRINT(debug_str, 0, "Verify PARAM Sector Image\n");
    for (i = 0; i < parm_image_max_size; i++)
    {
      if (BufVerify[i] != new_parm_image[i])
      {
        ret = 1;
        parm_fail=1;

        LOG_PRINT(debug_str, DEBUG_ERROR, "PARAM Sector Image Verify Failed at %d\n", i);
      }
    }

        if(parm_fail){
        printf("\nISP PARM Flash ERROR!!!!!!!!!!!!!!\n");
      }

    }


        if (update_flags & AUDIO_FIRMWARE){
          if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);

      LOG_PRINT(debug_str, 0, "Verify AUDIO Sector Image\n");
      for (i = 0; i < audio_image_max_size; i++)
      {
        if (BufVerify[i] != new_audio_image[i])
        {
          ret = 1;
          audio_fail=1;

          LOG_PRINT(debug_str, DEBUG_ERROR, "AUDIO Sector Image Verify Failed at %d\n", i);
        }
      }

        if(audio_fail){
          printf("\nAUDIO PARM Flash ERROR!!!!!!!!!!!!!!\n");
        }

      }


      if (update_flags & OSD_FONT_FIRMWARE) {
      if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, BufVerify, osd_font_image_max_size,image_max_size);


  LOG_PRINT(debug_str, 0, "Verify OSD FONT Sector Image\n");
  for (i = 0; i < osd_font_image_max_size; i++)
  {
    if (BufVerify[i] != new_osd_font_image[i])
    {
      ret = 1;
      osd_font_fail=1;

      LOG_PRINT(debug_str, DEBUG_ERROR, "OSD FONT Sector Image Verify Failed at %d\n", i);
    }
  }

    if(osd_font_fail){
      printf("\nOSD FONT Flash ERROR!!!!!!!!!!!!!!\n");
    }

  }


              if (update_flags & USB_FIRMWARE) {
      if(!usb_fail&&!isp_fail&&!parm_fail){
         printf("\ndownload ok\n");

        memset(header_verify,0,sizeof(header_verify));

      if(version==QIC_HWVERSION_QIC1806){
        ret = QicFlashWrite(QIC1816_FLASH_USB_ADDR, img_header, QIC_IMG_HEADER_SIZE, image_max_size);
        /*checking header*/
        ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, header_verify, QIC_IMG_HEADER_SIZE,image_max_size);

        if (header_verify[0] != 0x18&&header_verify[1] != 0x06){
          printf("QIC1806 header flash error\n");
          ret= 1;
        }
      }else if(version==QIC_HWVERSION_QIC1822){
        ret = QicFlashWrite(QIC1822_FLASH_USB_ADDR, img_header, QIC_IMG_HEADER_SIZE, image_max_size);
        /*checking header*/
        ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, header_verify, QIC_IMG_HEADER_SIZE,image_max_size);

        if (header_verify[0] != 0x18&&header_verify[1] != 0x22){
          printf("QIC1822 header flash error\n");
          ret= 1;
        }

      }else
        ret = QicFlashWrite(FLASH_USB_ADDR, img_header,QIC_IMG_HEADER_SIZE,image_max_size);
                }else{
        printf("\ndownload error\n");
        ret=1;


                }
              }

                  //     sum=CalculateCKSum(image_max_size);
                   //     printf("\n  checksum= %u\n",sum);


    if (ret) {
      free (BufVerify);
      goto ERR_CLOSE_MMAP;
    }

    free(BufVerify);

  /*release mem */
ERR_CLOSE_MMAP:
  if (new_isp_image != orig_isp_image) {
    munmap(new_isp_image, isp_img_size);
    close(isp_img_fd);
  }

  if (new_usb_image != orig_usb_image) {
    munmap(new_usb_image, usb_img_size);
    close(usb_img_fd);
  }

  if (new_parm_image != orig_parm_image) {
    munmap (new_parm_image, parm_img_size);
    close(parm_img_fd);
  }

  if (new_audio_image != orig_audio_image) {
    munmap (new_audio_image, audio_img_size);
    close(parm_img_fd);
  }

  if (new_osd_font_image != orig_osd_font_image) {
    munmap (new_osd_font_image, osd_font_img_size);
    close(osd_font_img_fd);
  }

ERR_CLOSE:
  free(orig_usb_image);
  free(orig_isp_image);
  free(orig_parm_image);
//  free(orig_audio_image);
  free(orig_osd_font_image);

  return ret;

}

int qic_backup_firmware_to_file(unsigned int dev_id, char *update_img_loc, unsigned char update_flags, char *backup_img_loc, unsigned char backup_flags, QicHwVer_t version) {


  unsigned char *orig_usb_image, *orig_isp_image, *orig_parm_image, *orig_audio_image, orig_osd_font_image[QIC1822_FLASH_OSD_FONT_MAX_SIZE]; /* read from system */
  unsigned char *new_usb_image, *new_isp_image, *new_parm_image, *new_audio_image,*new_osd_font_image; /*read from file */
  int ret = 0;
  char param_image_name[128];
  int usb_img_fd = -1;
  int isp_img_fd = -1;
  int parm_img_fd = -1;
  int audio_img_fd = -1;
  int osd_img_fd = -1;

  int usb_img_size = 0;
  int isp_img_size = 0;
  int parm_img_size = 0;
  int audio_img_size = 0;
  int osd_font_img_size = 0;

        unsigned char img_header[QIC_IMG_HEADER_SIZE];
        unsigned char usb_fail=0;
   unsigned char isp_fail=0;
  unsigned char parm_fail=0;
  unsigned char audio_fail=0;
  unsigned char osd_font_fail=0;

  unsigned char *usb_w= NULL;
  unsigned char *isp_w = NULL;
  unsigned char *param_w = NULL;
  unsigned char *audio_w = NULL;
  unsigned char *osd_font_w = NULL;
  //QicHwVer_t version;
        int image_max_size;
  int usb_image_max_size;
        int isp_image_max_size;
  int parm_image_max_size;
  int audio_image_max_size;
  int osd_font_image_max_size;
  /* read data from cam */
  unsigned int index;
  int count = 0;
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      count ++;
      QicChangeFD(dev_pt->cam[index].fd);
    }
  }

  //QicGetHWVersion(&version);

  if(version==QIC_HWVERSION_QIC1806){ // for qic1806
                image_max_size=QIC1816_FLASH_MAX_SIZE;
    usb_image_max_size=QIC1816_FLASH_USB_MAX_SIZE;
    parm_image_max_size=QIC1816_FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=QIC1816_FLASH_AUDIO_PARAM_MAX_SIZE;
      osd_font_image_max_size=0;
  }
  else if(version==QIC_HWVERSION_QIC1822){ // for qic1822
                image_max_size=QIC1822_FLASH_MAX_SIZE;
    usb_image_max_size=QIC1822_FLASH_USB_MAX_SIZE;
    parm_image_max_size=QIC1822_FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=QIC1822_FLASH_AUDIO_PARAM_MAX_SIZE;
    osd_font_image_max_size=QIC1822_FLASH_OSD_FONT_MAX_SIZE;
  }
  else{// for qic1802
               image_max_size=FLASH_MAX_SIZE;
    usb_image_max_size=FLASH_USB_MAX_SIZE;
                isp_image_max_size=FLASH_ISP_MAX_SIZE;
    parm_image_max_size=FLASH_PARAM_MAX_SIZE;
    audio_image_max_size=QIC1816_FLASH_AUDIO_PARAM_MAX_SIZE;
      osd_font_image_max_size=0;
  }

  orig_usb_image = calloc( 1, usb_image_max_size);
  orig_isp_image = calloc (1, isp_image_max_size);
  orig_parm_image = calloc (1, parm_image_max_size);
  orig_audio_image = calloc (1, audio_image_max_size);
  //orig_osd_font_image = calloc (1, osd_font_image_max_size);

  //printf("%p, ");

  new_usb_image = NULL;
  new_isp_image = NULL;
  new_parm_image = NULL;
  new_audio_image = NULL;
  new_osd_font_image = NULL;

  if ((orig_usb_image == NULL) || (orig_isp_image == NULL) || (orig_parm_image == NULL)|| (orig_audio_image == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "%s: can't malloc\n", __func__);
    goto ERR_CLOSE;
  }

  ret = QicSetPll();
  ret = QicFlashSetSpiConfig(0x10, MODE_FLASH_PROGRAM_PP);


    /* read to ram */
    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, orig_usb_image, usb_image_max_size, image_max_size);
    else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, orig_usb_image, usb_image_max_size, image_max_size);
    else
      ret = QicFlashRead(FLASH_USB_ADDR, orig_usb_image, usb_image_max_size, image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "USB block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, 0, "USB block read from flash\n");
      if ( backup_flags & USB_FIRMWARE)
        usb_w = orig_usb_image;

      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;


    }
  if(version==QIC_HWVERSION_QIC1806){
    LOG_PRINT(debug_str,DEBUG_INFO,"QIC1816/1806 don't need to flash ISP image\n");
  }
  else if(version==QIC_HWVERSION_QIC1822){
    LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to flash ISP image\n");
  }
  else{
    ret = QicFlashRead(FLASH_ISP_ADDR, orig_isp_image, isp_image_max_size, image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "ISP block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, 0, "ISP block read from flash\n");
      if (backup_flags & ISP_FIRMWARE)
        isp_w = orig_isp_image;

      new_isp_image = orig_isp_image;
      usb_img_size = isp_image_max_size;

    }
  }


        if(version==QIC_HWVERSION_QIC1806)
    ret = QicFlashRead(QIC1816_FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size, image_max_size);
        else if(version==QIC_HWVERSION_QIC1822)
    ret = QicFlashRead(QIC1822_FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size, image_max_size);
  else
    ret = QicFlashRead(FLASH_PARAM_ADDR, orig_parm_image, parm_image_max_size, image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "PARAM block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, DEBUG_INFO, "PARAM block read from flash\n");
      if (backup_flags & PARAM_FIRMWARE)
        param_w = orig_parm_image;

      new_parm_image = orig_parm_image;
      usb_img_size = parm_image_max_size;

    }

 /*audio data*/
  if (backup_flags & AUDIO_FIRMWARE){
        if(version==QIC_HWVERSION_QIC1806)
    ret = QicFlashRead(QIC1816_FLASH_AUDIO_PARAM_ADDR, orig_audio_image, audio_image_max_size, image_max_size);
        else if(version==QIC_HWVERSION_QIC1822)
    ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, orig_audio_image, audio_image_max_size, image_max_size);
  else
      ret = QicFlashRead(QIC1822_FLASH_AUDIO_PARAM_ADDR, orig_audio_image, audio_image_max_size, image_max_size);
    if (ret) {
      LOG_PRINT(debug_str,DEBUG_ERROR, "AUDIO block read error = %d\n", ret);
    }
    else {
      LOG_PRINT(debug_str, DEBUG_INFO, "AUDIO block read from flash\n");
      if (backup_flags & AUDIO_FIRMWARE)
        audio_w = orig_audio_image;

      new_audio_image = orig_audio_image;
      usb_img_size = audio_image_max_size;

    }
  }


    /*osd font data*/
       if (backup_flags & OSD_FONT_FIRMWARE){
          if(version==QIC_HWVERSION_QIC1822)

        printf("dddd %p\n",orig_osd_font_image);
         ret = QicFlashRead(QIC1822_FLASH_OSD_FONT_ADDR, orig_osd_font_image, osd_font_image_max_size, image_max_size);

         if (ret) {
           LOG_PRINT(debug_str,DEBUG_ERROR, "OSD font block read error = %d\n", ret);
         }
         else {
           LOG_PRINT(debug_str, DEBUG_INFO, "OSD font block read from flash\n");
           if (backup_flags & OSD_FONT_FIRMWARE)
             osd_font_w = orig_osd_font_image;

           new_osd_font_image = orig_osd_font_image;
           usb_img_size = osd_font_image_max_size;

         }
       }

    if (!ret) {
      //backup
      ret = qic_backup_firmware(backup_img_loc, (char*)usb_w,  (char*)isp_w,  (char*)param_w,  (char*)audio_w,(char*)osd_font_w, version);
      if (ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s: firmware backup error", __func__);
      }

      goto ERR_CLOSE;

    }
    else {  //return and not forward
      goto ERR_CLOSE;
    }

#if 0
    /* read image file */
    char image_name[512];

    if (update_img_loc == NULL) {
      char tmp[10];
      update_img_loc=tmp;
      strcpy(update_img_loc,".");
    }

    /* open file for reading */
    if (update_flags & USB_FIRMWARE) {  /* USB image */

    if(version==QIC_HWVERSION_QIC1806)
      snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, QIC1816_USB_IMAGE_NAME);
    else if(version==QIC_HWVERSION_QIC1822)
      snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, QIC1822_USB_IMAGE_NAME);
    else
      snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, QIC1802_USB_IMAGE_NAME);

      usb_img_fd = open(image_name, O_RDONLY);

      if ( -1 == usb_img_fd) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

        /* get the file size */
        usb_img_size = lseek(usb_img_fd, 0, SEEK_END);

        if (-1 == usb_img_size) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
          close(usb_img_fd);
          ret = 1;
        }
        else {
          lseek(usb_img_fd, 0, SEEK_SET);

          /* size check */
          if (usb_img_size > usb_image_max_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", usb_img_size, usb_image_max_size);
            close(usb_img_fd);
            ret = 1;
          }
          else {
            LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", usb_img_size);

            /* map the image */
            new_usb_image= mmap (0, usb_img_size, PROT_READ, MAP_SHARED, usb_img_fd, 0);
            if (MAP_FAILED == new_usb_image) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
              close(usb_img_fd);
              ret = 1;
            }
          }
        }
      }

    }
    else {
      new_usb_image = orig_usb_image;
      usb_img_size = usb_image_max_size;
    }

    if (ret) {
      goto ERR_CLOSE;
    }
    else {
      if (update_flags & ISP_FIRMWARE) {  /* ISP image */

        if(version==QIC_HWVERSION_QIC1806){
          LOG_PRINT(debug_str,DEBUG_INFO,"QIC1816/1806 don't need to flash ISP image\n");
        }
        else if(version==QIC_HWVERSION_QIC1822){
          LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to flash ISP image\n");
        }
        else{
            snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, QIC1802_ISP_IMAGE_NAME);

            isp_img_fd = open(image_name, O_RDONLY);

            if ( -1 == isp_img_fd) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
            new_isp_image = orig_isp_image;
            isp_img_size = isp_image_max_size;
            }
            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

              /* get the file size */
              isp_img_size = lseek(isp_img_fd, 0, SEEK_END);
              if (-1 == isp_img_size) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
                close(isp_img_fd);
                ret = 1;
              }
              else {
                lseek(isp_img_fd, 0, SEEK_SET);

                /* size check */
                if (isp_img_size > isp_image_max_size) {
                  LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", isp_img_size, isp_image_max_size);
                  close(isp_img_fd);
                  ret = 1;
                }
                else {
                  LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", isp_img_size);

                  /* map the image */
                  new_isp_image= mmap (0, isp_img_size, PROT_READ, MAP_SHARED, isp_img_fd, 0);
                  if (MAP_FAILED == new_isp_image) {
                    LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                    close(isp_img_fd);
                    ret = 1;
                  }
                }
              }
            }
          }
        }
      else {
        new_isp_image = orig_isp_image;
        isp_img_size = isp_image_max_size;
      }
    }

    if (ret) {
      goto ERR_CLOSE;
    }

    else {
      if (update_flags & PARAM_FIRMWARE) {  /* PARM image */

             if(version==QIC_HWVERSION_QIC1806){
          if(find_shaing_image(update_img_loc, QIC1816_SHAING_IMAGE_NAME, param_image_name)&&param_w)
              strcpy(param_image_name,QIC1816_PARAM_IMAGE_NAME);
              snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, param_image_name);
        }
        else if(version==QIC_HWVERSION_QIC1822){
          if(find_shaing_image(update_img_loc, QIC1822_SHAING_IMAGE_NAME, param_image_name)&&param_w)
              strcpy(param_image_name,QIC1822_PARAM_IMAGE_NAME);
              snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc, param_image_name);
        }
          else{
          if(find_shaing_image(update_img_loc, QIC1802_SHAING_IMAGE_NAME, param_image_name)&&param_w)
              strcpy(param_image_name,QIC1802_PARAM_IMAGE_NAME);
              snprintf(image_name, sizeof(image_name), "%s/%s", update_img_loc,  param_image_name);
        }
        parm_img_fd = open(image_name, O_RDONLY);

        if ( -1 == parm_img_fd) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "can't open file %s for reading f/w info: %s\n", image_name, strerror(errno));
        new_parm_image = orig_parm_image;
        parm_img_size = parm_image_max_size;

        }
        else {
          LOG_PRINT(debug_str, DEBUG_INFO, "open file %s for reading f/w version\n", image_name);

          /* get the file size */
          parm_img_size = lseek(parm_img_fd, 0, SEEK_END);
          if (-1 == parm_img_size) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "file %s size error: %s\n", image_name, strerror(errno));
            close(parm_img_fd);
            ret = 1;
          }
          else {
            lseek(parm_img_fd, 0, SEEK_SET);

            /* size check */
            if (parm_img_size > parm_image_max_size) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "image size erro: too big, (%d is larger than %d bytes)\n", parm_img_size, parm_image_max_size);
              close(parm_img_fd);
              ret = 1;
            }

            else {
              LOG_PRINT(debug_str, DEBUG_INFO, "image size: %d\n", parm_img_size);

              /* map the image */
              new_parm_image= mmap (0, parm_img_size, PROT_READ, MAP_SHARED, parm_img_fd, 0);
              if (MAP_FAILED == new_parm_image) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "mmap error: %s\n", strerror(errno));
                close(parm_img_fd);
                ret = 1;
              }
            }
          }
        }
      }
      else {
        new_parm_image = orig_parm_image;
        parm_img_size = parm_image_max_size;
      }

    }



    if (ret) {
      goto ERR_CLOSE;
    }

    /* erase flash */
    int i;
    unsigned char *BufVerify = calloc (1, image_max_size);

   if(version==QIC_HWVERSION_QIC1822){

    for (i = QIC1822_FLASH_USB_ADDR; i < image_max_size; i += FLASH_SECTOR_SIZE) {
    ret = QicFlashSectorErase(i);
      printf("\r [size:%d/total:%d]",i,image_max_size);
    usleep(50*1000);

  }
        usleep(1000*1000);
    printf("\n [size:%d/total:%d]\n",image_max_size,image_max_size);
    printf("Flash erasing... done\n");
  }else{
    QicFlashErase ();
            usleep(1000*1000);
    }

    /* check if erase complete */
    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
    else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);
    else
      ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, image_max_size,image_max_size);

    LOG_PRINT(debug_str, DEBUG_ERROR, "Check Flash USB sector erase ret=%d\n",ret);

  if(version==QIC_HWVERSION_QIC1822){
    for (i = 0; i < image_max_size; i++)
      {
        if (BufVerify[i] != 0xff)
        {
          ret = 1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
        }
      }
  }else{
  for (i = 0; i < image_max_size; i++)
    {
      if (BufVerify[i] != 0xff)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "Flash USB erase error at sector %x %x\n", i,BufVerify[i] );
      }
    }
    }

    if (ret) {
      free(BufVerify);
      goto ERR_CLOSE_MMAP;
    }

    /* write to flash */

    if (update_flags & USB_FIRMWARE)  {

                             memcpy(img_header,&new_usb_image[0],QIC_IMG_HEADER_SIZE);

      if(version==QIC_HWVERSION_QIC1806)
        ret = QicFlashWriteUSBIMG(QIC1816_FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE,image_max_size);
      else if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashWriteUSBIMG (QIC1822_FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE, image_max_size);
      else
        ret = QicFlashWriteUSBIMG (FLASH_USB_ADDR+QIC_IMG_HEADER_SIZE, new_usb_image+QIC_IMG_HEADER_SIZE, usb_img_size-QIC_IMG_HEADER_SIZE,image_max_size);

    LOG_PRINT(debug_str, DEBUG_INFO, "USB Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "USB Sector Image Write Failed\n");
    }
      }

             if (update_flags & ISP_FIRMWARE)   {
    if(version==QIC_HWVERSION_QIC1806){
      LOG_PRINT(debug_str,DEBUG_INFO,"QIC1806/1816 don't need to update ISP image!!\n");
    }
    else if(version==QIC_HWVERSION_QIC1822){
      LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to update ISP image!!\n");
    }
    else{
      ret = QicFlashWrite (FLASH_ISP_ADDR, new_isp_image, isp_img_size,image_max_size);
      LOG_PRINT(debug_str, DEBUG_INFO, "ISP Sector Image Write to Flash\n");
      if (ret != 0)
      {
        ret = 1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "ISP Sector Image Write Failed\n");
      }
    }
              }


              if (update_flags & PARAM_FIRMWARE)  {
                   if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashWrite (QIC1816_FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size, image_max_size);
                   else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashWrite (QIC1822_FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size, image_max_size);
      else
           ret = QicFlashWrite (FLASH_PARAM_ADDR, new_parm_image, parm_image_max_size, image_max_size);
    LOG_PRINT(debug_str, DEBUG_INFO, "PARAM Sector Image Write to Flash\n");
    if (ret != 0)
    {
      ret = 1;
      LOG_PRINT(debug_str, DEBUG_ERROR, "PARAM Sector Image Write Failed\n");
    }
              }


    /*verify*/
    if (update_flags & USB_FIRMWARE)  {

      if(version==QIC_HWVERSION_QIC1806)
        ret = QicFlashRead(QIC1816_FLASH_USB_ADDR, BufVerify, usb_img_size, image_max_size);
      else if(version==QIC_HWVERSION_QIC1822)
        ret = QicFlashRead(QIC1822_FLASH_USB_ADDR, BufVerify, usb_img_size, image_max_size);
      else
        ret = QicFlashRead(FLASH_USB_ADDR, BufVerify, usb_img_size,image_max_size);

      LOG_PRINT(debug_str, DEBUG_INFO, "Verify USB Sector Image\n");
      for (i = 0+QIC_IMG_HEADER_SIZE; i < usb_img_size-QIC_IMG_HEADER_SIZE; i++)
      {
        if (BufVerify[i] != new_usb_image[i])
        {
          ret = 1;
          usb_fail=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "USB Sector Image Verify Failed at %d\n", i);
        }
      }
    }

    if (update_flags & ISP_FIRMWARE)  {
      if(version==QIC_HWVERSION_QIC1806){
        LOG_PRINT(debug_str,DEBUG_INFO,"QIC1806/1816 don't need to verify ISP image!!\n");
    }
    else if(version==QIC_HWVERSION_QIC1822){
        LOG_PRINT(debug_str,DEBUG_INFO,"QIC1822 don't need to verify ISP image!!\n");
    }
    else{
        ret = QicFlashRead(FLASH_ISP_ADDR, BufVerify, isp_img_size,image_max_size);
        LOG_PRINT(debug_str, DEBUG_INFO, "Verify ISP Sector Image\n");
        for (i = 0; i < isp_img_size; i++)
        {
          if (BufVerify[i] != new_isp_image[i])
          {
            ret = 1;
            isp_fail=1;
            LOG_PRINT(debug_str, DEBUG_ERROR, "ISP Sector Image Verify Failed at %d\n", i);
          }
        }
      }
              }




              if (update_flags & PARAM_FIRMWARE)  {
                    if(version==QIC_HWVERSION_QIC1806)
      ret = QicFlashRead(QIC1816_FLASH_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);
                    else if(version==QIC_HWVERSION_QIC1822)
      ret = QicFlashRead(QIC1822_FLASH_PARAM_ADDR, BufVerify, parm_image_max_size,image_max_size);
        else
            ret = QicFlashRead(FLASH_PARAM_ADDR, BufVerify, parm_image_max_size, image_max_size);
    LOG_PRINT(debug_str, 0, "Verify PARAM Sector Image\n");
    for (i = 0; i < parm_image_max_size; i++)
    {
      if (BufVerify[i] != new_parm_image[i])
      {
        ret = 1;
        parm_fail=1;
        LOG_PRINT(debug_str, DEBUG_ERROR, "PARAM Sector Image Verify Failed at %d\n", i);
      }
    }
    }

              if (update_flags & USB_FIRMWARE) {
      if(!usb_fail&&!isp_fail&&!parm_fail){
         printf("download ok\n");
        if(version==QIC_HWVERSION_QIC1806)
          ret = QicFlashWrite(QIC1816_FLASH_USB_ADDR, img_header, QIC_IMG_HEADER_SIZE, image_max_size);
        else if(version==QIC_HWVERSION_QIC1822){
          ret = QicFlashWrite(QIC1822_FLASH_USB_ADDR, img_header, QIC_IMG_HEADER_SIZE, image_max_size);
        }else
          ret = QicFlashWrite(FLASH_USB_ADDR, img_header,QIC_IMG_HEADER_SIZE,image_max_size);
                }else{
        printf("download error\n");
                                ret=1;
                }
              }


    if (ret) {
      free (BufVerify);
      goto ERR_CLOSE_MMAP;
    }

    free(BufVerify);
  /*release mem */
ERR_CLOSE_MMAP:
  if (new_isp_image != orig_isp_image) {
    munmap(new_isp_image, isp_img_size);
    close(isp_img_fd);
  }

  if (new_usb_image != orig_usb_image) {
    munmap(new_usb_image, usb_img_size);
    close(usb_img_fd);
  }
  if (new_parm_image != orig_parm_image) {
    munmap (new_parm_image, parm_img_size);
    close(parm_img_fd);
  }
#endif



ERR_CLOSE:
  free(orig_usb_image);
  free(orig_isp_image);
  free(orig_parm_image);
  free(orig_audio_image);
//  free(orig_osd_font_image);

  return ret;

}


/**********************************************
 *  runtime control functions
**********************************************/
unsigned int qic_get_yuyv_bad_frame_count(void)
{
  return yuyv_bad_frame_count;
}

unsigned int qic_get_vp8_bad_frame_count(void)
{
  return vp8_h264_bad_frame_count;
}

unsigned int qic_get_h264_bad_frame_count(void)
{
  return vp8_h264_bad_frame_count;
}


int qic_getframe(unsigned int dev_id) {

  unsigned int index;
  int i;
  int kl;
  int cur_fd = -1;
  int ret = 0;
  int demux_loop = 0, demux_ret=0;
  struct v4l2_buffer buf;
  char *video_buffer;
  unsigned int video_buffer_len;
//  struct timespec ts;

  /*LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_getframe - IN\n");*/
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
#if defined(QIC1816)&&defined(QIC_MPEGTS_API)
  /* demux parameters */
  H264DATAFORMAT *d_out;
#endif
//  MUTEX_ON(crit_mutex);

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) { /* check all nodes for active device */
    if ((dev_pt->cam[index].dev_id & dev_id)&&(dev_pt->cam[index].is_on)) { /* get a device , and is_on*/
      cur_fd = dev_pt->cam[index].fd;

      /* dequeue the buffer */
      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      if (-1 == xioctl(cur_fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {

          case EAGAIN:
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s getframe error, EAGAIN, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
            ret = 1;
            break;
          case EIO:
            /* Could ignore EIO, see spec. */
            break;
          default:
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s VIDIOC_DQBUF error, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));

        }
      }
      kl=buf.index;
  //    LOG_PRINT(debug_str,DEBUG_FRAME,"buf.index[%02d] length/%6d loc/%p\n", buf.index, buf.bytesused, dev_pt->cam[index].buffers[buf.index].start);

      /* sync data */
      msync(dev_pt->cam[index].buffers[buf.index].start, buf.bytesused, MS_SYNC);

      /* process the image */
      if ((dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG) &&(dev_pt->cam[index].is_demux)){ /* is AVC stream & need demux*/
        /* call demuxer __CPU CONSUMING!!__ */
#if defined(QIC1816)&&defined(QIC_MPEGTS_API)

          /*raw data dump*/
        if (dev_pt->cam[index].raw_dump)
          (*dev_pt->cam[index].raw_dump)(buf.bytesused, dev_pt->cam[index].buffers[buf.index].start);

        if(dev_pt->cam[index].stream_format==FORMAT_STREAM_H264_CONTAINER){
        demux_loop = demux_parse_TS(&dev_pt->cam[index].demux_struc.data, \
            dev_pt->cam[index].buffers[buf.index].start, \
            buf.bytesused, \
            dev_pt->cam[index].out);
        }
        else{
          #ifdef CQIC_MOUDLE1
          if( demux_check_format(&dev_pt->cam[index].demux_struc.data, \
            dev_pt->cam[index].buffers[buf.index].start, \
            buf.bytesused)==1){
            demux_loop = demux_parse_TS(&dev_pt->cam[index].demux_struc.data, \
            dev_pt->cam[index].buffers[buf.index].start, \
            buf.bytesused, \
            dev_pt->cam[index].out);
          }
          else
          {
          demux_loop = demux_parse_ES(&dev_pt->cam[index].demux_struc.data, \
            dev_pt->cam[index].buffers[buf.index].start, \
            buf.bytesused, \
            dev_pt->cam[index].out);
          }
          #else
          demux_loop = demux_parse_ES(&dev_pt->cam[index].demux_struc.data, \
            dev_pt->cam[index].buffers[buf.index].start, \
            buf.bytesused, \
            dev_pt->cam[index].out);

          #endif
        }

        /* check demux output  */
        if (0 > demux_loop) {
          LOG_PRINT(debug_str, (DEBUG_ERROR), "cam %s demuxer error(%d)\n", dev_pt->cam[index].dev_name, demux_loop);
          ret = 1;
          continue;
        }
#endif
        /* re-queue the buffer */
        if (-1 == xioctl(cur_fd, VIDIOC_QBUF, &buf)) {
          LOG_PRINT(debug_str,DEBUG_ERROR, "dev %s VIDIOC_QBUF error\n", dev_pt->cam[index].dev_name);
          ret = 1;
        }
#if defined(QIC1816)&&defined(QIC_MPEGTS_API)
        /* check bad frames for instant I-Frame */
        if (dev_pt->cam[index].cur_bad_count < dev_pt->cam[index].demux_struc.data.bad_count) {
          dev_pt->cam[index].cur_bad_count = dev_pt->cam[index].demux_struc.data.bad_count;
          qic_setIframe(dev_pt->cam[index].dev_id);
          LOG_PRINT(debug_str, DEBUG_FRAME, "dev %s qic_setIframe for bad frame\n",dev_pt->cam[index].dev_name);
        }

        /* output the frame(s) */
        for (i = 0; i < demux_loop; i++) {
          d_out = dev_pt->cam[index].out[i];

          /* NRI change for Skype */
          if (dev_pt->cam[index].nri_set && (d_out->data[4] == 0x09) &&(d_out->data[5] == 0x10)) {
            nri_change((unsigned char *)d_out->data, d_out->length, &dev_pt->cam[index]);
            dev_pt->cam[index].nri_set = 0;
            LOG_PRINT(debug_str, DEBUG_FRAME, "cam %s NRI change for Skype\n",dev_pt->cam[index].dev_name);
          }

          /*callback function*/
          (*dev_pt->frame_output)(dev_pt->cam[index].dev_id, d_out->length, (char *)d_out->data,(d_out->timestamp/10));


          /*
          (*dev_pt->frame_output)(dev_pt->cam[index].dev_id, \
            dev_pt->cam[index].out[i].length, \
            dev_pt->cam[index].out[i].data);
          */
          /* __IMPORTANT__ clear the demux buffer before access it????
          memset(dev_pt->cam[index].out[i], 0, sizeof(H264DATAFORMAT));
           */
        }
#endif
      }
      else { /* is YUV/MJPEG stream , or MPEG2TS*/
        msync(dev_pt->cam[index].buffers[buf.index].start, buf.bytesused, MS_SYNC);

        if(dev_pt->cam[index].is_demux){
          if (dev_pt->cam[index].format == V4L2_PIX_FMT_YUYV){
            //LOG_PRINT(debug_str, DEBUG_INFO, "dev %s yuyv_data_length=%d, buf.bytesused=%d...!!\n", dev_pt->cam[index].dev_name, yuyv_data_length, buf.bytesused);
            if(yuyv_data_length == buf.bytesused){
              (*dev_pt->frame_output)(dev_pt->cam[index].dev_id, buf.bytesused, dev_pt->cam[index].buffers[buf.index].start,(unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000)));
            }else{
              yuyv_bad_frame_count++;
              //Drop YUV Frame
              //LOG_PRINT(debug_str, DEBUG_ERROR, "Error!! dev %s YUYV data lost!! Must %d bytes, but received %d bytes...!!\n", dev_pt->cam[index].dev_name, yuyv_data_length, buf.bytesused);
            }
          }else if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG){
            video_buffer = dev_pt->cam[index].buffers[buf.index].start;
            video_buffer_len = buf.bytesused;

            //printf("\nvideo_buffer=0x%lx\n",video_buffer);
            //printf("&video_buffer=0x%lx\n",&video_buffer);
            if(dev_pt->cam[index].codec_type==CODEC_VP8){
              demux_ret = demux_VP8_check_bad_frame( &video_buffer, &video_buffer_len);
              if(!demux_ret){
                if(generate_key_frame){
                  if((video_buffer[3]&0x00ff)==0x9d && (video_buffer[4]&0x00ff)==0x01 && (video_buffer[5]&0x00ff)==0x2a){
                    generate_key_frame=0;
                    LOG_PRINT(debug_str, DEBUG_INFO, "dev %s Found VP8 IDR Frame......!!\n", dev_pt->cam[index].dev_name);
                  }else{
                    //LOG_PRINT(debug_str, DEBUG_INFO, "dev %s VP8 P Frame...!!\n", dev_pt->cam[index].dev_name);
                  }
                }
              }

            }else if(dev_pt->cam[index].codec_type==CODEC_H264){
              demux_ret = demux_H264_check_bad_frame( video_buffer, video_buffer_len);
              if(!demux_ret){
                if(generate_key_frame){
                  for(i=0;i<(video_buffer_len-4);i++){
                    if((video_buffer[i+0] == 0x00) && (video_buffer[i+1] == 0x00) && (video_buffer[i+2] == 0x00) && (video_buffer[i+3] == 0x01) && ((video_buffer[i+4]&0x1F) == 0x05)){
                      generate_key_frame=0;
                      LOG_PRINT(debug_str, DEBUG_INFO, "dev %s Found H264 IDR Frame i=%d......!!\n", dev_pt->cam[index].dev_name, i);
                      break;
                    }
                  }
                  //LOG_PRINT(debug_str, DEBUG_INFO, "dev %s H264 P Frame...!!\n", dev_pt->cam[index].dev_name);
                }
              }
            }

            //printf("generate_key_frame=%d, demux_ret=%d\n",generate_key_frame, demux_ret);
            //printf("video_buffer=0x%lx\n",video_buffer);
            if(demux_ret){
              LOG_PRINT(debug_str, DEBUG_INFO, "Error!! Bad Frame found: %s, qic generate key frame!!\n\n", dev_pt->cam[index].dev_name);
              qic_generate_key_frame_EU(dev_pt->cam[index].dev_id,SIMULCAST_STREAM0,1,dev_pt->cam[index].key_frame_interval, 0);
              generate_key_frame=1;
              vp8_h264_bad_frame_count++;
            }else{
              if(!generate_key_frame){
                (*dev_pt->frame_output)(dev_pt->cam[index].dev_id, video_buffer_len, video_buffer,(unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000)));
              }else{
                //Drop VP8/H264 P Frame
                LOG_PRINT(debug_str, DEBUG_INFO, "dev %s Drop P Frame...!!\n", dev_pt->cam[index].dev_name);

              }
            }
          }

        }else{
          (*dev_pt->frame_output)(dev_pt->cam[index].dev_id, buf.bytesused, dev_pt->cam[index].buffers[buf.index].start,(unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000)));
        }

        /* re-queue the buffer */
        if (-1 == xioctl(cur_fd, VIDIOC_QBUF, &buf)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev(YUV) %s VIDIOC_QBUF error: %s\n", dev_pt->cam[index].dev_name, strerror(errno));
          ret = 1;
        }
      }

    }

  }

//  MUTEX_OFF(crit_mutex);
  /*LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_getframe - OUT\n");*/
  /* success */
  return ret;
}


static void send_vp8_simulcast_frame(unsigned int dev_id, char *data, unsigned int length,
            unsigned long timestamp,unsigned int stream_id, unsigned short key_frame_interval,
            unsigned char demux)
{
  int demux_ret=0;
  out_frame_t sframe;

  /*check if we got bad frame??*/
     extern int last_frame_size;

if(demux){
        last_frame_size = last_frame_sizes[stream_id];

  demux_ret = demux_VP8_check_bad_frame( &data, &length);
  last_frame_sizes[stream_id] = last_frame_size;

  if(!demux_ret){
    if(generate_key_frame){
      if((data[3]&0x00ff)==0x9d && (data[4]&0x00ff)==0x01 && (data[5]&0x00ff)==0x2a){
        generate_key_frame=0;
        LOG_PRINT(debug_str, DEBUG_INFO, " Found VP8 IDR Frame......!!\n");
      }else{
        //LOG_PRINT(debug_str, DEBUG_INFO, "dev %s VP8 P Frame...!!\n", dev_pt->cam[index].dev_name);
      }
    }
  }

  /*this is bad frame, we drop it and request key frame at once */
  if(demux_ret){
    LOG_PRINT(debug_str, DEBUG_INFO, "Error!! Bad Frame found: stream id %d, qic generate key frame!!\n\n", stream_id);
    qic_generate_key_frame_EU(dev_id,stream_id,1,key_frame_interval, 0);
    generate_key_frame=1;
    vp8_h264_bad_frame_count++;
  }else{
      if(!generate_key_frame){

        /*frame is good send to user*/
        sframe.bencding_stream=1;
        sframe.stream_id=stream_id;
        sframe.frame_data=data;
        sframe.frame_len=length;
        sframe.timestamp=timestamp;
        (*dev_pt->frame_output2)(dev_id, sframe);
      }else{
        //Drop VP8 P Frame
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s Drop steamd %d bad Frame...!!\n", stream_id);

      }
  }
}else{
  sframe.stream_id=stream_id;
  sframe.bencding_stream=1;
  sframe.frame_data=data;
  sframe.frame_len=length;
  sframe.timestamp=timestamp;
  (*dev_pt->frame_output2)(dev_id, sframe);

}


}


static void send_avc_simulcast_frame(unsigned int dev_id, char *data, unsigned int length,
              unsigned long timestamp,unsigned int stream_id, unsigned short key_frame_interval,
              unsigned char demux)
{
  int demux_ret=0;
  int is_pframe,temporal_id;
       out_frame_t sframe;
  int i;
#ifdef QIC_MD_API
   unsigned int sei_begin_offset;
#endif
  /*check if we got bad frame??*/
     extern int last_frame_size;

if(demux){
        last_frame_size = last_frame_sizes[stream_id];

  demux_ret = demux_H264_check_bad_frame( data, length);
  last_frame_sizes[stream_id] = last_frame_size;
    if(!demux_ret){
      if(generate_key_frame){
        for(i=0;i<(length-4);i++){
          if((data[i+0] == 0x00) && (data[i+1] == 0x00) && (data[i+2] == 0x00) && (data[i+3] == 0x01) && ((data[i+4]&0x1F) == 0x05)){
            generate_key_frame=0;
            LOG_PRINT(debug_str, DEBUG_INFO, "stream %d Found H264 IDR Frame i=%d......!!\n",stream_id, i);
            break;
          }
        }
        //LOG_PRINT(debug_str, DEBUG_INFO, "dev %s H264 P Frame...!!\n", dev_pt->cam[index].dev_name);
      }
    }


  /*this is bad frame, we drop it and request key frame at once */
  if(demux_ret){
    LOG_PRINT(debug_str, DEBUG_INFO, "Error!! Bad Frame found: stream id %d, qic generate key frame!!\n\n", stream_id);
    qic_generate_key_frame_EU(dev_id,stream_id,1,key_frame_interval, 0);
    generate_key_frame=1;
    vp8_h264_bad_frame_count++;
  }else{
      if(!generate_key_frame){
        is_pframe=check_for_P_frame((unsigned char*)data,length);
        sframe.bPframe=is_pframe;

        sframe.temporal_layer_id=get_stream_temporal_id((unsigned char*)data,length);
          //  printf("temp id=%d\n",sframe.temporal_layer_id);


#ifdef QIC_MD_API
        if(stream_id==STREAM0)
        AnalyzeSEIMessages((unsigned char*)data,length,&sei_begin_offset,&sframe.md_status);

#endif
        /*frame is good send to user*/
        sframe.bencding_stream=1;
        sframe.stream_id=stream_id;
        sframe.frame_data=data;
        sframe.frame_len=length;
        sframe.timestamp=timestamp;
        (*dev_pt->frame_output2)(dev_id, sframe);
      }else{
        //Drop bad Frame
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s Drop steamd %d bad Frame...!!\n", stream_id);

      }
  }

}else{

#ifdef QIC_MD_API
   memset(&sframe.md_status,0, sizeof(md_status_t));
  if(stream_id==STREAM0)
    AnalyzeSEIMessages((unsigned char*)data,length,&sei_begin_offset,&sframe.md_status);

#endif
  is_pframe=check_for_P_frame((unsigned char*)data,length);
  sframe.bPframe=is_pframe;

  sframe.temporal_layer_id=get_stream_temporal_id((unsigned char*)data,length);

  sframe.stream_id=stream_id;
  sframe.bencding_stream=1;
  sframe.frame_data=data;
  sframe.frame_len=length;
  sframe.timestamp=timestamp;
  (*dev_pt->frame_output2)(dev_id, sframe);

}

}


int qic_getframe2(unsigned int dev_id) {

  unsigned int index;
  int i;
  int kl;
  int cur_fd = -1;
  int ret = 0;
  int demux_loop = 0, demux_ret=0;
  struct v4l2_buffer buf;
  char *video_buffer;
  unsigned int video_buffer_len;
  unsigned short timestamp=0;
  int avc_stream_id;
  out_frame_t sframe;
//  struct timespec ts;

  /*LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_getframe - IN\n");*/
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
//  MUTEX_ON(crit_mutex);

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) { /* check all nodes for active device */
    if ((dev_pt->cam[index].dev_id & dev_id)&&(dev_pt->cam[index].is_on)) { /* get a device , and is_on*/
      cur_fd = dev_pt->cam[index].fd;

      /* dequeue the buffer */
      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      if (-1 == xioctl(cur_fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {

          case EAGAIN:
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s getframe error, EAGAIN, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
            ret = 1;
            break;
          case EIO:
            /* Could ignore EIO, see spec. */
            break;
          default:
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s VIDIOC_DQBUF error, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));

        }
      }
      kl=buf.index;

      /* sync data */

        msync(dev_pt->cam[index].buffers[buf.index].start, buf.bytesused, MS_SYNC);

          if (dev_pt->cam[index].format == V4L2_PIX_FMT_YUYV){

                  sframe.bencding_stream=0;
                  sframe.stream_id=0;
                  sframe.frame_data=dev_pt->cam[index].buffers[buf.index].start;
                  sframe.frame_len=buf.bytesused;
                  sframe.timestamp=(unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000));
                (*dev_pt->frame_output2)(dev_id, sframe);

          }else if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG){
            video_buffer = dev_pt->cam[index].buffers[buf.index].start;
            video_buffer_len = buf.bytesused;
            timestamp= (unsigned long)((buf.timestamp.tv_sec*1000)+(buf.timestamp.tv_usec/1000));

            if(dev_pt->cam[index].codec_type==CODEC_VP8){
              send_vp8_simulcast_frame(dev_pt->cam[index].dev_id,video_buffer, video_buffer_len, timestamp,STREAM0, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);

            }else if(dev_pt->cam[index].codec_type==CODEC_H264){

              send_avc_simulcast_frame(dev_pt->cam[index].dev_id,video_buffer, video_buffer_len, timestamp,STREAM0, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
            }else if(dev_pt->cam[index].codec_type==CODEC_VP8_SIMULCAST){

               if(vp8demux_ex((BYTE*)video_buffer,video_buffer_len,&vp8_frames))
                        {
                            if(vp8_frames.frames[STREAM0].size!=0){
                  send_vp8_simulcast_frame(dev_pt->cam[index].dev_id,(char*) vp8_frames.frames[STREAM0].data, vp8_frames.frames[STREAM0].size, timestamp,STREAM0, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
                            }if(vp8_frames.frames[STREAM1].size!=0){
                  send_vp8_simulcast_frame(dev_pt->cam[index].dev_id, (char*) vp8_frames.frames[STREAM1].data, vp8_frames.frames[STREAM1].size, timestamp,STREAM1, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
                          }if(vp8_frames.frames[STREAM2].size!=0){
                  send_vp8_simulcast_frame(dev_pt->cam[index].dev_id, (char*) vp8_frames.frames[STREAM2].data, vp8_frames.frames[STREAM2].size, timestamp,STREAM2, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
                            }if(vp8_frames.frames[STREAM3].size!=0){
                  send_vp8_simulcast_frame(dev_pt->cam[index].dev_id, (char*) vp8_frames.frames[STREAM3].data, vp8_frames.frames[STREAM3].size, timestamp,STREAM3, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
                            }
              }else{
                           LOG_PRINT(debug_str, DEBUG_ERROR, "vp8demux return error !!\n");
                        }

            }else if(dev_pt->cam[index].codec_type==CODEC_H264_SIMULCAST){
              avc_stream_id=get_avc_stream_id((unsigned char*)video_buffer,video_buffer_len);
              send_avc_simulcast_frame(dev_pt->cam[index].dev_id,video_buffer, video_buffer_len, timestamp,avc_stream_id, dev_pt->cam[index].key_frame_interval,dev_pt->cam[index].is_demux);
            }else{
              sframe.bencding_stream=0;
              sframe.stream_id=0;
              sframe.frame_data=video_buffer;
              sframe.frame_len=video_buffer_len;
              sframe.timestamp=timestamp;
              (*dev_pt->frame_output2)(dev_id, sframe);


            }

          }


        /* re-queue the buffer */
        if (-1 == xioctl(cur_fd, VIDIOC_QBUF, &buf)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev(YUV) %s VIDIOC_QBUF error: %s\n", dev_pt->cam[index].dev_name, strerror(errno));
          ret = 1;
        }
      }

    }


//  MUTEX_OFF(crit_mutex);
  /*LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_getframe - OUT\n");*/
  /* success */
  return ret;
}


int qic_change_flip(unsigned int dev_id, unsigned char flip) {

  unsigned int index;
  int ret = 0;
  unsigned char h_flip, v_flip;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      {

        h_flip = flip & H_FLIP;
        v_flip = (flip & V_FLIP)>>1;

        QicChangeFD(dev_pt->cam[index].fd);
        qic_ret = QicSetFlipMode(v_flip, h_flip);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
        h_flip = 0;
        v_flip = 0;
        qic_ret = QicGetFlipMode(&v_flip, &h_flip);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetFlipMode success,  H_FLIP=%s, V_FLIP=%s\n", h_flip?"on":"off", v_flip?"on":"off");
#endif
      }


      }

    }

  /* success */
  return ret;

}

int qic_change_LED_Mode( unsigned char mode) {

  int ret = 0;
  unsigned char led_mode;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicSetLedMode(mode);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
        led_mode = 0;
        qic_ret = QicGetLedMode(&led_mode);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetLedMode success,  LED_Mode=%d, \n", led_mode);
#endif

  /* success */
  return ret;

}


int qic_change_LED_Brightness( unsigned char brightness) {

  int ret = 0;
  unsigned char led_brightness;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


        qic_ret =  QicSetLEDBrightness(brightness);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
        led_brightness = 0;
        qic_ret = QicGetLEDBrightness(&led_brightness);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetLEDBrightness success,  LED_brightness=%d, \n", led_brightness);
#endif

  /* success */
  return ret;

}



int qic_change_Shutter_Mode( unsigned char mode) {

  int ret = 0;
  unsigned char shutter_mode;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicSetShutterMode(mode);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetShutterMode success\n");

#ifdef DEBUG_LOG
        shutter_mode = 0;
        qic_ret = QicGetShutterMode(&shutter_mode);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetShutterMode success,  Shutter_Mode=%d, \n", shutter_mode);
#endif

  /* success */
  return ret;

}

/* MJPEG QP control */
int qic_get_MJPEG_QP_Value(unsigned char *value)
{
  int qic_ret;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }

  qic_ret = QicGet_MJPEG_QP_Value(value);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGet_MJPEG_QP_Value success, MJPEG_QP_Value=%d ...\n", *value);

  return qic_ret;
}

int qic_change_MJPEG_QP_Value(unsigned char value)
{
  unsigned char retValue=0;
  int qic_ret;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }

  if(value==0 || value>100)
  {
    LOG_PRINT(debug_str, DEBUG_INFO, "Error!! MJPEG QP value over range=%d, correct rage 1~100...\n",value);
    return 1;
  }

  qic_ret = QicSet_MJPEG_QP_Value(value);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSet_MJPEG_QP_Value success...\n");

#ifdef DEBUG_LOG
  qic_ret = QicGet_MJPEG_QP_Value(&retValue);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGet_MJPEG_QP_Value success, MJPEG_QP_Value=%d ...\n", retValue);
#endif

  return qic_ret;
}

#if defined(QIC_MPEGTS_API)
int qic_setIframe(unsigned int dev_id){
  unsigned int index;
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG||
        dev_pt->cam[index].is_encoding_video){ /* H.264, XUCTRL */
        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicEncoderSetIFrame();
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
#if defined(QIC1802)
        dev_pt->cam[index].nri_set = 1;
#endif
      }else{
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't encoding device, can't set trimming\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }
    }

  }

  /* success */
  return ret;

}


int qic_get_number_of_profiles(unsigned int dev_id, int *num) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicEncoderGetNumberOfProfiles(num);
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s: Number of Profiles = %d\n", dev_pt->cam[index].dev_name, *num);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

    }
  }

  /* success */
  return ret;

}


int qic_get_profile(unsigned int dev_id, int idx, int *max_level, int *profile, int *constraint_flags) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicEncoderGetProfile(idx, max_level, profile, constraint_flags);
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s: Profile =  %d, Max Level = %d, Constraint Flags = %d\n", dev_pt->cam[index].dev_name, *profile, *max_level, *constraint_flags);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

    }
  }

  /* success */
  return ret;

}

int qic_set_profile_and_level(unsigned int dev_id, int level, int profile, int constraint_flags) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {


        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicEncoderSetProfileAndLevel(level, profile, constraint_flags);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

    }
  }

  /* success */
  return ret;

}


int qic_get_profile_and_level(unsigned int dev_id, int *level, int *profile, int *constraint_flags) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {


        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicEncoderGetProfileAndLevel(level, profile, constraint_flags);
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s: Profile =  %d, Level = %d, Constraint Flags = %d\n", dev_pt->cam[index].dev_name, *profile, *level, *constraint_flags);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

    }
  }

  /* success */
  return ret;

}



int qic_change_gop(unsigned int dev_id, int gop) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  int gop_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        if (gop >= 0) {

          dev_pt->cam[index].gop = gop;

          QicChangeFD(dev_pt->cam[index].fd);
          ret = QicEncoderSetGOP(gop);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
#ifdef DEBUG_LOG
          /*TIME_DELAY(1);*/
          qic_ret = QicEncoderGetGOP(&gop_ret);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "gop = %d\n", gop_ret);
#endif


        }
        else { /* error format */

          LOG_PRINT(debug_str, DEBUG_ERROR, "GOP(%d) must larger than 0\n", gop);
          ret = 1;
          continue;
        }

    }

  }

  /* success */
  return ret;

}



int qic_change_slicesize(unsigned int dev_id, unsigned char slicesize) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        if (slicesize >= 0) {

          if ((dev_pt->cam[index].width == 1280) && (dev_pt->cam[index].height == 720)) { /* HD format, slicesize __MUST BE ZERO__ */

            LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s is in HD format, slicesize must be zero\n", dev_pt->cam[index].dev_name);
            continue;
          }

          QicChangeFD(dev_pt->cam[index].fd);

          EncoderParams_t tmp;
          CLEAR(tmp);
          /*
          tmp.uiBitrate = dev_pt->cam[index].bitrate;
          tmp.usWidth = dev_pt->cam[index].width;
          tmp.usHeight = dev_pt->cam[index].height;
          tmp.ucFramerate = dev_pt->cam[index].framerate;
          */
          tmp.ucSlicesize = dev_pt->cam[index].slicesize;

          ret = QicEncoderSetParams (&tmp, QIC_XU1_ENCODER_SLICESIZE);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
          CLEAR(tmp);
          qic_ret = QicEncoderGetParams(&tmp);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "slicesize = %d\n", tmp.ucSlicesize);

#endif
          dev_pt->cam[index].slicesize= slicesize;
        }
        else { /* error format */

          LOG_PRINT(debug_str, DEBUG_ERROR, "slicesize is incorrect\n");
          ret = 1;
          continue;
        }



    }

  }

  /* success */
  return ret;

}

int qic_change_framerate(unsigned int dev_id, unsigned char framerate) {

  unsigned int index;
  int is_valid;
  struct v4l2_streamparm setfps;
  int ret = 0;
  int qic_ret = 0;
  QicHwVer_t version;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG||
        dev_pt->cam[index].is_encoding_video){ /* H.264, XUCTRL */

          QicChangeFD(dev_pt->cam[index].fd);

          EncoderParams_t tmp;
          CLEAR(tmp);

          QicEncoderGetParams(&tmp);

          /* check if valid*/

          QicGetHWVersion(&version);
        if(version!=QIC_HWVERSION_QIC1802){
          if (1<=framerate && framerate <= 30)
            is_valid = format_check((SUPPORT_FORMAT *)AVC_format,dev_pt->cam[index].width,dev_pt->cam[index].height,0);
          else
            is_valid=0;

        }else{

          if (1<=framerate && framerate <= 30)
          {


#if defined(QIC1802)

            if ((dev_pt->cam[index].width > 1280) || (dev_pt->cam[index].height > 720)){
              is_valid = 0;
                        }else{
              if ((dev_pt->cam[index].width == 1280) && (dev_pt->cam[index].height == 720)){
                if(framerate>22){
                     framerate=22;

                }
            is_valid = 1;
                                             }
              else
                is_valid = format_check((SUPPORT_FORMAT *)AVC_format,dev_pt->cam[index].width,dev_pt->cam[index].height,0);
            }
#endif

            is_valid = 1;

          }
          else
            is_valid = 0;
        }

          if (is_valid) {

            /*
            tmp.uiBitrate = dev_pt->cam[index].bitrate;
            tmp.usWidth = dev_pt->cam[index].width;
            tmp.usHeight = dev_pt->cam[index].height;
            */
            tmp.ucFramerate = framerate;
            /*
            tmp.ucSlicesize = dev_pt->cam[index].slicesize;
            */

            ret = QicEncoderSetParams (&tmp, QIC_XU1_ENCODER_FRAMERATE);
            LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
#ifdef DEBUG_LOG
            /*TIME_DELAY(1);*/
            CLEAR(tmp);
            qic_ret = QicEncoderGetParams(&tmp);
            LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
            if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "%dx%d-%dfps, bitrate%d\n", tmp.usWidth, tmp.usHeight, tmp.ucFramerate, tmp.uiBitrate);
#endif
            dev_pt->cam[index].framerate = framerate;

          }
          else {
            LOG_PRINT(debug_str, DEBUG_ERROR, "framerate %dfps not supported\n", framerate);
            ret = 1;
            continue;

          }

      }
      else { /* workable only if is_bind = 0 in YUV */
        if (dev_pt->cam[index].is_bind == 0) { /* not bind */
          is_valid = format_check((SUPPORT_FORMAT *) YUV_format, dev_pt->cam[index].width, dev_pt->cam[index].height,framerate);

          if (is_valid) {

            MUTEX_ON(crit_mutex); /* _not check_ change framerate between dequeue */
            /* set the framerate */
            CLEAR(setfps);
            setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            setfps.parm.capture.timeperframe.numerator = 1;
            setfps.parm.capture.timeperframe.denominator = framerate;
            if(-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_S_PARM, &setfps)) {
              LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s unable to set frame rate, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));

            } else {
              LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s frame rate set to %d\n", dev_pt->cam[index].dev_name, framerate);
            }
            /* __MEMO__: can't read fps from qic_module...
            CLEAR(setfps);
            if(0 == xioctl(dev_pt->cam[index].fd, VIDIOC_G_PARM, &setfps)) {
                if (setfps.parm.capture.timeperframe.numerator != 1 ||
                    setfps.parm.capture.timeperframe.denominator != framerate) {

                  LOG_PRINT(debug_str, 1, "%s Frame rate: %u/%u fps (requested frame rate %u fps is not supported by device)\n", \
                    dev_pt->cam[index].dev_name, \
                    setfps.parm.capture.timeperframe.denominator, \
                    setfps.parm.capture.timeperframe.numerator,  \
                    framerate);

                }
                else {
                  dev_pt->cam[index].framerate = framerate;
                  LOG_PRINT(debug_str, 0, "%s framerate: %d fps\n", dev_pt->cam[index].dev_name, dev_pt->cam[index].framerate);
                }
            }
            else {
              LOG_PRINT(debug_str, 2, "%s unable to read out current frame rate\n", dev_pt->cam[index].dev_name);
              ret = 1;
              continue;
            }
            */

            MUTEX_OFF(crit_mutex);
          }
          else {
            LOG_PRINT(debug_str,DEBUG_ERROR,"framerate (%dfps) setting is not valid, aborted\n", framerate);
            ret = 1;
            continue;
          }

        }
        else { /* no can do */

          LOG_PRINT(debug_str, DEBUG_ERROR, "Binded device %s can't set framerate, please set the master device\n", dev_pt->cam[index].dev_name);
          ret = 1;
          continue;
        }
      }

    }

  }

  /* success */
  return ret;

}

int qic_change_streamformat(unsigned int dev_id, EncoderStreamFormat_t mode)
{
  unsigned int index;
  int ret = 0;
  EncoderStreamFormat_t format;
  int qic_ret = 0;
  unsigned char fw_api_verson;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {


      QicChangeFD(dev_pt->cam[index].fd);

      SetH264format(mode);
      qic_ret=QicGetFwAPIVersion (&fw_api_verson);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "fw api version=%d\n", fw_api_verson);

      if(mode==FORMAT_STREAM_H264_CONTAINER){
        format=mode;
      }
      else
      {
          if(fw_api_verson>=4)
            format=mode;
          else
            format=FORMAT_STREAM_H264_RAW;

      }

      ret = QicSetStreamFormat( format);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "stream format=0x%x\n", format);
#ifdef DEBUG_LOG
      format=0x00;
      qic_ret = QicGetStreamFormat(&format);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "stream format=0x%x\n", format);

#endif


    }

  }

  /* success */


  return ret;
}







int qic_change_stream_size_format(unsigned int dev_id, EncoderStreamSizeFormat_t  mode)
{
  unsigned int index;
  int ret = 0;
  EncoderStreamSizeFormat_t format;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

          QicChangeFD(dev_pt->cam[index].fd);

          format=mode;

          ret = QicSetEncoderFrameSizeFormat( format);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "stream format size=0x%x\n", format);
#ifdef DEBUG_LOG
          qic_ret = QicGetEncoderFrameSizeFormat(&format);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "Encoder Frame Size =0x%x\n", format);

#endif


    }

  }

  /* success */


  return ret;
}


int qic_change_rate_control_mode(int dev_id, EncoderRateControlMode_t rate_control_type, unsigned char min_QP_value, unsigned char max_QP_value)
{
  int qic_ret = 0;
  int ret=0;
  unsigned int index;
        EncoderRateControlMode_t got_rate_control_type;
        unsigned char got_min_QP_value;
        unsigned char got_max_QP_value;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  for (index = 0; index < dev_pt->num_devices; index++) {
      if(dev_pt->cam[index].dev_id & dev_id){

        QicChangeFD(dev_pt->cam[index].fd);
        ret = QicVideoSetRateControl(rate_control_type, min_QP_value, max_QP_value);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicVideoSetRateControl success,  type=%d, min QP=%d, max QP=%d\n", rate_control_type, min_QP_value, max_QP_value);

#ifdef DEBUG_LOG
        qic_ret = QicVideoGetRateControl(&got_rate_control_type, &got_min_QP_value, &got_max_QP_value);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicVideoGetRateControl success,  type=%d, min QP=%d, max QP=%d\n", got_rate_control_type, got_min_QP_value, got_max_QP_value);

#endif

          }
  }
  /* success */
  return qic_ret;
}


int qic_change_bitrate(unsigned int dev_id, unsigned int bitrate) {

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {


        if(bitrate <= 16000000){

          QicChangeFD(dev_pt->cam[index].fd);
          /*TIME_DELAY(1);*/

          EncoderParams_t tmp;
          CLEAR(tmp);

          tmp.uiBitrate = bitrate;


          ret = QicEncoderSetParams (&tmp, QIC_XU1_ENCODER_BITRATE);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
#ifdef DEBUG_LOG
          /*TIME_DELAY(1);*/
          CLEAR(tmp);
          qic_ret = QicEncoderGetParams(&tmp);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "%dx%d-%dfps, bitrate%d\n", tmp.usWidth, tmp.usHeight, tmp.ucFramerate, tmp.uiBitrate);

#endif
          dev_pt->cam[index].bitrate = bitrate;

        }
        else { /* error format */

          LOG_PRINT(debug_str, DEBUG_ERROR, "bitrate too large (%d), can't set bitrate\n", bitrate);
          ret = 1;
          continue;
        }


      }


    }

  /* success */
  return ret;

}


#endif

int qic_change_pan_tilt(unsigned int dev_id, signed short pan, signed short tilt)
{
        unsigned int index;
        int ret = 0;
        int qic_ret = 0;
//  unsigned char fw_api_verson;
#ifdef DEBUG_LOG
        signed short got_pan, got_tilt;
#endif

#ifdef COMMIT_CHECK
        /* check if committed */
        if ((!config_is_commit) &&(dev_pt == NULL)) {
                LOG_PRINT(debug_str, DEBUG_ERROR, "QIC library config is not committed\n");
                return 1;
        }
#endif
/*
        if(10 <pan||-10> pan)
        {
            LOG_PRINT(debug_str, DEBUG_ERROR, "Pan out of range!!\n");
         return -1;
        }
        if(10 <tilt||-10> tilt)
        {
            LOG_PRINT(debug_str, DEBUG_ERROR, "Tilt out of range!!\n");
       return -1;
        }
*/
        if(pan<0)
        {
          pan=(~pan)+0x8001;
        }

        if(tilt<0)
        {
          tilt=(~tilt)+0x8001;
        }
/* device array loop */
        for (index = 0; index < dev_pt->num_devices; index++) {
                if (dev_pt->cam[index].dev_id & dev_id) {

                        QicChangeFD(dev_pt->cam[index].fd);

                        qic_ret = QicSetPanTilt( pan,tilt);
                        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
                        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "Set: Pan=%x, Tilt=%x\n", pan,tilt);
#ifdef DEBUG_LOG

                        qic_ret = QicGetPanTilt(&got_pan,&got_tilt);
                        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
                        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "Get: Pan=%x, Tilt=%x\n", got_pan,got_tilt);
#endif
                }

        }

        /* success */

        return ret;
}


int qic_set_cropped_image(unsigned int dev_id,unsigned int crop_switch,CROPRECT crop_window)
{
  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  CROPRECT window;
//  int boundary_x, boundary_y;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].format == V4L2_PIX_FMT_YUYV) {

                                /*QIC1816 can't support width of cropped window over 360 */
        if(crop_window.Wx > 360 && crop_switch ){
          ret=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "out of range, can't set cropped image\n");
          goto ERR_SETTING;
        }

        QicChangeFD(dev_pt->cam[index].fd);

        if(crop_switch) {
          qic_ret = QicSetCropped(1, crop_window);
        }else {
          window.x=0;
          window.y=0;
          window.Wx=0;
          window.Wy=0;
          qic_ret = QicSetCropped(0, window);
        }
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "corpped enable= %d\n", crop_switch);

        if(qic_ret) {
          ret=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_cropped_image failed!!\n");
        }
      }
      else { /* N/A */
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't Preview device, can't set cropped image\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }

    }

  }

  /* success */

ERR_SETTING:


  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}


int qic_set_trimming(unsigned int dev_id,unsigned int trim_enable,unsigned int x, unsigned y)
{
  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  TRIMRECT window;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG||
                        dev_pt->cam[index].is_encoding_video==1
      ) {

                          /*so far QIC1816 only support HD trimming */
        if(dev_pt->cam[index].width!=1280&&dev_pt->cam[index].height!=720){
          ret=1;
          LOG_PRINT(debug_str, DEBUG_INFO,"%dx%d\n",dev_pt->cam[index].width,dev_pt->cam[index].height);
          LOG_PRINT(debug_str, DEBUG_ERROR, "out of range, only support HD resolution\n");
          goto ERR_SETTING;
        }

        QicChangeFD(dev_pt->cam[index].fd);

        if(trim_enable) {
          /*so far QIC1816 only support HD trimming */
            window.x=x;
            window.y=y;
                      window.Wx=1280;
            window.Wy=720;
             qic_ret = QicSetTrimming(1, window);
        }else {
          /*disable trimming, set to default value */
          qic_ret = QicSetTrimming(0, window);
        }

          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(qic_ret) {
          ret=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_trimming failed!!\n");
        }
      }
      else { /* N/A */
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't encoding device, can't set trimming\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }

    }

  }

  /* success */

ERR_SETTING:
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}



int qic_set_trim_zoom(unsigned int dev_id,unsigned int x, unsigned int y, unsigned int wdith, unsigned int height, unsigned char enable)
{
  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  TRIMRECT window;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        QicChangeFD(dev_pt->cam[index].fd);

            window.x=x;
            window.y=y;
            window.Wx=wdith;
             window.Wy=height;
             if(enable)
              qic_ret = QicSetTrimming(1, window); //enable trim zoom function
             else
            qic_ret = QicSetTrimming(0, window); //disable trim zoom function

          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(qic_ret) {
          ret=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_trim_zoom failed!!\n");
        }


    }

  }

  /* success */
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}


int qic_get_trim_zoom(unsigned int dev_id,unsigned int* x, unsigned int* y, unsigned int* wdith, unsigned int* height, unsigned char* enable)
{
  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  unsigned char enable_trim;
  TRIMRECT window;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

/* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        QicChangeFD(dev_pt->cam[index].fd);

           qic_ret = QicGetTrimming(&enable_trim, &window);

        if(qic_ret) {
          ret=1;
          LOG_PRINT(debug_str, DEBUG_ERROR, "qic_get_trim_zoom failed!!\n");
        }

        printf("get trim==> enable=%d, x=%d, y=%d, Wx=%d, Wy=%d\n",enable_trim,window.x,window.y,window.Wx,window.Wy);

        *enable=enable_trim;
        *x=window.x;
        *y=window.y;
        *wdith=window.Wx;
        *height=window.Wy;
    }

  }

  /* success */
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}


int qic_get_ALS(unsigned int dev_id, unsigned short *ALS){
  int qic_ret=0;
  unsigned int index=0;

#ifdef COMMIT_CHECK
    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
      return 1;
    }
#endif

    /* device array loop */
    for (index = 0; index < dev_pt->num_devices; index++) {
      if (dev_pt->cam[index].dev_id & dev_id) {
          QicChangeFD(dev_pt->cam[index].fd);
        qic_ret=QicGetALS(ALS);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetALS success\n");
        }
      }


  return qic_ret;



}

int qic_get_Capability_List( unsigned char *maximun_number, unsigned int *support_list){
  int qic_ret=0;
  unsigned int index=0;

#ifdef COMMIT_CHECK
    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
      return 1;
    }
#endif

    qic_ret=QicGetCapabilityList(maximun_number, support_list);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetCapabilityList success\n");

    printf("maximun support number=%d, support list=%d\n",*maximun_number, *support_list);
    printf("ALS=%d, IR=%d, OSD=%d, PT=%d, Simulcast=%d\n",*support_list&0x0001, (*support_list>>1)&0x0001,(*support_list>>2)&0x0001,(*support_list>>3)&0x0001,(*support_list>>4)&0x0001);

  return qic_ret;



}


int qic_change_motor_control( unsigned char control, unsigned char direction) {

  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicSetMotorControl(control, direction);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_change_motor_control success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_change_motor_control failed\n");
        qic_ret=1;
          }

  return qic_ret;

}



int qic_get_motor_status( unsigned char *motor_status) {

  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicGetMotorStatus(motor_status);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_motor_status success motor status=%d\n", *motor_status);
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_motor_status failed\n");
        qic_ret=1;
          }

  return qic_ret;

}



int qic_set_WUSB_wired(void)
{
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  qic_ret = QicSetWUSBWired();

  if(qic_ret) {
    ret=1;
    LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_WUSB_wired failed!!\n");
  }
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}

int qic_set_WUSB_wireless(void)
{
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  qic_ret = QicSetWUSBWireless();

  if(qic_ret) {
    ret=1;
    LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_WUSB_wireless failed!!\n");
  }
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}

int qic_set_WUSB_disable_auto_switch(void)
{
  int ret = 0;
  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  qic_ret = QicSetWUSBDisableAutoSwitch();

  if(qic_ret) {
    ret=1;
    LOG_PRINT(debug_str, DEBUG_ERROR, "qic_set_WUSB_disable_auto_switch failed!!\n");
  }
  LOG_PRINT(debug_str, DEBUG_INFO,"ret=%d\n",ret);
  return ret;
}


int qic_set_lock_steam_control( unsigned char lock) {

  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicSetLockStream(lock);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_Set_lock_steam_control success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_Set_lock_steam_control failed\n");
        qic_ret=1;
          }


  return qic_ret;

}


int qic_check_lock_stream_status( unsigned char *isLock, unsigned char *isStream) {

  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
        qic_ret = QicGetCheckLockStreamStatus(isLock, isStream);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_check_lock_stream_status => lock %d, stream=%d\n", *isLock, *isStream);
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_check_lock_stream_status failed\n");
        qic_ret=1;
          }

  return qic_ret;

}




int qic_get_Sensor_AEInfo(unsigned short *AE_statistic){
  int qic_ret=0;
  unsigned int index=0;

#ifdef COMMIT_CHECK
    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
      return 1;
    }
#endif

        qic_ret=  QicGetSensorAEInfo(AE_statistic);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetSensorAEInfo success\n");

  return qic_ret;



}


int qic_change_IR_control( unsigned char mode) {

  int qic_ret = 0;
  unsigned char ir_status, als_status;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicSetIRControl(mode);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_change_IR_control success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_change_IR_control failed\n");
        qic_ret=1;
          }

       QicGetIRStatus(&ir_status,&als_status);
        LOG_PRINT(debug_str, DEBUG_INFO, "IR status=%d, ALS status=%d\n",ir_status, als_status );

  return qic_ret;

}


int qic_get_IR_ALS_status( unsigned char *ir_status, unsigned char *ALS_status) {

  int qic_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        qic_ret = QicGetIRStatus(ir_status, ALS_status);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_IR_status success motor status=%d\n", *ir_status);
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_IR_status failed\n");
        qic_ret=1;
          }

  return qic_ret;

}




int qic_set_advance_motor_postion_single( MotorPosition_t Pos ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        PosSetting.PosArray[0].pan=Pos.pan;
        PosSetting.PosArray[0].tilt=Pos.tilt;
        PosSetting.TotalPositionNumbers=1;
        PosSetting.StartElementIndex=1;
        PosSetting.PosArray[0].pan=Pos.pan;
        PosSetting.PosArray[0].tilt=Pos.tilt;

        qic_ret =QicSetAdvMotorControl(ADV_MOTOR_SINGLE_SET,1,PosSetting);

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_postion_singl success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_postion_singl failed\n");
        qic_ret=1;
          }


  return qic_ret;

}


int qic_start_advance_motor( void ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        PosSetting.TotalPositionNumbers=0;
        PosSetting.StartElementIndex=1;

        qic_ret =QicSetAdvMotorControl(ADV_MOTOR_NAVIGATE_SET,0,PosSetting);

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_start_advance_motor success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_start_advance_motor failed\n");
        qic_ret=1;
          }


  return qic_ret;

}

int qic_stop_advance_motor( void ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        PosSetting.TotalPositionNumbers=0;
        PosSetting.StartElementIndex=1;
        PosSetting.PosArray[0].pan=0;
        PosSetting.PosArray[0].tilt=0;

        qic_ret =QicSetAdvMotorControl(ADV_MOTOR_STOP,0,PosSetting);

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_stop_advance_motor success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_stop_advance_motor failed\n");
        qic_ret=1;
          }


  return qic_ret;

}

int qic_reset_position_advance_motor( void ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        PosSetting.TotalPositionNumbers=0;
        PosSetting.StartElementIndex=1;

        qic_ret =QicSetAdvMotorControl(ADV_MOTOR_RESET,0,PosSetting);

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_reset_position_advance_motor success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_reset_position_advance_motor failed\n");
        qic_ret=1;
          }


  return qic_ret;

}


int qic_set_advance_motor_speed( unsigned short pan_speed, unsigned short tilt_speed ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

        PosSetting.StartElementIndex=1;
        PosSetting.TotalPositionNumbers=1;
        PosSetting.PosArray[0].pan=pan_speed;
        PosSetting.PosArray[0].tilt=tilt_speed;

        qic_ret =QicSetAdvMotorControl(ADV_MOTOR_SPEED,1,PosSetting);

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_speed success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_speed failed\n");
        qic_ret=1;
          }


  return qic_ret;

}


int qic_get_advance_motor_current_postion( MotorPosition_t *Pos ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


        qic_ret=QicGetAdvMotorPosition(Pos);


        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_advance_motor_current_postion success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_advance_motor_current_postion failed\n");
        qic_ret=1;
          }


  return qic_ret;

}


int qic_set_advance_motor_postions( unsigned char set_numbers,MotorPosition_t Pos[] ) {

  int qic_ret = 0;
  AdvMotorCtl_t PosSetting;
  unsigned char i=0;
  unsigned char temp_number,temp_start_index;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


        if(set_numbers<=5){
          PosSetting.TotalPositionNumbers=set_numbers;
          PosSetting.StartElementIndex=1;

          memcpy(&PosSetting.PosArray[0],Pos,sizeof(MotorPosition_t)*set_numbers);
          printf("0: %d, %d, 1:%d, %d, 2:%d, %d,3:%d, %d,4:%d, %d,",PosSetting.PosArray[0].pan, PosSetting.PosArray[0].tilt
            ,PosSetting.PosArray[1].pan, PosSetting.PosArray[1].tilt,PosSetting.PosArray[2].pan, PosSetting.PosArray[2].tilt
            ,PosSetting.PosArray[3].pan, PosSetting.PosArray[3].tilt,PosSetting.PosArray[4].pan, PosSetting.PosArray[4].tilt);
          qic_ret =QicSetAdvMotorControl(ADV_MOTOR_NAVIGATE_POSITION_SET,set_numbers,PosSetting);

        }else{
            temp_number=set_numbers/5;
            temp_start_index=set_numbers%5;

            printf("temp_number=%d, temp_start_index=%d\n",temp_number,temp_start_index);

          for(i=0;i<temp_number*5;i+=5)
        {
              PosSetting.TotalPositionNumbers=5;
              PosSetting.StartElementIndex=i+1;

              memcpy(&PosSetting.PosArray[0],Pos+i,sizeof(MotorPosition_t)*5);
              qic_ret =QicSetAdvMotorControl(ADV_MOTOR_NAVIGATE_POSITION_SET,5,PosSetting);

            printf("i=%d, end=%d\n",i,temp_number*5);
            printf("TotalPositionNumbers=%d\n",PosSetting.TotalPositionNumbers);
            printf("StartElementIndex=%d\n",PosSetting.StartElementIndex);
            printf("0: %d, %d, 1:%d, %d, 2:%d, %d,3:%d, %d,4:%d, %d,",PosSetting.PosArray[0].pan, PosSetting.PosArray[0].tilt
            ,PosSetting.PosArray[1].pan, PosSetting.PosArray[1].tilt,PosSetting.PosArray[2].pan, PosSetting.PosArray[2].tilt
            ,PosSetting.PosArray[3].pan, PosSetting.PosArray[3].tilt,PosSetting.PosArray[4].pan, PosSetting.PosArray[4].tilt);

            printf("i=%d, end=%d\n",i,temp_number*5);
        }

          if(temp_start_index!=0)
          {

              PosSetting.TotalPositionNumbers=temp_start_index;
              PosSetting.StartElementIndex=(temp_number*5)+1;
              memcpy(&PosSetting.PosArray[0],Pos+temp_number*5,sizeof(MotorPosition_t)*temp_start_index);
              qic_ret =QicSetAdvMotorControl(ADV_MOTOR_NAVIGATE_POSITION_SET,temp_start_index,PosSetting);
              printf("TotalPositionNumbers=%d\n",PosSetting.TotalPositionNumbers);
            printf("StartElementIndex=%d\n",PosSetting.StartElementIndex);
            printf("0: %d, %d, 1:%d, %d, 2:%d, %d,3:%d, %d,4:%d, %d,",PosSetting.PosArray[0].pan, PosSetting.PosArray[0].tilt
            ,PosSetting.PosArray[1].pan, PosSetting.PosArray[1].tilt,PosSetting.PosArray[2].pan, PosSetting.PosArray[2].tilt
            ,PosSetting.PosArray[3].pan, PosSetting.PosArray[3].tilt,PosSetting.PosArray[4].pan, PosSetting.PosArray[4].tilt);

          }


        }

        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_postions success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_set_advance_motor_postions failed\n");
        qic_ret=1;
          }


  return qic_ret;

}

/* Get H.264/VP8 Encoder Number of Capabilities*/
int qic_get_frame_number_of_encoder(unsigned char *count)
{
  int qic_ret = 0;

  #ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

   qic_ret=QicEncoderGetNumberOfCapabilities( count);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_frame_number_of_encoder success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_frame_number_of_encoder failed\n");
        qic_ret=1;
        }

  return qic_ret;

}


int qic_get_descriptor_of_encoder(unsigned char index, EncoderCapability_t *capability)
{
  int qic_ret = 0;

  #ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

   qic_ret=QicEncoderGetStreamCaps (index, capability);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

        if(!qic_ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_descriptor_of_encoder success\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "qic_get_descriptor_of_encoder failed\n");
        qic_ret=1;
        }

  return qic_ret;

}

int qic_change_entropy(unsigned int dev_id,EncoderEntropy_t entropy)
{

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  EncoderEntropy_t entropy_ret;
  unsigned char xulib_ver;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

        QicChangeFD(dev_pt->cam[index].fd);

        qic_ret=QicGetFwAPIVersion (&xulib_ver);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "fw SW version=%d\n", xulib_ver);

        if(xulib_ver>=5){
          ret = QicSetEncoderEntropy(entropy);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
          qic_ret = QicGetEncoderEntropy(&entropy_ret);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "entropy = %d\n", entropy_ret);
#endif
        }
        else
          LOG_PRINT(debug_str, DEBUG_INFO, "fw SW version=%d\n", xulib_ver);

    }
  }

  /* success */
  return ret;

}


#if defined(QIC1816)
int qic_change_resolution(unsigned int dev_id, unsigned short width, unsigned short height){

#ifdef SKYPEAPI
endpoint_cfg_t   endpoint_config;
unsigned int stream_ID,temp_u32;
  dev_capability_t  cur_cap;
//  unsigned int  frame_interval;
 // format_id_t format=0;
#endif

  unsigned int index;
  int ret = 0;
  int qic_ret = 0;
  int is_valid_format = 0;
        QicHwVer_t version;
  SysInfo_t sysinfo;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* _MUST_ stop all camera device -> un-init device -> set parameters -> re-allocate buffer -> start camera */
  unsigned int started_devices = 0;


        /* stop all camera */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].is_on == 1 &&dev_pt->cam[index].dev_id & dev_id) {

      printf("stop: %d\n",index);
      started_devices += dev_pt->cam[index].dev_id;
      qic_ret = qic_stop_capture(dev_pt->cam[index].dev_id);
      if (qic_ret) { /* error stoping */
        LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s qic_stop_capture failed\n", dev_pt->cam[index].dev_name);
        ret = 1;
      }
    }
  }

#ifdef USE_THREAD
  syscontrol_working=1;
#endif
  /* device array loop */
  MUTEX_ON(crit_mutex);

  for (index = 0; index < dev_pt->num_devices; index++) {
  if (dev_pt->cam[index].dev_id & dev_id) {
    /* un-init device */
    printf("un-init device: %d\n",index);
    /* munmap old buffers */
    unsigned int buff_index = 0;
    for (buff_index = 0; buff_index < dev_pt->cam[index].num_mmap_buffer; ++buff_index) {
      if (-1 == munmap(dev_pt->cam[index].buffers[buff_index].start, dev_pt->cam[index].buffers[buff_index].length)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "buffers munmap error on cam %s, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d munmap success\n", dev_pt->cam[index].dev_name, buff_index);
      }
    }
    free(dev_pt->cam[index].buffers);

                struct v4l2_requestbuffers req;
    CLEAR(req);


    req.count = 0;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_REQBUFS, &req)) {
      if (EINVAL == errno) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "AA does not support memory mapping\n");
        ret = 1;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_ERROR, "BB ioctl VIDIOC_REQBUFS failed\n");
        ret = 1;
      }
    }

    /* set parameters */
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG) { /* H.264, XUCTRL */
#if defined(QIC_MPEGTS_API)

        /* check valid format */
        is_valid_format = format_check((SUPPORT_FORMAT *) AVC_format, width, height, 0);

        QicGetHWVersion(&version);
        if(version==QIC_HWVERSION_QIC1802)  /* qic1802 chip*/
        {
          if ((width > 1280) || (height > 720))
          {
            is_valid_format=0;
          }
        }
        if (!is_valid_format) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s can't change to error resolution %dx%d\n", dev_pt->cam[index].dev_name, width, height);
          ret = 1;
        }
        else {

          //still need S_FMT before XU CTRL -- joe
          struct v4l2_format fmt;
          CLEAR(fmt);
        //  QicGetSysInfo(&sysinfo);

          fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          fmt.fmt.pix.width   = width;
          fmt.fmt.pix.height    = height;
          if(sysinfo.sHW<=QIC1816A)
            fmt.fmt.pix.pixelformat = dev_pt->cam[index].format;
          #ifdef QIC1822
                 fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;// for 1822
          #else
          fmt.fmt.pix.pixelformat = dev_pt->cam->format; //for 1816, 1802
          #endif

          fmt.fmt.pix.field   = V4L2_FIELD_ANY;

          if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_S_FMT, &fmt)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
            ret = 1;
          } else {
            LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", dev_pt->cam[index].dev_name);
          }

          QicChangeFD(dev_pt->cam[index].fd);

          EncoderParams_t tmp;

          CLEAR(tmp);

          qic_ret = QicEncoderGetParams(&tmp);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

          /*
          tmp.uiBitrate = dev_pt->cam[index].bitrate;
          */
          tmp.usWidth = width;
          tmp.usHeight = height;
          /*
          tmp.ucFramerate = dev_pt->cam[index].framerate;
          tmp.ucSlicesize = dev_pt->cam[index].slicesize;
          */

          qic_ret = QicEncoderSetParams (&tmp, QIC_XU1_ENCODER_RESOLUTION);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
#ifdef DEBUG_LOG
          CLEAR(tmp);

          qic_ret = QicEncoderGetParams(&tmp);
          LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
          if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "%dx%d-%dfps, bitrate%d\n", tmp.usWidth, tmp.usHeight, tmp.ucFramerate, tmp.uiBitrate);
#endif
          dev_pt->cam[index].height = height;
          dev_pt->cam[index].width = width;

        }
#endif

      }
      else if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG) { /*MJPEG, XUCTRL */
        if (dev_pt->cam[index].is_bind) { /* is_bind */
          is_valid_format = format_check((SUPPORT_FORMAT *)MJPEG_bind_format, width, height, 0);
        }
        else { /* not bind */
          is_valid_format = format_check((SUPPORT_FORMAT *)MJPEG_format, width, height, 0);
        }

        /* check valid resolution */
        if (!is_valid_format) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s can't change to error resolution %dx%d\n", dev_pt->cam[index].dev_name, width, height);
          ret = 1;
        }

#if 0
        /* check repeat */
        if ((dev_pt->cam[index].width== width) &&(dev_pt->cam[index].height == height)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s has same resolution %dx%d\n", dev_pt->cam[index].dev_name, width, height);

          ret = 1;
        }
        else
#endif
        {
          dev_pt->cam[index].width = width;
          dev_pt->cam[index].height = height;
#ifdef SKYPEAPI
        if(dev_pt->cam[index].skype_stream==1){
          dev_pt->cam[index].width = REQ_WIDTH;
          dev_pt->cam[index].height = REQ_HEIGHT;
        }
#endif

          /* set parameters */
          struct v4l2_format fmt;

          CLEAR(fmt);

          fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          fmt.fmt.pix.width   = dev_pt->cam[index].width;
          fmt.fmt.pix.height    = dev_pt->cam[index].height;
          fmt.fmt.pix.pixelformat   = dev_pt->cam[index].format;
          fmt.fmt.pix.field     = V4L2_FIELD_ANY;

          if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_S_FMT, &fmt)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
            ret = 1;
          } else {
            LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", dev_pt->cam[index].dev_name);
          }

      }
      }
      else { /* YUV */
        if (dev_pt->cam[index].is_bind) { /* is_bind */
          is_valid_format = format_check((SUPPORT_FORMAT *)YUV_bind_format, width, height, 0);
        }
        else { /* not bind */
          is_valid_format = format_check((SUPPORT_FORMAT *)YUV_format, width, height, 0);
        }

        /* check valid resolution */
        if (!is_valid_format) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s can't change to error resolution %dx%d\n", dev_pt->cam[index].dev_name, width, height);
          ret = 1;
        }
#if 0
        /* check repeat */
        if ((dev_pt->cam[index].width== width) &&(dev_pt->cam[index].height == height)) {
          LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s has same resolution %dx%d\n", dev_pt->cam[index].dev_name, width, height);
          ret = 1;
        }
        else
#endif
        {
          dev_pt->cam[index].width = width;
          dev_pt->cam[index].height = height;

          /* set parameters */
          struct v4l2_format fmt;

          CLEAR(fmt);

          fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          fmt.fmt.pix.width   = dev_pt->cam[index].width;
          fmt.fmt.pix.height    = dev_pt->cam[index].height;
          fmt.fmt.pix.pixelformat   = dev_pt->cam[index].format;
          fmt.fmt.pix.field     = V4L2_FIELD_ANY;

          if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_S_FMT, &fmt)) {
            LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
            ret = 1;
          } else {
            LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", dev_pt->cam[index].dev_name);
          }
        }
      }

    }

    /* re-allocate buffer */
    qic_dev *cam;
    cam = &dev_pt->cam[index];
    CLEAR(req);

    req.count = cam->num_mmap_buffer;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(cam->fd, VIDIOC_REQBUFS, &req)) {
      if (EINVAL == errno) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s does not support memory mapping, (%d)%s\n", cam->dev_name, errno, strerror(errno));
        ret = 1;
      }
      else {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s ioctl VIDIOC_REQBUFS failed, (%d)%s\n", cam->dev_name, errno, strerror(errno));
        ret = 1;
      }
    }

    LOG_PRINT(debug_str, DEBUG_INFO, "%s v4l2_requestbuffers number = %d\n", cam->dev_name, req.count);

    if (req.count < 2) {

      LOG_PRINT(debug_str, DEBUG_ERROR, "%s insufficient buffer memory = %d\n", cam->dev_name, req.count);
      ret = 1;
    }

    cam->buffers = calloc(req.count, sizeof(*cam->buffers));

    if (!cam->buffers) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "malloc: out of memory\n");
      ret = 1;
    }

    for (cam->num_mmap_buffer = 0; cam->num_mmap_buffer < req.count; ++cam->num_mmap_buffer)
    {

      struct v4l2_buffer buf;

      CLEAR(buf);

      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = cam->num_mmap_buffer;

      if (-1 == xioctl(cam->fd, VIDIOC_QUERYBUF, &buf)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s ioctl VIDIOC_QUERYBUF failed: (%d)%s\n", cam->dev_name, errno, strerror(errno));
        ret = 1;
      }

      cam->buffers[cam->num_mmap_buffer].length = buf.length;

      cam->buffers[cam->num_mmap_buffer].start =
          mmap(NULL /* start anywhere */,
               buf.length,
               PROT_READ | PROT_WRITE /* required */,
               MAP_SHARED /* recommended */,
               cam->fd, buf.m.offset);

      LOG_PRINT(debug_str, DEBUG_ERROR,  "mmap fd[%04d] offset[%7d] index[%02d] length[%7d] loc[%p]\n", \
        cam->fd, \
        buf.m.offset, \
        cam->num_mmap_buffer, \
        buf.length, \
        cam->buffers[cam->num_mmap_buffer].start);

      if (MAP_FAILED == cam->buffers[cam->num_mmap_buffer].start) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s mmap call failed\n", cam->dev_name);
        ret = 1;
        continue;
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d mmap succeeded\n", cam->dev_name, buf.index);
      }
    }


    /* Queue the buffer to kernel */
    unsigned int queue_index;
    for (queue_index = 0; queue_index < cam->num_mmap_buffer; ++queue_index)
    {

      struct v4l2_buffer buf;

      CLEAR(buf);

      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = queue_index;

      if (-1 == xioctl(cam->fd, VIDIOC_QBUF, &buf)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "%s ioctl VIDIOC_QBUF failed, (%d)%s\n", cam->dev_name, errno, strerror(errno));
        ret = 1;
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "%s buffer:%d ioctl VIDIOC_QBUF succcess\n", cam->dev_name, queue_index);
      }
    }


  }

  }

  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
    if (dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG) {
#if defined(QIC_MPEGTS_API)
      qic_ret =qic_change_stream_size_format(dev_pt->cam[index].dev_id, dev_pt->cam[index].stream_size_format);
      if (qic_ret) { /* error change stream format */
        LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s qic_change_stream_size_format failed\n", dev_pt->cam[index].dev_name);
        ret = 1;
      }
      qic_ret = qic_change_streamformat(dev_pt->cam[index].dev_id,dev_pt->cam[index].stream_format);
      if (qic_ret) { /* error change stream format */
        LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s qic_change_stream_format failed\n", dev_pt->cam[index].dev_name);
        ret = 1;
      }
#endif
    }
    if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG&&dev_pt->cam[index].is_encoding_video==1) {
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)
       qic_config_codec_EU(dev_pt->cam[index].dev_id,dev_pt->cam[index].codec_type );
       qic_change_bitrate_EU(dev_pt->cam[index].dev_id, SIMULCAST_STREAM0, dev_pt->cam[index].bitrate);
        qic_change_frame_interval_EU(dev_pt->cam[index].dev_id, SIMULCAST_STREAM0, dev_pt->cam[index].frame_interval);
                     qic_change_resolution_EU(dev_pt->cam[index].dev_id,SIMULCAST_STREAM0,width,height);
#endif
      }
#ifdef SKYPEAPI
    if (dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG&&dev_pt->cam[index].skype_stream==1) {
      sky_config_commit(dev_pt->cam[index].dev_id,SID_MAIN ,F_AVC,dev_pt->cam[index].width, dev_pt->cam[index].height,
     dev_pt->cam[index].frame_interval,dev_pt->cam[index].bitrate);
      }
#endif

    }
  }

  /* start capture if needed */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
    printf("start: %d\n",index);
    if (dev_pt->cam[index].dev_id &started_devices) {
      qic_ret = qic_start_capture(dev_pt->cam[index].dev_id);
      if (qic_ret) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "dev %s qic_start_capture failed\n", dev_pt->cam[index].dev_name);
        ret = 1;
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "dev %s qic_start_capture succeeded\n", dev_pt->cam[index].dev_name);
      }
    }
  }
  }

          MUTEX_OFF(crit_mutex);

#ifdef USE_THREAD
  syscontrol_working=0;
#endif
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_change_resolution - OUT\n");

  return ret;

}

#endif


int qic_stop_capture(unsigned int dev_id){

  unsigned int index;
  enum v4l2_buf_type type;
  int ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif


  /* device array loop */
  for ( index = 0; index < dev_pt->num_devices; index++) {
    if ((dev_pt->cam[index].dev_id & dev_id)&&(dev_pt->cam[index].is_on == 1)) {

      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

#ifdef USE_THREAD
  syscontrol_working=1;
#endif

      MUTEX_ON(crit_mutex);

      if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_STREAMOFF, &type)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_STREAMOFF failed, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
        ret = 1;
        continue;
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_STREAMOFF succeeded\n", dev_pt->cam[index].dev_name);
      }

      dev_pt->cam[index].is_on = 0;
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s stop capturing\n", dev_pt->cam[index].dev_name);

  /* just sleep and wake a while */
      usleep(50*1000);
      MUTEX_OFF(crit_mutex);

#ifdef USE_THREAD
  syscontrol_working=0;
#endif
    }
  }


  return ret;

}

int qic_start_capture( unsigned int dev_id) {

  unsigned int index;
  enum v4l2_buf_type type;
  int ret = 0;
#ifdef USE_THREAD
  int something_is_activated = 0;
#endif

//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_start_capture - IN\n");
#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) && (dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

//  MUTEX_ON(crit_mutex);
  /* device array loop */
  for ( index = 0; index < dev_pt->num_devices; index++) {
    if ((dev_pt->cam[index].dev_id & dev_id)&&(dev_pt->cam[index].is_on == 0)) {

      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

      if (-1 == xioctl(dev_pt->cam[index].fd, VIDIOC_STREAMON, &type)) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_STREAMON failed, (%d)%s\n", dev_pt->cam[index].dev_name, errno, strerror(errno));
        ret = 1;
        continue;
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_STREAMON succeeded\n", dev_pt->cam[index].dev_name);
      }
      dev_pt->cam[index].is_on = 1;
#ifdef USE_THREAD
      something_is_activated = 1;
#endif
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s start capturing\n", dev_pt->cam[index].dev_name);

      if(dev_pt->cam[index].codec_type==CODEC_VP8_SIMULCAST ||dev_pt->cam[index].codec_type==CODEC_H264_SIMULCAST ){
        last_frame_sizes[0]=0;
        last_frame_sizes[1]=0;
        last_frame_sizes[2]=0;
        last_frame_sizes[3]=0;
      }

    }
  }
//  MUTEX_OFF(crit_mutex);

#ifdef USE_THREAD
  if (something_is_activated) {
    new_video_on = 1;
    MUTEX_ON(condition_mutex);
    pthread_cond_signal( &condition_cond );
    MUTEX_OFF(condition_mutex);
  }
#endif
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_start_capture - OUT\n");

  return ret;

}




/**********************************************
 *  individual camera interface setup function - internal use only
**********************************************/

static int qic_open_device (qic_dev *cam) {
  struct stat st;

  if (-1 == stat(cam->dev_name, &st)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "Cannot identify '%s', (%d)%s\n", cam->dev_name, errno, strerror(errno));
    return 1;
  }

  if (!S_ISCHR(st.st_mode)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s is no device\n", cam->dev_name);
    return 1;
  }

  cam->fd = open(cam->dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == cam->fd) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "Cannot open '%s', (%d)%s\n", cam->dev_name, errno, strerror(errno));
    return 1;
  } else {
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s opened\n", cam->dev_name);
  }

  /* init device */
  struct v4l2_capability cap;

  /* check if device is video device */
  if (-1 == xioctl(cam->fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s is no V4L2 device, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_QUERYCAP failed, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    }
  }

  LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_QUERYCAP succeeded\n", cam->dev_name);

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s is no video capture device\n", cam->dev_name);
    return 1;
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s does not support streaming I/O\n", cam->dev_name);
    return 1;
  }


  //LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_open_device - OUT\n");

  return 0;
}


static int qic_initial_device (qic_dev *cam) {

  struct v4l2_format fmt;
  struct v4l2_streamparm setfps;
//  SysInfo_t sysinfo;
  int ret;
  int qic_ret = 0;

  if (cam->format == V4L2_PIX_FMT_MPEG) { /* H.264/AVC */

#if defined(QIC1816)&&defined(QIC_MPEGTS_API)
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s H.264 configure\n", cam->dev_name);

    CLEAR(fmt);

  //   QicGetSysInfo(&sysinfo);


    fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = cam->width;
    fmt.fmt.pix.height    = cam->height;
    #ifdef QIC1822
           fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;// for 1822
    #else
      fmt.fmt.pix.pixelformat = cam->format; //for 1816, 1802
    #endif

    fmt.fmt.pix.field   = V4L2_FIELD_ANY;

    if (-1 == xioctl(cam->fd, VIDIOC_S_FMT, &fmt)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", cam->dev_name);
    }

    /* check if the device support H.264 */
    /* to be added */
    /* LOG_PRINT(debug_str, DEBUG_ERROR, "NOT_IMPLEMENT: %s check if support H.264/AVC\n", cam->dev_name); */

    /* initial demux with no condition, is_demux is for runtime check*/
    int demux_index;

    for(demux_index=0;demux_index<DEMUX_BUF_MAX;demux_index++) {
      cam->out[demux_index] = (H264DATAFORMAT*)malloc(sizeof(H264DATAFORMAT));
    }
    demux_init(&cam->demux_struc.data);
    demux_set_maxbuffer(&cam->demux_struc.data, DEMUX_BUF_MAX);
    demux_adopt_recover(&cam->demux_struc.data);
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s demuxer initialized\n", cam->dev_name);

    /* qic_module XU control*/
    qic_ret = QicSetDeviceHandle(cam->fd);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, 0); /* don't display this annoying message on monitor*/

    EncoderParams_t tmp;
    tmp.uiBitrate = cam->bitrate;
    tmp.usWidth = cam->width;
    tmp.usHeight = cam->height;
    tmp.ucFramerate = cam->framerate;
    tmp.ucSlicesize = cam->slicesize;
    qic_ret = QicEncoderSetParams (&tmp, QIC_XU1_ENCODER_CONFIG_ALL);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
    CLEAR(tmp);
    qic_ret = QicEncoderGetParams(&tmp);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "%dx%d-%dfps, bitrate: %d\n", tmp.usWidth, tmp.usHeight, tmp.ucFramerate, tmp.uiBitrate);
#endif

    qic_ret = QicEncoderSetGOP(cam->gop);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);

#ifdef DEBUG_LOG
    qic_ret = QicEncoderGetGOP(&ret);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "gop = %d\n", ret);
#endif

#endif

  }
  else { /* YUV or MJPEG */
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s YUV or MJPEG configure\n", cam->dev_name);

    CLEAR(fmt);

    fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = cam->width;
    fmt.fmt.pix.height    = cam->height;
    fmt.fmt.pix.pixelformat = cam->format;
    fmt.fmt.pix.field   = V4L2_FIELD_ANY;

    /* qic_module XU control*/
  #ifdef SKYPEAPI
    qic_ret = SkypeSetDeviceHandle(cam->fd);
  #endif
                qic_ret = QicSetDeviceHandle(cam->fd);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, 0); /* don't display this annoying message on monitor*/


    if (-1 == xioctl(cam->fd, VIDIOC_S_FMT, &fmt)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", cam->dev_name);
    }


    if (cam->is_bind) { /* is_bind - limited function */
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s is bind, fps refer to H.264/AVC setting\n", cam->dev_name);
    }
    else { /* not bind YUV device */

      /* set the framerate */
      CLEAR(setfps);
      setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      setfps.parm.capture.timeperframe.numerator = 1;
      setfps.parm.capture.timeperframe.denominator = cam->framerate;
      ret = xioctl(cam->fd, VIDIOC_S_PARM, &setfps);
      if(ret == -1) {
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s unable to set frame rate, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s set frame rate to %d\n", cam->dev_name, cam->framerate);
      }

    }
  }

  /* init mmap buffer */
  struct v4l2_requestbuffers req;

  CLEAR(req);

  req.count = cam->num_mmap_buffer;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(cam->fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s does not support memory mapping, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    }
    else {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_REQBUFS failed, (%d)%s\n", cam->dev_name, errno, strerror(errno));
      return 1;
    }
  }

  LOG_PRINT(debug_str, DEBUG_INFO, "cam %s v4l2_requestbuffers number = %d\n", cam->dev_name, req.count);

  if (req.count < 2) {

    LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s insufficient buffer memory = %d\n", cam->dev_name, req.count);
    return 1;
  }

  cam->buffers = calloc(req.count, sizeof(*cam->buffers));

  if (!cam->buffers) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "malloc: out of memory\n");
    return 1;
  }

  LOG_PRINT(debug_str, DEBUG_INFO, "system getpagesize() = %d\n", getpagesize());

  for (cam->num_mmap_buffer = 0; cam->num_mmap_buffer < req.count; ++cam->num_mmap_buffer)
  {

    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = cam->num_mmap_buffer;

    if (-1 == xioctl(cam->fd, VIDIOC_QUERYBUF, &buf)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s buffer:%d ioctl VIDIOC_QUERYBUF failed, (%d)%s\n", cam->dev_name, buf.index, errno, strerror(errno));
      return 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d ioctl VIDIOC_QUERYBUF succeeded\n", cam->dev_name, buf.index);
    }

    cam->buffers[cam->num_mmap_buffer].length = buf.length;

    cam->buffers[cam->num_mmap_buffer].start =
        mmap(NULL /* start anywhere */,
             buf.length,
             PROT_READ | PROT_WRITE /* required */,
             MAP_SHARED /* recommended */,
             cam->fd, buf.m.offset);

    LOG_PRINT(debug_str, DEBUG_INFO,  "mmap fd[%04d] offset[%7d] index[%02d] length[%7d] loc[%p]\n", \
      cam->fd, \
      buf.m.offset, \
      cam->num_mmap_buffer, \
      buf.length, \
      cam->buffers[cam->num_mmap_buffer].start);

    if (MAP_FAILED == cam->buffers[cam->num_mmap_buffer].start) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s buffer:%d mmap call failed, (%d)%s\n", cam->dev_name, buf.index, errno, strerror(errno));
      return 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d mmap succeeded\n", cam->dev_name, buf.index);
    }
  }


  /* Queue the buffer to kernel */
  unsigned int queue_index;
  for (queue_index = 0; queue_index < cam->num_mmap_buffer; ++queue_index)
  {

    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = queue_index;

    if (-1 == xioctl(cam->fd, VIDIOC_QBUF, &buf)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s buffer:%d ioctl VIDIOC_QBUF failed, (%d)%s\n", cam->dev_name, buf.index, errno, strerror(errno));
      return 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d ioctl VIDIOC_QBUF succcess\n", cam->dev_name, buf.index);
    }
  }
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_initial_device - OUT\n");

  return 0;

}

static int qic_clean_device(qic_dev *cam) {
  /* clean malloc only */
  unsigned int index;

  /* release demux buffer */
  if (cam->format == V4L2_PIX_FMT_MPEG)  {
#if defined(QIC1816)&&defined(QIC_MPEGTS_API)
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s release demux buffer\n", cam->dev_name);

    for(index=0;index<DEMUX_BUF_MAX;index++) {
      if (cam->out[index] != NULL) {
        free(cam->out[index]);
        cam->out[index] = NULL;
      }
    }
#endif
  }

  /* release queue buffer */

  if (cam->buffers != NULL) {
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s release queue buffer\n", cam->dev_name);

    for (index = 0; index < cam->num_mmap_buffer; ++index) {
      if (-1 == munmap(cam->buffers[index].start, cam->buffers[index].length)) {

        LOG_PRINT(debug_str, DEBUG_ERROR, "buffers munmap error on cam %s, (%d)%s\n", cam->dev_name, errno, strerror(errno));

      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam %s buffer:%d munmap success\n", cam->dev_name, index);
      }
    }

    free(cam->buffers);
    cam->buffers = NULL;
  }
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_clean_device - OUT\n");

  return 0;

}


/**********************************************
 *  setup functions
**********************************************/
int qic_config_commit(void){
  unsigned int i;
  int check = 0;

  /* check if committed */
  if ((config_is_commit) &&(dev_pt != NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is committed already\n");
    return 0;
  }

  /* function pointer check */
#ifdef DEBUG_LOG
  if (dev_pt->debug_print == NULL) {
    dev_pt->debug_print = &debug_print_initial;
    LOG_PRINT(debug_str, DEBUG_ERROR, "debug_print is set to NULL, default to stdout\n");
  }
#endif

  if (dev_pt->frame_output == NULL) {
    dev_pt->frame_output = &frame_output_initial;
    LOG_PRINT(debug_str, DEBUG_ERROR, "frame_output is set to NULL, default to stdout\n");
  }

  /* change the prio if set */
  if (dev_pt->high_prio) {
    set_scheduler();
    LOG_PRINT(debug_str, DEBUG_INFO, "set scheduler to high priority\n");
  }
  /* check default configuration */
  for ( i = 0; i < dev_pt->num_devices; i++) {
    if (dev_pt->cam[i].dev_name == NULL) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d dev_name is not set, NULL\n", dev_pt->cam[i].dev_id);
      check = 1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d dev_name = %s\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].dev_name);
    }

    /* check mmap buffer setting */
    if (dev_pt->cam[i].num_mmap_buffer < 2 ) {
      dev_pt->cam[i].num_mmap_buffer = 12;
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d mmap_buffer is too small, set to default 12\n", dev_pt->cam[i].dev_id);
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d mmap_buffer size = %d\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].num_mmap_buffer);
    }

    if (check) {

      LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d config error, dev_name not defined, aborted\n", dev_pt->cam[i].dev_id);
      return check;
    }
    /* check video format configuration */
    if (dev_pt->cam[i].format == V4L2_PIX_FMT_MPEG) { /* H.264/AVC device */
#if defined(QIC_MPEGTS_API)
      check = 0;

      /* check if format correct */
      check = format_check((SUPPORT_FORMAT *) AVC_format, dev_pt->cam[i].width, dev_pt->cam[i].height,0);

      if(dev_pt->cam[i].framerate==0||dev_pt->cam[i].framerate>30)
        check=0;

      if (!check) {
        dev_pt->cam[i].width = AVC_format[6].width;
        dev_pt->cam[i].height = AVC_format[6].height;
        dev_pt->cam[i].framerate = AVC_format[6].framerate;

        LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d format error , set to %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
          AVC_format[6].width, \
          AVC_format[6].height, \
          AVC_format[6].framerate);
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d format: %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
          dev_pt->cam[i].width, \
          dev_pt->cam[i].height, \
          dev_pt->cam[i].framerate);
      }

      /* check bitrate */
      if (dev_pt->cam[i].bitrate > 8000000 ) {
        dev_pt->cam[i].bitrate = 8000000;
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d bitrate too high, set to 8000000\n", dev_pt->cam[i].dev_id);
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d bitrate = %d\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].bitrate);
      }


      /* check slicesize */
      if ((dev_pt->cam[i].width == 1280)&& (dev_pt->cam[i].height == 720)) {
        if(dev_pt->cam[i].slicesize != 0) {
          dev_pt->cam[i].slicesize = 0;
          LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d HD resolution must set slicesize to 0\n", dev_pt->cam[i].dev_id);
        }
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d slicesize = %d\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].slicesize);
      }

      /* check gop */
      if (dev_pt->cam[i].gop < 0 ) {
        dev_pt->cam[i].gop = 0;
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d GOP set to 0(auto)\n", dev_pt->cam[i].dev_id);
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d GOP = %d\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].gop);
      }

      /* check demux */
      dev_pt->cam[i].is_demux = dev_pt->cam[i].is_demux & 0x01;
      LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d demux set to %s\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].is_demux == 1?"ON":"OFF");
#endif
    }
      else if (dev_pt->cam[i].format == V4L2_PIX_FMT_MJPEG) { /* MJPEG image device */
          /* remove bitrate, slice, demux setting */
        //  dev_pt->cam[i].bitrate = 0;
        //  dev_pt->cam[i].slicesize = 0;
          //dev_pt->cam[i].is_demux = 0;
          if(dev_pt->cam[i].is_demux){
            vp8_h264_bad_frame_count=0;
            generate_key_frame=0;
            demux_VP8_H264_check_bad_frame_initial();
          }
        //  dev_pt->cam[i].gop = 0;
          LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d is MJPEG, set bitrate/slice/demux/gop to 0\n", dev_pt->cam[i].dev_id);
          /* is_bind check */
          dev_pt->cam[i].is_bind = dev_pt->cam[i].is_bind & 0x01;
          LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d format source: %s\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].is_bind == 1?"MJPEG_bind_format":"MJPEG_format");

          /* check video format configuration */
          check = 0;
          if (dev_pt->cam[i].is_bind) {
            /*clear the framerate */
            dev_pt->cam[i].framerate = 0;
            check = format_check((SUPPORT_FORMAT * )MJPEG_bind_format, dev_pt->cam[i].width, dev_pt->cam[i].height, dev_pt->cam[i].framerate);
          }
          else
            check = format_check((SUPPORT_FORMAT * )MJPEG_format, dev_pt->cam[i].width, dev_pt->cam[i].height, 0);
          if (!check) {
            if (dev_pt->cam[i].is_bind) {
              dev_pt->cam[i].width = MJPEG_bind_format[0].width;
              dev_pt->cam[i].height = MJPEG_bind_format[0].height;
              dev_pt->cam[i].framerate = MJPEG_bind_format[0].framerate;
            }
            else {
              dev_pt->cam[i].width = MJPEG_format[0].width;
              dev_pt->cam[i].height = MJPEG_format[0].height;
              dev_pt->cam[i].framerate = MJPEG_format[0].framerate;
            }

            LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d format error , set to %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
              dev_pt->cam[i].width, \
              dev_pt->cam[i].height, \
              dev_pt->cam[i].framerate);
          } else {
            LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d format: %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
            dev_pt->cam[i].width, \
            dev_pt->cam[i].height, \
            dev_pt->cam[i].framerate);
          }
        }
    else if (dev_pt->cam[i].format == V4L2_PIX_FMT_YUYV) { /* YUV raw image device */
      /* remove bitrate, slice, demux setting */
      dev_pt->cam[i].bitrate = 0;
      dev_pt->cam[i].slicesize = 0;
      //dev_pt->cam[i].is_demux = 0;
      if(dev_pt->cam[i].is_demux){
        yuyv_bad_frame_count=0;
        yuyv_data_length = dev_pt->cam[i].width*dev_pt->cam[i].height*2;
      }
      dev_pt->cam[i].gop = 0;
      LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d is YUYV, set bitrate/slice/demux/gop to 0\n", dev_pt->cam[i].dev_id);
      /* is_bind check */
      dev_pt->cam[i].is_bind = dev_pt->cam[i].is_bind & 0x01;
      LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d format source: %s\n", dev_pt->cam[i].dev_id, dev_pt->cam[i].is_bind == 1?"YUV_bind_format":"YUV_format");

      /* check video format configuration */
      check = 0;
      if (dev_pt->cam[i].is_bind) {
        /*clear the framerate */
        dev_pt->cam[i].framerate = 0;
        check = format_check((SUPPORT_FORMAT * )YUV_bind_format, dev_pt->cam[i].width, dev_pt->cam[i].height, dev_pt->cam[i].framerate);
      }
      else
        check = format_check((SUPPORT_FORMAT * )YUV_format, dev_pt->cam[i].width, dev_pt->cam[i].height, /*dev_pt->cam[i].framerate*/0);
      if (!check) {
        if (dev_pt->cam[i].is_bind) {
          dev_pt->cam[i].width = YUV_bind_format[0].width;
          dev_pt->cam[i].height = YUV_bind_format[0].height;
          dev_pt->cam[i].framerate = YUV_bind_format[0].framerate;
        }
        else {
          dev_pt->cam[i].width = YUV_format[0].width;
          dev_pt->cam[i].height = YUV_format[0].height;
          dev_pt->cam[i].framerate = YUV_format[0].framerate;
        }

        LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d format error , set to %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
          dev_pt->cam[i].width, \
          dev_pt->cam[i].height, \
          dev_pt->cam[i].framerate);
      } else {
        LOG_PRINT(debug_str, DEBUG_INFO, "cam #%d format: %dx%d, %dfps\n", dev_pt->cam[i].dev_id, \
        dev_pt->cam[i].width, \
        dev_pt->cam[i].height, \
        dev_pt->cam[i].framerate);
      }
    }
    else {
      /* printf("  Frame format: "FOURCC_FORMAT"\n", FOURCC_ARGS(my_dev->cam[i].format)); */
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d format unknown("FOURCC_FORMAT"), aborted\n", dev_pt->cam[i].dev_id, FOURCC_ARGS(dev_pt->cam[i].format));

      return 1; /*error~*/
    }

    /* clear flag */
    dev_pt->cam[i].is_on = 0;
    /* initial device */
    check = qic_open_device(&dev_pt->cam[i]);
    if (check) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s open failed\n", dev_pt->cam[i].dev_name);
      return check;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s open succeeded\n", dev_pt->cam[i].dev_name);
    }
    check = qic_initial_device(&dev_pt->cam[i]);
    if (check) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s initial failed\n", dev_pt->cam[i].dev_name);
      return check;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s initial succeeded\n", dev_pt->cam[i].dev_name);
    }
  }
  /* signal the commit */
  config_is_commit = 1;
//  LOG_PRINT(debug_str, DEBUG_DETAIL, "qic_config_commit - OUT\n");

  return 0;
}



/**********************************************
 *  teardown functions
**********************************************/
int qic_release(void) {
  unsigned int i;

  /* check if it is commited */
  if ((!config_is_commit)&&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_INFO, "QIC module library is not committed, need no release\n");
    return 0;
  }

  /* release all cam resources */
  for (i = 0; i < dev_pt->num_devices ; i++) {

    qic_clean_device(&dev_pt->cam[i]);
    LOG_PRINT(debug_str, DEBUG_INFO, "cam %s all buffer released\n", dev_pt->cam[i].dev_name);

    /* close the device */
    if (-1 == close(dev_pt->cam[i].fd)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s fd close error, (%d)%s\n", dev_pt->cam[i].dev_name, errno, strerror(errno));
    }
    else {
      dev_pt->cam[i].fd = -1;
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s closed\n", dev_pt->cam[i].dev_name);
    }

  }

  /* release all memory */
  free(dev_pt->cam);
  LOG_PRINT(debug_str, DEBUG_INFO, "camera memory is released\n");


  free(dev_pt);
  dev_pt = NULL;
#ifdef ENABLE_PRINT
  printf( "QIC module library terminated\n");
#endif
  config_is_commit = 0;

  return 0;

}

/**********************************************
 *  initial functions
**********************************************/
qic_module* qic_initialize(int num_devices) {


  /* no more init*/
  if ((config_is_commit) &&(dev_pt != NULL)){
    LOG_PRINT(debug_str, DEBUG_INFO, "QIC module config is committed, no more init\n");
    return dev_pt;
  }

  qic_module *my_dev;
  qic_dev *cams;
  unsigned int dev_id = 0x01;

  int i;

  /* lock to 8 devices */
  if ((num_devices > MAX_SUPPORT_DEVICES) || (num_devices < 1)) {
    printf("QIC module library does not support more than %d devices\n", MAX_SUPPORT_DEVICES);
    return NULL;
  }
  my_dev = malloc(sizeof(qic_module));
  cams = calloc(num_devices, sizeof(qic_dev));


  my_dev->num_devices = num_devices;
  my_dev->cam = cams;

  my_dev->frame_output = NULL;
  my_dev->debug_print = NULL;

  /* default - no change prio */
  my_dev->high_prio = 0;

  for ( i = 0; i < num_devices; i++) {
    my_dev->cam[i].dev_name = NULL;
    my_dev->cam[i].fd = -1;
    my_dev->cam[i].dev_id = dev_id;
    my_dev->cam[i].is_bind = 0;
    my_dev->cam[i].is_on = 0;
    my_dev->cam[i].num_mmap_buffer = 12; /* more for jitter buffer? */
    my_dev->cam[i].format = V4L2_PIX_FMT_MJPEG;
    my_dev->cam[i].bitrate = 0;
    my_dev->cam[i].width = 0;
    my_dev->cam[i].height = 0;
    my_dev->cam[i].framerate = 0;
    my_dev->cam[i].gop = 0;
    my_dev->cam[i].slicesize = 0; /* auto */
    my_dev->cam[i].is_demux = 0;
    my_dev->cam[i].buffers = NULL;
    my_dev->cam[i].nri_iframe = 1; /* default as 1, can't be zero*/
    my_dev->cam[i].nri_pps = 1; /* default as 1, can't be zero*/
    my_dev->cam[i].nri_sps = 1; /* default as 1, can't be zero*/
    my_dev->cam[i].nri_set = 0;

    dev_id = dev_id << 1;
  }

  dev_pt = my_dev;

  return my_dev;

}


int qic_force_config(void)
{
  int check = 0;
  int ret;


  if (dev_pt->high_prio) {
    set_scheduler();
  }

    dev_pt->cam[0].is_on = 0;

    check = qic_open_device(&dev_pt->cam[0]);

    if (check) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam #%d open failed\n", 0);
      return 1;
    }

  struct v4l2_format fmt;
    CLEAR(fmt);

    fmt.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width   = dev_pt->cam[0].width;
    fmt.fmt.pix.height    = dev_pt->cam[0].height;
    fmt.fmt.pix.pixelformat = dev_pt->cam[0].format;
    fmt.fmt.pix.field   = V4L2_FIELD_ANY;

    if (-1 == xioctl(dev_pt->cam[0].fd, VIDIOC_S_FMT, &fmt)) {
      LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s ioctl VIDIOC_S_FMT error, (%d)%s\n", dev_pt->cam[0].dev_name, errno, strerror(errno));
      ret=1;
    } else {
      LOG_PRINT(debug_str, DEBUG_INFO, "cam %s ioctl VIDIOC_S_FMT success\n", dev_pt->cam[0].dev_name);
      ret=0;
  }

    if(!ret)
  config_is_commit = 1;


    ret = QicSetDeviceHandle(dev_pt->cam[0].fd);
      if (ret) {
        printf("QicSetDeviceHandle error\n");
        return 1;
      }



  return ret;
}


int video_control_iter_next(int fd, struct v4l2_queryctrl *query)
{
  if (query->id == 0)
    query->id = V4L2_CID_BASE;
  else {
#ifndef V4L2_CTRL_FLAG_NEXT_CTRL
    query->id++;
    if (query->id > V4L2_CID_LASTP1)
      return false;
#else
    query->id |= V4L2_CTRL_FLAG_NEXT_CTRL;
#endif
  }
  printf("ioctl(%d, VIDIOC_QUERYCTRL, {id=0x%08x})\n", fd, query->id);
  return ioctl(fd, VIDIOC_QUERYCTRL, query) == 0;
}


int get_control(int fd, unsigned int id, int type, unsigned int reserved_0,signed long *val)
{
  struct v4l2_ext_controls ctrls;
  struct v4l2_ext_control ctrl;
  int ret;

  memset(&ctrls, 0, sizeof(ctrls));
  memset(&ctrl, 0, sizeof(ctrl));

  ctrls.reserved[0] = reserved_0;
  ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(id);
  ctrls.count = 1;
  ctrls.controls = &ctrl;

  ctrl.id = id;

  printf("ioctl(%d, VIDIOC_G_EXT_CTRLS, {count=%d})\n",fd, ctrls.count);
  ret = ioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls);
  if (ret != -1) {
    if (type == V4L2_CTRL_TYPE_INTEGER64)
      *val = ctrl.value64;
    else
      *val = ctrl.value;
    return 0;
  }
  if (errno == EINVAL || errno == ENOTTY) {
    struct v4l2_control old;

    old.id = id;
    printf("ioctl(%d, VIDIOC_G_CTRL, {id=%d} size=%d  )\n", fd, id,ctrl.size);
    ret = ioctl(fd, VIDIOC_G_CTRL, &old);
    if (ret != -1) {
      *val = old.value;
      return 0;
    }
  }

  printf("unable to get control 0x%8.8x: %s (%d).",id, strerror(errno), errno);
  return -1;
}



int enum_controls(int fd)
{
  struct v4l2_queryctrl queryctrl;
  struct v4l2_querymenu querymenu;
  struct v4l2_control   control_s;
  struct v4l2_input*    getinput;

unsigned int ret=0;
  //Name of the device
  signed int menu_index;
  getinput=(struct v4l2_input *) calloc(1, sizeof(struct v4l2_input));
  memset(getinput, 0, sizeof(struct v4l2_input));
  getinput->index=0;
 ret= ioctl(fd,VIDIOC_ENUMINPUT , getinput);
//  printf (" Available controls of device '%s' (Type 1=Integer 2=Boolean 3=Menu 4=Button 5=Integer64 6=class)\n",getinput->name);
LOG_PRINT(debug_str, DEBUG_INFO, " Available controls of device '%s' (Type 1=Integer 2=Boolean 3=Menu 4=Button 5=Integer64 6=class)\n",getinput->name);
  //subroutine to read menu items of controls with type 3
  void enumerate_menu (void) {
LOG_PRINT(debug_str, DEBUG_INFO,"  Menu items:\n");
    memset (&querymenu, 0, sizeof (querymenu));
    querymenu.id = queryctrl.id;
 menu_index=(unsigned int)querymenu.index;

    for (menu_index = queryctrl.minimum;
         menu_index <= queryctrl.maximum;
         menu_index++) {
      if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu)) {
LOG_PRINT(debug_str, DEBUG_INFO,"  index:%d name:%s\n", menu_index, querymenu.name);
  TIME_DELAY(1);
      } else {
        printf ("error getting control menu");
        break;
      }
    }
  }

  //predefined controls
LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_BASE         (predefined controls):\n");
  memset (&queryctrl, 0, sizeof (queryctrl));
  for (queryctrl.id = V4L2_CID_BASE;
       queryctrl.id < V4L2_CID_LASTP1;
       queryctrl.id++) {
    if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(1);
LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();
    } else {
      if (errno == EINVAL)
        continue;
      perror ("error getting base controls");
      goto fatal_controls;
    }
  }

  //predefined controls
 LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_CAMERA_CLASS_BASE         (predefined controls):\n");
  memset (&queryctrl, 0, sizeof (queryctrl));
  for (queryctrl.id = V4L2_CID_CAMERA_CLASS_BASE;
       queryctrl.id < V4L2_CID_ZOOM_ABSOLUTE+1;
       queryctrl.id++) {
    if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(1);
      LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();
    } else {
      if (errno == EINVAL)
        continue;
      perror ("error getting base controls");
      goto fatal_controls;
    }
  }

  //driver specific controls
  LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_PRIVATE_BASE (driver specific controls):\n");
  for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
       queryctrl.id++) {
    if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(2);
      LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);


      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();
    } else {
      if (errno == EINVAL)
        break;
      perror ("error getting private base controls");
      goto fatal_controls;
      }
  }


  return 0;
 fatal_controls:
  return -1;
}


int qic_enum_controls(int dev_id)
{
  int ret=0;
  unsigned int index=0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
      QicChangeFD(dev_pt->cam[index].fd);
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id)
      ret=  enum_controls(dev_pt->cam[index].fd);

    }
return ret;


}



int qic_get_control_setting(int dev_id, sqicV4L2 *camerav4l2 )
{
   struct v4l2_queryctrl queryctrl;
  struct v4l2_querymenu querymenu;
  struct v4l2_control   control_s;
  struct v4l2_input*    getinput;
unsigned int ret=0;
unsigned int index=0;
  //Name of the device
 signed int menu_index;


for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
  getinput=(struct v4l2_input *) calloc(1, sizeof(struct v4l2_input));
  memset(getinput, 0, sizeof(struct v4l2_input));
  getinput->index=0;
 ret= ioctl(dev_pt->cam[index].fd,VIDIOC_ENUMINPUT , getinput);
//  printf (" Available controls of device '%s' (Type 1=Integer 2=Boolean 3=Menu 4=Button 5=Integer64 6=class)\n",getinput->name);
LOG_PRINT(debug_str, DEBUG_INFO, " Available controls of device '%s' (Type 1=Integer 2=Boolean 3=Menu 4=Button 5=Integer64 6=class)\n",getinput->name);
  //subroutine to read menu items of controls with type 3
  void enumerate_menu (void) {
LOG_PRINT(debug_str, DEBUG_INFO,"  Menu items:\n");
    memset (&querymenu, 0, sizeof (querymenu));
    querymenu.id = queryctrl.id;
    menu_index=(unsigned int)querymenu.index;
    for (menu_index= queryctrl.minimum;
         menu_index <= queryctrl.maximum;
         menu_index++) {
      if (0 == ioctl (dev_pt->cam[index].fd, VIDIOC_QUERYMENU, &querymenu)) {
LOG_PRINT(debug_str, DEBUG_INFO,"  index:%d name:%s\n", menu_index, querymenu.name);
  TIME_DELAY(1);
      } else {
        printf ("error getting control menu");
        break;
      }
    }
  }

  //predefined controls
LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_BASE         (predefined controls):\n");
  memset (&queryctrl, 0, sizeof (queryctrl));
  for (queryctrl.id = V4L2_CID_BASE;
       queryctrl.id < V4L2_CID_LASTP1;
       queryctrl.id++) {
    if (0 == ioctl (dev_pt->cam[index].fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(dev_pt->cam[index].fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(1);
LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();

     if(V4L2_CID_BRIGHTNESS==queryctrl.id)
      {
    camerav4l2->Brightness.defuat=queryctrl.default_value;
  camerav4l2->Brightness.max=queryctrl.maximum;
  camerav4l2->Brightness.min=queryctrl.minimum;
  camerav4l2->Brightness.now=control_s.value;
  }

     if(V4L2_CID_CONTRAST==queryctrl.id)
      {
    camerav4l2->Contrast.defuat=queryctrl.default_value;
  camerav4l2->Contrast.max=queryctrl.maximum;
  camerav4l2->Contrast.min=queryctrl.minimum;
  camerav4l2->Contrast.now=control_s.value;
  }

     if(V4L2_CID_SATURATION==queryctrl.id)
      {
    camerav4l2->Saturation.defuat=queryctrl.default_value;
  camerav4l2->Saturation.max=queryctrl.maximum;
  camerav4l2->Saturation.min=queryctrl.minimum;
  camerav4l2->Saturation.now=control_s.value;
  }

     if(V4L2_CID_HUE==queryctrl.id)
     {
      camerav4l2->Hue.defuat=queryctrl.default_value;
    camerav4l2->Hue.max=queryctrl.maximum;
    camerav4l2->Hue.min=queryctrl.minimum;
    camerav4l2->Hue.now=control_s.value;
         }

      if(V4L2_CID_GAMMA==queryctrl.id)
     {
      camerav4l2->Gamma.defuat=queryctrl.default_value;
    camerav4l2->Gamma.max=queryctrl.maximum;
    camerav4l2->Gamma.min=queryctrl.minimum;
    camerav4l2->Gamma.now=control_s.value;
         }

        if(V4L2_CID_GAIN==queryctrl.id)
     {
      camerav4l2->Gain.defuat=queryctrl.default_value;
    camerav4l2->Gain.max=queryctrl.maximum;
    camerav4l2->Gain.min=queryctrl.minimum;
    camerav4l2->Gain.now=control_s.value;
         }


    if(V4L2_CID_POWER_LINE_FREQUENCY==queryctrl.id)
     {
      camerav4l2->Plf.defuat=queryctrl.default_value;
    camerav4l2->Plf.max=queryctrl.maximum;
    camerav4l2->Plf.min=queryctrl.minimum;
    camerav4l2->Plf.now=control_s.value;
         }

        if(V4L2_CID_WHITE_BALANCE_TEMPERATURE==queryctrl.id)
     {
      camerav4l2->WB.defuat=queryctrl.default_value;
    camerav4l2->WB.max=queryctrl.maximum;
    camerav4l2->WB.min=queryctrl.minimum;
    camerav4l2->WB.now=control_s.value;
         }

    if(V4L2_CID_SHARPNESS==queryctrl.id)
     {
      camerav4l2->Sharpness.defuat=queryctrl.default_value;
    camerav4l2->Sharpness.max=queryctrl.maximum;
    camerav4l2->Sharpness.min=queryctrl.minimum;
    camerav4l2->Sharpness.now=control_s.value;
         }

    if(V4L2_CID_BACKLIGHT_COMPENSATION==queryctrl.id)
     {
      camerav4l2->BC.defuat=queryctrl.default_value;
    camerav4l2->BC.max=queryctrl.maximum;
    camerav4l2->BC.min=queryctrl.minimum;
    camerav4l2->BC.now=control_s.value;
         }

    } else {
      if (errno == EINVAL)
        continue;
      perror ("error getting base controls");
      goto fatal_controls;
    }




  }

  //predefined controls
 LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_CAMERA_CLASS_BASE         (predefined controls):\n");
  memset (&queryctrl, 0, sizeof (queryctrl));
  for (queryctrl.id = V4L2_CID_CAMERA_CLASS_BASE;
       queryctrl.id < V4L2_CID_ZOOM_ABSOLUTE+1;
       queryctrl.id++) {
    if (0 == ioctl (dev_pt->cam[index].fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(dev_pt->cam[index].fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(1);
      LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();

    if(V4L2_CID_EXPOSURE_ABSOLUTE==queryctrl.id)
     {
      camerav4l2->Exposure.defuat=queryctrl.default_value;
    camerav4l2->Exposure.max=queryctrl.maximum;
    camerav4l2->Exposure.min=queryctrl.minimum;
    camerav4l2->Exposure.now=control_s.value;
         }

    if(V4L2_CID_EXPOSURE_AUTO_PRIORITY==queryctrl.id)
     {
      camerav4l2->E_priority.defuat=queryctrl.default_value;
    camerav4l2->E_priority.max=queryctrl.maximum;
    camerav4l2->E_priority.min=queryctrl.minimum;
    camerav4l2->E_priority.now=control_s.value;
         }

    if(V4L2_CID_FOCUS_ABSOLUTE==queryctrl.id)
     {
      camerav4l2->Focus.defuat=queryctrl.default_value;
    camerav4l2->Focus.max=queryctrl.maximum;
    camerav4l2->Focus.min=queryctrl.minimum;
    camerav4l2->Focus.now=control_s.value;
         }
   if(V4L2_CID_PAN_ABSOLUTE==queryctrl.id)
     {
      camerav4l2->Pan.defuat=queryctrl.default_value;
    camerav4l2->Pan.max=queryctrl.maximum/queryctrl.step;
    camerav4l2->Pan.min=queryctrl.minimum/queryctrl.step;
    camerav4l2->Pan.now=control_s.value/queryctrl.step;
         }
    if(V4L2_CID_TILT_ABSOLUTE==queryctrl.id)
     {
      camerav4l2->Tilt.defuat=queryctrl.default_value;
    camerav4l2->Tilt.max=queryctrl.maximum/queryctrl.step;
    camerav4l2->Tilt.min=queryctrl.minimum/queryctrl.step;
    camerav4l2->Tilt.now=control_s.value/queryctrl.step;
         }

   if(V4L2_CID_ZOOM_ABSOLUTE==queryctrl.id)
     {
      camerav4l2->Zoom.defuat=queryctrl.default_value;
    camerav4l2->Zoom.max=queryctrl.maximum;
    camerav4l2->Zoom.min=queryctrl.minimum;
    camerav4l2->Zoom.now=control_s.value;
         }

    } else {
      if (errno == EINVAL)
        continue;
      perror ("error getting base controls");
      goto fatal_controls;
    }
  }

  //driver specific controls
  LOG_PRINT(debug_str, DEBUG_INFO,"V4L2_CID_PRIVATE_BASE (driver specific controls):\n");
  for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
       queryctrl.id++) {
    if (0 == ioctl (dev_pt->cam[index].fd, VIDIOC_QUERYCTRL, &queryctrl)) {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      control_s.id=queryctrl.id;
      ioctl(dev_pt->cam[index].fd, VIDIOC_G_CTRL, &control_s);
      TIME_DELAY(20);
      LOG_PRINT(debug_str, DEBUG_INFO," index:%-10d name:%-32s type:%d min:%-5d max:%-5d step:%-5d def:%-5d now:%d\n",
              queryctrl.id, queryctrl.name, queryctrl.type, queryctrl.minimum,
              queryctrl.maximum, queryctrl.step, queryctrl.default_value, control_s.value);
      if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
        enumerate_menu ();
    } else {
      if (errno == EINVAL)
        break;
      perror ("error getting private base controls");
      goto fatal_controls;
      }
  }

}
}

  return 0;
 fatal_controls:
  return -1;

}

int qic_test_controls(int fd)
{
    struct v4l2_control tmp_ctrl;

    tmp_ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    tmp_ctrl.value = 0;
    ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl);
    ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl);
    printf ("auto wb : %d\n", tmp_ctrl.value);

    tmp_ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;

    if (0 == ioctl (fd, VIDIOC_G_CTRL, &tmp_ctrl)) {
  printf ("wb temp : %d\n", tmp_ctrl.value);
  //tmp_ctrl.value = tmp_ctrl.value--;
         tmp_ctrl.value--;

  if (0 != ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl))
  {
    perror ("VIDIOC_S_CTRL");
  }
  ioctl (fd, VIDIOC_G_CTRL, &tmp_ctrl);
  printf ("wb temp : %d\n", tmp_ctrl.value);

    } else {
  perror ("VIDIOC_G_CTRL");
    }

    tmp_ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    tmp_ctrl.value = 1;
    ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl);
    ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl);
    printf ("auto wb : %d\n", tmp_ctrl.value);

    tmp_ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;

    if (0 == ioctl (fd, VIDIOC_G_CTRL, &tmp_ctrl)) {
  printf ("wb temp : %d\n", tmp_ctrl.value);
 // tmp_ctrl.value = tmp_ctrl.value--;
         tmp_ctrl.value--;

  if (0 != ioctl (fd, VIDIOC_S_CTRL, &tmp_ctrl))
  {
    perror ("VIDIOC_S_CTRL");
  }
  ioctl (fd, VIDIOC_G_CTRL, &tmp_ctrl);
  printf ("wb temp : %d\n", tmp_ctrl.value);

    } else {
  perror ("VIDIOC_G_CTRL");
    }

    return 0;
}


int qic_change_mirror_mode(int dev_id, unsigned char mode)
{
  unsigned char mirror_mode;
  int qic_ret = 0;
  unsigned int index;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  mode=mode&0x01;

  for (index = 0; index < dev_pt->num_devices; index++) {
    if(dev_pt->cam[index].dev_id & dev_id){
      QicChangeFD(dev_pt->cam[index].fd);
      qic_ret = QicGetMirror(&mirror_mode);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
      if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetMirror success,  Mirror_Mode=%d, \n", mirror_mode);

        if((dev_pt->cam[index].format == V4L2_PIX_FMT_MPEG)
          ||(dev_pt->cam[index].format == V4L2_PIX_FMT_MJPEG&&dev_pt->cam[index].is_encoding_video)) {
          mode= ((mode << 1) & 0x03)|(mirror_mode&0x01);
        }else
          mode= mode|(mirror_mode&0x02);

  qic_ret = QicSetMirror(mode);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetMirror success mode=%d\n",mode);

#ifdef DEBUG_LOG
  mirror_mode = 0;
  qic_ret = QicGetMirror(&mirror_mode);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetMirror success,  Mirror_Mode=%d, \n", mirror_mode);
#endif
          }
  }
  /* success */
  return qic_ret;
}



int qic_get_sysinfo(int dev_id,SysInfo_t *sysinfo)
{
  int qic_ret=0;
  unsigned int index=0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=  QicGetSysInfo(sysinfo);
        LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
        if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetSysInfo success\n");
      }
    }


return qic_ret;

}

int qic_get_chip_id(QicChipId_t *chip_id)
{

  qic_module *my_qic = NULL;
  int ret=0;

    FILE *fp ;
    char buffer[100],tmp[100] ;
    size_t bytes_read ;
    char *match ;
    static char qic_devname[2][20];

  /* check if committed */
  if((!config_is_commit) &&(dev_pt == NULL))
  {
    fp = fopen("/proc/qicuvc", "r") ;

     if(fp!=NULL){

    bytes_read = fread(buffer, 1, sizeof(buffer), fp) ;
    fclose(fp) ;
    /* Bail if read failed or if buffer isn't big enough */
    if(bytes_read == 0 || bytes_read == sizeof(buffer))
    {
         printf("read file fail");
        return 1;
    }
    /* NUL-terminate the text */
    buffer[bytes_read] = '\0' ;
    /* Locate the line that starts with "cpu MHz" */
    match = strstr(buffer, "YUV:") ;
    if(match == NULL)
    {     printf("can not found YUV \r\n");
        return 1;
    }
    /* Parse the line to extract the clock speed */

     sscanf(match, "YUV:%s",tmp) ;
     sprintf(qic_devname[0],"/dev/%s",tmp);

    match = strstr(buffer, "AVC:") ;
    if(match == NULL)
    {     printf("can not found AVC \r\n");
        return 1;
    }
    /* Parse the line to extract the clock speed */

     sscanf(match, "AVC:%s",tmp) ;
     sprintf(qic_devname[1],"/dev/%s",tmp);

     printf("get qic device name %s %s \r\n",qic_devname[0],qic_devname[1]);

  }
  else
  {
    printf("open /proc/qicuvc error!!\n");
    return 1;
  }


  my_qic = qic_initialize(1);

  if (my_qic == NULL) {
    printf("qic_initialize error\n");
    return 1;
  }

  /* call back functions */
  my_qic->debug_print =&debug_print_initial;

  /*  set scheduler */
  my_qic->high_prio = 1;


  my_qic->cam[0].dev_name ="/dev/video0";
  my_qic->cam[0].format = V4L2_PIX_FMT_YUYV;
  my_qic->cam[0].width = 320; /* bitrate/gop/framerate/slicesize is not needed */
  my_qic->cam[0].height = 240;
  my_qic->cam[0].is_bind = 0; /* 2-way output from single QIC module */
  my_qic->cam[0].num_mmap_buffer = 0; /*  less memory */

  ret=qic_force_config();
  if (ret) {
    printf("qic_force_config error\n");
    return 1;
  }
  ret = QicSetDeviceHandle(dev_pt->cam[0].fd);
  if (ret) {
    printf("QicSetDeviceHandle error\n");
    return 1;
  }

  QicGetChipVer(chip_id);

  ret = qic_release();
  if (ret) {
    printf("qic_release error\n");
    return 1;
  }
  }
  else
  {
    QicGetChipVer(chip_id);
  }
  return 0;
}

/*Audio Function control*/
int qic_change_NR_mode(unsigned char onoff)
{
  unsigned char mode=0;
  int qic_ret;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }

  qic_ret = QicSetANRStatus(onoff);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetANRStatus success...\n");

#ifdef DEBUG_LOG
  qic_ret = QicGetANRStatus(&mode);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetANRStatus success,  OnOff=%d ..., \n", mode);
#endif

  return qic_ret;
}

int qic_change_EQ_mode(unsigned char onoff, unsigned char type)
{
  unsigned char mode=0, mode_type=0;
  int qic_ret;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }

  qic_ret = QicSetEQStatus(onoff, type);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetEQStatus success...\n");

#ifdef DEBUG_LOG
  qic_ret = QicGetEQStatus(&mode, &mode_type);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetEQStatus success, OnOff=%d, Type=%d ..., \n", mode, mode_type);
#endif

  return qic_ret;
}

int qic_change_BF_mode(unsigned char onoff)
{
  unsigned char mode=0;
  int qic_ret;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }

  qic_ret = QicSetBFStatus(onoff);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetBFStatus success...\n");

#ifdef DEBUG_LOG
  qic_ret = QicGetBFStatus(&mode);
  LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetBFStatus success,  OnOff=%d ..., \n", mode);
#endif

  return qic_ret;
}


int qic_change_ADC_mode(ADCMode_t mode)
{
    ADCMode_t cur_mode;
    int qic_ret;

    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL))
    {
      LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
      return 1;
    }

    qic_ret = QicSetADCMode(mode);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicSetADCMode success...\n");

  #ifdef DEBUG_LOG
    qic_ret = QicGetADCMode(&cur_mode);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicGetADCMode success,  current mode=%d ..., \n", cur_mode);
  #endif

    return qic_ret;
}


int enum_frame_intervals(int fd, unsigned int pixfmt, unsigned int width, unsigned int height)
{
    int ret;
    struct v4l2_frmivalenum fival;

    memset(&fival, 0, sizeof(fival));
    fival.index = 0;
    fival.pixel_format = pixfmt;
    fival.width = width;
    fival.height = height;
    printf("\tTime interval between frame: ");
    while ((ret = ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
        if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                printf("%u/%u, ", fival.discrete.numerator, fival.discrete.denominator);
        } else if (fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
                printf("{min { %u/%u } .. max { %u/%u } }, ",
                        fival.stepwise.min.numerator, fival.stepwise.min.numerator,
                        fival.stepwise.max.denominator, fival.stepwise.max.denominator);
                break;
        } else if (fival.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
                printf("{min { %u/%u } .. max { %u/%u } / "
                        "stepsize { %u/%u } }, ",
                        fival.stepwise.min.numerator, fival.stepwise.min.denominator,
                        fival.stepwise.max.numerator, fival.stepwise.max.denominator,
                        fival.stepwise.step.numerator, fival.stepwise.step.denominator);
                break;
        }
        fival.index++;
    }
    printf("\n");
    if (ret != 0 && errno != EINVAL) {
        printf("ERROR enumerating frame intervals: %d\n", errno);
        return errno;
    }

    return 0;
}


int enum_device_formats(int fd, char* format, char print)
{
  int ret;
  struct v4l2_fmtdesc fmt;
      int h_width = 0, h_height = 0;
    struct v4l2_frmsizeenum sizes;
  struct v4l2_frmivalenum frmsize;

  memset(&fmt, 0, sizeof(fmt));
  fmt.index = 0;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
    fmt.index++;

  if(print){
    printf("{ pixelformat = '%c%c%c%c', description = '%s' }\n",
        fmt.pixelformat & 0xFF, (fmt.pixelformat >> 8) & 0xFF,
        (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
        fmt.description);

    /* Enumerate resolutions */
    CLEAR(sizes);
    sizes.pixel_format =fmt.pixelformat;
    sizes.index     = 0;

    if (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &sizes) == -1)
    {
      printf("Error enumerating frame sizes. Webcam does not support hardware MJPEG compression?");
    }
    if (sizes.type != V4L2_FRMSIZE_TYPE_DISCRETE)
    {
      printf("Frame size type is not discrete");
    }

    /* find out largest supported resolution */
    do
    {

      sizes.index++;
      printf("resolution:(%dx%d)\n",sizes.discrete.width,sizes.discrete.height);
      enum_frame_intervals( fd, sizes.pixel_format, sizes.discrete.width,sizes.discrete.height);

    }
    while ( ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &sizes) != -1 );



  }
            if(strcmp((char*)fmt.description,"YUV 4:2:2 (YUYV)")==0||strcmp((char*)fmt.description,"MPEG2-TS")==0
          ||strcmp((char*)fmt.description,"MJPEG")==0)
      strcpy(format,(char*)fmt.description);

  }
  if (errno != EINVAL) {
    printf("ERROR enumerating frame formats: %d\n", errno);
    return -1;
  }

  return 0;
}


#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)

int qic_config_codec_EU(unsigned int dev_id,EuExSelectCodec_t tCodec)
{

int ret=0;
unsigned int index;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
    dev_pt->cam[index].codec_type=tCodec;

        QicChangeFD(dev_pt->cam[index].fd);
          ret=QicEuExSetSelectCodec(tCodec);

      if (dev_pt->cam[index].is_demux){
        if(tCodec==CODEC_VP8||tCodec==CODEC_VP8_SIMULCAST){
          ret=QicMmioWrite(0x6F00009C, 1); //Enable VP8 debug mode
        }else if(tCodec==CODEC_H264||tCodec==CODEC_H264_SIMULCAST){
          ret=QicMmioWrite(0x6F0000A0, 1); //Enable H264 debug mode
        }
        if(ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "Enable Encoding stream debug mode Error...\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "Enable Encoding stream debug mode success...\n");
        }
      }else{
        if(tCodec==CODEC_VP8||tCodec==CODEC_VP8_SIMULCAST){
          ret=QicMmioWrite(0x6F00009C, 0); //Disable VP8 debug mode
        }else if(tCodec==CODEC_H264||tCodec==CODEC_H264_SIMULCAST){
          ret=QicMmioWrite(0x6F0000A0, 0); //Disable H264 debug mode
        }
        if(ret){
          LOG_PRINT(debug_str, DEBUG_INFO, "Disable Encoding stream debug mode Error...\n");
        }else{
          LOG_PRINT(debug_str, DEBUG_INFO, "Disable Encoding stream debug mode success...\n");
        }
      }

#ifdef DEBUG_LOG
EuExSelectCodec_t bCodec;
    ret=QicEuExGetSelectCodec(&bCodec);
    printf("QicEuExGetSelectCodec=======%d\n",bCodec);

#endif
      }
    }

  return ret;

}


int qic_simulcast_config_codec_EU(unsigned int dev_id,EuExSelectCodec_t tCodec,simulcast_configs_t config_setting)
{

int ret=0;
unsigned int index;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
    dev_pt->cam[index].codec_type=tCodec;
             dev_pt->cam[index].simulcast_configs_setting=config_setting;
      QicChangeFD(dev_pt->cam[index].fd);

    ret=QicEuExSetSelectCodec(tCodec);
    if(config_setting.config_all!=0){
      ret= QicEuSetSelectLayer(SIMULCAST_STREAM0);
      ret=QicEuSetVideoResolution(config_setting.configs[0].width,config_setting.configs[0].height);
      ret=QicEuSetMinimumFrameInterval(config_setting.configs[0].frame_interval);
      ret=QicEuSetAverageBitrateControl(config_setting.configs[0].bitrate);
      ret=QicEuSetSynchronizationAndLongTermReferenceFrame(1, config_setting.configs[0].key_frame_interval, 0);

      ret= QicEuSetSelectLayer(SIMULCAST_STREAM1);
      ret=QicEuSetVideoResolution(config_setting.configs[1].width,config_setting.configs[1].height);
      ret=QicEuSetMinimumFrameInterval(config_setting.configs[1].frame_interval);
      ret=QicEuSetAverageBitrateControl(config_setting.configs[1].bitrate);
      ret=QicEuSetSynchronizationAndLongTermReferenceFrame(1, config_setting.configs[1].key_frame_interval, 0);

      ret= QicEuSetSelectLayer(SIMULCAST_STREAM2);
      ret=QicEuSetVideoResolution(config_setting.configs[2].width,config_setting.configs[2].height);
      ret=QicEuSetMinimumFrameInterval(config_setting.configs[2].frame_interval);
      ret=QicEuSetAverageBitrateControl(config_setting.configs[2].bitrate);
      ret=QicEuSetSynchronizationAndLongTermReferenceFrame(1, config_setting.configs[2].key_frame_interval, 0);

      ret= QicEuSetSelectLayer(SIMULCAST_STREAM3);
      ret=QicEuSetVideoResolution(config_setting.configs[3].width,config_setting.configs[3].height);
      ret=QicEuSetMinimumFrameInterval(config_setting.configs[3].frame_interval);
      ret=QicEuSetAverageBitrateControl(config_setting.configs[3].bitrate);
      ret=QicEuSetSynchronizationAndLongTermReferenceFrame(1, config_setting.configs[3].key_frame_interval, 0);
    }else{
      ret= QicEuSetSelectLayer(SIMULCAST_STREAMALL);
      ret=QicEuSetVideoResolution(config_setting.configs[0].width,config_setting.configs[0].height);
      ret=QicEuSetMinimumFrameInterval(config_setting.configs[0].frame_interval);
      ret=QicEuSetAverageBitrateControl(config_setting.configs[0].bitrate);
      ret=QicEuSetSynchronizationAndLongTermReferenceFrame(1, config_setting.configs[0].key_frame_interval, 0);
      }

#ifdef DEBUG_LOG

unsigned short wWidth, wHeight;
unsigned int dwFrameInterval;
unsigned int dwAverageBitRate;
unsigned char bSyncFrameType;
unsigned short wSyncFrameInterval;
unsigned char bGradualDecoderRefresh;
EuExSelectCodec_t bCodec;


    QicEuExGetSelectCodec(&bCodec);
    printf("QicEuExGetSelectCodec=======%d\n",bCodec);

    QicEuGetVideoResolution(&wWidth,&wHeight);
    printf("QicEuGetVideoResolution=======%d, %d\n",wWidth,wHeight);

    QicEuGetMinimumFrameInterval(&dwFrameInterval);
    printf("QicEuGetMinimumFrameInterval=======%u\n",dwFrameInterval);

    QicEuGetAverageBitrateControl(&dwAverageBitRate);
    printf("QicEuGetAverageBitrateControl=======%u\n",dwFrameInterval);

    QicEuGetSynchronizationAndLongTermReferenceFrame(&bSyncFrameType, &wSyncFrameInterval, &bGradualDecoderRefresh);
    printf("QicEuGetSynchronizationAndLongTermReferenceFrame=======%d, %d, %d\n",bSyncFrameType,wSyncFrameInterval,bGradualDecoderRefresh);
#endif
      }
    }

  return ret;

}


int qic_set_temporal_layer_number_EU(unsigned int dev_id,unsigned short stream_id,unsigned char temp_layer_num)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_id);
  qic_ret= QicEuExSetTsvc( temp_layer_num);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuExSetTsvc success \n");

  #ifdef DEBUG_LOG
  unsigned char number;
    qic_ret=QicEuExGetTsvc(&number);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuExGetTsvc=======%d\n",number);
  #endif
      }
    }

  /* success */
  return qic_ret;

}



int qic_change_bitrate_EU(unsigned int dev_id,unsigned short stream_id, unsigned int ubitrate) {

  unsigned int index, CPBsize;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif



  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
    printf("%d, %d, %d, %d\n",dev_pt->cam[index].simulcast_configs_setting.configs[0].width,dev_pt->cam[index].simulcast_configs_setting.configs[1].width
            ,dev_pt->cam[index].simulcast_configs_setting.configs[2].width,dev_pt->cam[index].simulcast_configs_setting.configs[3].width);
    qic_ret=QicEuSetSelectLayer(stream_id);
    if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

    dev_pt->cam[index].bitrate=ubitrate;
    qic_ret=QicEuSetAverageBitrateControl(ubitrate);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetAverageBitrateControl success \n");

    CPBsize = ubitrate >>1;
    qic_ret=QicEuSetCpbSizeControl(CPBsize);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetCpbSizeControl success \n");

  #ifdef DEBUG_LOG
  unsigned int dwAverageBitRate;
    qic_ret=QicEuGetAverageBitrateControl(&dwAverageBitRate);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetAverageBitrateControl=======%u\n",dwAverageBitRate);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_ErrorResiliency_EU(unsigned int dev_id, unsigned short errorResiliency)
  {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
    qic_ret=QicEuSetErrorResiliency(errorResiliency);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetErrorResiliency success \n");

  #ifdef DEBUG_LOG
  unsigned short bmErrorResiliencyFeatures;
    QicEuGetErrorResiliency(&bmErrorResiliencyFeatures);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetErrorResiliency=======%d\ n",bmErrorResiliencyFeatures);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_resolution_EU(unsigned int dev_id,unsigned short stream_id, unsigned short width, unsigned short height) {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
    qic_ret=QicEuSetSelectLayer(stream_id);
    if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

    qic_ret=QicEuSetVideoResolution(width,height);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetVideoResolution success \n");

  #ifdef DEBUG_LOG
  unsigned short wWidth, wHeight;
    QicEuGetVideoResolution(&wWidth,&wHeight);
    printf("QicEuGetVideoResolution=======%d, %d\n",wWidth,wHeight);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetVideoResolution=======%d, %d\n",wWidth,wHeight);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_start_stop_layer_EU(unsigned int dev_id,unsigned short stream_layer,unsigned char on)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
        QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_layer);
      if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

    qic_ret=QicEuSetStartOrStopLayer(on);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetStartOrStopLayer success\n");

    #ifdef DEBUG_LOG
    unsigned char bUpdate;
      qic_ret=QicEuGetStartOrStopLayer(&bUpdate);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetStartOrStopLayer=======%u\n",bUpdate);
    #endif
      }
    }

  /* success */
  return qic_ret;

}

int qic_change_frame_interval_EU(unsigned int dev_id,unsigned short stream_id,unsigned int uframe_interval) {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
          QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_id);
      if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

      dev_pt->cam[index].frame_interval=uframe_interval;

    qic_ret=QicEuSetMinimumFrameInterval(uframe_interval);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetMinimumFrameInterval success \n");

    #ifdef DEBUG_LOG
    unsigned int dwFrameInterval;
      qic_ret=QicEuGetMinimumFrameInterval(&dwFrameInterval);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetMinimumFrameInterval=======%u\n",dwFrameInterval);
    #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_CPB_size_EU(unsigned int dev_id,unsigned short stream_layer,unsigned int CPBsize)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
          QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_layer);
      if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

      qic_ret=QicEuSetCpbSizeControl(CPBsize);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetCpbSizeControl success \n");

    #ifdef DEBUG_LOG
    unsigned int dwCPBsize;
      qic_ret=QicEuGetCpbSizeControl(&dwCPBsize);
      LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
    if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetCpbSizeControl=======%u\n",dwCPBsize);
    #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_generate_key_frame_EU(unsigned int dev_id, unsigned short stream_id,unsigned char cSyncFrameType, unsigned short sSyncFrameInterval, unsigned char cGradualDecoderRefresh) {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
          QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_id);
      if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

    dev_pt->cam[index].key_frame_interval=sSyncFrameInterval;

  qic_ret=QicEuSetSynchronizationAndLongTermReferenceFrame(cSyncFrameType, sSyncFrameInterval, cGradualDecoderRefresh);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSynchronizationAndLongTermReferenceFrame success \n");

  #ifdef DEBUG_LOG
  unsigned char bSyncFrameType;
  unsigned short wSyncFrameInterval;
  unsigned char bGradualDecoderRefresh;
    qic_ret=QicEuGetSynchronizationAndLongTermReferenceFrame(&bSyncFrameType, &wSyncFrameInterval, &bGradualDecoderRefresh);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetSynchronizationAndLongTermReferenceFrame==>bSyncFrameType=%d, wSyncFrameInterval=%d, bGradualDecoderRefresh=%d\n",bSyncFrameType
      ,wSyncFrameInterval, bGradualDecoderRefresh);
  #endif
      }
    }

  /* success */
  return qic_ret;

}



int qic_change_profile_toolset_EU(unsigned int dev_id, unsigned short stream_id, unsigned short wProfile, unsigned char bmSettings) {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
    qic_ret=QicEuSetSelectLayer(stream_id);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

  qic_ret=QicEuSetProfileAndToolset(wProfile,  0, bmSettings);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetProfileAndToolset success \n");

  #ifdef DEBUG_LOG

  unsigned short wProfile;
  unsigned short wConstrainedToolset;
  unsigned char bmSettings;

    qic_ret=QicEuGetProfileAndToolset(&wProfile, &wConstrainedToolset, &bmSettings);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetProfileAndToolset==>wProfile=%d, wConstrainedToolset=%d, bmSettings=%d\n",wProfile
      ,wConstrainedToolset, bmSettings);
  #endif
      }
    }

  /* success */
  return qic_ret;

}



int qic_change_slice_size_EU(unsigned int dev_id,unsigned short stream_id, unsigned short wSliceConfigSetting) {

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_id);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");


  qic_ret=QicEuSetSliceMode(0x03, wSliceConfigSetting);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSliceMode success \n");

  #ifdef DEBUG_LOG

  unsigned short wSliceMode;
  unsigned short wSliceConfigSetting;

    qic_ret=QicEuGetSliceMode(&wSliceMode, &wSliceConfigSetting);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetSliceMode==>wSliceMode=%d, wSliceConfigSetting=%d\n",wSliceMode
      ,wSliceConfigSetting);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_rete_control_mode_EU(unsigned int dev_id,unsigned short stream_id, unsigned char bRateControlMode)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
  qic_ret=QicEuSetSelectLayer(stream_id);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

  qic_ret=QicEuSetRateControlMode( bRateControlMode);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetRateControlMode success \n");

  #ifdef DEBUG_LOG

  unsigned char bRateControlMode;

    qic_ret=QicEuGetRateControlMode(&bRateControlMode);
    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetRateControlMode==>bRateControlMode=%d\n",bRateControlMode);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_QP_EU(unsigned int dev_id ,unsigned short stream_layer, unsigned short value)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
  qic_ret=QicEuSetSelectLayer(stream_layer);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

  qic_ret=QicEuSetQuantizationParameter((unsigned short)value, (unsigned short)(value>>16), (unsigned short)((value>>16)>>16));

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetQuantizationParameter success \n");

  #ifdef DEBUG_LOG

  unsigned short QpPrime_I;
  unsigned short QpPrime_P;
  unsigned short QpPrime_B;

    qic_ret=QicEuGetQuantizationParameter(&QpPrime_I, &QpPrime_P, &QpPrime_B);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetQuantizationParameter==>QpPrime_I=%d,QpPrime_P=%d,QpPrime_B=%d\n",QpPrime_I
    ,QpPrime_P,QpPrime_B);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_level_EU(unsigned int dev_id ,unsigned short stream_id,unsigned char bLevelIDC)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
  qic_ret=QicEuSetSelectLayer(stream_id);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");


  qic_ret=QicEuSetLevelIdc(bLevelIDC);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetLevelIdc success \n");

  #ifdef DEBUG_LOG

  unsigned char LevelIDC;

    qic_ret=QicEuGetLevelIdc(&LevelIDC);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetLevelIdc==>LevelIDC=%d\n",LevelIDC);
  #endif
      }
    }

  /* success */
  return qic_ret;

}


int qic_change_QP_range_EU(unsigned int dev_id ,unsigned short stream_id,unsigned char bMinQp,unsigned char bMaxQp)
{

  unsigned int index;
  int qic_ret = 0;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      QicChangeFD(dev_pt->cam[index].fd);
      qic_ret=QicEuSetSelectLayer(stream_id);
  if(qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");

    qic_ret=QicEuSetQpRange(bMinQp, bMaxQp);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetQpRange success \n");

  #ifdef DEBUG_LOG

  unsigned char MinQp;
  unsigned char MaxQp;
    qic_ret=QicEuGetQpRange(&MinQp, &MaxQp);

    LOG_XU_PRINT(debug_str, debug_xuctrl_str, qic_ret);
  if(!qic_ret) LOG_PRINT(debug_str, DEBUG_INFO, "QicEuGetQpRange==>MinQp=%d,MaxQp=%d\n",MinQp,MaxQp);
  #endif
      }
    }

  /* success */
  return qic_ret;

}

#ifdef QIC_OSD_API

int qic_osd_change_status(unsigned int dev_id,unsigned char on)
{

int ret=0;
unsigned int index;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdStatusSet(on);


#ifdef DEBUG_LOG
unsigned char osd_status;
    QicOsdStatusGet (&osd_status);
    printf("QicOsdStatusGet=======%d\n",osd_status);

#endif
      }
    }

  return ret;

}


int qic_osd_change_transparency_blink(unsigned int dev_id,OsdMiscell_t set_misc)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret= QicOsdMiscellSet (set_misc);


#ifdef DEBUG_LOG
OsdMiscell_t osd_miscell;
    QicOsdMiscellGet (&osd_miscell);
    printf("QicOsdMiscellGet FG_alpha=%d, BG_alpha=%d, blink_on=%d, blink_off=%d=======%d\n",osd_miscell.fg_alpha,osd_miscell.bg_alpha,osd_miscell.blink_on_frame_count, osd_miscell.blink_off_frame_count );

#endif
      }
    }

  return ret;



}


int qic_osd_get_transparency_blink(unsigned int dev_id,OsdMiscell_t *set_misc)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret= QicOsdMiscellGet (set_misc);


      }
    }

  return ret;

}


int qic_osd_change_color(unsigned int dev_id,OsdColor_t osd_color[OSD_COLOR_MAX])
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdColorSet(osd_color);


#ifdef DEBUG_LOG
 OsdColor_t osd_color_get[OSD_COLOR_MAX];
    QicOsdColorGet(osd_color_get);
  int i=0;
    for(i=0;i<OSD_COLOR_MAX;i++)
    LOG_PRINT(debug_str, DEBUG_INFO, "y=%d, u=%d, u=%d\n",osd_color_get[i].color_y,osd_color_get[i].color_u,osd_color_get[i].color_v);


#endif
      }
    }

  return ret;

}

int qic_osd_get_color(unsigned int dev_id,OsdColor_t *osd_color)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdColorGet(osd_color);

#ifdef DEBUG_LOG
  int i=0;
    for(i=0;i<OSD_COLOR_MAX;i++)
    LOG_PRINT(debug_str, DEBUG_INFO, "y=%d, u=%d, u=%d\n",osd_color[i].color_y,osd_color[i].color_u,osd_color[i].color_v);

#endif

      }
    }

  return ret;

}


int qic_osd_change_frame(unsigned int dev_id,OsdFrame_t osd_frame[OSD_FRAME_MAX])
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdFrameSet(osd_frame);


#ifdef DEBUG_LOG
 OsdFrame_t osd_frame_get[OSD_FRAME_MAX];
int i=0;
    QicOsdFrameGet(osd_frame_get);
    for(i=0;i<OSD_FRAME_MAX;i++)
    LOG_PRINT(debug_str, DEBUG_INFO, "frame_height=%d, frame_y_start=%d\n",osd_frame_get[i].frame_height, osd_frame_get[i].frame_y_start);

#endif
      }
    }

  return ret;

}

int qic_osd_get_frame(unsigned int dev_id,OsdFrame_t *osd_frame)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdFrameGet(osd_frame);
#ifdef DEBUG_LOG
  int i=0;
      for(i=0;i<OSD_FRAME_MAX;i++)
      LOG_PRINT(debug_str, DEBUG_INFO,"frame_height=%d, frame_y_start=%d\n",osd_frame[i].frame_height, osd_frame[i].frame_y_start);
#endif
      }
    }

  return ret;

}


int qic_osd_change_string(unsigned int dev_id,unsigned char line_id,
                  unsigned char start_char_index, //start char position 0~31
                  unsigned char *str, /*Display string*/
                  OsdCharAttr_t char_attr)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret= QicOsdLineStringSet(line_id,start_char_index,str, char_attr);


#ifdef DEBUG_LOG
 unsigned char str_get[OSD_CHAR_MAX];
 OsdCharAttr_t char_attr_get;
    QicOsdLineStringGet(line_id,start_char_index,str_get, &char_attr_get);
      LOG_PRINT(debug_str, DEBUG_INFO, "str_get=%s bg_color_on=%d, fg_color=%d, color_mode=%d, updated=%d\n",
            str_get, char_attr_get.bg_color_on, char_attr_get.fg_color, char_attr_get.color_mode,char_attr_get.updated);

#endif
      }
    }

  return ret;

}


int qic_osd_change_char_of_string(unsigned int dev_id,unsigned char line_id, unsigned char *str, OsdCharAttr_t char_attr[OSD_CHAR_MAX])
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret= QicOsdLineArraySet(line_id,str,char_attr);


#ifdef DEBUG_LOG
 unsigned char str_get[OSD_CHAR_MAX];
 OsdCharAttr_t char_attr_get[OSD_CHAR_MAX];
     QicOsdLineArrayGet(line_id,str_get, char_attr_get);
      LOG_PRINT(debug_str, DEBUG_INFO, "str_get=%s bg_color_on[0]=%d, fg_color[0]=%d, color_mode[0]=%d, updated[0]=%d\n",
            str_get, char_attr_get[line_id].bg_color_on, char_attr_get[line_id].fg_color, char_attr_get[line_id].color_mode,char_attr_get[line_id].updated);

#endif
      }
    }

  return ret;

}

int qic_osd_get_char_of_string(unsigned int dev_id,unsigned char line_id, unsigned char *str, OsdCharAttr_t *char_attr)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
      QicChangeFD(dev_pt->cam[index].fd);
    ret= QicOsdLineArrayGet(line_id,str, char_attr);


      }
    }

  return ret;

}


int qic_osd_change_line_attr(unsigned int dev_id,unsigned char line_id,OsdLineAttr_t line_attr)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdLineAttrSet(line_id, line_attr);


#ifdef DEBUG_LOG
 OsdLineAttr_t line_attr_get;
     QicOsdLineAttrGet(line_id, &line_attr_get);
      LOG_PRINT(debug_str, DEBUG_INFO, "char_count=%d,char_x_repeat=%d,char_y_repeat=%d,enabled=%d,spacing=%d, x_start=%d, y_start=%d\n",
            line_attr_get.char_count,line_attr_get.char_x_repeat,line_attr_get.char_y_repeat,line_attr_get.enabled,
            line_attr_get.spacing,line_attr_get.x_start,line_attr_get.y_start);

#endif
      }
    }

  return ret;

}

int qic_osd_get_line_attr(unsigned int dev_id,unsigned char line_id,OsdLineAttr_t *line_attr)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){

      QicChangeFD(dev_pt->cam[index].fd);
    ret= QicOsdLineAttrGet(line_id, line_attr);


      }
    }

  return ret;

}


int qic_osd_change_timer(unsigned int dev_id,unsigned char line_id,unsigned char enable,OsdTimer_t timer )
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicOsdTimerSet(line_id, enable,timer);


      }
    }

  return ret;

}



#endif


#ifdef QIC_MD_API
/* Motion detection APIs */

int qic_md_get_version(unsigned int dev_id,int *major_version, int *minor_version)
{
  int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicMDGetVersion(major_version, minor_version);


      }
    }

  return ret;
}


int qic_md_start_stop(unsigned int dev_id,unsigned char on)
{

  int ret=0;
  unsigned int index;
  unsigned char md_status;

    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL))
    {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
    }
      /* device array loop */
    for (index = 0; index < dev_pt->num_devices; index++) {
      if (dev_pt->cam[index].dev_id & dev_id){
          QicChangeFD(dev_pt->cam[index].fd);
        ret=QicMmioWrite(0x6F0000A0, 1); //Enable MD info in SEI of H.264
        ret=QicMDSetEnable(on);


#ifdef DEBUG_LOG
  unsigned char osd_status;
      QicMDGetEnable (&md_status);
      printf("QicMDGetEnable=======%d\n",md_status);

#endif
        }
      }

    return ret;


}


int qic_md_change_config(unsigned int dev_id,md_config_t* config, unsigned char interrupt_mode)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicMDSetInterruptMode(interrupt_mode);

      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicMDSetInterruptMode failed\n");
        return 1;
      }
      ret=QicMDSetConfiguration(config);

      if(ret)LOG_PRINT(debug_str, DEBUG_ERROR, "QicMDSetConfiguration failed\n");
      }
    }

  return ret;

}



int qic_md_change_Peak_Bitrate(unsigned int dev_id, unsigned int stream_id, unsigned int peakBitrate)
{

int ret=0;
unsigned int index;
unsigned int bitrate;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);

        ret=QicEuSetSelectLayer(stream_id);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");
        return 1;
      }
      ret=QicSetPeakBitrate(peakBitrate);

      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicSetPeakBitrate failed\n");
        return 1;
      }
      ret=QicGetPeakBitrate(&bitrate);

      if(ret)LOG_PRINT(debug_str, DEBUG_ERROR, "QicGetPeakBitrate failed\n");
      }
    }

  return ret;

}


int qic_md_get_Peak_Bitrate(unsigned int dev_id, unsigned short stream_id, unsigned int *peakBitrate)
{

int ret=0;
unsigned int index;
unsigned int bitrate;

  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);

        ret=QicEuSetSelectLayer(stream_id);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_INFO, "QicEuSetSelectLayer failed \n");
        return 1;
      }
      ret=QicGetPeakBitrate(peakBitrate);

      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicGetPeakBitrate failed\n");
        return 1;
      }


    }
        }

  return ret;

}



int qic_md_get_config(unsigned int dev_id,md_config_t* config, unsigned char* interrupt_mode)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){

      QicChangeFD(dev_pt->cam[index].fd);
    ret= QicMDGetInterruptMode( interrupt_mode);
    if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicMDGetInterruptMode failed\n");

      }
    ret= QicMDGetConfiguration(config);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicMDGetConfiguration failed\n");

      }

    }
  }

  return ret;

}


int qic_md_get_status(unsigned int dev_id,md_status_t* status)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
      QicChangeFD(dev_pt->cam[index].fd);
    ret= QicMDGetStatus(status);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicMDGetStatus failed\n");

      }

    }
  }

  return ret;

}




int qic_td_get_version(unsigned int dev_id,int *major_version, int *minor_version)
{
  int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);
      ret=QicTDGetVersion(major_version, minor_version);


      }
    }

  return ret;
}


int qic_td_start_stop(unsigned int dev_id,unsigned char on)
{

  int ret=0;
  unsigned int index;
  unsigned char td_status;

    /* check if committed */
    if ((!config_is_commit) &&(dev_pt == NULL))
    {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
    }
      /* device array loop */
    for (index = 0; index < dev_pt->num_devices; index++) {
      if (dev_pt->cam[index].dev_id & dev_id){
          QicChangeFD(dev_pt->cam[index].fd);

        ret=QicTDSetEnable(on);


#ifdef DEBUG_LOG
      QicTDGetEnable (&td_status);
      printf("QicTDGetEnable=======%d\n",td_status);

#endif
        }
      }

    return ret;


}


int qic_td_change_config(unsigned int dev_id,td_config_t* config)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
        QicChangeFD(dev_pt->cam[index].fd);

      ret=QicTDSetConfiguration(config);

      if(ret)LOG_PRINT(debug_str, DEBUG_ERROR, "QicTDSetConfiguration failed\n");
      }
    }

  return ret;

}


int qic_td_get_config(unsigned int dev_id,td_config_t* config)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){

      QicChangeFD(dev_pt->cam[index].fd);

    ret= QicTDGetConfiguration(config);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicTDGetConfiguration failed\n");

      }

    }
  }

  return ret;

}


int qic_td_get_status(unsigned int dev_id,int* status)
{

int ret=0;
unsigned int index;


  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL))
  {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
    /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id){
      QicChangeFD(dev_pt->cam[index].fd);

    ret= QicTDGetStatus(status);
      if(ret){
        LOG_PRINT(debug_str, DEBUG_ERROR, "QicTDGetStatus failed\n");

      }

    }
  }

  return ret;

}


#endif


#endif

#ifdef SKYPEAPI


int sky_config_commit(unsigned int dev_id, sid_id_t u_stream_ID ,format_id_t u_format,unsigned short u_width, unsigned short u_height,
    unsigned int u_frame_interval,unsigned int u_bitrate){

endpoint_cfg_t   endpoint_config=0;
unsigned int version=0;
unsigned int firmware_days=0;
unsigned int stream_ID=0;
unsigned int temp_u32=0;
  dev_capability_t  cur_cap;
  format_id_t format=0;
  unsigned short W=0;
  unsigned int  frame_interval=0;
unsigned int bitrate=0;
unsigned short height=0;
micphone_configuration_data_t   mic_configureation;
last_error_t     last_error=0;
unsigned char profile_request=0;

int ret=0;
unsigned int index;

  memset(&cur_cap,0,sizeof(cur_cap));
        memset(&mic_configureation,0,sizeof(mic_configureation));

         /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {
      SkypeChangeFD(dev_pt->cam[index].fd);

      /*(1) get XU version */
      SkypeECXU_Version( GET_CUR, &version);
      printf("version=%d\n",version);

      /*(2) get current main and preview endpoint configuration*/
      SkypeECXU_EndpointSetting(GET_CUR,&endpoint_config);
      printf("endpoint_config=%d\n",endpoint_config);

       SkypeECXU_FirmwareDays( GET_CUR , &firmware_days );
      printf("firmware_days=%d\n",firmware_days);

      /*(3) get Stream ID with MAX and MIN to discover how many payloads each frame contain*/
      SkypeECXU_StreamID( GET_MAX, &temp_u32 );
          printf( "Maximum SID supported = %d.\n", temp_u32 );

      SkypeECXU_StreamID(GET_MIN, &temp_u32 );
          printf( "Minimum SID supported = %d.\n", temp_u32 );

      /*(4) set streamID to 0 for dual enpoints*/
       SkypeECXU_StreamID(GET_CUR,&stream_ID);
       printf("stream_ID=%d\n",stream_ID);

          temp_u32 = u_stream_ID;
       SkypeECXU_StreamID( SET_CUR, &temp_u32 );

      /*(5) call StreamFormatProb control to seting with the descired configureation*/
          cur_cap.format            = u_format;
          cur_cap.resolution.width  = u_width;
          cur_cap.resolution.height = u_height;
          //Set the framerate and bitrate to nonsensical values.
          cur_cap.frame_interval    = u_frame_interval;
          cur_cap.bitrate           = u_bitrate;
          printf( "Probing for H264(%d), %dx%d, stream %d, f-intvl %d, brate %d.\n",
                  cur_cap.format           ,
                  cur_cap.resolution.width ,
                  cur_cap.resolution.height,
                  u_stream_ID                 ,
                  cur_cap.frame_interval   ,
                  cur_cap.bitrate           );
       SkypeECXU_StreamFormatProbe( SET_CUR, &cur_cap );

          //Retrieve closest matching settings.  Expect that frameinterval and
          //bitrate will come back with different values than were set.
      SkypeECXU_StreamFormatProbe( GET_CUR, &cur_cap );
          printf( "Found Type %d, %dx%d, stream %d, f-intvl %d, brate %d.\n",
                  cur_cap.format           ,
                  cur_cap.resolution.width ,
                  cur_cap.resolution.height,
                  u_stream_ID                 ,
                  cur_cap.frame_interval   ,
                  cur_cap.bitrate           );



   dev_pt->cam[index].width=cur_cap.resolution.width;
   dev_pt->cam[index].height=cur_cap.resolution.height;

      /*(6) Call the StreamFormatCommit control to setting with discovered setting*/
        SkypeECXU_StreamFormatCommit(  SET_CUR, cur_cap );

      SkypeECXU_StreamFormatCommit( GET_CUR, cur_cap );
          printf( "Found Type %d, %dx%d, stream %d, f-intvl %d, brate %d.\n",
                  cur_cap.format           ,
                  cur_cap.resolution.width ,
                  cur_cap.resolution.height,
                  u_stream_ID                 ,
                  cur_cap.frame_interval   ,
                  cur_cap.bitrate           );


      SkypeECXU_StreamFormatCommit( GET_MAX, cur_cap );
          printf( "Found Type %d, %dx%d, stream %d, f-intvl %d, brate %d.\n",
                  cur_cap.format           ,
                  cur_cap.resolution.width ,
                  cur_cap.resolution.height,
                  u_stream_ID                 ,
                  cur_cap.frame_interval   ,
                  cur_cap.bitrate           );

       SkypeECXU_LastError( GET_CUR, &last_error   );
       printf("last error=%d\n",last_error);

      SkypeECXU_ProbeType(GET_CUR,&format);
      printf("probeType=%d\n",format);


      SkypeECXU_ProbeType(GET_MAX,&format);
      printf("MAX probeType=%d\n",format);

      SkypeECXU_ProbeType(GET_MIN,&format);
      printf("MIN probeType=%d\n",format);


      format=u_format;
      SkypeECXU_ProbeType(SET_CUR,&format);
      printf("probeType=%d\n",format);

      SkypeECXU_ProbeWidth( GET_MAX ,&W);
      printf("MAX W=%d\n",W);

      SkypeECXU_ProbeWidth( GET_MIN ,&W);
      printf("MIN W=%d\n",W);


      SkypeECXU_ProbeFrameInterval( GET_MAX,&frame_interval );
        printf("GET_MAX, frame_interval=%d\n",frame_interval);

      SkypeECXU_ProbeFrameInterval( GET_MIN,&frame_interval );
        printf("GET_MIN, frame_interval=%d\n",frame_interval);

      SkypeECXU_ProbeBitrate(  GET_MAX,&bitrate      ) ;
        printf("GET_MAX, bitrate=%d\n",bitrate);

      SkypeECXU_ProbeBitrate(  GET_MIN,&bitrate      ) ;
        printf("GET_MIN, bitrate=%d\n",bitrate);


       SkypeECXU_ProbeHeight( GET_MAX,  &height       );
         printf("GET_MAX, height=%d\n",height);

      SkypeECXU_ProbeHeight( GET_MIN,  &height       );
         printf("GET_MIN, height=%d\n",height);

       SkypeECXU_MicphoneConfiguration( GET_CUR,&mic_configureation);
          printf("GET_CUR, %d, %d,%d, %d, %d, %d, %d \n",mic_configureation.wOuterSpacing,mic_configureation.wMicConfiguration,mic_configureation.wInnerSpacing
            ,mic_configureation.wGainrangeDB,mic_configureation.bDefaultStep, mic_configureation.bStepCount, mic_configureation.wFeatureSet );

         SkypeECXU_ProfileRequest( GET_CUR,&profile_request);
                                   printf("GET_CUR, profile_request=%d\n",profile_request);


        SkypeECXU_ProfileRequest( SET_CUR,&profile_request);
         printf("SET_CUR, profile_request=%d\n",profile_request);

      }
    }

return ret;

}


int sky_change_bitrate(unsigned int dev_id, unsigned int bitrate){

  unsigned int index;
  int ret = 0;
  unsigned int max_bps, min_bps;

  driver_error_t sky_ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].skype_stream== 1){ /* H.264, XUCTRL */


    SkypeECXU_ProbeBitrate(  GET_MAX,&max_bps      ) ;
      printf(" GET_MAX, bitrate=%d\n",max_bps);
              SkypeECXU_ProbeBitrate(  GET_MIN,&min_bps      ) ;
      printf(" GET_MIN, bitrate=%d\n",min_bps);


        if (bitrate <= max_bps&&bitrate>=min_bps) {

          SkypeChangeFD(dev_pt->cam[index].fd);


          sky_ret=SkypeECXU_Bitrate( SET_CUR, bitrate );
          //LOG_XU_PRINT(debug_str, debug_xuctrl_str, sky_ret);

          dev_pt->cam[index].bitrate = bitrate;

        }
        else { /* error format */

          LOG_PRINT(debug_str, DEBUG_ERROR, "bitrate out of range (%d), can't set bitrate\n", bitrate);
          ret = 1;
          continue;
        }


      }
      else { /* N/A */
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't AVC device, can't set bitrate\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }

    }

  }
  /* success */
  return ret;

}


int sky_change_frame_interval(unsigned int dev_id, unsigned int frame_interval){

  unsigned int index;
  int ret = 0;
  unsigned int max_frame_interval, min_frame_interval;


#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].skype_stream== 1){ /* H.264, XUCTRL */


    SkypeECXU_ProbeFrameInterval(  GET_MAX,&max_frame_interval      ) ;
      printf(" GET_MAX, frame_interval=%d\n",max_frame_interval);
              SkypeECXU_ProbeFrameInterval(  GET_MIN,&min_frame_interval      ) ;
      printf(" GET_MIN, frame_interval=%d\n",min_frame_interval);


        if (frame_interval <= max_frame_interval&&frame_interval>=min_frame_interval) {

          SkypeChangeFD(dev_pt->cam[index].fd);

          SkypeECXU_FrameInterval( SET_CUR, frame_interval );
          //LOG_XU_PRINT(debug_str, debug_xuctrl_str, sky_ret);

          dev_pt->cam[index].frame_interval = frame_interval;

        }
        else { /* error format */

          LOG_PRINT(debug_str, DEBUG_ERROR, "Frame Interval out of range (%d), can't set frame_interval\n", frame_interval);
          ret = 1;
          continue;
        }


      }
      else { /* N/A */
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't AVC device, can't set frame_interval\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }

    }

  }
  /* success */
  return ret;

}


int sky_Generate_Key_Frame(unsigned int dev_id){

  unsigned int index;
  int ret = 0;

#ifdef COMMIT_CHECK
  /* check if committed */
  if ((!config_is_commit) &&(dev_pt == NULL)) {
    LOG_PRINT(debug_str, DEBUG_ERROR, "QIC module library config is not committed\n");
    return 1;
  }
#endif

  /* device array loop */
  for (index = 0; index < dev_pt->num_devices; index++) {
    if (dev_pt->cam[index].dev_id & dev_id) {

      if (dev_pt->cam[index].skype_stream== 1){ /* H.264, XUCTRL */

          SkypeChangeFD(dev_pt->cam[index].fd);

          SkypeECXU_GenerateKeyFrame( SET_CUR, 1);
          //LOG_XU_PRINT(debug_str, debug_xuctrl_str, sky_ret);

      }
      else { /* N/A */
        LOG_PRINT(debug_str, DEBUG_ERROR, "cam %s isn't AVC device, can't Generate Key Frame\n", dev_pt->cam[index].dev_name);
        ret = 1;
        continue;
      }

    }

  }
  /* success */
  return ret;

}

#endif


