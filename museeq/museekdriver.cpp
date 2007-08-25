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

#include "system.h"

#include "museekdriver.h"
#include "museekmessages.h"

#include <qsocket.h>
#include <iostream>

MuseekDriver::MuseekDriver(QObject* parent, const char* name)
          : QObject(parent, name), mSocket(0), mHaveSize(false), mPassword(QString::null) {

}

void MuseekDriver::connectToHost(const QString& host, Q_UINT16 port, const QString& password) {
	if(mSocket) {
		mSocket->deleteLater();
		mSocket = 0;
	}
	
	mPassword = password;
	
	mHaveSize = false;
	
	mSocket = new QSocket(this);
	connect(mSocket, SIGNAL(hostFound()), this, SIGNAL(hostFound()));
	connect(mSocket, SIGNAL(connected()), SIGNAL(connected()));
	connect(mSocket, SIGNAL(connectionClosed()), SIGNAL(connectionClosed()));
	connect(mSocket, SIGNAL(delayedCloseFinished()), SIGNAL(connectionClosed()));
	connect(mSocket, SIGNAL(error(int)), SIGNAL(error(int)));
	connect(mSocket, SIGNAL(readyRead()), SLOT(dataReady()));
	mSocket->connectToHost(host, port);
}

void MuseekDriver::connectToUnix(const QString& path, const QString& password) {
	if(mSocket) {
		mSocket->deleteLater();
		mSocket = 0;
	}
	
	mPassword = password;
	
	mHaveSize = false;
	
#ifdef HAVE_SYS_UN_H
	int sock = ::socket(PF_UNIX, SOCK_STREAM, 0);
	
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	
	qstrncpy(addr.sun_path, (const char*)path.local8Bit(), sizeof(addr.sun_path));

	if(::connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
		perror("connect: ");
		emit error(QSocket::ErrConnectionRefused);
		return;
	}
	
	mSocket = new QSocket(this);
	connect(mSocket, SIGNAL(hostFound()), this, SIGNAL(hostFound()));
	connect(mSocket, SIGNAL(connected()), SIGNAL(connected()));
	connect(mSocket, SIGNAL(connectionClosed()), SIGNAL(connectionClosed()));
	connect(mSocket, SIGNAL(delayedCloseFinished()), SIGNAL(connectionClosed()));
	connect(mSocket, SIGNAL(error(int)), SIGNAL(error(int)));
	connect(mSocket, SIGNAL(readyRead()), SLOT(dataReady()));
	mSocket->setSocket(sock);
#else
	emit error(QSocket::ErrConnectionRefused);
#endif
}

void MuseekDriver::disconnect() {
	if(mSocket) {
		mSocket->disconnect();
		mSocket->deleteLater();
		mSocket = 0;
		emit connectionClosed();
	}
	// Disconnecting while connecting causes a crash after 2-3 seconds
}

void MuseekDriver::dataReady() {
	if(! mSocket) { return; } // Crashes even with this.
	if(! mHaveSize) {
		if(mSocket->bytesAvailable() < 4)
			return;
		
		mHaveSize = true;
		mMsgSize = 0;
		for(int i = 0; i < 4; i++)
			mMsgSize += mSocket->getch() << (i * 8);
	}
	
	if(mSocket->bytesAvailable() < mMsgSize)
		return;
	
	mHaveSize = false;
	
	unsigned int mtype = 0;
	for(int i = 0; i < 4; i++)
		mtype += mSocket->getch() << (i * 8);
	
	QValueList<unsigned char> data;
	for(uint i = 4; i < mMsgSize; i++)
		data.push_back(mSocket->getch());
	
	switch(mtype) {
	case 0x0001: {
		NChallenge m(data);
		
		QString chresp = m.challenge + mPassword;
		
		unsigned char digest[20];
		const char *_chresp = chresp;
		shaBlock((unsigned char*)_chresp, chresp.length(), digest);
		
		char hexdigest[41];
		hexDigest(digest, 20, hexdigest);
		
		NLogin n("SHA1", hexdigest, 1 | 2 | 4 | 8 | 16 | 32 | 64);
		send(n);
		break;
	}
	case 0x0002: {
		NLogin m(data);
		if(m.ok) {
			const char* key = mPassword;
			cipherKeySHA256(&mContext, (char*)key, mPassword.length());
		}
		emit loggedIn(m.ok, m.msg);
		break;
	}
	case 0x0003: {
		NServerState m(data);
		emit serverState(m.connected, m.username);
		break;
	}
	case 0x0010: {
		NStatusMessage m(data);
		emit statusMessage(m.type, m.message);
		break;
	}
	case 0x0300: {
		NRoomState m(data);
		emit roomState(m.roomlist, m.rooms, m.tickers);
		break;
	}
	case 0x0301: {
		NGetRoomList m(data);
		emit roomList(m.roomlist);
		break;
	}
	case 0x0600: {
		NGetRecommendations m(data);
		emit aRecommendations(m.recommendations);
		break;
	}
	case 0x0601: {
		NGetGlobalRecommendations m(data);
		emit Recommendations(m.recommendations);
		break;
	}
	case 0x0602: {
		NGetSimilarUsers m(data);
		emit similarUsers(m.users);
		break;
	}
	case 0x0603: {
		NGetItemRecommendations m(data);
		emit itemRecommendations(m.item, m.recommendations);
		break;
	}
	case 0x0604: {
		NGetItemSimilarUsers m(data);
		emit itemSimilarUsers(m.item, m.users);
		break;
	}
	case 0x0610: {
		NAddInterest m(data);
		emit addInterest(m.interest);
		break;
	}
	case 0x0612: {
		NAddHatedInterest m(data);
		emit addHatedInterest(m.interest);
		break;
	}
	case 0x0611: {
		NRemoveInterest m(data);
		emit removeInterest(m.interest);
		break;
	}
	case 0x0613: {
		NRemoveHatedInterest m(data);
		emit removeHatedInterest(m.interest);
		break;
	}
	case 0x0004: {
		NCheckPrivileges m(data);
		emit privilegesLeft(m.secondsleft);
		break;
	}
	case 0x0005: {
		NSetStatus m(data);
		emit statusSet(m.status);
		break;
	}
	case 0x0401: {
		NSearchRequest m(data);
		emit searchToken(m.query, m.token);
		break;
	}
	case 0x0402: {
		NSearchResults m(data);
		emit searchResults(m.token, m.username, m.slotsfree, m.speed, m.queue, m.results);
		break;
	}
	case 0x0307: {
		NSayChatroom m(data);
		emit sayChatroom(m.room, m.user, m.line);
		break;
	}
	case 0x0303: {
		NJoinRoom m(data);
		emit joinRoom(m.room, m.users);
		break;
	}
	case 0x0304: {
		NLeaveRoom m(data);
		emit leaveRoom(m.room);
		break;
	}
	case 0x0305: {
		NUserJoined m(data);
		emit userJoined(m.room, m.username, m.userdata);
		break;
	}
	case 0x0306: {
		NUserLeft m(data);
		emit userLeft(m.room, m.username);
		break;
	}
	case 0x0308: {
		NRoomTickers m(data);
		emit roomTickers(m.room, m.tickers);
		break;
	}
	case 0x0309: {
		NRoomTickerSet m(data);
		emit roomTickerSet(m.room, m.user, m.message);
		break;
	}
	case 0x0302: {
		NPrivateMessage m(data);
		emit privateMessage(m.direction, m.timestamp, m.username, m.message);
		break;
	}
	case 0x0204: {
		NUserInfo m(data);
		emit userInfo(m.username, m.info, m.picture, m.upslots, m.queue, m.slotsfree);
		break;
	}
	case 0x0205: {
		NUserShares m(data);
		emit userShares(m.username, m.shares);
		break;
	}
	case 0x0500: {
		NTransferState m(data);
		emit transferState(m.downloads, m.uploads);
		break;
	}
	case 0x0501: {
		NTransferUpdate m(data);
		emit transferUpdate(m.isUpload, m.transfer);
		break;
	}
	case 0x0201: {
		NUserExists m(data);
		emit userExists(m.user, m.exists);
		break;
	}
	case 0x0202: {
		NUserStatus m(data);
		emit userStatus(m.user, m.status);
		break;
	}
	case 0x0203: {
		NUserStats m(data);
		emit userData(m.user, m.speed, m.files);
		break;
	}
	case 0x0206: {
		NUserAddress m(data);
		emit userAddress(m.user, m.ip, m.port);
		break;
	}
	case 0x0100: {
		NConfigState m(&mContext, data);
		emit configState(m.config);
		break;
	}
	case 0x0101: {
		NConfigSet m(&mContext, data);
		emit configSet(m.domain, m.key, m.value);
		break;
	}
	case 0x0102: {
		NConfigRemove m(&mContext, data);
		emit configRemove(m.domain, m.key);
		break;
	}
	case 0x0502: {
		NTransferRemove m(data);
		emit transferRemove(m.isUpload, m.user, m.path);
		break;
	}
	default:
		std::cerr << "Unknown message " << mtype << std::endl;
	}
 
	dataReady();
}

void MuseekDriver::send(const MuseekMessage& m) {
	if(! mSocket) { return; }
	const QValueList<unsigned char>& data = m.data();
	
	uint i = data.size() + 4;
	for(uint j = 0; j < 4; ++j) {
		mSocket->putch(i & 0xff);
		i = i >> 8;
	}
	
	i = m.MType();
	for(uint j = 0; j < 4; ++j) {
		mSocket->putch(i & 0xff);
		i = i >> 8;
	}
	
	QValueList<unsigned char>::ConstIterator it = data.begin();
	for(; it != data.end(); ++it)
		mSocket->putch(*it);
}

void MuseekDriver::doSayChatroom(const QString& room, const QString& line) {
	send(NSayChatroom(room, line));
}

void MuseekDriver::doSendPrivateMessage(const QString& user, const QString& msg) {
	send(NPrivateMessage(user, msg));
}

void MuseekDriver::doStartSearch(uint type, const QString& query) {
	send(NSearchRequest(type, query));
}

void MuseekDriver::doStartUserSearch(const QString& user, const QString& query) {
	send(NUserSearchRequest(user, query));
}

void MuseekDriver::doStartWishListSearch(const QString& query) {
	send(NWishListSearchRequest( query));
}

void MuseekDriver::doJoinRoom(const QString& room) {
	send(NJoinRoom(room));
}

void MuseekDriver::doLeaveRoom(const QString& room) {
	send(NLeaveRoom(room));
}

void MuseekDriver::getUserShares(const QString& user) {
	send(NUserShares(user));
}

void MuseekDriver::getUserInfo(const QString& user) {
	send(NUserInfo(user));
}

void MuseekDriver::doDownloadFileTo(const QString& user, const QString& path, const QString& local, Q_INT64 size) {
	send(NDownloadFileTo(user, path, local, size));
}

void MuseekDriver::doDownloadFile(const QString& user, const QString& path, Q_INT64 size) {
	send(NDownloadFile(user, path, "", size));
}

void MuseekDriver::getFolderContents(const QString& user, const QString& path) {
	send(NFolderContents(user, path));
}

void MuseekDriver::doUploadFile(const QString& user, const QString& path) {
	send(NUploadFile(user, path));
}

void MuseekDriver::getUserExists(const QString& user) {
	send(NUserExists(user));
}

void MuseekDriver::getUserStatus(const QString& user) {
	send(NUserStatus(user));
}

void MuseekDriver::getUserData(const QString& user) {
	send(NUserStatus(user));
}

void MuseekDriver::setConfig(const QString& domain, const QString& key, const QString& value) {
	send(NConfigSet(&mContext, domain, key, value));
}

void MuseekDriver::removeConfig(const QString& domain, const QString& key) {
	send(NConfigRemove(&mContext, domain, key));
}

void MuseekDriver::getRoomList() {
	send(NGetRoomList());
}

void MuseekDriver::getGlobalRecommendations() {
	send(NGetGlobalRecommendations());
}

void MuseekDriver::doConnectServer() {
	send(NConnectServer());
}

void MuseekDriver::doDisconnectServer() {
	send(NDisconnectServer());
}

void MuseekDriver::doReloadShares() {
	send(NReloadShares());
}

void MuseekDriver::getRecommendations() {
	send(NGetRecommendations());
}

void MuseekDriver::getItemRecommendations(const QString& item) {
	send(NGetItemRecommendations(item));
}
void MuseekDriver::getSimilarUsers() {
	send(NGetSimilarUsers());
}
void MuseekDriver::getItemSimilarUsers(const QString& item) {
	send(NGetItemSimilarUsers(item));
}
void MuseekDriver::doAddInterest(const QString& interest) {
	send(NAddInterest(interest));
}
void MuseekDriver::doAddHatedInterest(const QString& interest) {
	send(NAddHatedInterest(interest));
}
void MuseekDriver::doRemoveInterest(const QString& interest) {
	send(NRemoveInterest(interest));
}
void MuseekDriver::doRemoveHatedInterest(const QString& interest) {
	send(NRemoveHatedInterest(interest));
}


void MuseekDriver::doStopSearch(uint token) {
	send(NSearchResults(token));
}

void MuseekDriver::doRemoveTransfer(bool isUpload, const QString& user, const QString& path) {
	send(NTransferRemove(isUpload, user, path));
}

void MuseekDriver::doAbortTransfer(bool isUpload, const QString& user, const QString& path) {
	send(NTransferAbort(isUpload, user, path));
}

void MuseekDriver::doGetIPAddress(const QString& user) {
	send(NUserAddress(user));
}

void MuseekDriver::setUserImage(const QByteArray& d) {
	send(NConfigSetUserImage(d));
}

void MuseekDriver::updateTransfer(const QString& user, const QString& path) {
	send(NTransferUpdate(user, path));
}

void MuseekDriver::checkPrivileges() {
	send(NCheckPrivileges());
}

void MuseekDriver::setStatus(uint status) {
	send(NSetStatus(status));
}

void MuseekDriver::givePrivileges(const QString& user, uint days) {
	send(NGivePrivileges(user, days));
}

void MuseekDriver::setTicker(const QString& room, const QString& message) {
	send(NRoomTickerSet(room, message));
}

