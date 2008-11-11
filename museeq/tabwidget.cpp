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

#include "tabwidget.h"
#include "images.h"
#include "usermenu.h"
#include "util.h"

#include <QToolButton>
#include <QDropEvent>
#include <QShortcut>
#include <QUrl>

TabWidget::TabWidget(QWidget* parent, const char* name, bool isUser)
          : QTabWidget(parent), mFirstProtected(0), mLastProtected(0){

	const QString& Name = name ;
	if ( Name == "userInfo")
		mLastProtected = 3;

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
	if(( (currentIndex() >= mFirstProtected) && (currentIndex() <= mLastProtected) ))
		return QString::null;
	return tabText(index);
}

QWidget * TabWidget::getCurrentWidget() const {
	return currentWidget();
}

int TabWidget::firstProtected() const {
	return mFirstProtected;
}
int TabWidget::lastProtected() const {
	return mLastProtected;
}
bool TabWidget::canDrop() const {
	return mTabBar->acceptDrops();
}

void TabWidget::setCanDrop(bool b) {
	mTabBar->setAcceptDrops(b);
}

void TabWidget::setLastProtected(int lastProtected) {
	mLastProtected = lastProtected;
	cornerWidget()->setEnabled(!( (currentIndex() >= mFirstProtected) && (currentIndex() <= mLastProtected) ));
}

void TabWidget::setFirstProtected(int firstProtected) {
	mFirstProtected = firstProtected;
	cornerWidget()->setEnabled(!( (currentIndex() >= mFirstProtected) && (currentIndex() <= mLastProtected) ));
}

void TabWidget::doCurrentChanged(QWidget*) {
	cornerWidget()->setEnabled(!( (currentIndex() >= mFirstProtected) && (currentIndex() <= mLastProtected) ));
}

void TabWidget::closeCurrent() {
	if(!( (currentIndex() >= mFirstProtected) && (currentIndex() <= mLastProtected) ))
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
