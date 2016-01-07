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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#ifdef USE_LIBUSB
#include <usb.h>
#endif

#define _GNU_SOURCE
#include <getopt.h>

//#include "qic_control.h"
#include "../../sources/hardware/include/qic/qic_control.h"


#ifdef DMALLOC
#include <dmalloc.h>
#endif

//#undef QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP

char g_runing=1;

void debug_log(int level, char *string) {

  /* debug mesg level */
  char *debug_level[] = {"INFO", "WARN", "CRIT", "FATL", "DEAD"};

  printf("QIC module debug_print (%s):%s", debug_level[level], string);

}

static void usage(FILE * fp, int argc, char **argv)
{
  fprintf(fp,
  "Usage: %s [options]\n\n"
  "Options:\n"
  "-r | --readsys \t read system f/w version\n"
  "-R | --readfile \tread file f/w version\n"
  "-b | --bdir \t set backup dir\n"
  "-f | --usbbin \t USB firmware file name\n"
  "-i | --ispbin \t ISP Parameter file name\n"
  "-s | --BL2ndbin \t 2ns Boot Rom file name\n"
  "-a | --audiobin \t audio calibration data file name\n"
  "-h | --help   Print this message\n"
  "",
  argv[0]);
}


static const char short_options [] = "hrRb:i:s:a:f:";


static const struct option long_options [] =
{
  { "readsys",    no_argument,  NULL, 'r' },
  { "readfile",   no_argument,  NULL, 'R' },
  { "bdir",   required_argument,  NULL, 'b' },
  { "usbbin",   required_argument,  NULL, 'f' },
  { "ispbin",   required_argument,  NULL, 'i' },
  { "BL2ndbin",   required_argument,  NULL, 's' },
  { "audiobin",   required_argument,  NULL, 'a' },
  { "help",   no_argument,    NULL, 'h' },

  { 0, 0, 0, 0 }
};

#ifdef USE_LIBUSB
//find out QIC camera with VID:0408 PID: 0x1802, 0x1806, 0x1822 that mean QIC camera is boot from boot rom
// and it have to erase all of flash partition
int find_camera_by_pid_vid()
{
struct usb_bus *busses ;
struct usb_bus *bus ;
int ret=0;

usb_init();
usb_find_busses();
usb_find_devices();

busses=usb_get_busses();

for (bus = busses; bus; bus = bus->next) {
struct usb_device *dev;

  for (dev = bus->devices; dev; dev = dev->next) {
  struct usb_device_descriptor *desc;

  desc = &(dev->descriptor);
  printf("Vendor/Product ID: %04x,%04x\n", desc->idVendor,desc->idProduct);

    if(desc->idVendor==0x0408){
      if(desc->idProduct==0x1802)
      {
        printf("found 1802\n");
        ret=1;
      }

      if(desc->idProduct==0x1806)
      {
        printf("found 1806\n");
        ret=1;
      }

      if(desc->idProduct==0x1822)
      {
        printf("found 1822\n");
        ret=1;
      }

    return ret;

    }
  }
}

return 0;
}

#endif





int main(int argc,char ** argv)
{

  unsigned short yuv_width = 320;
  unsigned short yuv_height = 240;


  int read_system_version = 0;
  int read_file_version = 0;
  char *backup_dir = NULL;
  char *usb_bin=NULL;
  char *isp_param_bin=NULL;
  char *BL2nd_bin=NULL;
  char *audio_bin=NULL;
  char *backup_BL2nd=NULL;
  char backup_path[512];
  unsigned char update_flags=0;
  qic_module *my_qic = NULL;
  char have_to_erased_all=0;
  unsigned char audio_data[4];
  int backup_audio=0;
  int ret;
  Qic1822HwVer_t qic1822_version;
        QicHwVer_t hw_version;
  SysInfo_t qicSysInfo;
    static struct timespec start_ts,end_ts;
    long diff_sec,diff_nsec;
    double diff_msec;
    unsigned char isLock, isStream;



  qic_dev_name_s video_name;



  if (getuid() != 0){
    printf("please run as root\n");
    exit(0);
  }


  for (;;)
  {
    int index;
    int c;

    c = getopt_long(argc, argv,
                    short_options, long_options,
                    &index);

    if (-1 == c)
      break;

    switch (c)
    {

    case 0: /* getopt_long() flag */
      break;

    case 'r':
      read_system_version = 1;
      break;

    case 'R':
      read_file_version = 1;
      break;

    case 'b':
      backup_dir = optarg;
      break;

    case 'f':
      usb_bin= optarg;
      break;
    case 'i':
      isp_param_bin=optarg;
      break;
    case 's':
      BL2nd_bin=optarg;
      break;
    case 'a':
      audio_bin=optarg;
      break;
    case 'h':
      usage(stdout, argc, argv);
      exit(EXIT_SUCCESS);

    default:
      usage(stderr, argc, argv);
      exit(EXIT_FAILURE);
    }
  }


  memset(&video_name,0, sizeof(video_name));
  ret=qic_enum_device_formats(&video_name);
//  printf("\n QIC1822 encdoing video=%s, raw video=%s\n",video_name.dev_avc, video_name.dev_yuv);
  if(ret){
               printf("\033[1;31m Not supported camera !\033[m\n");
         return 0;

  }


/************************************************
 *
 * first step, init the qic module capture library
 * two devices, /dev/video0 as YUV raw
 *                   /dev/video1 as H.264/AVC
 *
 *************************************************/
  my_qic = qic_initialize(1);

  if (my_qic == NULL) {
    printf("qic_initialize error\n");
    return 1;
  }

#ifdef USE_LIBUSB
  have_to_erased_all=find_camera_by_pid_vid();
#endif

/************************************************
 *
 * step 2: set the parameters and commit the settings
 * need to setup two call back functions, debug_print & frame_output
 *
 *************************************************/
  /* call back functions */
  my_qic->debug_print = &debug_log;

  /*  set scheduler */
  my_qic->high_prio = 1;



     if(strlen(video_name.dev_yuv)>0)
  my_qic->cam[0].dev_name = video_name.dev_yuv;
     else
        my_qic->cam[0].dev_name ="/dev/video0";

  my_qic->cam[0].format = V4L2_PIX_FMT_YUYV;
  my_qic->cam[0].width = yuv_width;
  my_qic->cam[0].height = yuv_height;
  my_qic->cam[0].is_bind = 1;
  my_qic->cam[0].num_mmap_buffer = 0; /*  less memory */

  ret=qic_force_config();
  if (ret) {
    printf("\033[1;31m qic cmaera configure failed \033[m\n");
    return 1;
  }





/************************************************
 *
 * step 3: update the firmware
 *
 *************************************************/

  version_info_t qic_info;


  //jennifer : check return status and response respectively
  ret =  QicGetHWVersion(&hw_version);
  if(ret){
    printf("\n\033[1;31mGetting firmware version failed\033[m\n");
    return 1;
  }

  if(backup_dir){
    printf("\n");
    printf("\nfirmware backuping .............................................\n");
    ret= qic_update_firmware(DEV_ID_0, NULL, 0, backup_dir, USB_FIRMWARE|ISP_FIRMWARE|PARAM_FIRMWARE|AUDIO_FIRMWARE,hw_version);
    if(!ret)
      printf("\n\033[1;32mfirmware backup success\033[m\n");
    else{
      printf("\n\033[1;31mfirmware backup failed\033[m\n");
      //jennifer : response respectively
      return 1;
    }

#ifdef QIC_SUPPORT_2ND_BL
      if(BL2nd_bin!=NULL)
        backup_BL2nd=BL2nd_bin;
      else {
        snprintf(backup_path, sizeof(backup_path), "%s/%s", backup_dir, "QIC1822A_2ndBL.bin");
        backup_BL2nd=backup_path;
      }

      ret=qic_backup_2nd_Boot_Rom(DEV_ID_0,backup_BL2nd );
      if(!ret)
      printf("\n\033[1;32m2nd Boot Rom backup success\033[m\n");
    else
      printf("\n\033[1;31m2nd Boot Rom backup failed\033[m\n");
#endif

    return 0;
  }

  printf("\n\nUSB bin file:%s\n",usb_bin);
        printf("ISP bin file:%s\n\n",isp_param_bin);

  if (read_system_version) {
    //jennifer : check return status and response respectively
    ret = qic_get_system_version(DEV_ID_0, &qic_info);
    if(ret){
      printf("\n\033[1;31mGetting system version failed\033[m\n");
      return 1;
    }

    //jennifer : check return status and response respectively
    ret = qic_get_sysinfo(DEV_ID_0,&qicSysInfo);
    if(ret){
      printf("\n\033[1;31mGetting system info failed\033[m\n");
      return 1;
    }

    if(hw_version==QIC_HWVERSION_QIC1822){
      //jennifer : check return status and response respectively
      ret = QicGet1822HWVersion(&qic1822_version);
      if(ret){
        printf("\n\033[1;31mGetting 1822 hardware version failed\033[m\n");
        return 1;
      }

      if(qic1822_version==QIC1822MP_VERSION)
        printf("\n \033[1;33m QIC1822 MP version\033[m \n");
      else if(qic1822_version==QIC1822MPW_VERSION)
        printf("\n \033[1;33m QIC1822 MPW version\033[m \n");
      else
        printf("\n \033[1;33m Not QIC1822 version\033[m \n");
    }else if(hw_version==QIC_HWVERSION_QIC1806){
      printf("\n \033[1;33m QIC1806 version\033[m \n");
    }else if(hw_version==QIC_HWVERSION_QIC1802){
      printf("\n \033[1;33m QIC1802 version\033[m \n");
    }else
      printf("\n \033[1;33m unknow HW version\033[m \n");


    printf("\nQIC(system) VID:0x%.4lX PID:0x%.4lX REV:0x%.4lX svn:(%d) \n", qicSysInfo.sVID, qicSysInfo.sPID, qicSysInfo.sREV, qicSysInfo.sSVN);
    printf("\nQICHW:%.5lX VID:0x%.4lX PID:0x%.4lX REV:0x%.4lX svn(%d), fw_api_version(%d), update dir=%s \n",qic_info.hw_version, qic_info.vid, qic_info.pid, qic_info.revision,qic_info.svn,qic_info.fw_api_version,usb_bin);

    return 0;
  }

  if(read_file_version && (usb_bin != NULL)) {
    //jennifer : check return status and response respectively
    ret = qic_get_image_version_by_filename(usb_bin, &qic_info);
    if(ret){
      printf("\n\033[1;31mGetting firmware version from file failed\033[m\n");
      return 1;
    }

    printf("\nQIC(file) HW:%.5lX VID:0x%.4lX PID:0x%.4lX REV:0x%.4lX svn(%d), fw_api_version(%d), update dir=%s \n",qic_info.hw_version, qic_info.vid, qic_info.pid,
      qic_info.revision,qic_info.svn,qic_info.fw_api_version,usb_bin);

    return 0;
  }

  ret=qic_get_sysinfo(DEV_ID_0,&qicSysInfo);
  if(!ret){
    printf("\033[1;33mQIC (Current Camera Version) VID:0x%.4x PID:0x%.4x REV:0x%.4x \033[m\n", qicSysInfo.sVID, qicSysInfo.sPID,qicSysInfo.sREV);

    /*checking QIC is streaming?*/
    ret=qic_check_lock_stream_status(&isLock, &isStream);

    if(!ret&& isStream){
      printf("\n\033[1;31mQIC is streaming now, please upgrade firmware later !\033[m\n");
      return FW_IS_STREAMING;
    }
  }else
    printf("\033[1;31mEnter camera recovery mode!, boot from boot rom!\033[m\n");



#ifdef QIC_SUPPORT_2ND_BL

  if(BL2nd_bin){

    if(have_to_erased_all){
#ifdef QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP

      QicFlashRead(QIC1816_FLASH_AUDIO_PARAM_ADDR, audio_data, 4, QIC1822_FLASH_MAX_SIZE);

      if(audio_data[0]==0xff&&audio_data[1]==0xff&&audio_data[2]==0xff&&audio_data[3]==0xff)
      {
        backup_audio=0;  // audio calibration data is not exist
        printf("\n\033[1;31madudio data is not exist, please re-download!\033[m\n");
        usleep(1000*1000*1);
      }else{
        backup_audio=1;  // audio calibration data is  exist
      }

      if(backup_audio){
        // due to update 2nd boot rom will rease all of sector of flash, so audio calibration data have to backup before update 2nd boot rom
        // backup audio data to current folder
        printf("\n\033[1;33m;Backup Audio Calibration data to ./QIC1822A_AUDIO_DATA.bin ....\033[m\n");
        ret= qic_update_firmware(DEV_ID_0, NULL, 0, ".", AUDIO_FIRMWARE,hw_version);

        if(!ret)
          printf("\n\033[1;32mbackup audio data success!\033[m\n");
        else{
          printf("\n\033[1;31mbackup adudio data failed!\033[m\n");
          return 0;
        }
      }
#endif
    }
    printf("\n");
    printf("\nStart 2nd Boot Rom upgrade ....\n");

    qic_set_lock_steam_control( 1);

    if(have_to_erased_all)
           ret=qic_update_2nd_bootrom_by_filename(DEV_ID_0,1, BL2nd_bin);
    else
    ret=qic_update_2nd_bootrom_by_filename(DEV_ID_0,0, BL2nd_bin);

             printf("\nRESULT:");
    if(!ret)
      printf("\n\033[1;32m2nd Boot Rom update success!\033[m\n");
    else{
      qic_set_lock_steam_control( 0);
      printf("\n\033[1;31m2nd Boot Rom update failed!\033[m\n");

      return 0;
    }

   if(have_to_erased_all){
#ifdef QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP
    if(backup_audio)
    ret=qic_update_audio_data_by_filename(DEV_ID_0,"./QIC1822A_AUDIO_DATA.bin");
#endif
   }
   qic_set_lock_steam_control( 0);

   if(have_to_erased_all){
#ifdef QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP
     printf("\nRESULT:");
     if(!ret)
       printf("\n\033[1;32maudio data update success!\033[m\n");
     else{
       printf("\n\033[1;31maudio data update failed!\033[m\n");
       return 0;
     }
#endif
   }

   if(!usb_bin){
     if(!ret){
       QicReset();
     }

     ret = qic_release();
     if (ret) {
       printf("qic_release error\n");
       return 1;
     }

     return 0;
   }
  }
#endif


#ifdef QIC_SUPPORT_AUDIO_CALIBRATION_BACKUP
  if(audio_bin){

    printf("\n\033[1;32maudio file=%s \033[m\n",audio_bin);
    qic_set_lock_steam_control( 1);
    ret=qic_update_audio_data_by_filename(DEV_ID_0,audio_bin);
    qic_set_lock_steam_control( 0);

    printf("\nRESULT:");
    if(!ret)
      printf("\n\033[1;32maudio data update success!!!\033[m\n");
    else{
      printf("\n\033[1;31maudio data update failed!!!\033[m\n");
      return 0;
    }
  }
#endif

  if(usb_bin||isp_param_bin){

    printf("usb bin file=%s, isp bin file=%s\n",usb_bin,isp_param_bin);

    if(usb_bin)
      update_flags=update_flags|USB_FIRMWARE;

    if(isp_param_bin)
      update_flags=update_flags|PARAM_FIRMWARE;

    if(hw_version==QIC_HWVERSION_QIC1822){
      //jennifer : check return status and response respectively
      ret = QicGet1822HWVersion(&qic1822_version);
      if(ret){
        printf("\n\033[1;31mGetting 1822 hardware version failed\033[m\n");
        return 1;
      }

      if(qic1822_version==QIC1822MP_VERSION)
        printf("\n \033[1;33m QIC1822 MP version\033[m \n");
      else if(qic1822_version==QIC1822MPW_VERSION)
        printf("\n \033[1;33m QIC1822 MPW version\033[m \n");
      else
        printf("\n \033[1;33m Not QIC1822 version\033[m \n");

    }else if(hw_version==QIC_HWVERSION_QIC1806){
      printf("\n \033[1;33m QIC1806 version\033[m \n");
    }else if(hw_version==QIC_HWVERSION_QIC1802){
      printf("\n \033[1;33m QIC1802 version\033[m \n");
    }else
      printf("\n \033[1;33m unknow HW version\033[m \n");

    ret=qic_get_sysinfo(DEV_ID_0,&qicSysInfo);
    if(!ret){
      printf("\033[1;33mQIC (Current Camera Version) VID:0x%.4x PID:0x%.4x REV:0x%.4x \033[m\n", qicSysInfo.sVID, qicSysInfo.sPID,
        qicSysInfo.sREV);
    }else
      printf("\033[1;31mEnter camera recovery mode!!\033[m\n");


    //jennifer : check return status and response respectively
    ret = qic_get_image_version_by_filename(usb_bin, &qic_info);
    if(ret){
      printf("\n\033[1;31mGetting firmware version from file failed\033[m\n");
      return 1;
    }
    printf("\n \033[1;33m QIC(file)HW:%.5lX VID:0x%.4lX PID:0x%.4lX REV:0x%.4lX  \033[m \n",qic_info.hw_version, qic_info.vid, qic_info.pid,
    qic_info.revision);



    clock_gettime(0,&start_ts);
    printf("\n");
    printf("\nStart firmware upgrade ....\n");
    qic_set_lock_steam_control( 1);
    ret=qic_update_firmware_by_filename(DEV_ID_0, usb_bin,NULL,isp_param_bin,update_flags,hw_version);
    qic_set_lock_steam_control( 0);
    g_runing=0;
    clock_gettime(0,&end_ts);

    printf("\nRESULT:");
    if(!ret)
      printf("\n\033[1;32mfirmware update success!\033[m\n");
    else{

      if(ret==FW_NO_2ND_BL)
        printf("\n\033[1;31m2nd boot rom no found, please download it !\033[m\n");
      else if(ret==FW_IS_STREAMING)
        printf("\n\033[1;31mQIC is streaming now, please upgrade firmware later !\033[m\n");
      else
      printf("\n\033[1;31mfirmware update failed!\033[m\n");

      return 0;
    }
    diff_sec= end_ts.tv_sec-start_ts.tv_sec;
    diff_nsec=end_ts.tv_nsec-start_ts.tv_nsec;
    diff_msec=diff_nsec/1000000;
    printf("elapsed time= %u.%u sec \n", (unsigned int)diff_sec,(unsigned char)diff_msec);

    if(!ret){
      //jennifer : check return status and response respectively
      ret = QicReset();
      if(ret){
        printf("\n\033[1;31mReset QIC failed\033[m\n");
        return 1;
      }
    }

    ret = qic_release();
    if (ret) {
      printf("qic_release error\n");
      return 1;
    }

    if(!ret){
      printf("\n\nre-boot camera...\n");

    usleep(3000*1000);

    qic_dev_name_s video_name;
    memset(&video_name,0, sizeof(video_name));
    qic_enum_device_formats(&video_name);
    //  printf("\n QIC1822 encdoing video=%s, raw video=%s\n",video_name.dev_avc, video_name.dev_yuv);

    my_qic = qic_initialize(1);

    if (my_qic == NULL) {
      printf("qic_initialize error\n");
      return 1;
    }

    /* call back functions */
    my_qic->debug_print = &debug_log;

    /*  set scheduler */
    my_qic->high_prio = 1;

    if(strlen(video_name.dev_yuv)>0)
      my_qic->cam[0].dev_name = video_name.dev_yuv;
    else
      my_qic->cam[0].dev_name ="/dev/video0";

    my_qic->cam[0].format = V4L2_PIX_FMT_YUYV;
    my_qic->cam[0].width = yuv_width;
    my_qic->cam[0].height = yuv_height;
    my_qic->cam[0].is_bind = 1;
    my_qic->cam[0].num_mmap_buffer = 0; /*  less memory */

    //qic_force_config();
    /* commit and init the video dev */
    ret=qic_force_config();
    if (ret) {
      printf("\033[1;31m qic cmaera configure failed \033[m\n");
      return 1;
    }

    QicGetHWVersion(&hw_version);

    if(hw_version==QIC_HWVERSION_QIC1822){
     QicGet1822HWVersion(&qic1822_version);
        if(qic1822_version==QIC1822MP_VERSION)
              printf("\n \033[1;33m QIC1822 MP version\033[m \n");
      else if(qic1822_version==QIC1822MPW_VERSION)
              printf("\n \033[1;33m QIC1822 MPW version\033[m \n");
      else
        printf("\n \033[1;33m Not QIC1822 version\033[m \n");
    }else if(hw_version==QIC_HWVERSION_QIC1806){
      printf("\n \033[1;33m QIC1806 version\033[m \n");
    }else if(hw_version==QIC_HWVERSION_QIC1802){
            printf("\n \033[1;33m QIC1802 version\033[m \n");
    }else
         printf("\n \033[1;33m unknow HW version\033[m \n");

    qic_get_system_version(DEV_ID_0, &qic_info);
    printf("\n\n \033[1;33mQIC (New Camera Version) VID:0x%.4lX PID:0x%.4lX REV:0x%.4lX \033[m\n", qic_info.vid, qic_info.pid,
    qic_info.revision);
    printf("\n");

    ret = qic_release();
    if (ret) {
      printf("qic_release error\n");
      return 1;
    }

  }
  }

  return 0;
}
