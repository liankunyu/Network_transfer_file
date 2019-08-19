/**	   @file udp.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器udp线程，应答客户端的udp连接
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

#include "udp.h"
#include "sha256.h"
#include "protocol.h"
#include "startsocket.h"

/**	   @brief 服务器探测线程处理函数
 *	   @param arg 探测线程输入参数
 *	   @param 
 *	   @return 返回void
 */
void *udp_process(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());

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

	int sockfd = 0;
	/*udp连接的客户端id*/
	int client_sock = 0;
	Net_packet packet;
	sockfd = create_udp_server(UDPSERVER_PORT);
	if (-1 == sockfd)
	{
		exit(-1);
	}

	/*客户端地址结构*/
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	while (1)
	{
		/*接收数据包，根据packet.data_type判断是上传/下载*/
		recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &addrlen);
		memset(filepath, '\0', sizeof(filepath));
		file_path_len = recvfrom(sockfd, filepath, 256, 0, (struct sockaddr *)&client_addr, &addrlen);
		printf("文件路径 :%s\n", filepath);

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
		if (packet.data_type == 'u' | packet.data_type == 'U')
		{

			int fp = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
			if (fp != -1)
			{
				while (recv_len = recvfrom(sockfd, buffer, BUFF_SIZE, 0, (struct sockaddr *)&client_addr, &addrlen))
				{

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
					sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&client_addr, addrlen);
					bzero(buffer, BUFF_SIZE);
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
				read(fp, readbuf, filesize);
				/*sha256 验证*/
				sha_256(digest, (char *)readbuf);
				/*发送验证码*/
				sendto(sockfd, digest, 1000, 0, (struct sockaddr *)&client_addr, addrlen);
				printf("udp客户端文件上传完成\n");
				/*释放malloc申请的内存*/
				free(readbuf);
				/* 关闭文件 */
				close(fp);
			}
		}

		if (packet.data_type == 'd' | packet.data_type == 'D')
		{
			int send_len = 0;
			int fd = open(filepath, O_RDONLY, 0666); //只读打开文件
			if (fd < 0)
			{
				perror("file_send_thread");
				continue;
			}
			/**********发送文件************/
			char buf[BUFF_SIZE] = {0};
			int buf_len = 0;
			while ((buf_len = read(fd, buf, sizeof(buf))) > 0)
			{
				if ((send_len = sendto(sockfd, buf, buf_len, 0, (struct sockaddr *)&client_addr, addrlen)) < 0)
				{
					printf("发送错误! \n");
					break;
				}
				bzero(buf, BUFF_SIZE);
				recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addrlen);
				bzero(buf, BUFF_SIZE);
				//break;
			}
			/*最后发了0字节的数据，可通过此数据帧向服务器发送文件完成指令*/
			sendto(sockfd, buf, 0, 0, (struct sockaddr *)&client_addr, addrlen);

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
			read(fd, readbuf, filesize);
			/*sha256 验证*/
			sha_256(digest, (char *)readbuf);
			/*发送验证码*/
			sendto(sockfd, digest, 1000, 0, (struct sockaddr *)&client_addr, addrlen);
			printf("udp发送完成!\n");
			/*释放malloc申请的内存*/
			free(readbuf);
			/*关闭文件*/
			close(fd);
		}
	}
	close(sockfd);
}
