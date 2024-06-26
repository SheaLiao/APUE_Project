/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 17:44:15"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "logger.h"
#include "socket.h"


/* description : initial socket context
 * input args  : $sock	  : socket context pointer
 * 				 $hostname: connect server hostname for client mode
 * 				 $port	  : connect server port for client mode 
 * return value: <0: failure  0:ok
 */
int socket_init(socket_t *sock, char *hostname, int port)
{
	if( !sock || port <= 0 )
	{
		return -1;
	}

	memset(sock, 0, sizeof(sock));
	sock->fd = -1;
	strncpy(sock->servip, hostname, 64);
	sock->port = port;
	sock->net_state = 0;
	return 0;
}


/* description : close socket
 * input args  : $sock : socket context pointer
 * return value: 0: ok 
 */
int socket_close(socket_t *sock)
{
	if( !sock )
	{
		return -1;

	}
	close(sock->fd);
	sock->fd = -1;
	return 0;
}


/*  description: socket connect to server in block mode
 *   input args:
 *               $sock:  socket context pointer
 * return value: <0: failure   0:ok
 */
int socket_connect(socket_t *sock)
{
    int                 rv = 0;
    int                 sockfd = 0;
    char                service[20];
    struct addrinfo     hints, *rp;
    struct addrinfo    *res = NULL;
    struct in_addr      inaddr;
    struct sockaddr_in  addr;
    int                 len = sizeof(addr);

    if( !sock )
        return -1;

    socket_close(sock);

    /*+--------------------------------------------------+
     *| use getaddrinfo() to do domain name translation  |
     *+--------------------------------------------------+*/

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Only support IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP; /* TCP protocol */

    /* If $host is a valid IP address, then don't use name resolution */
    if( inet_aton(sock->servip, &inaddr) )
    {
        log_info("%s is a valid IP address, don't use domain name resolution.\n", sock->servip);
        hints.ai_flags |= AI_NUMERICHOST;
    }

    /* Obtain address(es) matching host/port */
    snprintf(service, sizeof(service), "%d", sock->port);
    if( (rv=getaddrinfo(sock->servip, service, &hints, &res)) )
    {
        log_error("getaddrinfo() parser [%s:%s] failed: %s\n", sock->servip, service, gai_strerror(rv));
        return -3;
    }
    
	/* getaddrinfo() returns a list of address structures. Try each
       address until we successfully connect or bind */
    for (rp=res; rp!=NULL; rp=rp->ai_next)
    {
#if 1 
        char                  ipaddr[INET_ADDRSTRLEN];
        struct sockaddr_in   *sp = (struct sockaddr_in *) rp->ai_addr;

        /* print domain name translation result */
        memset( ipaddr, 0, sizeof(ipaddr) );
        if( inet_ntop(AF_INET, &sp->sin_addr, ipaddr, sizeof(ipaddr)) )
        {
            log_info("domain name resolution [%s->%s]\n", sock->servip, ipaddr);
        }
#endif

        /*  Create the socket */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if( sockfd < 0)
        {
            log_error("socket() create failed: %s\n", strerror(errno));
            rv = -3;
            continue;
        }

        /* connect to server */
        rv = connect(sockfd, rp->ai_addr, len);
        if( 0 == rv )
        {
            sock->fd = sockfd;
            log_info("Connect to server[%s:%d] on fd[%d] successfully!\n", sock->servip, sock->port, sockfd);
            break;
        }
        else
        {
            /* socket connect get error, try another IP address */
            close(sockfd);
            continue;
        }
    }

    freeaddrinfo(res);
    return rv;
}


/* description : judge socket state 
 * input args  : $sock: socket context pointer
 * return value: <0:disconnection  0:ok
 */
int judge_socket_state(socket_t *sock)
{
	int             	sock_opt;
	struct tcp_info 	optval;
	int             	opt_len = sizeof(optval);

	sock_opt = getsockopt(sock->fd, IPPROTO_TCP, TCP_INFO, &optval, (socklen_t *)&opt_len);
	if( optval.tcpi_state != TCP_ESTABLISHED )
	{
		log_debug("Server disconnect\n");
		socket_close(sock);
		sock->net_state = 0;
		return -6;
	}
	sock->net_state = 1;

	return 0;
}

int socket_send(socket_t *sock, char *data, int bytes)
{
	int		rv = -1;
	int		i = 0;
	int		left_bytes = bytes;

	if( !sock || !data || bytes <= 0 )
	{
		return -7;
	}

	while( left_bytes > 0 )
	{
		rv = write(sock->fd, &data[i], left_bytes);
		if( rv < 0 )
		{
			log_info("socket[%d] write failure: %s, close socket now\n", sock->fd,strerror(errno));
			socket_close(sock);
			return -8;
		}
		else if( rv == left_bytes )
		{
			log_info("socket send %d bytes data over\n", bytes);
			return 0;
		}
		else
		{
			i += rv;
			left_bytes -= rv;
			continue;
		}
	}
	return 0;
}
