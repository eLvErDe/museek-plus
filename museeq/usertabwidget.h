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

#ifndef USERTABWIDGET_H
#define USERTABWIDGET_H

#include "tabwidget.h"

#include <QWidget>

class BuddyList;
class IgnoreList;
class BanList;
class TrustList;
class QUrl;

class UserWidget : public QWidget {
	Q_OBJECT
public:
	UserWidget(const QString& _u, QWidget* _p = 0, const char* _n = 0)
	         : QWidget(_p), mUser(_u), mHighlight(0) { };

	QString user() const { return mUser; };
	int highlighted() const {return mHighlight;};
public slots:
	void setHighlight(int i);
	void selected();
	void setHighlighted(int newH) {mHighlight = newH;};
signals:
	void encodingChanged(const QString&, const QString&);
	void highlight(int);

private:
	QString mUser;
	int mHighlight;
};

class UserTabWidget : public TabWidget {
	Q_OBJECT
public:
	UserTabWidget(QWidget* = 0, const char* = 0);

public slots:
	void dropSlsk(const QList<QUrl>&);
	void setPage(const QString&);
	void setHighlight(int pos, int highlight);

signals:
	void newPage(const QString&);
	void encodingChanged(const QString&, const QString&);
	void highlight(int);

protected:
	UserWidget* page(const QString&, bool = false);
	virtual UserWidget* makeNewPage(const QString&);
protected slots:
	void doCurrentChanged(QWidget *);
private:

	BuddyList* mBuddyList;
	BanList* mBanList;
	IgnoreList* mIgnoreList;
	TrustList* mTrustList;
};

#endif // USERTABWIDGET_H
