/**	   @file procotol.h
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 文件传输协议
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

#ifndef _PROTOCOL
#define _PROTOCOL

#define EXPOTOCOL_PORT 8848
#define TCPSERVER_PORT 8888
#define UDPSERVER_PORT 9999

#define BRO_ADDR "255.255.255.255"

#define CMD_HEAD 0x1234

#define MAX_RETRY 3
#define RESEND_TIME_US 500000 //<500ms

/*最大长度,文件路径*/
#define MAX_DATA_SIZE 256
/*读写文件缓冲区长度*/
#define BUFF_SIZE 1024

/*commands between server and client*/
typedef enum
{
	/*magic num*/
	DISCOVERY = 0x1000,
	ACK,
	TCP,
	UDP,
	UPLOAD,
	DOWNLOAD,
} cmd_type;

/*命令交互包格式*/
typedef struct
{
	int head;
	/*DISCOVERY, ACK, TCP, UDP*/
	cmd_type type;
	/*purpose for verification*/
	int pack_size;
	/*the max times of retry is 3*/
	int retry;
	/*服务器服务端口号，server_port[0]=探测服务端口，server_port[1]=TCP服务端口，server_port[2]=UDP服务端口*/
	int server_port[3];
	struct _file_cmd
	{
		/*TCP, UDP*/
		cmd_type protol;
		/*file name*/
		char name[100];
	} file_cmd;
} cmd_pack;

/* 网络协议包 */
typedef struct NET_PACKET
{
	/* 发送方地址 */
	int src_ip;
	int src_port;

	/* 接收方地址 */
	int dst_ip;
	int dst_port;

	/* 数据类型,U---上传；D---下载 */
	char data_type;

} Net_packet;

#endif
