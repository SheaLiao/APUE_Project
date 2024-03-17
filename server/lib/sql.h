/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sql.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(11/03/24)
 *         Author:  Liao Shengli<linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "11/03/24 15:12:59"
 *                 
 ********************************************************************************/

#ifndef _SQL_H_
#define _SQL_H_

#include <sqlite3.h>
#include <stdio.h>

extern sqlite3 *open_database();
extern void store_database(sqlite3 *db, char *devid,float *temperature, char *datetime);

#endif
