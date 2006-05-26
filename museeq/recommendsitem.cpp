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

#include "recommendsitem.h"
#include "recommendsview.h"

RecommendsItem::RecommendsItem(RecommendsView* _p, const QString& _r, unsigned int _n)
                 : QListViewItem(static_cast<QListView *>(_p)), mInterest(_r), mNum(_n) {
	setText(0, mInterest);
	setText(1, QString().sprintf("%u", mNum));
}

static int cmp(unsigned int a, unsigned int b) {
	if(a > b)
		return 1;
	if(a == b)
		return 0;
	return -1;
}

int RecommendsItem::compare(QListViewItem * i, int col, bool) const {
	RecommendsItem *r = static_cast<RecommendsItem *>(i);
	
	if(col == 0)
		return mInterest.localeAwareCompare(r->mInterest);
	else
		return cmp(r->mNum, mNum);
}

QString RecommendsItem::interest() const {
	return mInterest;
}
