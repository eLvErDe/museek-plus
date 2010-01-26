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
#include "searchfilter.h"
#include "searchlistview.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QKeyEvent>

SearchFilter::SearchFilter(QWidget *parent, const char *name)
             : QWidget(parent) {

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QHBoxLayout *box = new QHBoxLayout(this);

	mRegExp = new QComboBox(this);
	box->addWidget(mRegExp);
	mRegExp->setLineEdit(new MyLineEdit(mRegExp));
	mRegExp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mRegExp->setEditable(true);
	connect(mRegExp->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));

	box->addWidget(new QLabel(tr("Size:")));
	mSize = new QComboBox(this);
	box->addWidget(mSize);
	mSize->setLineEdit(new MyLineEdit(mSize));
	mSize->setEditable(true);
	connect(mSize->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));

	box->addWidget(new QLabel(tr("Bitrate:")));
	mBitrate = new QComboBox(this);
	box->addWidget(mBitrate);
	mBitrate->setLineEdit(new MyLineEdit(mBitrate));
	mBitrate->setEditable(true);
	connect(mBitrate->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));

	mFreeSlot = new QCheckBox(tr("Free slot"), this);
	box->addWidget(mFreeSlot);
	connect(mFreeSlot, SIGNAL(toggled(bool)), SLOT(updateFilter()));

	updateFilter();
}

bool SearchFilter::match(SearchListItem *item) {
	if(! mEnabled)
		return true;

	if(! mFilterRegExp.isEmpty() && mFilterRegExp.indexIn(item->path()) == -1)
		return false;

	// Gotta love these:
	if(mFilterSize && ! (mFilterSizeExact ? (static_cast<int>(item->size()) == mFilterSize) : ((mFilterSize < 0) ? (static_cast<int>(item->size()) <= (-mFilterSize)) : (static_cast<int>(item->size()) >= mFilterSize))))
		return false;

	if(mFilterBitrate && ! (mFilterBitrateExact ? ((int)item->bitrate() == mFilterBitrate) : ((mFilterBitrate < 0) ? ((int)item->bitrate() <= (-mFilterBitrate)) : ((int)item->bitrate() >= mFilterBitrate))))
		return false;

	if(mFilterFreeSlot && ! item->freeSlot())
		return false;

	return true;
}

void SearchFilter::refilter(SearchListView *list) {
	QTreeWidgetItemIterator it(list);
 	while(*it) {
 	    SearchListItem* item = dynamic_cast<SearchListItem*>(*it);
 	    if (item)
            (*it)->setHidden(!match(item));
 		it++;
 	}
}

void SearchFilter::showEvent(QShowEvent *ev) {
	if(! mEnabled) {
		mEnabled = true;
		emit filterChanged();
	}
}

void SearchFilter::hideEvent(QHideEvent *ev) {
	if(mEnabled) {
		mEnabled = false;
		emit filterChanged();
	}
}

void SearchFilter::updateFilter() {
	mFilterRegExp.setPattern(mRegExp->currentText());
	mFilterRegExp.setCaseSensitivity(Qt::CaseInsensitive);
	if(! mFilterRegExp.isValid()) {
		QMessageBox::warning(this, tr("Warning - Museeq"), tr("Invalid regular expression, disabling it"));
		mFilterRegExp.setPattern(QString::null);
	}

	bool ok, neg = false;
	qint64 factor = 1;
	QString s = mSize->currentText();
	mFilterSizeExact = false;
	mFilterSize = 0;
	if(! s.isEmpty()) {
		if(s[0] == '=') {
			mFilterSizeExact = true;
			s = s.mid(1);
		} else if(s[0] == '<') {
			neg = true;
			s = s.mid(1);
		}

		if(! s.isEmpty()) {
			QString l = s.right(1).toLower();
			if(l == tr("k"))
				factor = 1024;
			else if(l == tr("m"))
				factor = 1024*1024;
			else if(l == tr("g"))
				factor = 1024*1024*1024;
			if(factor != 1)
				s = s.left(s.length() - 1);
		}

		mFilterSize = s.toInt(&ok);
		if(! ok) {
			QMessageBox::warning(this, tr("Warning - Museeq"), tr("Invalid size filter, disabling it"));
			mFilterSizeExact = false;
			mFilterSize = 0;
		} else {
			if(neg)
				mFilterSize *= -1;
			mFilterSize *= factor;
		}
	}

	neg = false;
	s = mBitrate->currentText();
	mFilterBitrateExact = false;
	mFilterBitrate = 0;
	if(! s.isEmpty()) {
		if(s[0] == '=') {
			mFilterBitrateExact = true;
			s = s.mid(1);
		} else if(s[0] == '<') {
			neg = true;
			s = s.mid(1);
		}
		mFilterBitrate = s.toInt(&ok);
		if(! ok) {
			QMessageBox::warning(this, tr("Warning - Museeq"), tr("Invalid bitrate filter, disabling it"));
			mFilterBitrateExact = false;
			mFilterBitrate = 0;
		} else if(neg)
			mFilterBitrate *= -1;
	}

	mFilterFreeSlot = mFreeSlot->isChecked();

	emit filterChanged();
}

void MyLineEdit::keyReleaseEvent(QKeyEvent *e) {
	if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
		emit enterPressed();
	else
		QLineEdit::keyReleaseEvent(e);
}
