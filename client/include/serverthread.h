/**	   @file serverthread.h
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 客户端tcp/udp发送接收线程的接口，主函数调用用于发送客户端信息
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

#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

/* tcp文件发送线程 */
extern void *tcp_send_thread(void *tcp_fd);

/* tcp文件接收线程 */
extern void *tcp_recv_thread(void *tcp_fd);

/* udp文件发送线程 */
extern void *udp_send_thread(void *tcp_fd);

/* udp文件接收线程 */
extern void *udp_recv_thread(void *tcp_fd);

#endif
