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

#include "roomlistitem.h"
#include "roomlistview.h"
#include "util.h"

RoomListItem::RoomListItem(RoomListView* _p, const QString& _r, unsigned int _n, const QString& _s)
                 : QTreeWidgetItem(_p), mRoom(_r), mStatus(_s), mUsers(_n) {
	setText(0, mRoom);
	setText(1, QString("%1").arg(mUsers));
	setText(2, mStatus);
}

QString RoomListItem::room() const {
	return mRoom;
}

bool RoomListItem::operator<(const QTreeWidgetItem & other_) const {
  const RoomListItem * other = dynamic_cast<const RoomListItem *>(&other_);
  if (!other)
    return false;

  int col = 0;
  if(treeWidget())
    col = treeWidget()->sortColumn();

  switch(col)
  {
    case 0:
      return mRoom.toLower() < other->mRoom.toLower();
    case 1:
      return mUsers < other->mUsers;
    case 2:
      return mStatus < other->mStatus;

  }

  return false;
}
