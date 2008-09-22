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
#include "searches.h"
#include "tabwidget.h"
#include "images.h"
#include "interests.h"
#include "search.h"

#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QIcon>
#include <QLayout>
#include <QLineEdit>

Searches::Searches(QWidget* parent, const char* name)
         : QWidget(parent) {

	QVBoxLayout* vbox = new QVBoxLayout(this);
	QHBoxLayout* hbox = new QHBoxLayout;
	vbox->addLayout(hbox);
	hbox->setMargin(5);
	hbox->setSpacing(5);
	mEntry = new QComboBox(this);
	mEntry->setEditable(true);
	hbox->addWidget(mEntry);
	mEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	mUserLabel =  new QLabel(tr("User:"));
	hbox->addWidget(mUserLabel);
	mUserEntry = new QComboBox(this);
	mUserEntry->setEditable(true);
	hbox->addWidget(mUserEntry);
	mUserEntry->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QIcon mSearchIcon = IMG("search-small");
	mSearch = new QPushButton(tr("Search"));
	hbox->addWidget(mSearch);
	mSearch->setIcon(mSearchIcon);
	mSearch->setText(tr("Search"));
	QGroupBox * methods = new QGroupBox(tr("Method"), this);
	vbox->addWidget(methods);
	QHBoxLayout* methodsLayout = new QHBoxLayout(methods);
	methodsLayout->setMargin(2);
	mGlobal = new QRadioButton(this);
	mGlobal->setText(tr("Global"));
	mGlobal->setChecked(true);
	methodsLayout->addWidget(mGlobal);
	mRooms = new QRadioButton(mGlobal);
	mRooms->setText(tr("Rooms"));
	methodsLayout->addWidget(mRooms);
	mBuddies = new QRadioButton(mGlobal);
	mBuddies->setText(tr("Buddies"));
	methodsLayout->addWidget(mBuddies);
	mWishList = new QRadioButton(mGlobal);
	mWishList->setText(tr("WishList"));
	methodsLayout->addWidget(mWishList);
	mUser = new QRadioButton(mGlobal);
	mUser->setText(tr("User"));
	methodsLayout->addWidget(mUser);

	searchModeSelected();
	connect(mEntry, SIGNAL(activated(const QString&)), SLOT(doSearch(const QString&)));
	connect(mEntry, SIGNAL(highlighted(const QString&)), SLOT(setSearchText(const QString&)));
	connect(mUserEntry, SIGNAL(highlighted(const QString&)), SLOT(setUserSearchText(const QString&)));
	connect(mUserEntry, SIGNAL(activated(const QString&)), SLOT(doSearch()));
	connect(mUser, SIGNAL(toggled(bool)), SLOT(searchModeSelected()));
	connect(mSearch, SIGNAL(clicked()), SLOT(doSearch()));

    mSearchTabWidget = new SearchTabWidget(this);
	mInterests = new Interests(mSearchTabWidget);
	mInterests->setDisabled(true);
	vbox->addWidget(mSearchTabWidget);
	vbox->addWidget(mInterests);
	mSearchTabWidget->addTab(mInterests, tr("*Search*"));

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
    qDebug() << "Search results from " << user;
	for(int i = 1; i < mSearchTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mSearchTabWidget->widget(i));
		if(search->hasToken(token)) {
			search->append(user, free, speed, files, r);
			return;
		}
	}
}

void Searches::setToken(const QString& query, uint token) {
	for(int i= 1; i < mSearchTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mSearchTabWidget->widget(i));
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
	mEntry->setCurrentIndex(0);

	int i;
	for(i = 1; i < mSearchTabWidget->count(); ++i) {
		Search* search = static_cast<Search*>(mSearchTabWidget->widget(i));
		if(search->query() == q) {
			mSearchTabWidget->setCurrentIndex(i);
			break;
		}
	}
	if(i == mSearchTabWidget->count()) {
		Search* s = new Search(q, mSearchTabWidget, false);
		mSearchTabWidget->addTab(s, q);

        // Highlight the search icon so that the user know that there are some new search results
		connect(s, SIGNAL(highlight(int, QWidget*)), SIGNAL(highlight(int)));

        // Highlight the tab where we have received new search results
		connect(s, SIGNAL(highlight(int, QWidget*)), mSearchTabWidget, SLOT(setHighlight(int, QWidget*)));

		mSearchTabWidget->setCurrentWidget(s);
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



SearchTabWidget::SearchTabWidget(QWidget* _p, const char* _n)
              : TabWidget(_p, _n)
{
    connect(this, SIGNAL(currentChanged(QWidget*)), SLOT(selected(QWidget*)));
}

void SearchTabWidget::setHighlight(int highlight, QWidget* widget) {
	Search * uw = static_cast<Search*>(widget);
    int pos = indexOf(uw);
	if(( currentIndex() != pos) && highlight > uw->highlighted() )
	{
		uw->setHighlighted(highlight);

		if (uw->highlighted() > 0) {
			// Icon on tab
			tabBar()->setTabIcon(pos, QIcon(IMG("online")));
		}
		if (uw->highlighted() > 1) {
			// Red tab
			tabBar()->setTabTextColor(pos, QColor(255, 0, 0));
		}
	} else if (highlight == 0) {
		tabBar()->setTabTextColor(pos, tabBar()->palette().buttonText().color());
		tabBar()->setTabIcon(pos, QIcon());
	}
}

void SearchTabWidget::selected(QWidget* searchwidget) {
	if (currentIndex() == 0)
		return;
	Search * uw = static_cast<Search*>(searchwidget);
	if(uw->highlighted() != 0) {
		uw->setHighlighted(0);
		setHighlight(uw->highlighted(), uw );
	}
}
