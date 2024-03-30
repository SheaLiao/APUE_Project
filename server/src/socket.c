/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "11/03/24 15:16:14"
 *                 
 ********************************************************************************/


#include "logger.h"
#include "socket.h"


int socket_server_init(char *listen_ip, int listen_port)
{
        struct sockaddr_in      servaddr;
        int                     rv = 0;
        int                     on = 1;
        int                     listenfd = -1;

        listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if( listenfd < 0 )
        {
                log_error("Use socket() to create a TCP socket failure: %s\n", strerror(errno));
                return -1;
        }

        setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    	memset(&servaddr,0,sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(listen_port);

        if( !listen_ip )
        {
                servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
                if( inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0 )
                {
                        log_error("inet_pton() set listen IP address failure.\n");
                        rv = -2;
                        goto CleanUp;
                }
        }

        if( bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
        {
                log_error("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
                rv = -3;
                goto CleanUp;
        }

        if(listen(listenfd, 1024) < 0)
        {
                log_error("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
                rv = -4;
                goto CleanUp;
        }

CleanUp:
        if( rv < 0 )
        {
                close(listenfd);
        }
        else
        {
                rv = listenfd;
        }

        return rv;
}

void set_socket_rlimit(void)
{
        struct rlimit limit = {0};

        getrlimit(RLIMIT_NOFILE, &limit);
        limit.rlim_cur = limit.rlim_max;
        setrlimit(RLIMIT_NOFILE, &limit);

        log_info("Set socket open fd max count to %ld\n",limit.rlim_max);
}

