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

#include "banlist.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qinputdialog.h>
#include <qurl.h>

#include "userlistview.h"
#include "userlistitem.h"
#include "museeq.h"

BanList::BanList(QWidget* _p, const char* _n)
          : QVBox(_p, _n) {
	
	mUserList = new UserListView(true, this, "userlist");
	mUserList->setAcceptDrops(true);
	connect(mUserList, SIGNAL(activated(const QString&)), SIGNAL(activated(const QString&)));
	connect(mUserList, SIGNAL(dropSlsk(const QStringList&)), SLOT(slotDropSlsk(const QStringList&)));
	connect(museeq, SIGNAL(addedBanned(const QString&, const QString&)), mUserList, SLOT(add(const QString&, const QString&)));
	connect(museeq, SIGNAL(removedBanned(const QString&)), mUserList, SLOT(remove(const QString&)));
	connect(museeq, SIGNAL(disconnected()), mUserList, SLOT(clear()));
	
	QHBox *box = new QHBox(this);
	new QLabel("Add:", box);

	mEntry = new QLineEdit(box, "newBanned");
	connect(mEntry, SIGNAL(returnPressed()), SLOT(addBanned()));
}

void BanList::addBanned() {
	QString n = mEntry->text();
	mEntry->setText(QString::null);
	
	if(n.isEmpty())
		return;
	
	editComments(n);
}

void BanList::editComments(const QString& n) {
	QString _c;
	UserListItem* item = mUserList->findItem(n);
	if(item)
		_c = item->comments();
	
	QString c = QInputDialog::getText("Comments", "Comments for " + n, QLineEdit::Normal, _c);
	museeq->addBanned(n, c);
}

void BanList::showEvent(QShowEvent*) {
	mEntry->setFocus();
}

void BanList::slotDropSlsk(const QStringList& l) {
	QStringList::const_iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url(*it);
		if(url.protocol() == "slsk" && url.hasHost()) {
			QString user = url.host();
			QUrl::decode(user);
			mUserList->findItem(user);
			editComments(user);
		}
	}
}
