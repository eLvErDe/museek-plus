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
#include "chatpanel.h"
#include "chattext.h"
#include "aclineedit.h"

ChatPanel::ChatPanel(const QString& _tf, QWidget* _p)
         : QWidget(_p) {
	QVBoxLayout * MainLayout = new QVBoxLayout(this);
	MainLayout->setMargin(0);
	mScroll = new ChatText(_tf, this);
	MainLayout->addWidget(mScroll);

	mBox = new QWidget(this);
	MainLayout->addWidget(mBox);
	BoxLayout = new QHBoxLayout(mBox);
	BoxLayout->setMargin(0);
	mEntry = new ACLineEdit(mBox);
	mEntry->setEnabled(museeq->isConnected());
	BoxLayout->addWidget(mEntry);

	connect(mEntry, SIGNAL(returnPressed()), SLOT(slotSendMessage()));
	connect(museeq, SIGNAL(disconnectedFromServer()), SLOT(slotDisconnected()));
	connect(museeq, SIGNAL(connectedToServer()), SLOT(slotConnected()));
}

void ChatPanel::append(const QString& _u, const QString& _l) {
	mScroll->append(_u, _l);
	if(! _u.isEmpty())
		mEntry->pushCompletor(_u);

}

void ChatPanel::append(uint _ts, const QString& _l) {
	mScroll->append(_ts, _l);

}

void ChatPanel::append(uint _ts, const QString& _u, const QString& _l) {
	mScroll->append(_ts, _u, _l);

}

void ChatPanel::clear() {
	mScroll->clear();
}

void ChatPanel::slotSendMessage() {
	QString line = mEntry->text();
	mEntry->reset();
	emit send(line);
}

void ChatPanel::showEvent(QShowEvent*) {
 	mEntry->setFocus();
}

void ChatPanel::slotDisconnected() {
	mEntry->setDisabled(true);
	append("-- disconnected from server --", QString::null);
}

void ChatPanel::slotConnected() {
	mEntry->setEnabled(true);
	append("-- connected to server --", QString::null);
}
