/**	   @file tcp.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器tcp线程，应答客户端的tcp连接
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

#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcp.h"
#include "sha256.h"
#include "protocol.h"
#include "startsocket.h"

/*处理tcp客户端上传/下载文件请求*/
void *tcp_data_handle(void *arg);

/**	   @brief 服务器tcp线程处理函数
 *	   @param arg tcp线程输入参数
 *	   @param 
 *	   @return 返回void
 */
void *tcp_process(void *arg)
{
	/*不能设置分离属性，与pthread_join冲突*/
	/*pthread_detach(pthread_self());*/

	int sockfd;
	/*tcp连接的客户端id*/
	int client_sock;
	sockfd = create_tcp_server(TCPSERVER_PORT);

	if (-1 == sockfd)
	{
		exit(-1);
	}
	while (1)
	{
		/*接受客户端连接*/
		socklen_t addrlen = sizeof(struct sockaddr);
		/*客户端地址结构*/
		struct sockaddr_in client_addr;
		client_sock = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);

		if (client_sock < 0)
		{
			printf("accept error\n");
		}
		else
		{
			printf("accept success\n");
		}
		/*创建新线程负责处理客户端的消息*/
		pthread_t pid;
		if (pthread_create(&pid, NULL, tcp_data_handle, (void *)&client_sock) < 0)
		{
			printf("pthread_create error\n");
		}
	}
	close(sockfd);
}

/**	@fn	void *tcp_data_handle(void * arg) 
 *	@brief	用于处理客户端TCP上传/下载文件 
 *	@param  arg  
 *	@return	void
 */
void *tcp_data_handle(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());
	/*参数传进来，同步client_sock*/
	int client_sock = *((int *)(arg));

	/*文件路径*/
	char filepath[MAX_DATA_SIZE] = {0};
	/*文件名*/
	char filename[MAX_DATA_SIZE] = {0};
	/*缓冲区*/
	char buffer[BUFF_SIZE] = {0};
	/*文件接收长度*/
	int recv_len = 0;
	/*文件写入长度*/
	int write_len = 0;
	/*文件路径长度*/
	int file_path_len = 0;

	Net_packet packet;
	/*接收数据包，根据packet.data_type判断是上传/下载*/
	recv(client_sock, &packet, sizeof(packet), 0);
	memset(filepath, '\0', sizeof(filepath));
	file_path_len = recv(client_sock, filepath, 256, 0);
	printf("文件路径:%s\n", filepath);

	if (file_path_len < 0)
	{
		printf("接收文件路径错误!\n");
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
		strcpy(filename, filepath + (strlen(filepath) - k) + 1);
		printf("文件名 :%s\n", filename);
	}
	if ((packet.data_type == 'u') | (packet.data_type == 'U'))
	{

		int fp = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (fp != -1)
		{
			fd_set rset;
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
				FD_SET(client_sock, &rset);
				select(client_sock + 1, &rset, NULL, NULL, &timeout);
				if (FD_ISSET(client_sock, &rset))
				{
					recv_len = recv(client_sock, buffer, BUFF_SIZE, 0);
					if (recv_len < 0)
					{
						printf("接收错误!\n");
						break;
					}
					write_len = write(fp, buffer, recv_len);
					if (write_len < recv_len)
					{
						printf("写入错误!\n");
						break;
					}
					bzero(buffer, BUFF_SIZE);
				}
				else
				{
					break;
				}
			}

			/*用来存储sha256签名*/
			char digest[1000] = {0};
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
			//将文件的读写位置移动到文件的开始
			lseek(fp, 0, SEEK_SET);
			read(fp, readbuf, filesize);
			/*sha256 验证*/
			sha_256(digest, (char *)readbuf);
			/*发送验证码*/
			send(client_sock, digest, 1000, 0);
			printf("文件接收完成\n");
			/*释放malloc申请的内存*/
			free(readbuf);
			/* 关闭文件 */
			close(fp);
		}
		else
		{
			printf("文件名是 null!\n");
		}
		close(client_sock);
	}

	if ((packet.data_type == 'd') | (packet.data_type == 'D'))
	{
		int send_len;
		int fd = open(filepath, O_RDONLY, 0666);
		if (fd < 0)
		{
			perror("file_send_thread");
			return NULL;
		}

		/**********发送文件************/
		char buf[BUFF_SIZE] = {0};
		int buf_len = 0;
		while ((buf_len = read(fd, buf, sizeof(buf))) > 0)
		{
			if ((send_len = send(client_sock, buf, buf_len, 0)) < 0)
			{
				printf("发送失败! \n");
				break;
			}
			bzero(buf, BUFF_SIZE);
		}

		/*使用recv函数阻塞send验证码，不然会和上文环境中的send粘在一起*/
		recv(client_sock, buf, sizeof(buf), 0);
		/*用来存储sha256签名*/
		char digest[1000] = {0};
		/*获得文件大小*/
		struct stat statbuf;
		stat(filepath, &statbuf);
		int filesize = statbuf.st_size;
		char *readbuf = (char *)malloc(filesize + 1);
		if (NULL == readbuf)
		{
			printf("文件读取内存申请失败,程序退出！\n");
			exit(1);
		}
		//将文件的读写位置移动到文件的开始
		lseek(fd, 0, SEEK_SET);
		read(fd, readbuf, filesize);
		/*sha256 验证*/
		sha_256(digest, (char *)readbuf);
		/*发送验证码*/
		send(client_sock, digest, 1000, 0);
		printf("文件发送完成\n");
		/*释放malloc申请的内存*/
		free(readbuf);
		/*关闭文件 */
		close(fd);
		/*关闭socket */
		close(client_sock);
	}
}
