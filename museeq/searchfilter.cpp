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
#include "searchfilter.h"

#include "searchlistview.h"
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qmessagebox.h>

SearchFilter::SearchFilter(QWidget *parent, const char *name)
             : QHBox(parent, name) {
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setSpacing(5);
	
	mFilterRegExp.setCaseSensitive(false);
	
	mRegExp = new QComboBox(this);
	mRegExp->setLineEdit(new MyLineEdit(mRegExp));
	mRegExp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	mRegExp->setEditable(true);
	connect(mRegExp->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));
	
	QHBox *box = new QHBox(this);
	new QLabel("Size:", box);
	mSize = new QComboBox(box);
	mSize->setLineEdit(new MyLineEdit(mSize));
	mSize->setEditable(true);
	connect(mSize->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));
	
	box = new QHBox(this);
	new QLabel("Bitrate:", box);
	mBitrate = new QComboBox(box);
	mBitrate->setLineEdit(new MyLineEdit(mBitrate));
	mBitrate->setEditable(true);
	connect(mBitrate->lineEdit(), SIGNAL(enterPressed()), SLOT(updateFilter()));
	
	mFreeSlot = new QCheckBox("Free slot", this);
	connect(mFreeSlot, SIGNAL(toggled(bool)), SLOT(updateFilter()));
	
	updateFilter();
}

bool SearchFilter::match(SearchListItem *item) {
	if(! mEnabled)
		return true;
	
	if(! mFilterRegExp.isEmpty() && mFilterRegExp.search(item->path()) == -1)
		return false;
	
	// Gotta love these:
	if(mFilterSize && ! (mFilterSizeExact ? (item->size() == mFilterSize) : ((mFilterSize < 0) ? (item->size() <= (-mFilterSize)) : (item->size() >= mFilterSize))))
		return false;
	
	if(mFilterBitrate && ! (mFilterBitrateExact ? ((int)item->bitrate() == mFilterBitrate) : ((mFilterBitrate < 0) ? ((int)item->bitrate() <= (-mFilterBitrate)) : ((int)item->bitrate() >= mFilterBitrate))))
		return false;
	
	if(mFilterFreeSlot && ! item->freeSlot())
		return false;
	
	return true;
}

void SearchFilter::refilter(SearchListView *list) {
	QListViewItemIterator it(list);
	while(it.current()) {
		it.current()->setVisible(match(static_cast<SearchListItem*>(it.current())));
		it++;
	}
}

void SearchFilter::showEvent(QShowEvent *ev) {
	QHBox::showEvent(ev);
	if(! mEnabled) {
		mEnabled = true;
		emit filterChanged();
	}
}

void SearchFilter::hideEvent(QHideEvent *ev) {
	QHBox::hideEvent(ev);
	if(mEnabled) {
		mEnabled = false;
		emit filterChanged();
	}
}

void SearchFilter::updateFilter() {
	mFilterRegExp.setPattern(mRegExp->currentText());
	if(! mFilterRegExp.isValid()) {
		QMessageBox::warning(this, "Warning - Museeq", "Invalid regular expression, disabling it");
		mFilterRegExp.setPattern(QString::null);
	}
	
	bool ok, neg = false;
	Q_INT64 factor = 1;
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
			QString l = s.right(1).lower();
			if(l == "k")
				factor = 1024;
			else if(l == "m")
				factor = 1024*1024;
			else if(l == "g")
				factor = 1024*1024*1024;
			if(factor != 1)
				s = s.left(s.length() - 1);
		}
		
		mFilterSize = s.toInt(&ok);
		if(! ok) {
			QMessageBox::warning(this, "Warning - Museeq", "Invalid size filter, disabling it");
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
			QMessageBox::warning(this, "Warning - Museeq", "Invalid bitrate filter, disabling it");
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
