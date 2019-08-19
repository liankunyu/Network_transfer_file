/**	   @file client_main.c
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端主函数
 *
 *	   @author		   xxx
 *	   @date		   2019/08/12
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "main.h"
#include "protocol.h"
#include "startsocket.h"
#include "serverthread.h"

/*最多服务器数量*/
#define MAXNUM_SER 30
static struct sockaddr_in ser_addr[MAXNUM_SER];
static int server_total_num;
/*当前连接服务器的地址信息*/
static struct sockaddr_in ser_connct;
char *connect_ip = "1234";

/**	@fn	int start_discovery(void)
 *	@brief	开始查找服务器
 *	
 *	@param	无
 *	@return	0表示写入失败。1表示写入成功。 
 */
int start_discovery(void)
{
	server_total_num = 0;
	int sockfd = -1;
	int ret = -1;
	const int optval = 1;
	struct sockaddr_in data_from_addr;
	socklen_t data_from_addrlen;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		perror("sockfd");
		exit(-1);
	}

	/*设置sockfd广播模式*/
	ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
	if (-1 == ret)
	{
		perror("setsockopt");
		close(sockfd);
		exit(-1);
	}
	cmd_pack discovery_pack =
		{
			.head = CMD_HEAD,
			.type = DISCOVERY,
			.pack_size = sizeof(cmd_pack),
			.retry = 0,
		};

	cmd_pack ack_pack;
	//bzero(&ack_pack, sizeof(ack_pack));

	/*设置广播目标IP地址 */
	struct sockaddr_in sockaddr_bro;
	bzero(&sockaddr_bro, sizeof(sockaddr_bro));
	sockaddr_bro.sin_family = AF_INET;
	sockaddr_bro.sin_port = htons(EXPOTOCOL_PORT);
	sockaddr_bro.sin_addr.s_addr = inet_addr(BRO_ADDR);

	/*设置本地IP地址*/
	struct sockaddr_in sockaddr_local;
	bzero(&sockaddr_local, sizeof(sockaddr_local));
	sockaddr_local.sin_family = AF_INET;
	sockaddr_local.sin_port = 0;
	sockaddr_local.sin_addr.s_addr = inet_addr("0.0.0.0");

	if (-1 == bind(sockfd, (const struct sockaddr *)&sockaddr_local, sizeof(sockaddr_local)))
	{
		perror("bind");
		close(sockfd);
		exit(-1);
	}

	fd_set rset;
	/*将指定的文件描述符集清空*/
	FD_ZERO(&rset);
	struct timeval timeout =
		{
			.tv_sec = 0,
			.tv_usec = RESEND_TIME_US,
		};
	for (discovery_pack.retry = 0; discovery_pack.retry < MAX_RETRY; discovery_pack.retry++)
	{
		/*向广播段地址发送探测包*/
		ret = sendto(sockfd, &discovery_pack, sizeof(discovery_pack), 0,
					 (const struct sockaddr *)&sockaddr_bro, sizeof(sockaddr_bro));
		if (-1 == ret)
		{
			perror("sendto");
			close(sockfd);
			exit(-1);
		}
		/*printf("the num bytes send is %d.\n", ret);*/
		/*select用来监视socketfd的状态变化,程序会停在select这里等待，直到sockfd发生变化，这样可以防止recvfrom函数一直阻塞，https://blog.csdn.net/baidu_35381300/article/details/51736431*/
		FD_SET(sockfd, &rset);
		select(sockfd + 1, &rset, NULL, NULL, &timeout);
		timeout.tv_sec = 0;
		timeout.tv_usec = RESEND_TIME_US;
		int i = 0;
		for (i = 0; i < MAXNUM_SER; i++)
		{
			if (FD_ISSET(sockfd, &rset))
			{

				bzero(&ack_pack, sizeof(ack_pack));
				recvfrom(sockfd, &ack_pack, sizeof(ack_pack), 0,
						 (struct sockaddr *)&(ser_addr[server_total_num]), &data_from_addrlen);
				if (ack_pack.head == CMD_HEAD && ack_pack.type == ACK && ack_pack.pack_size == sizeof(cmd_pack))
				{
					server_total_num++;
					discovery_pack.retry = 0;
				}
				FD_SET(sockfd, &rset);
				select(sockfd + 1, &rset, NULL, NULL, &timeout);
				timeout.tv_sec = 0;
				timeout.tv_usec = RESEND_TIME_US;
			}
		}
		if (server_total_num > 0)
		{
			return 1;
		}

		printf("重试发送探测包\n");
	}
	if (discovery_pack.retry >= MAX_RETRY)
	{
		return 0;
	}
}

/**	@fn	int server_discovery(void)
 *	@brief	查找服务器
 *	
 *	@param	无
 *	@return	0表示写入失败。1表示写入成功。 
 */
int server_discovery(void)
{
	int ret = -1;
	int i;
	ret = start_discovery();
	if (0 == ret)
	{
		printf("没有可用的服务器\n");
		/*等待服务器可用*/
		while (!start_discovery())
			;
	}
	if (1 == ret)
	{
		for (i = 0; i < server_total_num; i++)
		{
			printf("可用服务器号和IP地址：\n");
			printf("#%d, ip:[%s]\n", i, inet_ntoa(ser_addr[i].sin_addr));
		}
	}

	while (1)
	{
		printf("请输入数字号码以选择服务器或输入任何单个非数字字符刷新服务器：");
		setbuf(stdin, NULL);
		ret = scanf("%d", &i);
		if (ret == 0)
		{
			while (!start_discovery())
				;
			for (i = 0; i < server_total_num; i++)
			{
				printf("可用服务器号和IP地址： \n");
				printf("#%d, ip:[%s]\n", i, inet_ntoa(ser_addr[i].sin_addr));
			}
			continue;
		}
		if (i < 0 || i >= server_total_num)
		{
			printf("输入错误！可用服务器编号范围为 [0, %d)\n", server_total_num);
			continue;
		}
		else
		{
			connect_ip = inet_ntoa(ser_connct.sin_addr);
			memcpy(&ser_connct, &ser_addr[i], sizeof(struct sockaddr_in));
			printf("您选择的服务器，IP地址： %s, Port number: %d.\n",
				   inet_ntoa(ser_connct.sin_addr), ntohs(ser_connct.sin_port));
			break;
		}
	}

	return 0;
}

/**	@fn	main(int argc,char *argv[])
 *	@brief	主函数.
 *	@param	argv[1]	  客户端TCP/UDP连接方式选择 
 *	@return	-1表示写入失败。0表示写入成功。 
 */
int main(int argc, char *argv[])
{
	server_discovery();
	/*packet.data_type  U--上传；D--下载*/
	Net_packet packet;
	int inputnum = 0;
	while (1)
	{
		printf("\n");
		bzero(&packet, sizeof(packet));
		while (1)
		{
			inputnum = 0;
			printf("请输入文件传输类型，输入1:TCP连接，2:UDP连接\n");
			/*清空标准输入*/
			getchar();
			scanf("%d", &inputnum);
			if (1 == inputnum)
			{
				printf("进入TCP文件传输 \n");
				break;
			}
			if (2 == inputnum)
			{
				printf("进入UDP文件传输 \n");
				break;
			}
			else
			{
				printf("输入错误！输入值编号范围为1或2\n");
				continue;
			}
		}

		/************************ tcp *******************************/
		if (1 == inputnum)
		{
			/*创建连接*/
			int tcp_fd = connect_tcp(inet_ntoa(ser_connct.sin_addr), TCPSERVER_PORT);
			if (-1 == tcp_fd)
			{
				return -1;
			}
			else
			{
				while (1)
				{
					printf("请输入文件传输类型，输入U/u:上传，D/d:下载\n");
					/*清空标准输入*/
					getchar();
					/*输入指令，上传或下载*/
					scanf("%c", &packet.data_type);
					if (packet.data_type == 'U' | packet.data_type == 'u' | packet.data_type == 'D' | packet.data_type == 'd')
					{
						break;
					}
					else
					{
						printf("输入错误！输入值范围为U/u,D/d\n");
						continue;
					}
				}
				/*发送指令*/
				send(tcp_fd, &packet, sizeof(packet), 0);
				if (packet.data_type == 'U' | packet.data_type == 'u')
				{
					/*tcp文件发送线程*/
					pthread_t tcp_send_tid;
					void *result = NULL;
					if (pthread_create(&tcp_send_tid, NULL, tcp_send_thread, (void *)&tcp_fd) != 0)
					{
						return 1;
					}
					/*阻塞本线程*/
					pthread_join(tcp_send_tid, &result);
				}
				if (packet.data_type == 'D' | packet.data_type == 'd')
				{
					/*tcp文件下载线程*/
					pthread_t tcp_recv_tid;
					void *result = NULL;
					if (pthread_create(&tcp_recv_tid, NULL, tcp_recv_thread, (void *)&tcp_fd) != 0)
					{
						return 1;
					}
					/*阻塞本线程*/
					pthread_join(tcp_recv_tid, &result);
				}
			}
			close(tcp_fd);
		}
		/************************ end tcp *******************************/

		/************************ udp *******************************/
		if (2 == inputnum)
		{
			/*创建连接*/
			int udp_fd = connect_udp(inet_ntoa(ser_connct.sin_addr), UDPSERVER_PORT);
			if (-1 == udp_fd)
			{
				return -1;
			}
			else
			{
				while (1)
				{
					printf("请输入文件传输类型，输入U/u:上传，D/d:下载\n");
					/*清空标准输入*/
					getchar();
					/*输入指令，上传或下载*/
					scanf("%c", &packet.data_type);
					if (packet.data_type == 'U' | packet.data_type == 'u' | packet.data_type == 'D' | packet.data_type == 'd')
					{
						break;
					}
					else
					{
						printf("输入错误！输入值范围为U/u,D/d\n");
						continue;
					}
				}
				/*发送指令*/
				send(udp_fd, &packet, sizeof(packet), 0);
				if (packet.data_type == 'U' | packet.data_type == 'u')
				{
					/* udp文件发送线程*/
					pthread_t udp_send_tid;
					void *result = NULL;
					if (pthread_create(&udp_send_tid, NULL, udp_send_thread, (void *)&udp_fd) != 0)
					{
						return 1;
					}
					/*阻塞本线程*/
					pthread_join(udp_send_tid, &result);
				}
				if (packet.data_type == 'D' | packet.data_type == 'd')
				{
					/*udp文件下载线程*/
					pthread_t udp_recv_tid;
					void *result = NULL;
					if (pthread_create(&udp_recv_tid, NULL, udp_recv_thread, (void *)&udp_fd) != 0)
					{
						return 1;
					}
					/*阻塞本线程*/
					pthread_join(udp_recv_tid, &result);
				}
			}
			close(udp_fd);
		}
	}
	return 0;
}
