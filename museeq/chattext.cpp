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

#include "chattext.h"

#include <qregexp.h>
#include "museeq.h"

ChatText::ChatText(const QString& _tf, QWidget* _p, const char* _n)
         : QTextBrowser(_p, _n), mTimeFormat(_tf), mNickname(museeq->nickname()) {
	
	setReadOnly(true);
	setTextFormat(Qt::RichText);
// 	setFocusPolicy(NoFocus);
	connect(museeq, SIGNAL(nicknameChanged(const QString&)), SLOT(setNickname(const QString&)));
	connect(this, SIGNAL(linkClicked(const QString&)), museeq, SLOT(showURL(const QString&)));
}

void ChatText::append(const QString& _u, const QString& _l) {
	append(QDateTime::currentDateTime().toTime_t(), _u, _l);
}

void ChatText::append(uint ts,  const QString& _l) {
	QString line ="";
	QDateTime _t;
	_t.setTime_t(ts);
	if (museeq->mShowTimestamps) {
		if(mTimeFormat.isEmpty()) 
		
			line = QString("<span style='"+museeq->mFontTime+"'><font color='"+museeq->mColorTime+"'>%1</font></span>").arg(_t.toString());
		else
			line = QString("<span style='"+museeq->mFontTime+"'><font color='"+museeq->mColorTime+"'>%1</font></span>").arg(_t.toString(mTimeFormat));
	}
	line +=  "<span style='" + museeq->mFontMessage +  "'><font color='"+museeq->mColorNickname+"'>  * " + _l +"</font></span>";
	QTextBrowser::append(line);
}

#define escape QStyleSheet::escape
static QRegExp url_rx("[a-zA-Z\\-]+:(//)?[0-9a-zA-Z:\\.]?[0-9a-zA-Z:\\.\\+/?,=%~_$&:#;{}@!\\\\|[\\]'-]+");

void ChatText::append(uint ts, const QString& _u, const QString& _l) {
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
			line = QString("<span style='"+fontTime+"'><font color='"+colorTime+"'>%1</font></span>").arg(_t.toString());
		else
			line = QString("<span style='"+fontTime+"'><font color='"+colorTime+"'>%1</font></span>").arg(_t.toString(mTimeFormat));
	} else 
		line = "";

	if(_l.startsWith("/me ")) {
		line += "<span style='" + fontMessage + "'><font color='"+colorMe+"'> * " + escape(u) + " ";
		l = _l.mid(4);
	} else if(museeq->nickname() == _u) {
		line += " <span style='"+fontTime+"'><font color='"+colorTime+"'>[</font></span><span style='" + fontMessage + "'><font color='"+colorNickname+"'>" + escape(u) + "</font></span><span style='"+fontTime+"'><font color='"+colorTime+"'>]</font></span> <span style='" + fontMessage +  "'><font color='"+colorNickname+"'> ";
	} else {
		line += " <span style='"+fontTime+"'><font color='"+colorTime+"'>[</font></span><span style='" + fontMessage + "'>";
		 if(museeq->isBanned(_u)) {
			line += "<font color='"+colorBanned+"'>";
		} else if(museeq->isTrusted(_u)) {
			line += "<font color='"+colorTrusted+"'>";
		} else if(museeq->isBuddy(_u)) {
			line += "<font color='"+colorBuddied+"'>";
		} else
			line += "<font color='"+colorRemote+"'>";
		line +=  escape(u) + "</font></span><span style='"+fontTime+"'><font color='"+colorTime+"'>]</font></span><span style='" + fontMessage + "'><font color='"+colorRemote+"'> ";
	}
	int ix;
	while((ix = url_rx.match(l)) != -1) {
		int len = url_rx.matchedLength();
		QString url = l.mid(ix, len);

		line += postProcess( l.left(ix), _l, _u); 
		line += "<a href=\"" + url + "\">" + escape(url) + "</a>";
		l = l.mid(ix + len);
	}

	if(! l.isEmpty()) {
		line += postProcess(l, _l, _u)+"</font></span>";
	}

	QTextBrowser::append(line);
}

QString ChatText::postProcess(const QString& _s, const QString& _l, const QString& _u) {
	if(! mNickname.isNull()) {
		if(_l.startsWith("/me ")) {
			return escape(_s).replace(mNickname, "</font><font color='"+ museeq->mColorMe+"'>" + mNickname + "</font><font color='"+ museeq->mColorMe+"'>");
		} else if(museeq->nickname() == _u) {
			return escape(_s).replace(mNickname, "</font><font color='"+ museeq->mColorMe+"'>" + mNickname + "</font><font color='"+ museeq->mColorNickname+"'>");
		} else
			return escape(_s).replace(mNickname, "</font><font color='"+ museeq->mColorMe+"'>" + mNickname + "</font><font color='"+ museeq->mColorRemote+"'>");
	} else
		return escape(_s);
}

void ChatText::setNickname(const QString& nickname) {
	mNickname = nickname;
}
