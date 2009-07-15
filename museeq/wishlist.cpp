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
#include "wishlist.h"
#include "wishlistview.h"
#include "wishlistitem.h"

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

WishList::WishList(QWidget* _p, const char* _n)
         : QWidget(_p) {

	mWishList = new WishListView(this);

	QVBoxLayout *MainLayout = new QVBoxLayout(this);
	MainLayout->addWidget(mWishList);

	QHBoxLayout *layout = new QHBoxLayout;
	MainLayout->addLayout(layout);

	QLabel *label = new QLabel(tr("Add:"), this);
	layout->addWidget(label);
	mEntry = new QLineEdit(this);
	layout->addWidget(mEntry);

	mAdd = new QPushButton(tr("Add"), this);
	layout->addWidget(mAdd);

	connect(mEntry, SIGNAL(returnPressed()), SLOT(slotAddWish()));
	connect(mAdd, SIGNAL(clicked()), SLOT(slotAddWish()));
	connect(museeq, SIGNAL(addedWishItem(const QString&, uint)), SLOT(added(const QString&, uint)));
	connect(museeq, SIGNAL(removedWishItem(const QString& )), SLOT(removed(const QString& )));
}

void WishList::slotAddWish() {
	QString s = mEntry->text();
	if(s.isEmpty())
		return;
	mEntry->setText(QString::null);
	museeq->addWishItem(s);
}

void WishList::added(const QString& item, uint lastSearched) {
	QList<QTreeWidgetItem *> itemMatch = mWishList->findItems(item, Qt::MatchExactly, 0);
	if (! itemMatch.isEmpty()) {
        WishListItem* itemFound = dynamic_cast<WishListItem*>(itemMatch.at(0));
        if (itemFound)
            itemFound->setLastSearched(lastSearched);
	}
	else {
	    new WishListItem(mWishList, item, lastSearched);
	    mWishList->adaptColumnSize(0);
	}
}

void WishList::removed(const QString& item) {
	QList<QTreeWidgetItem *> itemMatch = mWishList->findItems(item, Qt::MatchExactly, 0);
	if (itemMatch.isEmpty())
		return;
	QTreeWidgetItem* i = itemMatch.at(0);
	if (! i)
		return;
	delete i;
    mWishList->adaptColumnSize(0);
}

void WishList::showEvent(QShowEvent*) {
	mEntry->setFocus();
}
