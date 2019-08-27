/**	   @file startsocket.h
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端tcp/udp开启连接socket服务的函数接口
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

#ifndef STARTSOCKET_H
#define STARTSOCKET_H

int connectsock(char *, int, int);
int connect_tcp(char *, int);
int connect_udp(char *, int);

#endif
