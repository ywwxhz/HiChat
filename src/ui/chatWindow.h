/*
 * chatWindow.h
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#ifndef CHATWINDOW_H_
#define CHATWINDOW_H_

typedef struct {
	GtkWidget *chatWindow;
	GtkWidget *textview_output, *textview_intput;
	GtkProgressBar *progress;
	GtkWidget *label_file;
	GtkWidget *FileFrame;
} ChatWindow; // 聊天对话框数据结构

ChatWindow *showChatBox(gchar *name,gchar *id);   //创建聊天窗体
char *getInfoFromID(char *id);                   //获取用户信息
void fontSelect(GtkWidget *widget, gpointer data);//字体设置函数
void showHistoryMsg(GtkWidget *widget, gchar *id);//显示历史消息函数
void sendFile(GtkButton *button, gpointer data);//发送文件
char *getCurrentTime();//获取当前时间
void sendTextMsg(GtkWidget *widget, gchar *id);//发送文本消息
void receiveTextMsg(char *text_title, char *id_from, char *text_msg);//接收文本消息
void scrollTextview(char *text_title);//滚动消息框
void closeDialog(GtkWidget *widget, gpointer data);//关闭聊天窗体

#endif /* CHATWINDOW_H_ */
