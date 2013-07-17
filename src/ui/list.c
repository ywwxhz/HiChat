/*
 * list.c
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#include "../main.h"
#include "list.h"
extern GData *chatWindows; // 聊天窗口集, 用于获取已经建立的聊天窗口
void setupModel(GtkWidget *treeview, GArray *userList) {
	if (userList != NULL && userList->len > 0) {
		GtkTreeStore *treestore;
		GtkTreeIter toplevelonline, childonline;
		GtkTreeIter topleveloffline, childoffline;
		treestore = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING,
		G_TYPE_STRING);
		guint i;
		gtk_tree_store_append(treestore, &toplevelonline, NULL);
		gtk_tree_store_set(treestore, &toplevelonline, NAME, "在线好友", ID, "", IP,
				"", -1);
		gtk_tree_store_append(treestore, &topleveloffline, NULL);
		gtk_tree_store_set(treestore, &topleveloffline, NAME, "离线好友", ID, "",
				IP, "", -1);
		for (i = 0; i < userList->len; i++) {
			treeItem *pItem = &g_array_index(userList, treeItem, i);
			if (pItem->onlineStat == 1) {
				gtk_tree_store_append(treestore, &childonline, &toplevelonline);
				gtk_tree_store_set(treestore, &childonline, NAME, pItem->name,
						ID, pItem->id, IP, inet_ntoa(pItem->ip_addr), -1);

			} else {
				gtk_tree_store_append(treestore, &childoffline,
						&topleveloffline);
				gtk_tree_store_set(treestore, &childoffline, NAME, pItem->name,
						ID, pItem->id, IP, "", -1);
			}
		}
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),
		GTK_TREE_MODEL (treestore));
		gtk_tree_view_expand_all(GTK_TREE_VIEW (treeview));
		g_object_unref(treestore);
	}
}
void setupView(GtkWidget *treeview) {
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "好友列表");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", NAME);
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "ID");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", ID);
	gtk_tree_view_column_set_visible(col, FALSE);
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "IP");
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", IP);
	g_signal_connect(G_OBJECT (treeview), "row-activated",
			G_CALLBACK (row_activated), NULL);
}
void row_activated(GtkTreeView *treeview, GtkTreePath *path,
		GtkTreeViewColumn *column, gpointer data) {
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, path)) {
		gboolean haschild;
		if (!(haschild = gtk_tree_model_iter_has_child(model, &iter))) {
			gchar *item_text, *item_id;
			gtk_tree_model_get(model, &iter, NAME, &item_text, ID, &item_id,
					-1);
			if (strlen(item_id) > 2) {
//				printf("open chat window\n");
				// 不重复建立聊天对话窗口, 已经建立的通过 g_datalist_get_data 获得窗口
				ChatWindow *chatWindow = (ChatWindow *) g_datalist_get_data(
						&chatWindows, item_id);
				if (chatWindow == NULL) {
//					g_print(
//							"UI::friendList::row_activated:create new chat window\n");
					chatWindow = showChatBox(item_text, item_id);

					g_datalist_set_data(&chatWindows, item_id, chatWindow);
				} else {
//					g_print(
//							"UI::friendList::row_activated:show exist chat window\n");
					gtk_widget_show_all(chatWindow->chatWindow);
				}
			}
		} else {
			if (!gtk_tree_view_collapse_row(GTK_TREE_VIEW (treeview), path)) {
//				printf("expand path\n");
				gtk_tree_view_expand_to_path(GTK_TREE_VIEW (treeview), path);
			}
		}
	}
}
