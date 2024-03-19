# APUE_Project
#项目介绍
树莓派上通过一线协议连接DS18B20，然后采用网络socket编程同时实现客户端和服务器端程序，其中客户端主要实现定时上报的功能，服务器端用来采集客户端上报的数据并存储到数据库中。

#目录结构
```
.
├── client						//客户端
│   ├── makefile
│   ├── src
│   │   ├── ！
│   │   ├── data.c				//获取序列号和时间，打包和发送数据
│   │   ├── data.h
│   │   ├── debug.h				//调试打印
│   │   ├── ds18b20.c			//获取温度
│   │   ├── ds18b20.h
│   │   ├── makefile
│   │   ├── socket_client.c
│   │   ├── socket_client.h
│   │   ├── sql.c
│   │   └── sql.h
│   └── test
│       ├── main.c
│       └── makefile
├── README.md
└── servern						//服务器端
    ├── makefile
    ├── src
    │   ├── debug.h
    │   ├── makefile
    │   ├── socket_server.c
    │   ├── socket_server.h
    │   ├── sql.c
    │   └── sql.h
    └── test
        ├── main.c
        ├── makefile
        └── test.sh				//通过shell脚本代替make run的两条指令

