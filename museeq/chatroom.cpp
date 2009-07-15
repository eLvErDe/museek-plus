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
#include "chatroom.h"
#include "chatpanel.h"
#include "userlistview.h"
#include "chatticker.h"
#include "codeccombo.h"
#include "aclineedit.h"
#include "tickerdialog.h"
#include "mainwin.h"

#include <QSplitter>
#include <QTextEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QLayout>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QSettings>

#define _TIME QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</span> "

#define _TIMES QDateTime::currentDateTime().toTime_t ()

ChatRoom::ChatRoom(const QString _r, QWidget * parent, const QString name)
        : QWidget(parent), mHighlight(0), mRoom(_r), mNickname(museeq->nickname()) {
	QVBoxLayout * mainlayout = new QVBoxLayout(this);
	mTicker = new ChatTicker(this, "ticker", museeq->mTickerLength);
	connect(mTicker, SIGNAL(clicked()), SLOT(setTicker()));
	if ( ! museeq->settings()->value("showTickers", true).toBool() )
		mTicker->hide();
	mainlayout->addWidget(mTicker);

	mainlayout->setMargin(2);
	mainlayout->setSpacing(5);
	QSplitter *hsplit = new QSplitter(this);

	mainlayout->addWidget(hsplit, 10);
	QSplitter *split = new QSplitter(hsplit);
	hsplit->setCollapsible(hsplit->indexOf(split), false);
	split->setOrientation(Qt::Vertical);

	mLog = new QTextEdit(split);
	split->setStretchFactor(split->indexOf(mLog), 0);

	mLog->setReadOnly(true);
	mLog->setAcceptRichText(true);
	mLog->setFocusPolicy(Qt::NoFocus);
	mLog->resize(0, 100);

	mChatPanel = new ChatPanel("hh:mm:ss", split);
	split->setCollapsible(split->indexOf(mChatPanel), false);
	split->setStretchFactor(0,1);
	split->setStretchFactor(1,10);
	connect(mChatPanel, SIGNAL(send(const QString&)), SLOT(sendMessage(const QString&)));
	QWidget* rightSide = new QWidget(hsplit);
	QVBoxLayout* vbox = new QVBoxLayout(rightSide);

	vbox->setSpacing(2);
	vbox->setMargin(2);
	mUserList = new UserListView(false, rightSide, "roomsUserlist");
	vbox->addWidget(mUserList);
	mUserList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	QHBoxLayout* hbox = new QHBoxLayout;
	vbox->addLayout(hbox);
	mAutoJoin = new QCheckBox(tr("Auto-join"), rightSide);
	hbox->addWidget(mAutoJoin);
	if(museeq->isAutoJoined(mRoom))
		mAutoJoin->toggle();
	connect(mAutoJoin, SIGNAL(toggled(bool)), SLOT(slotAutoJoin(bool)));

	vbox->addWidget(new CodecCombo("encoding.rooms", mRoom, rightSide, "encoding"));

	connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(setUserStatus(const QString&, uint)));
	connect(museeq, SIGNAL(nicknameChanged(const QString&)), SLOT(setNickname(const QString&)));
	connect(museeq, SIGNAL(autoJoin(const QString&, bool)), SLOT(setAutoJoin(const QString&, bool)));
	connect(museeq, SIGNAL(disconnectedFromServer()), mUserList, SLOT(clear()));
	connect(museeq->mainwin(), SIGNAL(hideAllTickers()), SLOT(hideThisTicker()));
	connect(museeq->mainwin(), SIGNAL(showAllTickers()), SLOT(showThisTicker()));
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
			s = line.mid(1).split(' ', QString::KeepEmptyParts);
			cmd = s[0].toLower();
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
		else if(cmd == "clear")
			mChatPanel->clear();
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
		else if(cmd == "setticker" && ! s.empty())
			museeq->setTicker(mRoom, s.join(" "));
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
		else if(cmd == "colors" || cmd == "fonts" || cmd == "f")
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
	else if(mNickname.isNull() || !_l.contains(mNickname) ) {
		emit highlight(1, this);
	} else {
		emit highlight(2, this);
    }
	logMessage(_u, _l);
}

/* Write add local timestamp to chat message before writing to disk */
void ChatRoom::logMessage(const QString& user, const QString& _l) {

	logMessage(_TIMES, user, _l);
}

/* Write chat message to disk */
void ChatRoom::logMessage(uint ts, const QString& user, const QString& _l) {
	if (! museeq->mLogRooms) {
		return;
	}
	if (! museeq->mRoomLogDir.isEmpty() and QDir(museeq->mRoomLogDir).exists() ) {
		QFile logfile ( museeq->mRoomLogDir+"/"+mRoom);
		if (! logfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
			museeq->output(QString("Write Error: could not write to: " +museeq->mRoomLogDir+"/"+mRoom));
			return;
		}
		QDateTime _t;
		_t.setTime_t(ts);
		QTextStream textstream( &logfile );
		textstream << _t.toString() << " [" << user << "]\t" << _l << "\n";
		logfile.close();
	} else {
		museeq->output(QString("Write Error: directory doesn't exist: " +museeq->mRoomLogDir+"/"));
	}

}

void ChatRoom::setUsers(const NRoom& r) {
	mUserList->clear();
	mStatus.clear();

	QStringList users;
	mUserList->sorting(false);
	NRoom::const_iterator it = r.begin();
	for(; it != r.end(); ++it) {
		users << it.key();
		mStatus[it.key()] = (*it).status;
		mUserList->add(it.key(), (*it).status, (*it).speed, (*it).files, "", it->country);
		museeq->flush();
	}
	mUserList->sorting(true);
	mChatPanel->entry()->setCompletors(users);
}

void ChatRoom::setOperators(const QStringList& ops) {
    QStringList::const_iterator it = ops.begin();
    for (; it != ops.end(); it++) {
        mUserList->setOperator(*it, true);
    }
}

void ChatRoom::setOwner(const QString& ow) {
    mUserList->setOwner(ow, true);
}

void ChatRoom::userJoined(const QString& _u, const NUserData& _data) {
	if(mUserList->findItem(_u))
		return;
	if (! museeq->isIgnored(_u)) {
		if (museeq->mShowTimestamps)
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("%1 joined the room")+"</span>").arg(Qt::escape(_u)));
		else
			mLog->append( QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+ tr("%1 joined the room")+"</span>" ).arg(Qt::escape(_u) ) ) ;
	}

	mUserList->add(_u , _data.status, _data.speed, _data.files, QString::null, _data.country);

	mStatus[_u] = _data.status;

	mChatPanel->entry()->addCompletor(_u);
}

void ChatRoom::userLeft(const QString& _u) {
	if(! mUserList->findItem(_u))
		return;
	mStatus.remove(_u);
	if (! museeq->isIgnored(_u))  {
		if (museeq->mShowTimestamps)
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("%1 left the room")+"</span>").arg(Qt::escape(_u)));
		else
			mLog->append(QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("%1 left the room")+"</span>").arg(Qt::escape(_u)));
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
				l += QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("%1 has gone away")+"</span>").arg(Qt::escape(_u));
			else if(_s == 2)
				l += QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("%1 has returned")+"</span>").arg(Qt::escape(_u));
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

void ChatRoom::updateTickers(uint size) {
	mTicker->setSize(size);
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
	if(mRoom == room && mAutoJoin->isChecked() != on)
		mAutoJoin->toggle();
}

void ChatRoom::setTicker() {
	TickerDialog * dlg = new TickerDialog(museeq->mainwin());
	QString ticker;
	if (mTicker->tickers().contains(museeq->nickname())) {
		ticker = mTicker->tickers()[museeq->nickname()];
		dlg->mMessage->setText(ticker);
	}
	if(dlg->exec() == QDialog::Accepted)
	{
		if (dlg->mThisTime->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
		} else if (dlg->mAlways->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
			if(msg.isEmpty())
				museeq->removeConfig("tickers", mRoom);
			else
				museeq->setConfig("tickers", mRoom, msg);
		} else if (dlg->mDefault->isChecked()) {
			QString msg = dlg->mMessage->text();
			museeq->setTicker(mRoom, msg);
			if(msg.isEmpty())
				museeq->removeConfig("default-ticker", "ticker");
			else
				museeq->setConfig("default-ticker", "ticker", msg);
		}

	}
	delete dlg;

}


