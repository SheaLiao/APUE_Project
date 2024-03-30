/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 21:01:39"
 *                 
 ********************************************************************************/

#ifndef _SOCKET_H_
#define _SOCKET_H_

typedef struct socket_s
{
	int		fd;
	char	servip[64];
	int		port;
	int		net_state;
} socket_t;

extern int socket_init(socket_t *sock, char *hostname, int port);
extern int socket_close(socket_t *sock);
extern void hostname_to_ip(char *hostname);
extern int judge_ip_hostname(char *servip);
extern int socket_connect(socket_t *sock);
extern int judge_socket_state(socket_t *sock);
extern int socket_send(socket_t *sock, char *data, int bytes);


#endif
