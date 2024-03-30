/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(21/03/24)
 *         Author:  Liao Shengli <li@email.com>
 *      ChangeLog:  1, Release initial version on "21/03/24 14:02:13"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>

#include "logger.h"
#include "ds18b20.h"

int get_temperature(float *temp)
{
	int				fd = -1;
	int				found = 0;
	char			buf[128];
	char			*ptr = NULL;
	char			w1_path[64] = "/sys/bus/w1/devices/";
	char			chip[32];
	DIR				*dirp = NULL;
	struct dirent	*direntp = NULL;

	if( NULL == ( dirp = opendir(w1_path) ) )
	{
		log_error("Open dir error: %s\n", strerror(errno));
		return -1;
	}

	while( NULL != ( direntp = readdir(dirp) ) )
	{
		if( strstr(direntp->d_name, "28-") )
		{
			strncpy(chip, direntp->d_name, sizeof(chip));
			found = 1;
		}
	}
	closedir(dirp);

	if( !found )
	{
		log_error("Can't find path\n");
		return -2;
	}

	strncat(w1_path, chip, sizeof(w1_path) - strlen(w1_path));
	strncat(w1_path, "/w1_slave", sizeof(w1_path) - strlen(w1_path));

	fd = open(w1_path, O_RDONLY);
	if( fd < 0 )
	{
		log_error("Open file failure: %s\n", strerror(errno));
		return -3;
	}

	memset(buf, 0, sizeof(buf));

	if( read(fd, buf, sizeof(buf)) < 0 )
	{
		log_error("Read data faliure.\n");
		return -4;
		close(fd);
	}

	ptr = strstr(buf, "t=");
	if( !ptr )
	{
		log_error("Can't find t=\n");
		return -5;
		close(fd);
	}

	ptr += 2;
	*temp = atof(ptr)/1000;

	return 0;
}
