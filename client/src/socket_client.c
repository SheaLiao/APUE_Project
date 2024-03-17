/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:34:59"
 *                 
 ********************************************************************************/

#include "debug.h"
#include "socket_client.h"


//连接服务器
int connect_socket(char *serv_ip, int port)
{
    int                     conn_fd = -1;
    int                     rv = -1;
    struct sockaddr_in		serv_addr;

    conn_fd = socket(AF_INET,SOCK_STREAM,0);
    if( conn_fd < 0 )
    {
		dbg_print("Creat socket failure: %s\n",strerror(errno));
		return -1;
	}
    dbg_print("Soket creat fd[%d] successfully\n",conn_fd);

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_aton(serv_ip,&serv_addr.sin_addr);

    rv = connect( conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) );
    if(rv < 0)
    {
		dbg_print("Connect to server [%s:%d] failure: %s\n",serv_ip,port,strerror(errno));
		return -2;
    }
    dbg_print("Connect to server [%s:%d] successfully!\n",serv_ip,port);

    return conn_fd;
}

