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

#ifndef USERMENU_H
#define USERMENU_H

#include <QMenu>

class Usermenu : public QMenu {
	Q_OBJECT
public:
	Usermenu(QWidget* = 0, const char* = 0);
	QString user() const;
	void setup(const QString&);
	QAction * ActionPrivate, * ActionUserInfo, * ActionUserShares, * ActionGivePrivileges, * ActionBuddy, * ActionBan, * ActionTrust, * ActionIgnore, * ActionIp, * ActionAlert, * ActionComments;
	QMenu *mMenuPrivMember, *mMenuPrivOperator, *mMenuPrivOwner;
public slots:
	void exec(const QString&);
	void exec(const QString&, const QPoint&);

protected slots:
	void slotPrivateChat();
	void slotUserInfo();
	void slotBrowser();
	void slotBuddy();
	void slotTrusted();
	void slotBanned();
	void slotIgnored();
	void slotIP();
	void slotAlert();
	void slotPrivileges();
	void slotComments();
	void switchPrivMember(QAction*);
	void switchPrivOperator(QAction*);
	void switchPrivOwner(QAction*);

private:
	QString mUser;
};

#endif // USERMENU_H
