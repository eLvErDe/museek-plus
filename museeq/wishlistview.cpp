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

#include "museeq.h"
#include "wishlistview.h"
#include "wishlistitem.h"
#include "images.h"
#include "mainwin.h"

#include <QMenu>
#include <QHeaderView>
#include <QSettings>

WishListView::WishListView(QWidget* _p, const char* _n)
             : QTreeWidget(_p) {
	setColumnCount(2);

	QStringList headers;
	headers << tr("Item") << tr("Last try");
	setColumnWidth(0, 200);
	setHeaderLabels(headers);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
	setRootIsDecorated(false);
 	setAllColumnsShowFocus(true);

    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "wishlist_Layout";
        header()->restoreState(museeq->settings()->value(optionName).toByteArray());
    }

	mPopup = new QMenu(this);
	ActionRemove = new QAction(IMG("remove"), tr("Remove"), this);
	connect(ActionRemove, SIGNAL(triggered()), this, SLOT(slotRemove()));
	mPopup->addAction(ActionRemove);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void WishListView::onClosingMuseeq() {
    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "wishlist_Layout";
        museeq->settings()->setValue(optionName, header()->saveState());
    }
}

void WishListView::slotRemove() {
	museeq->removeWishItem(mPopped);
}

void WishListView::slotActivate(QTreeWidgetItem* item, int column) {
	WishListItem* _item = dynamic_cast<WishListItem*>(item);
	if(item)
		museeq->mainwin()->startSearch(_item->query());
}

void WishListView::slotContextMenu(const QPoint& pos) {
	WishListItem* item = dynamic_cast<WishListItem*>(itemAt(pos));

	if (! item ) {
		mPopped = QString::null;
		ActionRemove->setEnabled(false);
	}
	else {
		mPopped = item->query();
		ActionRemove->setEnabled(true);
	}

	mPopup->exec(mapToGlobal(pos));
}

void WishListView::adaptColumnSize(int column) {
    if (!museeq->mainwin()->isSavingAllLayouts())
        resizeColumnToContents(column);
}
