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

#include "interests.h"

#include <qlistview.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qlabel.h>

#include "userlistview.h"
#include "interestlist.h"
#include "recommendsview.h"
#include "museeq.h"

Interests::Interests(QWidget* parent, const char* name)
          : QVBox(parent, name) {
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	
	QHButtonGroup* bbox = new QHButtonGroup(this);
	bbox->setFrameStyle(NoFrame);
	bbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	QLabel* w = new QLabel(tr("Interests"), bbox);
	QFont f = w->font();
	f.setBold(true);
	w->setFont(f);
	
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mGlobal = new QPushButton(tr("Global"), bbox);
	connect(museeq, SIGNAL(connectedToServer(bool)), mGlobal, SLOT(setEnabled(bool)));
	connect(mGlobal, SIGNAL(clicked()), SLOT(getGlobalRecommendations()));
	
	mRecommend = new QPushButton(tr("Recommend"), bbox);
	connect(museeq, SIGNAL(connectedToServer(bool)), mRecommend, SLOT(setEnabled(bool)));
	connect(mRecommend, SIGNAL(clicked()), SLOT(getRecommendations()));
	
	mSimilar = new QPushButton(tr("Similar"), bbox);
	connect(museeq, SIGNAL(connectedToServer(bool)), mSimilar, SLOT(setEnabled(bool)));
	connect(mSimilar, SIGNAL(clicked()), SLOT(getSimilarUsers()));

	QSplitter* split = new QSplitter(this);
	
	QVBox* box = new QVBox(split);
	box->setSpacing(5);
	split->setResizeMode(box, QSplitter::KeepSize);

	mILove = new InterestList(tr("I like:"), box);
	mIHate = new InterestList(tr("I hate:"), box);
	
	mRecommendsList = new RecommendsView(split);

	mUserList = new UserListView(false, split);
	split->setResizeMode(mUserList, QSplitter::KeepSize);

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
			mUserList->add( (it.key()),  (it.data()));
	}
}
void Interests::addItemUsers(const QString& _item, const NItemSimilarUsers& _list) {

	mUserList->clear();
	QMap<QString, unsigned int>::const_iterator it = _list.begin();
	for(; it != _list.end(); ++it)
		if(! it.key().isEmpty())
			mUserList->add( (it.key()),  (it.data()));

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
