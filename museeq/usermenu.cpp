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

#include "usermenu.h"
#include "museeq.h"
#include "mainwin.h"

Usermenu::Usermenu(QWidget* parent, const char* name)
         : QPopupMenu(parent, name) {
	
	setCheckable(true);
	insertItem("Private chat", (int)0);
	insertItem("Get user info", (int)1);
	insertItem("Get user shares", (int)2);
	insertItem("Give Soulseek privileges", (int)9);
	insertSeparator();
	insertItem("Buddy", (int)3);
	insertItem("Trusted", (int)4);
	insertItem("Banned", (int)5);
	insertItem("Ignored", (int)6);
	insertSeparator();
	insertItem("Show IP", (int)7);
	insertSeparator();
	insertItem("Online alert", (int)8);
	insertItem("Edit Comments", (int) 10);

	connect(this, SIGNAL(activated(int)), SLOT(slotActivated(int)));
}

void Usermenu::setup(const QString& user) {
	mUser = user;
	
	bool connected = museeq->isConnected();
	setItemEnabled(0, connected);
	setItemEnabled(1, connected);
	setItemEnabled(2, connected);
	setItemEnabled(6, connected);
	setItemEnabled(8, connected);
	
	setItemChecked(3, museeq->isBuddy(user));
	setItemChecked(4, museeq->isTrusted(user));
	setItemChecked(5, museeq->isBanned(user));
	setItemChecked(6, museeq->isIgnored(user));
	
	setItemEnabled(8, museeq->isBuddy(user));
	setItemChecked(8, museeq->hasAlert(user));
	setItemEnabled(10, (museeq->isBuddy(user)  ));
}

void Usermenu::exec(const QString& user) {
	setup(user);
	QPopupMenu::exec();
}

void Usermenu::exec(const QString& user, const QPoint& pos) {
	setup(user);
	QPopupMenu::exec(pos);
}

QString Usermenu::user() const {
	return mUser;
}

void Usermenu::slotActivated(int id) {
	switch(id) {
	case 0:
		museeq->mainwin()->showPrivateChat(mUser);
		break;
	case 1:
		museeq->mainwin()->showUserInfo(mUser);
		break;
	case 2:
		museeq->mainwin()->showBrowser(mUser);
		break;
	case 3:
		if(! museeq->isBuddy(mUser))
			museeq->addBuddy(mUser);
		else
			museeq->removeBuddy(mUser);
		break;
	case 4:
		if(! museeq->isTrusted(mUser))
			museeq->addTrusted(mUser);
		else
			museeq->removeTrusted(mUser);
		break;
	case 5:
		if(! museeq->isBanned(mUser))
			museeq->addBanned(mUser);
		else
			museeq->removeBanned(mUser);
		break;
	case 6:
		if(! museeq->isIgnored(mUser))
			museeq->addIgnored(mUser);
		else
			museeq->removeIgnored(mUser);
		break;
	case 7:
		museeq->mainwin()->showIPDialog(mUser);
		break;
	case 8:
		if(! museeq->hasAlert(mUser))
			museeq->addAlert(mUser);
		else
			museeq->removeAlert(mUser);
		break;
	case 9:
		museeq->mainwin()->givePrivileges(mUser);
		break;
	case 10:
		museeq->editComments(mUser);
		break;
	}
}
