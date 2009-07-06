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

#include "usermenu.h"
#include "museeq.h"
#include "mainwin.h"
#include "images.h"

Usermenu::Usermenu(QWidget* parent, const char* name)
         : QMenu(parent) {

	ActionPrivate = new QAction(IMG("privatechat-small"), tr("Private chat"), this);
	connect(ActionPrivate, SIGNAL(triggered()), this, SLOT(slotPrivateChat()));
	addAction(ActionPrivate);

	ActionUserInfo = new QAction(IMG("userinfo-small"), tr("Get user info"), this);
	connect(ActionUserInfo, SIGNAL(triggered()), this, SLOT(slotUserInfo()));
	addAction(ActionUserInfo);

	ActionUserShares = new QAction(IMG("browser-small"), tr("Get user shares"), this);
	connect(ActionUserShares, SIGNAL(triggered()), this, SLOT(slotBrowser()));
	addAction(ActionUserShares);

	ActionGivePrivileges = new QAction(IMG("privileges"), tr("Give Soulseek privileges"), this);
	connect(ActionGivePrivileges, SIGNAL(triggered()), this, SLOT(slotPrivileges()));
	addAction(ActionGivePrivileges);
	addSeparator();
	ActionBuddy = new QAction(tr("Buddy"), this);
	ActionBuddy->setCheckable(true);
	connect(ActionBuddy, SIGNAL(triggered()), this, SLOT(slotBuddy()));
	addAction(ActionBuddy);

	ActionTrust = new QAction(tr("Trusted"), this);
	ActionTrust->setCheckable(true);
	connect(ActionTrust, SIGNAL(triggered()), this, SLOT(slotTrusted()));
	addAction(ActionTrust);

	ActionBan = new QAction(tr("Banned"), this);
	ActionBan->setCheckable(true);
	connect(ActionBan, SIGNAL(triggered()), this, SLOT(slotBanned()));
	addAction(ActionBan);

	ActionIgnore = new QAction(tr("Ignored"), this);
	ActionIgnore->setCheckable(true);
	connect(ActionIgnore, SIGNAL(triggered()), this, SLOT(slotIgnored()));
	addAction(ActionIgnore);
	addSeparator();

	mMenuPrivMember = addMenu(tr("Member of private rooms"));
    connect(mMenuPrivMember, SIGNAL(triggered(QAction*)), this, SLOT(switchPrivMember(QAction*)));
	mMenuPrivOperator = addMenu(tr("Operator of private rooms"));
    connect(mMenuPrivOperator, SIGNAL(triggered(QAction*)), this, SLOT(switchPrivOperator(QAction*)));
	mMenuPrivOwner = addMenu(tr("Owner of private rooms"));
    connect(mMenuPrivOwner, SIGNAL(triggered(QAction*)), this, SLOT(switchPrivOwner(QAction*)));

	addSeparator();

	ActionIp = new QAction(IMG("ip"), tr("Show IP"), this);
	connect(ActionIp, SIGNAL(triggered()), this, SLOT(slotIP()));
	addAction(ActionIp);
	addSeparator();
	ActionAlert = new QAction(IMG("alert"), tr("Online alert"), this);
	ActionAlert->setCheckable(true);
	connect(ActionAlert, SIGNAL(triggered()), this, SLOT(slotAlert()));
	addAction(ActionAlert);

	ActionComments = new QAction(IMG("comments"), tr("Edit comments"), this);
	connect(ActionComments, SIGNAL(triggered()), this, SLOT(slotComments()));
	addAction(ActionComments);
}

void Usermenu::setup(const QString& user) {
	mUser = user;

	bool connected = museeq->isConnected();

	ActionPrivate->setEnabled(connected);
	ActionUserInfo->setEnabled(connected);
	ActionUserShares->setEnabled(connected);
	ActionGivePrivileges->setEnabled(connected);
	ActionIgnore->setEnabled(connected);
	ActionBuddy->setEnabled(connected);
	ActionBan->setEnabled(connected);
	ActionTrust->setEnabled(connected);
	ActionIp->setEnabled(connected);
	ActionAlert->setEnabled(connected);
	ActionComments->setEnabled(museeq->isBuddy(user));

	ActionBuddy->setChecked(museeq->isBuddy(user));
	ActionTrust->setChecked(museeq->isTrusted(user));
	ActionBan->setChecked(museeq->isBanned(user));
	ActionIgnore->setChecked(museeq->isIgnored(user));
	ActionAlert->setChecked(museeq->hasAlert(user));

    // Fill private rooms stuff
	mMenuPrivMember->clear();
	mMenuPrivOperator->clear();
	mMenuPrivOwner->clear();
	mMenuPrivMember->setEnabled(false);
	mMenuPrivOperator->setEnabled(false);
	mMenuPrivOwner->setEnabled(false);

	bool ourSelf = (mUser == museeq->nickname());
	if (connected) {
	    NPrivRoomList privRoomList = museeq->getPrivRoomList();
	    NPrivRoomList::const_iterator it = privRoomList.begin();
        for (; it != privRoomList.end(); it++) {
            QString roomName = it.key();

            bool noAuthority = (it->second == 0);
            bool canAddAsMember = museeq->canAddAsMember(roomName, mUser);
            bool canDismember = museeq->canDismember(roomName, mUser);
            bool canAddAsOperator = museeq->canAddAsOperator(roomName, mUser);
            bool canDisop = museeq->canDisop(roomName, mUser);
            bool canDisown = museeq->isRoomOwned(roomName);

            if (canAddAsMember || canDismember || (ourSelf && !canDisop && !canDisown && noAuthority)) {
                QAction * addedAction = mMenuPrivMember->addAction(roomName);
                addedAction->setCheckable(true);
                addedAction->setChecked(!canAddAsMember);
                mMenuPrivMember->setEnabled(true);
            }

            if (canAddAsOperator || canDisop || (ourSelf && !noAuthority && !canDisown)) {
                QAction * addedAction = mMenuPrivOperator->addAction(roomName);
                addedAction->setCheckable(true);
                addedAction->setChecked(!canAddAsOperator);
                mMenuPrivOperator->setEnabled(true);
            }

            if (!ourSelf)
                continue;

            if (it->second == 2) { // Room owned by us
                QAction * addedAction = mMenuPrivOwner->addAction(roomName);
                addedAction->setCheckable(true);
                addedAction->setChecked(true);
                mMenuPrivOwner->setEnabled(true);
            }
        }
	}
}

void Usermenu::exec(const QString& user) {
	setup(user);
	QMenu::exec();
}

void Usermenu::exec(const QString& user, const QPoint& pos) {
	setup(user);
	QMenu::exec(pos);
}

QString Usermenu::user() const {
	return mUser;
}

void Usermenu::slotPrivateChat() {
	printf("open private chat\n");
	museeq->mainwin()->showPrivateChat(mUser);
}

void Usermenu::slotUserInfo() {
	museeq->mainwin()->showUserInfo(mUser);
}

void Usermenu::slotBrowser() {
	museeq->mainwin()->showBrowser(mUser);
}

void Usermenu::slotBuddy() {
	if(! museeq->isBuddy(mUser))
		museeq->addBuddy(mUser);
	else
		museeq->removeBuddy(mUser);
}

void Usermenu::slotTrusted() {
	if(! museeq->isTrusted(mUser))
		museeq->addTrusted(mUser);
	else
		museeq->removeTrusted(mUser);
}

void Usermenu::slotBanned() {
	if(! museeq->isBanned(mUser))
		museeq->addBanned(mUser);
	else
		museeq->removeBanned(mUser);
}

void Usermenu::slotIgnored() {
	if(! museeq->isIgnored(mUser))
		museeq->addIgnored(mUser);
	else
		museeq->removeIgnored(mUser);
}

void Usermenu::slotIP() {
	museeq->mainwin()->showIPDialog(mUser);


}

void Usermenu::slotAlert() {
	if(! museeq->hasAlert(mUser))
		museeq->addAlert(mUser);
	else
		museeq->removeAlert(mUser);
}

void Usermenu::slotPrivileges() {
	museeq->mainwin()->givePrivileges(mUser);
}

void Usermenu::slotComments() {
	museeq->editComments(mUser);
}

void Usermenu::switchPrivMember(QAction* action) {
    if (action->isChecked())
        museeq->mainwin()->doPrivRoomAddUser(action->text(), mUser);
    else if (mUser == museeq->nickname())
        museeq->mainwin()->doPrivRoomDismember(action->text());
    else
        museeq->mainwin()->doPrivRoomRemoveUser(action->text(), mUser);
}

void Usermenu::switchPrivOperator(QAction* action) {
    if (action->isChecked())
        museeq->mainwin()->doPrivRoomAddOperator(action->text(), mUser);
    else if (mUser == museeq->nickname())
        museeq->mainwin()->doPrivRoomDisown(action->text());
    else
        museeq->mainwin()->doPrivRoomRemoveOperator(action->text(), mUser);
}

void Usermenu::switchPrivOwner(QAction* action) {
    if (!action->isChecked())
        museeq->mainwin()->doPrivRoomDisown(action->text());
}
