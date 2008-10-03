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
#include "transferlistview.h"
#include "util.h"
#include "museeq.h"

TransferListItem::TransferListItem(QTreeWidget* p, const QString& _u, const QString& _p)
                 : QTreeWidgetItem(p), mUser(_u), mPath(_p), separation(10) {

	setText(0, mUser);
	setTextAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
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

TransferListItem::TransferListItem(QTreeWidgetItem* p, const QString& _u, const QString& _p)
                : QTreeWidgetItem(p), mUser(_u), mPath(_p), separation(10) {

	setText(0, mUser);
	setTextAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
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
		int ix = mPath.lastIndexOf('\\');
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
			setText(2, TransferListView::tr("Finished"));
			setBackground(2, QBrush(QColor(25,108,0)));
			setForeground(2, QBrush(QColor(255,255,255)));
			break;
		case 1:
			setText(2, TransferListView::tr("Transferring"));
			setBackground(2, QBrush(QColor(54,232,96)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 2:
			setText(2, TransferListView::tr("Negotiating"));
			setForeground(2, QBrush(QColor(15,15,15)));
			setBackground(2, QBrush(QColor(4,197,245)));
			break;
		case 3:
			setText(2, TransferListView::tr("Waiting"));
			setForeground(2, QBrush(QColor(15,15,15)));
			setBackground(2, QBrush(QColor(4,197,245)));
			break;
		case 4:
			setText(2, TransferListView::tr("Establishing"));
			setForeground(2, QBrush(QColor(15,15,15)));
			setBackground(2, QBrush(QColor(4,197,245)));
			break;
		case 5:
			setText(2, TransferListView::tr("Initiating"));
			setForeground(2, QBrush(QColor(15,15,15)));
			setBackground(2, QBrush(QColor(4,197,245)));
			break;
		case 6:
			setText(2, TransferListView::tr("Connecting"));
			setForeground(2, QBrush(QColor(15,15,15)));
			setBackground(2, QBrush(QColor(4,197,245)));
			break;
		case 7:
			setText(2, TransferListView::tr("Queued"));
			setBackground(2, QBrush(QColor(233,232,135)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 8:
			setText(2, TransferListView::tr("Getting address"));
			break;
		case 9:
			setText(2, TransferListView::tr("Getting status"));
			setBackground(2, QBrush(QColor(230,255,114)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 10:
			setText(2, TransferListView::tr("User Offline"));
			setBackground(2, QBrush(QColor(200,200,200)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 11:
			setText(2, TransferListView::tr("Connection closed by peer"));
			setBackground(2, QBrush(QColor(255,138,166)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 12:
			setText(2, TransferListView::tr("Cannot connect"));
			setBackground(2, QBrush(QColor(255,155,135)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 13:
			setText(2, TransferListView::tr("Aborted"));
			setBackground(2, QBrush(QColor(230,255,0)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
		case 14:
			mError = QString::null;
			setBackground(2, QBrush(QColor(179,29,29)));
			setForeground(2, QBrush(QColor(255,255,255)));
			break;
		case 15:
			setText(2, QString::null);
			break;
		case 16:
			setText(2, TransferListView::tr("Queued"));
			setBackground(2, QBrush(QColor(233,232,135)));
			setForeground(2, QBrush(QColor(15,15,15)));
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
		setText(6, Util::makeSize(mRate) + TransferListView::tr("/s"));
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


qint64 TransferListItem::position() const {
	return mPosition;
}


qint64 TransferListItem::size() const {
	return mSize;
}

uint TransferListItem::rate() const {
	return mRate;
}

void TransferListItem::updateStats() {
	qint64 __f = 0, __t = 0;
	uint __r = 0;
	setText(3, "");
	if (! childCount())
		return;
	TransferListItem* file;
	int pos = 0;
	for(; pos < childCount(); pos++) {
		file = static_cast<TransferListItem*>(child(pos));
		if (file) {
			__f += file->mPosition;
			if(file->mSize == 0)
				__t = -1;
			else if(__t > -1)
				__t += file->mSize;
			if(file->mState == 1)
				__r = file->mRate;
		}
	}

	setText(4, Util::makeSize(__f));
	if(__t == -1)
		setText(5, "?");
	else
		setText(5, Util::makeSize(__t));

	setText(6, Util::makeSize(__r) + TransferListView::tr("/s"));

}

void TransferListItem::update(const NTransfer& transfer) {
	if(mPath.isNull())
		updateStats();
    else {
		update(transfer, false);
		if((static_cast<TransferListView *>(treeWidget()))->groupMode() == (static_cast<TransferListView *>(treeWidget()))->None)
			return;
		static_cast<TransferListItem*>(parent())->updateStats();
	}
}

bool TransferListItem::operator<(const QTreeWidgetItem & other_) const {
	const TransferListItem * other = static_cast<const TransferListItem *>(&other_);
	int col = 0;
	if(treeWidget())
        col = treeWidget()->sortColumn();

	switch(col) {
	case 0:
		return user() < other->user();
	case 1:
		return text(1) < other->text(1);
	case 2:
		if(text(2) == other->text(2))
			return user() < other->user();
		return text(2) < other->text(2);
	case 3:
		if(mPlaceInQueue == other->mPlaceInQueue)
			return user() < other->user();
		return mPlaceInQueue < other->mPlaceInQueue;
	case 4:
		if(position() == other->position())
			return user() < other->user();
		return position() < other->position();
	case 5:
		if(size() == other->size())
			return user() < other->user();
		return size() < other->size();
	case 6:
		if(rate() == other->rate())
			return user() < other->user();
		return rate() < other->rate();
	case 7:
		if(path() == other->path())
			return user() < other->user();
		return path() < other->path();

	}

  return false;
}
