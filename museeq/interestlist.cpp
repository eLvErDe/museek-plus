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

#include "interestlist.h"

#include <qlistview.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qlabel.h>
#include "museeq.h"
#include <qpopupmenu.h>
#include "interestlistview.h"
InterestList::InterestList(const QString& caption, QWidget* parent, const char* name)
             : QVBox(parent, name) {
	
	mListView = new InterestListView(caption, this);

	mListView->setResizeMode(QListView::LastColumn);
	QHBox* box = new QHBox(this);
	new QLabel(tr("Add:"), box);
	mEntry = new QLineEdit(box);

	
	if ( caption == tr("I like:") ) {
		connect(mEntry, SIGNAL(returnPressed()), SLOT(slotDoAddInterest()));
 	}
	else if ( caption == tr("I hate:") ) {
		connect(mEntry, SIGNAL(returnPressed()), SLOT(slotDoAddHatedInterest()));

	}
}

void InterestList::added(const QString& item) {
	if(mListView->findItem(item, 0) != 0)
		return;
	new QListViewItem(mListView, item);
}

void InterestList::removed(const QString& item) {
	QListViewItem* i = mListView->findItem(item, 0);
	if(! i)
		return;
	delete i;
}

void InterestList::slotDoAddInterest() {
	QString s = mEntry->text();
	if(s.isEmpty())
		return;
	mEntry->setText(QString::null);
	museeq->addInterest(s);
}

void InterestList::slotDoAddHatedInterest() {
	QString s = mEntry->text();
	if(s.isEmpty())
		return;
	mEntry->setText(QString::null);
	museeq->addHatedInterest(s);
}
void InterestList::slotDoRemoveInterest() {
}

void InterestList::slotDoRemoveHatedInterest() {
}
