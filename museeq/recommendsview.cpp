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
#include "mainwin.h"
#include "recommendsview.h"
#include "recommendsitem.h"
#include "images.h"

#include <QMenu>
#include <QHeaderView>
#include <QSettings>

RecommendsView::RecommendsView(QWidget* _p, const char* _n)
             : QTreeWidget(_p) {
	QStringList headers;
	headers << tr("Interests") << tr("Number");
	setHeaderLabels(headers);
	setSortingEnabled(true);
	sortByColumn(1, Qt::DescendingOrder);
	setColumnWidth(0, 200);

	setRootIsDecorated(false);

 	setAllColumnsShowFocus(true);

	mPopup = new QMenu(this);


	ActionAddLike = new QAction(IMG("add"),tr("Add item to Likes"), this);
	connect(ActionAddLike, SIGNAL(triggered()), this, SLOT(slotAddLike()));
	mPopup->addAction(ActionAddLike);

	ActionAddHate = new QAction(IMG("add"),tr("Add item to Hates"), this);
	connect(ActionAddHate, SIGNAL(triggered()), this, SLOT(slotAddHate()));
	mPopup->addAction(ActionAddHate);

    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "recommends_Layout";
        header()->restoreState(museeq->settings()->value(optionName).toByteArray());
    }

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(museeq, SIGNAL(Recommendations(const NGlobalRecommendations&)), SLOT(setGlobalRecs(const NGlobalRecommendations&)));

	connect(museeq, SIGNAL(aRecommendations(const NRecommendations&)), SLOT(setRecs(const NRecommendations&)));
	connect(museeq, SIGNAL(itemRecommendations(const QString&, const NItemRecommendations&)), SLOT(setItemRecs(const QString&, const NItemRecommendations&)));

	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void RecommendsView::onClosingMuseeq() {
    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "recommends_Layout";
        museeq->settings()->setValue(optionName, header()->saveState());
    }
}

void RecommendsView::setGlobalRecs(const NGlobalRecommendations& _r) {
	clear();

	QMap<QString, int>::const_iterator it = _r.begin();
	for(; it != _r.end(); ++it)
		new RecommendsItem(this, it.key(), it.value());
}

void RecommendsView::setRecs(const NRecommendations& _r) {
	clear();

	QMap<QString, int>::const_iterator rit = _r.begin();
	for(; rit != _r.end(); ++rit)
		new RecommendsItem(this, rit.key(), rit.value());
}

void RecommendsView::setItemRecs(const QString& _i,  const NItemRecommendations& _r) {
	clear();

	QMap<QString, int>::const_iterator iit = _r.begin();
	for(; iit != _r.end(); ++iit)
		new RecommendsItem(this, iit.key(), iit.value());
}

void RecommendsView::slotAddLike() {
	museeq->addInterest(mPopped);
}

void RecommendsView::slotAddHate() {
	museeq->addHatedInterest(mPopped);
}

void RecommendsView::slotContextMenu(const QPoint& pos) {
	RecommendsItem* item = dynamic_cast<RecommendsItem*>(itemAt(pos));

	if (item ) {
		mPopped = item->interest();
		ActionAddLike->setEnabled(museeq->isConnected());
		ActionAddHate->setEnabled(museeq->isConnected());

	} else {
		mPopped = QString::null;
		ActionAddLike->setEnabled(false);
		ActionAddHate->setEnabled(false);

	}

	mPopup->exec(mapToGlobal(pos));
}


void RecommendsView::slotActivate(QTreeWidgetItem* item, int column) {
	slotActivate( item);
}

void RecommendsView::slotActivate(QTreeWidgetItem* item) {
	RecommendsItem* _item = dynamic_cast<RecommendsItem*>(item);
	if(item)
		museeq->joinRoom(_item->interest());
}

void RecommendsView::adaptColumnSize(int column) {
    if (!museeq->mainwin()->isSavingAllLayouts())
        resizeColumnToContents(column);
}
