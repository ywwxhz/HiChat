/*
 * mainWindow.c
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#include "../main.h"
#include "mainWindow.h"

//全局变量声明
GArray *userlist = NULL; // 储存用户信息的全局变量s
char myName[20];     			//用户名
char myID[10];  				//用户ID
extern int boardcastfd;        //广播socket描述符
GtkWidget *mainUserlist;		//用户列表
int utimer_online, utimer_count = 0;
gboolean thread_quit = FALSE;	//线程控制

void *changeUserName(GtkWidget *button, GtkWidget *entry);	//点击用户名响应函数
void *usernameEntryFinish(GtkWidget *entry, GtkWidget *button);	//用户名编辑完毕处理函数

void sendOnline() {	//发送在线信息
	printf("sendOnline::send online stat\n");
	struct in_addr boardcast;
	boardcast.s_addr = INADDR_BROADCAST;
	char sendbuf[100];
	bzero(sendbuf, sizeof(sendbuf));
	sprintf(sendbuf, "%s:%s:%s", PROTOCOL_ONLINE, myID, myName);
	sendMsg(boardcastfd, sendbuf, boardcast, BOARDCASTPORT);
}
void sendOffline() {	//发送离线消息
	printf("sendOffline::send offline stat\n");
	struct in_addr boardcast;
	boardcast.s_addr = INADDR_BROADCAST;
	char sendbuf[100];
	bzero(sendbuf, sizeof(sendbuf));
	sprintf(sendbuf, "%s:%s:%s", PROTOCOL_OFFLINE, myID, myName);
	sendMsg(boardcastfd, sendbuf, boardcast, BOARDCASTPORT);
}
void *changeUserName(GtkWidget *button, GtkWidget *entry) {	//点击用户名响应函数
	printf("change username start \n");
	gtk_widget_set_visible(button, FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry),
			gtk_button_get_label(GTK_BUTTON(button)));
	gtk_widget_set_visible(entry, TRUE);
	return NULL;
}
void *usernameEntryFinish(GtkWidget *entry, GtkWidget *button) {//用户名编辑完毕处理函数
	printf("change username finish \n");
	gtk_widget_set_visible(button, TRUE);
	gtk_button_set_label(GTK_BUTTON(button),
			gtk_entry_get_text(GTK_ENTRY(entry)));
	strcpy(myName, gtk_entry_get_text(GTK_ENTRY(entry)));
	gtk_widget_set_visible(entry, FALSE);
	char buf[30];
	memset(buf,'\0', sizeof(buf));
	sprintf(buf,"%s:%s",myID,myName);
	printf("pre write buf :%s\n", buf);
	int fp = open("username.conf", O_WRONLY);
	write(fp, buf, sizeof(buf));
	close(fp);
	sendOnline();
	return NULL;
}
void mainWindowInit(int argc, char *argv[]) {	//调用主窗体初始化
	gtk_init(&argc, &argv);
	userlist = g_array_new(FALSE, TRUE, sizeof(userInfo));
	//初始化套接字
	socketInit();
	//初始化用户信息
	if (access("username.conf", 0) == -1) {
		char buf[30];
		memset(buf,'\0', sizeof(buf));
		srand((int) time(0));
		sprintf(myID, "%d", random(100000000));
		printf("random id:%s\n", myID);
		sprintf(buf,"%s:Anonymous",myID);
		printf(
				"createMainWindows::can't find username.conf use default setting\n");
		printf("create username.conf\n");
		int fp = creat("username.conf", 0644);
		write(fp, buf, sizeof(buf));
		strcpy(myName, "Anonymous");
		if (access(myID, 0) == -1) // 创建一个以自己 id 为名的文件夹用于保存聊天记录
				{
			if (mkdir(myID, 0777)) //如果不存在就用mkdir函数来创建
					{
				perror("creat folder failed");
			}
		}
	} else {
		char buf[30];
		printf("read username from file\n");
		int fp = open("username.conf", O_RDONLY);
		read(fp, buf,sizeof(buf));
		close(fp);
		sscanf(buf,"%[^:]:%s",myID, myName);
		printf("read from file: username %s id %s\n", myName, myID);
	}
	//创建主窗体
	createMainWindow();
	//发送在线信息
	sendOnline();
	//初始化gtk多线程
	if (!g_thread_supported()) {
		g_thread_init(NULL);
	}

	gdk_threads_init();
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
}
void createMainWindow() { //创建主窗体
	GtkBuilder *mainbuilder;
	GtkWidget *mainWindow;
	GtkWidget *mainUsername;
	GtkWidget *mainUserNameEntry;
	mainbuilder = gtk_builder_new();
	gtk_builder_add_from_file(mainbuilder, "ui.xml", NULL);
	mainWindow = GTK_WIDGET(gtk_builder_get_object(mainbuilder,"mainWindow"));
	mainUsername = GTK_WIDGET(gtk_builder_get_object(mainbuilder,"userName"));
	gtk_button_set_label(GTK_BUTTON(mainUsername), myName);
	mainUserlist = GTK_WIDGET(gtk_builder_get_object(mainbuilder,"userList"));
	mainUserNameEntry =
	GTK_WIDGET(gtk_builder_get_object(mainbuilder,"userNameEntry"));
	g_signal_connect(G_OBJECT (mainUsername), "clicked",
			G_CALLBACK (changeUserName), mainUserNameEntry);
	g_signal_connect(G_OBJECT (mainUserNameEntry), "activate",
			G_CALLBACK (usernameEntryFinish), mainUsername);
	g_signal_connect_swapped(G_OBJECT (mainWindow), "destroy",
			G_CALLBACK (HiChatQuit), NULL);
	setupView(mainUserlist);
	setupModel(mainUserlist, userlist);
	gtk_builder_connect_signals(mainbuilder, NULL);
	g_object_unref(G_OBJECT(mainbuilder));
	gtk_widget_show_all(mainWindow);
	//添加定时器
	utimer_online = g_timeout_add(3000, (GSourceFunc) onlineTimeout, NULL);
	//设定系统信号
	signal(SIGINT, HiChatQuit);
	signal(SIGTERM, HiChatQuit);
}
gboolean onlineTimeout(void *arg) { //在线时间定时器
//	printf("loginTimeout::utimer_count:%d\n", utimer_count++);
	sendOnline();
	return TRUE;
}
void HiChatQuit() { //主程序退出
	g_source_remove(utimer_online);
	thread_quit = TRUE;
	sendOffline();
	gtk_main_quit();
}
