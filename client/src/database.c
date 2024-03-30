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


/* description : judge whether there is data in database
 * input args  : none
 * return value: <0:failure or no data  >0:there is data
 */
int query_database()
{
	char	*errmsg = NULL;
	char	*sql;
	int		rc = 0;
	char	**dbResult;
	int		nRow = 0;
	int		nColumn = 0;
	
	if( !s_db )
	{
		log_error("open database failure\n");
		return -5;
	}

	sql = "SELECT * FROM Temp";
	rc = sqlite3_get_table(s_db, sql, &dbResult, &nRow, &nColumn,&errmsg);
	if( rc != SQLITE_OK )
    {
        log_error("database select error: %s\n", errmsg);
        sqlite3_free(errmsg);
		return -6;
    }
	else
	{
		if( nRow > 0 )
		{
			log_debug("database is not null\n");
			return 1;
		}
		else
		{
			return -7;
		}
	}

}

/* description : pop a row of data
 * input args  : $pack: blob packet data address
 * 				 $size: the lenthof pack
 * return value: <0: failure  0:ok
 */
int pop_database(void *pack, int size, int *bytes)
{
	char			*errmsg = NULL;
	char			*sql;
	int				rc = 0;
	sqlite3_stmt	*stat = NULL;
	const void 		*blob_ptr = NULL;

	if( !s_db )
	{
		log_error("open database failure:%s\n", errmsg);
		rc = -1;
		goto OUT;
	}
	
	sql = "SELECT packet FROM Temp WHERE rowid = (SELECT rowid FROM Temp LIMIT 1)";
	rc = sqlite3_prepare_v2(s_db, sql, -1, &stat, NULL);
	if( rc != SQLITE_OK )
	{
		log_error("pop sqlite3_prepare_v2 failure\n");
		rc = -2;
		goto OUT;
	}	

	sqlite3_step(stat);
	if( rc != SQLITE_OK )
	{
		log_error("pop sqlite3_step failure\n");
		rc = -3;
		goto OUT;
	}

	blob_ptr = sqlite3_column_blob(stat, 0);
	if( !blob_ptr )
	{
		rc = -4;
		goto OUT;
	}

	*bytes = sqlite3_column_bytes(stat, 0);
	if( *bytes > size )
	{
		log_error("pop packet bytes[%d] larger than bufsize[%d]\n", *bytes, size);
		*bytes = 0;
		rc = -5;
	}

	memcpy(pack, blob_ptr, *bytes);
	rc = 0;

OUT:
	sqlite3_finalize(stat);

	if( rc < 0 )
	{
		log_error("pop packet from database failure, rc=%d\n", rc);
	}
	else
	{	
		log_debug("pop packet from database successfully\n");
	}

	return rc;
}

/* description : delete the first row data
 * input args  : $pack: blob packet data address
 * 				 $size: the lenthof pack
 * return value: <0: failure  0:ok
 */
int delete_database()
{
	char	*errmsg = NULL;
	char	*sql;
	int		rc = 0;

	sql = "DELETE FROM Temp WHERE rowid IN (SELECT rowid FROM Temp LIMIT 1) ";
	rc = sqlite3_exec(s_db, sql, 0, 0, &errmsg);
	if( rc != SQLITE_OK )
    {
        log_error("delete database failure: %s\n", errmsg);
        sqlite3_free(errmsg);
		return -8;
    }
	log_debug("delete packet from database successfully\n");

	sqlite3_exec(s_db, "VACUUM;", NULL, 0, NULL);

	return 0;
}

