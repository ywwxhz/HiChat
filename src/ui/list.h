/*
 * list.h
 *
 *  Created on: 2013年7月13日
 *      Author: ywwxhz
 */

#ifndef LIST_H_
#define LIST_H_
enum {
	NAME = 0,IP,ID, NUM_COLS
};
typedef struct {
	char id[20];
	char name[20]; // user nickname
	struct in_addr ip_addr; // user ip address at in_addr format
	char onlineStat; // user is online
} userInfo, treeItem;
void setupModel(GtkWidget *treeview,GArray *userList);
void setupView(GtkWidget *treeview);
void row_activated(GtkTreeView *treeview, GtkTreePath *path,
		GtkTreeViewColumn *column, gpointer data);
#endif /* LIST_H_ */
