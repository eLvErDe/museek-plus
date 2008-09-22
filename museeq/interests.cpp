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
#include "interests.h"
#include "images.h"
#include "userlistview.h"
#include "interestlist.h"
#include "recommendsview.h"

#include <QPushButton>
#include <QSplitter>
#include <QLabel>
#include <QLayout>

Interests::Interests(QWidget* parent, const char* name)
          : QWidget(parent) {

	QVBoxLayout* MainLayout = new QVBoxLayout(this);
	QHBoxLayout* bbox = new QHBoxLayout;
	MainLayout->addLayout(bbox);

	QLabel* w = new QLabel(tr("Interests"));
	bbox->addWidget(w);
	QFont f = w->font();
	f.setBold(true);
	w->setFont(f);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	mGlobal = new QPushButton(tr("Global"));
	mGlobal->setIcon(QIcon(IMG("refresh")));
	bbox->addWidget(mGlobal);
	connect(museeq, SIGNAL(connectedToServer(bool)), mGlobal, SLOT(setEnabled(bool)));
	connect(mGlobal, SIGNAL(clicked()), SLOT(getGlobalRecommendations()));

	mRecommend = new QPushButton(tr("Recommend"));
	mRecommend->setIcon(QIcon(IMG("refresh")));
	bbox->addWidget(mRecommend);
	connect(museeq, SIGNAL(connectedToServer(bool)), mRecommend, SLOT(setEnabled(bool)));
	connect(mRecommend, SIGNAL(clicked()), SLOT(getRecommendations()));

	mSimilar = new QPushButton(tr("Similar"));
	mSimilar->setIcon(QIcon(IMG("refresh")));
	bbox->addWidget(mSimilar);
	connect(museeq, SIGNAL(connectedToServer(bool)), mSimilar, SLOT(setEnabled(bool)));
	connect(mSimilar, SIGNAL(clicked()), SLOT(getSimilarUsers()));

	QSplitter* split = new QSplitter(this);
	MainLayout->addWidget(split);
	QWidget * interestsWidget = new QWidget(split);
	QVBoxLayout* box = new QVBoxLayout(interestsWidget);
	box->setSpacing(5);
	box->setMargin(0);

	mILove = new InterestList(tr("I like:"));
	box->addWidget(mILove);
	mIHate = new InterestList(tr("I hate:"));
	box->addWidget(mIHate);

	mRecommendsList = new RecommendsView(split);

	mUserList = new UserListView(false, split);

	connect(museeq, SIGNAL(addedInterest(const QString& )), SLOT(gAddInterest(const QString& )));
	connect(museeq, SIGNAL(addedHatedInterest(const QString& )), SLOT(gAddHatedInterest(const QString& )));
	connect(museeq, SIGNAL(removedInterest(const QString& )), SLOT(gRemoveInterest(const QString& )));
	connect(museeq, SIGNAL(removedHatedInterest(const QString& )), SLOT(gRemoveHatedInterest(const QString& )));
	connect(museeq, SIGNAL(similarUsers(const NSimilarUsers& )), SLOT(addUsers(const NSimilarUsers&)));
	connect(museeq, SIGNAL(itemSimilarUsers(const QString&, const NItemSimilarUsers& )), SLOT(addItemUsers(const QString&, const NItemSimilarUsers&)));


}

void Interests::getGlobalRecommendations() {
	museeq->updateGlobalRecommendations();
}

void Interests::getRecommendations() {
	museeq->updateRecommendations();
}
void Interests::getSimilarUsers() {
	museeq->updateSimilarUsers();
}
void Interests::addUsers(const NSimilarUsers& _list) {

	mUserList->clear();
	QMap<QString, unsigned int>::const_iterator it = _list.begin();
	for(; it != _list.end(); ++it) {
		if(! it.key().isEmpty())
			mUserList->add( (it.key()),  (it.value()));
	}
	mUserList->resizeColumnToContents(0);
	mUserList->resizeColumnToContents(1);
	mUserList->setSortingEnabled(true);
	mUserList->sortItems(1, Qt::AscendingOrder);
}
void Interests::addItemUsers(const QString& _item, const NItemSimilarUsers& _list) {

	mUserList->clear();
	QMap<QString, unsigned int>::const_iterator it = _list.begin();
	for(; it != _list.end(); ++it)
		if(! it.key().isEmpty())
			mUserList->add( (it.key()),  (it.value()));
	mUserList->resizeColumnToContents(0);
	mUserList->resizeColumnToContents(1);
	mUserList->setSortingEnabled(true);
	mUserList->sortItems(1, Qt::AscendingOrder);
}
void Interests::gAddInterest(const QString& interest) {
	mILove->added(interest);
}

void Interests::gRemoveInterest(const QString& interest) {
	mILove->removed(interest);
}
void Interests::gAddHatedInterest(const QString& interest) {
	mIHate->added(interest);
}
void Interests::gRemoveHatedInterest(const QString& interest) {
	mIHate->removed(interest);
}
