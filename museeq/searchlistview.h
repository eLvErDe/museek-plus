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

#ifndef SEARCHLISTVIEW_H
#define SEARCHLISTVIEW_H

#include <qlistview.h>
#include "museeqtypes.h"

class QPopupMenu;
class SearchFilter;

class SearchListView : public QListView {
	Q_OBJECT
public:
	SearchListView(SearchFilter*, QWidget* = 0, const char* = 0);
	
public slots:
	void append(const QString&, bool, uint, uint, const NFolder&);

protected:
	void setupUsers();
	QDragObject* dragObject();
	
protected slots:
	void popupMenu(QListViewItem*, const QPoint&, int);
	void downloadFiles();
	void downloadFolders();
	
private:
	SearchFilter* mFilter;
	Q_UINT64 mN;
	QPopupMenu* mPopupMenu, * mUsersMenu;
};

class SearchListItem : public QListViewItem {
public:
	SearchListItem(QListView*, Q_UINT64, const QString&, bool, uint, uint, const QString&, Q_INT64, uint, uint, bool);
	
	Q_UINT64 n() const { return mN; };
	QString user() const { return mUser; }
	QString dir() const { return mDir; }
	QString path() const { return mPath; }
	uint speed() const { return mSpeed; }
	uint inQueue() const { return mInQueue; }
	uint length() const { return mLength; }
	uint bitrate() const { return mBitrate; }
	Q_INT64 size() const { return mSize; }
	bool vbr() const { return mVBR; }
	bool freeSlot() const { return mFree; }

protected:
	int compare(QListViewItem*, int, bool) const;
	
private:
	Q_UINT64 mN;
	QString mUser, mPath, mFilename, mDir;
	uint mSpeed, mInQueue, mLength, mBitrate;
	Q_INT64 mSize;
	bool mFree, mVBR;
};

#endif // SEARCHLISTVIEW_H
