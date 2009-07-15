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

#include "recommendsitem.h"
#include "recommendsview.h"
#include "util.h"

RecommendsItem::RecommendsItem(RecommendsView* _p, const QString& _r, int _n)
                 : QTreeWidgetItem(static_cast<QTreeWidget *>(_p)), mInterest(_r), mNum(_n) {
	setText(0, mInterest);
	setText(1, QString("%1").arg(mNum));
}

QString RecommendsItem::interest() const {
	return mInterest;
}

bool RecommendsItem::operator<(const QTreeWidgetItem & other_) const {
  const RecommendsItem * other = dynamic_cast<const RecommendsItem *>(&other_);
  if (!other)
    return false;

  int col = 0;
  if(treeWidget())
    col = treeWidget()->sortColumn();

  switch(col)
  {
    case 0:
      return interest() < other->interest();
    case 1:
      return mNum < other->mNum;

  }

  return false;
}
