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

#include "privatechats.h"
#include "privatechat.h"
#include "museeq.h"

PrivateChats::PrivateChats(QWidget* _p, const char* _n)
             : UserTabWidget(_p, _n) {

	connect(museeq, SIGNAL(privateMessage(uint, uint, const QString&, const QString&)), SLOT(append(uint, uint, const QString&, const QString&)));
	connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(setUserStatus(const QString&, uint)));
}

void PrivateChats::append(uint _dir, uint _ts, const QString& _u, const QString& _m) {
	PrivateChat* tab = dynamic_cast<PrivateChat*>(page(_u, true));
	if (tab)
        tab->append(_dir, _ts, _u, _m);
}

void PrivateChats::setUserStatus(const QString& _u, uint _s) {

	PrivateChat* tab = dynamic_cast<PrivateChat*>(page(_u, false));
	if (tab)
		tab->status(_u, _s);

}

UserWidget* PrivateChats::makeNewPage(const QString& _u) {
	PrivateChat* pm = new PrivateChat(_u, this);
	museeq->slotUserExists(_u);
	return pm;
}

