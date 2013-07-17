/*
 * mainWindow.h
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_


void mainWindowInit(int argc,char *argv[]);//调用主窗体初始化
void createMainWindow();   //创建主窗体
void sendOnline();         //发送在线信息
gboolean onlineTimeout(void *arg); //在线时间定时器
void HiChatQuit();       //主程序退出

#endif /* MAINWINDOW_H_ */
