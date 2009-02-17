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

#include "userlistitem.h"
#include "userlistview.h"
#include "util.h"
#include "images.h"
#include "museeq.h"

#include <QIcon>

UserListItem::UserListItem(UserListView *_parent, const QString& _u, uint _s, uint _sp, uint _f, const QString& _c)
	     : QTreeWidgetItem(static_cast<QTreeWidget *>(_parent)), mUser(_u) {

	setText(1, mUser);
	setTextAlignment(2, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	setStatus(_s);
	setSpeed(_sp);
	setFiles(_f);
	setComments(_c);
}
void UserListItem::updateUserStatus() {
	setStatus(mStatus);
}
void UserListItem::setStatus(uint s) {
	mStatus = s;

	QString icon;
	switch(s) {
	case 0:
		if (museeq->isBanned(mUser))
			icon = "offline-banned";
		else if (museeq->isIgnored(mUser))
			icon = "offline-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "offline-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "offline-buddied";
		else
			icon = "offline";
		break;
	case 1:
		if (museeq->isBanned(mUser))
			icon = "away-banned";
		else if (museeq->isIgnored(mUser))
			icon = "away-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "away-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "away-buddied";
		else
			icon = "away";
		break;
	default:
		if (museeq->isBanned(mUser))
			icon = "online-banned";
		else if (museeq->isIgnored(mUser))
			icon = "online-ignored";
		else if (museeq->isTrusted(mUser))
			icon = "online-trusted";
		else if (museeq->isBuddy(mUser))
			icon = "online-buddied";
		else
			icon = "online";
	}
	QIcon newicon;
	newicon.addPixmap(IMG(icon));
	setIcon(0, newicon);
}

void UserListItem::setSpeed(uint s) {
	mSpeed = s;
	setText(2, Util::makeSize(mSpeed) + UserListView::tr("/s"));
}

void UserListItem::setFiles(uint f) {
	mFiles = f;
	setText(3, QString("%1").arg(mFiles));
}

void UserListItem::setComments(const QString& _c) {
	mComments = _c;
	setText(4, mComments);
}

void UserListItem::setAll(uint st, uint sp, uint f, const QString& c) {
	setStatus(st);
	setSpeed(sp);
	setFiles(f);
	setComments(c);
}

uint UserListItem::status() const {
	return mStatus;
}

QString UserListItem::user() const {
	return mUser;
}

uint UserListItem::speed() const {
	return mSpeed;
}

uint UserListItem::files() const {
	return mFiles;
}

QString UserListItem::comments() const {
	return mComments;
}

bool UserListItem::operator<(const QTreeWidgetItem & other_) const {
	const UserListItem * other = dynamic_cast<const UserListItem *>(&other_);
	if (!other)
        return false;

	int col = 0;
	if(treeWidget())
	col = treeWidget()->sortColumn();

	switch(col) {
	case 0:
		if(status() == other->status())
			return user().toLower() < other->user().toLower();
		return status() < other->status();
	case 1:
		return user().toLower() < other->user().toLower();
	case 2:
		if(speed() == other->speed())
			return user().toLower() < other->user().toLower();
		return speed() < other->speed();
	case 3:
		if(files() == other->files())
			return user() < other->user();
		return files() < other->files();
	case 4:
		return comments().toLower() < other->comments().toLower();

	}

  return false;
}
