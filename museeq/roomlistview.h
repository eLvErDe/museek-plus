/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ROOMLISTVIEW_H
#define ROOMLISTVIEW_H

#include "museeqtypes.h"

#include <QTreeWidget>

class QMenu;

class RoomListView : public QTreeWidget {
	Q_OBJECT
public:
	RoomListView(QWidget* = 0, const char* = 0);

signals:
	void join(const QString&);
	void leave(const QString&);
	void refresh();

protected slots:
	void setRooms(const NRoomList&);
	void setPrivRooms(const NPrivRoomList&);
	void setRoomsFromCache();
	void slotJoin();
	void slotLeave();
	void slotDisown();
	void slotDismember();
	void slotRefresh();
	void slotActivate(QTreeWidgetItem*,  int);
	void slotContextMenu(const QPoint&);
	void adaptColumnSize(int);
	void onClosingMuseeq();

protected:
	QAction * ActionJoin, * ActionLeave, * ActionRefresh, * mActionDisown, * mActionDismember;
	QMenu *mPopup;
	QString mPopped;

	NRoomList m_roomListCache;
};

#endif // ROOMLISTVIEW_H
