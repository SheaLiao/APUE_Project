/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 13:36:46"
 *                 
 ********************************************************************************/

#ifndef _PACKET_H_
#define _PACKET_H_

#define MAX_ID_LEN 20

typedef struct packet_s
{
	char	devid[64];
	float	temp;
	char	datetime[128];
} packet_t;

/*pack_proc_t function pointer type*/
typedef int (*pack_proc_t)(packet_t *pack, char *pack_buf, int size);

extern int get_devid(char *devid, int len);
extern int get_time(char *datetime);
extern int sample_data(packet_t *pack);
extern int packet_data(packet_t *pack, char *buf, int size);
extern int packet_json(packet_t *pack, char *buf, int size);

#endif
