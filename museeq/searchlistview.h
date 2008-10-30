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

#ifndef SEARCHLISTVIEW_H
#define SEARCHLISTVIEW_H

#include "museeqtypes.h"

#include <QTreeWidget>

class QMenu;
class SearchFilter;
class QMouseEvent;

class SearchListView : public QTreeWidget {
	Q_OBJECT
public:
	SearchListView(SearchFilter*, QWidget* = 0, const char* = 0);

public slots:
	void append(const QString&, bool, uint, uint, const NFolder&);

protected:
	void setupUsers();

protected slots:
	void downloadFiles();
	void downloadFilesTo();
	void downloadFolders();
	void slotContextMenu(const QPoint&);
	void slotActivate(QTreeWidgetItem*,  int);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void headerClicked(int);

private:
	SearchFilter* mFilter;
	quint64 mN;
	QMenu* mPopupMenu, * mUsersMenu;
	QPoint mDragStartPosition;
};

class SearchListItem : public QTreeWidgetItem {
public:
	SearchListItem(QTreeWidget*, quint64, const QString&, bool, uint, uint, const QString&, quint64, uint, uint, bool);

	quint64 n() const { return mN; };
	QString user() const { return mUser; }
	QString dir() const { return mDir; }
	QString path() const { return mPath; }
	uint speed() const { return mSpeed; }
	uint inQueue() const { return mInQueue; }
	uint length() const { return mLength; }
	uint bitrate() const { return mBitrate; }
	quint64 size() const { return mSize; }
	bool vbr() const { return mVBR; }
	bool freeSlot() const { return mFree; }
	bool operator<(const QTreeWidgetItem & other) const;

private:
	quint64 mN;
	QString mUser, mPath, mFilename, mDir;
	uint mSpeed, mInQueue, mLength, mBitrate;
	quint64 mSize;
	bool mFree, mVBR;
};

#endif // SEARCHLISTVIEW_H
