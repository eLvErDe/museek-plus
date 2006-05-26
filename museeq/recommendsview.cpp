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

#include "recommendsview.h"

#include <qpopupmenu.h>
#include "recommendsitem.h"
#include "museeq.h"

RecommendsView::RecommendsView(QWidget* _p, const char* _n)
             : QListView(_p, _n) {

	addColumn("Interests");
	addColumn("Num");

	setColumnAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
	setSorting(1);
	setShowSortIndicator(true);
	setAllColumnsShowFocus(true);
	
	mPopup = new QPopupMenu(this);
	mPopup->insertItem("Add item to Likes", this, SLOT(slotAddLike()), 0, 0);
	mPopup->insertItem("Add item to Hates", this, SLOT(slotAddHate()), 0, 1);
	
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(slotPopupMenu(QListViewItem*, const QPoint&, int)));
	connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)));
	connect(this, SIGNAL(returnPressed(QListViewItem*)), SLOT(slotReturnPressed(QListViewItem*)));
	
	connect(museeq, SIGNAL(Recommendations(const NGlobalRecommendations&)), SLOT(setGlobalRecs(const NGlobalRecommendations&)));

	connect(museeq, SIGNAL(aRecommendations(const NRecommendations&)), SLOT(setRecs(const NRecommendations&)));
	connect(museeq, SIGNAL(itemRecommendations(const QString&, const NItemRecommendations&)), SLOT(setItemRecs(const QString&, const NItemRecommendations&)));

	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}

void RecommendsView::setGlobalRecs(const NGlobalRecommendations& _r) {
	clear();
	
	QMap<QString, unsigned int>::const_iterator it = _r.begin();
	for(; it != _r.end(); ++it)
		new RecommendsItem(this, it.key(), it.data());
}

void RecommendsView::setRecs(const NRecommendations& _r) {
	clear();
	
	QMap<QString, unsigned int>::const_iterator rit = _r.begin();
	for(; rit != _r.end(); ++rit)
		new RecommendsItem(this, rit.key(), rit.data());
}

void RecommendsView::setItemRecs(const QString& _i,  const NItemRecommendations& _r) {
	clear();
	
	QMap<QString, unsigned int>::const_iterator iit = _r.begin();
	for(; iit != _r.end(); ++iit)
		new RecommendsItem(this, iit.key(), iit.data());
}

void RecommendsView::slotAddLike() {
	museeq->addInterest(mPopped);
}

void RecommendsView::slotAddHate() {
	museeq->addHatedInterest(mPopped);
}


void RecommendsView::slotPopupMenu(QListViewItem* item, const QPoint& pos, int) {
	RecommendsItem* _item = static_cast<RecommendsItem*>(item);
	if(item) {
		mPopped = _item->interest();
		mPopup->setItemEnabled(0, museeq->isConnected());
		mPopup->setItemEnabled(1, museeq->isConnected());
	} else {
		mPopped = QString::null;
		mPopup->setItemEnabled(0, false);
		mPopup->setItemEnabled(1, false);
	}
	mPopup->exec(pos);
}

void RecommendsView::slotDoubleClicked(QListViewItem* item, const QPoint&, int) {
	slotReturnPressed(item);
}

void RecommendsView::slotReturnPressed(QListViewItem* item) {
	RecommendsItem* _item = static_cast<RecommendsItem*>(item);
	if(item)
		museeq->joinRoom(_item->interest());
}
