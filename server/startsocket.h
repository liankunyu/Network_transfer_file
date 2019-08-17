/**	   @file startsocket.h
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器创建tcp/udp的socket函数接口
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

#ifndef STARTSOCKET_H
#define STARTSOCKET_H

extern int create_tcp_server(int);
extern int create_udp_server(int);

#endif
