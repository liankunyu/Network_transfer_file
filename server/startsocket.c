/**	   @file startsocket.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器创建tcp/udp的socket，负责客户端连接
 *
 *	   @author		   xxx
 *	   @date		   2019/08/07
 *
 *	   @note 下面的note和warning为可选项目
 *	   @note 这里填写本文件的详细功能描述和注解
 *	   @note 历史记录：
 *	   @note V1.0.0  添加
 *
 *	   @warning 这里填写本文件相关的警告信息
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "startsocket.h"

#define LISTEN_SIZE 20

/**	@fn	start_server(int port, int type)
 *	@brief	建立服务器
 *	@port   服务器端口.
 *	@param	type	    TCP/UDP连接方式选择 
 *	@return	-1表示写入失败。sock_fd表示写入成功。 
 */

int start_server(int port, int type)
{
    /*建立服务器套接字*/
    int sock_fd = socket(AF_INET, type, 0);
    if (sock_fd < 0)
    {
        printf("create socket error\n");
        return -1;
    }

    int reuse = 1;
    /*SO_REUSEADDR允许重用本地地址和端口，SO_BROADCAST　允许发送广播*/
    setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &reuse, sizeof(int));
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error\n");
        return -1;
    }
    //TCP
    if (SOCK_STREAM == type)
    {

        if (listen(sock_fd, LISTEN_SIZE) < 0)
        {
            printf("listen error\n");
            return -1;
        }
        printf("tcp server start\n");
    }
    else
        printf("udp server start\n");
    return sock_fd;
}

int create_tcp_server(int port)
{
    start_server(port, SOCK_STREAM);
}

int create_udp_server(int port)
{
    start_server(port, SOCK_DGRAM);
}
