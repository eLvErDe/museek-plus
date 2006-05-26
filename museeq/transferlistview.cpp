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

#include "transferlistview.h"

#include <qpainter.h>

#include "transferlistitem.h"
#include "slskdrag.h"
#include "museeq.h"

TransferListView::TransferListView(bool place, QWidget* _p, const char* _n)
                 : QListView(_p, _n), mGroupMode(None) {
	
	setAllColumnsShowFocus(true);
	setShowSortIndicator(true);
	setSelectionMode(Extended);
	
	addColumn("User", 100);
	addColumn("File", 250);
	addColumn("Status");
	addColumn("Place", 50);
	if(! place)
		setColumnWidth(3, 0);
	addColumn("Position", 100);
	addColumn("Size", 100);
	addColumn("Speed", 100);
	addColumn("Path");
	
	setColumnAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
	
	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}

void TransferListView::update(const NTransfer& transfer) {
	TransferListItem* file = static_cast<TransferListItem*>(firstChild());
	
	for(; file != 0; file = static_cast<TransferListItem*>(file->nextSibling()))
		if(file->user() == transfer.user && (mGroupMode != None || file->path() == transfer.filename))
			break;
	
	if(! file) {
		file = new TransferListItem(this, transfer.user, (mGroupMode == None) ? transfer.filename : QString::null);
		file->setOpen(false);
	}
	
	file->update(transfer);
}

void TransferListView::remove(const QString& _u, const QString& _p) {
	TransferListItem* item = static_cast<TransferListItem*>(firstChild());
	for(; item != 0; item = static_cast<TransferListItem*>(item->nextSibling()))
		if(item->user() == _u && (mGroupMode != None || item->path() == _p)) {
			if(mGroupMode == None)
				delete item;
			else {
				item->remove(_p);
				if(item->childCount() == 0)
					delete item;
			}
			return;
		}
}

TransferListView::GroupMode TransferListView::groupMode() const {
	return mGroupMode;
}

void TransferListView::setGroupMode(GroupMode mode) {
	if(mode == mGroupMode)
		return;

	mGroupMode = mode;
	
	if(mGroupMode == None) {
		QPtrList<QListViewItem> users;
		
		QListViewItem* user = firstChild();
		for(; user != 0; user = user->nextSibling())
			users.append(user);
		
		QPtrList<QListViewItem>::iterator it = users.begin();
		for(; it != users.end(); ++it) {
			QPtrList<QListViewItem> items;
			
			QListViewItem* item = (*it)->firstChild();
			for(; item != 0; item = item->nextSibling())
				items.append(item);
			
			QPtrList<QListViewItem>::iterator items_it = items.begin();
			for(; items_it != items.end(); ++items_it) {
				(*it)->takeItem(item);
				insertItem(*items_it);
			}
			delete (*it);
		}
		setRootIsDecorated(false);
	} else {
		QPtrList<TransferListItem> items;
		
		QListViewItem* item = firstChild();
		for(; item != 0; item = item->nextSibling())
			items.append(static_cast<TransferListItem*>(item));
		
		QMap<QString, TransferListItem*> users;
		
		QPtrList<TransferListItem>::iterator items_it = items.begin();
		for(; items_it != items.end(); ++items_it) {
			TransferListItem* user;
			
			QMap<QString, TransferListItem*>::iterator it = users.find((*items_it)->user());
			if(it != users.end())
				user = it.data();
			else {
				user = new TransferListItem(this, (*items_it)->user());
				user->setOpen(false);
				users[(*items_it)->user()] = user;
			}
			takeItem(*items_it);
			user->insertItem(*items_it);
		}
		
		QMap<QString, TransferListItem*>::iterator user_it = users.begin();
		for(; user_it != users.end(); ++user_it)
			user_it.data()->updateStats();
		
		setRootIsDecorated(true);
	}
}

void TransferListView::dragEnterEvent(QDragEnterEvent* event) {
	event->accept(SlskDrag::canDecode(event, true));
}

void TransferListView::dropEvent(QDropEvent* event) {
	QStringList l;
	if(SlskDrag::decode(event, l))
		emit dropSlsk(l);
}

QDragObject* TransferListView::dragObject() {
	SlskDrag* drag = new SlskDrag(viewport());
	
	QValueList<TransferListItem*> items;
	QStringList users;
	
	QListViewItemIterator it(this, QListViewItemIterator::Selected);
	while(it.current())
	{
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		if(users.find(item->user()) == users.end())
			users << item->user();
		drag->append(item->user(), item->path());
		items << item;
		
		it++;
	}
	
	if(! items.count()) {
		delete drag;
		return 0;
	}
	
	QString x;
	if(items.count() == 1)
		x = "1 transfer (1 user)";
	else if(users.count() == 1)
		x = QString("%1 transfers (1 user)").arg(items.count());
	else
		x = QString("%1 transfers (%2 users)").arg(items.count()).arg(users.count());
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
	
	return drag;
}
