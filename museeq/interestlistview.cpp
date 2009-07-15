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
#include "interestlistview.h"
#include "images.h"
#include "interestlist.h"

#include <QMenu>
#include <QHeaderView>
#include <QSettings>

InterestListView::InterestListView( const QString& caption,  QWidget* _p, bool readOnly )
             : QTreeWidget(_p), mReadOnly(readOnly) {

	QStringList headers;
	headers << caption ;
	setHeaderLabels(headers);

 	setAllColumnsShowFocus(true);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
	setRootIsDecorated(false);
	mPopup = new QMenu(this);
    if (!mReadOnly) {
        ActionRemove = new QAction(IMG("remove"),tr("Remove"), this);

        if ( caption == tr("I like:") ) {
            connect(ActionRemove, SIGNAL(triggered()), this, SLOT(slotRemoveInterest()));
            mStateOptionName = "likeInterests_Layout";
        }
        else if ( caption == tr("I hate:") ) {
            connect(ActionRemove, SIGNAL(triggered()), this, SLOT(slotRemoveHatedInterest()));
            mStateOptionName = "hateInterests_Layout";
        }
        mPopup->addAction(ActionRemove);

        if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
            header()->restoreState(museeq->settings()->value(mStateOptionName).toByteArray());
        }
    }

	ActionRecommendations = new QAction(tr("Recommendations for this item"), this);
	connect(ActionRecommendations, SIGNAL(triggered()), this, SLOT(slotItemRecommendations()));
	mPopup->addAction(ActionRecommendations);

	ActionItemSimilarUsers = new QAction(tr("Similar users for this item"), this);
	connect(ActionItemSimilarUsers, SIGNAL(triggered()), this, SLOT(slotItemSimilarUsers()));
	mPopup->addAction(ActionItemSimilarUsers);


	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void InterestListView::onClosingMuseeq() {
    if (!mReadOnly && museeq->settings()->value("saveAllLayouts", false).toBool() && !mStateOptionName.isEmpty()) {
        museeq->settings()->setValue(mStateOptionName, header()->saveState());
    }
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

