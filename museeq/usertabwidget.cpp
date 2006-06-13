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

#include "usertabwidget.h"

#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <qaccel.h>
#include <qurl.h>

#include "codeccombo.h"
#include "buddylist.h"
#include "banlist.h"
#include "ignorelist.h"
#include "museeq.h"

UserTabWidget::UserTabWidget(QWidget* _p, const char* _n)
              : TabWidget(_p, _n, true) {
	
	setCanDrop(false);

	const QString& Name = _n ;
	mBuddyList = new BuddyList(0, "buddyList");
	connect(mBuddyList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));
	addTab(mBuddyList, tr("*Buddies*"));
	if (Name == "userInfo") {
		mBanList = new BanList(0, "banList");
		mIgnoreList = new IgnoreList(0, "ignoreList");	
		connect(mBanList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));	
		connect(mIgnoreList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));	
		addTab(mIgnoreList, tr("*Ignored*"));
		addTab(mBanList, tr("*Banned*"));
	}

	
	
	connect(museeq, SIGNAL(connectedToServer(bool)), SLOT(setCanDrop(bool)));
}

UserWidget* UserTabWidget::page(const QString& _u, bool _m) {
	for(int i = 1; i < count(); ++i) {
		UserWidget* frame = static_cast<UserWidget*>(QTabWidget::page(i));
		if(frame->user() == _u) {
			return frame;
		}
	}
	
	if(! _m)
		return 0;
		
	UserWidget* frame = makeNewPage(_u);
	connect(frame, SIGNAL(highlight(int)), SIGNAL(highlight(int)));
	emit(highlight(2));
	
	Tab* new_tab = new Tab(this, _u);
	connect(frame, SIGNAL(highlight(int)), new_tab, SLOT(setHighlight(int)));
	addTab(frame, new_tab);
	emit newPage(_u);
	
	return frame;
}

UserWidget* UserTabWidget::makeNewPage(const QString& _u) {
	return new UserWidget(_u);
}

void UserTabWidget::setPage(const QString& _u) {
	if(canDrop())
		showPage(page(_u, true));
}

void UserTabWidget::dropSlsk(const QStringList& l) {
	QStringList::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url = QUrl(*it);
		if(url.protocol() == "slsk" && url.hasHost()) {
			QString user = url.host();
			QUrl::decode(user);
			setPage(user);
		}
	}
}
