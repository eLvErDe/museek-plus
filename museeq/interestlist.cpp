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

#include "museeq.h"
#include "interestlist.h"
#include "interestlistview.h"
#include "interestlistitem.h"

#include <QLineEdit>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

InterestList::InterestList(const QString& caption, QWidget* parent, bool readOnly)
             : QWidget(parent), mReadOnly(readOnly) {
	QVBoxLayout* vbox = new QVBoxLayout(this);
	vbox->setSpacing(5);
	vbox->setMargin(0);
	mListView = new InterestListView(caption, this, mReadOnly);
	vbox->addWidget(mListView);
	if (!mReadOnly) {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->setSpacing(3);
        hbox->setMargin(2);
        vbox->addLayout(hbox);
        hbox->addWidget(new QLabel(tr("Add:")));
        mEntry = new QLineEdit(this);
        hbox->addWidget(mEntry);

        mAdd = new QPushButton(tr("Add"), this);
        hbox->addWidget(mAdd);

        if ( caption == tr("I like:") ) {
            connect(mEntry, SIGNAL(returnPressed()), SLOT(slotDoAddInterest()));
            connect(mAdd, SIGNAL(clicked()), SLOT(slotDoAddInterest()));
        }
        else if ( caption == tr("I hate:") ) {
            connect(mEntry, SIGNAL(returnPressed()), SLOT(slotDoAddHatedInterest()));
            connect(mAdd, SIGNAL(clicked()), SLOT(slotDoAddHatedInterest()));
        }
	}
}

void InterestList::added(const QString& item) {
	QList<QTreeWidgetItem *> InterestsMatch = mListView->findItems(item, Qt::MatchExactly, 0);
	if (! InterestsMatch.isEmpty())
		return;
	new InterestListItem(mListView, item);
}

void InterestList::removed(const QString& item) {
	QList<QTreeWidgetItem *> InterestsMatch = mListView->findItems(item, Qt::MatchExactly, 0);
	if (InterestsMatch.isEmpty())
		return;
	QTreeWidgetItem* i = InterestsMatch.at(0);
	if (! i)
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
