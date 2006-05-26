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

#include <qtoolbutton.h>
#include <qaccel.h>
#include <qpainter.h>

#include "slskdrag.h"
#include "images.h"
#include "usermenu.h"

TabWidget::TabWidget(QWidget* parent, const char* name, bool isUser)
          : QTabWidget(parent, name), mProtectFirst(true), mProtectThird(false){

	const QString& Name = name ;
	if ( Name == "userInfo")
		setProtectThird(true);

	mTabBar = new TabBar(isUser, this);
	setTabBar(mTabBar);
	connect(mTabBar, SIGNAL(dropSlsk(const QStringList&)), SLOT(dropSlsk(const QStringList&)));
	
	setTabPosition(QTabWidget::Bottom);
	
	QToolButton* b = new QToolButton(this, "closeButton");
	b->setIconSet(IMG("tab_remove"));
	b->adjustSize();
	setCornerWidget(b, QTabWidget::BottomRight);
	connect(b, SIGNAL(clicked()), SLOT(closeCurrent()));
	
	connect(this, SIGNAL(currentChanged(QWidget*)), SLOT(doCurrentChanged(QWidget*)));
	
	QAccel *accel = new QAccel(this);
	accel->connectItem(accel->insertItem(ALT + Key_Left), this, SLOT(previousPage()));
	accel->connectItem(accel->insertItem(ALT + Key_Right), this, SLOT(nextPage()));
	accel->connectItem(accel->insertItem(CTRL + Key_W), this, SLOT(closeCurrent()));
}

QString TabWidget::getCurrentPage() const {
	int index = currentPageIndex();
	if(mProtectFirst && index == 0)
		return QString::null;
	return label(index);
}

QWidget * TabWidget::getCurrentWidget() const {
	return currentPage();
}

bool TabWidget::protectFirst() const {
	return mProtectFirst;
}
bool TabWidget::protectThird() const {
	return mProtectThird;
}
bool TabWidget::canDrop() const {
	return mTabBar->canDrop();
}

void TabWidget::setCanDrop(bool b) {
	mTabBar->setCanDrop(b);
}

void TabWidget::setProtectThird(bool protectThird) {
	mProtectThird = protectThird;
	if(currentPageIndex() == 0 || currentPageIndex() == 1 || currentPageIndex() == 2 ) {
		if(! mProtectThird && ! currentPageIndex() == 0)
			cornerWidget()->setEnabled(true);
		else
			cornerWidget()->setEnabled(false);
	}
}

void TabWidget::doCurrentChanged(QWidget*) {
	if(mProtectFirst && ! mProtectThird)
		cornerWidget()->setEnabled(currentPageIndex() != 0);
	if(mProtectThird)
		cornerWidget()->setEnabled(currentPageIndex() != 0 && currentPageIndex() != 1 && currentPageIndex() != 2 );
	if(Tab* tab = dynamic_cast<Tab*>(tabBar()->tab(tabBar()->currentTab())))
		tab->selected();
}

void TabWidget::closeCurrent() {
	if(! mProtectThird || ! mProtectFirst  ||  mProtectFirst && currentPageIndex() > 0 || mProtectThird && currentPageIndex() > 2)
		delete currentPage();
}

void TabWidget::previousPage() {
	int ix = currentPageIndex() - 1;
	if(ix < 0)
		ix = count() - 1;
	setCurrentPage(ix);
}

void TabWidget::nextPage() {
	setCurrentPage((currentPageIndex() + 1) % count());
}

void TabWidget::dropSlsk(const QStringList&) {
}

bool TabWidget::isCurrent(Tab* tab) {
	return tabBar()->tab(tabBar()->currentTab()) == tab;
}

void TabWidget::repaintTabBar() {
	tabBar()->update();
}

TabBar::TabBar(bool isUser, QWidget* parent, const char* name)
       : QTabBar(parent, name), mCanDrop(false) {
	
	setAcceptDrops(true);
	
	if(isUser)
		mUsermenu = new Usermenu(this);
	else
		mUsermenu = 0;
}

void TabBar::dragMoveEvent(QDragMoveEvent* event) {
	event->accept(mCanDrop && SlskDrag::canDecode(event));
	
	QTab* tab = selectTab(event->pos());
	if(! tab)
		return;
	
	setCurrentTab(tab);
}

bool TabBar::canDrop() const {
	return mCanDrop;
}

void TabBar::setCanDrop(bool b) {
	mCanDrop = b;
}

void TabBar::dropEvent(QDropEvent* event) {
	QStringList l;
	if(SlskDrag::decode(event, l))
		emit dropSlsk(l);
}

void TabBar::paintLabel(QPainter* p, const QRect& br, QTab* t, bool has_focus) const {
	QFont oldfont = p->font(), font = oldfont;
	QPen oldpen = p->pen();
	
	p->setPen(colorGroup().buttonText());
	
	if(Tab* tab = dynamic_cast<Tab*>(t)) {
		if(tab->highlighted() > 0)
			font.setUnderline(true);
		p->setFont(font);
		
		if(tab->highlighted() > 1)
			p->setPen(QColor(255, 0, 0));
	}
	
	p->drawText(br, Qt::AlignAuto | Qt::AlignVCenter, t->text());
	
	p->setFont(oldfont);
	p->setPen(oldpen);
}

void TabBar::contextMenuEvent(QContextMenuEvent* e) {
	if(mUsermenu) {
		QTab* _tab = selectTab(e->pos());
		if(_tab && _tab != tab(0)) {
			mUsermenu->exec(_tab->text(), e->globalPos());
			return;
		}
	}
	e->ignore();
}

Tab::Tab(TabWidget* parent, const QString& text)
    : QObject(parent), QTab(text), mHighlight(0) {
};

void Tab::setHighlight(int i) {
	TabWidget* p = static_cast<TabWidget*>(parent());
	if(! p->isCurrent(this) && i > mHighlight) {
		mHighlight = i;
		p->repaintTabBar();
	}
}

void Tab::selected() {
	if(mHighlight != 0) {
		mHighlight = 0;
		static_cast<TabWidget*>(parent())->repaintTabBar();
	}
}

int Tab::highlighted() const {
	return mHighlight;
}

