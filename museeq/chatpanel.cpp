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

#include "chatpanel.h"

#include <qstylesheet.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include "chattext.h"
#include "aclineedit.h"
#include "museeq.h"

ChatPanel::ChatPanel(const QString& _tf, QWidget* _p, const char* _n)
         : QVBox(_p, _n) {
	
	mScroll = new ChatText(_tf, this, "text");
	
	mBox = new QHBox(this);
	
	mEntry = new ACLineEdit(mBox, "line");
	mEntry->setEnabled(museeq->isConnected());
	
	connect(mEntry, SIGNAL(returnPressed()), SLOT(slotSendMessage()));
	connect(museeq, SIGNAL(disconnectedFromServer()), SLOT(slotDisconnected()));
	connect(museeq, SIGNAL(connectedToServer()), SLOT(slotConnected()));
}

void ChatPanel::append(const QString& _u, const QString& _l) {
	mScroll->append(_u, _l);
	if(! _u.isEmpty())
		mEntry->pushCompletor(_u);

}

void ChatPanel::append(uint _ts, const QString& _u, const QString& _l) {
	mScroll->append(_ts, _u, _l);

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
