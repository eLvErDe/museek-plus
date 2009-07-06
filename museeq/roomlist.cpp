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
#include "roomlist.h"
#include "roomlistview.h"

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

RoomList::RoomList(QWidget* _p, const char* _n)
         : QWidget(_p), mPublicChatStarted(false) {

	mRoomList = new RoomListView(this);

	QVBoxLayout *MainLayout = new QVBoxLayout(this);
	MainLayout->addWidget(mRoomList);

	QHBoxLayout *layout = new QHBoxLayout;
	MainLayout->addLayout(layout);

	QLabel *label = new QLabel(tr("Create a new room:"), this);
	layout->addWidget(label);
	mEntry = new QLineEdit(this);
	mEntry->setMaxLength(24);
	layout->addWidget(mEntry);

    mPrivate = new QCheckBox(tr("Private"), this);
    mPrivate->setChecked(false);
	layout->addWidget(mPrivate);

	mCreate = new QPushButton(tr("Create"), this);
	layout->addWidget(mCreate);

    layout->addStretch();

	mPublicChatToggle = new QPushButton(tr("Show public chat"), this);
	layout->addWidget(mPublicChatToggle);

	connect(mEntry, SIGNAL(returnPressed()), SLOT(slotJoinRoom()));
	connect(mCreate, SIGNAL(clicked()), SLOT(slotJoinRoom()));
	connect(mPublicChatToggle, SIGNAL(clicked()), SLOT(slotPublicChatToggle()));

	connect(museeq, SIGNAL(askedPublicChat()), SLOT(askedPublicChat()));
	connect(museeq, SIGNAL(stoppedPublicChat()), SLOT(stoppedPublicChat()));
}

void RoomList::slotJoinRoom() {
	QString s = mEntry->text();
	if(s.isEmpty())
		return;
	mEntry->setText(QString::null);
	museeq->joinRoom(s, mPrivate->isChecked());
}

void RoomList::slotPublicChatToggle() {
    if (!mPublicChatStarted) {
        museeq->askPublicChat();
    }
    else
        museeq->stopPublicChat();

    doTogglePublicChat();
}

void RoomList::doTogglePublicChat() {
    if (mPublicChatStarted)
        mPublicChatToggle->setText(tr("Show public chat"));
    else
        mPublicChatToggle->setText(tr("Stop public chat"));

    mPublicChatStarted = !mPublicChatStarted;
}

void RoomList::showEvent(QShowEvent*) {
	mEntry->setFocus();
}

void RoomList::askedPublicChat() {
    if (!mPublicChatStarted) { // Avoid infinite loops
        doTogglePublicChat();
    }
}

void RoomList::stoppedPublicChat() {
    if (mPublicChatStarted) { // Avoid infinite loops
        doTogglePublicChat();
    }
}
