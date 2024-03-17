/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:38:28"
 *                 
 ********************************************************************************/

#include "ds18b20.h"
#include "debug.h"


//获取温度和设备号
int get_temperature(float *temperature)
{
    int     fd = -1;
    int     found = 0;
    char    buf[128];
    char    *ptr = NULL;
    char    w1_path[64] = "/sys/bus/w1/devices/";
    char    chip[32];
    DIR     *dirp = NULL;
    struct  dirent *direntp = NULL;

    if( NULL ==(dirp = opendir(w1_path)))
    {
        dbg_print("open %s failure: %s\n",w1_path,strerror(errno));
        return -1;
    }

    while( NULL != (direntp = readdir(dirp)) )
    {
        if( strstr(direntp->d_name,"28-") )
        {
            strncpy(chip,direntp->d_name,sizeof(chip));
            found = 1;
        }
    }
    closedir(dirp);

    if( !found )
    {
        dbg_print("cannot find ds18b20 path\n");
        return -2;
    }

    strncat(w1_path,chip,sizeof(w1_path)-strlen(w1_path));
    strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));


    fd = open(w1_path,O_RDONLY);
    if( fd < 0 )
    {
        dbg_print("open file failure:%s\n",strerror(errno));
        return -3;
    }


    memset(buf,0,sizeof(buf));

    if( read(fd,buf,sizeof(buf)) < 0 )
    {
        dbg_print("read data failure\n");
        return -4;
        close(fd);
    }


    ptr = strstr(buf,"t=");
    if( !ptr )
    {
        dbg_print("cannot find t=\n");
        return -5;
        close(fd);
    }

    ptr += 2;
    *temperature = atof(ptr)/1000;

    sleep(5);

    return 0;
}

