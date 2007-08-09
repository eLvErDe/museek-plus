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

#include "transfers.h"

#include <qlabel.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qurl.h>
#include <qpopupmenu.h>
#include <qspinbox.h>

#include "transferlistview.h"
#include "transferlistitem.h"
#include "museeq.h"
#include "usermenu.h"

Transfers::Transfers(QWidget* _p, const char* _n)
          : QSplitter(_p, _n) {
	
	setOrientation(Vertical);
	
	QVBox *box = new QVBox(this);
	QHBox *hbox = new QHBox(box);
	(new QLabel(tr("Downloads:"), hbox))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mGroupDownloads = new QCheckBox(tr("Group by user"), hbox);
	connect(mGroupDownloads, SIGNAL(toggled(bool)), SLOT(groupDownloadsSet(bool)));
	
	mDownloads = new TransferListView(true, box);
	mDownloads->setAcceptDrops(true);
	connect(museeq, SIGNAL(downloadUpdated(const NTransfer&)), mDownloads, SLOT(update(const NTransfer&)));
	connect(museeq, SIGNAL(downloadRemoved(const QString&, const QString&)), mDownloads, SLOT(remove(const QString&, const QString&)));
	connect(mDownloads, SIGNAL(dropSlsk(const QStringList&)), SLOT(dropSlsk(const QStringList&)));
	
	box = new QVBox(this);
	hbox = new QHBox(box);
	hbox->setSpacing(5);
	(new QLabel(tr("Uploads:"), hbox))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mGroupUploads = new QCheckBox(tr("Group by user"), hbox);
	connect(mGroupUploads, SIGNAL(toggled(bool)), SLOT(groupUploadsSet(bool)));
	
	QFrame* frame = new QFrame(hbox);
	frame->setFrameShape(QFrame::VLine);
	frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	
	mUpslots = new QSpinBox(0, 99, 1, hbox);
	mUploadSlotsChanging = false;
	new QLabel(mUpslots, tr("slots"), hbox);
	connect(mUpslots, SIGNAL(valueChanged(const QString&)), SLOT(setUpslots(const QString&)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));
	
	mUploads = new TransferListView(false, box);
	connect(museeq, SIGNAL(uploadUpdated(const NTransfer&)), mUploads, SLOT(update(const NTransfer&)));
	connect(museeq, SIGNAL(uploadRemoved(const QString&, const QString&)), mUploads, SLOT(remove(const QString&, const QString&)));
	
	mTransferMenu = new QPopupMenu(this);
	mTransferMenu->insertItem(tr("Retry"), this, SLOT(retrySelected()), 0, 0);
	mTransferMenu->insertItem(tr("Abort"), this, SLOT(abortSelected()));
	mTransferMenu->insertItem(tr("Check Position"), this, SLOT(updateSelected()), 0, 1);
	
	mClearMenu = new QPopupMenu(mTransferMenu);
	mClearMenu->insertItem(tr("Selected"), this, SLOT(clearSelected()));
	mClearMenu->insertSeparator();
	mClearMenu->insertItem(tr("Finished"), this, SLOT(clearFinished()));
	mClearMenu->insertItem(tr("Aborted"), this, SLOT(clearAborted()));
	mClearMenu->insertItem(tr("Offline"), this, SLOT(clearAwaiting()));
	mClearMenu->insertItem(tr("Cruft"), this, SLOT(clearCruft())); 
	mClearMenu->insertItem(tr("Finished / aborted"), this, SLOT(clearFinishedAborted()));
	mClearMenu->insertItem(tr("Queued"), this, SLOT(clearQueued()));
	mTransferMenu->insertItem(tr("Clear"), mClearMenu);
	
	mTransferMenu->insertSeparator();
	
	mUsersMenu = new QPopupMenu(mTransferMenu);
	mTransferMenu->insertItem(tr("Users"), mUsersMenu);
	
	connect(mUploads, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(popupUploads(QListViewItem*, const QPoint&, int)));
	connect(mDownloads, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(popupDownloads(QListViewItem*, const QPoint&, int)));
}

TransferListView* Transfers::uploads() const {
	return mUploads;
}

TransferListView* Transfers::downloads() const {
	return mDownloads;
}

void Transfers::groupDownloadsSet(bool on) {
	if(on)
		museeq->setConfig("museeq.group", "downloads", "true");
	else
		museeq->setConfig("museeq.group", "downloads", "false");

}

void Transfers::groupDownloads(bool on) {	
	TransferListView::GroupMode m = on ? TransferListView::User : TransferListView::None;
	if(mDownloads->groupMode() == m)
		return;	
	mDownloads->setGroupMode(m);
	if(on != mGroupDownloads->isOn())
		mGroupDownloads->toggle();
}

void Transfers::groupUploadsSet(bool on) {
	if(on)
		museeq->setConfig("museeq.group", "uploads", "true");
	else
		museeq->setConfig("museeq.group", "uploads", "false");
}

void Transfers::groupUploads(bool on) {
	TransferListView::GroupMode m = on ? TransferListView::User : TransferListView::None;
	if(mUploads->groupMode() == m)
		return;
	
	mUploads->setGroupMode(m);

	if(on != mGroupUploads->isOn())
		mGroupUploads->toggle();
}

void Transfers::dropSlsk(const QStringList& l) {
	QStringList::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url = QUrl(*it);
		if(url.protocol() == "slsk" && url.hasHost() && url.hasPath()) {
			QStringList s = QStringList::split("/", *it, true);
			QStringList::iterator it = s.begin();
			++it;
			++it;

			Q_INT64 size = 0;

			QString user, path;
			
			QStringList s2 = QStringList::split("@", *it, true);
			if(s2.size() == 2) {
				bool ok;
				size = s2[0].toLongLong(&ok);
				if(! ok)
					size = 0;
				user = s2[1];
			} else
				user = *it;
				
			QUrl::decode(user);
			++it;
			while(it != s.end()) {
				path += (*it);
				++it;
				if(it != s.end())
					path += '\\';
			}
			QUrl::decode(path);
			if(path.right(1) == "\\")
				museeq->downloadFolder(user, QString(path));
			else
				museeq->downloadFile(user, QString(path), size);
		}
	}
}

void Transfers::setupUsers() {
	mUsersMenu->clear();
	
	QValueList<QString> users;
	QListViewItemIterator it(mPoppedUpload ? mUploads : mDownloads, QListViewItemIterator::Selected | QListViewItemIterator::Visible);
	for(; *it; ++it) {
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		if(users.find(item->user()) == users.end())
		{
			users << item->user();
			Usermenu *m = new Usermenu(mUsersMenu);
			m->setup(item->user());
			mUsersMenu->insertItem(item->user(), m);
		}
	}
}

void Transfers::popupUploads(QListViewItem*, const QPoint& pos, int) {
	mTransferMenu->setItemEnabled(0, true);
// 	mTransferMenu->setItemEnabled(0, false);
	mTransferMenu->setItemEnabled(1, false);
	mPoppedUpload = true;
	setupUsers();
	mTransferMenu->exec(pos);
}

void Transfers::popupDownloads(QListViewItem*, const QPoint& pos, int) {
	mTransferMenu->setItemEnabled(0, true);
	mTransferMenu->setItemEnabled(1, true);
	mPoppedUpload = false;
	setupUsers();
	mTransferMenu->exec(pos);
}

QValueList<QPair<QString, QString> > Transfers::findSelected(TransferListView* l) {
	QValueList<QPair<QString, QString> > items;
	QListViewItemIterator it(mPoppedUpload ? mUploads : mDownloads, QListViewItemIterator::Selected | QListViewItemIterator::Visible);
	for(; *it; ++it) {
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		items += QPair<QString, QString>(item->user(), item->path());
	}
	return items;
}

void Transfers::clearSelected() {
	QValueList<QPair<QString, QString> > items = findSelected(mPoppedUpload ? mUploads : mDownloads);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearAwaiting() {  // added by d
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 10);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearCruft() {  // added by d
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 10);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 13);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 12);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 11);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 0);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::abortSelected() {
	QValueList<QPair<QString, QString> > items = findSelected(mPoppedUpload ? mUploads : mDownloads);
	if(mPoppedUpload)
		museeq->abortUploads(items);
	else
		museeq->abortDownloads(items);
}

void Transfers::retrySelected() {
	QValueList<QPair<QString, QPair<QString, Q_INT64> > > items;
	QListViewItemIterator it(mPoppedUpload ? mUploads : mDownloads, QListViewItemIterator::Selected | QListViewItemIterator::Visible);
	for(; *it; ++it) {
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		items += QPair<QString, QPair<QString, Q_INT64> >(item->user(), QPair<QString, Q_INT64>(item->path(), item->size()));
	}
	QValueList<QPair<QString, QPair<QString, Q_INT64> > >::iterator sit = items.begin();
	if (mPoppedUpload) {
		QValueList<QPair<QString, QString> > items = findSelected(mPoppedUpload ? mUploads : mDownloads);
		museeq->removeUploads(items);
	}
	for(; sit != items.end(); ++sit) {
		if(mPoppedUpload)
			museeq->uploadFile((*sit).first, (*sit).second.first);
		else
			museeq->downloadFile((*sit).first, (*sit).second.first, (*sit).second.second);
	}
}

void Transfers::updateSelected() {
	QValueList<QPair<QString, QString> > items;
	QListViewItemIterator it(mDownloads, QListViewItemIterator::Selected | QListViewItemIterator::Visible);
	for(; *it; ++it) {
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		items += QPair<QString, QString>(item->user(), item->path());
	}
	QValueList<QPair<QString, QString> >::iterator sit = items.begin();
	for(; sit != items.end(); ++sit)
		museeq->updateTransfer((*sit).first, (*sit).second);
}

QValueList<QPair<QString, QString> > Transfers::findByState(TransferListView* l, uint state) {
	QValueList<QPair<QString, QString> > items;
	QListViewItemIterator it(mPoppedUpload ? mUploads : mDownloads, QListViewItemIterator::Selectable);
	for(; *it; ++it) {
		TransferListItem* item = static_cast<TransferListItem*>(*it);
		if(item->state() == state)
			items += QPair<QString, QString>(item->user(), item->path());
	}
	return items;
}

void Transfers::clearFinished() {
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 0);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearAborted() {
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 13);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearFinishedAborted() {
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 0);
	items += findByState(mPoppedUpload ? mUploads : mDownloads, 13);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::clearQueued() {
	QValueList<QPair<QString, QString> > items = findByState(mPoppedUpload ? mUploads : mDownloads, 7);
	if(mPoppedUpload)
		museeq->removeUploads(items);
	else
		museeq->removeDownloads(items);
}

void Transfers::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {
	if(domain == "museeq.group") {
		bool on = value == "true";
		if(key == "downloads")
			groupDownloads(on);
		else if(key == "uploads")
			groupUploads(on);
	} else if(domain == "transfers" && key == "upload_slots" && value != mUpslots->cleanText()) {
		mUploadSlotsChanging = true;
		mUpslots->setValue(value.toUInt());
		mUploadSlotsChanging = false;
	}
}

void Transfers::setUpslots(const QString& upslots) {
	if (mUploadSlotsChanging)
		return;
	museeq->setConfig("transfers", "upload_slots", upslots);
}
