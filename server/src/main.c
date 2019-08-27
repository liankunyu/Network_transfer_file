/**	   @file main.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 网络C/S文件传输工具，支持服务器探测，支持TCP，UDP两种传输协议，支持文件校验
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
#include "sha256.h"
#include "explore.h"
#include "tcp.h"
#include "udp.h"
#include "protocol.h"
//#include "PassiveServer.h"

/**	   @brief 程序主入口
 *	   @param argc输入参数个数
 *	   @param argv输入的参数
 *	   @return 返回0正常退出
 */
int main(int argc, char *argv[])
{
	printf("服务开启\n");

	pthread_t explore_pid, tcp_pid, udp_pid;
	char *explore_msg = "thread";
	char *tcp_msg = "thread";
	char *udp_msg = "thread";

	/*创建探测线程处理client的多播*/
	if (pthread_create(&explore_pid, NULL, explore_process, (void *)&explore_msg) < 0)
	{
		printf("探测线程创建失败\n");
	}
	/*在创建线程后等待100毫秒*/
	usleep(100);
	/*创建tcp线程处理client的tcp连接*/
	if (pthread_create(&tcp_pid, NULL, tcp_process, (void *)&tcp_msg) < 0)
	{
		printf("tcp服务线程创建失败\n");
	}
	usleep(100);
	/*创建udp线程处理client的udp连接*/
	if (pthread_create(&udp_pid, NULL, udp_process, (void *)&udp_msg) < 0)
	{
		printf("udp线程创建失败\n");
	}
	usleep(100);

	void *result = NULL;
	if (pthread_join(explore_pid, &result) < 0)
	{
		printf("回收探测线程失败\n");
		exit(1);
	}
	if (pthread_join(tcp_pid, &result) < 0)
	{
		printf("回收tcp线程失败\n");
		exit(1);
	}
	if (pthread_join(udp_pid, &result) < 0)
	{
		printf("回收udp线程失败\n");
		exit(1);
	}

	return 0;
}
