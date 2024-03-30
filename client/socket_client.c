/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(25/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "25/03/24 11:08:10"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include "database.h"
#include "ds18b20.h"
#include "logger.h"
#include "packet.h"
#include "socket.h"

int check_sample_time(int interval, time_t *last_time);

void print_usage(char *progname)
{
    printf("Usage: %s [OPTION]...\n",progname);
    printf("-i(--ipaddr): sepcify server IP address.\n");
    printf("-p(--port): sepcify server port.\n");
    printf("-t(--interval): sepcify sample interval.\n");
    printf("-h(--help): print this help information.\n");

	printf("\nExample: %s -i 192.168.2.40 -p 8888 -t 10\n", progname);
    return ;
}


int main(int argc, char *argv[])
{
	int				rv = -1;
	int				time_flag = 0;

	char			*logfile = "socket_client.log";
	int				loglevel = LOG_LEVEL_INFO;
	int				logsize = 10;

	char			*servip = NULL;
	int				port = 0;
	int				interval = 60;
	time_t			last_time = 0;
	socket_t		sock;
	
	char			pack_buf[1024];
	int				pack_bytes= 0;
	packet_t		pack;
	pack_proc_t		pack_proc = packet_data;

	int				opt = -1;
	const char		*optstring = "i:p:t:h";
	struct option	opts[] = {
		{"ipaddr", required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"interval", required_argument, NULL, 't'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
	{
		switch( opt )
		{
			case 'i':
				servip = optarg;
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 't':
				interval = atoi(optarg);
				break;

			case 'h':
				print_usage(argv[0]);
				return 0;

			default:
				break;
		}
	}

	if( !servip || !port )
	{
		print_usage( argv[0] );
		return 0;
	}
	
	if( log_open(logfile, loglevel, logsize, THREAD_LOCK_NONE) < 0 )
	{
		fprintf(stderr, "initial log system failure\n");
		return -1;
	}

	open_database("client.db");
	socket_init(&sock, servip, port);
	
	while(1)
	{
		time_flag = 0;	
		if( check_sample_time(interval, &last_time) )
		{
			log_info("Start sample\n");
			time_flag = 1;
			
			get_temperature(&pack.temp);
			get_devid(pack.devid, MAX_ID_LEN);
			get_time(pack.datetime);

			//pack_bytes = packet_data(&pack, pack_buf, sizeof(pack_buf));
			pack_bytes = pack_proc(&pack, pack_buf, sizeof(pack_buf));
			printf("pack_bytes:%d, data:%s\n", pack_bytes, pack_buf);
		}
		
		if( sock.fd < 0 )
		{
			log_info("Socket is disconnected, start reconnecting...\n");
			rv = socket_connect(&sock);
		}
	
		(void)judge_socket_state(&sock);
		if( sock.net_state == 0 )
		{
			if( time_flag )
			{
				log_info("Sampling time is up, data is stored in the database\n");
				insert_database(pack_buf, pack_bytes);
			}
			continue;
		}

		/*socket is connected*/
		if( time_flag )
		{
			log_info("Socket send data\n");
			rv = socket_send(&sock, pack_buf, pack_bytes);
			if( rv < 0 )
			{
				log_info("Send data failure, data is stored in the database\n");
				insert_database(pack_buf, pack_bytes);
			}
		}

		/*send data of database*/
		if( query_database() > 0 )
		{
			log_info("Socket send data of database\n");
			rv = pop_database(pack_buf, sizeof(pack_buf), &pack_bytes);
			rv = socket_send(&sock, pack_buf, pack_bytes);
			if( rv < 0 )
			{
				log_error("Socket send data of database failure\n");
				socket_close(&sock);
			}
			else
			{
				log_info("data send successfully, remove the data\n");
				rv = delete_database();
				if( rv < 0 )
				{
					log_error("delete data failure\n");
				}
				log_info("delete data successfully\n");
			}

		}
	}

	socket_close(&sock);
	close_database();
	log_close();
	return 0;
}


int check_sample_time(int interval, time_t *last_time)
{
	int		flag = 0;
	time_t	now;

	time(&now);
	if( (now - *last_time) >= interval )
	{
		flag = 1;
		*last_time = now;
	}
	return flag;
}
