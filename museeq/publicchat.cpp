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
#include "publicchat.h"
#include "chattext.h"
#include "mainwin.h"

#include <QLayout>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#define _TIME QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</span> "

#define _TIMES QDateTime::currentDateTime().toTime_t ()

PublicChat::PublicChat(QWidget * parent)
        : QWidget(parent), mHighlight(0) {
	QVBoxLayout * mainlayout = new QVBoxLayout(this);

	mChatText = new ChatText("hh:mm:ss", this);
	mainlayout->addWidget(mChatText);

	connect(museeq, SIGNAL(receivedPublicChat(const QString&, const QString&, const QString&)), SLOT(append(const QString&, const QString&, const QString&)));
}

void PublicChat::append(const QString& _r, const QString& _u, const QString& _l) {
	mChatText->append(_r, _u, _l);

	emit highlight(1, this);

	logMessage(_r, _u, _l);
}

/* Write add local timestamp to chat message before writing to disk */
void PublicChat::logMessage(const QString& room, const QString& user, const QString& _l) {

	logMessage(_TIMES, room, user, _l);
}

/* Write chat message to disk */
void PublicChat::logMessage(uint ts, const QString& room, const QString& user, const QString& _l) {
	if (! museeq->mLogRooms) {
		return;
	}
	if (! museeq->mRoomLogDir.isEmpty() and QDir(museeq->mRoomLogDir).exists() ) {
		QFile logfile ( museeq->mRoomLogDir+"/PublicChat.log");
		if (! logfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
			museeq->output(QString("Write Error: could not write to: " +museeq->mRoomLogDir+"/PublicChat.log"));
			return;
		}
		QDateTime _t;
		_t.setTime_t(ts);
		QTextStream textstream( &logfile );
		textstream << _t.toString() << " [" << room << "]\t" << " [" << user << "]\t" << _l << "\n";
		logfile.close();
	} else {
		museeq->output(QString("Write Error: directory doesn't exist: " +museeq->mRoomLogDir+"/"));
	}

}

