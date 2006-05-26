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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <qtabbar.h>
#include <qtabwidget.h>

class TabWidget;
class Usermenu;

class Tab : public QObject, public QTab {
	Q_OBJECT
public:
	Tab(TabWidget* parent, const QString& text);
	int highlighted() const;
	
public slots:
	void setHighlight(int i);
	void selected();

private:
	int mHighlight;
};
	
class TabBar : public QTabBar {
	Q_OBJECT
	
public:
	TabBar(bool, QWidget* = 0, const char* = 0);

public:
	bool canDrop() const;
	
public slots:
	void setCanDrop(bool);
	
signals:
	void dropSlsk(const QStringList&);
	
protected:
	void dragMoveEvent(QDragMoveEvent*);
	void dropEvent(QDropEvent*);
	bool mCanDrop;
	void paintLabel(QPainter*, const QRect&, QTab*, bool) const;
	void contextMenuEvent(QContextMenuEvent*);
	
private:
	Usermenu* mUsermenu;
};

class TabWidget : public QTabWidget {
	Q_OBJECT
	Q_PROPERTY(QString current READ getCurrentPage)
public:
	TabWidget(QWidget* = 0, const char* = 0, bool = false);
	bool isCurrent(Tab*);
	QString getCurrentPage() const;
	
public slots:
	QWidget * getCurrentWidget() const;
	void repaintTabBar();
	
protected:
	bool protectFirst() const;
	bool protectThird() const;
	bool canDrop() const;
	
protected slots:
	void setCanDrop(bool);
	virtual void dropSlsk(const QStringList&);
	
	void setProtectThird(bool);
	void doCurrentChanged(QWidget *);
	virtual void closeCurrent();
	void previousPage();
	void nextPage();

private:
	bool mProtectFirst;
	bool mProtectThird;
	TabBar* mTabBar;
};

#endif // TABWIDGET_H
