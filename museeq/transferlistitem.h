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

#ifndef TRANSFERLISTITEM_H
#define TRANSFERLISTITEM_H

#include <qlistview.h>
#include "museeqtypes.h"

class TransferListItem : public QListViewItem {
public:
	TransferListItem(QListView*, const QString&, const QString& = QString::null);
	TransferListItem(QListViewItem*, const QString&, const QString&);
	
	int compare(QListViewItem *, int, bool) const;
	
	void update(const NTransfer&);
	void remove(const QString&);
	void updateStats();
	
	QString user() const;
	QString path() const;
	uint state() const;
	QString error() const;
	Q_INT64 position() const;
	Q_INT64 size() const;
	uint rate() const;
	
protected:
	void updatePath();
	void update(const NTransfer&, bool);
	
private:
	uint mState, mRate, mPlaceInQueue;
	QString mUser, mPath, mError;
	Q_INT64 mPosition, mSize;
};


#endif // TRANSFERLISTITEM_H
