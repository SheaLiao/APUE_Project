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
#include <cJSON.h>

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

/*字符串形式*/
int packet_data(packet_t *pack, char *pack_buf, int size)
{

	memset(pack_buf, 0, size);
	snprintf(pack_buf, size, "deviceID:%s | temperature:%.2f | time:%s", pack->devid, pack->temp, pack->datetime);
	return strlen(pack_buf);
}


/*JSON格式*/
int packet_json(packet_t *pack, char *pack_buf, int size)
{
	cJSON *json = cJSON_CreateObject();

	cJSON_AddStringToObject(json, "deviceID", pack->devid);
	cJSON_AddNumberToObject(json, "temperature", pack->temp);
	cJSON_AddStringToObject(json, "time", pack->datetime);
	snprintf(pack_buf, size, cJSON_Print(json));
	
	cJSON_Delete(json);

	return strlen(pack_buf);
}
