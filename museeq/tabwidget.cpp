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

#include "tabwidget.h"
#include "images.h"
#include "usermenu.h"
#include "util.h"

#include <QToolButton>
#include <QDropEvent>
#include <QShortcut>
#include <QUrl>

TabWidget::TabWidget(QWidget* parent, const char* name, bool isUser)
          : QTabWidget(parent), mProtectFirst(true), mProtectThird(false){

	const QString& Name = name ;
	if ( Name == "userInfo")
		setProtectThird(true);

	mTabBar = new TabBar(isUser, this);
	setTabBar(mTabBar);

	setTabPosition(QTabWidget::North);

	QToolButton* b = new QToolButton(this);
	b->setIcon(IMG("tab_remove"));
	b->adjustSize();
	setCornerWidget(b, Qt::BottomRightCorner);
	connect(b, SIGNAL(clicked()), SLOT(closeCurrent()));

	connect(this, SIGNAL(currentChanged(QWidget*)), SLOT(doCurrentChanged(QWidget*)));

	new QShortcut(Qt::ALT + Qt::Key_Left, this, SLOT(previousPage()));
	new QShortcut(Qt::ALT + Qt::Key_Right, this, SLOT(nextPage()));
	new QShortcut(Qt::CTRL + Qt::Key_W, this, SLOT(closeCurrent()));
}

QString TabWidget::getCurrentPage() const {
	int index = currentIndex();
	if(mProtectFirst && index == 0)
		return QString::null;
	return tabText(index);
}

QWidget * TabWidget::getCurrentWidget() const {
	return currentWidget();
}

bool TabWidget::protectFirst() const {
	return mProtectFirst;
}
bool TabWidget::protectThird() const {
	return mProtectThird;
}
bool TabWidget::canDrop() const {
	return mTabBar->acceptDrops();
}

void TabWidget::setCanDrop(bool b) {
	mTabBar->setAcceptDrops(b);
}

void TabWidget::setProtectThird(bool protectThird) {
	mProtectThird = protectThird;
	if(currentIndex() >= 0 && currentIndex() <= 3 ) {
		if(! mProtectThird && ! currentIndex() == 0)
			cornerWidget()->setEnabled(true);
		else
			cornerWidget()->setEnabled(false);
	}
}

void TabWidget::doCurrentChanged(QWidget*) {
	if(mProtectFirst && ! mProtectThird)
		cornerWidget()->setEnabled(currentIndex() != 0);
	if(mProtectThird)
		cornerWidget()->setEnabled(currentIndex() > 3);
}

void TabWidget::closeCurrent() {
	if(! mProtectFirst  ||  (! mProtectThird && mProtectFirst && currentIndex() > 0) || (mProtectThird && currentIndex() > 3))
		delete currentWidget();
}

void TabWidget::previousPage() {
	int ix = currentIndex() - 1;
	if(ix < 0)
		ix = count() - 1;
	setCurrentIndex(ix);
}

void TabWidget::nextPage() {
	setCurrentIndex((currentIndex() + 1) % count());
}

void TabWidget::repaintTabBar() {
	tabBar()->update();
}


TabBar::TabBar(bool isUser, QWidget* parent, const char* name)
       : QTabBar(parent) {

 	setAcceptDrops(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	if(isUser) {
		mUsermenu = new Usermenu(this);
		connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));

	} else
		mUsermenu = 0;

}

void TabBar::dragMoveEvent(QDragMoveEvent* event) {
    // Find the tab
 	int tab = tabAt(event->pos());

    // Switch to the tab if any found
 	if(tab >= 0)
 		setCurrentIndex(tab);

    // We can drop urls directly on the icon: accept
    if(Util::hasSlskUrls(event) && tab >= 0 && acceptDrops())
        event->acceptProposedAction();
}

void TabBar::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void TabBar::dropEvent(QDropEvent* event) {
    int item = currentIndex();

    if (item >= 0 && Util::hasSlskUrls(event) && acceptDrops())
        emit dropSlsk(event->mimeData()->urls());

    event->acceptProposedAction();
}

void TabBar::slotContextMenu(const QPoint& pos) {
	QString username;
	if(mUsermenu) {
 		int index = tabAt(pos);
		if (tabData(index).toString() == "1")
			return;
		else {
			username = tabText(index);
			mUsermenu->exec(username, mapToGlobal(pos));
		}
	}

}
