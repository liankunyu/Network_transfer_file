/**	   @file tcp_recv_thread.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端tcp接收线程的实现，主函数调用用于接收客户端tcp协议信息
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

#include <sys/time.h>
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
#include "protocol.h"
#include "sha256.h"

/**	@fn	void *tcp_recv_thread(void * arg) 
 *	@brief	客户端TCP下载文件线程 
 *	@param  arg  参数同步，线程创建的时候.
 *	@return	无. 
 */
void *tcp_recv_thread(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());
	int tcp_fd = *(int *)arg;
	char filepath[MAX_DATA_SIZE];
	char filename[MAX_DATA_SIZE];
	int recv_len = 0;
	printf("请输入下载文件路径 \n");
	scanf("%s", filepath);
	/*将下载文件路径发送到服务器*/
	int file_path_len = send(tcp_fd, &filepath, sizeof(filepath), 0);
	if (file_path_len < 0)
	{
		printf("下载文件路径错误 !\n");
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
		/*从文件路径中获取文件名*/
		strcpy(filename, filepath + (strlen(filepath) - k) + 1);
	}

	/*计算传输用时*/
	struct timeval starttime, endtime;
	gettimeofday(&starttime, 0);

	int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd < 0)
	{
		perror("文件接收线程: ");
		return NULL;
	}
	fd_set rset;
	char buf[BUFF_SIZE] = {0};
	struct timeval timeout =
		{
			.tv_sec = 0,
			.tv_usec = RESEND_TIME_US,
		};
	while (1)
	{
		/*将指定的文件描述符集清空*/
		FD_ZERO(&rset);
		/*select用来监视socketfd的状态变化,程序会停在select这里等待，直到sockfd发生变化，这样可以防止recvfrom函数一直阻塞，https://blog.csdn.net/baidu_35381300/article/details/51736431*/
		FD_SET(tcp_fd, &rset);
		select(tcp_fd + 1, &rset, NULL, NULL, &timeout);
		if (FD_ISSET(tcp_fd, &rset))
		{
			recv_len = recv(tcp_fd, buf, sizeof(buf), 0);
			write(fd, buf, recv_len);
			bzero(buf, BUFF_SIZE);
		}
		else
		{
			break;
		}
	}

	/*开启tcp.c中recv的阻塞，使其发送验证码*/
	memset(buf, 1, 100);
	send(tcp_fd, buf, 100, 0);
	/*用来存储客户端sha256签名*/
	char digestclient[1000] = {0};
	char digestserver[1000] = {0};
	/*获得文件大小*/
	struct stat statbuf;
	stat(filename, &statbuf);
	int filesize = statbuf.st_size;
	char *readbuf = (char *)malloc(filesize + 1);
	if (NULL == readbuf)
	{
		printf("文件读取内存申请失败,程序退出！\n");
		exit(1);
	}
	read(fd, readbuf, filesize);
	/*sha256 验证*/
	sha_256(digestclient, (char *)readbuf);
	/*接收验证码*/
	recv(tcp_fd, digestserver, 1000, 0);
	if (0 == strcmp((const char *)digestclient, (const char *)digestserver))
	{
		printf("经过sha256校验，文件下载完成\n");
	}
	else
	{
		printf("文件接收失败\n");
	}

	/*计算传输用时*/
	gettimeofday(&endtime, 0);
	double timeuse = 1000000 * (endtime.tv_sec - starttime.tv_sec) + endtime.tv_usec - starttime.tv_usec;
	/*除以1000则进行毫秒计时，如果除以1000000则进行秒级别计时，如果除以1则进行微妙级别计时*/
	timeuse /= 1000;
	printf("文件传输用时=%f\n", timeuse);

	/*释放malloc申请的内存*/
	free(readbuf);
	/*关闭文件*/
	close(fd);
	return NULL;
}
