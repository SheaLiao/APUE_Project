/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sql.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(13/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:14:16"
 *                 
 ********************************************************************************/

#ifndef _SQL_H_
#define _SQL_H_

#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"

#define MAX_ROW         1000

typedef struct data
{
        float           temperature;
        char    datetime[32];
} data_t;


static int callback(void *data, int argc, char **argv, char **azColName);
extern sqlite3 *open_database();
extern void store_database(sqlite3 *db, float *temperature, char *datetime);
extern void select_database(sqlite3 *db, int conn_fd);
extern void delete_database(sqlite3 *db);

#endif
