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

#include "chatrooms.h"

#include <qtoolbutton.h>

#include "roomlist.h"
#include "chatroom.h"
#include "museeq.h"

ChatRooms::ChatRooms(QWidget* parent, const char* name)
          : TabWidget(parent, name) {
	
	mRoomList = new RoomList(this);
	addTab(mRoomList, "*Rooms*");
	
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
		delete page(1);
}

void ChatRooms::joined(const QString& room, const NRoom& r) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->setUsers(r);
			return;
		}
	}
	
	ChatRoom* _room = new ChatRoom(room);
	connect(_room, SIGNAL(highlight(int)), SIGNAL(highlight(int)));
	_room->setUsers(r);
	Tab* new_tab = new Tab(this, room);
	connect(_room, SIGNAL(highlight(int)), new_tab, SLOT(setHighlight(int)));
	addTab(_room, new_tab);
}

void ChatRooms::left(const QString& room) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			delete _room;
			return;
		}
	}
}

void ChatRooms::append(const QString& room, const QString& user, const QString& line) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->append(user, line);
			return;
		}
	}
}

void ChatRooms::userJoined(const QString& room, const QString& user, const NUserData& data) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->userJoined(user, data.status, data.speed, data.files);
			return;
		}
	}
}

void ChatRooms::userLeft(const QString& room, const QString& user) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->userLeft(user);
			return;
		}
	}
}

void ChatRooms::closeCurrent() {
	ChatRoom* room = static_cast<ChatRoom*>(currentPage());
	museeq->leaveRoom(room->room());
}

void ChatRooms::setTickers(const QString& room, const NTickers& tickers) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->setUserTicker(tickers);
			break;
		}
	}
}

void ChatRooms::setTicker(const QString& room, const QString& user, const QString& message) {
	for(int ix = 1; ix < count(); ++ix) {
		ChatRoom* _room = static_cast<ChatRoom*>(page(ix));
		if(_room->room() == room) {
			_room->setUserTicker(user, message);
			break;
		}
	}
}
