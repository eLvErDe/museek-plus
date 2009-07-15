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

#ifndef MAIN_H
#define MAIN_H

#include "museeqtypes.h"

#include <QObject>
#include <QList>
#include <QSystemTrayIcon>
#include <QtNetwork/QTcpSocket>

class MuseekDriver;
class OnlineAlert;
class MainWindow;

class QMenu;
class QApplication;
class QSettings;
class QUrl;
class QStringList;
#ifdef HAVE_QTSCRIPT
class Script;
#endif //HAVE_QTSCRIPT

class Museeq : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString nickname READ nickname)
	Q_PROPERTY(bool connected READ isConnected)
	Q_PROPERTY(bool away READ isAway WRITE setAway)

public:
	Museeq(QApplication *);
	bool mOnlineAlert, mShowTimestamps, mIPLog, mUsetray, mLogRooms, mLogPrivate;
	uint mTickerLength;
	inline MuseekDriver* driver() const { return mDriver; }
	inline bool isConnected() const { return mConnected; }
	QMenu *mTrayMenu;
	inline MainWindow* mainwin() const { return mMainWin; }

	inline const QString& nickname() const { return mNickname; }

	inline const QMap<QString, QString>& buddies() const { return mBuddies; }
	inline bool isBuddy(const QString& u) const { return mBuddies.contains(u); }

	inline bool hasAlert(const QString& u) const { return isBuddy(u) && mAlerts.contains(u); }

	inline bool isBanned(const QString& u) const { return mBanned.contains(u); }
	inline bool isTrusted(const QString& u) const { return mTrusted.contains(u); }
	inline const QStringList& ignored() const { return mIgnored; }
	inline bool isIgnored(const QString& u) const { return mIgnored.contains(u); }

    inline bool isInWishlist(const QString& q) {return mWishlist.contains(q);}

	inline const QStringList& autoJoined() const { return mAutoJoin; }
	inline bool isAutoJoined(const QString& r) const { return mAutoJoin.contains(r); }
	void output(const QString& message);
	inline const QStringList& joinedRooms() const { return mJoinedRooms; }
	inline bool isJoined(const QString& r) const { return mJoinedRooms.contains(r); }

    bool isRoomOperated(const QString&);
    bool isRoomOwned(const QString&);
    bool isRoomPrivate(const QString&);
    NPrivRoomList getPrivRoomList();
    bool canAddAsMember(const QString&, const QString&);
    bool canDismember(const QString&, const QString&);
    bool canAddAsOperator(const QString&, const QString&);
    bool canDisop(const QString&, const QString&);

	const QMap<QString, QString>& protocolHandlers() const { return mProtocolHandlers; }
	QString mColorBanned, mColorBuddied, mColorTime, mColorMe, mColorNickname, mColorTrusted, mColorRemote, mFontTime, mFontMessage, mIconTheme, mPrivateLogDir, mRoomLogDir;
	bool isAway() const { return mAway; }
	QSystemTrayIcon* trayicon() { return mTray; }

	int scriptCallbackId();
	bool hasScript(const QString&);

	QSettings* settings() {return mSettings;};

public slots:

	inline const QStringList& banned() const { return mBanned; }
	inline const QStringList& trusted() const { return mTrusted; }

	const QString& config(const QString& domain, const QString& key);

	void registerMenu(const QString&, QMenu *);
	void loadScript(const QString&);
	void unloadScript(const QString&);

	void showURL(const QUrl&);
	void startDaemon();
	void stopDaemon();
	void setAway(bool);

	void addBuddy(const QString&, const QString& = QString::null);
	void removeBuddy(const QString&);
	void addAlert(const QString&);
	void removeAlert(const QString&);
	void addBanned(const QString&, const QString& = QString::null);
	void removeBanned(const QString&);
	void addIgnored(const QString&, const QString& = QString::null);
	void removeIgnored(const QString&);
	void addTrusted(const QString&, const QString& = QString::null);
	void removeTrusted(const QString&);
	void editComments(const QString&);

	void joinRoom(const QString&, bool = false);
	void leaveRoom(const QString&);

	void addWishItem(const QString&);
	void removeWishItem(const QString&);

	void addInterest(const QString&);
	void addHatedInterest(const QString&);
	void removeInterest(const QString&);
	void removeHatedInterest(const QString&);

	void updateRoomList();

	void updateGlobalRecommendations();
	void updateRecommendations();
	void updateItemRecommendations(const QString&);
	void updateItemSimilarUsers(const QString&);
	void updateSimilarUsers();

	void setAutoJoin(const QString&, bool);
	void sayRoom(const QString&, const QString&);
	void sayPrivate(const QString&, const QString&);
	void messageBuddies(const QString&);
	void messageDownloadingUsers(const QString&);
	void messageUsers(const QString&, const QStringList&);
	void setTicker(const QString&, const QString&);
	void slotUserExists(const QString&);

	void askPublicChat();
	void stopPublicChat();

	void privRoomListReceived(const NPrivRoomList&);

	void updateTransfer(const QString&, const QString&);
	void downloadFile(const QString&, const QString&, qint64);
	void downloadFileTo(const QString&, const QString&, const QString&,qint64);
	void downloadFolder(const QString&, const QString&);
	void downloadFolderTo(const QString&, const QString&, const QString&);
	void uploadFolder(const QString&, const QString&);
	void uploadFile(const QString&, const QString&);

	void removeDownload(const QString&, const QString&);
	void removeDownloads(const QList<QPair<QString, QString> >&);
	void abortDownload(const QString&, const QString&);
	void abortDownloads(const QList<QPair<QString, QString> >&);

	void removeUpload(const QString&, const QString&);
	void removeUploads(const QList<QPair<QString, QString> >&);
	void abortUpload(const QString&, const QString&);
	void abortUploads(const QList<QPair<QString, QString> >&);

	void search(const QString&);
	void buddySearch(const QString&);
	void roomSearch(const QString&);
	void userSearch(const QString&, const QString&);
	void wishListSearch(const QString&);
	void terminateSearch(uint);

	void getUserInfo(const QString&);
	void getUserInterests(const QString&);
	void getUserShares(const QString&);
	void setNewPassword(const QString&);

	void setConfig(const QString&, const QString&, const QString&);
	void removeConfig(const QString&, const QString&);
	void setProtocolHandlers(const QMap<QString, QString>&);

	void flush();

	void disconnectServer();
	void connectServer();
	void reloadShares();
	void saveSettings();
	void trayicon_load();
	void trayicon_setIcon(const QString&);
	void trayicon_show();
	void trayicon_hide();

signals:
	// Museekd related signals
	void connected();
	void disconnected();

	void closingMuseeq();

	void configChanged(const QString& domain, const QString& key, const QString& value);

	// Server related signals
	void connectedToServer();
	void disconnectedFromServer();
	void connectedToServer(bool);
	void nicknameChanged(const QString&);
	void newPasswordSet(const QString&);

	// User status and statistics
	void userStatus(const QString&, uint);
	void doUpdateStatus(const QString&);
	void userData(const QString&, uint, uint, const QString&);

	// Buddy list signals
	void sortingEnabled(bool);
	void addedBuddy(const QString&, const QString&);
	void removedBuddy(const QString&);

	void addedBanned(const QString&, const QString&);
	void removedBanned(const QString&);

	void addedIgnored(const QString&, const QString&);
	void removedIgnored(const QString&);

	void addedTrusted(const QString&, const QString&);
	void removedTrusted(const QString&);

	// Chat related signals
	void roomList(const NRoomList&);
	void joinedRoom(const QString&, const NRoom&, const QString&, const QStringList&);
	void leftRoom(const QString&);

	void userJoinedRoom(const QString&, const QString&, const NUserData&);
	void userLeftRoom(const QString&, const QString&);

	void saidChatroom(const QString&, const QString&, const QString&);
	void autoJoin(const QString&, bool);
	void roomTickers(const QString&, const NTickers&);
	void roomTickerSet(const QString&, const QString&, const QString&);
	void roomMembers(const NRooms&, const NPrivRoomOperators&, const NPrivRoomOwners&);
	void roomsTickers(const NTickerMap&);
	void showAllTickers();
	void hideAllTickers();
	void privateMessage(uint, uint, const QString&, const QString&);

	void askedPublicChat();
	void stoppedPublicChat();
	void receivedPublicChat(const QString&, const QString&, const QString&);

	void privRoomToggled(bool);
	void privRoomList(const NPrivRoomList&);

	// Interests & Recommendations
	void Recommendations(const NGlobalRecommendations&);
	void aRecommendations(const NRecommendations&);
	void similarUsers( const NSimilarUsers&);
	void itemSimilarUsers(const QString&, const NItemSimilarUsers&);
	void itemRecommendations(const QString&, const NItemRecommendations&);

	void addedInterest(const QString& interest);
	void addedHatedInterest(const QString&);
 	void removedInterest(const QString&);
 	void removedHatedInterest(const QString&);
	void addedWishItem(const QString&, uint);
 	void removedWishItem(const QString&);
	// Transfer related signals
	void downloadUpdated(const NTransfer&);
	void uploadUpdated(const NTransfer&);
	void transfersSorting(bool);
	void downloadRemoved(const QString&, const QString&);
	void uploadRemoved(const QString&, const QString&);

	// Search related signals:
	void searchResults(uint, const QString&, bool, uint, uint, const NFolder&);
	void searchToken(const QString&, uint);

	// Peer stuff:
	void userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool);
	void userInterests(const QString&, const QStringList&, const QStringList&);
	void userShares(const QString&, const NShares&);

	void toggleCountries(bool);

protected slots:
	void slotConnectionClosed();
	void slotError(QAbstractSocket::SocketError);

	void slotLoggedIn(bool, const QString&);
	void slotServerState(bool, const QString&);
	void slotRoomMembers(const NRooms&, const NPrivRoomOperators&, const NPrivRoomOwners&);
	void slotTransferState(const NTransfers&, const NTransfers&);
	void slotTransferUpdate(bool, const NTransfer&);
	void slotConfigState(const QMap<QString, QMap<QString, QString> >&);
	void slotConfigSet(const QString&, const QString&, const QString&);
	void slotConfigRemove(const QString&, const QString&);
	void slotLeftRoom(const QString&);

	void slotTransferRemoved(bool, const QString&, const QString&);
	void slotStatusSet(uint);

    void receivedPrivRoomAlterableMembers(const QString&, const QStringList&);
    void receivedPrivRoomAlterableOperators(const QString&, const QStringList&);
    void privRoomAddedUser(const QString&, const QString&);
    void privRoomRemovedUser(const QString&, const QString&);
    void privRoomAddedOperator(const QString&, const QString&);
    void privRoomRemovedOperator(const QString&, const QString&);

	void slotTrayIconActivated(QSystemTrayIcon::ActivationReason);

protected:
	QString handleInput(bool, const QString&, QString);

private:
	QApplication * mApplication;
	QString mNickname;
	MuseekDriver* mDriver;
	QSystemTrayIcon* mTray;
	bool mConnected, mAway;
	QStringList mBanned, mIgnored, mTrusted, mAutoJoin, mJoinedRooms, mLovedInterests, mHatedInterests, mWishlist;
	QMap<QString, QString> mBuddies;
	QMap<QString, OnlineAlert *> mAlerts;

	NPrivRoomList m_privRoomListCache;
	QMap<QString, QStringList> mPrivRoomAlterableMembers, mPrivRoomAlterableOperators;

	MainWindow* mMainWin;

	QMap<QString, QMap<QString, QString> > mConfig; // Copy of museekd config
	QMap<QString, QString> mProtocolHandlers; // Protocol handlers (ex: http -> firefox)

	QSettings* mSettings;

#ifdef HAVE_QTSCRIPT
    QList<Script*> mScripts; // List of all loaded scripts
	QMap<QString, QMenu*> mMenus; // Menus that should be registered to any new script
	int mCallbackCount; // Used to give an ID for callbacks
#endif //HAVE_QTSCRIPT
};

extern Museeq* museeq;

#endif // MAIN_H
