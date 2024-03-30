/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  database.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 13:15:24"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

#include "database.h"
#include "logger.h"

#define TABLE_NAME "packTable"

static sqlite3 *s_db = NULL;


/* description : open database and create table if not exist
 * input args  : $db_file: 数据库名
 * return value: <0: failure  0;ok
 */
int open_database(const char *db_file)
{
    char        *errmsg = NULL;
    int         rc;
    char        *sql;

    rc = sqlite3_open(db_file, &s_db);
    if( rc )
    {
        log_error("Can't open database:%s\n", sqlite3_errmsg(s_db));
        return -1;
    }

    //创建表
	sqlite3_exec(s_db, "pragma synchronous = OFF;", 0, 0, 0); //不与当前磁盘同步
	sqlite3_exec(s_db, "pragma auto_vacuum = 1;", 0, 0, 0);
    sql = "CREATE TABLE IF NOT EXISTS Temp ( packet blob );";
    rc = sqlite3_exec(s_db, sql, 0, 0, &errmsg);
    if( rc != SQLITE_OK )
    {
    	log_error("Creat table error:%s\n", errmsg);
        sqlite3_free(errmsg);
		sqlite3_close(s_db);
		unlink(db_file);
        return -2;
    }

    return 0;
}


/* description : close database
 * input args  : none 
 */
void close_database(void)
{
	log_warn("close database\n");
	sqlite3_close(s_db);
	return ;
}


/* description : store data to database
 * input args  : $pack: blob packet data address
 * 				 $size: the lenthof pack
 * return value: <0: failure  0:ok
 */
int insert_database(void *pack, int size)
{
	char			*errmsg = NULL;
	int				rc = 0;
	char			*sql;
	sqlite3_stmt	*stat = NULL;

	if( !s_db )
	{
		log_error("open database failure:%s\n", errmsg);
		rc = -1;
		goto OUT;
	}

	sql = sqlite3_mprintf("INSERT INTO Temp (packet) VALUES (?)");
	rc = sqlite3_prepare_v2(s_db, sql, -1, &stat, NULL);
	if( rc != SQLITE_OK )
	{
		log_error("insert sqlite3_prepare_v2 failure\n");
		rc = -2;
		goto OUT;
	}

	sqlite3_bind_blob(stat, 1, pack, size, NULL);
	if( rc != SQLITE_OK )
	{
		log_error("insert sqlite3_bind_blob failure\n");
		rc = -3;
		goto OUT;
	}

	sqlite3_step(stat);
	if( rc != SQLITE_OK )
	{
		log_error("insert sqlite3_step failure\n");
		rc = -4;
		goto OUT;
	}

OUT:
	sqlite3_finalize(stat);

	if( rc < 0 )
	{
		log_error("add packet into database failure, rc=%d\n", rc);
	}
	else
	{	
		log_debug("add packet into database successfully\n");
	}

	return rc;
}


