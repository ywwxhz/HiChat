/*
 * main.h
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#ifndef MAIN_H_
#define MAIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <limits.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "ui/chatWindow.h"
#include "ui/list.h"
#include "ui/mainWindow.h"
#include "net/clientSocket.h"

//定义数据包头
#define PROTOCOL_OFFLINE          "0"
#define PROTOCOL_ONLINE           "1"
#define PROTOCOL_MESSAGE          "2"
#define PROTOCOL_FILEREQUEST      "3"
#define PROTOCOL_FILERACCEPT      "4"
#define PROTOCOL_FILERREADY       "5"

//定义端口
#define BOARDCASTPORT 7000
#define CONNPORT      7010
#define FILEPORT      7020

//生成随机ID
#define random(x) (rand()%x)

#endif /* MAIN_H_ */
