/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  database.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(22/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "22/03/24 17:39:04"
 *                 
 ********************************************************************************/

#ifndef _DATABASE_H_
#define _DATABASE_H_

#define SQL_COMMAND_LEN		256

extern int open_database(const char *db_file);
extern void close_database();
extern int insert_database(void *pack, int size);
extern int query_database();
extern int pop_database(void *pack, int size, int *bytes);
extern int delete_database();


#endif




