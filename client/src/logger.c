/*********************************************************************************
 *      Copyright:  (C) 2023 LingYun IoT System Studio.
 *                  All rights reserved.
 *
 *       Filename:  logger.c
 *    Description:  This file is common logger API functions
 *
 *        Version:  1.0.0(11/08/23)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/08/23 16:18:43"
 *
 ********************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>

#include "logger.h"

typedef void (*log_LockFn)(void *udata, int lock);

static struct {
    char        file[32]; /* logger file name */
    FILE       *fp;       /* logger file pointer */
    long        size;     /* logger file max size */
    int         level;    /* logger level */
    log_LockFn  lockfn;   /* lock function */
    void       *udata;    /* lock data */
} L;

static const char *level_names[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG",
    "TRACE"
};

static const char *level_colors[] = {
    "\x1b[31m",
    "\x1b[33m",
    "\x1b[32m",
    "\x1b[36m",
    "\x1b[94m"
};

static inline void time_to_str(char *buf)
{
    struct timeval   tv;
    struct tm       *tm;
    int              len;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    len = sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%06d ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tv.tv_usec);

    buf[len] = '\0';
}

static void mutex_lock(void *udata, int lock)
{
    int              err;
    pthread_mutex_t *l = (pthread_mutex_t *) udata;

    if (lock)
    {
        if ( (err = pthread_mutex_lock(l)) != 0 )
            log_error("Unable to lock log lock: %s", strerror(err));
    }
    else
    {
        if ( (err = pthread_mutex_unlock(l) != 0) )
            log_error("Unable to unlock log lock: %s", strerror(err));
    }
}

int log_open(char *fname, int level, int size, int lock)
{
    FILE            *fp;

    L.level = level;
    L.size = size*1024;

    if( !fname || !strcmp(fname, "console") || !strcmp(fname, "stderr") )
    {
        strcpy(L.file, "console");
        L.fp = stderr;
        L.size = 0; /* console don't need rollback */
    }
    else
    {
        if ( !(fp = fopen(fname, "a+")) )
        {
            fprintf(stderr, "%s() failed: %s\n", __func__, strerror(errno));
            return -2;
        }
        L.fp = fp;
        strncpy(L.file, fname, sizeof(L.file));
    }


    if( lock )
    {
        static pthread_mutex_t     log_lock;

        pthread_mutex_init(&log_lock, NULL);
        L.udata = (void *)&log_lock;
        L.lockfn = mutex_lock;
    }

    fprintf(L.fp, "\n");
    log_info("logger system(%s) start: file:\"%s\", level:%s, maxsize:%luKiB\n\n",
            LOG_VERSION, L.file, level_names[level], size);

    return 0;
}

void log_close(void)
{
    if( L.fp && L.fp!=stderr )
        fclose(L.fp);

    if (L.udata )
        pthread_mutex_destroy( L.udata);
}

static void log_rollback(void)
{
    char       cmd[128]={0};
    long       fsize;

    /* don't need rollback */
    if(L.size <= 0 )
        return ;

    fsize = ftell(L.fp);
    if( fsize < L.size )
        return ;

    /* backup current log file  */
    snprintf(cmd, sizeof(cmd), "cp %s %s.bak", L.file, L.file);
    system(cmd);

    /* rollback file */
    fseek(L.fp, 0, SEEK_SET);
    truncate(L.file, 0);

    fprintf(L.fp, "\n");
    log_info("logger system(%s) rollback: file:\"%s\", level:%s, maxsize:%luKiB\n\n",
            LOG_VERSION, L.file, level_names[L.level], L.size/1024);

    return ;
}

void _log_write(int level, const char *file, int line, const char *fmt, ...)
{
    va_list    args;
    char       time_string[100];

    if ( !L.fp || level>L.level )
        return;

    /* Acquire lock */
    if ( L.lockfn )
        L.lockfn(L.udata, 1);

    log_rollback();

    /* check and rollback file */
    time_to_str(time_string);

    /* Log to stderr */
    if ( L.fp == stderr )
    {
        fprintf(L.fp, "%s %s %-5s\x1b[0m \x1b[90m%s:%03d:\x1b[0m ",
                time_string, level_colors[level], level_names[level], file, line);
    }
    else /* Log to file */
    {
        fprintf(L.fp, "%s %-5s %s:%03d: ", time_string, level_names[level], file, line);
    }

    va_start(args, fmt);
    vfprintf(L.fp, fmt, args);
    va_end(args);

    fflush(L.fp);

    /* Release lock */
    if ( L.lockfn )
        L.lockfn(L.udata, 0);
}

#define LINELEN 81
#define CHARS_PER_LINE 16
static char *print_char =
"                "
"                "
" !\"#$%&'()*+,-./"
"0123456789:;<=>?"
"@ABCDEFGHIJKLMNO"
"PQRSTUVWXYZ[\\]^_"
"`abcdefghijklmno"
"pqrstuvwxyz{|}~ "
"                "
"                "
" ???????????????"
"????????????????"
"????????????????"
"????????????????"
"????????????????"
"????????????????";

void log_dump(int level, const char *prompt, char *buf, size_t len)
{
    int rc;
    int idx;
    char prn[LINELEN];
    char lit[CHARS_PER_LINE + 2];
    char hc[4];
    short line_done = 1;

    if (!L.fp || level>L.level)
        return;

    if( prompt )
        _log_write(level, __FILE__, __LINE__, "%s", prompt);

    rc = len;
    idx = 0;
    lit[CHARS_PER_LINE] = '\0';

    while (rc > 0)
    {
        if (line_done)
            snprintf(prn, LINELEN, "%08X: ", idx);

        do
        {
            unsigned char c = buf[idx];
            snprintf(hc, 4, "%02X ", c);
            strncat(prn, hc, LINELEN);

            lit[idx % CHARS_PER_LINE] = print_char[c];
        }
        while (--rc > 0 && (++idx % CHARS_PER_LINE != 0));

        line_done = (idx % CHARS_PER_LINE) == 0;
        if (line_done)
        {
            if (L.fp)
                fprintf(L.fp, "%s  %s\n", prn, lit);
        }
    }

    if (!line_done)
    {
        int ldx = idx % CHARS_PER_LINE;
        lit[ldx++] = print_char[(int)buf[idx]];
        lit[ldx] = '\0';

        while ((++idx % CHARS_PER_LINE) != 0)
            strncat(prn, "   ", sizeof(prn)-strlen(prn));

        if (L.fp)
            fprintf(L.fp, "%s  %s\n", prn, lit);

    }
}
