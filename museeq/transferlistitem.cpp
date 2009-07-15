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

#include "transferlistitem.h"
#include "transferlistview.h"
#include "util.h"
#include "museeq.h"

#include <QProgressBar>
#include <QPainter>

TransferListItem::TransferListItem(QTreeWidget* p, const QString& _u, const QString& _p)
                 : QTreeWidgetItem(p), mUser(_u), mPath(_p), separation(10) {

	setText(0, mUser);
	setTextAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(6, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(7, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(8, Qt::AlignRight|Qt::AlignVCenter);
	updatePath();

	NTransfer t;
	t.state = 100;
	t.error = QString::null;
	t.user = QString::null;
	t.filename = QString::null;
	t.filepos = 0;
	t.filesize = 0;
	t.rate = 0;
	t.placeInQueue = (uint)-1;

	update(t, true);
}

TransferListItem::TransferListItem(QTreeWidgetItem* p, const QString& _u, const QString& _p)
                : QTreeWidgetItem(p), mUser(_u), mPath(_p), separation(10) {

	setText(0, mUser);
	setTextAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(6, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(7, Qt::AlignRight|Qt::AlignVCenter);
	setTextAlignment(8, Qt::AlignRight|Qt::AlignVCenter);
	updatePath();

	NTransfer t;
	t.state = 100;
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
			setText(9, mPath.left(ix + 1));
		} else
			setText(1, mPath);
	}
}

void TransferListItem::update(const NTransfer& transfer, bool force) {
	if(transfer.state != mState || transfer.error != mError || force) {
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
			setText(2, TransferListView::tr("User offline"));
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
			mError = transfer.error;
			setText(2, TransferListView::tr("Remote: ") + mError);
			setBackground(2, QBrush(QColor(179,29,29)));
			setForeground(2, QBrush(QColor(255,255,255)));
			break;
		case 15:
			mError = transfer.error;
			setText(2, TransferListView::tr("Local: ") + mError);
			setBackground(2, QBrush(QColor(179,29,29)));
			setForeground(2, QBrush(QColor(255,255,255)));
			break;
		case 16:
			setText(2, TransferListView::tr("Queued"));
			setBackground(2, QBrush(QColor(233,232,135)));
			setForeground(2, QBrush(QColor(15,15,15)));
			break;
        default:
			setText(2, QString::null);
			break;
		}
	}

	uint place = (uint)-1;
	if(mState == 7)
		place = transfer.placeInQueue;
	if(place != mPlaceInQueue || force) {
		mPlaceInQueue = place;
		if(mPlaceInQueue != (uint)-1)
			setText(4, QString::number(mPlaceInQueue));
		else
			setText(4, "");
	}
	if((transfer.rate != mRate) || (transfer.filepos != mPosition) || (transfer.filesize != mSize) || force) {
	    if (transfer.rate > 0)
            mTimeLeft = (transfer.filesize - transfer.filepos) / transfer.rate;
	    else
            mTimeLeft = 0;

        if (mTimeLeft > 0)
            setText(8, Util::makeTime(mTimeLeft));
        else
            setText(8, TransferListView::tr("?"));
	}
	if(transfer.filepos != mPosition || force) {
		mPosition = transfer.filepos;
		setText(5, Util::makeSize(mPosition));
	}
	if(transfer.filesize != mSize || force) {
		mSize = transfer.filesize;
		setText(6, Util::makeSize(mSize));
	}
	if(transfer.rate != mRate || force) {
		mRate = transfer.rate;
		setText(7, Util::makeSize(mRate) + TransferListView::tr("/s"));
	}

    int progress = 0;
    if (mState == 0)
        progress = 1000;
    else if ((mSize > 0) && (mPosition > 0))
        progress = static_cast<uint>((static_cast<float>(mPosition)/static_cast<float>(mSize)) * 1000);
    if (progress > 1000)
        progress = 1000;
    setData(3, Qt::DisplayRole, progress);
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
	qint64 groupPosition = 0, groupSize = 0;
	uint groupRate = 0, groupTimeLeft;
	setText(4, "");
	if (! childCount())
		return;
	TransferListItem* file;
	int pos = 0;
	for(; pos < childCount(); pos++) {
		file = dynamic_cast<TransferListItem*>(child(pos));
		if (file) {
			groupPosition += file->mPosition;
			if(file->mSize == 0)
				groupSize = -1;
			else if(groupSize > -1)
				groupSize += file->mSize;
			if(file->mState == 1)
				groupRate = file->mRate;
		}
	}

	setText(5, Util::makeSize(groupPosition));
	if(groupSize == -1)
		setText(6, "?");
	else
		setText(6, Util::makeSize(groupSize));

	setText(7, Util::makeSize(groupRate) + TransferListView::tr("/s"));

    if (groupRate > 0)
        groupTimeLeft = (groupSize - groupPosition) / groupRate;
    else
        groupTimeLeft = 0;

    if (groupTimeLeft > 0)
        setText(8, Util::makeTime(groupTimeLeft));
    else
        setText(8, TransferListView::tr("?"));

    // Update progress bar
    int progress = 0;
    if ((groupSize > 0) && (groupPosition > 0))
        progress = static_cast<uint>((static_cast<float>(groupPosition)/static_cast<float>(groupSize)) * 1000);
    if (progress > 1000)
        progress = 1000;
    setData(3, Qt::DisplayRole, progress);
}

void TransferListItem::update(const NTransfer& transfer) {
	if(mPath.isNull())
		updateStats();
    else {
		update(transfer, false);
		TransferListView * tree = dynamic_cast<TransferListView *>(treeWidget());
		if(!tree || tree->groupMode() == tree->None)
			return;
        TransferListItem* p = dynamic_cast<TransferListItem*>(parent());
        if (p)
            p->updateStats();
	}
}

bool TransferListItem::operator<(const QTreeWidgetItem & other_) const {
	const TransferListItem * other = dynamic_cast<const TransferListItem *>(&other_);
	if (!other)
        return false;

	int col = 0;
	if(treeWidget())
        col = treeWidget()->sortColumn();

	switch(col) {
	case 0:
		return user() < other->user();
	case 1:
		return text(1) < other->text(1);
	case 2:
		return text(2) < other->text(2);
    case 3:
		return data( 3, Qt::DisplayRole ).toInt() < other->data( 3, Qt::DisplayRole ).toInt();
	case 4:
		return mPlaceInQueue < other->mPlaceInQueue;
	case 5:
		return position() < other->position();
	case 6:
		return size() < other->size();
	case 7:
		return rate() < other->rate();
	case 8:
		return timeLeft() < other->timeLeft();
	case 9:
		return path() < other->path();

	}

  return false;
}
