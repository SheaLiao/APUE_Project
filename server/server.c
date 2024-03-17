/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/03/24)
 *         Author:  liaoshengli <2928382441li@gmail.com>
 *
 *      ChangeLog:  1, Release initial version on "04/03/24 15:55:51"
 *                 
 ********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sqlite3.h>


#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
#define dbg_print(format, args...) printf(format,##args)
#else
#define dbg_print(format, args...) do{} while(0)
#endif


#define MAX_EVENTS 512
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static inline void print_usage(char *progname);
int socket_server_init(char *listen_ip, int listen_port);
void set_socket_rlimit(void);
sqlite3 *open_database();
void store_database(sqlite3 *db, char *devid,float *temperature, char *datetime);


int main(int argc, char *argv[])
{
	int					listenfd = -1;
	int					connfd = -1;
	int					port = 0;
	int					daemon_run = 0;
	char				*progname = NULL;
	int					opt = -1;
	int					rv = -1;
	int					i,j;
	int					found = 0;
	int					maxfd = 0;
	char				buf[1024];
	float				temperature;
	char				devid[32];
	char				date[64];
	char				time[64];
	char				datetime[128];
	sqlite3				*db;
	int					epollfd = -1;
    struct epoll_event	event;
	struct epoll_event	event_array[MAX_EVENTS];
	int					events;
	const char     		*optstring = "bp:h";
    struct option   	opts[] = {
		{"daemon", no_argument, NULL, 'b'},
        {"prot", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
        };

    progname = basename(argv[0]);

	while ( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
    {
        switch (opt)
        {
            case 'b':
                daemon_run = 1;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'h':
                print_usage(progname);
                return EXIT_SUCCESS;
			default:
				break;
        }
    }

    if( !port )
    {
        print_usage(progname);
        return -1;
    }

	set_socket_rlimit();

	listenfd = socket_server_init(NULL, port);
	if( listenfd < 0 )
	{
		dbg_print("ERROR: %s server listen on port %d failure\n", argv[0],port);
 		return -2;
	}
	dbg_print("%s server start to listen on port %d\n", argv[0],port);

	if( daemon_run )
	{
		daemon(0, 0);
	}

	epollfd = epoll_create(MAX_EVENTS);//创建epoll实例
	if( epollfd < 0 )
	{
		dbg_print("epoll_creat() failure:%s\n",strerror(errno));
		return -3;
	}

	event.events = EPOLLIN | EPOLLET;
	event.data.fd = listenfd;

	if( epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0 )
	{
		dbg_print("epoll add listen socket failure:%s\n",strerror(errno));
		return -4;
	}

	db = open_database();

	while(1)
	{
		events = epoll_wait(epollfd, event_array, MAX_EVENTS, -1);
		if( events < 0 )
		{
			dbg_print("epoll failure: %s\n", strerror(errno));
            break;
		}
		else if( events == 0 )
		{
			dbg_print("epoll get timeout\n");
			continue;
		}

		for( i=0; i<events; i++)
		{
			if( (event_array[i].events&EPOLLERR) || (event_array[i].events&EPOLLHUP) )
			{
				 dbg_print("epoll_wait get error on fd[%d]: %s\n", event_array[i].data.fd, strerror(errno));
				 epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
				 close(event_array[i].data.fd);
			}

			if( event_array[i].data.fd == listenfd )
			{
				connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
                if( connfd < 0 )
                {

                    dbg_print("Accept new client failure:%s\n",strerror(errno));

                    continue;
                 }

				event.data.fd = connfd;
				event.events = EPOLLIN;
				if( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
				{
					dbg_print("epoll add client socket failure: %s\n", strerror(errno));
 					close(event_array[i].data.fd);
 					continue;
				}
				dbg_print("epoll add new client socket[%d] ok.\n", connfd);
			}
			else
			{
				rv = read(event_array[i].data.fd, buf, sizeof(buf));
				if( rv <= 0 )
				{
					dbg_print("socket[%d] read failure or get disconncet and will be removed.\n",
					event_array[i].data.fd);
 					epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
 					close(event_array[i].data.fd);
 					continue;
				}
				else
				{
					dbg_print("socket[%d] read get %d bytes data\n", event_array[i].data.fd, rv);
			
					sscanf(buf,"%s %f %s %s",devid, &temperature, date,time);//解析数据
					snprintf(datetime,sizeof(datetime),"%s %s",date,time);
		
					printf("%s %.2f %s",devid, temperature, datetime);
					store_database(db,devid,&temperature,datetime);//存入数据库
				}
			}
		}
	}

	sqlite3_close(db);

CleanUp:
	close(listenfd);
	return 0;

}


static inline void print_usage(char *progname)
{
 	printf("Usage: %s [OPTION]...\n", progname);

 	printf(" %s is a socket server program, which used to verify client and echo back string from it\n",progname);
 	printf("\nMandatory arguments to long options are mandatory for short options too:\n");

	printf(" -b[daemon ] set program running on background\n");
 	printf(" -p[port ] Socket server port address\n");
 	printf(" -h[help ] Display this help information\n");

 	printf("\nExample: %s -b -p 8900\n", progname);
 	return ;
}

int socket_server_init(char *listen_ip, int listen_port)
{
	struct sockaddr_in	servaddr;
	int			rv = 0;
	int			on = 1;
	int			listenfd = -1;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if( listenfd < 0 )
	{
		dbg_print("Use socket() to create a TCP socket failure: %s\n", strerror(errno));
 		return -1;
	}

	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);

	if( !listen_ip )
	{
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		if( inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0 )
		{
			dbg_print("inet_pton() set listen IP address failure.\n");
 			rv = -2;
 			goto CleanUp;
		}
	}

	if( bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
	{
		dbg_print("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
 		rv = -3;
 		goto CleanUp;
	}

	if(listen(listenfd, 1024) < 0)
 	{
 		dbg_print("Use bind() to bind the TCP socket failure: %s\n", strerror(errno));
 		rv = -4;
 		goto CleanUp;
 	}

CleanUp:
	if( rv < 0 )
	{
		close(listenfd);
	}
	else
	{
		rv = listenfd;
	}

	return rv;
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};

	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	dbg_print("Set socket open fd max count to %ld\n",limit.rlim_max);
}


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

