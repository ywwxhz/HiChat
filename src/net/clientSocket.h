/*
 * clientSorket.h
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#ifndef CLIENTSORKET_H_
#define CLIENTSORKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE   2048
#define MAX_FILENAME  255


void socketInit();//初始化套接字和接收线程
void *recvBoardcastProc(void *args);  //接收UDP广播线程
void *recvConProc(void *args);			//接收UDP点对点线程
void msg_pass(char *buf, int nread, struct sockaddr_in addr);//消息传递
void handleUserOnline(char *buf, int nread, struct sockaddr_in addr);//处理用户在线
void handleUserOffline(char *buf);//处理用户离线
void handleMsg(char *buf, int nread);//处理消息
void handleFile(char *buf, int nread);//处理文件
void sendMsg(int socket, char *buf, struct in_addr addr, int port);//发送消息
#endif /* CLIENTSORKET_H_ */
