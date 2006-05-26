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

#ifndef INTERESTS_H
#define INTERESTS_H

#include <qvbox.h>
#include "museeqtypes.h"
class UserListView;
class RecommendsView;
class QListView;
class QPushButton;
class InterestList;

class Interests : public QVBox {
	Q_OBJECT
public:
	Interests(QWidget* = 0, const char* = 0);
	InterestList* mIHate;
	InterestList* mILove;

public slots:
	void getRecommendations();
	void getSimilarUsers();
	void addUsers(const NSimilarUsers& );
	void addItemUsers(const QString&, const NItemSimilarUsers& );
	void gAddInterest(const QString&);
	void gAddHatedInterest(const QString&);
	void gRemoveInterest(const QString&);
	void gRemoveHatedInterest(const QString&);


protected slots:
	void getGlobalRecommendations();


private:
	UserListView* mUserList;
	RecommendsView* mRecommendsList ;

	QPushButton* mGlobal,* mRecommend,* mSimilar;
	
};

#endif // INTERESTS_H
