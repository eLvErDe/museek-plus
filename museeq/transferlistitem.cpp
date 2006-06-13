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

#include "transferlistitem.h"
#include "util.h"
#include "museeq.h"
TransferListItem::TransferListItem(QListView* p, const QString& _u, const QString& _p)
                 : QListViewItem(p), mUser(_u), mPath(_p) {
	
	setSelectable(! mPath.isNull());
	setDragEnabled(true);
	setText(0, mUser);
	updatePath();
	
	NTransfer t;
	t.state = 15;
	t.error = QString::null;
	t.filepos = 0;
	t.filesize = 0;
	t.rate = 0;
	t.placeInQueue = (uint)-1;
	update(t, true);
}

TransferListItem::TransferListItem(QListViewItem* p, const QString& _u, const QString& _p)
                 : QListViewItem(p), mUser(_u), mPath(_p) {
	
	setDragEnabled(true);
	setText(0, mUser);
	updatePath();
	
	NTransfer t;
	t.state = 15;
	t.error = QString::null;
	t.filepos = 0;
	t.filesize = 0;
	t.rate = 0;
	update(t, true);
}

void TransferListItem::updatePath() {
	if(! mPath.isNull()) {
		int ix = mPath.findRev('\\');
		if(ix != -1) {
			setText(1, mPath.mid(ix + 1));
			setText(7, mPath.left(ix + 1));
		} else
			setText(1, mPath);
	}
}

void TransferListItem::update(const NTransfer& transfer, bool force) {
	if(transfer.state != mState || force) {
		mState = transfer.state;
		switch(mState) {
		case 0:
			setText(2, QT_TR_NOOP("Finished"));
			break;
		case 1:
			setText(2, QT_TR_NOOP("Transferring"));
			break;
		case 2:
			setText(2, QT_TR_NOOP("Negotiating"));
			break;
		case 3:
			setText(2, QT_TR_NOOP("Waiting"));
			break;
		case 4:
			setText(2, QT_TR_NOOP("Establishing"));
			break;
		case 5:
			setText(2, QT_TR_NOOP("Initiating"));
			break;
		case 6:
			setText(2, QT_TR_NOOP("Connecting"));
			break;
		case 7:
			setText(2, QT_TR_NOOP("Queued"));
			break;
		case 8:
			setText(2, QT_TR_NOOP("Getting address"));
			break;
		case 9:
			setText(2, QT_TR_NOOP("Getting status"));
			break;
		case 10:
			setText(2, QT_TR_NOOP("User Offline"));
			break;
		case 11:
			setText(2, QT_TR_NOOP("Connection closed by peer"));
			break;
		case 12:
			setText(2, QT_TR_NOOP("Cannot connect"));
			break;
		case 13:
			setText(2, QT_TR_NOOP("Aborted"));
			break;
		case 14:
			mError = QString::null;
			break;
		case 15:
			setText(2, QString::null);
			break;
		}
	}
	if(mState == 14 && (transfer.error != mError || force)) {
		mError = transfer.error;
		setText(2, mError);
	}
	uint place = (uint)-1;
	if(mState == 7)
		place = transfer.placeInQueue;
	if(place != mPlaceInQueue || force) {
		mPlaceInQueue = place;
		if(mPlaceInQueue != (uint)-1)
			setText(3, QString::number(mPlaceInQueue));
		else
			setText(3, "");
	}
	if(transfer.filepos != mPosition || force) {
		mPosition = transfer.filepos;
		setText(4, Util::makeSize(mPosition));
	}
	if(transfer.filesize != mSize || force) {
		mSize = transfer.filesize;
		setText(5, Util::makeSize(mSize));
	}
	if(transfer.rate != mRate || force) {
		mRate = transfer.rate;
		setText(6, Util::makeSize(mRate) + QT_TR_NOOP("/s"));
	}
}

QString TransferListItem::user() const {
	return mUser;
}

QString TransferListItem::path() const {
	return mPath;
}


uint TransferListItem::state() const {
	return mState;
}

QString TransferListItem::error() const {
	return mError;
}


Q_INT64 TransferListItem::position() const {
	return mPosition;
}


Q_INT64 TransferListItem::size() const {
	return mSize;
}

uint TransferListItem::rate() const {
	return mRate;
}

void TransferListItem::updateStats() {
	Q_INT64 __f = 0, __t = 0;
	uint __r = 0;
	TransferListItem* file = static_cast<TransferListItem*>(firstChild());
	for(; file != 0; file = static_cast<TransferListItem*>(file->nextSibling())) {
		__f += file->mPosition;
		if(file->mSize == 0)
			__t = -1;
		else if(__t > -1)
			__t += file->mSize;
		if(file->mState == 1)
			__r = file->mRate;
	}
	setText(3, "");
	setText(4, Util::makeSize(__f));
	if(__t == -1)
		setText(5, "?");
	else
		setText(5, Util::makeSize(__t));
	
	setText(6, Util::makeSize(__r) + QT_TR_NOOP("/s"));
}

void TransferListItem::update(const NTransfer& transfer) {
	if(mPath.isNull()) {
		TransferListItem* file;
		if(! transfer.filename.isNull()) {
			file = static_cast<TransferListItem*>(firstChild());
			for(; file != 0; file = static_cast<TransferListItem*>(file->nextSibling()))
				if(file->mPath == transfer.filename)
					break;
			if(! file)
				file = new TransferListItem(this, mUser, transfer.filename);
		
			file->update(transfer, true);
		}
		updateStats();
	} else
		update(transfer, false);
}

void TransferListItem::remove(const QString& _p) {
	TransferListItem* file = static_cast<TransferListItem*>(firstChild());
	for(; file != 0; file = static_cast<TransferListItem*>(file->nextSibling())) {
		if(file->path() == _p) {
			delete file;
			updateStats();
			return;
		}
	}
}

int TransferListItem::compare(QListViewItem* i, int col, bool) const {
	TransferListItem* item = static_cast<TransferListItem*>(i);
	switch(col) {
	case 0: return mUser.localeAwareCompare(item->mUser);
	case 1: return text(1).localeAwareCompare(item->text(1));
	case 2: return text(2).localeAwareCompare(item->text(2));
	case 3: return Util::cmp(mPlaceInQueue, item->mPlaceInQueue);
	case 4: return Util::cmp(mPosition, item->mPosition);
	case 5: return Util::cmp(mSize, item->mSize);
	case 6: return Util::cmp(mRate, item->mRate);
	case 7: return text(7).localeAwareCompare(item->text(7));
	}
	return 0;
}
