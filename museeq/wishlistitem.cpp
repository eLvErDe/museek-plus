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

#include "wishlistitem.h"
#include "wishlistview.h"

#include <QDateTime>

WishListItem::WishListItem(WishListView* _p, const QString& _r, unsigned int _n)
                 : QTreeWidgetItem(_p), mQuery(_r), mLastSearched(_n) {
    QString strLast("Never");
    if (mLastSearched > 0) {
        QDateTime _t;
        _t.setTime_t(mLastSearched);
        strLast = _t.toString();
    }

	setText(0, mQuery);
	setText(1, strLast);
}

QString WishListItem::query() const {
	return mQuery;
}

void WishListItem::setLastSearched(uint lastSearched) {
    mLastSearched = lastSearched;

    QString strLast("Never");
    if (mLastSearched > 0) {
        QDateTime _t;
        _t.setTime_t(mLastSearched);
        strLast = _t.toString();
    }
	setText(1, strLast);
}

bool WishListItem::operator<(const QTreeWidgetItem & other_) const {
  const WishListItem * other = dynamic_cast<const WishListItem *>(&other_);
  if (!other)
    return false;

  int col = 0;
  if(treeWidget())
    col = treeWidget()->sortColumn();

  switch(col)
  {
    case 0:
      return mQuery.toLower() < other->mQuery.toLower();
    case 1:
      return mLastSearched < other->mLastSearched;

  }

  return false;
}
