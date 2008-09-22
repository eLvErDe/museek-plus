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

#include "museeq.h"
#include "interestlistview.h"
#include "images.h"

#include <QMenu>

InterestListView::InterestListView( const QString& caption,  QWidget* _p, const char* _n )
             : QTreeWidget(_p) {

	QStringList headers;
	headers << caption ;
	setHeaderLabels(headers);

 	setAllColumnsShowFocus(true);
	setSortingEnabled(true);
	setRootIsDecorated(false);
	mPopup = new QMenu(this);

	ActionRemove = new QAction(IMG("remove"),tr("Remove"), this);

	if ( caption == tr("I like:") ) {
		connect(ActionRemove, SIGNAL(triggered()), this, SLOT(slotRemoveInterest()));
	}
	else if ( caption == tr("I hate:") ) {
		connect(ActionRemove, SIGNAL(triggered()), this, SLOT(slotRemoveHatedInterest()));
	}
	mPopup->addAction(ActionRemove);

	ActionRecommendations = new QAction(tr("Recommendations for this Item"), this);
	connect(ActionRecommendations, SIGNAL(triggered()), this, SLOT(slotItemRecommendations()));
	mPopup->addAction(ActionRecommendations);

	ActionItemSimilarUsers = new QAction(tr("Similar Users for this Item"), this);
	connect(ActionItemSimilarUsers, SIGNAL(triggered()), this, SLOT(slotItemSimilarUsers()));
	mPopup->addAction(ActionItemSimilarUsers);


	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));


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



void InterestListView::slotContextMenu(const QPoint& pos) {
	QTreeWidgetItem * item = itemAt(pos);
	if(item) {
		mPopped = item->text(0);
		mPopup->exec(mapToGlobal(pos));
	}
}


void InterestListView::slotActivate(QTreeWidgetItem* item, int column) {
	slotActivate( item);
}
void InterestListView::slotActivate(QTreeWidgetItem* item) {
 	if(item)
 		museeq->updateItemRecommendations(item->text(0));
}

