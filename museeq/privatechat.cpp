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

#include "privatechat.h"
#include "privatechats.h"
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qtextedit.h>
#include "codeccombo.h"
#include "chatpanel.h"
#include "museeq.h"
#include "aclineedit.h"
PrivateChat::PrivateChat(const QString& _t, QWidget* _p, const char* _n)
            : UserWidget(_t, _p, _n) {
	
	mChatPanel = new ChatPanel(QString::null, this, "panel");
	connect(mChatPanel, SIGNAL(send(const QString&)), this, SLOT(slotSend(const QString&)));
	mStatus = 3;
	new CodecCombo("encoding.users", user(), mChatPanel->box(), "encoding");
	
}

#define escape QStyleSheet::escape

#define _TIME QString("<span style='"+museeq->mFontTime+"'><font color='"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</font></span> "

#define _TIMES QDateTime::currentDateTime().toTime_t ()

void PrivateChat::status(const QString& _u, uint _s) {
	if (_s == mStatus)
		return;
	else
		mStatus = _s;
	if (! museeq->isIgnored(_u)) {
		QString l = ""; 

		if(_s == 0)
			l += tr("%1 is offline").arg(_u);
		else if(_s == 1)
			l += tr("%1 is away").arg(_u);
		else if(_s == 2)
			l += tr("%1 is online").arg(_u);
		if (_s == 0 || _s == 1 || _s == 2)
			mChatPanel->append(_TIMES,  l);
// 		}	
	}
}

void PrivateChat::append(uint dir, uint ts, const QString& _u, const QString& _l) {
	if(dir == 0)
	{
		mChatPanel->append(ts, _u, _l);
		emit highlight(1);
	}
	else if (dir == 1)
	{
		mChatPanel->append(ts, museeq->nickname(), _l);
		emit highlight(1);
	}
}

void PrivateChat::slotSend(const QString& line) {
	if (! line.isEmpty()) {
		museeq->sayPrivate(user(), line);
		mChatPanel->append(QString::null, line);
	}
}
