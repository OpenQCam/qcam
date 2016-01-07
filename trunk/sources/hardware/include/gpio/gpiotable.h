// ==================================================================
// // This confidential and proprietary software may be used only as
// // authorized by a licensing agreement from Quanta Computer Inc.
// //
// //            (C) COPYRIGHT Quanta Computer Inc.
// //                   ALL RIGHTS RESERVED
// //
// // ==================================================================
// // ------------------------------------------------------------------
// // Date
// // Version :
// // Author  :
// // ------------------------------------------------------------------
// // Purpose :
// // ======================================================================
//
#ifndef _GPIOTABLE_H
#define _GPIOTABLE_H

#define AP_STA_SWITCH

typedef signed char             cei8;
typedef unsigned char           ceu8;
typedef signed short            cei16;
typedef unsigned short          ceu16;
typedef signed long             cei32;
typedef unsigned long           ceu32;
typedef signed long long        cei64;
typedef unsigned long long      ceu64;
typedef float                   cefloat;

// use the following two types when integer size is not that important
typedef cei32                   cei;
typedef ceu32                   ceu;

struct gpio_status {
  bool enablePIR;
  bool enableALS;
};

/* GPIO ommand */
/* check the commands in SDK/linux-2.6.21.x/drivers/char/ce_gpio/ce_gpio.c */
//       IOCTL INDEX           NAME
#define CMD_LL_DIR_SET      "lldirset"
#define CMD_LL_DIR_READ     "lldirread"
#define CMD_LL_POL_SET      "llpolwrite"
#define CMD_LL_POL_READ     "llpolread"
#define CMD_LL_WRITE        "llwrite"
#define CMD_LL_READ         "llread"
#define CMD_LL_CLEAR        "llclear"
#define CMD_LL_SET          "llset"
#define CMD_LL_MODE_SET     "llmodset"
#define CMD_WRITE_BIT       "write"
#define CMD_CLEAR_BIT       "clear"
#define CMD_SET_BIT         "set"
#define CMD_GET_BIT         "get"
#define CMD_LED_SET         "led"
#define CMD_BUTTON_SET      "buttonset"
#define CMD_BUTTON_GET      "buttonget"
#define CMD_BUTTON_RESET    "buttonrst"
#define CMD_BUTTONS_GET     "buttonsget"
#define CMD_DEBUG           "debug"
#define CMD_STATUS          "status"
#define CMD_STOP_START      "timeronoff"
#define CMD_CHANGE_INTERVAL "interval"
#define CMD_PN_GET          "pnget"
#define CMD_GCP_PRINT       "gcp"

/* GPIO table, move to lua scripts */
//      Function                     Number
#ifdef AP_STA_SWITCH
#define GPIO_NUM_AP_STA_SWITCH        0
#else
//#define GPIO_NUM_IVA_BUTTON           0
#endif

#define GPIO_NUM_WPS_BUTTON           1
#define GPIO_NUM_RESET_BUTTON         2
// NC                                 2
// NC                                 3
#define GPIO_NUM_PIR_INTERRUPT        4
#define GPIO_NUM_ALS_INTERRUPT        5
//#define GPIO_NUM_POWER_LED            6
//#define GPIO_NUM_WLAN_LED             7
//#define GPIO_NUM_LINK_LED             8
//#define GPIO_NUM_AP_STA_LED_B         9
//#define GPIO_NUM_WPS_LED              10
//#define GPIO_NUM_AUDIO_VIDEO_LED_B    11
// NC                                 12
#define GPIO_NUM_IR_LED               13
//#define GPIO_NUM_AP_STA_LED_G         14
//#define GPIO_NUM_AUDIO_VIDEO_LED_G    15

//#define GPIO_NUM_THERMAL_INTERRUPT    16

/* LED type */
#define LED_TYPE_POWER             "power"
#define LED_TYPE_WLAN              "wlan"
#define LED_TYPE_LINK              "link"
#define LED_TYPE_WPS               "wps"
#define LED_TYPE_AP_STA_B          "ap_sta_b"
#define LED_TYPE_AP_STA_G          "ap_sta_g"
#define LED_TYPE_AUDIO_VIDEO_B     "audio_video_b"
#define LED_TYPE_AUDIO_VIDEO_G     "audio_video_g"
#define LED_TYPE_IR                "ir"


/* GPIO pipe signal (uint32_t) */
#ifdef AP_STA_SWITCH
#define GPIO_SIGNAL_AP_SWITCH         0x01
#define GPIO_SIGNAL_STA_SWITCH       (0x01 << 1)
#else
#define GPIO_SIGNAL_IVA_BUTTON        0x01
#endif
#define GPIO_SIGNAL_WPS_BUTTON       (0x01 << 2)
#define GPIO_SIGNAL_PIR              (0x01 << 3)
#define GPIO_SIGNAL_ALS              (0x01 << 4)
#define GPIO_SIGNAL_LAN_CONNECTED    (0x01 << 5)
#define GPIO_SIGNAL_LAN_DISCONNECTED (0x01 << 6)
#define GPIO_SIGNAL_WIFI_ENABLED     (0x01 << 7)
#define GPIO_SIGNAL_WIFI_DISABLED    (0x01 << 8)
#define GPIO_SIGNAL_WPS_SUCCESS      (0x01 << 9)
#define GPIO_SIGNAL_WPS_TIME_OUT     (0x01 << 10)
#define GPIO_SIGNAL_REBOOT           (0x01 << 11)
#define GPIO_SIGNAL_FACTORY_RESET    (0x01 << 12)

#endif  /* _GPIOTABLE_H */
