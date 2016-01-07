#ifndef __QMP4LOG_H__
#define __QMP4LOG_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

void qmp4_log_msg(int level, const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
;

#define qmp4_log_err    WARN
#define qmp4_log_warn   WARN
#define qmp4_log_info   INFO
//#define qmp4_log_debug  DEBUG
#define qmp4_log_debug(...) 

#if 0
#define qmp4_log_err(...) qmp4_log_msg(0, ##__VA_ARGS__)
#define qmp4_log_warn(...) qmp4_log_msg(1, ##__VA_ARGS__)
#define qmp4_log_info(...) qmp4_log_msg(2, ##__VA_ARGS__)
//#define qmp4_log_debug(...) qmp4_log_msg(3, ##__VA_ARGS__)
#define qmp4_log_debug(...) 
#endif
    
#ifdef __cplusplus
}
#endif

#endif /* __QMP4LOG_H__ */

