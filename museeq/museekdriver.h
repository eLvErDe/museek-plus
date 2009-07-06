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

#ifndef MUSEEKDRIVER_H
#define MUSEEKDRIVER_H

#include "museeqtypes.h"

#include <QObject>
#include <QtNetwork/QTcpSocket>

#include <Mucipher/mucipher.h>

class MuseekMessage;

class MuseekDriver : public QObject {
	Q_OBJECT
public:
	MuseekDriver(QObject* = 0, const char* = 0);

	void connectToHost(const QString&, quint16, const QString&);
	void connectToUnix(const QString& path, const QString& password);

signals:
	void hostFound();
	void connected();
	void connectionClosed();
	void error(QAbstractSocket::SocketError);

	void loggedIn(bool, const QString&);
	void privilegesLeft(uint);
	void statusSet(uint);
	void serverState(bool, const QString&);
	void statusMessage(bool, const QString&);
	void roomList(const NRoomList&);

	void Recommendations(const NGlobalRecommendations&);
	void aRecommendations(const NRecommendations&);
	void similarUsers(const NSimilarUsers&);
	void itemRecommendations(const QString& item, const NItemRecommendations&);
	void itemSimilarUsers(const QString& item, const NItemSimilarUsers&);

	void addWishItem(const QString&, uint);
	void removeWishItem(const QString&);

	void addInterest(const QString&);
	void addHatedInterest(const QString&);
	void removeInterest(const QString&);
	void removeHatedInterest(const QString&);

	void searchToken(const QString&, uint);
	void searchResults(uint, const QString&, bool, uint, uint, const NFolder&);
	void sayChatroom(const QString&, const QString&, const QString&);
	void joinRoom(const QString&, const NRoom&, const QString&, const QStringList&);
	void leaveRoom(const QString&);
	void userJoined(const QString&, const QString&, const NUserData&);
	void userLeft(const QString&, const QString&);
	void roomTickers(const QString&, const NTickers&);
	void roomTickerSet(const QString&, const QString&, const QString&);
	void roomMembers(const NRooms&, const NPrivRoomOperators&, const NPrivRoomOwners&);
	void roomsTickers(const NTickerMap&);
	void privRoomAlterableMembers(const QString&, const QStringList&);
	void privRoomAlterableOperators(const QString&, const QStringList&);
	void privRoomAddedUser(const QString&, const QString&);
	void privRoomRemovedUser(const QString&, const QString&);
	void privRoomAddedOperator(const QString&, const QString&);
	void privRoomRemovedOperator(const QString&, const QString&);
	void privateMessage(uint, uint, const QString&, const QString&);
	void userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool);
	void userInterests(const QString&, const QStringList&, const QStringList&);
	void userShares(const QString&, const NShares&);
	void transferState(const NTransfers&, const NTransfers&);
	void transferUpdate(bool, const NTransfer&);
	void transferRemove(bool, const QString&, const QString&);
	void userExists(const QString&, bool);
	void userStatus(const QString&, uint);
	void userData(const QString&, uint, uint, const QString&);
	void userAddress(const QString&, const QString&, uint);
	void newPasswordSet(const QString&);
	void configState(const QMap<QString, QMap<QString, QString> >&);
	void configSet(const QString&, const QString&, const QString&);
	void configRemove(const QString&, const QString&);

	void askedPublicChat();
	void stoppedPublicChat();
	void receivedPublicChat(const QString&, const QString&, const QString&);

	void privRoomToggled(bool);
	void privRoomList(const NPrivRoomList&);

public slots:
	void disconnect();

	void checkPrivileges();
	void setStatus(uint);
	void getRoomList();
	void getGlobalRecommendations();
	void getRecommendations();
	void doConnectServer();
	void doDisconnectServer();
	void doReloadShares();
	void getItemRecommendations(const QString&);
	void getSimilarUsers();
	void getItemSimilarUsers(const QString&);
	void doSayChatroom(const QString&, const QString&);
	void doSendPrivateMessage(const QString&, const QString&);
	void doMessageBuddies(const QString&);
	void doMessageDownloadingUsers(const QString&);
	void doMessageUsers(const QString&, const QStringList&);
	void doStartSearch(uint, const QString&);
	void doStartUserSearch(const QString&, const QString&);
	void doStartWishListSearch(const QString&);
	void doStopSearch(uint);

	void doAddWishItem(const QString&);
	void doRemoveWishItem(const QString&);

	void doAddInterest(const QString&);
	void doAddHatedInterest(const QString&);
	void doRemoveInterest(const QString&);
	void doRemoveHatedInterest(const QString&);

	void doJoinRoom(const QString&, bool);
	void doLeaveRoom(const QString&);
	void getUserInfo(const QString&);
	void getUserInterests(const QString&);
	void getUserShares(const QString&);
	void givePrivileges(const QString&, uint);
	void doDownloadFile(const QString&, const QString&, qint64);
	void doDownloadFileTo(const QString&, const QString&, const QString&, qint64);
	void getFolderContents(const QString&, const QString&);
	void doDownloadFolderTo(const QString&, const QString&, const QString&);
	void doUploadFolder(const QString&, const QString&);
	void doUploadFile(const QString&, const QString&);
	void getUserExists(const QString&);
	void askPublicChat();
	void stopPublicChat();
	void getUserStatus(const QString&);
	void setNewPassword(const QString&);
	void setConfig(const QString&, const QString&, const QString&);
	void removeConfig(const QString&, const QString&);
	void doRemoveTransfer(bool, const QString&, const QString&);
	void doAbortTransfer(bool, const QString&, const QString&);
	void doGetIPAddress(const QString&);
	void setUserImage(const QByteArray&);
	void updateTransfer(const QString&, const QString&);
	void setTicker(const QString&, const QString&);

    void doPrivRoomToggle(bool);
    void doPrivRoomAddUser(const QString&, const QString&);
    void doPrivRoomRemoveUser(const QString&, const QString&);
    void doPrivRoomDismember(const QString&);
    void doPrivRoomDisown(const QString&);
    void doPrivRoomAddOperator(const QString&, const QString&);
    void doPrivRoomRemoveOperator(const QString&, const QString&);

protected:
	void send(const MuseekMessage&);

protected slots:
	void dataReady();
	void readMessage();

private:
	QTcpSocket* mSocket;
	bool mHaveSize;
	uint mMsgSize;
	QString mPassword;
	CipherContext mContext;
};

#endif // MUSEEKDRIVER_H
