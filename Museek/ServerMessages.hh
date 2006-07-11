/* Museek - Museek's 'core' library
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

#ifndef __SERVERMESSAGES_HH__
#define __SERVERMESSAGES_HH__

#include <Museek/Messages.hh>

class ServerMessage : public NetworkMessage {};
#define SERVERMESSAGE(mtype, m_id) NETWORKMESSAGE(ServerMessage, mtype, m_id)

#define SINTEGERMESSAGE(mtype, m_id) \
	SERVERMESSAGE(mtype, m_id) \
		mtype() {}; \
		mtype(uint32 v) { value = v; }; \
		MAKE \
			pack(value); \
		END_MAKE \
		PARSE \
			value = unpack_int(); \
		END_PARSE \
		uint32 value; \
	END

#define SSTRINGMESSAGE(mtype, m_id) \
	SERVERMESSAGE(mtype, m_id) \
		mtype() {}; \
		mtype(const std::string& v) : value(v) {}; \
		MAKE \
			pack(value); \
		END_MAKE \
		PARSE \
			value = unpack_string(); \
		END_PARSE \
		std::string value;\
	END

#define SSTRINGSMESSAGE(mtype, m_id) \
	SERVERMESSAGE(mtype, m_id) \
		mtype() {}; \
		mtype(const std::vector<std::string>& v) : values(v) {}; \
		MAKE \
			pack((uint32)values.size()); \
			std::vector<std::string>::iterator it = values.begin(); \
			for(; it != values.end(); ++it) \
				pack(*it); \
		END_MAKE \
		PARSE \
			values.clear(); \
			uint32 j = unpack_int(); \
			while(j) { \
				values.push_back(unpack_string()); \
				j--; \
			} \
		END_PARSE \
		std::vector<std::string> values; \
	END

SERVERMESSAGE(SLogin, 1)
	SLogin(const std::string& _username = "", const std::string& _password = "") :
		greet(""), username(_username), password(_password)
		{ success = 0; };

	MAKE
		pack(username);
		pack(password);
		pack((uint32)182);
		return buffer;
	END_MAKE

	PARSE
		buffer = data;
		success = unpack_char();
		greet = unpack_string();
	END_PARSE

	std::string greet, username, password;
	uchar success;
END


SINTEGERMESSAGE(SSetListenPort, 2)

SERVERMESSAGE(SGetPeerAddress, 3)
	SGetPeerAddress(const std::string& _u = "") : user(_u) {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		ip = unpack_ip();
		port = unpack_int();
	END_PARSE

	std::string user, ip;
	uint32 port;
END

SERVERMESSAGE(SAddUser, 5)
	SAddUser(const std::string& _u = "") : user(_u) {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		exists = unpack_char();
	END_PARSE

	std::string user;
	bool exists;
END

SERVERMESSAGE(SGetStatus, 7)
	SGetStatus(const std::string& _u) : user(_u) {};
	SGetStatus() {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		status = unpack_int();
	END_PARSE

	std::string user;
	uint32 status;
END

SERVERMESSAGE(SSayChatroom, 13)
	SSayChatroom(const std::string& r = "", const std::string& l = ""): room(r), line(l) {};

	MAKE
		pack(room);
		pack(line);
	END_MAKE

	PARSE
		room = unpack_string();
		user = unpack_string();
		line = unpack_string();
	END_PARSE

	std::string room, user, line;
END

SERVERMESSAGE(SJoinRoom, 14)
	SJoinRoom(const std::string& r = "") : room(r) {};
	MAKE
		pack(room);
	END_MAKE

	PARSE
		room = unpack_string();
		uint32 n = unpack_int();
		std::vector<std::string> _u;
		for(uint32 i = 0; i < n; i++)
			_u.push_back(unpack_string());
		std::vector<UserData> _d;
		unpack_int();
		for(uint32 i = 0; i < n; i++) {
			UserData _data;
			_data.status = unpack_int();
			_d.push_back(_data);
		}
		unpack_int();
		std::vector<UserData>::iterator it = _d.begin();
		for(; it != _d.end(); ++it) {
			(*it).avgspeed = unpack_int();
			(*it).downloadnum = unpack_int();
			unpack_int();
			(*it).files = unpack_int();
			(*it).dirs = unpack_int();
		}
		for(it = _d.begin(); it != _d.end(); ++it)
			(*it).slotsfull = unpack_int();

		it = _d.begin();
		std::vector<std::string>::iterator sit = _u.begin();
		for(; it != _d.end(); ++it, ++sit )
			users[*sit] = *it;
	END_PARSE

	std::string room;
	RoomData users;
END

SSTRINGMESSAGE(SLeaveRoom, 15)

SERVERMESSAGE(SUserJoinedRoom, 16)
	PARSE
		room = unpack_string();
		user = unpack_string();
		userdata.status = unpack_int();
		userdata.avgspeed = unpack_int();
		userdata.downloadnum = unpack_int();
		unpack_int();
		userdata.files = unpack_int();
		userdata.dirs = unpack_int();
		userdata.slotsfull = unpack_int();
	END_PARSE

	std::string room, user;
	UserData userdata;
END

SERVERMESSAGE(SUserLeftRoom, 17)
	PARSE
		room = unpack_string();
		user = unpack_string();
	END_PARSE

	std::string room, user;
END

SERVERMESSAGE(SConnectToPeer, 18)
	SConnectToPeer() {};
	SConnectToPeer(uint32 _tok, const std::string& _u, const std::string& _t) : user(_u), type(_t) { token = _tok; };

	MAKE
		pack(token);
		pack(user);
		pack(type);
	END_MAKE

	PARSE
		user = unpack_string();
		type = unpack_string();
		ip = unpack_ip();
		port = unpack_int();
		token = unpack_int();
	END_PARSE

	std::string user, type, ip;
	uint32 port, token;
END

SERVERMESSAGE(SPrivateMessage, 22)
	SPrivateMessage() {};
	SPrivateMessage(const std::string& _u, const std::string& _m) : user(_u), message(_m) {};

	MAKE
		pack(user);
		pack(message);
	END_MAKE

	PARSE
		ticket = unpack_int();
		timestamp = unpack_int();
		user = unpack_string();
		message = unpack_string();
	END_PARSE

	std::string user, message;
	uint32 ticket, timestamp;
END

SINTEGERMESSAGE(SAckPrivateMessage, 23)

SERVERMESSAGE(SFileSearch, 26)
	SFileSearch() {};
	SFileSearch(uint32 _t, const std::string& _q) : query(_q) { ticket = _t; };

	MAKE
		pack(ticket);
		pack(query);
	END_MAKE

	PARSE
		user = unpack_string();
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE

	std::string user, query;
	uint32 ticket;
END


SINTEGERMESSAGE(SSetStatus, 28)

SERVERMESSAGE(SPing, 32)
	MAKE
	END_MAKE

	PARSE
	END_PARSE
END

SERVERMESSAGE(SSendSpeed, 34)
	SSendSpeed(const std::string& _u, uint32 _s) : user(_u), speed(_s) { };
	
	MAKE
		pack(user);
		pack(speed);
	END_MAKE
	
	std::string user;
	uint32 speed;
END

SERVERMESSAGE(SSharedFoldersFiles, 35)
	SSharedFoldersFiles(uint32 _d, uint32 _f) { dirs = _d; files = _f; };

	MAKE
		pack(dirs);
		pack(files);
	END_MAKE

	uint32 dirs, files;
END

SERVERMESSAGE(SGetUserStats, 36)
	SGetUserStats() {};
	SGetUserStats(const std::string& _u) : user(_u) {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		avgspeed = unpack_int();
		downloadnum = unpack_int();
		unpack_int();
		files = unpack_int();
		dirs = unpack_int();
	END_PARSE

	std::string user;
	uint32 avgspeed, downloadnum, files, dirs;
END

SERVERMESSAGE(SKicked, 41)
	MAKE
	END_MAKE

	PARSE
	END_PARSE
END

SERVERMESSAGE(SUserSearch, 42)
	SUserSearch() {};
	SUserSearch(const std::string& _u, uint32 _t, const std::string& _q) : uuser(_u), query(_q) { ticket = _t; };

	MAKE
		pack(uuser);
		pack(ticket);
		pack(query);
	END_MAKE

	PARSE
		user = unpack_string();
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE

	std::string uuser, user, query;
	uint32 ticket;
END


SSTRINGMESSAGE(SInterestAdd, 51)

SSTRINGMESSAGE(SInterestRemove, 52)

SSTRINGMESSAGE(SInterestHatedAdd, 117)

SSTRINGMESSAGE(SInterestHatedRemove, 118)

SERVERMESSAGE(SGetRecommendations, 54)
	SGetRecommendations() {};
	
	MAKE
	END_MAKE
	
	PARSE
		uint32 n = unpack_int();
		while(n) {
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_int();
			n--;
		}
	END_PARSE
	
	std::map<std::string, uint32> recommendations;
END

SERVERMESSAGE(SGetGlobalRecommendations, 56)
	SGetGlobalRecommendations() {};
	
	MAKE
	END_MAKE
	
	PARSE
		uint32 n = unpack_int();
		while(n) {
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_int();
			n--;
		}
	END_PARSE
	
	std::map<std::string, uint32> recommendations;
END

SERVERMESSAGE(SRoomList, 64)
	SRoomList() {};

	MAKE
	END_MAKE

	PARSE
		uint32 n = unpack_int();
		std::vector<std::string> rooms;
		while(n) {
			rooms.push_back(unpack_string());
			n--;
		}
		unpack_int();
		std::vector<std::string>::iterator it = rooms.begin();
		for(; it != rooms.end(); ++it)
			roomlist[*it] = unpack_int();
	END_PARSE

	std::map<std::string, uint32> roomlist;
END

SERVERMESSAGE(SExactFileSearch, 65)
	SExactFileSearch() { };
	
	PARSE
		user = unpack_string();
		ticket = unpack_int();
		filename = unpack_string();
		path = unpack_string();
		filesize = unpack_off();
		checksum = unpack_int();
	END_PARSE
	
	std::string user, filename, path;
	uint32 ticket, checksum;
	off_t filesize;
END

SSTRINGMESSAGE(SGlobalMessage, 66)
// SERVERMESSAGE(SGlobalMessage, 66)
// 	SGlobalMessage() {};
// 	
// 	PARSE
// 		msg = unpack_string();
// 	END_PARSE
// 	
// 	std::string msg;
// END

SSTRINGSMESSAGE(SPrivilegedUsers, 69)

SERVERMESSAGE(SHaveNoParents, 71)
	SHaveNoParents(bool _h) : have_parents(_h) { };
	
	MAKE
		pack((uchar)have_parents);
	END_MAKE
	
	bool have_parents;
END

SINTEGERMESSAGE(SMsg83, 83)

SINTEGERMESSAGE(SMsg84, 84)

SINTEGERMESSAGE(SParentInactivityTimeout, 86)

SINTEGERMESSAGE(SSearchInactivityTimeout, 87)

SINTEGERMESSAGE(SMinParentsInCache, 88)

SINTEGERMESSAGE(SDistribAliveInterval, 90)

SSTRINGMESSAGE(SAddPrivileged, 91)

SERVERMESSAGE(SCheckPrivileges, 92)
	SCheckPrivileges() : time_left(0) { };
	
	MAKE
	END_MAKE
	
	PARSE
		time_left = unpack_int();
	END_PARSE
	
	uint32 time_left;
END

SERVERMESSAGE(SNetInfo, 102)
	SNetInfo() {};
	
	MAKE
	END_MAKE
	
	PARSE
		uint32 n = unpack_int();
		while(n) {
			std::string  user = unpack_string(),
			             ip   = unpack_ip();
			uint32 port = unpack_int();
			users[user] = std::pair<std::string, uint32>(ip, port);
			n--;
		}
	END_PARSE
	
	std::map<std::string, std::pair<std::string, uint32> > users;
END

SERVERMESSAGE(SWishlistSearch, 103)
	SWishlistSearch(uint32 _t, const std::string& _q) : query(_q) { ticket = _t; };
	
	MAKE
		pack(ticket);
		pack(query);
	END_MAKE
	
	std::string query;
	uint32 ticket;
END

SINTEGERMESSAGE(SWishlistInterval, 104)

SERVERMESSAGE(SGetSimilarUsers, 110)
	SGetSimilarUsers() {};
	
	MAKE
	END_MAKE
	
	PARSE
		uint32 n = unpack_int();
		while(n) {
			std::string user = unpack_string();
			users[user] = unpack_int();
			n--;
		}
	END_PARSE
	
	std::map<std::string, uint32>  users;
END

SERVERMESSAGE(SGetItemRecommendations, 111)
	SGetItemRecommendations() : item("") {};
	SGetItemRecommendations(const std::string& _i) : item(_i) {};
	
	MAKE
		pack(item);
	END_MAKE
	
	PARSE
		item = unpack_string();
		uint32 n = unpack_int();
		while(n) {
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_int();
			n--;
		}
	END_PARSE
	
	std::string item;
	std::map<std::string, uint32> recommendations;
END

SERVERMESSAGE(SGetItemSimilarUsers, 112)
	SGetItemSimilarUsers() : item("") {};
	SGetItemSimilarUsers(const std::string& _i) : item(_i) {};
	
	MAKE
		pack(item);
	END_MAKE
	
	PARSE
		item = unpack_string();
		uint32 n = unpack_int();
		while(n) {
			std::string user = unpack_string();
			users[user] = 0;
			n--;
		}
	END_PARSE
	
	std::string item;
	std::map<std::string, uint32> users;
END

SERVERMESSAGE(SRoomTickers, 113)
	SRoomTickers() {};
	
	PARSE
		room = unpack_string();
		uint32 n = unpack_int();
		while(n) {
			std::string user = unpack_string();
			tickers[user] = unpack_string();
			n--;
		}
	END_PARSE
	
	std::string room;
	std::map<std::string, std::string> tickers;
END

SERVERMESSAGE(SRoomTickerAdd, 114)
	SRoomTickerAdd() {};
	
	PARSE
		room = unpack_string();
		user = unpack_string();
		ticker = unpack_string();
	END_PARSE
	
	std::string room, user, ticker;
END

SERVERMESSAGE(SRoomTickerRemove, 115)
	SRoomTickerRemove() {};
	
	PARSE
		room = unpack_string();
		user = unpack_string();
	END_PARSE
	
	std::string room, user;
END

SERVERMESSAGE(SSetRoomTicker, 116)
	SSetRoomTicker(const std::string& _r, const std::string& _t) : room(_r), ticker(_t) {};
	
	MAKE
		pack(room);
		pack(ticker);
	END_MAKE
	
	std::string room, ticker;
END

SERVERMESSAGE(SRoomSearch, 120)
	SRoomSearch() {};
	SRoomSearch(const std::string& _r, uint32 _t, const std::string& _q) : room(_r), query(_q) { ticket = _t; };

	MAKE
		pack(room);
		pack(ticket);
		pack(query);
	END_MAKE

	PARSE
		user = unpack_string();
		ticket = unpack_int();
		query = unpack_string();
	END_PARSE

	std::string room, user, query;
	uint32 ticket;
END



SERVERMESSAGE(SUserPrivileges, 122)
	SUserPrivileges() {}
	SUserPrivileges(const std::string& _u) : user(_u) {}
	
	MAKE
		pack(user);
	END_MAKE
	
	PARSE
		user = unpack_string();
		privileged = (unpack_char() != 0);
	END_PARSE
	
	std::string user;
	bool privileged;
END

SERVERMESSAGE(SGivePrivileges, 123)
	SGivePrivileges(const std::string& _u, uint32 _d) : user(_u), days(_d) {}
	
	MAKE
		pack(user);
		pack(days);
	END_MAKE
	
	std::string user;
	uint32 days;
END

SERVERMESSAGE(SCannotConnect, 1001)
	SCannotConnect() {};
	SCannotConnect(const std::string& _u, uint32 _t) : token(_t), user(_u) {};
	
	MAKE
		pack(token);
		pack(user);
	END_MAKE
	
	PARSE
		token = unpack_int();
		user = unpack_string();
	END_PARSE
	
	uint32 token;
	std::string user;
END

#endif // __SERVERMESSAGES_HH_
