/*
 * clientSorket.c
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */
#include "../main.h"
#include "clientSocket.h"
//extern GData *chatWindows; // 聊天窗口集, 用于获取已经建立的聊天窗口
extern GArray *userlist; 			// 储存用户信息的全局变量
extern GtkWidget *mainUserlist;		//用户列表
extern gboolean thread_quit;		//线程控制
int boardcastfd;		//广播socket
int sockfd;				//独立socket
typedef struct FileTrans{
	char *filepath;  //文件路径
	int size;        //文件大小
	int userindex;   //用户组位置
	int socketfd;    //文件发送socket描述符
}FileTrans;

void socketInit() {				//初始化套接字和接收线程
	pthread_t recvboardcastthread, recvconthread;
	socklen_t boardlen, conlen;
	struct sockaddr_in boardcast;
	int so_broadcast = 1;
	int so_con = 1;
	struct sockaddr_in conaddr;
	//创建socket
	boardcastfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (boardcastfd < 0) {
		perror("socket error!\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket error!\n");
		exit(1);
	}
	//创建套接口
	boardcast.sin_family = AF_INET;
	boardcast.sin_port = ntohs(BOARDCASTPORT);
	boardcast.sin_addr.s_addr = INADDR_BROADCAST;
	boardlen = sizeof(struct sockaddr);
	setsockopt(boardcastfd, SOL_SOCKET, SO_BROADCAST, &so_broadcast,
			sizeof(so_broadcast));
	conaddr.sin_family = AF_INET;
	conaddr.sin_port = ntohs(CONNPORT);
	conaddr.sin_addr.s_addr = INADDR_ANY;
	conlen = sizeof(struct sockaddr);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_con, sizeof(so_con));
	//绑定套接口
	if (bind(boardcastfd, (struct sockaddr *) &boardcast, boardlen) < 0) {
		perror("bind boardcast error!");
		exit(1);
	}
	if (bind(sockfd, (struct sockaddr *) &conaddr, conlen) < 0) {
		perror("bind connect error!");
		exit(1);
	}
	if (pthread_create(&recvboardcastthread, NULL, recvBoardcastProc, NULL)) {
		printf("create recvboardcast thread fail\n");
	}
	if (pthread_create(&recvconthread, NULL, recvConProc, NULL)) {
		printf("create recvcon thread fail\n");
	}
}
void *recvBoardcastProc(void *args) {				//接收UDP广播线程
	//printf("recvBoardcastProc working\n");
	char buf[BUFFER_SIZE];
	int nread;
	struct sockaddr_in cl_con;
	socklen_t len = sizeof(cl_con);
	while (!thread_quit) {
		nread = recvfrom(boardcastfd, buf, BUFFER_SIZE, 0,
				(struct sockaddr *) &cl_con, &len);
		buf[nread] = '\0';
		if (buf[0] != '0') {
			if (buf[0] != '1') {
				printf("recvBoardcastProc:%s from %s:%d\n", buf,
						inet_ntoa(cl_con.sin_addr), ntohs(cl_con.sin_port));
			}
		}
		msg_pass(buf, nread, cl_con);
	}
	return NULL;
}

void *recvConProc(void *args) {	//接收UDP点对点线程
//	printf("recvConProc working\n");
	char buf[BUFFER_SIZE];
	int nread;
	struct sockaddr_in cl_con;
	socklen_t size = sizeof(cl_con);
	while (!thread_quit) {
		nread = recvfrom(sockfd, buf, BUFFER_SIZE, 0,
				(struct sockaddr *) &cl_con, &size);
		buf[nread] = '\0';
		if (buf[0] != '0') {
			if (buf[0] != '1') {
				printf("recvConProc:%s from %s:%d\n", buf,
						inet_ntoa(cl_con.sin_addr), ntohs(cl_con.sin_port));
			}
		}
		msg_pass(buf, nread, cl_con);
	}
	return NULL;
}
void msg_pass(char *buf, int nread, struct sockaddr_in addr) {	//消息传递
	if (!thread_quit) {
		char msgType = buf[0];
		switch (msgType) {
		case '0':
			printf("msg_pass::user offline \n");
			handleUserOffline(buf);
			break;
		case '1':
			printf("msg_pass::user online or change name\n");
			handleUserOnline(buf, nread, addr);
			break;
		case '2':
			//printf("msg_pass::got message\n");
			handleMsg(buf, nread);
			break;
		case '3':
			//printf("msg_pass::request file trans\n");
			handleFile(buf, nread);
			break;
		default:
			printf(
					"msg_pass::unknow message type:%c\norangal message:%s %d bytes from %s:%d\n",
					msgType, buf, nread, inet_ntoa(addr.sin_addr),
					ntohs(addr.sin_port));
		}
	}
}
void handleUserOnline(char *buf, int nread, struct sockaddr_in addr) {	//处理用户在线
//	printf("handleUserOnline::get user id, name and ip\n");
	char type, userID[10], userName[20];
	memset(userID, '\0', sizeof(userID));
	memset(userName, '\0', sizeof(userName));
	sscanf(buf, "%c:%[^:]:%[^:]", &type, userID, userName);
//	printf("handleUserOnline::id %s name %s ip %s\n", userID, userName,
//			inet_ntoa(addr.sin_addr));
//	printf("handleUserOnline::check if user is exist\n");
//	printf("length of userlist %d\n", userList->len);
	if (userlist->len != 0) {
		int i;
		for (i = 0; i < userlist->len; i++) {
			//printf("loop\n");
			if (!strcmp(g_array_index(userlist, userInfo, i).id, userID)) {
				printf("handleUserOnline::user exist update info\n");
				userInfo uinfo = g_array_index(userlist, userInfo, i);
				strcpy(uinfo.id, userID);
				strcpy(uinfo.name, userName);
				uinfo.ip_addr = addr.sin_addr;
				uinfo.onlineStat = 1;
				g_array_index(userlist, userInfo, i) = uinfo;
				break;
			}
		}
		if (i >= userlist->len) {
			printf("handleUserOnline::user not exist add to userlist\n");
			userInfo s;
			strcpy(s.id, userID);
			strcpy(s.name, userName);
			s.ip_addr = addr.sin_addr;
			s.onlineStat = 1;
			g_array_append_val(userlist, s);
		}
	} else {
		printf("handleUserOnline::user not exist add to userlist\n");
		userInfo s;
		strcpy(s.id, userID);
		strcpy(s.name, userName);
		s.ip_addr = addr.sin_addr;
		s.onlineStat = 1;
		g_array_append_val(userlist, s);
	}
	gdk_threads_enter(); //在需要与图形窗口交互的时候加
	setupModel(mainUserlist, userlist);
	gdk_threads_leave(); //搭配上面的
}

void handleUserOffline(char *buf) { //处理用户离线
//	printf("handleUserOffline::get user id, name and ip\n");
	char type, userID[10], userName[20];
	memset(userID, '\0', sizeof(userID));
	memset(userName, '\0', sizeof(userName));
	sscanf(buf, "%c:%[^:]:%[^:]", &type, userID, userName);
//	printf("handleUserOffline::id %s name %s\n", userID, userName);
//	printf("handleUserOffline::check if user is exist\n");
//	printf("length of userlist %d\n", userList->len);
	if (userlist->len != 0) {
		int i;
		for (i = 0; i < userlist->len; i++) {
//			printf("loop\n");
			if (strcmp(g_array_index(userlist, userInfo, i).id, userID) == 0) {
//				printf("handleUserOffline::user exist update info\n");
				userInfo uinfo = g_array_index(userlist, userInfo, i);
				strcpy(uinfo.id, userID);
				strcpy(uinfo.name, userName);
				uinfo.ip_addr.s_addr = INADDR_ANY;
				uinfo.onlineStat = 0;
				g_array_index(userlist, userInfo, i) = uinfo;
				break;
			}
		}
		if (i > userlist->len) {
//			printf("handleUserOffline::user not exist add to userlist\n");
			userInfo s;
			strcpy(s.id, userID);
			strcpy(s.name, userName);
			s.ip_addr.s_addr = INADDR_ANY;
			s.onlineStat = 0;
			g_array_append_val(userlist, s);
		}
	} else {
//		printf("handleUserOffline::user not exist add to userlist\n");
		userInfo s;
		strcpy(s.id, userID);
		strcpy(s.name, userName);
		s.ip_addr.s_addr = INADDR_ANY;
		s.onlineStat = 0;
		g_array_append_val(userlist, s);
	}
	gdk_threads_enter(); //在需要与图形窗口交互的时候加
	setupModel(mainUserlist, userlist);
	gdk_threads_leave(); //搭配上面的
}

void handleMsg(char *buf, int nread) { //处理消息
//	printf("handleMsg::get user id,name and msg\n");
	char type, userID[10], userName[20], msg[BUFFER_SIZE];
	memset(userID, '\0', sizeof(userID));
	memset(msg, '\0', sizeof(msg));
	memset(userName, '\0', sizeof(userName));
	sscanf(buf, "%c:%[^:]:%[^:]:%[^:]", &type, userID, userName, msg);
	printf("handleMsg::id %s name %s msg %s \n", userID, userName, msg);
	gdk_threads_enter(); //在需要与图形窗口交互的时候加
	// 开始接收消息
	receiveTextMsg(userName, userID, msg);
	gdk_threads_leave(); //搭配上面的
}
void sendMsg(int socket, char *buf, struct in_addr addr, int port) { //发送消息
	if (strcmp(inet_ntoa(addr), "0.0.0.0")) {
		struct sockaddr_in conaddr;
		conaddr.sin_family = AF_INET;
		conaddr.sin_port = ntohs(port);
		conaddr.sin_addr = addr;
		printf("sendMsg::socket %d  %s:%d buff %s\n", socket, inet_ntoa(addr),
				port, buf);
		socklen_t boardlen = sizeof(struct sockaddr);
		sendto(socket, buf, strlen(buf), 0, (struct sockaddr *) &conaddr,
				boardlen);
	}
}

void handleFile(char *buf, int nread) { //处理文件
	printf("handleFile::get user id, fileName,size,stat\n");
	char type, userID[10], fileName[MAX_FILENAME], size[10], stat[5];
	memset(userID, '\0', sizeof(userID));
	memset(fileName, '\0', sizeof(fileName));
	memset(size, '\0', sizeof(size));
	memset(stat, '\0', sizeof(stat));
	sscanf(buf, "%c:%[^:]:%[^:]:%[^:]:%s", &type, userID, fileName, size, stat);
	printf("handleFile::id %s filename %s size %s stat %s \n", userID, fileName,
			size, stat);
	int i;
	for (i = 0; i < userlist->len; i++) {
		if (strcmp(g_array_index(userlist, userInfo, i).id, userID) == 0)
			break;
	}

	userInfo uinfo;
	if (i < userlist->len)
		uinfo = g_array_index(userlist, userInfo, i);

	g_strdup_printf("用户名: \n%s\nIP地址:\n%s", uinfo.name,
			inet_ntoa(uinfo.ip_addr));
	printf("handleFile::check if chat window is opened\n");
	printf("handleFile::chat window opened add msg\n");
	printf("handleFile::chat window not opened open window\n");
}
