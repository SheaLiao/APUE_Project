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

#include <sys/epoll.h>
#include <getopt.h>
#include "debug.h"
#include "sql.h"
#include "socket_server.h"


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
    char                *progname = NULL;
    int                 opt = -1;
	int                 rv = -1;
    int                 i,j;
    int                 found = 0;
    int                 maxfd = 0;
    char                buf[1024];
    float               temperature;
    char                devid[32];
    char                date[64];
    char                time[64];
	char                datetime[128];
    sqlite3             *db;
    int                 epollfd = -1;
    struct epoll_event  event;
    struct epoll_event  event_array[MAX_EVENTS];
    int                 events;
    const char          *optstring = "bp:h";
    struct option		opts[] = {
		{"daemon", no_argument, NULL, 'b'},
        {"prot", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
        };

    progname = basename(argv[0]);

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
                print_usage(progname);
                return EXIT_SUCCESS;
			default:
				break;
        }
    }

    if( !port )
    {
        print_usage(progname);
        return -1;
    }

    set_socket_rlimit();

    listenfd = socket_server_init(NULL, port);
    if( listenfd < 0 )
    {
        dbg_print("ERROR: %s server listen on port %d failure\n", argv[0],port);
        return -2;
    }
    dbg_print("%s server start to listen on port %d\n", argv[0],port);

    if( daemon_run )
    {
        daemon(0, 0);
    }

    epollfd = epoll_create(MAX_EVENTS);
    if( epollfd < 0 )
    {
        dbg_print("epoll_creat() failure:%s\n",strerror(errno));
        return -3;
    }

    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listenfd;

    if( epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0 )
    {
        dbg_print("epoll add listen socket failure:%s\n",strerror(errno));
        return -4;
    }

    db = open_database();

    while(1)
    {
        events = epoll_wait(epollfd, event_array, MAX_EVENTS, -1);
        if( events < 0 )
        {
            dbg_print("epoll failure: %s\n", strerror(errno));
            break;
        }
        else if( events == 0 )
        {
            dbg_print("epoll get timeout\n");
            continue;
        }

        for( i=0; i<events; i++)
        {
            if( (event_array[i].events&EPOLLERR) || (event_array[i].events&EPOLLHUP) )
            {
                dbg_print("epoll_wait get error on fd[%d]: %s\n", event_array[i].data.fd, strerror(errno));
                epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
                close(event_array[i].data.fd);
            }

            if( event_array[i].data.fd == listenfd )
            {
                connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
                if( connfd < 0 )
                {
					dbg_print("Accept new client failure:%s\n",strerror(errno));

                    continue;
                }

                event.data.fd = connfd;
                event.events = EPOLLIN;
                if( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
                {
                    dbg_print("epoll add client socket failure: %s\n", strerror(errno));
                    close(event_array[i].data.fd);
                    continue;
                }
                dbg_print("epoll add new client socket[%d] ok.\n", connfd);
            }
            else
            {
                rv = read(event_array[i].data.fd, buf, sizeof(buf));
                if( rv <= 0 )
                {
                    dbg_print("socket[%d] read failure or get disconncet and will be removed.\n",
                    event_array[i].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
                    close(event_array[i].data.fd);
                    continue;
                }
                else
                {
					dbg_print("socket[%d] read get %d byte data\n", event_array[i].data.fd, rv);

					sscanf(buf,"%s %f %s %s ", devid, &temperature, date, time);//解析数据
					memset(datetime,0,sizeof(datetime));
					snprintf(datetime,sizeof(datetime),"%s %s",date,time);

                    printf("data2:%s %.2f %s\n",devid, temperature, datetime);
                    store_database(db,devid,&temperature,datetime);//存入数据库
                }
            }
        }
    }

    sqlite3_close(db);

CleanUp:
    close(listenfd);
    return 0;

}

