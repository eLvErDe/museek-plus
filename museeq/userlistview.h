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

#ifndef USERLISTVIEW_H
#define USERLISTVIEW_H

#include "museeqtypes.h"

#include <qlistview.h>

class UserListItem;
class QDragObject;
class Usermenu;

class UserListView : public QListView {
	Q_OBJECT
public:
	UserListView(bool, QWidget * = 0, const char * = 0);
	
	UserListItem* findItem(const QString&);
	
	uint status(const QString&);
	uint speed(const QString&);
	uint files(const QString&);
	QString comments(const QString&);
	
public slots:
	void setComments(const QString&, const QString&);
	void add(const QString&, uint = 0, uint = 0, uint = 0, const QString& = QString::null);
	void add(const QString&, const QString&);
	void remove(const QString&);
	
signals:
	void activated(const QString&);
	void activated(const QString&, const QString&);
	void dropSlsk(const QStringList&);
	
protected:
	QDragObject* dragObject();
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
	
protected slots:
	void updateStatus(const QString&);
	void setStatus(const QString&, uint);
	void setData(const QString&, uint, uint);
	void slotActivate(QListViewItem*);
	void slotActivate(QListViewItem*, const QPoint&, int);
	void slotContextMenu(QListViewItem*, const QPoint&, int);
	
private:
	Usermenu* mUsermenu;
};

#endif // USERLISTVIEW_H
