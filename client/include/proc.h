/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  proc.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(01/04/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "01/04/24 14:53:45"
 *                 
 ********************************************************************************/

#ifndef _PROC_H_
#define _PROC_H_

typedef struct proc_sig_s
{
	unsigned		stop;
	int				signal;
} proc_sig_t;

extern proc_sig_t g_signal;

extern void signal_handler(int sig);
extern void install_signal(void);

#endif
