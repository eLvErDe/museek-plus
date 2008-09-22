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

#include "museeq.h"
#include "roomlist.h"
#include "roomlistview.h"

#include <QLayout>
#include <QLabel>
#include <QLineEdit>

RoomList::RoomList(QWidget* _p, const char* _n)
         : QWidget(_p) {

	mRoomList = new RoomListView(this);

	QVBoxLayout *MainLayout = new QVBoxLayout(this);
	MainLayout->addWidget(mRoomList);

	QHBoxLayout *layout = new QHBoxLayout;
	MainLayout->addLayout(layout);

	QLabel *label = new QLabel(tr("Create:"), this);
	layout->addWidget(label);
	mEntry = new QLineEdit(this);
	layout->addWidget(mEntry);


	connect(mEntry, SIGNAL(returnPressed()), SLOT(slotJoinRoom()));
}

void RoomList::slotJoinRoom() {
	QString s = mEntry->text();
	if(s.isEmpty())
		return;
	mEntry->setText(QString::null);
	museeq->joinRoom(s);
}

void RoomList::showEvent(QShowEvent*) {
	mEntry->setFocus();
}
