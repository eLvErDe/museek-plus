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

#include "searches.h"

#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qhbuttongroup.h>
#include <qlineedit.h>
#include <qiconset.h>
#include "tabwidget.h"
#include "images.h"
#include "interests.h"
#include "search.h"
#include "museeq.h"

Searches::Searches(QWidget* parent, const char* name)
         : QVBox(parent, name) {
	

	QHBox* hbox = new QHBox(this);
	hbox->setMargin(5);
	hbox->setSpacing(5);
	mEntry = new QComboBox(true, hbox);
	mEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mEntry->insertItem("");

	mUserLabel =  new QLabel(tr("User:"), hbox);
	mUserEntry = new QComboBox(true, hbox);
	mUserEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mUserEntry->insertItem("");

	QIconSet mSearchIcon = IMG("search-small");
	mSearch = new QPushButton(tr("Search"), hbox);
	mSearch->setIconSet(mSearchIcon);
// 	mSearch->setText(tr("Search"));
	QHButtonGroup* group = new QHButtonGroup(tr("Method"), this);
	group->setRadioButtonExclusive(true);
// 	group->setFrameShape(QFrame::NoFrame);
	group->setMargin(0);
	mGlobal = new QRadioButton(tr("Global"), group);
	mRooms = new QRadioButton(tr("Rooms"), group);
	mBuddies = new QRadioButton(tr("Buddies"), group);
	mWishList = new QRadioButton(tr("WishList"), group);
	mUser = new QRadioButton(tr("User"), group);
	group->setButton(0);
	searchModeSelected();	
	connect(mEntry, SIGNAL(activated(const QString&)), SLOT(doSearch(const QString&)));
	connect(mEntry, SIGNAL(highlighted(const QString&)), SLOT(setSearchText(const QString&)));
	connect(mUserEntry, SIGNAL(highlighted(const QString&)), SLOT(setUserSearchText(const QString&)));	
	connect(mUserEntry, SIGNAL(activated(const QString&)), SLOT(doSearch()));
	connect(mUser, SIGNAL(toggled(bool)), SLOT(searchModeSelected()));
	connect(mSearch, SIGNAL(clicked()), SLOT(doSearch()));


	


	QFrame* frame = new QFrame(this);
	frame->setFrameShape(QFrame::HLine);
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mTabWidget = new TabWidget(this);
	mInterests = new Interests(mTabWidget);
	mInterests->setDisabled(true);
	mTabWidget->addTab(mInterests, tr("*Search*"));
	
	connect(museeq, SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)), SLOT(append(uint, const QString&, bool, uint, uint, const NFolder&)));
	connect(museeq, SIGNAL(searchToken(const QString&, uint)), SLOT(setToken(const QString&, uint)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mEntry, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mSearch, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mInterests, SLOT(setEnabled(bool)));
}

void Searches::searchModeSelected() {
	if (mUser->isChecked ()) {
		mUserEntry->setDisabled(false);
		mUserEntry->show();
		mUserLabel->show();
	} else {
		mUserEntry->setDisabled(true);
		mUserEntry->hide();
		mUserLabel->hide();
	}
}

void Searches::setSearchText(const QString& text) {
	if(! text.isEmpty())
	{
		mEntry->setEditText(text);
		mEntry->lineEdit()->selectAll();
	}
}
void Searches::setUserSearchText(const QString& text) {
	if(! text.isEmpty())
	{
		mUserEntry->setEditText(text);
		mUserEntry->lineEdit()->selectAll();
	}
}
void Searches::append(uint token, const QString& user, bool free, uint speed, uint files, const NFolder& r) {
	for(int i = 1; i < mTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mTabWidget->page(i));
		if(search->hasToken(token)) {
			search->append(user, free, speed, files, r);
			return;
		}
	}
}

void Searches::setToken(const QString& query, uint token) {
	for(int i= 1; i < mTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mTabWidget->page(i));
		if(search->query() == QString(query))
			search->setToken(token);
	}
}

void Searches::doSearch(const QString& q) {
	if(q.isEmpty())
		return;
	QString user;
 	if (mUser->isChecked ()) {
		if ( mUserEntry->currentText().isEmpty() )
			return;
		else {
			user = mUserEntry->currentText();
		}
	}
	mEntry->setCurrentItem(0);
	
	int i;
	for(i = 1; i < mTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mTabWidget->page(i));
		if(search->query() == q) {
			mTabWidget->setCurrentPage(i);
			break;
		}
	}
	if(i == mTabWidget->count()) {
		Search* s = new Search(q, mTabWidget);
		connect(s, SIGNAL(highlight(int)), SIGNAL(highlight(int)));
		Tab* new_tab = new Tab(mTabWidget, q);
		connect(s, SIGNAL(highlight(int)), new_tab, SLOT(setHighlight(int)));
		mTabWidget->addTab(s, new_tab);
		mTabWidget->showPage(s);
	}
	
	if(mBuddies->isChecked())
		museeq->buddySearch(q);
	else if (mRooms->isChecked ())
		museeq->roomSearch (q);
	else if (mGlobal->isChecked ())
		museeq->search(q);
	else if (mUser->isChecked ()) {
		if ( ! user.isEmpty() )
			museeq->userSearch(user, q);
	} else if (mWishList->isChecked ())
		museeq->wishListSearch(q);
}

void Searches::doSearch() {
	QString q = mEntry->currentText();
	doSearch(q);
}

