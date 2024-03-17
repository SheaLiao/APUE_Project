/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  data.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(13/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:44:33"
 *                 
 ********************************************************************************/

#ifndef _DATA_H_
#define _DATA_H_

#define MAX_ID_LEN	20

extern int get_devid(char *id, int len);
extern int get_datetime(char * psDateTime);
extern int data_string(char *s_buf);
extern int send_data(int conn_fd, char *buf);

#endif
