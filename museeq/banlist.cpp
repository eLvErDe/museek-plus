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
#include "banlist.h"
#include "userlistview.h"
#include "userlistitem.h"
#include "mainwin.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QInputDialog>
#include <QLayout>
#include <QUrl>


BanList::BanList(QWidget* _p, const char* _n)
          : QWidget(){
    setAcceptDrops(true);
	mUserList = new UserListView(true, this, "banlist");
	mUserList->setAcceptDrops(true);
	connect(mUserList, SIGNAL(activated(const QString&)), SIGNAL(activated(const QString&)));
	connect(mUserList, SIGNAL(dropSlsk(const QList<QUrl>&)), SLOT(slotDropSlsk(const QList<QUrl>&)));
	connect(museeq, SIGNAL(addedBanned(const QString&, const QString&)), mUserList, SLOT(add(const QString&, const QString&)));
	connect(museeq, SIGNAL(removedBanned(const QString&)), mUserList, SLOT(remove(const QString&)));
	connect(museeq, SIGNAL(addedBanned(const QString&, const QString&)), this, SLOT(bannedListChanged()));
	connect(museeq, SIGNAL(removedBanned(const QString&)), this, SLOT(bannedListChanged()));
	connect(museeq, SIGNAL(disconnected()), mUserList, SLOT(clear()));


	QVBoxLayout *MainLayout = new QVBoxLayout(this);
	MainLayout->addWidget(mUserList);

	QHBoxLayout *layout = new QHBoxLayout;
	MainLayout->addLayout(layout);

	QLabel *label = new QLabel(tr("Ban a new user:"), this);
	layout->addWidget(label);
	mEntry = new QLineEdit(this);
	layout->addWidget(mEntry);

	mAdd = new QPushButton(tr("Add"), this);
	layout->addWidget(mAdd);

    layout->addStretch();

	mSendMessageToAll = new QPushButton(tr("Send a message to all banned users"), this);
	mSendMessageToAll->setEnabled(false);
	layout->addWidget(mSendMessageToAll);

	connect(mEntry, SIGNAL(returnPressed()), SLOT(addBanned()));
	connect(mAdd, SIGNAL(clicked()), SLOT(addBanned()));
	connect(mSendMessageToAll, SIGNAL(clicked()), SLOT(sendMessageToAll()));
}

void BanList::bannedListChanged() {
    mSendMessageToAll->setEnabled(museeq->banned().size() > 0);
}

void BanList::addBanned() {
	QString n = mEntry->text();
	mEntry->setText(QString::null);

	if(n.isEmpty())
		return;

	editComments(n);
}

void BanList::sendMessageToAll() {
    bool res;
	QString m = QInputDialog::getText(museeq->mainwin(), tr("Send a message to all banned users"), tr("Write the message you want to send to all users in your banned list"), QLineEdit::Normal, QString::null, &res);
	if (res && !m.isEmpty())
        museeq->messageUsers(m, museeq->banned());
}

void BanList::editComments(const QString& n) {
	QString _c;
	UserListItem* item = mUserList->findItem(n);
	if(item)
		_c = item->comments();

    bool res;
	QString c = QInputDialog::getText(museeq->mainwin(), tr("Comments"), tr("Comments for %1").arg(n), QLineEdit::Normal, _c, &res);
	if (res)
        museeq->addBanned(n, c);
}

void BanList::showEvent(QShowEvent*) {
	mEntry->setFocus();
}

void BanList::slotDropSlsk(const QList<QUrl>& l) {
	QList<QUrl>::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url = QUrl(*it);
		if(url.scheme() == "slsk" && !url.host().isEmpty()) {
			QString user = url.userName();
			if (user.isEmpty())
                user = url.host();

			if (!mUserList->findItem(user))
                editComments(user);
		}
	}
}
