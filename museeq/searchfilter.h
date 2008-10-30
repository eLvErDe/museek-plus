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

#ifndef SEARCHFILTER_H
#define SEARCHFILTER_H

#include "museeqtypes.h"

#include <QWidget>
#include <QLineEdit>
#include <QRegExp>

class SearchListView;
class SearchListItem;
class QCheckBox;
class QComboBox;
class QShowEvent;
class QHideEvent;
class QKeyEvent;

class SearchFilter : public QWidget {
	Q_OBJECT
public:
	SearchFilter(QWidget* = 0, const char * = 0);
	bool match(SearchListItem *item);
	void refilter(SearchListView *list);

signals:
	void filterChanged();

protected:
	void showEvent(QShowEvent *);
	void hideEvent(QHideEvent *);

protected slots:
	void updateFilter();

private:

	bool mEnabled;
	QComboBox* mRegExp, * mSize, * mBitrate;
	QCheckBox* mFreeSlot;

	QRegExp mFilterRegExp;
	qint64 mFilterSize;
	int mFilterBitrate;
	bool mFilterFreeSlot, mFilterSizeExact, mFilterBitrateExact;
};

class MyLineEdit : public QLineEdit {
	Q_OBJECT
public:
	MyLineEdit(QWidget* p = 0, const char* n = 0) : QLineEdit(p) { };
signals:
	void enterPressed();
protected:
	void keyReleaseEvent(QKeyEvent*);
};

#endif // SEARCHFILTER_H
