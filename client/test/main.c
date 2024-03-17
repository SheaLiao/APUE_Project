/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  main.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(13/03/24)
 *         Author:  Liao Shengli <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "13/03/24 16:17:50"
 *                 
 ********************************************************************************/

#include "socket_client.h"
#include "debug.h"
#include "sql.h"
#include "ds18b20.h"
#include "data.h"

void print_usage(char *progname)
{
    printf("%s usage: \n",progname);
    printf("-i(--ipaddr): sepcify server IP address.\n");
    printf("-p(--port): sepcify server port.\n");
    printf("-h(--help): print this help information.\n");

    return ;
}


int main(int argc,char *argv[])
{
    int             conn_fd = -1;
    int             rv = -1;
    int             sock_opt;
    struct tcp_info optval;
    int             opt_len = sizeof(optval);
    int             net_state = 1;
    float           temperature;
    char            datetime[128];
    char            id[32];
    sqlite3         *db;
    ssize_t         send_size;
    char            s_buf[256];
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
                net_state = 0;//标记为故障
                dbg_print("Server disconnect.\n");
            }
            else //连接正常，发送数据
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
            dbg_print("Data saved to database\n");

            conn_fd = connect_socket(serv_ip,port);//重连

            if( conn_fd > 0 )//重连成功
            {
                net_state = 1;//将网络状态标记为正常
                select_database(db, conn_fd);
            }
        }

    }

    delete_database(db);
    close(conn_fd);
    sqlite3_close(db);

    return 0;
}

