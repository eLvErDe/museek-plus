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
#include "searches.h"
#include "tabwidget.h"
#include "images.h"
#include "interests.h"
#include "wishlist.h"
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

	QIcon searchIcon = IMG("search-small");
	mSearch = new QPushButton(tr("Search"));
	hbox->addWidget(mSearch);
	mSearch->setIcon(searchIcon);
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
	connect(mEntry->lineEdit(), SIGNAL(returnPressed()), SLOT(doSearch()));
	connect(mUserEntry->lineEdit(), SIGNAL(returnPressed()), SLOT(doSearch()));
	connect(mUser, SIGNAL(toggled(bool)), SLOT(searchModeSelected()));
	connect(mSearch, SIGNAL(clicked()), SLOT(doSearch()));

    mSearchTabWidget = new SearchTabWidget(this);
	mInterests = new Interests(mSearchTabWidget);
	mInterests->setDisabled(true);
	vbox->addWidget(mSearchTabWidget);
	vbox->addWidget(mInterests);
	mSearchTabWidget->addTab(mInterests, tr("*Interests*"));

	mWishListView = new WishList(mSearchTabWidget);
	mWishListView->setDisabled(true);
	vbox->addWidget(mWishListView);
	mSearchTabWidget->addTab(mWishListView, tr("*Wishlist*"));

	connect(museeq, SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&, const NFolder&)), SLOT(append(uint, const QString&, bool, uint, uint, const NFolder&, const NFolder&)));
	connect(museeq, SIGNAL(searchToken(const QString&, uint)), SLOT(setToken(const QString&, uint)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mEntry, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mSearch, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mInterests, SLOT(setEnabled(bool)));
	connect(museeq, SIGNAL(connectedToServer(bool)), mWishListView, SLOT(setEnabled(bool)));
	connect(mSearchTabWidget, SIGNAL(currentChanged(int)), SLOT(tabSelected(int)));
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

void Searches::append(uint token, const QString& user, bool free, uint speed, uint files, const NFolder& r, const NFolder& lr) {
    qDebug() << "Search results from " << user;
    int i;
	for(i = 2; i < mSearchTabWidget->count(); ++i) {
		Search* search = dynamic_cast<Search*>(mSearchTabWidget->widget(i));
		if(search && search->hasToken(token)) {
			search->append(user, free, speed, files, r, lr);
			return;
		}
	}

    // Create the tab if it's a first result from a wishlist search
	if (i == mSearchTabWidget->count()) {
        QMap<uint, QString>::const_iterator i = mWishListTokens.find(token);
        while (i != mWishListTokens.end() && i.key() == token) {
            QString query = i.value();
            if (museeq->isInWishlist(query)) {
        		Search* s = new Search(query, mSearchTabWidget);
        		mSearchTabWidget->addTab(s, query);

                // Highlight the search icon so that the user know that there are some new search results
        		connect(s, SIGNAL(highlight(int, QWidget*)), SIGNAL(highlight(int)));

                // Highlight the tab where we have received new search results
        		connect(s, SIGNAL(highlight(int, QWidget*)), mSearchTabWidget, SLOT(setHighlight(int, QWidget*)));

        		mSearchTabWidget->setCurrentWidget(s);
        		s->setToken(token);
	            s->append(user, free, speed, files, r, lr);
            }
            ++i;
        }
	}
}

void Searches::setToken(const QString& query, uint token) {
    int i;
	for(i = 2; i < mSearchTabWidget->count(); ++i) {
		Search* search = dynamic_cast<Search*>(mSearchTabWidget->widget(i));
		if(search && search->query() == QString(query)) {
			search->setToken(token);
			return;
		}
	}

    // Keep the token in memory to create the tab if needed (on first result)
    if ((i == mSearchTabWidget->count()) && museeq->isInWishlist(query)) {
        mWishListTokens[token] = query;
    }
}

void Searches::doSearch(const QString& q) {
	if(q.isEmpty())
		return;

    // Cleanup query string
    QString clean_q = q;
    clean_q = clean_q.replace(QRegExp("\\s+[-–_.!?/]\\s+"), " ");
    clean_q = clean_q.remove(QRegExp("^[-–_.!?/]\\s+"));
    clean_q = clean_q.remove(QRegExp("\\s+[-–_.!?/]$"));
    clean_q = clean_q.simplified();

	QString user;
 	if (mUser->isChecked ()) {
		if ( mUserEntry->currentText().isEmpty() )
			return;
		else {
			user = mUserEntry->currentText();
		}
	}

	int i;
	for(i = 2; i < mSearchTabWidget->count(); ++i) {
		Search* search = dynamic_cast<Search*>(mSearchTabWidget->widget(i));
		if(search && search->query() == clean_q) {
			mSearchTabWidget->setCurrentIndex(i);
			break;
		}
	}
	if(i == mSearchTabWidget->count()) {
		Search* s = new Search(clean_q, mSearchTabWidget);
		mSearchTabWidget->addTab(s, clean_q);

        // Highlight the search icon so that the user know that there are some new search results
		connect(s, SIGNAL(highlight(int, QWidget*)), SIGNAL(highlight(int)));

        // Highlight the tab where we have received new search results
		connect(s, SIGNAL(highlight(int, QWidget*)), mSearchTabWidget, SLOT(setHighlight(int, QWidget*)));

		mSearchTabWidget->setCurrentWidget(s);
	}

	mEntry->setEditText("");

	if(mBuddies->isChecked())
		museeq->buddySearch(clean_q);
	else if (mRooms->isChecked ())
		museeq->roomSearch (clean_q);
	else if (mGlobal->isChecked ())
		museeq->search(clean_q);
	else if (mUser->isChecked ()) {
		if ( ! user.isEmpty() )
			museeq->userSearch(user, clean_q);
	} else if (mWishList->isChecked ())
		museeq->wishListSearch(clean_q);
}

void Searches::doSearch() {
	QString q = mEntry->currentText();
	doSearch(q);
}

void Searches::tabSelected(int curIndex) {
	if (curIndex <= 1)
		return;
	Search * uw = dynamic_cast<Search*>(mSearchTabWidget->currentWidget());
	if (uw)
        setSearchText(uw->query());
}


SearchTabWidget::SearchTabWidget(QWidget* _p, const char* _n)
              : TabWidget(_p, _n)
{
    setLastProtected(1);
    connect(this, SIGNAL(currentChanged(int)), SLOT(selected(int)));
}

void SearchTabWidget::setHighlight(int highlight, QWidget* widget) {
	Search * uw = dynamic_cast<Search*>(widget);
	if (!uw)
        return;

    int pos = indexOf(uw);
	if(( currentIndex() != pos) && highlight > uw->highlighted() )
	{
		uw->setHighlighted(highlight);

		if (uw->highlighted() > 0) {
			// Icon on tab
			tabBar()->setTabIcon(pos, QIcon(IMG("new-element")));
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

void SearchTabWidget::selected(int curIndex) {
	if (curIndex <= 1)
		return;
	Search * uw = dynamic_cast<Search*>(currentWidget());
	if(uw && uw->highlighted() != 0) {
		uw->setHighlighted(0);
		setHighlight(uw->highlighted(), uw );
	}
}
