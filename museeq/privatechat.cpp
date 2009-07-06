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
#include "mainwin.h"
#include "aclineedit.h"
#include "privatechat.h"
#include "privatechats.h"
#include "codeccombo.h"
#include "chatpanel.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#define _TIME QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</span> "

#define _TIMES QDateTime::currentDateTime().toTime_t ()

PrivateChat::PrivateChat(const QString& _t, QWidget* _p, const char* _n)
            : UserWidget(_t, _p, _n) {
	MainLayout = new QVBoxLayout(this);
	mChatPanel = new ChatPanel(QString::null, this);
	MainLayout->addWidget(mChatPanel);
	connect(mChatPanel, SIGNAL(send(const QString&)), this, SLOT(slotSend(const QString&)));
	mStatus = 3;
	mChatPanel->boxLayout()->addWidget(new CodecCombo("encoding.users", user(), mChatPanel->box(), "encoding"));

}

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

		mChatPanel->append(_TIMES,  l);
	}
}

void PrivateChat::append(uint dir, uint ts, const QString& _u, const QString& _l) {
	if(dir == 0)
	{
		mChatPanel->append(ts, _u, _l);

		if (_l.contains(museeq->nickname()))
			emit highlight(2);
		else
			emit highlight(1);
		logMessage(_u, ts, _u, _l);
	}
	else if (dir == 1)
	{
		mChatPanel->append(ts, museeq->nickname(), _l);
		logMessage(_u, ts, museeq->nickname(), _l);
	}
}

/* Write add local timestamp to chat message before writing to disk */
void PrivateChat::logMessage(const QString& user, const QString& speaker, const QString& _l) {

	logMessage(user, _TIMES, speaker, _l);
}
/* Write chat message to disk */
void PrivateChat::logMessage(const QString& user, uint ts, const QString& speaker, const QString& _l) {
	if (! museeq->mLogPrivate) {
		return;
	}
	if (! museeq->mPrivateLogDir.isEmpty() and QDir(museeq->mPrivateLogDir).exists() ) {
		QFile logfile ( museeq->mPrivateLogDir+"/"+user);
		if (! logfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
			museeq->output(QString("Write Error: could not write to: " +museeq->mPrivateLogDir+"/"+user));
			return;
		}
		QDateTime _t;
		_t.setTime_t(ts);
		QTextStream textstream( &logfile );
		textstream << _t.toString() << " [" << speaker << "]\t" << _l << "\n";
		logfile.close();
	} else {
		museeq->output(QString("Write Error: directory doesn't exist: " +museeq->mPrivateLogDir+"/"));
	}
}

void PrivateChat::slotSend(const QString& line_) {
	QString line = line_;
	if (line.isEmpty())
		return;

    if(line.startsWith("//")) {
        line = line.mid(1);
    } else if(line[0] == '/') {
        QStringList s;
        QString cmd("");
        if(line.length() > 1)
        {
            s = line.mid(1).split(' ', QString::KeepEmptyParts);
            cmd = s[0].toLower();
            s.pop_front();
        }

        if(cmd == "me") {
            museeq->sayPrivate(user(), line);
            mChatPanel->append(QString::null, line);
        } else if(cmd == "ip" ) {
            if (s.empty())
                museeq->mainwin()->showIPDialog(user());
            else
                museeq->mainwin()->showIPDialog(s.join(" "));
        } else if((cmd == "c" || cmd == "chat")  &&  s.empty())
            museeq->mainwin()->changeCMode();
        else if(cmd == "pm" || cmd == "private") {
            if(! s.empty())
                museeq->mainwin()->showPrivateChat(s.join(" "));
            else
                museeq->mainwin()->changePMode();
        } else if(cmd == "transfers" || cmd == "transfer")
            museeq->mainwin()->changeTMode();
		else if(cmd == "clear")
			mChatPanel->clear();
        else if(cmd == "s" || cmd == "search") {
            if(! s.empty())
                museeq->mainwin()->startSearch(s.join(" "));
            else
                museeq->mainwin()->changeSMode();
        } else if(cmd == "u" || cmd == "userinfo") {
            if(! s.empty())
                museeq->mainwin()->showUserInfo(s.join(" "));
            else
                museeq->mainwin()->showUserInfo(user());
        } else if(cmd == "b" || cmd == "browse") {
            if(! s.empty())
                museeq->mainwin()->showBrowser(s.join(" "));
            else
                museeq->mainwin()->showBrowser(user());
        } else if(cmd == "commands")
            museeq->mainwin()->displayCommandsDialog();
        else if(cmd == "about")
            museeq->mainwin()->displayAboutDialog();
        else if(cmd == "help")
            museeq->mainwin()->displayHelpDialog();
        else if(cmd == "ban" && s.empty())
            museeq->addBanned(user(), "");
        else if(cmd == "unban" && s.empty())
            museeq->removeBanned(user());
        else if(cmd == "ignore" &&  s.empty())
            museeq->addIgnored(user(), "");
        else if(cmd == "unignore" && s.empty())
            museeq->removeIgnored(user());
        else if(cmd == "buddy" && s.empty())
            museeq->addBuddy(user(), "");
        else if(cmd == "unbuddy" && s.empty())
            museeq->removeBuddy(user());
        else if(cmd == "trust" &&  s.empty())
            museeq->addTrusted(user(), "");
        else if(cmd == "distrust" &&  s.empty())
            museeq->removeTrusted(user());
        else if(cmd == "slap" && s.empty()) {
            museeq->sayPrivate(user(), "/me slaps "+user()+" around with a large trout");
            mChatPanel->append(QString::null, "/me slaps "+user()+" around with a large trout");
        } else if((cmd == "j" || cmd == "join") && ! s.empty())
            museeq->joinRoom(s.join(" "));
        else if((cmd == "jp" || cmd == "joinpriv") && ! s.empty()) // Private room
            museeq->joinRoom(s.join(" "), true);
        else if(cmd == "settings")
            museeq->mainwin()->changeSettings();
        else if(cmd == "log")
            museeq->mainwin()->toggleLog();
        else if(cmd == "ticker" || cmd == "tickers" || cmd == "t")
            museeq->mainwin()->toggleTickers();
        else if(cmd == "colors" || cmd == "fonts" || cmd == "f")
            museeq->mainwin()->changeColors();
		else if(cmd == "p" || cmd== "part" || cmd == "l" || cmd == "leave") {
			if(! s.empty())
				museeq->leaveRoom(s.join(" "));
			}
        else
            mChatPanel->entry()->setText(line);
        return;
        }
    museeq->sayPrivate(user(), line);

    QStringList lines = line.split("\n");
    QStringList::iterator it, end = lines.end();
    for(it = lines.begin(); it != end; ++it) {
        logMessage(user(), museeq->nickname(), *it);
        mChatPanel->append(QString::null, *it);
    }
}
