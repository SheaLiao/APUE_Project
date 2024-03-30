/*********************************************************************************
 *      Copyright:  (C) 2023 LingYun IoT System Studio.
 *                  All rights reserved.
 *
 *       Filename:  logger.h
 *    Description:  This file is common logger API functions
 *
 *        Version:  1.0.0(11/08/23)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/08/23 16:18:43"
 *
 ********************************************************************************/

#ifndef  _LOGGER_H_
#define  _LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "v0.1"

/* log level */
enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_MAX
};

enum {
    LOG_LOCK_DISABLE, /* disable lock */
    LOG_LOCK_ENABLE,  /* enable lock */
};

#define ROLLBACK_NONE          0

/* description: Initial the logger system
 * arguments  :
 *             $fname: logger file name, NULL/"console"/"stderr" will log to console
 *             $level: logger level above;
 *             $size : logger file max size in KiB
 *             $lock : thread lock enable or not
 * return     : <0: Failed  ==0: Sucessfully
 */
#define THREAD_LOCK_NONE       0
#define THREAD_LOCK_EN         1
int log_open(char *fname, int level, int size, int lock);


/* description: Terminate the logger system */
void log_close(void);


/* description: log message into log file. Don't call this function directly. */
void _log_write(int level, const char *file, int line, const char *fmt, ...);


/* description: dump a buffer in hex to logger file */
void log_dump(int level, const char *prompt, char *buf, size_t len);

/* function: log message into logger file with different log level */
#define log_trace(...) _log_write(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) _log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  _log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  _log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) _log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
