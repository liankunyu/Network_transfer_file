/**	   @file tcp_send_thread.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端tcp发送线程的实现，主函数调用用于发送客户端tcp协议信息
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

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "serverthread.h"
#include "startsocket.h"
#include "protocol.h"

/**	@fn	void *tcp_send_thread(void * arg) 
 *	@brief	客户端TCP上传文件线程 
 *	@param  arg  参数同步，线程创建的时候.
 *	@return	无. 
 */
void *tcp_send_thread(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());
	/*把参数传进来，创建线程的时候*/
	int tcp_fd = *((int *)(arg));

	/*printf("%s:%d\n",__func__,__LINE__);*/

	/*读取要上传的文件路径 */
	char path[MAX_DATA_SIZE];
	printf("请输入上传文件路径 \n");
	scanf("%s", path);

	/*只读打开文件*/
	int fd = open(path, O_RDONLY, 0666);
	if (fd < 0)
	{
		perror("文件上传线程 ");
		return NULL;
	}

	/* 发送文件路径名(包含路径) */
	send(tcp_fd, &path, sizeof(path), 0);

	/* 发送文件 */
	char buf[BUFF_SIZE] = {0};
	int buf_len = 0;
	while ((buf_len = read(fd, buf, sizeof(buf))) > 0)
	{
		if (send(tcp_fd, buf, buf_len, 0) < 0)
		{
			printf("TCP上传失败 ! \n");
			break;
		}
		bzero(buf, BUFF_SIZE);
	}

	/*用来存储客户端sha256签名*/
	char digestclient[1000] = {0};
	char digestserver[1000] = {0};
	/*获得文件大小*/
	struct stat statbuf;
	stat(path, &statbuf);
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
		printf("经过sha256校验，文件上传完成\n");
	}
	else
	{
		printf("文件接收失败\n");
	}
	/*释放malloc申请的内存*/
	free(readbuf);
	/* 关闭文件 */
	close(fd);
	return NULL;
}
