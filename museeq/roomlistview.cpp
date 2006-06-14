/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
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

#include "roomlistview.h"

#include <qpopupmenu.h>
#include "roomlistitem.h"
#include "museeq.h"

RoomListView::RoomListView(QWidget* _p, const char* _n)
             : QListView(_p, _n) {

	addColumn(tr("Room"));
	addColumn(tr("Users"));

	setColumnAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
	setSorting(1);
	setShowSortIndicator(true);
	setAllColumnsShowFocus(true);
	
	mPopup = new QPopupMenu(this);
	mPopup->insertItem(tr("Join room"), this, SLOT(slotJoin()), 0, 0);
	mPopup->insertItem(tr("Leave room"), this, SLOT(slotLeave()), 0, 1);
	mPopup->insertSeparator();
	mPopup->insertItem(tr("Refresh"), this, SLOT(slotRefresh()), 0, 2);
	
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(slotPopupMenu(QListViewItem*, const QPoint&, int)));

	connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)));

	connect(this, SIGNAL(returnPressed(QListViewItem*)), SLOT(slotReturnPressed(QListViewItem*)));
	
	connect(museeq, SIGNAL(roomList(const NRoomList&)), SLOT(setRooms(const NRoomList&)));
	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}

void RoomListView::setRooms(const NRoomList& _r) {
	clear();
	
	QMap<QString, unsigned int>::const_iterator it = _r.begin();
	for(; it != _r.end(); ++it)
		new RoomListItem(this, it.key(), it.data());
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

void RoomListView::slotPopupMenu(QListViewItem* item, const QPoint& pos, int) {
	RoomListItem* _item = static_cast<RoomListItem*>(item);
	if(item) {
		mPopped = _item->room();
		mPopup->setItemEnabled(0, ! museeq->isJoined(mPopped));
		mPopup->setItemEnabled(1, museeq->isJoined(mPopped));
		mPopup->setItemEnabled(2, museeq->isConnected());
	} else {
		mPopped = QString::null;
		mPopup->setItemEnabled(0, false);
		mPopup->setItemEnabled(1, false);
		mPopup->setItemEnabled(2, museeq->isConnected());
	}
	mPopup->exec(pos);
}

void RoomListView::slotDoubleClicked(QListViewItem* item, const QPoint&, int) {
	slotReturnPressed(item);
}

void RoomListView::slotReturnPressed(QListViewItem* item) {
	RoomListItem* _item = static_cast<RoomListItem*>(item);
	if(item)
		museeq->joinRoom(_item->room());
}
