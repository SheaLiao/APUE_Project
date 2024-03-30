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
	close(sock->fd);
	sock->fd = -1;
	return 0;
}


/* description : resolve domain name
 * input args  : $sock : socket context pointer
 * return value: <0: failure  0:ok
 */
void hostname_to_ip(char *hostname)
{
	struct hostent		*host;
	char				**ptr = NULL;
	
	host = gethostbyname(hostname);
	if( !host )
	{
		log_error("gethostbyname failure:%s\n",strerror(errno));
		return;
	}

	switch( host->h_addrtype )
	{
		case AF_INET:
		case AF_INET6:
			ptr = host->h_addr_list;
			for( ; *ptr != NULL; ptr++ )
			{
				inet_ntop(host->h_addrtype, *ptr, hostname, sizeof(hostname));
			}
			break;
		default:
			log_error("unknown address type\n");
			break;
	}
	log_debug("get ip [%s] successfully\n", hostname);
	return ;
}

/* description : judge ip or hostname
 * input args  : $servip: connect server servip for client mode
 * return value: <0: hostname  >0: ip
 */
int judge_ip_hostname(char *servip)
{
	unsigned long	add;
	
	add = inet_addr(servip);
	if( add == INADDR_NONE )
	{
		log_debug("It is a hostname\n");
		return -3;
	}
	else
	{
		log_debug("It is an ip\n");
		return 1;
	}
}


/* description : socket connect to server
 * input args  : $sock : socket context pointer
 * return value: <0: failure  0:ok
 */
int socket_connect(socket_t *sock)
{
	int						rv = -1;
	int						fd = 0;
	struct sockaddr_in		serv_addr;

	//socket_close(sock);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if( fd < 0 )
	{
		log_error("Creat socket failure:%s\n", strerror(errno));
		close(fd);
		return -4;
	}
	log_info("Socket create fd[%d] successfully\n", fd);
	
	rv = judge_ip_hostname(sock->servip);
	if( rv < 0 )
	{
		hostname_to_ip(sock->servip);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(sock->port);
	inet_aton(sock->servip, &serv_addr.sin_addr);

	rv = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if( rv < 0 )
	{
		log_error("Connect to server [%s:%d] failure:%s\n", sock->servip, sock->port, strerror(errno));
		close(fd);
		return -5;
	}
	else
	{
		sock->fd = fd;
		log_debug("Connect to server [%s:%d] successfully\n", sock->servip, sock->port);
	}
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
