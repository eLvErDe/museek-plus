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

#ifndef TRANSFERS_H
#define TRANSFERS_H

#include "museeqtypes.h"

#include <QList>
#include <QWidget>

class TransferListView;
class QMenu;
class QCheckBox;
class QSpinBox;
class QUrl;
class QSplitter;

class Transfers : public QWidget {
	Q_OBJECT
public:
	Transfers(QWidget* = 0, const char* = 0);

	TransferListView* uploads() const;
	TransferListView* downloads() const;

public slots:
	void groupDownloads(bool);
	void groupUploads(bool);

protected:
	QList<QPair<QString, QString> > findSelected(TransferListView* l);
	QList<QPair<QString, QString> > findByState(TransferListView* l, uint state);
	QList<QPair<QString, QString> > findByStates(TransferListView* l, QList<uint> states);

protected slots:
	void dropSlsk(const QList<QUrl>&);

	void groupDownloadsSet(bool);
	void groupUploadsSet(bool);

	void popupUploads(const QPoint&);
	void popupDownloads(const QPoint&);

	void retrySelected();
	void updateSelected();
	void messageDownloadingUsers();
	void abortSelected();
	void clearSelected();
	void clearAwaiting();
	void clearCruft();
	void clearFinished();
	void clearAborted();
	void clearFinishedAborted();
	void clearQueued();

	void slotConfigChanged(const QString&, const QString&, const QString&);
	void setDownslots(const QString&);
	void setDownrate(const QString&);
	void setUpslots(const QString&);
	void setUprate(const QString&);
	void setSorting(bool);
	void setupUsers();
	void onClosingMuseeq();

private:
	QAction * ActionRetry, * ActionAbort, * ActionCheckPosition, * ActionClearSelected, * ActionClearFinished, * ActionClearAborted, * ActionClearAwaiting, * ActionClearCruft, * ActionClearQueued, * ActionClearFinishedAborted, *ActionMessageDownloading;
	TransferListView* mUploads,
	                * mDownloads;

	bool mPoppedUpload;
	bool mDownloadSlotsChanging, mDownloadRateChanging, mUploadSlotsChanging, mUploadRateChanging;
	bool mUpGroupingChanging;
	bool mDownGroupingChanging;
	QMenu* mTransferMenu, * mClearMenu, * mUsersMenu;
	QCheckBox* mGroupDownloads, * mGroupUploads;
	QSpinBox* mDownslots, *mDownrate, *mUpslots, *mUprate;
	QWidget * downloadsWidget, * uploadsWidget;
	QSplitter * transferSplitter;
};

#endif
