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

#include "userinfos.h"
#include "userinfo.h"
#include "museeq.h"

UserInfos::UserInfos(QWidget* parent, const char* name)
          : UserTabWidget(parent, name) {

	connect(museeq, SIGNAL(userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)), SLOT(setUserInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)));
	connect(museeq, SIGNAL(userInterests(const QString&, const QStringList&, const QStringList&)), SLOT(setUserInterests(const QString&, const QStringList&, const QStringList&)));
}

UserWidget* UserInfos::makeNewPage(const QString& user) {
	UserInfo* w = new UserInfo(user);
	w->getUserInfo();
	return dynamic_cast<UserWidget*>(w);
}

void UserInfos::setUserInfo(const QString& user, const QString& info, const QByteArray& picture, uint upslots, uint queue, bool free) {
	UserInfo* _info = dynamic_cast<UserInfo*>(page(user));
	if(_info)
		_info->setInfo(info, picture, upslots, queue, free);
}

void UserInfos::setUserInterests(const QString& user, const QStringList& likes, const QStringList& hates) {
	UserInfo* _info = dynamic_cast<UserInfo*>(page(user));
	if(_info)
		_info->setInterests(likes, hates);
}
