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

#ifndef TRANSFERS_H
#define TRANSFERS_H

#include "museeqtypes.h"

#include <qsplitter.h>

class TransferListView;
class QListViewItem;
class QPopupMenu;
class QCheckBox;
class QSpinBox;

class Transfers : public QSplitter {
	Q_OBJECT
public:
	Transfers(QWidget* = 0, const char* = 0);

	TransferListView* uploads() const;
	TransferListView* downloads() const;
	
public slots:
	void groupDownloads(bool);
	void groupUploads(bool);

protected:
	QValueList<QPair<QString, QString> > findSelected(TransferListView* l);
	QValueList<QPair<QString, QString> > findByState(TransferListView* l, uint state);

protected slots:
	void dropSlsk(const QStringList&);
	
	void groupDownloadsSet(bool);
	void groupUploadsSet(bool);

	void popupUploads(QListViewItem*, const QPoint&, int);
	void popupDownloads(QListViewItem*, const QPoint&, int);
	
	void retrySelected();
	void updateSelected();
	void abortSelected();
	void clearSelected();
	void clearAwaiting(); //added by d
	void clearCruft(); //added by d
	void clearFinished();
	void clearAborted();
	void clearFinishedAborted();
	void clearQueued();
	
	void slotConfigChanged(const QString&, const QString&, const QString&);
	void setUpslots(const QString&);
	
	void setupUsers();
	
private:
	TransferListView* mUploads,
	                * mDownloads;
	
	bool mPoppedUpload;
	bool mUploadSlotsChanging;
	bool mUpGroupingChanging;
	bool mDownGroupingChanging;
	QPopupMenu* mTransferMenu, * mClearMenu, * mUsersMenu;
	QCheckBox* mGroupDownloads, * mGroupUploads;
	QSpinBox* mUpslots;
};

#endif
