/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifndef MUSEEK_SERVERMESSAGES_H
#define MUSEEK_SERVERMESSAGES_H

#include "networkmessage.h"

class ServerMessage : public NetworkMessage {
protected:
    void default_garbage_collector() { }
};
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
                if (buffer.count() < 4) \
                    break; \
				values.push_back(unpack_string()); \
				j--; \
			} \
		END_PARSE \
		std::vector<std::string> values; \
	END

SERVERMESSAGE(SLogin, 1)
	SLogin(const std::string& _username = "", const std::string& _password = "") :
		greet(""), username(_username), password(_password), publicip(""), success(0)
		{};

	MAKE
		pack(username);
		pack(password);
		pack((uint32)182);
	END_MAKE

	PARSE
		success = (unpack_char() != 0);
		greet = unpack_string();
		publicip = unpack_ip();
	END_PARSE

	std::string greet, username, password, publicip;
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
		exists = (unpack_char() != 0);
		userdata.status = unpack_int();
		userdata.avgspeed = unpack_int();
		userdata.downloadnum = unpack_off();
		userdata.files = unpack_int();
		userdata.dirs = unpack_int();
		userdata.country = unpack_string();
	END_PARSE

	std::string user;
	bool exists;
	UserData userdata;
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
		privileged = (unpack_char() != 0); // Not used (SAddPrivileged and SPrivilegedUsers should do the trick)
	END_PARSE

	std::string user;
	uint32 status;
	bool privileged;
END

SERVERMESSAGE(SSayRoom, 13)
	SSayRoom(const std::string& r = "", const std::string& l = ""): room(r), line(l) {};

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
	SJoinRoom(const std::string& r = "", bool p = false) : room(r), isPrivate(p) {};
	MAKE
		pack(room);
		if (isPrivate)
            pack(1);
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

		// User data for each user
		unpack_int();
		std::vector<UserData>::iterator it = _d.begin();
		for(; it != _d.end(); ++it) {
			(*it).avgspeed = unpack_int();
			(*it).downloadnum = unpack_off();
			(*it).files = unpack_int();
			(*it).dirs = unpack_int();
		}

		// Slotsfull for each user
		unpack_int();
		for(it = _d.begin(); it != _d.end(); ++it)
			(*it).slotsfull = unpack_int();


        // Country code for each user
		unpack_int();
		for(it = _d.begin(); it != _d.end(); ++it)
			(*it).country = unpack_string();

		// Create user list
		it = _d.begin();
		std::vector<std::string>::iterator sit = _u.begin();
		for(; it != _d.end(); ++it, ++sit )
			users[*sit] = *it;

		if(! buffer.empty()) {
		    isPrivate = true;
            owner = unpack_string();

            uint32 no = unpack_int();
            for(uint32 io = 0; io < no; io++)
                ops.push_back(unpack_string());
		}
	END_PARSE

	std::string room, owner;
	std::vector<std::string> ops;
	bool isPrivate;
	RoomData users;
END

SSTRINGMESSAGE(SLeaveRoom, 15)

SERVERMESSAGE(SUserJoinedRoom, 16)
	PARSE
		room = unpack_string();
		user = unpack_string();
		userdata.status = unpack_int();
		userdata.avgspeed = unpack_int();
		userdata.downloadnum = unpack_off();
		userdata.files = unpack_int();
		userdata.dirs = unpack_int();
		userdata.slotsfull = unpack_int();
		userdata.country = unpack_string();
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
		privileged = (unpack_char() != 0);
	END_PARSE

	std::string user, type, ip;
	uint32 port, token;
	bool privileged;
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
		if(! buffer.empty())
            isAdmin = (unpack_char() != 0);
	END_PARSE

	std::string user, message;
	uint32 ticket, timestamp;
	bool isAdmin;
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


/* DEPRECATED in 2005 */
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

// Deprecated. See SAddUser
SERVERMESSAGE(SGetUserStats, 36)
	SGetUserStats() {};
	SGetUserStats(const std::string& _u) : user(_u) {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		avgspeed = unpack_int();
		downloadnum = unpack_off();
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
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_signed_int();
			n--;
		}
		uint32 nu = unpack_int();
		while(nu) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_signed_int();
			nu--;
		}
	END_PARSE

	std::map<std::string, int32> recommendations;
END

SERVERMESSAGE(SGetGlobalRecommendations, 56)
	SGetGlobalRecommendations() {};

	MAKE
	END_MAKE

	PARSE
		uint32 n = unpack_int();
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string recommendation = unpack_string();
			recommendations[recommendation] = static_cast<int32>(unpack_int());
			n--;
		}
		uint32 nu = unpack_int();
		while(nu) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_signed_int();
			nu--;
		}
	END_PARSE

	std::map<std::string, int32> recommendations;
END

SERVERMESSAGE(SUserInterests, 57)
	SUserInterests() {};
	SUserInterests(const std::string& _u) : user(_u) {};

	MAKE
		pack(user);
	END_MAKE

	PARSE
		user = unpack_string();
		uint32 n1 = unpack_int();
		while(n1) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			likes.push_back(unpack_string());
			n1--;
		}
		uint32 n2 = unpack_int();
		while(n2) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			hates.push_back(unpack_string());
			n2--;
		}
	END_PARSE

	std::string user;
	std::vector<std::string> hates, likes;

END

SERVERMESSAGE(SRoomList, 64)
	SRoomList() {};

	MAKE
	END_MAKE

	PARSE
		uint32 n = unpack_int();
		std::vector<std::string> rooms;
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			rooms.push_back(unpack_string());
			n--;
		}
		unpack_int();
		std::vector<std::string>::iterator it = rooms.begin();
		for(; it != rooms.end(); ++it)
			roomlist[*it] = unpack_int();

        // Get rooms owned by us
		uint32 no = unpack_int();
		std::vector<std::string> privroomsOwned;
		while(no) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			privroomsOwned.push_back(unpack_string());
			no--;
		}
		unpack_int();
		std::vector<std::string>::iterator ito = privroomsOwned.begin();
		for(; ito != privroomsOwned.end(); ++ito)
			privroomlist[*ito] = std::pair<uint32, uint32>(unpack_int(), 2);

        // Get rooms where we're member
		uint32 nm = unpack_int();
		std::vector<std::string> privroomsMember;
		while(nm) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			privroomsMember.push_back(unpack_string());
			nm--;
		}
		unpack_int();
		std::vector<std::string>::iterator itp = privroomsMember.begin();
		for(; itp != privroomsMember.end(); ++itp)
			privroomlist[*itp] = std::pair<uint32, uint32>(unpack_int(), 0);

        // Get rooms where we're operator (no users nb as it is given in rooms where we're member)
		uint32 np = unpack_int();
		std::vector<std::string> privrooms;
		while(np) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
		    std::string opedRoom = unpack_string();
		    if (privroomlist.find(opedRoom) != privroomlist.end())
                privroomlist[opedRoom].second = 1;
			np--;
		}
	END_PARSE

	RoomList roomlist;
	PrivRoomList privroomlist; // [name[users, status]] with status : { 0 => member, 1 => operator, 2 => owner}
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
	uint64 filesize;
END

SSTRINGMESSAGE(SGlobalMessage, 66)

SSTRINGSMESSAGE(SPrivilegedUsers, 69)

SERVERMESSAGE(SHaveNoParents, 71)
	SHaveNoParents(bool _h) : have_parents(_h) { };

	MAKE
		pack((uchar)have_parents);
	END_MAKE

	bool have_parents;
END

SERVERMESSAGE(SParentIP, 73)
	SParentIP(std::string _ip) : ip(_ip) { };

	MAKE
		pack_ip(ip);
	END_MAKE

	std::string ip;
END

SINTEGERMESSAGE(SParentMinSpeed, 83)

SINTEGERMESSAGE(SParentSpeedRatio, 84)

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

SERVERMESSAGE(SSearchRequest, 93)
	SSearchRequest() { };

	MAKE
	END_MAKE

	PARSE
        dCode = unpack_char();
        unknown = unpack_int();
        username = unpack_string();
        token = unpack_int();
        query = unpack_string();
	END_PARSE

    char dCode;
	uint32 unknown, token;
	std::string username, query;
END

SERVERMESSAGE(SAcceptChildren, 100)
	SAcceptChildren(bool _a) : accept(_a) { };

	MAKE
		pack((uchar)accept);
	END_MAKE

	bool accept;
END

SERVERMESSAGE(SNetInfo, 102)
	SNetInfo() {};

	MAKE
	END_MAKE

	PARSE
		uint32 n = unpack_int();
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
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
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string user = unpack_string();
			users[user] = unpack_int();
			n--;
		}
	END_PARSE

	SimilarUsers users;
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
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string recommendation = unpack_string();
			recommendations[recommendation] = unpack_signed_int();
			n--;
		}
	END_PARSE

	std::string item;
	std::map<std::string, int32> recommendations;
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
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			std::string user = unpack_string();
			users[user] = 0;
			n--;
		}
	END_PARSE

	std::string item;
	SimilarUsers users;
END

SERVERMESSAGE(SRoomTickers, 113)
	SRoomTickers() {};

	PARSE
		room = unpack_string();
		uint32 n = unpack_int();
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
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

SERVERMESSAGE(SSendUploadSpeed, 121)
	SSendUploadSpeed(uint32 _s) : speed(_s) { };

	MAKE
		pack(speed);
	END_MAKE

	uint32 speed;
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

SERVERMESSAGE(SNotifyPrivileges, 124)
	SNotifyPrivileges(uint32 _t, const std::string& _u) : token(_t), user(_u) {}

	MAKE
		pack(token);
		pack(user);
	END_MAKE

	uint32 token;
	std::string user;
END

SERVERMESSAGE(SAckNotifyPrivileges, 125)
	SAckNotifyPrivileges() {}

	PARSE
		token = unpack_int();
	END_PARSE

	uint32 token;
END

SERVERMESSAGE(SBranchLevel, 126)
	SBranchLevel(uint32 _l) : level(_l) { };

	MAKE
		pack(level);
	END_MAKE

	uint32 level;
END

SERVERMESSAGE(SBranchRoot, 127)
	SBranchRoot(std::string _r) : root(_r) { };

	MAKE
		pack(root);
	END_MAKE

	std::string root;
END

SERVERMESSAGE(SChildDepth, 129)
	SChildDepth(uint32 _d) : depth(_d) { };

	MAKE
		pack(depth);
	END_MAKE

	uint32 depth;
END

SERVERMESSAGE(SPrivRoomAlterableMembers, 133)
    // These users are those that can be dismembered/oped by us
	SPrivRoomAlterableMembers() {}

	PARSE
		room = unpack_string();
		n = unpack_int();
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			users.push_back(unpack_string());
			n--;
		}
	END_PARSE

    std::string room;
	uint32 n;
	std::vector<std::string> users;
END

SERVERMESSAGE(SPrivRoomAddUser, 134)
    // Received when an alterable user has been removed from a private room
	SPrivRoomAddUser() {}
	SPrivRoomAddUser(const std::string& _r, const std::string& _u) : room(_r), user(_u) { };

	MAKE
		pack(room);
		pack(user);
	END_MAKE

	PARSE
		room = unpack_string();
		user = unpack_string();
	END_PARSE

    std::string room, user;
END

SERVERMESSAGE(SPrivRoomRemoveUser, 135)
	SPrivRoomRemoveUser() {}
	SPrivRoomRemoveUser(std::string _r, std::string _u) : room(_r), user(_u) {}

	MAKE
		pack(room);
		pack(user);
	END_MAKE

	PARSE
		room = unpack_string();
		user = unpack_string();
	END_PARSE

    std::string room, user;
END

SERVERMESSAGE(SPrivRoomDismember, 136)
	SPrivRoomDismember(std::string _r) : room(_r) {}

	MAKE
		pack(room);
	END_MAKE

    std::string room;
END

SERVERMESSAGE(SPrivRoomDisown, 137)
	SPrivRoomDisown(std::string _r) : room(_r) {}

	MAKE
		pack(room);
	END_MAKE

    std::string room;
END

SERVERMESSAGE(SPrivRoomUnknown138, 138)
	SPrivRoomUnknown138() {}
	SPrivRoomUnknown138(std::string _r) : room(_r) {}

	MAKE
		pack(room);
	END_MAKE

	PARSE
		room = unpack_string();
	END_PARSE

    std::string room;
END

SERVERMESSAGE(SPrivRoomAdded, 139)
    // This is received from the server (never sent to the server) when we get added as member to a room by someone else. This is not used in museek as this is redundant with SPrivRoomAlterableMembers and SPrivRoomAlterableOperators which are always sent by the server just after this message.
	SPrivRoomAdded() {}

	PARSE
		room = unpack_string();
	END_PARSE

    std::string room;
END

SERVERMESSAGE(SPrivRoomRemoved, 140)
    // This is received from the server (never sent to the server) when we get removed from a room where we were member by someone else. This is not used in museek as this is redundant with SPrivRoomAlterableMembers and SPrivRoomAlterableOperators which are always sent by the server just after this message.
	SPrivRoomRemoved() {}

	PARSE
		room = unpack_string();
	END_PARSE

    std::string room;
END

SERVERMESSAGE(SPrivRoomToggle, 141)
	SPrivRoomToggle() {}
	SPrivRoomToggle(bool _e) : enabled(_e) {}

	MAKE
		pack((uchar)enabled);
	END_MAKE

	PARSE
		enabled = (unpack_char() != 0);
	END_PARSE

    bool enabled;
END

SERVERMESSAGE(SNewPassword, 142)
	SNewPassword() {}
	SNewPassword(std::string _p) : newPassword(_p) {}

	MAKE
		pack(newPassword);
	END_MAKE

	PARSE
		newPassword = unpack_string();
	END_PARSE

    std::string newPassword;
END

SERVERMESSAGE(SPrivRoomAddOperator, 143)
	SPrivRoomAddOperator() {}
	SPrivRoomAddOperator(std::string _r, std::string _o) : room(_r), op(_o) {}

	MAKE
		pack(room);
		pack(op);
	END_MAKE

	PARSE
		room = unpack_string();
		op = unpack_string();
	END_PARSE

    std::string room, op;
END

SERVERMESSAGE(SPrivRoomRemoveOperator, 144)
	SPrivRoomRemoveOperator() {}
	SPrivRoomRemoveOperator(std::string _r, std::string _o) : room(_r), op(_o) {}

	MAKE
		pack(room);
		pack(op);
	END_MAKE

	PARSE
		room = unpack_string();
		op = unpack_string();
	END_PARSE

    std::string room, op;
END

SERVERMESSAGE(SPrivRoomOperatorAdded, 145)
    // This is received from the server (never sent to the server) when we get added as an operator to a room by someone else. This is not used in museek as this is redundant with SPrivRoomAlterableMembers and SPrivRoomAlterableOperators which are always sent by the server just after this message.
	SPrivRoomOperatorAdded() {}

	PARSE
		room = unpack_string();
	END_PARSE

    std::string room;
END

SERVERMESSAGE(SPrivRoomOperatorRemoved, 146)
    // This is received from the server (never sent to the server) when we get removed from a room where we were operator by someone else. This is not used in museek as this is redundant with SPrivRoomAlterableMembers and SPrivRoomAlterableOperators which are always sent by the server just after this message.
	SPrivRoomOperatorRemoved() {}

	PARSE
		room = unpack_string();
	END_PARSE

    std::string room;
END

SERVERMESSAGE(SPrivRoomAlterableOperators, 148)
    // These operators are those that can be disoped by us
	SPrivRoomAlterableOperators() {}

	PARSE
		room = unpack_string();
		n = unpack_int();
		while(n) {
            if (buffer.count() < 4)
                break; // If this happens, message is malformed. No need to continue (prevent huge loops)
			ops.push_back(unpack_string());
			n--;
		}
	END_PARSE

    std::string room;
	uint32 n;
	std::vector<std::string> ops;
END

SERVERMESSAGE(SMessageUsers, 149)
	SMessageUsers(const std::vector<std::string>& _u, std::string _m) : users(_u), message(_m) {}

	MAKE
        pack((uint32)users.size());
        std::vector<std::string>::iterator it = users.begin();
        for(; it != users.end(); ++it)
            pack(*it);
		pack(message);
	END_MAKE

	std::vector<std::string> users;
    std::string message;
END

SERVERMESSAGE(SAskPublicChat, 150)
	MAKE
	END_MAKE
END

SERVERMESSAGE(SStopPublicChat, 151)
	MAKE
	END_MAKE
END

SERVERMESSAGE(SPublicChat, 152)
	SPublicChat() {}

	PARSE
		room = unpack_string();
		user = unpack_string();
		message = unpack_string();
	END_PARSE

    std::string room, user, message;
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

#endif // MUSEEK_SERVERMESSAGES_H
