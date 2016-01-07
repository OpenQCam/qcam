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

#ifndef _AVSTREAMTYPE_H
#define _AVSTREAMTYPE_H


// QIC Stream Definition, max 8 character
#define QIC_STREAM_CURRENT_API_VERSION            "v2"
#define QIC_STREAM_STREAM_TYPE                    "streamtype"
#define QIC_STREAM_TYPE_AVC                       MAKE_TAG3('A','V','C')
#define QIC_STREAM_TYPE_MJPG                      MAKE_TAG4('M','J','P','G')
#define QIC_STREAM_TYPE_JPEG                      MAKE_TAG4('J','P','E','G')
#define QIC_STREAM_TYPE_MP4FILE                   MAKE_TAG7('M','P','4','F','I','L','E')
#define QIC_STREAM_TYPE_G711                      MAKE_TAG4('G','7','1','1')
#define QIC_STREAM_TYPE_WAVE                      MAKE_TAG4('W','A','V','E')
#define QIC_STREAM_TYPE_AAC                       MAKE_TAG3('A','A','C')
#define QIC_STREAM_TYPE_AMR                       MAKE_TAG3('A','M','R')
#define QIC_STREAM_TYPE_OPUS                      MAKE_TAG4('O','P','U','S')
#define QIC_STREAM_TYPE_MP3                       MAKE_TAG3('M','P','3')

// Video
#define QIC_STREAM_API_VERSION                    "apiversion"
#define QIC_STREAM_REQUEST                        "request"
#define QIC_STREAM_NAME                           "streamname"
#define QIC_STREAM_PARAMETERS                     "parameters"

#define QIC_STREAM_AVC                            "avcstream"
#define QIC_STREAM_RELAYAVC                       "relayavcstream"
#define QIC_STREAM_MJPG                           "mjpgstream"
#define QIC_STREAM_MJPG_FPS                       "fps"
#define QIC_STREAM_SNAPSHOT                       "snapshot"
#define QIC_STREAM_MP4FILE                        "mp4filestream"
#define QIC_STREAM_MP4ALARMFILE                   "mp4filealarmstream"
#define QIC_STREAM_CVRFILE                        "cvrfilestream"
#define QIC_STREAM_JPGFILE                        "jpgfilestream"
#define QIC_STREAM_JPG                            "jpgoutstream"
#define QIC_STREAM_MP4_FILENAME                   "file"
#define QIC_STREAM_MUX                            "avmuxstream"
#define QIC_STREAM_RELAYMUX                       "avrelaymuxstream"
#define QIC_STREAM_MOTION                         "motionstream"

// Audio name, client will use this name to find the corresponding stream
#define QIC_STREAM_G711                           "g711stream"
#define QIC_STREAM_WAVE                           "wavestream"
#define QIC_STREAM_AAC                            "aacstream"
#define QIC_STREAM_OPUS                           "opusstream"
#define QIC_STREAM_MP3                            "mp3stream"
#define QIC_STREAM_WAVEPLAY                       "waveplaystream"
#define QIC_STREAM_WAVEFILE                       "wavefilestream"

// System message stream
#define QIC_STREAM_SYSTEMMSG                      "sysinfostream"
// WEBSOCKET output stream name
#define QIC_WS_OUTSTREAM                          "wsoutnetstream"

#endif
