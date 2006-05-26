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

#include "interestlistview.h"

#include <qpopupmenu.h>

#include "museeq.h"

InterestListView::InterestListView( const QString& caption,  QWidget* _p, const char* _n )
             : QListView(_p, _n) {

	addColumn(caption);
	setSorting(0);
	setShowSortIndicator(true);
	setAllColumnsShowFocus(true);
	
	mPopup = new QPopupMenu(this);
	if ( caption == "I like:" ) {
		mPopup->insertItem("Remove", this, SLOT(slotRemoveInterest()), 0, 0);
	}
	else if ( caption == "I hate:" ) {
		mPopup->insertItem("Remove", this, SLOT(slotRemoveHatedInterest()), 0, 0);
	}
	mPopup->insertItem("Recommendations for this Item", this, SLOT(slotItemRecommendations()), 0, 1);
	mPopup->insertItem("Similar Users for this Item", this, SLOT(slotItemSimilarUsers()), 0, 2);

	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(slotPopupMenu(QListViewItem*, const QPoint&, int)));

	connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)), SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)));

	connect(this, SIGNAL(returnPressed(QListViewItem*)), SLOT(slotReturnPressed(QListViewItem*)));


	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}


void InterestListView::slotRemoveInterest() {
	museeq->removeInterest(mPopped);
}

void InterestListView::slotRemoveHatedInterest() {
	museeq->removeHatedInterest(mPopped);
}
void InterestListView::slotItemRecommendations() {
	museeq->updateItemRecommendations(mPopped);
}
void InterestListView::slotItemSimilarUsers() {
	museeq->updateItemSimilarUsers(mPopped);
}


void InterestListView::slotPopupMenu(QListViewItem* item, const QPoint& pos, int) {

 	if(item) {
		mPopped = item->text(0);
		mPopup->exec(pos);
	} 
	
}

void InterestListView::slotDoubleClicked(QListViewItem* item, const QPoint&, int) {
	slotReturnPressed(item);
}

void InterestListView::slotReturnPressed(QListViewItem* item) {
 	if(item) 
 		museeq->updateItemRecommendations(item->text(0));
}
