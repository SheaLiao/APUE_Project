/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "11/03/24 15:41:42"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>

#include "logger.h"
#include "database.h"
#include "socket.h"


#define MAX_EVENTS 512
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


static inline void print_usage(char *progname)
{
    printf("Usage: %s [OPTION]...\n", progname);

    printf(" %s is a socket server program, which used to verify client and echo back string from it\n",progname);
    printf("\nMandatory arguments to long options are mandatory for short options too:\n");

    printf(" -b[daemon ] set program running on background\n");
    printf(" -p[port ] Socket server port address\n");
    printf(" -h[help ] Display this help information\n");

    printf("\nExample: %s -b -p 8900\n", progname);
    return ;
}


int main(int argc, char *argv[])
{
    int                 listenfd = -1;
    int                 connfd = -1;
    int                 port = 0;
    int                 daemon_run = 0;
	int                 rv = -1;
    int                 i,j;
    int                 found = 0;
    int                 maxfd = 0;
    char                buf[1024];

	char				*logfile = "socket_server.log";
	int					loglevel = LOG_LEVEL_INFO;
	int					logsize = 10;

    int                 epollfd = -1;
    struct epoll_event  event;
    struct epoll_event  event_array[MAX_EVENTS];
    int                 events;

    const char          *optstring = "bp:h";
    int                 opt = -1;
    struct option		opts[] = {
		{"daemon", no_argument, NULL, 'b'},
        {"prot", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };


    while ( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
    {
        switch (opt)
        {
            case 'b':
                daemon_run = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
			default:
				break;
        }
    }

    if( !port )
    {
        print_usage(argv[0]);
        return -1;
    }

	if( log_open(logfile, loglevel, logsize, THREAD_LOCK_NONE ) < 0 )
	{
		fprintf(stderr, "initial log system failure\n");
	}

    set_socket_rlimit();

    listenfd = socket_server_init(NULL, port);
    if( listenfd < 0 )
    {
        log_error("%s server listen on port %d failure\n", argv[0],port);
        return -2;
    }
    log_info("%s server start to listen on port %d\n", argv[0],port);

    if( daemon_run )
    {
        daemon(0, 0);
    }

    epollfd = epoll_create(MAX_EVENTS);
    if( epollfd < 0 )
    {
        log_error("epoll_creat() failure:%s\n",strerror(errno));
        return -3;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listenfd;

    if( epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0 )
    {
        log_error("epoll add listen socket failure:%s\n",strerror(errno));
        return -4;
    }

	open_database("server.db");

    while(1)
    {
        events = epoll_wait(epollfd, event_array, MAX_EVENTS, -1);
        if( events < 0 )
        {
            log_error("epoll failure: %s\n", strerror(errno));
            break;
        }
        else if( events == 0 )
        {
            log_debug("epoll get timeout\n");
            continue;
        }

        for( i=0; i<events; i++)
        {
            if( (event_array[i].events&EPOLLERR) || (event_array[i].events&EPOLLHUP) )
            {
                log_error("epoll_wait get error on fd[%d]: %s\n", event_array[i].data.fd, strerror(errno));
                epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
                close(event_array[i].data.fd);
            }

            if( event_array[i].data.fd == listenfd )
            {
                connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
                if( connfd < 0 )
                {
					log_error("Accept new client failure:%s\n",strerror(errno));

                    continue;
                }

                event.data.fd = connfd;
                event.events = EPOLLIN;
                if( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
                {
                    log_error("epoll add client socket failure: %s\n", strerror(errno));
                    close(event_array[i].data.fd);
                    continue;
                }
                log_info("epoll add new client socket[%d] ok.\n", connfd);
            }
            else
            {
                rv = read(event_array[i].data.fd, buf, sizeof(buf));
                if( rv <= 0 )
                {
                    log_error("socket[%d] read failure or get disconncet and will be removed.\n",
                    event_array[i].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
                    close(event_array[i].data.fd);
                    continue;
                }
                else
                {
					log_info("socket[%d] read get %d byte data\n", event_array[i].data.fd, rv);

                    printf("%s\n", buf);
                    insert_database(buf, sizeof(buf));//存入数据库
                }
            }
        }
    }

    close_database();

CleanUp:
    close(listenfd);
    return 0;

}

