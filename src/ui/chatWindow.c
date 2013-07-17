/*
 * chatWindow.c
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#include "../main.h"
#include "chatWindow.h"
#include <pango/pango.h>

#define MAX_BUF				1024 * 2
GData *chatWindows = NULL; // 聊天窗口集, 用于获取已经建立的聊天窗口
extern GArray *userlist; // 储存用户信息的全局变量
extern int sockfd; //独立socket

extern char myName[20]; //  name
extern char myID[10];  //  id

ChatWindow *showChatBox(gchar *name, gchar *id) {
	ChatWindow *chatWindowHandle = g_slice_new(ChatWindow);
	GtkBuilder *chat_builder;
	GtkWidget *chatWindow;
	//builder初始化
	chat_builder = gtk_builder_new();
	gtk_builder_add_from_file(chat_builder, "ui.xml", NULL);
	gchar *title = g_strdup_printf("%s", name);
	chatWindow = GTK_WIDGET(gtk_builder_get_object(chat_builder, "chatWindow"));
	g_signal_connect(G_OBJECT(chatWindow), "destroy", G_CALLBACK(closeDialog),
			id);
	gtk_window_set_title(GTK_WINDOW(chatWindow), title);
	gtk_container_set_border_width(GTK_CONTAINER(chatWindow), 0);

	// 输出 text view
	GtkWidget *textview_output =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "chatOutput"));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_output),
			GTK_WRAP_WORD_CHAR);
	// 设置消息输出区域 text view 垂直滚动消息
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_output));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_create_mark(buffer, "scroll", &iter, TRUE);

	// 输入 text view
	GtkWidget *textview_input =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "chartInput"));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_input),
			GTK_WRAP_WORD_CHAR);

	// 发送和关闭按钮
	GtkWidget *chatClose =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "cancelMsg"));
	g_signal_connect(G_OBJECT(chatClose), "clicked", G_CALLBACK( closeDialog),
			id);
	GtkWidget *chatSend =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "sendMsg"));
	g_signal_connect(G_OBJECT(chatSend), "clicked", G_CALLBACK( sendTextMsg),
			id);

	GtkWidget *chatRight;
	GtkWidget *progress, *label_file, *fileFrame;
	progress = NULL; // 注意初始化, 本身群聊窗口是没有进度条的
	// 右侧控件布局
	chatRight =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "chatRight"));
	GtkWidget *decInfo =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "decInfo"));
	gtk_label_set_text(GTK_LABEL(decInfo), getInfoFromID(id));
	gtk_label_set_justify(GTK_LABEL (decInfo), GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap(GTK_LABEL (decInfo), TRUE);
	//文件传输框架
	fileFrame =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "Fileframe"));
	// 文件传送进度条和文件名

	progress =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "chatProgress")); // 右侧窗口进度条

	// 发送文件路径
	label_file =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "chatFilename"));

	// 左侧窗口中间的工具栏
	GtkWidget *selectFont;
	GtkWidget *histroyMsg;
	GtkWidget *SendFile;
	// 字体
	selectFont = GTK_WIDGET(gtk_builder_get_object(chat_builder, "selectFont"));
	g_signal_connect(selectFont, "clicked", G_CALLBACK(fontSelect),
			textview_input);
	// 聊天记录
	histroyMsg = GTK_WIDGET(gtk_builder_get_object(chat_builder, "histroyMsg"));
	g_signal_connect(histroyMsg, "clicked", G_CALLBACK(showHistoryMsg), id);
	// 发送文件
	SendFile =
	GTK_WIDGET(gtk_builder_get_object(chat_builder, "SendFile"));
	g_signal_connect(SendFile, "clicked", G_CALLBACK(sendFile), id);
	gtk_widget_set_visible(SendFile, TRUE);

	// 焦点置于输入 text view
	gtk_widget_grab_focus(textview_input);

	// send 按钮响应回车消息
	GtkAccelGroup *agChat;
	agChat = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(chatWindow), agChat);
	gtk_widget_add_accelerator(chatSend, "clicked", agChat, GDK_KEY_Return, 0,
			GTK_ACCEL_VISIBLE);

	gtk_builder_connect_signals(chat_builder, NULL);
	g_object_unref(G_OBJECT(chat_builder));
	gtk_widget_show_all(chatWindow);

	chatWindowHandle->chatWindow = chatWindow;
	chatWindowHandle->textview_intput = textview_input;
	chatWindowHandle->textview_output = textview_output;
	chatWindowHandle->progress = GTK_PROGRESS_BAR(progress);
	chatWindowHandle->label_file = label_file;
	chatWindowHandle->FileFrame = fileFrame;
	gtk_widget_set_visible(chatWindowHandle->FileFrame, FALSE);

	return chatWindowHandle;
}

char *getInfoFromID(char *id) {
	int i;
	for (i = 0; i < userlist->len; i++) {
		if (strcmp(g_array_index(userlist, userInfo, i).id, id) == 0)
			break;
	}

	userInfo uinfo;
	if (i < userlist->len)
		uinfo = g_array_index(userlist, userInfo, i);

	return g_strdup_printf("用户名: \n%s\nIP地址:\n%s", uinfo.name,
			inet_ntoa(uinfo.ip_addr));
}
void sendTextMsg(GtkWidget *widget, gchar *id) {
	// 获得相关的聊天窗口集所关联的数据, 获得当前聊天对话框的 textview_intput
	ChatWindow *chatWindowHandle = (ChatWindow *) g_datalist_get_data(
			&chatWindows, id);

	GtkTextIter start_in, end_in, start_out, end_out;
	gchar *text_msg = malloc(MAX_BUF);

	GtkTextBuffer *buff_input, *buff_output;
	buff_input = gtk_text_view_get_buffer(
	GTK_TEXT_VIEW(chatWindowHandle->textview_intput));
	buff_output = gtk_text_view_get_buffer(
	GTK_TEXT_VIEW(chatWindowHandle->textview_output));

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_input), &start_in, &end_in);
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_output), &start_out,
			&end_out);

	text_msg = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buff_input), &start_in,
			&end_in, FALSE);

	char *msg_title;
	if (text_msg == NULL || strlen(text_msg) == 0) {
		printf("没有消息\n");
		return;
	} else {
		char buf[BUFFER_SIZE];
		sprintf(buf, "%s:%s:%s:%s", PROTOCOL_MESSAGE, myID,myName, text_msg);
		char *text_temp = getInfoFromID(id),**split_text;
		split_text = g_strsplit(text_temp, "\n", 4);
		struct in_addr con_addr;
		con_addr.s_addr = inet_addr(split_text[3]);
		sendMsg(sockfd, buf,con_addr, CONNPORT);

		// 清除输入 text view 数据
		gtk_text_buffer_delete(GTK_TEXT_BUFFER(buff_input), &start_in, &end_in);

		char *time_str = getCurrentTime();
		msg_title = g_strdup_printf("%s  %s%s", myName, time_str, "\n");

		// 插入文本到消息接收 text view
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out,
				msg_title, strlen(msg_title));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, text_msg,
				strlen(text_msg));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, "\n",
				strlen("\n"));

		scrollTextview((char *) id);

		// 写入聊天记录文件
		char *history_file = g_strdup_printf("%s/%s.txt", myID, id);
		FILE *fp = fopen(history_file, "ab");
		fseek(fp, 0L, SEEK_END);
		fprintf(fp, "%s", g_locale_from_utf8(msg_title, strlen(msg_title),
		NULL, NULL, NULL));
		fprintf(fp, "%s\n\n", g_locale_from_utf8(text_msg, strlen(text_msg),
		NULL, NULL, NULL));
		fclose(fp);
	}
}

void fontSelect(GtkWidget *widget, gpointer data) {
	GtkWidget *textview_input = (GtkWidget *) data;

	GtkWidget *fontdlg = gtk_font_chooser_dialog_new("Select Font", NULL);
	GtkResponseType ret = gtk_dialog_run(GTK_DIALOG(fontdlg));

	// 设置为模态对话框
	gtk_window_set_modal(GTK_WINDOW(fontdlg), TRUE);

	if (ret == GTK_RESPONSE_OK || ret == GTK_RESPONSE_APPLY) {
		gtk_widget_modify_font(GTK_WIDGET(textview_input),
				gtk_font_chooser_get_font_desc((GtkFontChooser *) fontdlg));
	}
	gtk_widget_destroy(fontdlg);
}
void closeHis(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(window);
}
void showHistoryMsg(GtkWidget *widget, gchar *id) {
	ChatWindow *chatWindow = (ChatWindow *) g_datalist_get_data(&chatWindows,
			id);
	GtkBuilder *histroy_builder;
	GtkWidget *histroyWindow;
	GtkWidget *textViewHistory, *histroyClose;
	gchar *title;
	GtkTextBuffer *buffer;
	gchar *content, *file;
	//builder初始化
	histroy_builder = gtk_builder_new();
	gtk_builder_add_from_file(histroy_builder, "ui.xml", NULL);
	histroyWindow =
	GTK_WIDGET(gtk_builder_get_object(histroy_builder, "HistroyWindow"));
	histroyClose =
	GTK_WIDGET(gtk_builder_get_object(histroy_builder, "histroyClose"));

	title = g_strdup_printf("与 %s 的聊天记录",
			gtk_window_get_title(GTK_WINDOW(chatWindow->chatWindow)));
	gtk_window_set_title(GTK_WINDOW (histroyWindow), title);

	textViewHistory =
	GTK_WIDGET(gtk_builder_get_object(histroy_builder, "textViewHistory"));
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textViewHistory),
			GTK_WRAP_WORD_CHAR);
	file = g_strdup_printf("%s/%s.txt", myID, id);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (textViewHistory));
	if (!g_file_get_contents(file, &content, NULL, NULL)) {
		content = malloc(sizeof("消息记录为空"));
		strcpy(content, "消息记录为空");
	}
	gtk_text_buffer_set_text(buffer, content, -1);
	g_free(content);
	g_signal_connect(G_OBJECT(histroyClose), "clicked", G_CALLBACK(closeHis),
			histroyWindow);
	gtk_builder_connect_signals(histroy_builder, NULL);
	g_object_unref(G_OBJECT(histroy_builder));
	gtk_widget_show_all(histroyWindow);
}
char *getCurrentTime() {
	time_t timep;
	struct tm *time_cur;
	char *time_str;
	time(&timep);
	time_cur = localtime(&timep);

	time_str = g_strdup_printf("%02d:%02d:%02d", time_cur->tm_hour,
			time_cur->tm_min, time_cur->tm_sec);

	return time_str;
}
void receiveTextMsg(char *text_title, char *id_from, char *text_msg){
	// 不重复建立聊天对话窗口, 已经建立的通过 g_datalist_get_data 获得窗口
		ChatWindow *chatWindowHandle = (ChatWindow *) g_datalist_get_data(&chatWindows,
				id_from);
		if (chatWindowHandle == NULL) {
			chatWindowHandle = showChatBox(text_title,id_from);
			g_datalist_set_data(&chatWindows, id_from, chatWindowHandle);
		} else
			gtk_widget_show(chatWindowHandle->chatWindow);

		// 显示消息
		GtkTextIter start_out, end_out;

		GtkTextBuffer *buff_output;
		buff_output = gtk_text_view_get_buffer(
		GTK_TEXT_VIEW(chatWindowHandle->textview_output));

		gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_output), &start_out,
				&end_out);

		char *time_str = getCurrentTime();
		char *msg_title = g_strdup_printf("%s  %s%s", text_title, time_str, "\n");

		// 插入文本到消息接收 text view
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, msg_title,
				strlen(msg_title));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, text_msg,
				strlen(text_msg));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, "\n",
				strlen("\n"));

		scrollTextview(id_from);

		// 写入聊天记录文件
		char *history_file = g_strdup_printf("%s/%s.txt", myID, id_from);
		FILE *fp = fopen(history_file, "ab");
		fseek(fp, 0L, SEEK_END);
		fprintf(fp, "%s", g_locale_from_utf8(msg_title, strlen(msg_title), NULL,
		NULL, NULL));
		fprintf(fp, "%s\n\n", g_locale_from_utf8(text_msg, strlen(text_msg), NULL, NULL,
		NULL));
		fclose(fp);
}
void scrollTextview(char *text_title) {
	ChatWindow *chatWindowHandle = (ChatWindow *) g_datalist_get_data(
			&chatWindows, text_title);
	GtkWidget *textview = chatWindowHandle->textview_output;

	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextMark *mark;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_iter_set_line_offset(&iter, 0);
	mark = gtk_text_buffer_get_mark(buffer, "scroll");
	gtk_text_buffer_move_mark(buffer, mark, &iter);

	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), mark);
}
void closeDialog(GtkWidget *widget, gpointer data) {
	//printf("close window\n");
	ChatWindow *chatWindow = (ChatWindow *) g_datalist_get_data(&chatWindows,
			data);
	if (chatWindow != NULL) {
		// 设置聊天窗口集所关联的数据, 释放窗口数据, 下次再双击 tree item 的时候可以再创建聊天窗口
		g_datalist_set_data(&chatWindows, data, NULL);
		gtk_widget_destroy(chatWindow->chatWindow);
	}
}
void sendFile(GtkButton *button, gpointer id) {
	ChatWindow *chatWindowHandle = (ChatWindow *) g_datalist_get_data(&chatWindows,
			id);

	GtkWidget *dialog;
	GSList *filenames;
	gchar *str_file;

	dialog = gtk_file_chooser_dialog_new("打开文件 ...", NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog),
			g_get_home_dir());

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result == GTK_RESPONSE_ACCEPT) {
		filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
		gchar *file = (gchar*) filenames->data;
		printf("filename :%s\n",file);
		str_file = g_strdup(file);

		gtk_label_set_text(GTK_LABEL(chatWindowHandle->label_file), str_file);
		gtk_widget_show(GTK_WIDGET(chatWindowHandle->FileFrame));

		// 发送文件时, 先发送询问消息, 对方是否接收, 若对方接收, 则建立 TCP Socket 进行真正的传输
		// 格式为 PROTOCOL_RECV_FILE + 对方 id + 文件名(包含路径) + "-" + 文件大小
		struct stat stat_buf;
		stat(str_file, &stat_buf); // 获取文件大小
		char *size_buf = g_strdup_printf("-%ld", stat_buf.st_size);


	}
	gtk_widget_destroy(dialog);
}
