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

#include "museeq.h"
#include "roomlistview.h"
#include "roomlistitem.h"

#include <QMenu>

RoomListView::RoomListView(QWidget* _p, const char* _n)
             : QTreeWidget(_p) {
	setColumnCount(2);

	QStringList headers;
	headers << tr("Room") << tr("Users");
	setHeaderLabels(headers);
	setSortingEnabled(true);
	setRootIsDecorated(false);
 	setAllColumnsShowFocus(true);


	mPopup = new QMenu(this);


	ActionJoin = new QAction(tr("Join room"), this);
	connect(ActionJoin, SIGNAL(triggered()), this, SLOT(slotJoin()));
	mPopup->addAction(ActionJoin);

	ActionLeave = new QAction(tr("Leave room"), this);
	connect(ActionLeave, SIGNAL(triggered()), this, SLOT(slotLeave()));
	mPopup->addAction(ActionLeave);

	mPopup->addSeparator();

	ActionRefresh = new QAction(tr("Refresh"), this);
	connect(ActionRefresh, SIGNAL(triggered()), this, SLOT(slotRefresh()));
	mPopup->addAction(ActionRefresh);


	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(museeq, SIGNAL(roomList(const NRoomList&)), SLOT(setRooms(const NRoomList&)));
	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}

void RoomListView::setRooms(const NRoomList& _r) {
	clear();

	QMap<QString, unsigned int>::const_iterator it = _r.begin();
	for(; it != _r.end(); ++it)
		new RoomListItem(this, it.key(), it.value());
	resizeColumnToContents(0);
	sortItems(0, Qt::AscendingOrder);
}

void RoomListView::slotJoin() {
	museeq->joinRoom(mPopped);
}

void RoomListView::slotLeave() {
	museeq->leaveRoom(mPopped);
}

void RoomListView::slotRefresh() {
	museeq->updateRoomList();
}
void RoomListView::slotActivate(QTreeWidgetItem* item, int column) {

	RoomListItem* _item = static_cast<RoomListItem*>(item);
	if(item)
		museeq->joinRoom(_item->room());
}

void RoomListView::slotContextMenu(const QPoint& pos) {
	RoomListItem* item = static_cast<RoomListItem*>(itemAt(pos));

	if (! item ) {

		mPopped = QString::null;
		ActionJoin->setEnabled(false);
		ActionLeave->setEnabled(false);
		ActionRefresh->setEnabled(museeq->isConnected());

	} else {
		mPopped = item->room();
		ActionJoin->setEnabled(! museeq->isJoined(mPopped));
		ActionLeave->setEnabled(museeq->isJoined(mPopped));
		ActionRefresh->setEnabled(museeq->isConnected());
	}

	mPopup->exec(mapToGlobal(pos));
}

