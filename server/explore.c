/**	   @file explore_process.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器探测线程，应答客户端的多播
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
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <pthread.h>

#include "protocol.h"
#include "explore.h"

/**@fn  int socket_init(void)
  *  @brief  UDP的初始化
  *  
  *  @param  无
  *  @return -1表示写入失败。sockfd表示写入成功。 
  */
int socket_init(void)
{
	int sockfd = -1;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		perror("sockfd");
		exit(-1);
	}

	struct sockaddr_in sockaddr_local;

	/*set local ip address*/
	bzero(&sockaddr_local, sizeof(sockaddr_local));
	sockaddr_local.sin_family = AF_INET;
	sockaddr_local.sin_port = htons(EXPOTOCOL_PORT);
	sockaddr_local.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (-1 == bind(sockfd, (const struct sockaddr *)&sockaddr_local, sizeof(sockaddr_local)))
	{
		perror("bind");
		close(sockfd);
		exit(-1);
	}

	return sockfd;
}

/**@fn	send_ack(int sockfd, const struct sockaddr* des_address, socklen_t lenth)
 *	@brief	发送ack
 *	
 *	@param	sockfd        socket创建符
 *	@param	des_address   目标IP地址
 *  @param	lenth         地址长度
 *	@return	-1失败          成功，返回实际发送长度
 */
int send_ack(int sockfd, const struct sockaddr *des_address, socklen_t lenth)
{
	int ret = -1;
	cmd_pack ack_pack =
		{
			.server_port = {8848, 8849, 8850},
			.head = CMD_HEAD,
			.type = ACK,
			.pack_size = sizeof(cmd_pack),

		};
	ret = sendto(sockfd, &ack_pack, sizeof(ack_pack), 0, des_address, lenth);
	if (ret < 0)
	{
		perror("sendto");
		exit(-1);
	}
	return ret;
}

/**	   @brief 服务器探测线程处理函数
 *	   @param arg 探测线程输入参数
 *	   @param 
 *	   @return 返回void
 */
void *explore_process(void *arg)
{
	/*设置分离属性*/
	pthread_detach(pthread_self());
	int sockfd = socket_init();
	int ret = -1;
	struct sockaddr_in data_from_addr;
	socklen_t data_from_addrlen = sizeof(data_from_addr);
	while (1)
	{
		cmd_pack cmd_recv;
		bzero(&cmd_recv, sizeof(cmd_pack));
		ret = recvfrom(sockfd, &cmd_recv, sizeof(cmd_recv), 0, (struct sockaddr *)&data_from_addr, &data_from_addrlen);
		if (ret < 0)
		{
			perror("recvfrom");
			exit(-1);
		}
		if (cmd_recv.pack_size == sizeof(cmd_recv) && cmd_recv.head == CMD_HEAD)
		{
			switch (cmd_recv.type)
			{
			case DISCOVERY:
				ret = send_ack(sockfd, (const struct sockaddr *)&data_from_addr, data_from_addrlen);
				printf("the num bytes send is %d.\n", ret);
				break;
			case TCP:
				printf("erro\n");
			case UDP:
				printf("erro\n");
			}
		}
		printf("success\n");
	}
}
