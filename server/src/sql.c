/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sql.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "11/03/24 15:10:14"
 *                 
 ********************************************************************************/

#include "sql.h"

//插入数据
void store_database(sqlite3 *db, char *devid,float *temperature, char *datetime)
{
        char    *sql;
        int     rc;
        char    *zErrMsg = 0;

        rc = sqlite3_open("server.db", &db);
        if( rc )
        {
                fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
        }

        sql = sqlite3_mprintf("INSERT INTO Temperature(devid,temperature, time) VALUES ('%q',%f,'%q')", devid,*temperature, datetime);
        rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
                fprintf(stderr,"SQL insert error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
        }

        sqlite3_close(db);
}

//打开数据库，如果没有则创建
sqlite3 *open_database()
{
        sqlite3         *db;
        char            *zErrMsg = 0;
        int             rc;
        char            *sql;

        rc = sqlite3_open("server.db", &db);
        if( rc )
        {
                fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
                return NULL;
        }

        //创建表
        sql = "CREATE TABLE IF NOT EXISTS Temperature ( id INTEGER PRIMARY KEY AUTOINCREMENT,devid TEXT NOT NULL, temperature REAL NOT NULL, time TEXT NOT NULL );";
        rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
        if( rc != SQLITE_OK )
        {
                fprintf(stderr,"Creat table error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return NULL;
        }

        sqlite3_close(db);
        return db;

}
