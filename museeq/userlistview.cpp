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

#include "userlistview.h"

#include <qpixmap.h>
#include <qpainter.h>

#include "userlistitem.h"
#include "slskdrag.h"
#include "usermenu.h"
#include "museeq.h"

#include "images.h"

UserListView::UserListView(bool comments, QWidget * parent, const char * name)
	: QListView(parent, name) {
	
	mUsermenu = new Usermenu(this);
	
	setSelectionMode(Extended);
	
	addColumn(QString::null);
	addColumn(tr("User"), 150);
	addColumn(tr("Speed"));
	addColumn(tr("Files"));
	if(comments) {
		addColumn(tr("Comments"));
		setResizeMode(QListView::LastColumn);
	}
	
	setSorting(1);
	setShowSortIndicator(true);
	setColumnWidthMode(0, QListView::Manual);
	setColumnAlignment(2, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	QPixmap& p(IMG("online"));
	setColumnWidth(0, p.width()+5);
	setAllColumnsShowFocus(true);
	
	connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), SLOT(slotActivate(QListViewItem*, const QPoint&, int)));
	connect(this, SIGNAL(returnPressed(QListViewItem*)), SLOT(slotActivate(QListViewItem*)));
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(slotContextMenu(QListViewItem*, const QPoint&, int)));
	
	connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(setStatus(const QString&, uint)));
	connect(museeq, SIGNAL(doUpdateStatus(const QString&)), SLOT(updateStatus(const QString&)));
	connect(museeq, SIGNAL(userData(const QString&, uint, uint)), SLOT(setData(const QString&, uint, uint)));
}

UserListItem* UserListView::findItem(const QString& _u) {
	QListViewItem *item = QListView::findItem(_u, 1);
	return static_cast<UserListItem *>(item);
}

uint UserListView::status(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->status();
}

uint UserListView::speed(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->speed();
}

uint UserListView::files(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->files();
}

QString UserListView::comments(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return QString::null;
	return item->comments();
}

void UserListView::setStatus(const QString& _u, uint _s) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setStatus(_s);
}

void UserListView::updateStatus(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return;
	item->updateUserStatus();
}

void UserListView::setData(const QString& _u, uint _s, uint _f) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setSpeed(_s);
	item->setFiles(_f);
}

void UserListView::setComments(const QString& _u, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setComments(_c);
}

void UserListView::add(const QString& _u, uint _st, uint _s, uint _f, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(item) {
		item->setAll(_st, _s, _f, _c);
		return;
	}
	new UserListItem(this, _u, _st, _s, _f, _c);
}

void UserListView::add(const QString& _u, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(item) {
		item->setComments(_c);
		return;
	}
	new UserListItem(this, _u, 0, 0, 0, _c);
}

void UserListView::remove(const QString& _u) {
	UserListItem *item = findItem(_u);
	if(! item) {
		printf("warning, couldn't find user!\n");
		return;
	}
	delete item;
}

void UserListView::slotActivate(QListViewItem* item) {
	UserListItem* _item = static_cast<UserListItem*>(item);
	if(! _item)
		return;
	emit activated(_item->user());
	emit activated(_item->user(), _item->comments());
}

void UserListView::slotActivate(QListViewItem* item, const QPoint&, int) {
	slotActivate(item);
}

QDragObject* UserListView::dragObject() {
	SlskDrag* drag = new SlskDrag(viewport());

	QValueList<UserListItem*> items;
	UserListItem* item = static_cast<UserListItem*>(firstChild());
	for(; item != 0; item = static_cast<UserListItem*>(item->nextSibling()))
		if(isSelected(item)) {
			drag->append(item->user());
			items << item;
		}
	
	if(! items.count()) {
		delete drag;
		return 0;
	}
	
	if(items.count() < 6) {
		QSize dest(0, 0);
		QValueList<UserListItem*>::const_iterator it = items.begin();
		for(; it != items.end(); ++it) {
			QSize s1 = viewport()->fontMetrics().size(Qt::SingleLine, (*it)->user());
			QSize s2 = (*it)->pixmap()->size();
			s1 = s1.expandedTo(QSize(s1.width() + s2.width() + 2, s2.height()));
			dest = dest.expandedTo(QSize(s1.width(), s1.height() + 2));
		}
		
		QSize s(dest.width() + 6, dest.height() * items.count() + 4);
		
		QPixmap pix(s);
		QPainter p(&pix);
		p.setFont(viewport()->font());
		p.setPen(viewport()->foregroundColor());
		
		p.fillRect(QRect(QPoint(0, 0), s), viewport()->eraseColor());
		p.drawRect(QRect(QPoint(0, 0), s));
		
		int y = 2;
		for(it = items.begin(); it != items.end(); ++it) {
			const QPixmap* icon = (*it)->pixmap();
			p.drawPixmap(3, y + ((dest.height() - icon->height()) / 2), *icon);
			QRect r(QPoint(5 + icon->width(), y),
			        dest - QSize(icon->width() + 2, 0));
			p.drawText(r, AlignAuto | AlignVCenter, (*it)->user());
			y += dest.height();
		}
		
		p.end();
		
		drag->setPixmap(pix, QPoint(-25, -25));
	} else {
		QString x(QString().sprintf("%i users", items.count()));
		QSize s = viewport()->fontMetrics().size(Qt::SingleLine, x) + QSize(6, 4);
		
		QPixmap pix(s);
		QPainter p(&pix);
		p.setFont(viewport()->font());
		p.setPen(viewport()->foregroundColor());
		
		p.fillRect(QRect(QPoint(0, 0), s), viewport()->eraseColor());
		p.drawRect(QRect(QPoint(0, 0), s));
		p.drawText(QRect(QPoint(3, 3), s - QSize(3, 3)), AlignAuto | AlignVCenter, x);
		
		p.end();
		drag->setPixmap(pix, QPoint(-25, -25));
	}
	
	return drag;
}


void UserListView::dragEnterEvent(QDragEnterEvent* event) {
	event->accept(SlskDrag::canDecode(event));
}

void UserListView::dropEvent(QDropEvent* event) {
	QStringList l;
	if(SlskDrag::decode(event, l))
		emit dropSlsk(l);
}

void UserListView::slotContextMenu(QListViewItem* item, const QPoint& pos, int) {
	if(! item)
		return;
	mUsermenu->exec((static_cast<UserListItem *>(item))->user(), pos);
}
