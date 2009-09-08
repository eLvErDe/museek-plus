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

#ifndef MUSEEKMESSAGES_H
#define MUSEEKMESSAGES_H

#include "museeqtypes.h"

#include <Mucipher/mucipher.h>
#include <QByteArray>
#include <QList>
#include <QStringList>

class MuseekMessage {
public:
	MuseekMessage() {}
	MuseekMessage(const QList<unsigned char>& _data)
	                : mData(_data) { }
	virtual ~MuseekMessage() { }

	void pack(unsigned char c) {
		mData.push_back(c);
	}
	void pack(uint i) {
		for(int j = 0; j < 4; j++) {
			pack((unsigned char)(i & 0xff));
			i = i >> 8;
		}
	}
	void pack(qint64 i) {
		for(int j = 0; j < 8; j++) {
			pack((unsigned char)(i& 0xff));
			i = i >> 8;
		}
	}
	void pack(const QByteArray& d) {
		pack((uint)d.size());
		QByteArray::ConstIterator it, end = d.end();
		for(it = d.begin(); it != end; ++it)
			pack((unsigned char)*it);
	}
	void pack(const QString& s) {
		if(s.isEmpty())
			pack((uint)0);
		else {
			QByteArray utf8 = s.toUtf8();
			pack((uint)utf8.size() );
			QByteArray::ConstIterator it = utf8.begin();
			for(; it != utf8.end(); ++it)
				pack((unsigned char)(*it));
		}
	}
	void cipher(CipherContext* ctx, const QString& s) {
		if(s.isEmpty())
			pack((uint)0);
		else {
			QByteArray c = s.toUtf8();
			uint i = c.size();
			pack(i);
			unsigned char temp[CIPHER_BLOCK(i)];
			const char* data = c;
			blockCipher(ctx, (unsigned char*)data, i, temp);
			for(uint j = 0; j < CIPHER_BLOCK(i); ++j)
				pack(temp[j]);
		}
	}

	unsigned char unpack() {
	    if (mData.isEmpty())
            return 0;

		unsigned char r = mData.front();
		mData.pop_front();
		return r;
	}
	uint unpack_uint() {
		unsigned int r = 0;
		for(int j = 0; j < 4; j++)
			r += unpack() << (j * 8);
		return r;
	}
	int unpack_int() {
		int r = -1;
		for(int j = 0; j < 4; j++)
			r -= (unpack() ^ 0xff) << (j * 8);
		return r;
	}
	qint64 unpack_off() {
		qint64 r = 0;
		for(int j = 0; j < 8; j++)
			r += unpack() << (j * 8);
		return r;
	}
	QString unpack_str() {
		uint l = unpack_uint();
		char data[l];
		for(uint j = 0; j < l; ++j)
			data[j] = unpack();
		return QString::fromUtf8(data, l);
	}
	QByteArray unpack_array() {
		uint l = unpack_uint();
		QByteArray r;
		for(uint j = 0; j < l; ++j)
			r.append(unpack());
		return r;
	}
	NUserData unpack_user() {
		NUserData r;
		r.status = unpack_uint();
		r.speed = unpack_uint();
		r.downloadnum = unpack_uint();
		r.files = unpack_uint();
		r.dirs = unpack_uint();
		r.slotsfull = (unpack() != 0);
		r.country = unpack_str();
		return r;
	}

	NRoom unpack_room() {
		NRoom r;
		uint n = unpack_uint();
		while(n) {
			QString s = unpack_str();
			r[s] = unpack_user();
			--n;
		}
		return r;
	}

	QStringList unpack_stringList() {
		QStringList r;
		uint n = unpack_uint();
		while(n) {
			QString s = unpack_str();
			r << s;
			--n;
		}
		return r;
	}

	NTickers unpack_tickers() {
		NTickers r;
		uint n = unpack_uint();
		while(n) {
			QString s = unpack_str();
			r[s] = unpack_str().trimmed();
			--n;
		}
		return r;
	}

	NTransfer unpack_transfer() {
		NTransfer r;
		r.user = unpack_str();
		r.filename = unpack_str();
		r.placeInQueue = unpack_uint();
		r.state = unpack_uint();
		r.error = unpack_str();
		r.filepos = unpack_off();
		r.filesize = unpack_off();
		r.rate = unpack_uint();
		return r;
	}

	NFileData unpack_file() {
		NFileData r;
		r.size = unpack_off();
		QString ext = unpack_str();
		uint n = unpack_uint();
		QList<uint> attrs;
		while(n) {
			attrs.push_back(unpack_uint());
			n--;
		}
		if(ext == "mp3" && attrs.size() == 3) {
			r.bitrate = attrs[0];
			r.length = attrs[1];
			r.vbr = attrs[2] != 0;
		} else {
			r.length = 0;
			r.bitrate = 0;
			r.vbr = false;
		}
		return r;
	}

	NFolder unpack_folder() {
		NFolder r;
		uint n = unpack_uint();
		while(n) {
			QString f = unpack_str();
			r[f] = unpack_file();
			n--;
		}
		return r;
	}

	QString decipher(CipherContext* ctx) {
		uint l = unpack_uint(),
		     l_c = CIPHER_BLOCK(l);
		unsigned char temp1[l_c], temp2[l_c];

		for(uint i = 0; i < l_c; i++)
			temp1[i] = unpack();
		blockDecipher(ctx, temp1, l_c, temp2);

		return QString::fromUtf8((char*)temp2, l);
	}

	virtual const QList<unsigned char>& data() const {
		return mData;
	}

	virtual uint MType() const { return 0; }

protected:
	virtual void parse() { };

private:
	QList<unsigned char> mData;
};

#define MESSAGE(n, id) \
class n : public MuseekMessage { \
public: \
	virtual uint MType() const { return id; } \
	n(): MuseekMessage() { } \
	n(const QList<unsigned char>& _d) : MuseekMessage(_d) { parse(); }

#define CMESSAGE(n, id) \
class n : public MuseekMessage { \
public: \
	virtual uint MType() const { return id; } \
	n(): MuseekMessage() { } \
	n(CipherContext* ctx, const QList<unsigned char>& _d) : MuseekMessage(_d) { parse(ctx); }

#define END };
#define PARSE protected: virtual void parse() {
#define CPARSE protected: virtual void parse(CipherContext* context) {

MESSAGE(NPing, 0x0000)
	uint id;

	NPing(uint _id) {
		pack(_id);
	}

	PARSE
		id = unpack_uint();
	END
END

MESSAGE(NChallenge, 0x0001)
	uint version;
	QString challenge;

	PARSE
		version = unpack_uint();
		challenge = unpack_str();
	END
END

MESSAGE(NLogin, 0x0002)
	bool ok;
	QString msg, challenge;

	NLogin(const QString& algorithm, const QString& chresponse, uint _m) {
		pack(algorithm);
		pack(chresponse);
		pack(_m);
	}

	PARSE
		ok = unpack() != 0;
		msg = unpack_str();
		challenge = unpack_str();
	END
END

MESSAGE(NServerState, 0x0003)
	bool connected;
	QString username;

	PARSE
		connected = unpack() != 0;
		username = unpack_str();
	END
END

MESSAGE(NCheckPrivileges, 0x0004)
	uint secondsleft;

	PARSE
		secondsleft = unpack_uint();
	END
END

MESSAGE(NSetStatus, 0x0005)
	uint status;

	NSetStatus(uint _s) {
		pack(_s);
	}

	PARSE
		status = unpack_uint();
	END
END

MESSAGE(NStatusMessage, 0x0010)
	bool type;
	QString message;

	PARSE
		type = unpack() != 0;
		message = unpack_str();
	END
END

CMESSAGE(NConfigState, 0x0100)
	QMap<QString, QMap<QString, QString> > config;

	CPARSE
		uint n = unpack_uint();
		while(n) {
			QString domain = decipher(context);
			config[domain];
			uint o = unpack_uint();
			while(o) {
				QString key = decipher(context);
				QString val = decipher(context);
				config[domain][key] = val;
				o--;
			}
			n--;
		}
	END
END


CMESSAGE(NNewPassword, 0x0012)
    QString newPass;

	NNewPassword(CipherContext* context, const QString& _pass) {
	    cipher(context, _pass);
	}

	CPARSE
		newPass = decipher(context);
	END
END

CMESSAGE(NConfigSet, 0x0101)
	QString domain, key, value;

	NConfigSet(CipherContext* context, const QString& _domain, const QString& _key, const QString& _value) {
		cipher(context, _domain);
		cipher(context, _key);
		cipher(context, _value);
	}

	CPARSE
		domain = decipher(context);
		key = decipher(context);
		value = decipher(context);
	END
END

CMESSAGE(NConfigRemove, 0x0102)
	QString domain, key;

	NConfigRemove(CipherContext* context, const QString& _domain, const QString& _key) {
		cipher(context, _domain);
		cipher(context, _key);
	}

	CPARSE
		domain = decipher(context);
		key = decipher(context);
	END
END

MESSAGE(NConfigSetUserImage, 0x103)
	NConfigSetUserImage(const QByteArray& d) {
		pack(d);
	}
END

MESSAGE(NUserExists, 0x0201)
	QString user;
	bool exists;

	NUserExists(const QString& _user) {
		pack(_user);
	}

	PARSE
		user = unpack_str();
		exists = unpack() != 0;
	END
END

MESSAGE(NUserStatus, 0x0202)
	QString user;
	uint status;

	NUserStatus(const QString& _user) {
		pack(_user);
	}

	PARSE
		user = unpack_str();
		status = unpack_uint();
	END
END

MESSAGE(NUserStats, 0x0203)
	QString user, country;
	uint speed, downloads, files, dirs;
	bool slotsfull;

	NUserStats(const QString& _user) {
		pack(_user);
	}

	PARSE
		user = unpack_str();
		speed = unpack_uint();
		downloads = unpack_uint();
		files = unpack_uint();
		dirs = unpack_uint();
		slotsfull = (unpack() != 0);
		country = unpack_str();
	END
END

MESSAGE(NUserInfo, 0x0204)
	QString username, info;
	QByteArray picture;
	uint upslots;
	uint queue;
	bool slotsfree;

	NUserInfo(const QString& _user) {
		pack(_user);
	}

	PARSE
		username = unpack_str();
		info = unpack_str();
		picture = unpack_array();
		upslots = unpack_uint();
		queue = unpack_uint();
		slotsfree = unpack() != 0;
	END
END

MESSAGE(NUserShares, 0x0205)
	QString username;
	NShares shares;

	NUserShares(const QString& _user) {
		pack(_user);
	}

	PARSE
		username = unpack_str();
		uint n = unpack_uint();
		while(n) {
			QString folder = unpack_str();
			shares[folder] = unpack_folder();
			n--;
		}
	END
END

MESSAGE(NUserAddress, 0x0206)
	QString user, ip;
	uint port;

	NUserAddress(const QString& _user) {
		pack(_user);
	}

	PARSE
		user = unpack_str();
		ip = unpack_str();
		port = unpack_uint();
	END
END

MESSAGE(NGivePrivileges, 0x0207)
	NGivePrivileges(const QString& _user, uint _days) {
		pack(_user);
		pack(_days);
	}
END

MESSAGE(NGetRoomList, 0x0301)
	NRoomList roomlist;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			roomlist[r] = unpack_uint();
			--n;
		}
	END
END

MESSAGE(NPrivateMessage, 0x0302)
	uint direction, timestamp;
	QString username, message;

	NPrivateMessage(const QString& _user, const QString& _message) {
		pack(_user);
		pack(_message);
	}

	PARSE
		direction = unpack_uint();
		timestamp = unpack_uint();
		username = unpack_str();
		message = unpack_str();
	END
END

MESSAGE(NJoinRoom, 0x0303)
	QString room, owner;
	QStringList operators;
	bool isPrivate;
	NRoom users;

	NJoinRoom(const QString& _room, bool priv) {
		pack(_room);
		if (priv)
            pack((unsigned char)priv);
	}

	PARSE
		room = unpack_str();
		users = unpack_room();
		isPrivate = (unpack() != 0);
		if (isPrivate) {
		    owner = unpack_str();
		    operators = unpack_stringList();
		}
	END
END

MESSAGE(NLeaveRoom, 0x0304)
	QString room;

	NLeaveRoom(const QString& _room) {
		pack(_room);
	}

	PARSE
		room = unpack_str();
	END
END

MESSAGE(NUserJoined, 0x0305)
	QString room, username;
	NUserData userdata;

	PARSE
		room = unpack_str();
		username = unpack_str();
		userdata = unpack_user();
	END
END

MESSAGE(NUserLeft, 0x0306)
	QString room, username;

	PARSE
		room = unpack_str();
		username = unpack_str();
	END
END

MESSAGE(NSayChatroom, 0x0307)
	QString room, user;
	QString line;

	NSayChatroom(const QString& _room, const QString& _line) {
		pack(_room);
		pack(_line);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
		line = unpack_str();
	END
END

MESSAGE(NRoomTickers, 0x0308)
	QString room;
	NTickers tickers;

	PARSE
		room = unpack_str();
		tickers = unpack_tickers();
	END
END

MESSAGE(NRoomTickerSet, 0x0309)
	QString room, user, message;

	NRoomTickerSet(const QString& _room, const QString& _message) {
		pack(_room);
		pack(_message);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
		message = unpack_str();
	END
END

MESSAGE(NMessageUsers, 0x0310)
	NMessageUsers(QStringList _users, QString _msg) {
	    if (!_users.empty()) {
            pack(static_cast<uint>(_users.size()));
            QStringList::iterator it = _users.begin();
            for(; it != _users.end(); ++it) {
                pack(*it);
            }
	    }
	    else {
	        pack(static_cast<uint>(0));
	    }
        pack(_msg);
    }
END

MESSAGE(NMessageBuddies, 0x0311)
	NMessageBuddies(QString _msg) {
        pack(_msg);
	}
END

MESSAGE(NMessageDownloading, 0x0312)
	NMessageDownloading(QString _msg) {
        pack(_msg);
	}
END

MESSAGE(NAskPublicChat, 0x0313)
END

MESSAGE(NStopPublicChat, 0x0314)
END

MESSAGE(NPublicChat, 0x0315)
    QString room, user, message;

	PARSE
        room = unpack_str();
        user = unpack_str();
        message = unpack_str();
	END
END

MESSAGE(NPrivRoomToggle, 0x0320)
	bool enabled;

	NPrivRoomToggle(bool _e) {
		pack((unsigned char)_e);
	}

	PARSE
		enabled = unpack() != 0;
	END
END

MESSAGE(NGetPrivRoomList, 0x0321)
	NPrivRoomList roomlist;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			roomlist[r].first = unpack_uint();
			roomlist[r].second = unpack_uint();
			--n;
		}
	END
END

MESSAGE(NPrivRoomAddUser, 0x0322)
	QString room, user;

	NPrivRoomAddUser(QString _r, QString _u) {
		pack(_r);
		pack(_u);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
	END
END

MESSAGE(NPrivRoomRemoveUser, 0x0323)
	QString room, user;

	NPrivRoomRemoveUser(QString _r, QString _u) {
		pack(_r);
		pack(_u);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
	END
END

MESSAGE(NRoomMembers, 0x0324)
	NRooms rooms;
	NPrivRoomOperators operators;
	NPrivRoomOwners owners;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();

            NRoom data;
            uint nu = unpack_uint();
            while(nu) {
                QString s = unpack_str();
                data[s] = unpack_user();
                uint status = unpack_uint();
                if (status == 1) {
                    operators[r].push_back(s);
                }
                else if (status == 2) {
                    owners[r] = s;
                }
                --nu;
            }

			rooms[r] = data;
			n--;
		}
	END
END

MESSAGE(NRoomsTickers, 0x0325)
	NTickerMap tickers;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			tickers[r] = unpack_tickers();
			n--;
		}
	END
END

MESSAGE(NPrivRoomAlterableMembers, 0x0326)
    QString room;
	QStringList members;

	PARSE
		room = unpack_str();
		members = unpack_stringList();
	END
END

MESSAGE(NPrivRoomAlterableOperators, 0x0327)
    QString room;
	QStringList operators;

	PARSE
		room = unpack_str();
		operators = unpack_stringList();
	END
END

MESSAGE(NPrivRoomAddOperator, 0x0328)
	QString room, user;

	NPrivRoomAddOperator(QString _r, QString _u) {
		pack(_r);
		pack(_u);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
	END
END

MESSAGE(NPrivRoomRemoveOperator, 0x0329)
	QString room, user;

	NPrivRoomRemoveOperator(QString _r, QString _u) {
		pack(_r);
		pack(_u);
	}

	PARSE
		room = unpack_str();
		user = unpack_str();
	END
END

MESSAGE(NPrivRoomDismember, 0x0330)
	QString room;

	NPrivRoomDismember(QString _r) {
		pack(_r);
	}
END

MESSAGE(NPrivRoomDisown, 0x0331)
	QString room;

	NPrivRoomDisown(QString _r) {
		pack(_r);
	}
END

MESSAGE(NSearchRequest, 0x0401)
	QString query;
	uint token;

	NSearchRequest(uint _type, QString _query) {
		pack(_type);
		pack(_query);
	}

	PARSE
		query = unpack_str();
		token = unpack_uint();
	END
END

MESSAGE(NSearchResults, 0x0402)
	uint token;
	QString username;
	bool slotsfree;
	uint speed;
	uint queue;
	NFolder results;

	NSearchResults(uint _token) {
		pack(_token);
	}

	PARSE
		token = unpack_uint();
		username = unpack_str();
		slotsfree = unpack() != 0;
		speed = unpack_uint();
		queue = unpack_uint();
		results = unpack_folder();
	END
END

MESSAGE(NUserSearchRequest, 0x0403)

	NUserSearchRequest(QString _user, QString _query) {
		pack(_user);
		pack(_query);
	}
END

MESSAGE(NWishListSearchRequest, 0x0405)
	QString query;

	NWishListSearchRequest(QString _query) {
		pack(_query);
	}
END

MESSAGE(NAddWishItem, 0x0406)
	QString query;
	uint lastSearched;

	NAddWishItem(const QString& _query) {
		pack(_query);
	}

	PARSE
		query = unpack_str();
		lastSearched = unpack_uint();
	END
END

MESSAGE(NRemoveWishItem, 0x0407)
	QString query;

	NRemoveWishItem(const QString& _query) {
		pack(_query);
	}

	PARSE
		query = unpack_str();
	END
END

MESSAGE(NTransferState, 0x0500)
	NTransfers downloads, uploads;

	PARSE
		uint n = unpack_uint();
		while(n) {
			unsigned char upl = unpack() != 0;
			NTransfer transfer = unpack_transfer();
			if(upl)
				uploads.push_back(transfer);
			else
				downloads.push_back(transfer);
			n--;
		}
	END
END

MESSAGE(NTransferUpdate, 0x0501)
	bool isUpload;
	NTransfer transfer;

	NTransferUpdate(const QString& _user, const QString& _path) {
		pack(_user);
		pack(_path);
	}

	PARSE
		isUpload = unpack() != 0;
		transfer = unpack_transfer();
	END
END

MESSAGE(NTransferRemove, 0x0502)
	bool isUpload;
	QString user, path;

	NTransferRemove(bool _upload, const QString& _user, const QString& _path) {
		pack((unsigned char)_upload);
		pack(_user);
		pack(_path);
	}

	PARSE
		isUpload = unpack() != 0;
		user = unpack_str();
		path = unpack_str();
	END
END

MESSAGE(NDownloadFile, 0x0503)
	NDownloadFile(const QString& _user, const QString& _path, qint64 _size) {
		pack(_user);
		pack(_path);
		pack(_size);
	}
END

MESSAGE(NFolderContents, 0x0504)
	NFolderContents(const QString& _user, const QString& _path) {
		pack(_user);
		pack(_path);
	}
END

MESSAGE(NTransferAbort, 0x0505)
	NTransferAbort(bool _upload, const QString& _user, const QString& _path) {
		pack((unsigned char)_upload);
		pack(_user);
		pack(_path);
	}
END

MESSAGE(NUploadFile, 0x0506)
	NUploadFile(const QString& _user, const QString& _path) {
		pack(_user);
		pack(_path);
	}
END

MESSAGE(NDownloadFileTo, 0x0507)
	NDownloadFileTo(const QString& _user, const QString& _path, const QString& _local, qint64 _size) {
		pack(_user);
		pack(_path);
		pack(_local);
		pack(_size);
	}
END

MESSAGE(NDownloadFolderTo, 0x0508)
	NDownloadFolderTo(const QString& _user, const QString& _path, const QString& _local) {
		pack(_user);
		pack(_path);
		pack(_local);
	}
END

MESSAGE(NUploadFolder, 0x0509)
	NUploadFolder(const QString& _user, const QString& _path) {
		pack(_user);
		pack(_path);
	}
END

MESSAGE(NGetRecommendations, 0x0600)
	NRecommendations recommendations;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			recommendations[r] = unpack_int();
			--n;
		}
	END
END

MESSAGE(NGetGlobalRecommendations, 0x0601)
	NGlobalRecommendations recommendations;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			recommendations[r] = unpack_int();
			--n;
		}
	END
END

MESSAGE(NGetSimilarUsers, 0x0602)
	NSimilarUsers users;

	PARSE
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			users[r] = unpack_uint();
			--n;
		}
	END
END


MESSAGE(NGetItemRecommendations, 0x0603)
	NItemRecommendations recommendations;
	QString item;

	NGetItemRecommendations(const QString& _item) {
		pack(_item);
	}

	PARSE
		item = unpack_str();
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			recommendations[r] = unpack_int();
			--n;
		}
	END
END

MESSAGE(NGetItemSimilarUsers, 0x0604)
	NItemSimilarUsers users;
	QString item;

	NGetItemSimilarUsers(const QString& _item) {
		pack(_item);
	}

	PARSE
		item = unpack_str();
		uint n = unpack_uint();
		while(n) {
			QString r = unpack_str();
			users[r] = unpack_uint();
			--n;
		}
	END
END


MESSAGE(NAddInterest, 0x0610)
	QString interest;

	NAddInterest(const QString& _interest) {
		pack(_interest);
	}

	PARSE
		interest = unpack_str();
	END
END

MESSAGE(NAddHatedInterest, 0x0612)
	QString interest;

	NAddHatedInterest(const QString& _interest) {
		pack(_interest);
	}

	PARSE
		interest = unpack_str();
	END
END

MESSAGE(NRemoveInterest, 0x0611)
	QString interest;

	NRemoveInterest(const QString& _interest) {
		pack(_interest);
	}

	PARSE
		interest = unpack_str();
	END
END

MESSAGE(NRemoveHatedInterest, 0x0613)
	QString interest;

	NRemoveHatedInterest(const QString& _interest) {
		pack(_interest);
	}

	PARSE
		interest = unpack_str();
	END
END

MESSAGE(NUserInterests, 0x0614)
	QString user;
	QStringList likes;
	QStringList hates;

	NUserInterests(const QString& _user) {
		pack(_user);
	}

	PARSE
		user = unpack_str();
		likes = unpack_stringList();
		hates = unpack_stringList();
	END
END

MESSAGE(NConnectServer, 0x0700)
END

MESSAGE(NDisconnectServer, 0x0701)
END

MESSAGE(NReloadShares, 0x0703)
END

#endif // MUSEEKMESSAGES_H

