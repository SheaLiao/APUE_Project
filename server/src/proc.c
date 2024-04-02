/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  proc.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(01/04/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "01/04/24 14:21:06"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "logger.h"
#include "proc.h"

proc_sig_t g_signal;

void signal_handler(int sig)
{
	switch(sig)
	{
		case SIGINT:
			log_warn("SIGINT - stop\n");
			g_signal.stop = 1;
			break;

		case SIGPIPE:
			log_warn("SIGPIPE - stop\n");
			g_signal.stop = 1;
			break;

		case SIGSEGV:
			log_warn("SIGSEGV - stop\n");
			g_signal.stop = 1;
			break;

		case SIGTERM:
			log_warn("SIGTERM - stop\n");
			g_signal.stop = 1;
			break;
	}
}

void install_signal(void)
{
	struct sigaction sigact;

	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	sigaction(SIGINT, &sigact, 0);
	sigaction(SIGPIPE, &sigact, 0);
	sigaction(SIGSEGV, &sigact, 0);
	sigaction(SIGTERM, &sigact, 0);

}

