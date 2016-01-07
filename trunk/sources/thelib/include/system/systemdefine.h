/* ==================================================================
 * This confidential and proprietary software may be used only as
 * authorized by a licensing agreement from Quanta Computer Inc.
 *
 * (C) COPYRIGHT Quanta Computer Inc. ALL RIGHTS RESERVED
 *
 * ==================================================================
 * ------------------------------------------------------------------
 *  Date    :
 *  Version :
 *  Author  : Tim Hsieh (tim.hsieh@quantatw.com)
 *  Purpose :
 *  =================================================================
 */


#ifndef _SYSTEMDEFINE_H
#define _SYSTEMDEFINE_H
// system command
#define ICAM_INIT "icam_init"
#define ICAM_START "icam_start"
#define ICAM_GET_NVRAM "icam_get_nvram"
#define ICAM_SET_NVRAM "icam_set_nvram"

// network
#define DEVICE_NETWORK_INTERFACE "interface"
#define DEVICE_NETWORK_IP "ip"
#define DEVICE_NETWORK_MAC "mac"
#define DEVICE_NETWORK_SSID "ssid"
#define DEVICE_NETWORK_EXTERNAL "external"
#define DEVICE_NETWORK_INTERFACES "interfaces"

// device info
#define DEVICE_SERIAL_NO "serialNo"
#define DEVICE_CUID "cuid"
#define DEVICE_HASH_CODE "hashCode"
#define DEVICE_PRIVATE_KEY "privateKey"

#define DEVICE_VENDOR "vendor"
#define DEVICE_HARDWARE_MODEL "model"
#define DEVICE_HARDWARE_VERSION "version"

#define DEVICE_FIRMWARE_IMAGE "image"
#define DEVICE_FIRMWARE_SDK "sdk"
#define DEVICE_FIRMWARE_DSP "dsp"
#define DEVICE_FIRMWARE_APPLICATION "application"
#define DEVICE_FIRMWARE_SCRIPT "script"

// config
#define DEVICE_APP_CONFIG "application"
#define DEVICE_HW_CONFIG "hardware"

// unix domain socket thread
#define UX_SOCKET_EVENT                     "sock.event"    // for protocol
#define UX_SOCKET_SYSTEM                    "sock.system"     // for callback
#define UX_SOCKET_CVR                       "sock.cvr"        // for cvr
#define UX_SOCKET_INFO                      "sock.info"    // for protocol
#define UX_SOCKET_IPC_EVENT                 "sock.ipc.event"

//cvr thread ipc
#define VIDEO_AVC_DATA                     0x00
#define AUDIO_PCM_DATA                     (VIDEO_AVC_DATA|0x01)
#define START_CVR_RECORD                   0x80 //len, directory
#define STOP_CVR_RECORD                    0x81 //none
#define CVR_SPS                            0x82 //sps len, sps data,
#define CVR_PPS                            0x83 //pps len, pps data
#define CVR_AUDIO_CONFIG                   0x84 //4, 1, 1, 2
#define CVR_MAX_FRAMES                     0x85 //4 bytes
#define CVR_RESOLUTION                     0x86 //2 bytes width, 2bytes height
//info thread ipc
#define INFO_OP_SYSTEMINFO                 0x00000000
#define INFO_OP_CMD                        0x80000000
#define INFO_EVT_ID                        "eventId"
#define INFO_EVT_TYPE                      "eventType"
#define INFO_SRC_FILE                      "srcFile"
#define INFO_SRC_DIR                       "srcDir"
#define INFO_TAR_DIR                       "tarDir"
#define INFO_TAR                           "tar"



#endif /* _SYSTEMDEFINE_H */
