/*********************************************************************************
 *      Copyright:  (C) 2024 lingyun<lingyun>
 *                  All rights reserved.
 *
 *       Filename:  client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(29/01/24)
 *         Author:  Liao Shengli <2928382441li@gmail>
 *                 
 ********************************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <getopt.h>


#define CONFIG_DEBUG
#ifdef CONFIG_DEBUG
#define dbg_print(format, args...) printf(format,##args)
#else
#define dbg_print(format, args...) do{} while(0)
#endif


#define	MAX_ID_LEN	20
#define MAX_ROW		1000

typedef struct data
{
	float		temperature;
	char	datetime[32];
} data_t;

data_t 	data_arr[MAX_ROW];
int		data_count = 0;


void print_usage(char *progname);					/*命令行参数*/
int connect_socket(char *serv_ip, int port);
int get_devid(char *id, int len);					/*获取序列号*/
int get_temperature(float *temperature);			/*获取温度*/
int get_datetime(char * psDateTime);				/*获取时间*/
int data_string(char *s_buf);
int send_data(int conn_fd, char *buf);
static int callback(void *data, int argc, char **argv, char **azColName);/*sqlite3_execde回调函数*/
sqlite3 *open_database();							/*打开或创建数据库*/
void store_database(sqlite3 *db, float *temperature, char *datetime);
void select_database(sqlite3 *db, int conn_fd);
void delete_database(sqlite3 *db);					/*删除表中的所有数据*/


int main(int argc,char *argv[])
{
	int				conn_fd = -1;
	int				rv = -1;
	int				sock_opt;
	struct tcp_info	optval;
	int				opt_len = sizeof(optval);
	int				net_state = 1;
	float			temperature;
	char			datetime[128];
	char			id[32];
	sqlite3			*db;
	sqlite3_stmt	*stmt;
	ssize_t			send_size;
	char			s_buf[256];	
	char			s2_buf[256];
	int             opt = -1;
	char            *serv_ip =  NULL;
    int             port = 0;
    const char      *optstring = "i:p:h";
    struct option   opts[] = {
                {"ipaddr", required_argument, NULL, 'i'},
                {"prot", required_argument, NULL, 'p'},
                {"help", no_argument, NULL, 'h'},
                {NULL, 0, NULL, 0}
        };

    while ( (opt = getopt_long(argc, argv, optstring, opts, NULL)) != -1 )
    {
		switch (opt)
		{
			case 'i':
				serv_ip = optarg;
				break;
            case 'p':
				port = atoi(optarg);
                break;
			case 'h':
				print_usage(argv[0]);
                return 0;
		}
    }

    if( !serv_ip || !port )
	{
		print_usage(argv[0]);
        return 0;
	}

	db = open_database();
	conn_fd = connect_socket(serv_ip, port);

	while(1)
	{
		sock_opt = getsockopt(conn_fd,IPPROTO_TCP,TCP_INFO,&optval,(socklen_t *)&opt_len);
		if( net_state)//正常
		{
			if( optval.tcpi_state != TCP_ESTABLISHED )//断连
			{
				net_state = 0;
				dbg_print("Server disconnect.\n");
			}
			else
			{
				rv = data_string(s_buf);
        		send_size = send_data(conn_fd,s_buf);
            	if( send_size < 0 )
				{
					dbg_print("Send data failure:%s\n",strerror(errno));
				}
			}
		}
		else //故障
		{
			rv = get_temperature(&temperature);
            rv = get_datetime(datetime);
			store_database(db,&temperature,datetime);
			dbg_print("Data saved to database:%.2f %s\n",temperature, datetime);
			
			conn_fd = connect_socket(serv_ip,port);//重连
		
			if( conn_fd > 0 )//成功
			{
				net_state = 1;
				select_database(db, conn_fd);
			}
		}

	}

	delete_database(db);
	close(conn_fd);
	sqlite3_close(db);

	return 0;
}

void print_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("-i(--ipaddr): sepcify server IP address.\n");
    printf("-p(--port): sepcify server port.\n");
    printf("-h(--help): print this help information.\n");

    return ;
}


//连接服务器
int connect_socket(char *serv_ip, int port)
{
	int						conn_fd = -1;
	int						rv = -1;
	struct sockaddr_in		serv_addr;

	 conn_fd = socket(AF_INET,SOCK_STREAM,0);
	 if( conn_fd < 0 )
	 {
		 dbg_print("Creat socket failure: %s\n",strerror(errno));
		 return -1;
	 }
	 dbg_print("Soket creat fd[%d] successfully\n",conn_fd);

	 memset(&serv_addr,0,sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_port = htons(port);
	 inet_aton(serv_ip,&serv_addr.sin_addr);

	 rv = connect( conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) );
	 if(rv < 0)
	 {
		 dbg_print("Connect to server [%s:%d] failure: %s\n",serv_ip,port,strerror(errno));
		 return -2;
	 }
	 dbg_print("Connect to server [%s:%d] successfully!\n",serv_ip,port);
	 
	 return conn_fd;
}


//获取序列号
int get_devid(char *id, int len)
{
	int		sn=1;
	
	snprintf(id, len,"rpi%04d",sn);
	
	return 0;
}


//获取温度
int get_temperature(float *temperature)
{
    int     fd = -1;		
	int     found = 0;		
    char    buf[128];		
    char    *ptr = NULL;		
    char    w1_path[64] = "/sys/bus/w1/devices/";	
    char    chip[32];		
    DIR     *dirp = NULL;
    struct  dirent *direntp = NULL;

    if( NULL ==(dirp = opendir(w1_path)))	
    {
		dbg_print("open %s failure: %s\n",w1_path,strerror(errno));
        return -1;
    }

    while( NULL != (direntp = readdir(dirp)) )
    {
        if( strstr(direntp->d_name,"28-") )	
        {
    		strncpy(chip,direntp->d_name,sizeof(chip));
            found = 1;	
        }
    }

	closedir(dirp);	

    if( !found )	
    {
        dbg_print("cannot find ds18b20 path\n");
        return -2;
    }

    strncat(w1_path,chip,sizeof(w1_path)-strlen(w1_path));
    strncat(w1_path,"/w1_slave",sizeof(w1_path)-strlen(w1_path));


    fd = open(w1_path,O_RDONLY);	
    if( fd < 0 )
    {
        dbg_print("open file failure:%s\n",strerror(errno));
        return -3;
	}


    memset(buf,0,sizeof(buf));	

    if( read(fd,buf,sizeof(buf)) < 0 )	
    {
        dbg_print("read data failure\n");
        return -4;
        close(fd);
	}


	ptr = strstr(buf,"t=");	
    if( !ptr )
	{
		dbg_print("cannot find t=\n");
        return -5;
		close(fd);
    }

    ptr += 2;	
	*temperature = atof(ptr)/1000;	

	sleep(5);

	return 0;
}

//获取时间
int get_datetime(char * psDateTime) 
{
    time_t		nSeconds;
    struct tm	*pTM;
    
    time(&nSeconds);
    pTM = gmtime(&nSeconds);

    /* 系统日期和时间,格式: yyyymmddHHMMSS */
    sprintf(psDateTime, "%d-%d-%d %d:%d:%d",
            1900+pTM->tm_year, 1+pTM->tm_mon, pTM->tm_mday,
            8+pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
            
    return 0;
}

//将数据合为一个字符串
int data_string(char *s_buf)
{
	int		rv = -1;
	float	temperature;
	char	datetime[128];
	char	id[32];

	rv = get_temperature(&temperature);
	if (rv < 0 )
	{
		dbg_print("get temperature failure\n");
		return -1;
	}

	rv = get_datetime(datetime);
	if( rv != 0 )
	{
		dbg_print("Get time failure\n");
		return -1;
	}

	rv = get_devid(id, MAX_ID_LEN);
	if( rv != 0 )
	{
		dbg_print("Get devid failure\n");
		return -1;
	}
	
	memset(s_buf, 0, sizeof(s_buf));
	sprintf(s_buf, "%s %.2f %s",id, temperature, datetime);

	dbg_print("data:%s\n", s_buf);

	return 0;
}

//发送数据
int send_data(int conn_fd, char *buf)
{
	ssize_t	send_size;

	send_size = send(conn_fd, buf,strlen(buf),0);
	if( send_size == -1 )
	{
		dbg_print("Send data failure:%s\n",strerror(errno));
		return -1;
	}
	return 0;
}

//插入数据
void store_database(sqlite3 *db, float *temperature, char *datetime)
{
	char	*sql;
	int		rc;
	char	*zErrMsg = 0;

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

//打开数据库，如果没有则创建
sqlite3 *open_database()
{
	sqlite3		*db;
	char		*zErrMsg = 0;
	int			rc;
	char		*sql;

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

//数据库的回调函数
static int callback(void *data, int argc, char **argv, char **azColName)
{
   int		i;

   data_arr[data_count].temperature = atof(argv[0] ? argv[0] : "0.00");
   snprintf(data_arr[data_count].datetime,sizeof(data_arr[data_count].datetime), "%s", argv[1] ? argv[1] : "NULL");

   data_count++;
   return 0;
}

void select_database(sqlite3 *db, int conn_fd)
{
	char	*sql;
	int		rc;
	char	*zErrMsg = 0;
	char	id[32];
	int		i = 0;
	char	s_buf[128];

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
		snprintf(s_buf, sizeof(s_buf), "%s %.2f %s", id, data_arr[i].temperature, data_arr[i].datetime);
		dbg_print("data2:%s %.2f %s\n", id, data_arr[i].temperature, data_arr[i].datetime);
		
		send_data(conn_fd,s_buf);
		sleep(1);
	}
	
	delete_database(db);
	sqlite3_close(db);
}


//删除数据库中的数据
void delete_database(sqlite3 *db)
{
	char	*zErrMsg = 0;
	int		rc;
	char	*sql;

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
