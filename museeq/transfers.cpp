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

#include "transfers.h"
#include "transferlistview.h"
#include "transferlistitem.h"
#include "museeq.h"
#include "usermenu.h"
#include "mainwin.h"

#include <QLabel>
#include <QCheckBox>
#include <QUrl>
#include <QMenu>
#include <QSpinBox>
#include <QFrame>
#include <QSplitter>
#include <QLayout>
#include <QSettings>
#include <QInputDialog>

Transfers::Transfers(QWidget* _p, const char* _n)
          : QWidget(_p) {


	QVBoxLayout *MainBox = new QVBoxLayout(this);
	MainBox->setMargin(0);
	MainBox->setSpacing(0);
	transferSplitter = new QSplitter(this);
	MainBox->addWidget(transferSplitter);
	downloadsWidget = new QWidget(transferSplitter);
	uploadsWidget = new QWidget(transferSplitter);
	transferSplitter->setOrientation(Qt::Vertical);

    if (museeq->settings()->value("saveTransfersLayout", false).toBool()) {
        QString optionName = "transferSplitter_Layout";
        transferSplitter->restoreState(museeq->settings()->value(optionName).toByteArray());
    }

	// Downloads
	QVBoxLayout *downloadVbox = new QVBoxLayout(downloadsWidget);
	downloadVbox->setMargin(0);
	downloadVbox->setSpacing(3);
	QHBoxLayout *downloadHbox = new QHBoxLayout;
	downloadVbox->addLayout(downloadHbox);
	QLabel * downloadsLabel = new QLabel(tr("Downloads:"), downloadsWidget);
	downloadsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	downloadHbox->addWidget(downloadsLabel);

	mDownrate = new QSpinBox(downloadsWidget);
	mDownrate->setMaximum(9999);
	downloadHbox->addWidget(mDownrate);
	QLabel * downrateLabel = new QLabel(tr("KiB/s"), downloadsWidget);
	downloadHbox->addWidget(downrateLabel);
	connect(mDownrate, SIGNAL(valueChanged(const QString&)), SLOT(setDownrate(const QString&)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

	QFrame* sep = new QFrame(downloadsWidget);
	downloadHbox->addWidget(sep);
	sep->setFrameShape(QFrame::VLine);
	sep->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	mDownslots = new QSpinBox(downloadsWidget);
	downloadHbox->addWidget(mDownslots);
	QLabel * downslotsLabel = new QLabel(tr("slots"), downloadsWidget);
	downloadHbox->addWidget(downslotsLabel);
	connect(mDownslots, SIGNAL(valueChanged(const QString&)), SLOT(setDownslots(const QString&)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

	QFrame* sep2 = new QFrame(downloadsWidget);
	downloadHbox->addWidget(sep2);
	sep2->setFrameShape(QFrame::VLine);
	sep2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	mGroupDownloads = new QCheckBox(tr("Group by user"), downloadsWidget);
	downloadHbox->addWidget(mGroupDownloads);
	connect(mGroupDownloads, SIGNAL(toggled(bool)), SLOT(groupDownloadsSet(bool)));

	mDownloads = new TransferListView(true, downloadsWidget, "downloads");
 	downloadVbox->addWidget(mDownloads);
	mDownloads->setAcceptDrops(true);
	connect(museeq, SIGNAL(downloadUpdated(const NTransfer&)), mDownloads, SLOT(update(const NTransfer&)));
	connect(museeq, SIGNAL(downloadRemoved(const QString&, const QString&)), mDownloads, SLOT(remove(const QString&, const QString&)));
	connect(mDownloads, SIGNAL(dropSlsk(const QList<QUrl>&)), SLOT(dropSlsk(const QList<QUrl>&)));
	// Uploads
	QVBoxLayout *uploadVbox = new QVBoxLayout(uploadsWidget);
	uploadVbox->setMargin(0);
	uploadVbox->setSpacing(3);
	QHBoxLayout *uploadHbox = new QHBoxLayout;
	uploadVbox->addLayout(uploadHbox);

	uploadHbox->setSpacing(5);
	QLabel * uploadsLabel = new QLabel(tr("Uploads:"));
	uploadsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	uploadHbox->addWidget(uploadsLabel);

	mUprate = new QSpinBox(uploadsWidget);
	mUprate->setMaximum(9999);
	uploadHbox->addWidget(mUprate);
	QLabel * uprateLabel = new QLabel(tr("KiB/s"), uploadsWidget);
	uploadHbox->addWidget(uprateLabel);
	connect(mUprate, SIGNAL(valueChanged(const QString&)), SLOT(setUprate(const QString&)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

	QFrame* sep3 = new QFrame(uploadsWidget);
	uploadHbox->addWidget(sep3);
	sep3->setFrameShape(QFrame::VLine);
	sep3->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	mUpslots = new QSpinBox(uploadsWidget);
	uploadHbox->addWidget(mUpslots);
	QLabel * upslotsLabel = new QLabel(tr("slots"), uploadsWidget);
	uploadHbox->addWidget(upslotsLabel);
	connect(mUpslots, SIGNAL(valueChanged(const QString&)), SLOT(setUpslots(const QString&)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

	QFrame* sep4 = new QFrame(uploadsWidget);
	uploadHbox->addWidget(sep4);
	sep4->setFrameShape(QFrame::VLine);
	sep4->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	mGroupUploads = new QCheckBox(tr("Group by user"), uploadsWidget);
	uploadHbox->addWidget(mGroupUploads);
	connect(mGroupUploads, SIGNAL(toggled(bool)), SLOT(groupUploadsSet(bool)));

	mUploads = new TransferListView(false, uploadsWidget, "uploads");
	uploadVbox->addWidget(mUploads);
	connect(museeq, SIGNAL(uploadUpdated(const NTransfer&)), mUploads, SLOT(update(const NTransfer&)));
	connect(museeq, SIGNAL(uploadRemoved(const QString&, const QString&)), mUploads, SLOT(remove(const QString&, const QString&)));

	mTransferMenu = new QMenu(this);

	ActionRetry = new QAction(tr("Retry"), this);
	connect(ActionRetry, SIGNAL(triggered()), this, SLOT(retrySelected()));
	mTransferMenu->addAction(ActionRetry);

	ActionAbort = new QAction(tr("Abort"), this);
	connect(ActionAbort, SIGNAL(triggered()), this, SLOT(abortSelected()));
	mTransferMenu->addAction(ActionAbort);


	ActionCheckPosition = new QAction(tr("Check place"), this);
	connect(ActionCheckPosition, SIGNAL(triggered()), this, SLOT(updateSelected()));
	mTransferMenu->addAction(ActionCheckPosition);

	ActionMessageDownloading = new QAction(tr("Message downloading users"), this);
	connect(ActionMessageDownloading, SIGNAL(triggered()), this, SLOT(messageDownloadingUsers()));
	mTransferMenu->addAction(ActionMessageDownloading);

	mClearMenu = mTransferMenu->addMenu(tr("Clear"));

	ActionClearSelected = new QAction(tr("Selected"), this);
	connect(ActionClearSelected, SIGNAL(triggered()), this, SLOT(clearSelected()));
	mClearMenu->addAction(ActionClearSelected);

	mClearMenu->addSeparator();

	ActionClearFinished = new QAction(tr("Finished"), this);
	connect(ActionClearFinished, SIGNAL(triggered()), this, SLOT(clearFinished()));
	mClearMenu->addAction(ActionClearFinished);

	ActionClearAborted = new QAction(tr("Aborted"), this);
	connect(ActionClearAborted, SIGNAL(triggered()), this, SLOT(clearAborted()));
	mClearMenu->addAction(ActionClearAborted);

	ActionClearAwaiting = new QAction(tr("Offline"), this);
	connect(ActionClearAwaiting, SIGNAL(triggered()), this, SLOT(clearAwaiting()));
	mClearMenu->addAction(ActionClearAwaiting);

	ActionClearCruft = new QAction(tr("Cruft"), this);
	connect(ActionClearCruft, SIGNAL(triggered()), this, SLOT(clearCruft()));
	mClearMenu->addAction(ActionClearCruft);

	ActionClearFinishedAborted = new QAction(tr("Finished / aborted"), this);
	connect(ActionClearFinishedAborted, SIGNAL(triggered()), this, SLOT(clearFinishedAborted()));
	mClearMenu->addAction(ActionClearFinishedAborted);

	ActionClearQueued = new QAction(tr("Queued"), this);
	connect(ActionClearQueued, SIGNAL(triggered()), this, SLOT(clearQueued()));
	mClearMenu->addAction(ActionClearQueued);


	mTransferMenu->addSeparator();
	mUsersMenu = mTransferMenu->addMenu(tr("Users"));

	connect(mDownloads, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(popupDownloads(const QPoint&)));
	connect(mUploads, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(popupUploads(const QPoint&)));
	connect(museeq, SIGNAL(transfersSorting(bool)), SLOT(setSorting(bool)));

	mUpGroupingChanging = mDownGroupingChanging = true;
    groupDownloads(museeq->settings()->value("groupDownloads", false).toBool());
    groupUploads(museeq->settings()->value("groupUploads", false).toBool());
	mUpGroupingChanging = mDownGroupingChanging = false;

	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void Transfers::onClosingMuseeq() {
    if (museeq->settings()->value("saveTransfersLayout", false).toBool()) {
        QString optionName = "transferSplitter_Layout";
        museeq->settings()->setValue(optionName, transferSplitter->saveState());
    }
}

TransferListView* Transfers::uploads() const {
	return mUploads;
}

TransferListView* Transfers::downloads() const {
	return mDownloads;
}

void Transfers::setSorting(bool on) {
	if(on) {
		mDownloads->setSortingEnabled(on);
		mUploads->setSortingEnabled(on);
	} else {
		mDownloads->setSortingEnabled(on);
		mUploads->setSortingEnabled(on);
	}
	mDownloads->updateParentsStats();
	mUploads->updateParentsStats();
}
void Transfers::groupDownloadsSet(bool on) {
	if (mDownGroupingChanging)
		return;

    museeq->settings()->setValue("groupDownloads", on);
    mDownGroupingChanging = true;
    groupDownloads(on);
    mDownGroupingChanging = false;

}

void Transfers::groupDownloads(bool on) {

	TransferListView::GroupMode m = on ? TransferListView::User : TransferListView::None;
	if(mDownloads->groupMode() == m)
		return;
	mDownloads->setGroupMode(m);
	if(on != mGroupDownloads->isChecked())
		mGroupDownloads->toggle();
}

void Transfers::groupUploadsSet(bool on) {
	if (mUpGroupingChanging)
		return;

    museeq->settings()->setValue("groupUploads", on);
    mUpGroupingChanging = true;
    groupUploads(on);
    mUpGroupingChanging = false;
}

void Transfers::groupUploads(bool on) {

	TransferListView::GroupMode m = on ? TransferListView::User : TransferListView::None;
	if(mUploads->groupMode() == m)
		return;

	mUploads->setGroupMode(m);

	if(on != mGroupUploads->isChecked())
		mGroupUploads->toggle();
}

void Transfers::dropSlsk(const QList<QUrl>& l) {
	QList<QUrl>::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url = *it;
		if(url.scheme() == "slsk" && !url.host().isEmpty() && !url.path().isEmpty()) {
            // Try to find a size
			qint64 size = 0;
            bool ok;
            size = url.password().toLongLong(&ok);
            if(! ok)
				size = 0;

            // Find the user name
            QString user = url.userName();
			if (user.isEmpty())
                user = url.host();

            // Find the path (folder or file)
			QString path = url.path().replace("/", "\\");

			if(path.right(1) == "\\")
				museeq->downloadFolder(user, QString(path));
			else
				museeq->downloadFile(user, QString(path), size);
		}
	}
}

void Transfers::setupUsers() {
	mUsersMenu->clear();

	QStringList users;
	QTreeWidgetItemIterator it(mPoppedUpload ? mUploads : mDownloads, QTreeWidgetItemIterator::Selected );
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if(item && !users.contains(item->user()))
		{
			users << item->user();
		}
	}

    int numusers = users.size();
    mTransferMenu->removeAction(mUsersMenu->menuAction());
    delete mUsersMenu;
    if (numusers <= 0) {
        mUsersMenu = mTransferMenu->addMenu(tr("Users"));
        return;
    }
    else if (numusers == 1) {
        mUsersMenu = new Usermenu(mTransferMenu);
        dynamic_cast<Usermenu*>(mUsersMenu)->setup(users.first());
        QAction * usermenu = mTransferMenu->addMenu(mUsersMenu);
        usermenu->setText(tr("User '%1'").arg(users.first()));
    }
    else {
        QStringListIterator usersIt(users);
        mUsersMenu = mTransferMenu->addMenu(tr("Users"));
        while (usersIt.hasNext()) {
            QString username = usersIt.next();
            Usermenu *m = new Usermenu(mUsersMenu);
            m->setup(username);
            QAction * usermenu = mUsersMenu->addMenu(static_cast<QMenu*>(m));
            usermenu->setText(username);
        }
    }
}

void Transfers::popupUploads(const QPoint& pos) {
    bool hasItems = (mUploads->topLevelItemCount() > 0);
    bool hasSelectedItems = (mUploads->selectedItems().size() > 0);

	ActionRetry->setEnabled(hasSelectedItems);
	ActionAbort->setEnabled(hasSelectedItems);
	ActionCheckPosition->setVisible(false);
	ActionClearSelected->setEnabled(hasSelectedItems);
	mClearMenu->setEnabled(hasItems);
	ActionMessageDownloading->setEnabled(hasItems);
	ActionMessageDownloading->setVisible(true);
	mPoppedUpload = true;
	setupUsers();
	mUsersMenu->setEnabled(hasSelectedItems);
	mTransferMenu->exec(mUploads->mapToGlobal(pos));
}

void Transfers::popupDownloads(const QPoint& pos) {
    bool hasItems = (mDownloads->topLevelItemCount() > 0);
    bool hasSelectedItems = (mDownloads->selectedItems().size() > 0);

	ActionRetry->setEnabled(hasSelectedItems);
	ActionAbort->setEnabled(hasSelectedItems);
	ActionCheckPosition->setVisible(true);
	ActionCheckPosition->setEnabled(hasSelectedItems);
	ActionClearSelected->setEnabled(hasSelectedItems);
	mClearMenu->setEnabled(hasItems);
	ActionMessageDownloading->setEnabled(false);
	ActionMessageDownloading->setVisible(false);
	mPoppedUpload = false;
	setupUsers();
	mUsersMenu->setEnabled(hasSelectedItems);
	mTransferMenu->exec(mDownloads->mapToGlobal(pos));
}

QList<QPair<QString, QString> > Transfers::findSelected(TransferListView* l) {
	QList<QPair<QString, QString> > items;
	QTreeWidgetItemIterator it(mPoppedUpload ? mUploads : mDownloads, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if (!item)
            continue;

		if (!item->path().isEmpty())
            items += QPair<QString, QString>(item->user(), item->path());
        else {
            // We would like to delete a group of transfers
            int numChildren = item->childCount();
            for(int childId = 0; childId < numChildren; childId++) {
                TransferListItem* child = dynamic_cast<TransferListItem*>(item->child(childId));
                if (child)
                    items += QPair<QString, QString>(child->user(), child->path());
            }
        }
	}
	return items;
}

void Transfers::clearSelected() {
	QList<QPair<QString, QString> > items = findSelected(mPoppedUpload ? mUploads : mDownloads);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearAwaiting() {  // added by d
	QList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 10);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearCruft() {  // added by d
    QList<uint> states;
    states.append(10);
    states.append(13);
    states.append(12);
    states.append(11);
    states.append(0);
	QList<QPair<QString, QString> > items = findByStates(mPoppedUpload ? mUploads : mDownloads, states);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::abortSelected() {
	QList<QPair<QString, QString> > items = findSelected(mPoppedUpload ? mUploads : mDownloads);
	if(mPoppedUpload)
		museeq->abortUploads(items);
	else
		museeq->abortDownloads(items);
}

void Transfers::retrySelected() {
	QList<QPair<QString, QPair<QString, qint64> > > items;
	QTreeWidgetItemIterator it(mPoppedUpload ? mUploads : mDownloads, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if (item)
            items += QPair<QString, QPair<QString, qint64> >(item->user(), QPair<QString, qint64>(item->path(), item->size()));
	}

	QList<QPair<QString, QPair<QString, qint64> > >::iterator sit = items.begin();
	for(; sit != items.end(); ++sit) {
		if(mPoppedUpload)
			museeq->uploadFile((*sit).first, (*sit).second.first);
		else
			museeq->downloadFile((*sit).first, (*sit).second.first, (*sit).second.second);
	}
}

void Transfers::updateSelected() {
	QList<QPair<QString, QString> > items;
	QTreeWidgetItemIterator it(mDownloads, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if (item)
            items += QPair<QString, QString>(item->user(), item->path());
	}
	QList<QPair<QString, QString> >::iterator sit = items.begin();
	for(; sit != items.end(); ++sit)
		museeq->updateTransfer((*sit).first, (*sit).second);
}

void Transfers::messageDownloadingUsers() {
    bool res;
	QString m = QInputDialog::getText(museeq->mainwin(), tr("Send a message to all downloading users"), tr("Write the message you want to send to all users currently downloading from you"), QLineEdit::Normal, QString::null, &res);
	if (res && !m.isEmpty())
        museeq->messageDownloadingUsers(m);
}

QList<QPair<QString, QString> > Transfers::findByState(TransferListView* l, uint state) {
	QList<QPair<QString, QString> > items;
	QTreeWidgetItemIterator it(mPoppedUpload ? mUploads : mDownloads, QTreeWidgetItemIterator::Selectable);
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if(item && item->state() == state)
			items += QPair<QString, QString>(item->user(), item->path());
	}
	return items;
}

QList<QPair<QString, QString> > Transfers::findByStates(TransferListView* l, QList<uint> states) {
	QList<QPair<QString, QString> > items;
	QTreeWidgetItemIterator it(mPoppedUpload ? mUploads : mDownloads, QTreeWidgetItemIterator::Selectable);
	for(; *it; ++it) {
		TransferListItem* item = dynamic_cast<TransferListItem*>(*it);
		if(item && states.contains(item->state()))
			items += QPair<QString, QString>(item->user(), item->path());
	}
	return items;
}

void Transfers::clearFinished() {
	QList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 0);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearAborted() {
	QList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 13);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearFinishedAborted() {
	QList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 0);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 13);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearQueued() {
	QList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 7);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {
    if(domain == "transfers" && key == "upload_slots" && value != mUpslots->cleanText()) {
		mUploadSlotsChanging = true;
		mUpslots->setValue(value.toUInt());
		mUploadSlotsChanging = false;
	}
    else if(domain == "transfers" && key == "upload_rate" && value != mUprate->cleanText()) {
		mUploadRateChanging = true;
		mUprate->setValue(value.toDouble());
		mUploadRateChanging = false;
	}
    else if(domain == "transfers" && key == "download_slots" && value != mDownslots->cleanText()) {
		mDownloadSlotsChanging = true;
		mDownslots->setValue(value.toUInt());
		mDownloadSlotsChanging = false;
	}
    else if(domain == "transfers" && key == "download_rate" && value != mDownrate->cleanText()) {
		mDownloadRateChanging = true;
		mDownrate->setValue(value.toDouble());
		mDownloadRateChanging = false;
	}
}

void Transfers::setUpslots(const QString& upslots) {
	if (mUploadSlotsChanging)
		return;
	museeq->setConfig("transfers", "upload_slots", upslots);
}

void Transfers::setUprate(const QString& uprate) {
	if (mUploadRateChanging)
		return;
	museeq->setConfig("transfers", "upload_rate", uprate);
}

void Transfers::setDownslots(const QString& downslots) {
	if (mDownloadSlotsChanging)
		return;
	museeq->setConfig("transfers", "download_slots", downslots);
}

void Transfers::setDownrate(const QString& downrate) {
	if (mDownloadRateChanging)
		return;
	museeq->setConfig("transfers", "download_rate", downrate);
}
