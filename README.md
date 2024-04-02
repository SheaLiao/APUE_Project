# 项目简介
树莓派上通过一线协议连接DS18B20，采用网络socket编程实现客户端和服务器端程序，其中客户端主要实现定时上报的功能，服务器端用来采集客户端上报的数据并存储到数据库中。

# 目录结构
```bash
.
├── client
│   ├── include
│   │   ├── database.h
│   │   ├── ds18b20.h
│   │   ├── logger.h
│   │   ├── packet.h
│   │   ├── proc.h
│   │   └── socket.h
│   ├── makefile	#客户端的Makefile，用于编程运行程序
│   ├── openlibs	#开源库
│   │   ├── cJSON
│   │   │   ├── build.sh	#cJSON库的下载安装脚本
│   │   │   └── makefile	#执行脚本
│   │   └── sqlite
│   │       ├── build.sh
│   │       └── makefile
│   ├── socket_client.c		#客户端程序
│   └── src
│       ├── database.c		#数据库操作
│       ├── ds18b20.c		#获取温度
│       ├── logger.c		#日志系统
│       ├── makefile		#编译生成动态库和静态库
│       ├── packet.c		#数据处理
│       ├── proc.c			#信号处理
│       └── socket.c		#socket通信
├── README.md				#项目说明文档
└── server
    ├── include
    │   ├── database.h
    │   ├── logger.h
    │   └── socket.h
    ├── makefile		#服务器端的Makefile，编译运行程序
    ├── openlibs		#开源库
    │   └── sqlite
    │       ├── build.sh
    │       └── makefile
    ├── socket_server.c
    └── src
        ├── database.c		#数据库操作
        ├── logger.c		#日志系统
        ├── makefile		#编译生成动态库和静态库
        └── socket.c		#socket通信
```

# 功能
## 客户端
- 通过命令行参数设置连接服务器的主机名和端口号；
- 通过命令行参数设置定时采样的时间；
- 可以以字符串或JSON格式通过socket通信上报到服务器端；
- 可以捕捉信号使程序正常退出；
- 当服务器断开时，将采集的数据存放至本地数据库中，等再次连接上服务器时，将保存在数据库中的数据发送给服务器；
- 支持日志系统。

## 服务器端
- 通过命令行参数设置服务器程序监听的端口号；
- 服务器支持多个客户端并发访问；
- 可以捕捉信号使程序正常退出；
- 服务器端接受到客户端上报的数据并解析成功之后，将数据永久存储到数据库中；
- 支持日志系统。

# 使用说明
(以下以客户端为例，服务器端相同)
## 1、下载安装
下载安装sqlite3等开源库
```bash
cd APUE_Project/client/openlibs/sqlite/
make
```
编译生成动态库
```bash
cd APUE_Project/client/src/
make
```

## 2、运行
运行项目需要提供一些参数：
```bash
./socket_client -i xxx -p xxx -t x
```

