/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _STREAMSTYPES_H
#define _STREAMSTYPES_H



//Note- Use 'V' or 'A' character to judge video or audio stream
#define ST_NEUTRAL_RTMP         MAKE_TAG2('N','R')
#define ST_IN                   MAKE_TAG1('I')
#define ST_IN_CAM               MAKE_TAG2('I', 'C')
#define ST_IN_CAM_RAW           MAKE_TAG4('I', 'C', 'V', 'R')
#define ST_IN_CAM_MJPG          MAKE_TAG4('I', 'C', 'V', 'J')
#define ST_IN_CAM_AVC           MAKE_TAG4('I', 'C', 'V', 'A')
#define ST_IN_CAM_MOTION        MAKE_TAG4('I', 'C', 'V', 'M')
#define ST_IN_CAM_RELAY_AVC     MAKE_TAG5('I', 'C', 'V', 'R', 'A')
#define ST_IN_CAM_VP8           MAKE_TAG4('I', 'C', 'V', '8')
#define ST_IN_AUDIO             MAKE_TAG3('I', 'C', 'A')
#define ST_IN_AUDIO_G711        MAKE_TAG4('I', 'C', 'A', 'G')
#define ST_IN_AUDIO_PCM         MAKE_TAG4('I', 'C', 'A', 'P')
#define ST_IN_AUDIO_OPUS        MAKE_TAG4('I', 'C', 'A', 'O')
#define ST_IN_AUDIO_AAC         MAKE_TAG4('I', 'C', 'A', 'A')
#define ST_IN_AUDIO_AMR         MAKE_TAG4('I', 'C', 'A', 'M')
#define ST_IN_AUDIO_MP3         MAKE_TAG4('I', 'C', 'A', '3')
#define ST_IN_AV_MUX            MAKE_TAG5('I', 'C', 'A', 'V', 'M')
#define ST_IN_AV_RELAYMUX       MAKE_TAG6('I', 'C', 'A', 'V', 'R','M')

#define ST_OUT_AUDIO_PCM    MAKE_TAG4('O', 'C', 'A', 'P')

#define ST_IN_NET           MAKE_TAG2('I','N')
#define ST_IN_NET_RTMP      MAKE_TAG3('I','N','R')
#define ST_IN_NET_LIVEFLV   MAKE_TAG6('I','N','L','F','L','V')
#define ST_IN_NET_TS      MAKE_TAG4('I','N','T','S')
#define ST_IN_NET_RTP     MAKE_TAG3('I','N','P')
#define ST_IN_NET_RAW     MAKE_TAG3('I','N','W')
#define ST_IN_NET_AAC     MAKE_TAG3('I','N','A')
#define ST_IN_NET_MP3     MAKE_TAG3('I','N','M')
#define ST_IN_NET_EXT     MAKE_TAG5('I','N','E','X','T')
#define ST_IN_NET_PASSTHROUGH MAKE_TAG3('I','N','S')
#define ST_IN_FILE        MAKE_TAG2('I','F')
#define ST_IN_FILE_RTMP     MAKE_TAG3('I','F','R')
#define ST_IN_FILE_MP4        MAKE_TAG5('I','F','M','P','4')
#define ST_IN_FILE_PCM        MAKE_TAG5('I','F','P','C','M')
#define ST_OUT          MAKE_TAG1('O')
#define ST_OUT_NET        MAKE_TAG2('O','N')
#define ST_OUT_NET_RTMP     MAKE_TAG3('O','N','R')
#define ST_OUT_NET_RTMP_4_TS  MAKE_TAG6('O','N','R','4','T','S')
#define ST_OUT_NET_RTMP_4_RTMP  MAKE_TAG5('O','N','R','4','R')
#define ST_OUT_NET_RTP      MAKE_TAG3('O','N','P')
#define ST_OUT_NET_RAW      MAKE_TAG3('O','N','W')
#define ST_OUT_NET_AV     MAKE_TAG4('O','N','A','V')
#define ST_OUT_NET_TS     MAKE_TAG4('O','N','T','S')
#define ST_OUT_NET_PASSTHROUGH  MAKE_TAG3('O','N','S')
#define ST_OUT_NET_MJPGHTTP MAKE_TAG4('O','N','J','H')

#define ST_OUT_FILE           MAKE_TAG2('O','F')
#define ST_OUT_FILE_MP4       MAKE_TAG5('O','F','M','P','4')
#define ST_OUT_FILE_TS        MAKE_TAG4('O','F','T','S') 
#define ST_OUT_FILE_RTMP      MAKE_TAG3('O','F','R')
#define ST_OUT_FILE_RTMP_FLV  MAKE_TAG6('O','F','R','F','L','V')
#define ST_OUT_FILE_HLS       MAKE_TAG5('O','F','H','L','S')
#define ST_OUT_FILE_HDS       MAKE_TAG5('O','F','H','D','S')
#define ST_OUT_FILE_JPG       MAKE_TAG5('O','F','J','P','G')
#define ST_OUT_JPG            MAKE_TAG4('O','J','P','G')

//stream types for http protocol
#define ST_OUT_HTTP_SNAPSHOT  MAKE_TAG6('O','H','S','N','A','P')
#define ST_OUT_HTTP_MJPG  MAKE_TAG6('O','H','M','J','P','G')
#define ST_OUT_HTTP_MP4   MAKE_TAG5('O','H','M','P','4')

//streamp types for record
#define ST_OUT_RECORD_MP4   MAKE_TAG5('O','R','M','P','4')
#define ST_OUT_RECORD_MJPG  MAKE_TAG5('O','R','J','P','G')
#define ST_OUT_RECORD_AVC   MAKE_TAG5('O','R','A','V','C')

//streamp types for record
#define ST_OUT_IVA_JPEG      MAKE_TAG5('O','I','J','P','G')



//stream types in websocket protocol
#define ST_OUT_WSNET_MJPG       MAKE_TAG6('O','N','W','S','V','M')
#define ST_OUT_WSNET_AVC        MAKE_TAG6('O','N','W','S','V','A')
#define ST_OUT_WSNET_AUDIO      MAKE_TAG5('O','N','W','S','A')
#define ST_OUT_WSNET_MSG        MAKE_TAG5('O','N','W','S','M')
#define ST_IN_WSNET_AUDIO       MAKE_TAG5('I','N','W','S','A')
#define ST_IN_WSNET_FILE        MAKE_TAG5('I','N','W','S','F')
//system broadcast message
#define ST_IN_MSG_SYSTEM        MAKE_TAG5('I','M','S','Y','S')


#endif  /* _STREAMSTYPES_H */
