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

#ifndef BROWSER_H
#define BROWSER_H

#include "usertabwidget.h"
#include "museeqtypes.h"

#include <qlistview.h>

class QLineEdit;
class FolderListView;
class FileListView;
class SharesData;
class QPopupMenu;
class QPushButton;
class QLabel;

class Browser : public UserWidget {
	Q_OBJECT
public:
	Browser(const QString&, QWidget* = 0, const char* = 0);

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
	QString mUser;
};


class FolderListView : public QListView {
	Q_OBJECT
public:
	FolderListView(const QString&, QWidget* = 0, const char* = 0);
	~FolderListView();
	SharesData* shares() const { return mShares; }
public slots:
	void setShares(const NShares&);
	void show(const QStringList&);
signals:
	void currentChanged(const QString&, const NFolder&);
protected slots:
	void doCurrentChanged(QListViewItem*);
	void doPopupMenu(QListViewItem *, const QPoint&, int);
	void doDownloadFolder();
	void doCopyURL();
protected:
	QDragObject* dragObject();
private:
	QString mUser;
	SharesData* mShares;
	QPopupMenu* mPopupMenu;
	QListViewItem* mPopped;
};

	
class FileListView : public QListView {
	Q_OBJECT
public:
	FileListView(const QString&, QWidget* = 0, const char* = 0);
public slots:
	void setFiles(const QString&, const NFolder&);
	void match(const QString&);
protected:
	QDragObject* dragObject();
protected slots:
	void doPopupMenu(QListViewItem *, const QPoint&, int);
	void doDownloadFiles();
	void doUploadFiles();
	void doCopyURL();
private:
	QString mUser;
	QString mPath;
	QPopupMenu* mPopupMenu;
};

#endif // BROWSER_H
