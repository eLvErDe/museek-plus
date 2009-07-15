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
#include "mainwin.h"
#include "roomlistview.h"
#include "roomlistitem.h"

#include <QMenu>
#include <QHeaderView>
#include <QSettings>

RoomListView::RoomListView(QWidget* _p, const char* _n)
             : QTreeWidget(_p) {
	setColumnCount(3);

	QStringList headers;
	headers << tr("Room") << tr("Users") << tr("Status");
	setHeaderLabels(headers);
	setSortingEnabled(true);
	setRootIsDecorated(false);
 	setAllColumnsShowFocus(true);


    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "roomList_Layout";
        header()->restoreState(museeq->settings()->value(optionName).toByteArray());
    }

	mPopup = new QMenu(this);


	ActionJoin = new QAction(tr("Join room"), this);
	connect(ActionJoin, SIGNAL(triggered()), this, SLOT(slotJoin()));
	mPopup->addAction(ActionJoin);

	ActionLeave = new QAction(tr("Leave room"), this);
	connect(ActionLeave, SIGNAL(triggered()), this, SLOT(slotLeave()));
	mPopup->addAction(ActionLeave);

	mPopup->addSeparator();

	mActionDisown = new QAction(tr("Give up ownership"), this);
	connect(mActionDisown, SIGNAL(triggered()), this, SLOT(slotDisown()));
	mPopup->addAction(mActionDisown);

	mActionDismember = new QAction(tr("Give up membership"), this);
	connect(mActionDismember, SIGNAL(triggered()), this, SLOT(slotDismember()));
	mPopup->addAction(mActionDismember);

	mPopup->addSeparator();

	ActionRefresh = new QAction(tr("Refresh"), this);
	connect(ActionRefresh, SIGNAL(triggered()), this, SLOT(slotRefresh()));
	mPopup->addAction(ActionRefresh);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(museeq, SIGNAL(roomList(const NRoomList&)), SLOT(setRooms(const NRoomList&)));
	connect(museeq, SIGNAL(privRoomList(const NPrivRoomList&)), SLOT(setPrivRooms(const NPrivRoomList&)));
	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void RoomListView::onClosingMuseeq() {
    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "roomList_Layout";
        museeq->settings()->setValue(optionName, header()->saveState());
    }
}

void RoomListView::setRooms(const NRoomList& _r) {
	m_roomListCache = _r;

	setRoomsFromCache();
}

void RoomListView::setPrivRooms(const NPrivRoomList& _r) {
	setRoomsFromCache();
}

void RoomListView::setRoomsFromCache() {
	clear();

	QMap<QString, unsigned int>::const_iterator it = m_roomListCache.begin();
	for(; it != m_roomListCache.end(); ++it)
		new RoomListItem(this, it.key(), it.value(), tr("Public"));

    NPrivRoomList privRoomListCache = museeq->getPrivRoomList();
	NPrivRoomList::const_iterator itp = privRoomListCache.begin();
	for(; itp != privRoomListCache.end(); ++itp) {
	    QStringList role;
	    role << tr("member") << tr("operator") << tr("owner") << tr("unknown");
	    uint intRole = itp.value().second;
	    if ((int) intRole >= role.size())
            intRole = role.size() -1;
		new RoomListItem(this, itp.key(), itp.value().first, tr("Private (%1)").arg(role[intRole]));
	}

    if (!museeq->mainwin()->isSavingAllLayouts()) {
        sortItems(0, Qt::AscendingOrder);
        sortItems(2, Qt::AscendingOrder);
    }

	adaptColumnSize(0);
	adaptColumnSize(1);
}

void RoomListView::slotJoin() {
	museeq->joinRoom(mPopped);
}

void RoomListView::slotLeave() {
	museeq->leaveRoom(mPopped);
}

void RoomListView::slotDisown() {
	museeq->mainwin()->doPrivRoomDisown(mPopped);
}

void RoomListView::slotDismember() {
	museeq->mainwin()->doPrivRoomDismember(mPopped);
}

void RoomListView::slotRefresh() {
	museeq->updateRoomList();
}
void RoomListView::slotActivate(QTreeWidgetItem* item, int column) {

	RoomListItem* _item = dynamic_cast<RoomListItem*>(item);
	if(item)
		museeq->joinRoom(_item->room());
}

void RoomListView::slotContextMenu(const QPoint& pos) {
	RoomListItem* item = dynamic_cast<RoomListItem*>(itemAt(pos));

	if (! item ) {
		mPopped = QString::null;
		ActionJoin->setEnabled(false);
		ActionLeave->setEnabled(false);
		mActionDisown->setEnabled(false);
		mActionDismember->setEnabled(false);
		ActionRefresh->setEnabled(museeq->isConnected());

	} else {
		mPopped = item->room();
		ActionJoin->setEnabled(! museeq->isJoined(mPopped));
		ActionLeave->setEnabled(museeq->isJoined(mPopped));
        mActionDisown->setEnabled(museeq->isRoomOwned(mPopped));
        mActionDismember->setEnabled(museeq->isRoomPrivate(mPopped) && !museeq->isRoomOwned(mPopped));
		ActionRefresh->setEnabled(museeq->isConnected());
	}

	mPopup->exec(mapToGlobal(pos));
}

void RoomListView::adaptColumnSize(int column) {
    if (!museeq->mainwin()->isSavingAllLayouts())
        resizeColumnToContents(column);
}
