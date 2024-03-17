/********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  debug.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(11/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "11/03/24 15:26:34"
 *                 
 ********************************************************************************/

#ifndef	_DEBUG_H_
#define	_DEBUG_H_

#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
#define dbg_print(format, args...) printf(format,##args)
#else
#define dbg_print(format, args...) do{} while(0)
#endif


#endif
