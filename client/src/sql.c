/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  sql.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 14:18:57"
 *                 
 ********************************************************************************/

#include "sql.h"
#include "data.h"

data_t  data_arr[MAX_ROW];
int     data_count = 0;


//数据库的回调函数
static int callback(void *data, int argc, char **argv, char **azColName)
{
   int	i;

   /*将数据库中的数据暂存到结构体中*/
   data_arr[data_count].temperature = atof(argv[0] ? argv[0] : "0.00");
   snprintf(data_arr[data_count].datetime,sizeof(data_arr[data_count].datetime), "%s", argv[1] ? argv[1] : "NULL");

   data_count++;//data_arr的个数，++用于定位到下一个元素 
   return 0;
}


//打开数据库，如果没有则创建
sqlite3 *open_database()
{
    sqlite3		*db;
    char        *zErrMsg = 0;
    int         rc;
    char        *sql;

    rc = sqlite3_open("client.db", &db);
    if( rc )
    {
        fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
        return NULL;
    }

    //创建表
    sql = "CREATE TABLE IF NOT EXISTS Temperature ( id INTEGER PRIMARY KEY AUTOINCREMENT, temperature REAL NOT NULL, time TEXT NOT NULL );";
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


//插入数据
void store_database(sqlite3 *db, float *temperature, char *datetime)
{
    char    *sql;
    int     rc;
    char    *zErrMsg = 0;

    rc = sqlite3_open("client.db", &db);
    if( rc )
    {
        fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
    }

    sql = sqlite3_mprintf("INSERT INTO Temperature(temperature, time) VALUES (%f,'%q')", *temperature, datetime);
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr,"SQL insert error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);
}


void select_database(sqlite3 *db, int conn_fd)
{
    char    *sql;
    int     rc;
    char    *zErrMsg = 0;
    char    id[32];
    int     i = 0;
    char    s_buf[128];

    rc = sqlite3_open("client.db", &db);
    if( rc )
    {
        fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
    }

    sql = "SELECT temperature,time FROM Temperature";
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr,"SQL select error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    get_devid(id, MAX_ID_LEN);
    for( i=0;i<data_count;i++)
    {
		memset(s_buf,0,sizeof(s_buf));
        snprintf(s_buf, sizeof(s_buf), "%s %.2f %s", id, data_arr[i].temperature, data_arr[i].datetime);
        dbg_print("data2:%s\n", s_buf);

        send_data(conn_fd,s_buf);
		sleep(1);//防止数据发送太快，被打包成一个数据报文
    }

	delete_database(db);
    sqlite3_close(db);
}


//删除数据库中的数据
void delete_database(sqlite3 *db)
{
    char    *zErrMsg = 0;
    int     rc;
    char    *sql;

    rc = sqlite3_open("client.db", &db);
    if( rc )
    {
        fprintf(stderr,"Can't open database:%s\n", sqlite3_errmsg(db));
    }

    sql = "DELETE FROM Temperature";
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr,"SQL delete error:%s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}



