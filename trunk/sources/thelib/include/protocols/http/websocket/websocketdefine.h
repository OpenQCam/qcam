#ifdef HAS_PROTOCOL_HTTP
#ifndef _WSDEFINE_H
#define _WSDEFINE_H

//the websocket protocol name depends on av stream name
#include "protocols/avhttpstream/avstreamdefine.h"

#define WEBSOCKET_GUID  "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
//#define WS_STATE_HANDSHAKE     0
//#define WS_STATE_DATATRANSFER  1

//WebSocket outbound connection
#define WEBSOCKETCLIENTKEY "8Jiq1cBLtR0P3pYRyJqXEg=="
//#define WEBSOCKETCLIENTPROTOCOL "PushVideoAlert"
#define WEBSOCKETCLIENTPROTOCOL "testws"

#define TESTWS          "testws"
#define ECHOWS          "echo"
#define TESTSOUNDWS2    "testsound2"
#define ALARMME         "alarmme"
#define SYSTEMINFOWS    "systeminfo"
#define DISPLAY         "display"
#define CLICOMMAND      "clicommand"

//append "_WS" to stream name
#define WSSTR(STREAMNAME,POSFIX)  STREAMNAME#POSFIX

//Audio WebSocket Protocol Name
#define WS_PLAYWAVE_PROTOCOL WSSTR(QIC_STREAM_WAVE,_WS)
#define WS_PLAYMP3_PROTOCOL WSSTR(QIC_STREAM_WAVE,_WS)
#define WS_PLAYG711_PROTOCOL WSSTR(QIC_STREAM_G711,_WS)
#define WS_PLAYAAC_PROTOCOL WSSTR(QIC_STREAM_AAC,_WS)
#define WS_PLAYOPUS_PROTOCOL WSSTR(QIC_STREAM_OPUS,_WS)

//Video WebSocket Protocol Name
#define WS_PLAYAVC_PROTOCOL WSSTR(QIC_STREAM_AVC, _WS)
#define WS_PLAYMJPG_PROTOCOL WSSTR(QIC_STREAM_MJPG, _WS)
#define WS_PLAYMP4FILE_PROTOCOL WSSTR(QIC_STREAM_MP4FILE, _WS)

#define MAP_WSPROT2STM(wsprotocol) (wsprotocol.substr(0, wsprotocol.length()-3))


#endif  /* _WSDEFINE_H */
#endif  /* HAS_PROTOCOL_HTTP */
