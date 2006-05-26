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
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qhbuttongroup.h>
#include <qlineedit.h>

#include "tabwidget.h"
#include "interests.h"
#include "search.h"
#include "museeq.h"

Searches::Searches(QWidget* parent, const char* name)
         : QVBox(parent, name) {
	
	QHButtonGroup* group = new QHButtonGroup(this);
	group->setRadioButtonExclusive(true);
	group->setFrameShape(QFrame::NoFrame);
	group->setMargin(0);
	
	mEntry = new QComboBox(true, group);
	mEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mEntry->insertItem("");
	connect(mEntry, SIGNAL(activated(const QString&)), SLOT(doSearch(const QString&)));
	connect(mEntry, SIGNAL(highlighted(const QString&)), SLOT(setSearchText(const QString&)));
	
	mGlobal = new QRadioButton("Global", group);
	mRooms = new QRadioButton("Rooms", group);
	// mRooms->setEnabled(false);
	mBuddies = new QRadioButton("Buddies", group);
	group->setButton(0);
	
	mSearch = new QPushButton("Search", group);
	connect(mSearch, SIGNAL(clicked()), SLOT(doSearch()));
	
	QFrame* frame = new QFrame(this);
	frame->setFrameShape(QFrame::HLine);
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mTabWidget = new TabWidget(this);
	mInterests = new Interests(mTabWidget);
	mInterests->setDisabled(true);
	mTabWidget->addTab(mInterests, "*Search*");
	
	connect(museeq, SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)), SLOT(append(uint, const QString&, bool, uint, uint, const NFolder&)));
	connect(museeq, SIGNAL(searchToken(const QString&, uint)), SLOT(setToken(const QString&, uint)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mEntry, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mSearch, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mInterests, SLOT(setEnabled(bool)));
}

void Searches::setSearchText(const QString& text) {
	if(! text.isEmpty())
	{
		mEntry->setEditText(text);
		mEntry->lineEdit()->selectAll();
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
	else
		museeq->search(q);
}

void Searches::doSearch() {
	QString q = mEntry->currentText();
	doSearch(q);
}

