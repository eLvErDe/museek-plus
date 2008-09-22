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

#include "museeq.h"
#include "chatrooms.h"
#include "roomlist.h"
#include "chatroom.h"
#include "images.h"

ChatRooms::ChatRooms(QWidget* parent, const char* name)
          : TabWidget(parent, name) {

	mRoomList = new RoomList(this);
	addTab(mRoomList, tr("*Rooms*"));

	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));

	connect(museeq, SIGNAL(joinedRoom(const QString&, const NRoom&)), SLOT(joined(const QString&, const NRoom&)));
	connect(museeq, SIGNAL(leftRoom(const QString&)), SLOT(left(const QString&)));
	connect(museeq, SIGNAL(saidChatroom(const QString&, const QString&, const QString&)), SLOT(append(const QString&, const QString&, const QString&)));
	connect(museeq, SIGNAL(userJoinedRoom(const QString&, const QString&, const NUserData&)), SLOT(userJoined(const QString&, const QString&, const NUserData&)));
	connect(museeq, SIGNAL(userLeftRoom(const QString&, const QString&)), SLOT(userLeft(const QString&, const QString&)));
	connect(museeq, SIGNAL(roomTickers(const QString&, const NTickers&)), SLOT(setTickers(const QString&, const NTickers&)));
	connect(museeq, SIGNAL(roomTickerSet(const QString&, const QString&, const QString&)), SLOT(setTicker(const QString&, const QString&, const QString&)));
}

void ChatRooms::clear() {
	while(count() > 1)
		delete widget(1);
}

void ChatRooms::joined(const QString& room, const NRoom& r) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->setUsers(r);
			return;
		}
	}

	ChatRoom* _room = new ChatRoom(room, this, false);
	addTab(_room, room);
	connect(_room, SIGNAL(highlight(int, QWidget*)), this, SIGNAL(highlight(int)));
	_room->setUsers(r);
	connect(_room, SIGNAL(highlight(int, QWidget*)), this, SLOT(setHighlight(int, QWidget*)));

	connect(this, SIGNAL(currentChanged(QWidget*)), SLOT(doCurrentChanged(QWidget*)));
}

void ChatRooms::left(const QString& room) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			delete _room;
			return;
		}
	}
}

void ChatRooms::append(const QString& room, const QString& user, const QString& line) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->append(user, line);
			return;
		}
	}
}

void ChatRooms::userJoined(const QString& room, const QString& user, const NUserData& data) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->userJoined(user, data.status, data.speed, data.files);
			return;
		}
	}
}

void ChatRooms::userLeft(const QString& room, const QString& user) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->userLeft(user);
			return;
		}
	}
}

void ChatRooms::closeCurrent() {
	ChatRoom* room = static_cast<ChatRoom*>(currentWidget());
	museeq->leaveRoom(room->room());
}

void ChatRooms::setTickers(const QString& room, const NTickers& tickers) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->setUserTicker(tickers);
			break;
		}
	}
}

void ChatRooms::setTicker(const QString& room, const QString& user, const QString& message) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		if(_room->room() == room) {
			_room->setUserTicker(user, message);
			break;
		}
	}
}
void ChatRooms::updateTickers() {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(widget(ix));
		_room->updateTickers(museeq->mTickerLength);
	}
}
void ChatRooms::doCurrentChanged(QWidget* widget) {
	selected(widget);
}

void ChatRooms::setHighlight(int highlight, QWidget* chatwidget) {

	ChatRoom * uw = static_cast<ChatRoom*>(chatwidget);

	int pos = indexOf(uw);
	if(( currentIndex() != pos) && highlight > uw->highlighted() )
	{
		uw->setHighlighted(highlight);

		if (uw->highlighted() > 0) {
			// Icon on tab
			tabBar()->setTabIcon(pos, QIcon(IMG("online")));
		}
		if (uw->highlighted() > 1) {
			// Red tab
			tabBar()->setTabTextColor(pos, QColor(255, 0, 0));
		}

	} else if (uw->highlighted() == 0){
		tabBar()->setTabTextColor(pos, tabBar()->palette().buttonText().color());
		tabBar()->setTabIcon(pos, QIcon());
	}
}

void ChatRooms::selected(QWidget* chatwidget) {
	if (currentIndex() == 0)
		return;
	ChatRoom * uw = static_cast<ChatRoom*>(chatwidget);
	if(uw->highlighted() != 0) {
		uw->setHighlighted(0);
		setHighlight(uw->highlighted(), uw );
	}
}
