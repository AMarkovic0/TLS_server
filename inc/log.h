#ifndef _LOGC_H_
#define _LOGC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdarg.h>

// Enum that allows or restricts logging on stdout
typedef enum logging {
	LOG_ON = 1,
	LOG_OFF = 0
}_logs;

extern _logs log;

/*
 * This function will print formatted str in log is ON
 * In:
 *      const char *fmt         -> Formatted string
 *      ...                     -> Format arguments
*/
void dbg(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //_LOGC_H_
