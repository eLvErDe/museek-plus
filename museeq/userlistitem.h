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

#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QTreeWidget>

class UserListView;

class UserListItem : public QTreeWidgetItem {
public:
	UserListItem(UserListView *, const QString&, uint, uint, uint, const QString&, const QString&);

	uint status() const;
	void updateUserStatus();
	QString user() const;
	uint speed() const;
	uint files() const;
	QString comments() const;
	QString country() const;
	bool isOperator() const;
	bool isOwner() const;

	void setStatus(uint);
	void setSpeed(uint);
	void setFiles(uint);
	void setComments(const QString&);
	void setCountry(const QString&);
	void setOperator(bool);
	void setOwner(bool);
	void setAll(uint, uint, uint, const QString&, const QString&);
	bool operator<(const QTreeWidgetItem & other) const;
private:
	uint mStatus;
	bool mIsOperator, mIsOwner;
	QString mUser, mComments, mCountry;
	uint mSpeed, mFiles;
};

#endif // USERLISTVIEWITEM_H
