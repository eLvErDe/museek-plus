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
#include "chattext.h"

#include <QRegExp>
#include <QDateTime>

ChatText::ChatText(const QString& _tf, QWidget* _p)
         : QTextBrowser(_p), mTimeFormat(_tf), mNickname(museeq->nickname()) {

	setReadOnly(true);
	setOpenLinks(false);
	connect(museeq, SIGNAL(nicknameChanged(const QString&)), SLOT(setNickname(const QString&)));
	connect(this, SIGNAL(anchorClicked(const QUrl&)), museeq, SLOT(showURL(const QUrl&)));
}

void ChatText::append(const QString& _u, const QString& _l) {
	append(QDateTime::currentDateTime().toTime_t(), _u, _l);
}

void ChatText::append(const QString& _room, const QString& _u, const QString& _l) {
	append(QDateTime::currentDateTime().toTime_t(), _room, _u, _l);
}

/**
  * Append general information (not specific to a single user)
  */
void ChatText::append(uint ts, const QString& _l) {
	QString line ="";
	QDateTime _t;
	_t.setTime_t(ts);
	if (museeq->mShowTimestamps) {
		if(mTimeFormat.isEmpty())
			line = QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>%1</span>").arg(_t.toString());
		else
			line = QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>%1</span>").arg(_t.toString(mTimeFormat));
	}
	line +=  "<span style='" + museeq->mFontMessage +  ";color:"+museeq->mColorRemote+"'>  * " + _l +"</span>";
	QTextBrowser::append(line);
}

void ChatText::append(uint ts, const QString& _u, const QString& _l) {
    append(ts, QString::null, _u, _l);
}

void ChatText::append(uint ts, const QString& _room, const QString& _u, const QString& _l) {
	QString line,
	        l(_l),
            u(_u.isEmpty() ? mNickname : _u);

	QDateTime _t;
	_t.setTime_t(ts);

	QString colorBanned = museeq->mColorBanned;
	QString colorBuddied =  museeq->mColorBuddied;
	QString colorTrusted =  museeq->mColorTrusted;
	QString colorMe =  museeq->mColorMe;
	QString colorNickname =  museeq->mColorNickname;
	QString colorRemote =  museeq->mColorRemote;
	QString colorTime =  museeq->mColorTime;
	QString fontTime = museeq->mFontTime;
	QString fontMessage = museeq->mFontMessage;
	if (museeq->mShowTimestamps) {
		if(mTimeFormat.isEmpty())
			line = QString("<span style='"+fontTime+";color:"+colorTime+";'>%1</span>").arg(_t.toString());
		else
			line = QString("<span style='"+fontTime+";color:"+colorTime+";'>%1</span>").arg(_t.toString(mTimeFormat));
	} else
		line = "";

    if (!_room.isEmpty()) // For public chat
        line += " <span style='"+fontTime+";color:"+colorTime+";'>[" + Qt::escape(_room) + "]</span> ";

	if(_l.startsWith("/me ")) {
		line += "<span style='" + fontMessage + ";color:"+colorMe+";'> * " + Qt::escape(u) + " ";
		l = _l.mid(4);
	} else if(museeq->nickname() == u) {
		line += " <span style='"+fontTime+";color:"+colorTime+";'>[</span><span style='" + fontMessage + ";color:"+colorNickname+";'>" + Qt::escape(u) + "</span><span style='"+fontTime+";color:"+colorTime+";'>]</span> <span style='" + fontMessage + ";color:"+colorNickname+";'> ";
	} else {
		line += " <span style='"+fontTime+";color:"+colorTime+";'>[</span><span style='" + fontMessage;
		 if(museeq->isBanned(u)) {
			line += ";color:"+colorBanned+";'>";
		} else if(museeq->isTrusted(u)) {
			line += ";color:"+colorTrusted+";'>";
		} else if(museeq->isBuddy(u)) {
			line += ";color:"+colorBuddied+";'>";
		} else
			line += ";color:"+colorRemote+";'>";
		line +=  Qt::escape(u) + "</span><span style='"+fontTime+";color:"+colorTime+";'>]</span><span style='" + fontMessage + ";color:"+colorRemote+";'> ";
	}
	int ix;

	QMapIterator<QString, QString> i(museeq->protocolHandlers());
	QString rx = "(slsk";
	while (i.hasNext()) {
		i.next();
		rx += "|" + i.key();
	}
	rx += "):(//)?[0-9a-zA-Z:\\.]?[0-9a-zA-Z:\\.\\+/?,=%~_$&:#;{}@!\\\\|[\\]'-]+";

	QRegExp url_rx(rx);
	while((ix = url_rx.indexIn(l)) != -1) {
		int len = url_rx.matchedLength();
		QString url = l.mid(ix, len);

		line += postProcess( l.left(ix), _l, u);
		line += "<a href=\"" + url + "\">" + Qt::escape(url) + "</a>";
		l = l.mid(ix + len);
	}

	if(! l.isEmpty()) {
		line += postProcess(l, _l, u)+"</span>";
	}

	QTextBrowser::append(line);
}

QString ChatText::postProcess(const QString& _s, const QString& _l, const QString& _u) {
	if(! mNickname.isNull()) {
		if(_l.startsWith("/me ")) {
			return Qt::escape(_s).replace(mNickname, "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorMe+";'>" + mNickname + "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorMe+";'>");
		} else if(museeq->nickname() == _u) {
			return Qt::escape(_s).replace(mNickname, "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorMe+";'>" + mNickname + "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorNickname+";'>");
		} else
			return Qt::escape(_s).replace(mNickname, "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorMe+";'>" + mNickname + "</span><span style='" + museeq->mFontMessage + ";color:"+museeq->mColorRemote+";'>");
	} else
		return Qt::escape(_s);
}

void ChatText::setNickname(const QString& nickname) {
	mNickname = nickname;
}
