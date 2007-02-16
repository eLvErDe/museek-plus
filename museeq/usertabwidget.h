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

#ifndef USERTABWIDGET_H
#define USERTABWIDGET_H

#include "tabwidget.h"
#include <qvbox.h>

class BuddyList;
class IgnoreList;
class BanList;
class TrustList;

class UserWidget : public QVBox {
	Q_OBJECT
public:
	UserWidget(const QString& _u, QWidget* _p = 0, const char* _n = 0)
	         : QVBox(_p, _n), mUser(_u) { };
	
	QString user() const { return mUser; };

signals:
	void encodingChanged(const QString&, const QString&);
	void highlight(int);
	
private:
	QString mUser;
};

class UserTabWidget : public TabWidget {
	Q_OBJECT
public:
	UserTabWidget(QWidget* = 0, const char* = 0);

public slots:
	void dropSlsk(const QStringList&);
	void setPage(const QString&);
	
signals:
	void newPage(const QString&);
	void encodingChanged(const QString&, const QString&);
	void highlight(int);
	
	
protected:
	UserWidget* page(const QString&, bool = false);
	virtual UserWidget* makeNewPage(const QString&);
	
private:
	BuddyList* mBuddyList;
	BanList* mBanList;
	IgnoreList* mIgnoreList;
	TrustList* mTrustList;
};

#endif // USERTABWIDGET_H
