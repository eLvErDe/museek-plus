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

#include "chatroom.h"

#include <qsplitter.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qradiobutton.h>
#include "chatpanel.h"
#include "userlistview.h"
#include "chatticker.h"
#include "codeccombo.h"
#include "aclineedit.h"
#include "tickerdialog.h"
#include "museeq.h"
#include "mainwin.h"

ChatRoom::ChatRoom(const char * _r, QWidget * parent, const char * name) 
        : QVBox(parent, name), mRoom(_r), mNickname(museeq->nickname()) {
	
	mTicker = new ChatTicker(this, "ticker");
	connect(mTicker, SIGNAL(clicked()), SLOT(setTicker()));
	if ( ! museeq->mShowTickers == true )
		mTicker->hide();
	QSplitter *hsplit = new QSplitter(this);
	
	QSplitter *split = new QSplitter(hsplit);
	hsplit->setCollapsible(split, false);
	split->setOrientation(QSplitter::Vertical);
	
	mLog = new QTextEdit(split, "log");
	split->setResizeMode(mLog, QSplitter::KeepSize);
	mLog->setReadOnly(true);
	mLog->setTextFormat(Qt::RichText);
	mLog->setFocusPolicy(NoFocus);
	mLog->resize(0, 100);
	
	mChatPanel = new ChatPanel("hh:mm:ss", split, "panel");
	split->setCollapsible(mChatPanel, false);
	connect(mChatPanel, SIGNAL(send(const QString&)), SLOT(sendMessage(const QString&)));
	
	QVBox* vbox = new QVBox(hsplit);
	hsplit->setResizeMode(vbox, QSplitter::KeepSize);
	vbox->setSpacing(2);
	
	mUserList = new UserListView(false, vbox, "userlist");
	mUserList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	
	QHBox* hbox = new QHBox(vbox);
	mAutoJoin = new QCheckBox(tr("Auto-join"), hbox, "autojoin");
	if(museeq->isAutoJoined(mRoom))
		mAutoJoin->toggle();
	connect(mAutoJoin, SIGNAL(toggled(bool)), SLOT(slotAutoJoin(bool)));
	
	new CodecCombo("encoding.rooms", mRoom, vbox, "encoding");
	
	connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(setUserStatus(const QString&, uint)));
	connect(museeq, SIGNAL(nicknameChanged(const QString&)), SLOT(setNickname(const QString&)));
	connect(museeq, SIGNAL(autoJoin(const QString&, bool)), SLOT(setAutoJoin(const QString&, bool)));
	connect(museeq, SIGNAL(disconnectedFromServer()), mUserList, SLOT(clear()));
	connect(museeq, SIGNAL(hideAllTickers()), SLOT(hideThisTicker()));
	connect(museeq, SIGNAL(showAllTickers()), SLOT(showThisTicker()));
}

QString ChatRoom::room() const {
	return mRoom;
}

void ChatRoom::sendMessage(const QString& line_) {
	QString line = line_;
	if(line.isEmpty())
		return;
	if(line.startsWith("//"))
		line = line.mid(1);
	else if(line[0] == '/') {
		QStringList s;
		QString cmd("");
		if(line.length() > 1)
		{
			s = QStringList::split(' ', line.mid(1));
			cmd = s[0].lower();
			s.pop_front();
		}
		
		if(cmd == "me")
			museeq->sayRoom(mRoom, line);
		else if(cmd == "ip" && ! s.empty())
			museeq->mainwin()->showIPDialog(s.join(" "));
		else if((cmd == "c" || cmd == "chat")  &&  s.empty())
			museeq->mainwin()->changeCMode();
		else if(cmd == "pm" || cmd == "private")
			if(! s.empty())
				museeq->mainwin()->showPrivateChat(s.join(" "));
			else
				museeq->mainwin()->changePMode();
		else if(cmd == "transfers" || cmd == "transfer")
			museeq->mainwin()->changeTMode();
		 else if(cmd == "s" || cmd == "search")
			if(! s.empty())
				museeq->mainwin()->startSearch(s.join(" "));
			else
				museeq->mainwin()->changeSMode();
		else if(cmd == "u" || cmd == "userinfo")
			if(! s.empty())
				museeq->mainwin()->showUserInfo(s.join(" "));
			else
				museeq->mainwin()->changeUMode();
		else if(cmd == "b" || cmd == "browse")
			if(! s.empty())
				museeq->mainwin()->showBrowser(s.join(" "));
			else
				museeq->mainwin()->changeBMode();
		else if(cmd == "commands")
			museeq->mainwin()->displayCommandsDialog();
		else if(cmd == "about")
			museeq->mainwin()->displayAboutDialog();
		else if(cmd == "help")
			museeq->mainwin()->displayHelpDialog();
		else if(cmd == "ban" && ! s.empty())
			museeq->addBanned(s.join(" "), "");
		else if(cmd == "unban" && ! s.empty())
			museeq->removeBanned(s.join(" "));
		else if(cmd == "ignore" && ! s.empty())
			museeq->addIgnored(s.join(" "), "");
		else if(cmd == "unignore" && ! s.empty())
			museeq->removeIgnored(s.join(" "));
		else if(cmd == "buddy" && ! s.empty())
			museeq->addBuddy(s.join(" "), "");
		else if(cmd == "unbuddy" && ! s.empty())
			museeq->removeBuddy(s.join(" "));
		else if(cmd == "trust" && ! s.empty())
			museeq->addTrusted(s.join(" "), "");
		else if(cmd == "distrust" && ! s.empty())
			museeq->removeTrusted(s.join(" "));
		else if(cmd == "slap" && ! s.empty())
			museeq->sayRoom(mRoom, "/me slaps "+s.join(" ")+" around with a large trout");
		else if((cmd == "j" || cmd == "join") && ! s.empty())
			museeq->joinRoom(s.join(" "));
		else if(cmd == "settings")
			museeq->mainwin()->changeSettings();
		else if(cmd == "log")
			museeq->mainwin()->toggleLog();
		else if(cmd == "ticker" || cmd == "tickers" || cmd == "t") 
			museeq->mainwin()->toggleTickers();
		else if(cmd == "colors" || cmd == "fonts" || cmd == "c" || cmd == "f") 
			museeq->mainwin()->changeColors();
		else if(cmd == "p" || cmd== "part" || cmd == "l" || cmd == "leave") {
			if(! s.empty())
				museeq->leaveRoom(s.join(" "));
			else
				museeq->leaveRoom(mRoom);
			}
		else
			mChatPanel->entry()->setText(line);
		return;
	}
	museeq->sayRoom(mRoom, line);
}

void ChatRoom::append(const QString& _u, const QString& _l) {
	mChatPanel->append(_u, _l);
	if (mNickname == _u);
	else if(mNickname.isNull() || _l.find(mNickname) == -1)
		emit highlight(1);
	else
		emit highlight(2);
}

#define escape QStyleSheet::escape

void ChatRoom::setUsers(const NRoom& r) {
	mUserList->clear();
	mStatus.clear();
	
	QStringList users;
	
	NRoom::const_iterator it = r.begin();
	for(; it != r.end(); ++it) {
		users << it.key();
		mStatus[it.key()] = (*it).status;
		mUserList->add(it.key(), (*it).status, (*it).speed, (*it).files, "");
		museeq->flush();
	}
	
	mChatPanel->entry()->setCompletors(users);
}

#define _TIME QString("<span style='"+museeq->mFontTime+"'><font color='"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</font></span> "
void ChatRoom::userJoined(const QString& _u, int _s, unsigned int _sp, unsigned int _f) {
	if(mUserList->findItem(_u))
		return;
	if (! museeq->isIgnored(_u)) {
		if (museeq->mShowTimestamps)
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("%1 joined the room")+"</font></span>").arg(escape(_u)));
		else
			mLog->append( QString("<span style='"+museeq->mFontMessage+"'><font color='" + museeq->mColorRemote + "'>"+ tr("%1 joined the room")+"</font></span>" ).arg(escape(_u) ) ) ;
	}
	if(museeq->isBuddy(_u)) {
		mUserList->add(_u , _s, _sp, _f);
	} else if(museeq->isBanned(_u)) {
		mUserList->add(_u, _s, _sp, _f);
	} else {
		mUserList->add(_u, _s, _sp, _f);
	}
	
	
	mStatus[_u] = _s;
	
	mChatPanel->entry()->addCompletor(_u);
}

void ChatRoom::userLeft(const QString& _u) {
	if(! mUserList->findItem(_u))
		return;
	mStatus.remove(_u);
	if (! museeq->isIgnored(_u))  {
		if (museeq->mShowTimestamps) 
			mLog->append(QString(_TIME+"</font></span><span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("%1 left the room")+"</font></span>").arg(escape(_u)));
		else
			mLog->append(QString("</font></span><span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("%1 left the room")+"</font></span>").arg(escape(_u)));
	}
	mUserList->remove(_u);
	
	mChatPanel->entry()->removeCompletor(_u);
}

void ChatRoom::setUserStatus(const QString& _u, uint _s) {
	QMap<QString, uint>::const_iterator it = mStatus.find(_u);
	if(it == mStatus.end())
		return;
	
	uint old_s = *it;
	if (! museeq->isIgnored(_u)) {
		if(old_s > 0 && _s != old_s) {
			QString l = "";
			if (museeq->mShowTimestamps)
				l += _TIME;
			
			if(_s == 1)
				l += QString("<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("%1 has gone away")+"</font></span>").arg(escape(_u));
			else if(_s == 2)
				l += QString("<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("%1 has returned")+"</font></span>").arg(escape(_u));
		mLog->append(l);
		mStatus[_u] = _s;
		}	
	}
}

void ChatRoom::setUserTicker(const QString& _u, const QString& _m) {
	mTicker->setText(_u, _m);
}

void ChatRoom::setUserTicker(const NTickers& _t) {
	mTicker->setText(_t);
}

void ChatRoom::setNickname(const QString& nickname) {
	mNickname = nickname;
}

void ChatRoom::slotAutoJoin(bool on) {
	museeq->setAutoJoin(mRoom, on);
}

void ChatRoom::showThisTicker() {
	mTicker->show();
}
void ChatRoom::hideThisTicker() {
	mTicker->hide();
}

void ChatRoom::setAutoJoin(const QString& room, bool on) {
	if(mRoom == room && mAutoJoin->isOn() != on)
		mAutoJoin->toggle();
}

void ChatRoom::setTicker() {
	TickerDialog * dlg = new TickerDialog();
	if(dlg->exec() == QDialog::Accepted)
	{
		if (dlg->mThisTime->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
		} else if (dlg->mAlways->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
			museeq->setConfig("tickers", mRoom, msg);
		} else if (dlg->mDefault->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
			museeq->setConfig("default-ticker", "ticker", msg);
		}
		
	}
	delete dlg;
	
}
