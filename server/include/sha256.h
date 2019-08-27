/**	   @file sha256.h
 *	   @note Hangzhou Hikvision Digital Technology Co., Ltd. All Right Reserved.
 *	   @brief 服务器数据校验算法
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

#ifndef SHA256_H
#define SHA256_H

/* 警告输入长度不能超过这个长度*/
#define maxlen 1000000

typedef unsigned int uint;
typedef unsigned char ubyte;
typedef unsigned long long ull;

inline uint rrot(uint val, int pos);

void sha_256(char *digest, char *str);

#endif
