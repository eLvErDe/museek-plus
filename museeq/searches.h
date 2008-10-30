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

#ifndef SEARCHES_H
#define SEARCHES_H

#include "museeqtypes.h"
#include "tabwidget.h"

#include <QWidget>

class QComboBox;
class QRadioButton;
class QLabel;
class QPushButton;

class TabWidget;
class Interests;
class WishList;

class SearchTabWidget : public TabWidget {
	Q_OBJECT
public:
	SearchTabWidget(QWidget* = 0, const char* = 0);

signals:
	void highlight(int);

public slots:
	void setHighlight(int, QWidget*);
	void selected(QWidget*);

};

class Searches : public QWidget {
	Q_OBJECT
public:
	Searches(QWidget* = 0, const char* = 0);

signals:
	void highlight(int);

public slots:
	void setSearchText(const QString&);
	void setUserSearchText(const QString&);
	void searchModeSelected();
	void doSearch(const QString&);
	void tabSelected(QWidget*);

protected slots:
	void doSearch();
	void setToken(const QString&, uint);
	void append(uint, const QString&, bool, uint, uint, const NFolder&);

private:
	QComboBox* mEntry,* mUserEntry;
	QPushButton* mSearch;
	QRadioButton* mGlobal, * mRooms, * mBuddies, * mUser, * mWishList;
	QLabel* mUserLabel;
	Interests* mInterests;
	WishList* mWishListView;
	SearchTabWidget* mSearchTabWidget, mWishlistTabWidget;
};

#endif // SEARCHES_H
