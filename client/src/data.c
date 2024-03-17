/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  data.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:48:00"
 *                 
 ********************************************************************************/

#include "debug.h"
#include "data.h"
#include "ds18b20.h"

//获取序列号
int get_devid(char *id, int len)
{
	int		sn=1;

    snprintf(id, len,"rpi%04d",sn);

    return 0;
}


//获取时间
int get_datetime(char * psDateTime)
{
    time_t      nSeconds;
    struct tm   *pTM;

    time(&nSeconds);
    pTM = gmtime(&nSeconds);

    /* 系统日期和时间,格式: yyyymmddHHMMSS */
    sprintf(psDateTime, "%d-%d-%d %d:%d:%d",
            1900+pTM->tm_year, 1+pTM->tm_mon, pTM->tm_mday,
            8+pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

    return 0;
}

//将数据合为一个字符串
int data_string(char *s_buf)
{
    int     rv = -1;
    float   temperature;
    char    datetime[128];
    char    devid[32];

    rv = get_temperature(&temperature);
    if (rv < 0 )
    {
        dbg_print("get temperature failure\n");
        return -1;
    }

    rv = get_datetime(datetime);
    if( rv != 0 )
    {
        dbg_print("Get time failure\n");
        return -1;
    }

	rv = get_devid(devid,MAX_ID_LEN);
	if( rv != 0 )
	{
		dbg_print("Get devid failure\n");
		return -1;
	}

    memset(s_buf, 0, sizeof(s_buf));
    sprintf(s_buf, "%s %.2f %s",devid, temperature, datetime);

    dbg_print("data:%s\n", s_buf);

    return 0;
}

//发送数据
int send_data(int conn_fd, char *buf)
{
    ssize_t send_size;

    send_size = send(conn_fd, buf,strlen(buf),0);
    if( send_size == -1 )
    {
        dbg_print("Send data failure:%s\n",strerror(errno));
        return -1;
    }
    return 0;
}


