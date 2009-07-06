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

#ifndef CHATROOM_H
#define CHATROOM_H

#include "museeqtypes.h"

#include <QWidget>

class UserListView;
class ChatPanel;
class QTextEdit;
class ChatTicker;
class QCheckBox;

class ChatRoom : public QWidget {
	Q_OBJECT
public:
	ChatRoom(const QString, QWidget * = 0, const QString = "ChatRoom");

	QString room() const;
	int highlighted() const {return mHighlight;};

public slots:
	// Somebody said something
	void append(const QString&, const QString&);

	// Userlist slots
	void setUsers(const NRoom&);
	void setOperators(const QStringList&);
	void setOwner(const QString&);
	void userJoined(const QString&, const NUserData&);
	void userLeft(const QString&);

	// Tickers
	void setUserTicker(const QString&, const QString&);
	void setUserTicker(const NTickers&);
	void showThisTicker();
	void hideThisTicker();
	void updateTickers(uint);
	void setHighlighted(int newH) {mHighlight = newH;};

signals:
	void encodingChanged(const QString&, const QString&);
	void autoJoin(const QString&, bool);

	void highlight(int, QWidget*);

protected slots:
	void setNickname(const QString&);
	void slotAutoJoin(bool);
	void setUserStatus(const QString&, uint);
	void setAutoJoin(const QString&, bool);
	void sendMessage(const QString&);
	void setTicker();
	void logMessage(const QString& , const QString& );
	void logMessage(uint, const QString&, const QString&);
private:
	int mHighlight;
	QString mRoom, mNickname;
	QTextEdit *mLog;
	UserListView *mUserList;
	ChatPanel *mChatPanel;
	ChatTicker *mTicker;
	QCheckBox* mAutoJoin;
	QMap<QString, uint> mStatus;
};

#endif // CHATROOM_H
