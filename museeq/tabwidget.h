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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QTabWidget>

class QDragMoveEvent;
class QDropEvent;
class Usermenu;
class QUrl;

class TabBar : public QTabBar {
	Q_OBJECT

public:
	TabBar(bool, QWidget* = 0, const char* = 0);

signals:
	void dropSlsk(const QList<QUrl>&);

protected:
	void dragMoveEvent(QDragMoveEvent*);
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent*);

protected slots:

	void slotContextMenu(const QPoint&);
private:
	Usermenu* mUsermenu;
};

class TabWidget : public QTabWidget {
	Q_OBJECT
	Q_PROPERTY(QString current READ getCurrentPage)
public:
	TabWidget(QWidget* = 0, const char* = 0, bool = false);
	QString getCurrentPage() const;

public slots:
	QWidget * getCurrentWidget() const;

protected:
	int firstProtected() const;
	int lastProtected() const;
	bool canDrop() const;
	TabBar* mTabBar;

protected slots:
	void setCanDrop(bool);

	void setFirstProtected(int);
	void setLastProtected(int);
	void doCurrentChanged(QWidget *);
	virtual void closeCurrent();
	void previousPage();
	void nextPage();

private:
	int mFirstProtected, mLastProtected;
};

#endif // TABWIDGET_H
