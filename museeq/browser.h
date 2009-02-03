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

#ifndef BROWSER_H
#define BROWSER_H

#include "usertabwidget.h"
#include "museeqtypes.h"

#include <QTreeWidget>

class FolderListItem;
class FolderListView;
class FileListView;
class SharesData;
class QPushButton;
class QLabel;

class Browser : public UserWidget {
	Q_OBJECT
public:
	Browser(const QString&, QWidget* = 0, const char* = 0);
	bool isLoading() {return mLoading;};

public slots:
	void doSearch();
	void setShares(const NShares&);
	void getShares();

private:
	NShares mShares;
	NShares::const_iterator mCurrentResult;
	QString mQuery;
	QLineEdit* mEntry;
	FolderListView* mFolders;
	FileListView* mFiles;
	QLabel* mFileCount;
	QPushButton* mRefresh;
	QPushButton* mSearchButton;
	QString mUser;
	bool mLoading;
};


class FolderListView : public QTreeWidget {
	Q_OBJECT

public:
	FolderListView(const QString&, QWidget* = 0, const char* = 0);
	~FolderListView();
	QString parentPath(const QString&);

public slots:
	void setShares(const NShares&);
	void show(const QStringList&);
	FolderListItem * findParent(const QStringList&);

signals:
	void currentChanged(const QString&, const NFolder&);

protected slots:
	void doCurrentChanged(QTreeWidgetItem*, QTreeWidgetItem*);
	void doPopupMenu(QTreeWidgetItem *, const QPoint&, int);
	void doDownloadFolder();
	void doDownloadFolderTo();
	void doUploadFolder();
	void doCopyURL();
	void slotContextMenu(const QPoint&);
	void slotActivate(QTreeWidgetItem*, int);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

private:
	QString mUser;
	SharesData* mShares;
	QMenu* mPopupMenu;
	QTreeWidgetItem* mPopped;
	QPoint mDragStartPosition;
};


class FileListView : public QTreeWidget {
	Q_OBJECT
public:
	FileListView(const QString&, QWidget* = 0, const char* = 0);

public slots:
	void setFiles(const QString&, const NFolder&);
	void match(const QString&);

protected slots:
	void doPopupMenu(QTreeWidgetItem *, const QPoint&, int);
	void doDownloadFiles();
	void doDownloadFilesTo();
	void doUploadFiles();
	void doCopyURL();
	void slotContextMenu(const QPoint&);
	void slotActivate(QTreeWidgetItem*, int);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);

private:
	QString mUser;
	QString mPath;
	QMenu* mPopupMenu;
	QPoint mDragStartPosition;
};

#endif // BROWSER_H
