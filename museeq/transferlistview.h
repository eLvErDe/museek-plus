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

#ifndef TRANSFERLISTVIEW_H
#define TRANSFERLISTVIEW_H

#include "museeqtypes.h"

#include <QTreeWidget>

class QDropEvent;
class QDragEnterEvent;
class TransferListItem;
class TransferListItemProgress;

class TransferListView : public QTreeWidget {
	Q_OBJECT
public:
	typedef enum {
		User,
		None,
	} GroupMode;

	TransferListView(bool place, QWidget* = 0, const QString& = QString::null);

	GroupMode groupMode() const;

signals:
	void dropSlsk(const QList<QUrl>&);

public slots:
	void update(const NTransfer&);
	TransferListItem* findTransfer(const QString&, const QString&);
	TransferListItem* findParent(const QString&);
	void remove(const QString&, const QString&);
	void setGroupMode(GroupMode);
	void updateParentsStats();
	void onClosingMuseeq();

protected:
	void dragEnterEvent(QDragEnterEvent*);
	void dragMoveEvent(QDragMoveEvent* event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void dropEvent(QDropEvent*);

private:
	GroupMode mGroupMode;
	QPoint mDragStartPosition;
	TransferListItemProgress *mProgressBar;
	QString mName;
};


#endif // TRANSFERLISTVIEW_H
