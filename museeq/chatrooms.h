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

#ifndef CHATROOMS_H
#define CHATROOMS_H

#include "museeqtypes.h"
#include "tabwidget.h"

class RoomList;

class ChatRooms : public TabWidget {
	Q_OBJECT
public:
	ChatRooms(QWidget* = 0, const char* = 0);


signals:
	void encodingChanged(const QString&, const QString&);

	void join(const QString&);
	void leave(const QString&);
	void refresh();

	void highlight(int);
	void highlight(int, QWidget* );
public slots:
	void setHighlight(int highlight, QWidget*);
	void selected(QWidget*);
	void updateTickers();
protected slots:
	void clear();
	void closeCurrent();
	void doCurrentChanged(QWidget *);
	void joined(const QString&, const NRoom&, const QString&, const QStringList&);
	void left(const QString&);
	void append(const QString&, const QString&, const QString&);

	void setTicker(const QString&, const QString&, const QString&);
	void setTickers(const QString&, const NTickers&);
	void setTickersForAllRooms(const NTickerMap&);

	void userJoined(const QString&, const QString&, const NUserData&);
	void userLeft(const QString&, const QString&);

	void askedPublicChat();

private:
	RoomList* mRoomList;
	QString mNickname;
};

#endif // CHATROOMS_H
