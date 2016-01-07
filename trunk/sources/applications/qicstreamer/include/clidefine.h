/*
// ==================================================================
// This confidential and proprietary software may be used only as
// authorized by a licensing agreement from Quanta Computer Inc.
//
//            (C) COPYRIGHT Quanta Computer Inc.
//                   ALL RIGHTS RESERVED
//
// ==================================================================
// ------------------------------------------------------------------
// Date    : 2011/05/20
// Version :
// Author  : Tim Hsieh (tim.hsieh@quantatw.com)
// ------------------------------------------------------------------
// Purpose : command and parameter definition
// ==================================================================
*/

#ifndef _CLIDEFINE_H
#define _CLIDEFINE_H

namespace app_qicstreamer {

// QIC Command Definition
#define QIC_CLI_API_VERSION_V1                    "v1"
#define QIC_CLI_REQUEST                           "request"
#define QIC_CLI_ACTION                            "action"
#define QIC_CLI_PARAMETERS                        "parameters"
#define QIC_CLI_JSONP_CALLBACK                    "icallback"

#define QIC_CLI_API_VERSION_V2                    "v2"



// Request Type
#define QIC_CLI_REQ_DEVICE                        "device"
#define QIC_CLI_REQ_VIDEO                         "video"
#define QIC_CLI_REQ_AUDIO                         "audio"
#define QIC_CLI_REQ_RECORD                        "record"
#define QIC_CLI_REQ_IO                            "io"
#define QIC_CLI_REQ_EXTENSION                     "extension"
#define QIC_CLI_REQ_ONVIF                         "onvif"
// Response Type
#define QIC_CLI_RESPONSE_STATUS                   "status"
#define QIC_CLI_RESPONSE_MESSAGE                  "message"
#define QIC_CLI_RESPONSE_DATA                     "data"
#define QIC_CLI_RESPONSE_STATUS_OK                "ok"
#define QIC_CLI_RESPONSE_STATUS_ERROR             "error"

//Device
#define QIC_CLI_DEVICE_HARDWARE                    "hardware"
#define QIC_CLI_DEVICE_SYSTEM                      "system"
#define QIC_CLI_DEVICE_TEST                        "test"
//device hardware API
#define QIC_CLI_DEVICE_HW_DEVICE_INFO              "device_info"
#define QIC_CLI_DEVICE_HW_FW_INFO                  "fw_info"
#define QIC_CLI_DEVICE_HW_CAPABILITIES             "capabilities"
#define QIC_CLI_DEVICE_HW_STATUS                   "status"
//device system API
#define QIC_CLI_DEVICE_SYS_TIMESTAMP               "timestamp"
#define QIC_CLI_DEVICE_SYS_SYSINFO                 "sys_info"
#define QIC_CLI_DEVICE_SYS_NVRAM                   "nvram"
#define QIC_CLI_DEVICE_SYS_DATETIME                "datetime"
#define QIC_CLI_DEVICE_SYS_SETTINGS                "settings"
#define QIC_CLI_DEVICE_SYS_FW_DOWNLOAD             "fw_download"
#define QIC_CLI_DEVICE_SYS_FW_UPGRADE              "fw_upgrade"
#define QIC_CLI_DEVICE_SYS_FACTORY_RESET           "factory_reset"
#define QIC_CLI_DEVICE_SYS_REBOOT                  "reboot"
#define QIC_CLI_DEVICE_SYS_COMMAND                 "command"

//Service stream API
#define QIC_CLI_SERVICE_STREAM                          "stream"
#define QIC_CLI_SERVICE_STREAM_ALL                      "all"
//#define QIC_CLI_SERVICE_STREAM_GET_STREAM_BY_ID         "get_stream_by_id"
//#define QIC_CLI_SERVICE_STREAM_FIND_OUTSTREAMS_BY_ID    "find_outstreams_by_id"
#define QIC_CLI_SERVICE_STREAM_AVC                      "avc"
#define QIC_CLI_SERVICE_STREAM_PUSH_AVC                 "push_avc"
#define QIC_CLI_SERVICE_STREAM_DELETE_AVC               "delete_avc"
#define QIC_CLI_SERVICE_STREAM_MJPG                     "mjpg"
#define QIC_CLI_SERVICE_STREAM_PUSH_MJPG                "push_mjpg"
#define QIC_CLI_SERVICE_STREAM_PCM                      "pcm"
#define QIC_CLI_SERVICE_STREAM_PUSH_PCM                 "push_pcm"
#define QIC_CLI_SERVICE_STREAM_SNAP_JPG                 "snap_jpg"
//Service cloud API
#define QIC_CLI_SERVICE_CLOUD                           "cloud"
#define QIC_CLI_SERVICE_CLOUD_CONNECTION                "connection"
#define QIC_CLI_SERVICE_CLOUD_SETTING                   "setting"
#define QIC_CLI_SERVICE_CLOUD_UPDATE_FS                 "update_fs"
#define QIC_CLI_SERVICE_CLOUD_UPDATE_CA                 "update_ca"
#define QIC_CLI_SERVICE_CLOUD_WEBSOCKETINFO             "websocket_info"
#define QIC_CLI_SERVICE_CLOUD_AUTH                      "auth"
#define QIC_CLI_SERVICE_CLOUD_DEAUTH                    "deauth"
#define QIC_CLI_SERVICE_CLOUD_UNBIND                    "unbind"

// Video
#define QIC_CLI_VIDEO_ACTION_SNAPSHOT                   "snapshot"
#define QIC_CLI_VIDEO_ACTION_GET_KEY_FRAME              "get_key_frame"
#define QIC_CLI_VIDEO__STATUS                           "get_status"

#define QIC_CLI_VIDEO_MJPG                             "mjpg"
#define QIC_CLI_VIDEO_AVC                              "avc"
#define QIC_CLI_VIDEO_COMMON                           "common"

#define QIC_CLI_VIDEO_MJPG_CAPTURE                     "capture"
#define QIC_CLI_VIDEO_MJPG_STREAM                      "stream"
#define QIC_CLI_VIDEO_MJPG_RESOLUTION                  "resolution"
#define QIC_CLI_VIDEO_MJPG_FPS                         "fps"
#define QIC_CLI_VIDEO_MJPG_SETTING                     "setting"
#define QIC_CLI_VIDEO_MJPG_DEFAULT_SETTING             "default_setting"
#define QIC_CLI_VIDEO_MJPG_CONSTRAINTS                 "constraints"

#define QIC_CLI_VIDEO_AVC_CAPTURE                      "capture"
#define QIC_CLI_VIDEO_AVC_STREAM                       "stream"
#define QIC_CLI_VIDEO_AVC_RESOLUTION                   "resolution"
#define QIC_CLI_VIDEO_AVC_FPS                          "fps"
#define QIC_CLI_VIDEO_AVC_BITRATE                      "bitrate"
#define QIC_CLI_VIDEO_AVC_GOP                          "gop"
#define QIC_CLI_VIDEO_AVC_SETTING                      "setting"
#define QIC_CLI_VIDEO_AVC_DEFAULT_SETTING              "default_setting"
#define QIC_CLI_VIDEO_AVC_CONSTRAINTS                  "constraints"
#define QIC_CLI_VIDEO_AVC_CAPABILITIES                 "capabilities"
#define QIC_CLI_VIDEO_AVC_OSD                          "osd"
#define QIC_CLI_VIDEO_AVC_CLEAR_ALL_OSD                "clear_all_osd"
#define QIC_CLI_VIDEO_AVC_GENERATE_KEY_FRAME           "gen_key_frame"
#define QIC_CLI_VIDEO_AVC_MMIO                         "mmio"

#define QIC_CLI_VIDEO_COMMON_BRIGHTNESS                "brightness"
#define QIC_CLI_VIDEO_COMMON_CONTRAST                  "contrast"
#define QIC_CLI_VIDEO_COMMON_HUE                       "hue"
#define QIC_CLI_VIDEO_COMMON_SATURATION                "saturation"
#define QIC_CLI_VIDEO_COMMON_SHARPNESS                 "sharpness"
#define QIC_CLI_VIDEO_COMMON_GAMMA                     "gamma"
#define QIC_CLI_VIDEO_COMMON_BACKLIGHT_COMPENSATION    "backlight_compensation"
#define QIC_CLI_VIDEO_COMMON_FLIPMODE                  "flip"
#define QIC_CLI_VIDEO_COMMON_SETTING                   "setting"
#define QIC_CLI_VIDEO_COMMON_DEFAULT_SETTING           "default_setting"
#define QIC_CLI_VIDEO_COMMON_CONSTRAINTS               "constraints"
#define QIC_CLI_VIDEO_COMMON_PT_POSITION               "pt_position"
#define QIC_CLI_VIDEO_COMMON_FW_UPGRADE                "fw_upgrade"
#define QIC_CLI_VIDEO_COMMON_Y_VALUE                   "y_value"


// Audio microphone API
#define QIC_CLI_AUDIO_MIC                "mic"
#define QIC_CLI_AUDIO_MIC_STREAM         "stream"
#define QIC_CLI_AUDIO_MIC_DEVICE         "device"
#define QIC_CLI_AUDIO_MIC_SAMPLE_RATE    "sample_rate"

// Audio speaker API
#define QIC_CLI_AUDIO_SPEAKER              "speaker"
#define QIC_CLI_AUDIO_SPEAKER_STREAM       "stream"
#define QIC_CLI_AUDIO_SPEAKER_FILE         "file"
//#define QIC_CLI_AUDIO_SPEAKER_SIGNALPLAY   "signal_playend"
#define QIC_CLI_AUDIO_SPEAKER_PLAYBACK     "playback"
#define QIC_CLI_AUDIO_SPEAKER_DEVICE       "device"
#define QIC_CLI_AUDIO_SPEAKER_VOLUME       "volume"

//Event
#define QIC_CLI_EVENT_AUDIO                 "audio"
#define QIC_CLI_EVENT_AUDIO_ALARM           "alarm"
#define QIC_CLI_EVENT_AUDIO_RMS             "rms"
#define QIC_CLI_EVENT_AUDIO_TRIGGER         "trigger"
#define QIC_CLI_EVENT_PIR                   "pir"
#define QIC_CLI_EVENT_PIR_ALARM             "alarm"
#define QIC_CLI_EVENT_PIR_TRIGGER           "trigger"
#define QIC_CLI_EVENT_RECORD                "record"
#define QIC_CLI_EVENT_RECORD_DURATION       "duration"
#define QIC_CLI_EVENT_RECORD_RETENTION      "retention"
#define QIC_CLI_EVENT_STREAMER              "streamer"
#define QIC_CLI_EVENT_STREAMER_SIGNAL       "signal"
#define QIC_CLI_EVENT_STREAMER_SIGFILE      "sig_file"
#define QIC_CLI_EVENT_STREAMER_SIGUPLOAD    "sig_upload"
#define QIC_CLI_EVENT_STREAMER_SIGDELFILE   "sig_delfile"

// NETWORK
#define QIC_CLI_NETWORK_MANAGEMENT                     "management"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_INTERFACES       "interfaces"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_EXTERNAL_IP      "external_ip"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_DNS              "dns"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_DEFAULT_ROUTER   "default_router"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_NTP_SERVER       "ntp_server"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_IP_SETTING       "ip_setting"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_CLIENT_LIST      "client_list"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_PING             "ping"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_TRACEROUTE       "traceroute"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_throughput_test  "throughput_test"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_IP_FILTERS       "ip_filters"
#define QIC_CLI_NETWORK_MANAGEMENT_ID_STATISTICS       "statistics"

#define QIC_CLI_NETWORK_WIFI                           "wifi"
#define QIC_CLI_NETWORK_WIFI_ID_SECURITY               "security"
#define QIC_CLI_NETWORK_WIFI_ID_AVAILABLE_INTERFACES   "available_interfaces"
#define QIC_CLI_NETWORK_WIFI_ID_DO_WPS                 "do_wps"
#define QIC_CLI_NETWORK_WIFI_ID_SITE_SURVEY            "site_survey"
#define QIC_CLI_NETWORK_WIFI_ID_SETTINGS               "settings"

// IO
#define QIC_CLI_IO_LED                            "led"
#define QIC_CLI_IO_LED_ID_STATUS                  "status"
#define QIC_CLI_IO_LED_ID_EVENT                   "event"

#define QIC_CLI_IO_PT                             "pt"
#define QIC_CLI_IO_PT_SUPPORT                     "support"
#define QIC_CLI_IO_PT_POSITION                    "position"
#define QIC_CLI_IO_PT_ROTATE                      "rotate"
#define QIC_CLI_IO_PT_STOP                        "stop"
#define QIC_CLI_IO_PT_TAG_LIST                    "tag_list"
#define QIC_CLI_IO_PT_TAG                         "tag"
#define QIC_CLI_IO_PT_TAG_REPLACE                 "tag_replace"
#define QIC_CLI_IO_PT_TAG_RENAME                  "tag_rename"
#define QIC_CLI_IO_PT_TAG_DELETE                  "tag_delete"
#define QIC_CLI_IO_PT_TAG_LIST_CLEAR              "tag_list_clear"
#define QIC_CLI_IO_PT_NAVIGATE                    "navigate"
#define QIC_CLI_IO_PT_NAVIGATE_TO                 "navigate_to"
#define QIC_CLI_IO_PT_RESET                       "reset"


#define QIC_CLI_IO_PIR                            "pir"
#define QIC_CLI_IO_PIR_STATUS                     "status"
#define QIC_CLI_IO_IR_LED_STATUS                  "status"
#define QIC_CLI_IO_IR_LED                         "ir"
#define QIC_CLI_IO_ALS                            "als"
#define QIC_CLI_IO_ALS_VALUE                      "lux"




// Record
//#define QIC_CLI_RECORD_API_VERSION                "v1"
#define QIC_CLI_RECORD_ACTION_START               "start"
#define QIC_CLI_RECORD_ACTION_STOP                "stop"
#define QIC_CLI_RECORD_ACTION_MIICASA_START       "miicasastart"
#define QIC_CLI_RECORD_ACTION_MIICASA_STOP        "miicasastop"
#define QIC_CLI_RECORD_ACTION_MIICASA_SNAPSHOT    "miicasasnapshot"
#define QIC_CLI_RECORD_ACTION_GET_STATUS          "get_status"
#define QIC_CLI_RECORD_ACTION_SET_PARAMETERS      "set_parameters"
#define QIC_CLI_RECORD_ACTION_GET_STORAGES        "get_storages"
#define QIC_CLI_RECORD_ACTION_PIRALERT            "alert_pir"
#define QIC_CLI_RECORD_ACTION_GET_FILELIST        "get_fileslist"
#define QIC_CLI_RECORD_ACTION_SET_EVT_DURATION    "set_evt_fileduration"
#define QIC_CLI_RECORD_ACTION_GET_EVT_DURATION    "get_evt_fileduration"
#define QIC_CLI_RECORD_ACTION_CREATE_THREAD       "create_thread"
#define QIC_CLI_RECORD_ACTION_DUMP_AVC            "dumpavc"

// IVA
#define QIC_CLI_IVA_MD                       "md"
//#define QIC_CLI_IVA_MD_START                      "start"
//#define QIC_CLI_IVA_MD_STOP                       "stop"
#define QIC_CLI_IVA_MD_STATUS                     "status"
#define QIC_CLI_IVA_MD_FPSCONTROL                 "fpscontrol"
#define QIC_CLI_IVA_MD_ROI                        "roi"
#define QIC_CLI_IVA_MD_EVENT                      "event"
#define QIC_CLI_IVA_PD                       "pd"
#define QIC_CLI_IVA_PD_STATUS                     "status"

// Extension
//#define QIC_CLI_EXT_API_VERSION                   "v1"
#define QIC_CLI_EXT_ACTION_TESTJSON               "testjson"


// Request Type
#define QIC_CLI_REQ_TEST                          "test"
#define QIC_CLI_IO_TEST                           "test"
#define QIC_CLI_IO_TEST_COMMAND                   "cmd"
#define QIC_CLI_IO_TEST_LED                       "led"
#define QIC_CLI_IO_TEST_BUTTON                    "button"
#define QIC_CLI_IO_TEST_SWITCH                    "switch"
#define QIC_CLI_IO_TEST_SD                        "sd"
#define QIC_CLI_IO_TEST_FLASH                     "flash"


// CLI macro
//#define ACTION_RETURN_MSG(r, k, v) r[k]=v;
//#define ACTION_STATUS_OK(r, m)     r[QIC_CLI_RESPONSE_STATUS]=QIC_CLI_RESPONSE_STATUS_OK;ACTION_RETURN_MSG(r, QIC_CLI_RESPONSE_MESSAGE, m);
//#define ACTION_STATUS_ERROR(r, m)  r[QIC_CLI_RESPONSE_STATUS]=QIC_CLI_RESPONSE_STATUS_ERROR;ACTION_RETURN_MSG(r, QIC_CLI_RESPONSE_MESSAGE, m);
//#define ACTION_RETURN_DATA(r,m,v)  ACTION_STATUS_OK(r, m);r[QIC_CLI_RESPONSE_DATA]=v;

}



#endif
