/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 13:26:23"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "packet.h"
#include "ds18b20.h"
#include "logger.h"

int get_devid(char *devid, int len)
{
	int		sn = 1;
	
	memset(devid, 0, len);
	snprintf(devid, len, "rpi%04d", sn);
	return 0;
}

int get_time(char *datetime)
{
	time_t		times;
	struct tm	*ptm;
	
	time(&times);
	ptm = gmtime(&times);
	memset(datetime, 0, sizeof(datetime));
	snprintf(datetime, 64, "%d-%d-%d %d:%d:%d",
            1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday,
            8+ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	return 0;
}

int sample_data(packet_t *pack)
{
	if( !pack )
	{
		return -1;
	}

	memset(pack, 0, sizeof(*pack));
	get_temperature(&pack->temp);
	get_devid(pack->devid, MAX_ID_LEN);
	get_time(pack->datetime);

	return 0;
}

int packet_data(packet_t *pack, char *pack_buf, int size)
{
	//sample_data(pack);

	memset(pack_buf, 0, size);
	snprintf(pack_buf, size, "deviceID:%s | temperature:%.2f | time:%s", pack->devid, pack->temp, pack->datetime);
	return strlen(pack_buf);
}
