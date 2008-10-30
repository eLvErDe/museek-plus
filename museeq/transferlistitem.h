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

#ifndef TRANSFERLISTITEM_H
#define TRANSFERLISTITEM_H

#include "museeqtypes.h"

#include <QTreeWidget>
class QProgressBar;

class TransferListItem : public QTreeWidgetItem {
public:
	TransferListItem(QTreeWidget*, const QString&, const QString& = QString::null);
	TransferListItem(QTreeWidgetItem*, const QString&, const QString&);

	void update(const NTransfer&);
	void updateStats();

	const uint getSeparation() const { return separation; }
	void setSeparation(uint s) { separation = s; }

	QString user() const;
	QString path() const;
	uint state() const;
	QString error() const;
	qint64 position() const;
	qint64 size() const;
	uint rate() const;
	uint timeLeft() const { return mTimeLeft; }
	bool operator<(const QTreeWidgetItem & other) const;

protected:
	void updatePath();
	void update(const NTransfer&, bool);

private:
	uint mState, mRate, mPlaceInQueue, mTimeLeft;
	QString mUser, mPath, mError;
	qint64 mPosition, mSize;
	uint separation;
	QProgressBar *mProgress;
};


#endif // TRANSFERLISTITEM_H
