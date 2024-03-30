# APUE_Project
#项目介绍
树莓派上通过一线协议连接DS18B20，然后采用网络socket编程同时实现客户端和服务器端程序，其中客户端主要实现定时上报的功能，服务器端用来采集客户端上报的数据并存储到数据库中。

#目录结构
```
.
├── client
│   ├── include
│   │   ├── database.h
│   │   ├── ds18b20.h
│   │   ├── logger.h
│   │   ├── packet.h
│   │   └── socket.h
│   ├── makefile
│   ├── socket_client.c
│   ├── sqlite
│   │   ├── build.sh
│   │   └── makefile
│   └── src
│       ├── database.c
│       ├── ds18b20.c
│       ├── logger.c
│       ├── makefile
│       ├── packet.c
│       └── socket.c
├── README.md
└── server
    ├── include
    │   ├── database.h
    │   ├── logger.h
    │   └── socket.h
    ├── makefile
    ├── socket_server.c
    ├── sqlite
    │   ├── build.sh
    │   └── makefile
    └── src
        ├── database.c
        ├── logger.c
        ├── makefile
        └── socket.c

