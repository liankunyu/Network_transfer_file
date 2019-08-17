/**	   @file udp_recv_thread.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端udp接收线程的实现，主函数调用用于接收客户端udp协议信息
 *
 *	   @author		   xxx
 *	   @date		   2019/08/15
 *
 *	   @note 下面的note和warning为可选项目
 *	   @note 这里填写本文件的详细功能描述和注解
 *	   @note 历史记录：
 *	   @note V1.0.0  添加
 *
 *	   @warning 这里填写本文件相关的警告信息
 */

#include <time.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "serverthread.h"
#include "startsocket.h"
#include "protocol.h"
#include "sha256.h"
#include "main.h"

extern char *connect_ip;

/**	@fn	void *tcp_recv_thread(void * arg) 
 *	@brief	客户端UDP下载文件线程 
 *	@param  arg  参数同步，线程创建的时候.
 *	@return	无. 
 */
void *udp_recv_thread(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());
	int udp_fd = *(int *)arg;

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(UDPSERVER_PORT);
	inet_pton(AF_INET, connect_ip, &server_addr.sin_addr);
	socklen_t addrlen = sizeof(server_addr);

	char filepath[MAX_DATA_SIZE];
	char filename[MAX_DATA_SIZE];
	int recv_len = 0;
	printf("请输入下载文件路径\n");
	scanf("%s", filepath);
	int file_path_len = sendto(udp_fd, &filepath, sizeof(filepath), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if (file_path_len < 0)
	{
		printf("下载文件路径错误!\n");
	}
	else
	{
		int i = 0, k = 0;
		for (i = strlen(filepath); i >= 0; i--)
		{
			if (filepath[i] != '/')
			{
				k++;
			}
			else
			{
				break;
			}
		}
		strcpy(filename, filepath + (strlen(filepath) - k) + 1); //从文件路径中获取文件名
																 //printf("download file name :%s\n",filename);
	}

	int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd < 0)
	{
		perror("文件接收线程: ");
		return NULL;
	}

	/*接收文件*/
	char buf[BUFF_SIZE];
	while ((recv_len = recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr *)&server_addr, &addrlen)) > 0)
	{

		write(fd, buf, recv_len);
		bzero(buf, BUFF_SIZE);
		/*向服务器发送接收确认信息*/
		sendto(udp_fd, buf, recv_len, 0, (struct sockaddr *)&server_addr, addrlen);
		bzero(buf, BUFF_SIZE);
		/*大文件不能有*/
		/*break;*/
	}

	/*用来存储客户端sha256签名*/
	char digestclient[1000] = {0};
	char digestserver[1000] = {0};
	/*获得文件大小*/
	struct stat statbuf;
	stat(filename, &statbuf);
	int filesize = statbuf.st_size;
	char *readbuf = (char *)malloc(filesize + 1);
	read(fd, readbuf, filesize);
	/*sha256 验证*/
	sha_256(digestclient, (char *)readbuf);
	/*接收验证码*/
	recvfrom(udp_fd, digestserver, 1000, 0, (struct sockaddr *)&server_addr, &addrlen);
	if (0 == strcmp((const char *)digestclient, (const char *)digestserver))
	{
		printf("经过sha256校验，文件接收完成\n");
	}
	else
	{
		printf("文件接收失败\n");
	}
	close(fd); //关闭文件

	close(fd);
	return NULL;
}
