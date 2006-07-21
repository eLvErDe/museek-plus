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

#ifndef MAIN_H
#define MAIN_H

#include "museeqtypes.h"

#include <qobject.h>
#include <qstringlist.h>

#ifdef HAVE_QSA
#include <qptrstack.h>
#include <qsproject.h>
#endif // HAVE_QSA

class MuseekDriver;
class MainWindow;
class QPopupMenu;
class QApplication;
class OnlineAlert;

class Museeq : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString nickname READ nickname)
	Q_PROPERTY(bool connected READ isConnected)
	Q_PROPERTY(bool away READ isAway WRITE setAway)
	
public:
	Museeq(QApplication *);
	bool mShowTickers, mShowStatusLog, mOnlineAlert, mShowTimestamps, mIPLog;
	inline MuseekDriver* driver() const { return mDriver; }
	inline bool isConnected() const { return mConnected; }
	
	inline MainWindow* mainwin() const { return mMainWin; }
	
	inline const QString& nickname() const { return mNickname; }
	
	inline const QStringList& buddies() const { return mBuddies; }
	inline bool isBuddy(const QString& u) const { return mBuddies.find(u) != mBuddies.end(); }
	
	inline bool hasAlert(const QString& u) const { return isBuddy(u) && mAlerts.find(u) != mAlerts.end(); }
	
	inline bool isBanned(const QString& u) const { return mBanned.find(u) != mBanned.end(); }
	inline bool isTrusted(const QString& u) const { return mTrusted.find(u) != mTrusted.end(); }	
	inline const QStringList& ignored() const { return mIgnored; }
	inline bool isIgnored(const QString& u) const { return mIgnored.find(u) != mIgnored.end(); }
	
	inline const QStringList& autoJoined() const { return mAutoJoin; }
	inline bool isAutoJoined(const QString& r) const { return mAutoJoin.find(r) != mAutoJoin.end(); }
	
	inline const QStringList& joinedRooms() const { return mJoinedRooms; }
	inline bool isJoined(const QString& r) const { return mJoinedRooms.find(r) != mJoinedRooms.end(); }
	
	const QMap<QString, QString>& protocolHandlers() const { return mProtocolHandlers; }
	QString mColorBanned, mColorBuddied, mColorTime, mColorMe, mColorNickname, mColorTrusted, mColorRemote, mFontTime, mFontMessage, mIconTheme;
	bool isAway() const { return mAway; }
	


public slots:
	inline const QStringList& banned() const { return mBanned; }
	inline const QStringList& trusted() const { return mTrusted; }
	
	const QString& config(const QString& domain, const QString& key);
	
	void registerMenu(const QString&, QPopupMenu *);
	void addMenu(const QString&, const QString&, const QString&);
	void addInputHandler(const QString&);
	void loadScript(const QString&);
	void unloadScript(const QString&);
	
	void showURL(const QString&);
	void startDaemon();
	void stopDaemon();
	void saveConnectConfig();
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

	void joinRoom(const QString&);
	void leaveRoom(const QString&);

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
	void setTicker(const QString&, const QString&);
	void slotUserExists(const QString&);
	
	void updateTransfer(const QString&, const QString&);
	void downloadFile(const QString&, const QString&, Q_INT64);
	void downloadFolder(const QString&, const QString&);
	void uploadFile(const QString&, const QString&);
	
	void removeDownload(const QString&, const QString&);
	void removeDownloads(const QValueList<QPair<QString, QString> >&);
	void abortDownload(const QString&, const QString&);
	void abortDownloads(const QValueList<QPair<QString, QString> >&);
	
	void removeUpload(const QString&, const QString&);
	void removeUploads(const QValueList<QPair<QString, QString> >&);
	void abortUpload(const QString&, const QString&);
	void abortUploads(const QValueList<QPair<QString, QString> >&);
	
	void search(const QString&);
	void buddySearch(const QString&);
	void roomSearch(const QString&);
	void userSearch(const QString&, const QString&);
	void wishListSearch(const QString&);
	void terminateSearch(uint);
	
	void getUserInfo(const QString&);
	void getUserShares(const QString&);
	
	void setConfig(const QString&, const QString&, const QString&);
	void setProtocolHandlers(const QMap<QString, QString>&);
	
	void flush();

	void disconnectServer();
	void connectServer();
	void reloadShares();
	void saveSettings();
	 
signals:
	// Museekd related signals
	void connected();
	void disconnected();
	
	void configChanged(const QString& domain, const QString& key, const QString& value);
	
	// Server related signals
	void connectedToServer();
	void disconnectedFromServer();
	void connectedToServer(bool);
	void nicknameChanged(const QString&);
	
	// User status and statistics
	void userStatus(const QString&, uint);
	void doUpdateStatus(const QString&);
	void userData(const QString&, uint, uint);
	
	// Buddy list signals
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
	void joinedRoom(const QString&, const NRoom&);
	void leftRoom(const QString&);

	void userJoinedRoom(const QString&, const QString&, const NUserData&);
	void userLeftRoom(const QString&, const QString&);

	void saidChatroom(const QString&, const QString&, const QString&);
	void autoJoin(const QString&, bool);
	void roomTickers(const QString&, const NTickers&);
	void roomTickerSet(const QString&, const QString&, const QString&);
	void showAllTickers();
	void hideAllTickers();
	void privateMessage(uint, uint, const QString&, const QString&);

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
	// Transfer related signals
	void downloadUpdated(const NTransfer&);
	void uploadUpdated(const NTransfer&);
	
	void downloadRemoved(const QString&, const QString&);
	void uploadRemoved(const QString&, const QString&);
	
	// Search related signals:
	void searchResults(uint, const QString&, bool, uint, uint, const NFolder&);
	void searchToken(const QString&, uint);
	
	// Peer stuff:
	void userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool);
	void userShares(const QString&, const NShares&);

	



protected slots:
	void slotMenuActivated(int);
	
	void slotConnectionClosed();
	void slotError(int);
	
	void slotLoggedIn(bool, const QString&);
	void slotServerState(bool, const QString&);
	void slotRoomState(const NRoomList&, const NRooms&, const NTickerMap&);
	void slotTransferState(const NTransfers&, const NTransfers&);
	void slotTransferUpdate(bool, const NTransfer&);
	void slotConfigState(const QMap<QString, QMap<QString, QString> >&);
	void slotConfigSet(const QString&, const QString&, const QString&);
	void slotConfigRemove(const QString&, const QString&);
	void slotJoinedRoom(const QString&, const NRoom&);
	void slotLeftRoom(const QString&);
	


	void slotTransferRemoved(bool, const QString&, const QString&);
	void slotStatusSet(uint);
	
protected:
	QString handleInput(bool, const QString&, const QString&);
	
private:
	QApplication * mApplication;
	QString mNickname;
	MuseekDriver* mDriver;
	bool mConnected, mAway;
	QStringList mBuddies, mBanned, mIgnored, mTrusted, mAutoJoin, mJoinedRooms, mLovedInterests, mHatedInterests;
	QMap<QString, OnlineAlert *> mAlerts;
	
	MainWindow* mMainWin;
	
	QMap<QString, QMap<QString, QString> > mConfig;
	QMap<QString, QString> mProtocolHandlers;
	
#ifdef HAVE_QSA
	QMap<QString, QPopupMenu*> mMenus;
	int mCallbackCount;
	QMap<int, QPair<QSProject*, QString> > mCallbacks;
	QMap<QString, QSProject*> mProjects;
	QPtrStack<QSProject> mScriptContext;
	QValueList<QPair<QSProject *, QString> > mInputHandlers;
#endif
};

extern Museeq* museeq;

#endif // MAIN_H
