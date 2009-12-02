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

#include "usertabwidget.h"
#include "codeccombo.h"
#include "buddylist.h"
#include "banlist.h"
#include "ignorelist.h"
#include "trustlist.h"
#include "images.h"
#include "museeq.h"

#include <QUrl>

UserTabWidget::UserTabWidget(QWidget* _p, const char* _n)
              : TabWidget(_p, _n, true) {

	setCanDrop(false);

	const QString& Name = _n ;
	mBuddyList = new BuddyList(this, "buddyList-"+Name);
	// No popup
	connect(mBuddyList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));
	connect(mTabBar, SIGNAL(dropSlsk(const QList<QUrl>&)), SLOT(dropSlsk(const QList<QUrl>&)));
	addTab(mBuddyList, tr("*Buddies*"));
	tabBar()->setTabData(0, QVariant("1"));
	if (Name == "userInfo") {
		mBanList = new BanList(this, "banList");
		mIgnoreList = new IgnoreList(this, "ignoreList");
		mTrustList = new TrustList(this, "trustList");
		connect(mBanList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));
		connect(mIgnoreList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));
		connect(mTrustList, SIGNAL(activated(const QString&)), SLOT(setPage(const QString&)));
		addTab(mTrustList, tr("*Trusted*"));
		addTab(mBanList, tr("*Banned*"));
		addTab(mIgnoreList, tr("*Ignored*"));
		tabBar()->setTabData(1, QVariant("1"));
		tabBar()->setTabData(2, QVariant("1"));
		tabBar()->setTabData(3, QVariant("1"));
	}


	connect(this, SIGNAL(currentChanged(QWidget*)), SLOT(doCurrentChanged(QWidget*)));
	connect(museeq, SIGNAL(connectedToServer(bool)), SLOT(setCanDrop(bool)));
}

UserWidget* UserTabWidget::page(const QString& _u, bool _m) {
	for(int i = 1; i < count(); ++i) {
		UserWidget* frame = dynamic_cast<UserWidget*>(QTabWidget::widget(i));
		if(frame && frame->user() == _u) {
			return frame;
		}
	}

	if(! _m)
		return 0;

	UserWidget* frame = makeNewPage(_u);
	connect(frame, SIGNAL(highlight(int)), SIGNAL(highlight(int)));
	emit(highlight(2));

	int new_tab_position = addTab(frame, _u);
	connect(frame, SIGNAL(highlight(int)), widget(new_tab_position), SLOT(setHighlight(int)));

	emit newPage(_u);

	return frame;
}

UserWidget* UserTabWidget::makeNewPage(const QString& _u) {
	return new UserWidget(_u);
}

void UserTabWidget::doCurrentChanged(QWidget* userwidget) {
	UserWidget* uw = dynamic_cast<UserWidget*>(userwidget);
	if (uw)
        uw->selected();
}

void UserWidget::selected() {
	if(highlighted() != 0) {
		setHighlight(0);
		setHighlighted(0);
	}
}

void UserWidget::setHighlight(int i) {
	UserTabWidget* p = dynamic_cast<UserTabWidget*>(parentWidget()->parentWidget());
	if (p)
        p->setHighlight(p->indexOf(this), i);
}

void UserTabWidget::setHighlight(int pos, int highlight) {
	UserWidget * uw = dynamic_cast<UserWidget*>(widget(pos));

    if (!uw)
        return;

	if(( currentIndex() != pos) && highlight > uw->highlighted() )
	{
		uw->setHighlighted(highlight);

		if (uw->highlighted() > 0) {
			// Icon on tab
			tabBar()->setTabIcon(pos, QIcon(IMG("new-element")));
		}
		if (uw->highlighted() > 1) {
			// Red tab
			tabBar()->setTabTextColor(pos, QColor(255, 0, 0));
		}
	} else {
		tabBar()->setTabTextColor(pos, tabBar()->palette().buttonText().color());
		tabBar()->setTabIcon(pos, QIcon());
	}
}

void UserTabWidget::setPage(const QString& _u) {
	if(canDrop())
 	// Create page and switch to it
		setCurrentIndex(indexOf(page(_u, true)));
}

void UserTabWidget::dropSlsk(const QList<QUrl>& l) {
	QList<QUrl>::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url = QUrl(*it);
		if(url.scheme() == "slsk" && !url.host().isEmpty()) {
			QString user = url.userName();
			if (user.isEmpty())
                user = url.host();

            setPage(user);
		}
	}
}
