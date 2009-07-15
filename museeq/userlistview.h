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

#ifndef USERLISTVIEW_H
#define USERLISTVIEW_H

#include "museeqtypes.h"

#include <QTreeWidget>

class UserListItem;
class Usermenu;
class QDropEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QMouseEvent;

class UserListView : public QTreeWidget {
	Q_OBJECT
public:
	UserListView(bool, QWidget * = 0, const QString& = QString::null);

	UserListItem* findItem(const QString&);

	uint status(const QString&);
	uint speed(const QString&);
	uint files(const QString&);
	QString comments(const QString&);
	QString country(const QString&);

public slots:
	void setComments(const QString&, const QString&);
	void setCountry(const QString&, const QString&);
	void setOperator(const QString&, bool);
	void setOwner(const QString&, bool);
	void add(const QString&, uint = 0, uint = 0, uint = 0, const QString& = QString::null, const QString& = QString::null);
	void add(const QString&, const QString&);
	void remove(const QString&);
	void sorting(bool);
	void adaptColumnSize(int);
	void onClosingMuseeq();

signals:
	void activated(const QString&);
	void activated(const QString&, const QString&);
	void dropSlsk(const QList<QUrl>&);

protected:
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
    void dragMoveEvent(QDragMoveEvent*);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	QStringList headers;

protected slots:
	void keyboardSearch(const QString&);
	void updateStatus(const QString&);
	void setStatus(const QString&, uint);
	void setData(const QString&, uint, uint, const QString&);
	void slotActivate(QTreeWidgetItem*);
	void slotActivate(QTreeWidgetItem*,  int);
	void slotContextMenu(const QPoint&);
	void slotContextMenu(QTreeWidgetItem*, const QPoint&, int);
	void countryToggled(bool);

private:
	Usermenu* mUsermenu;
	QString mLastSearch, mName;

	uint mSearchPosition;
	QPoint mDragStartPosition;
};

#endif // USERLISTVIEW_H
