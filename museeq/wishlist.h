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

#ifndef WISHLIST_H
#define WISHLIST_H

#include "museeqtypes.h"

#include <QWidget>

class QLineEdit;
class WishListView;
class QShowEvent;
class QPushButton;

class WishList : public QWidget {
	Q_OBJECT
public:
	WishList(QWidget* = 0, const char* = 0);

protected:
	void showEvent(QShowEvent*);

protected slots:
	void slotAddWish();

public slots:
	void added(const QString&, uint);
	void removed(const QString&);

private:
	QLineEdit *mEntry;
	WishListView *mWishList;
	QPushButton * mAdd;
};

#endif // WISHLIST_H
