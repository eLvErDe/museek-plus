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

#include "museeq.h"
#include "prefix.h"
#include <system.h>
#include <string>
#include <iostream>

#include "museekdriver.h"
#include "mainwin.h"
#include "onlinealert.h"
#include "images.h"
#ifdef HAVE_QTSCRIPT
#include "script.h"
#endif // HAVE_QTSCRIPT

#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QList>
#include <QFileInfo>

using std::string;
Museeq::Museeq(QApplication * app)
      : QObject(0), mApplication(app), mConnected(false) {
#ifdef HAVE_QTSCRIPT
	mCallbackCount = 1000;
#endif // HAVE_QTSCRIPT

	museeq = this;
	mDriver = new MuseekDriver(this, "driver");
	mUsetray = false;
	mLogRooms = mLogPrivate = false;
	mFontTime = "font-family:fixed-width";
	mRoomLogDir = mPrivateLogDir = mFontMessage =  "";
	mOnlineAlert = false;
	mColorBanned = mColorBuddied = mColorTime = mColorNickname = mColorTrusted = mColorRemote = mColorMe = "";
	mIPLog = false;
	mTickerLength = 50;
	mSettings = new QSettings("MuseekPlus.org","Museeq");

	connect(mDriver, SIGNAL(connectionClosed()), SLOT(slotConnectionClosed()));
	connect(mDriver, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(slotError(QAbstractSocket::SocketError)));
	connect(mDriver, SIGNAL(loggedIn(bool, const QString&)), SLOT(slotLoggedIn(bool, const QString&)));
	connect(mDriver, SIGNAL(serverState(bool, const QString&)), SLOT(slotServerState(bool, const QString&)));
	connect(mDriver, SIGNAL(statusSet(uint)), SLOT(slotStatusSet(uint)));
	connect(mDriver, SIGNAL(newPasswordSet(const QString&)), SIGNAL(newPasswordSet(const QString&)));

	connect(mDriver, SIGNAL(transferState(const NTransfers&, const NTransfers&)), SLOT(slotTransferState(const NTransfers&, const NTransfers&)));
	connect(mDriver, SIGNAL(transferUpdate(bool, const NTransfer&)), SLOT(slotTransferUpdate(bool, const NTransfer&)));
	connect(mDriver, SIGNAL(configState(const QMap<QString, QMap<QString, QString> >&)), SLOT(slotConfigState(const QMap<QString, QMap<QString, QString> >&)));
	connect(mDriver, SIGNAL(configSet(const QString&, const QString&, const QString&)), SLOT(slotConfigSet(const QString&, const QString&, const QString&)));
	connect(mDriver, SIGNAL(configRemove(const QString&, const QString&)), SLOT(slotConfigRemove(const QString&, const QString&)));
	connect(mDriver, SIGNAL(userStatus(const QString&, uint)), SIGNAL(userStatus(const QString&, uint)));
	connect(mDriver, SIGNAL(userData(const QString&, uint, uint, const QString&)), SIGNAL(userData(const QString&, uint, uint, const QString&)));
	connect(mDriver, SIGNAL(joinRoom(const QString&, const NRoom&, const QString&, const QStringList&)), SIGNAL(joinedRoom(const QString&, const NRoom&, const QString&, const QStringList&)));
	connect(mDriver, SIGNAL(leaveRoom(const QString&)), SIGNAL(leftRoom(const QString&)));
	connect(mDriver, SIGNAL(leaveRoom(const QString&)), SLOT(slotLeftRoom(const QString&)));
	connect(mDriver, SIGNAL(userJoined(const QString&, const QString&, const NUserData&)), SIGNAL(userJoinedRoom(const QString&, const QString&, const NUserData&)));
	connect(mDriver, SIGNAL(userLeft(const QString&, const QString&)), SIGNAL(userLeftRoom(const QString&, const QString&)));

	connect(mDriver, SIGNAL(sayChatroom(const QString&, const QString&, const QString&)), SIGNAL(saidChatroom(const QString&, const QString&, const QString&)));
	connect(mDriver, SIGNAL(privateMessage(uint, uint, const QString&, const QString&)), SIGNAL(privateMessage(uint, uint, const QString&, const QString&)));
	connect(mDriver, SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)), SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)));
	connect(mDriver, SIGNAL(searchToken(const QString&, uint)), SIGNAL(searchToken(const QString&, uint)));
	connect(mDriver, SIGNAL(userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)), SIGNAL(userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)));
	connect(mDriver, SIGNAL(userInterests(const QString&, const QStringList&, const QStringList&)), SIGNAL(userInterests(const QString&, const QStringList&, const QStringList&)));
	connect(mDriver, SIGNAL(userShares(const QString&, const NShares&)), SIGNAL(userShares(const QString&, const NShares&)));
	connect(mDriver, SIGNAL(roomList(const NRoomList&)), SIGNAL(roomList(const NRoomList&)));
	connect(mDriver, SIGNAL(Recommendations(const NGlobalRecommendations&)), SIGNAL(Recommendations(const NGlobalRecommendations&)));
	connect(mDriver, SIGNAL(aRecommendations(const NRecommendations&)), SIGNAL(aRecommendations(const NRecommendations&)));

	connect(mDriver, SIGNAL(itemSimilarUsers(const QString&, const NItemSimilarUsers&)), SIGNAL(itemSimilarUsers(const QString&, const NItemSimilarUsers&)));
	connect(mDriver, SIGNAL(itemRecommendations(const QString&, const NItemRecommendations&)), SIGNAL(itemRecommendations(const QString&, const NItemRecommendations&)));
	connect(mDriver, SIGNAL(similarUsers(const NSimilarUsers&)), SIGNAL(similarUsers(const NSimilarUsers&)));

	connect(mDriver, SIGNAL(addInterest(const QString&)), SIGNAL(addedInterest(const QString&)));
	connect(mDriver, SIGNAL(addHatedInterest(const QString&)), SIGNAL(addedHatedInterest(const QString&)));
	connect(mDriver, SIGNAL(removeInterest(const QString&)), SIGNAL(removedInterest(const QString&)));
	connect(mDriver, SIGNAL(removeHatedInterest(const QString&)), SIGNAL(removedHatedInterest(const QString&)));

	connect(mDriver, SIGNAL(roomTickers(const QString&, const NTickers&)), SIGNAL(roomTickers(const QString&, const NTickers&)));
	connect(mDriver, SIGNAL(roomTickerSet(const QString&, const QString&, const QString&)), SIGNAL(roomTickerSet(const QString&, const QString&, const QString&)));
	connect(mDriver, SIGNAL(roomMembers(const NRooms&, const NPrivRoomOperators&, const NPrivRoomOwners&)), SLOT(slotRoomMembers(const NRooms&, const NPrivRoomOperators&, const NPrivRoomOwners&)));
	connect(mDriver, SIGNAL(roomsTickers(const NTickerMap&)), SIGNAL(roomsTickers(const NTickerMap&)));

	connect(mDriver, SIGNAL(transferRemove(bool, const QString&, const QString&)), SLOT(slotTransferRemoved(bool, const QString&, const QString&)));

	connect(mDriver, SIGNAL(askedPublicChat()), SIGNAL(askedPublicChat()));
	connect(mDriver, SIGNAL(stoppedPublicChat()), SIGNAL(stoppedPublicChat()));
	connect(mDriver, SIGNAL(receivedPublicChat(const QString&, const QString&, const QString&)), SIGNAL(receivedPublicChat(const QString&, const QString&, const QString&)));

	connect(mDriver, SIGNAL(privRoomToggled(bool)), SIGNAL(privRoomToggled(bool)));
	connect(mDriver, SIGNAL(privRoomList(const NPrivRoomList&)), SLOT(privRoomListReceived(const NPrivRoomList&)));
	connect(mDriver, SIGNAL(privRoomAlterableMembers(const QString&, const QStringList&)), SLOT(receivedPrivRoomAlterableMembers(const QString&, const QStringList&)));
	connect(mDriver, SIGNAL(privRoomAlterableOperators(const QString&, const QStringList&)), SLOT(receivedPrivRoomAlterableOperators(const QString&, const QStringList&)));
	connect(mDriver, SIGNAL(privRoomAddedUser(const QString&, const QString&)), SLOT(privRoomAddedUser(const QString&, const QString&)));
	connect(mDriver, SIGNAL(privRoomRemovedUser(const QString&, const QString&)), SLOT(privRoomRemovedUser(const QString&, const QString&)));
	connect(mDriver, SIGNAL(privRoomAddedOperator(const QString&, const QString&)), SLOT(privRoomAddedOperator(const QString&, const QString&)));
	connect(mDriver, SIGNAL(privRoomRemovedOperator(const QString&, const QString&)), SLOT(privRoomRemovedOperator(const QString&, const QString&)));

	QString s = mSettings->value("IconTheme").toString();
	if (s.isEmpty())
		mIconTheme = "default";
	else
		mIconTheme = s;

	mSettings->beginGroup("ProtocolHandlers");
	QStringList handlers = mSettings->childKeys();
	if(! handlers.empty()) {

		QStringList::const_iterator it, end = handlers.end();
		for(it = handlers.begin(); it != end; ++it)
			mProtocolHandlers[*it] = mSettings->value(*it).toString();
	} else
		mProtocolHandlers["http"] = "firefox $";
	mSettings->endGroup();
	mMainWin = new MainWindow(0);

	connect(mMainWin, SIGNAL(toggleCountries(bool)), SIGNAL(toggleCountries(bool)));
	connect(mMainWin, SIGNAL(closingMuseeq()), SIGNAL(closingMuseeq()));
	emit toggleCountries(settings()->value("showCountries", false).toBool());

	emit disconnectedFromServer();
	emit connectedToServer(false);

	QDir homedir = QDir::home();
	if(! homedir.exists(".museeq")) {
		// Create ~/.museeq directory
		homedir.mkdir (".museeq");
	}

#ifdef HAVE_QTSCRIPT
    // TODO add a dialog to load scripts in /usr/share/museekd/museeq/scripts
    QDir dir = QDir::home();
    if(dir.cd(".museeq")) {
        QStringList filters;
        filters << "*.qs";
        QStringList scripts = dir.entryList(filters);
        QStringList::iterator it = scripts.begin();
        for(; it != scripts.end(); ++it) {
            QString fn = dir.absolutePath() + QDir::separator() + *it;
            loadScript(fn);
        }
    }
#endif // HAVE_QTSCRIPT

	mTrayMenu = new QMenu();
	mTrayMenu->addAction(tr("&Restore"), mMainWin , SLOT( showNormal() ) );
	mTrayMenu->addAction(tr("&Museek settings"),  mMainWin , SLOT( changeSettings()) );
	mTrayMenu->addAction(tr("&Hide"),  mMainWin , SLOT( hide()  ) );
	mTrayMenu->addSeparator();
	mTrayMenu->addAction( tr("&Quit"),  mMainWin , SLOT( slotClose() ) );
	mTray =  new QSystemTrayIcon( IMG("icon"), this);
	mTray->setToolTip(tr("Museeq"));
	mTray->setContextMenu(mTrayMenu);
	QObject::connect( mTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),  this, SLOT(slotTrayIconActivated(QSystemTrayIcon::ActivationReason) ) );
}

/**
  * The trayicon has been activated.
  */
void Museeq::slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
	if(reason == QSystemTrayIcon::Trigger)
		mMainWin->toggleVisibility();
}

void Museeq::slotConnectionClosed() {
	mBuddies.clear();
	mBanned.clear();
	mIgnored.clear();
	mTrusted.clear();

	emit disconnected();
}

void Museeq::slotError(QAbstractSocket::SocketError err) {
	if(err == QAbstractSocket::UnknownSocketError)
// 		QSocket::ErrSocketRead
		slotConnectionClosed();
}

void Museeq::slotUserExists(const QString& user) {
	mDriver->getUserExists(user);
}

void Museeq::askPublicChat() {
	mDriver->askPublicChat();
}

void Museeq::stopPublicChat() {
	mDriver->stopPublicChat();
}

void Museeq::slotLoggedIn(bool success, const QString& msg) {
	if(success)
		emit connected();
}

void Museeq::slotServerState(bool connected, const QString& nick) {
	if(connected) {
		mNickname = nick;
		mConnected = true;
		emit connectedToServer();
		emit connectedToServer(true);
		emit nicknameChanged(nick);
	} else {
		mConnected = false;
		emit disconnectedFromServer();
		emit connectedToServer(false);
	}
}

void Museeq::slotRoomMembers(const NRooms& rooms, const NPrivRoomOperators& operators, const NPrivRoomOwners& owners) {
	mJoinedRooms.clear();

	NRooms::const_iterator it = rooms.begin();
	for(; it != rooms.end(); ++it) {
		mJoinedRooms << it.key();
		emit joinedRoom(it.key(), it.value(), owners[it.key()], operators[it.key()]);
	}
}

void Museeq::slotTransferState(const NTransfers& downloads, const NTransfers& uploads) {
	emit transfersSorting(false);
	NTransfers::const_iterator it = downloads.begin();
	for(; it != downloads.end(); ++it)
	{
		emit downloadUpdated(*it);
		flush();
	}

	it = uploads.begin();
	for(; it != uploads.end(); ++it)
	{
		emit uploadUpdated(*it);
		flush();
	}
	emit transfersSorting(true);
}

void Museeq::slotTransferUpdate(bool isUpload, const NTransfer& transfer) {
	if(isUpload)
		emit uploadUpdated(transfer);
	else
		emit downloadUpdated(transfer);
}

void Museeq::slotConfigState(const QMap<QString, QMap<QString, QString> >& _config) {
	mConfig = _config;
	emit sortingEnabled(false);
	QMap<QString, QMap<QString, QString> >::const_iterator it = _config.begin();
	for(; it != _config.end(); ++it) {
		QMap<QString, QString>::const_iterator it2 = (*it).begin();
		for(; it2 != (*it).end(); ++it2)
			slotConfigSet(it.key(), it2.key(), *it2);
	}
	emit sortingEnabled(true);
}

void Museeq::slotConfigSet(const QString& domain, const QString& key, const QString& value) {
	if(domain == "buddies") {
		mBuddies[key] = value;
		emit addedBuddy(key, value);
		emit doUpdateStatus(key);
	} else if(domain == "alerts") {
		if(mAlerts.find(key) == mAlerts.end())
		{
			OnlineAlert * dlg = new OnlineAlert(mMainWin);
			dlg->setUser(key);
			connect(dlg, SIGNAL(removeAlert(const QString&)), SLOT(removeAlert(const QString&)));
			mAlerts[key] = dlg;
		}
	} else if(domain == "banned") {
		mBanned += key;
		emit addedBanned(key, value);
		emit doUpdateStatus(key);
	} else if(domain == "ignored") {
		mIgnored += key;
		emit addedIgnored(key, value);
		emit doUpdateStatus(key);
	} else if(domain == "trusted") {
		mTrusted += key;
		emit addedTrusted(key, value);
		emit doUpdateStatus(key);
	} else if(domain == "autojoin") {
		mAutoJoin.append(key);
		emit autoJoin(key, true);
	} else if(domain == "interests.like") {
		mLovedInterests.append(key);
		emit addedInterest(key);
	} else if(domain == "interests.hate") {
		mHatedInterests.append(key);
		emit addedHatedInterest(key);
	} else if(domain == "wishlist") {
		mWishlist.append(key);
		emit addedWishItem(key, value.toUInt());
	}
	mConfig[domain][key] = value;
	emit configChanged(domain, key, value);
}

void Museeq::slotConfigRemove(const QString& domain, const QString& key) {
	if(domain == "buddies") {
		if (mBuddies.contains(key))
			mBuddies.remove(key);
		emit removedBuddy(key);
		emit doUpdateStatus(key);
		if(mAlerts.find(key) != mAlerts.end())
			removeAlert(key);
	} else if(domain == "alerts") {
		if(mAlerts.find(key) != mAlerts.end())
		{
			delete mAlerts[key];
			mAlerts.remove(key);
		}
	} else if(domain == "banned") {
		if (mBanned.indexOf(key) != -1)
			mBanned.removeAt(mBanned.indexOf(key));
		emit removedBanned(key);
		emit doUpdateStatus(key);
	} else if(domain == "ignored") {
		if (mIgnored.indexOf(key) != -1)
			mIgnored.removeAt(mIgnored.indexOf(key));
		emit removedIgnored(key);
		emit doUpdateStatus(key);
	} else if(domain == "trusted") {
		if (mTrusted.indexOf(key) != -1)
			mTrusted.removeAt(mTrusted.indexOf(key));
		emit removedTrusted(key);
		emit doUpdateStatus(key);
	} else if(domain == "autojoin") {
		if (mAutoJoin.indexOf(key) != -1)
			mAutoJoin.removeAt(mAutoJoin.indexOf(key));
		emit autoJoin(key, false);
	} else if(domain == "interests.like") {
		if (mLovedInterests.indexOf(key) != -1)
			mLovedInterests.removeAt(mLovedInterests.indexOf(key));
		emit removedInterest(key);
	} else if(domain == "interests.hate") {
		if (mHatedInterests.indexOf(key) != -1)
			mHatedInterests.removeAt(mHatedInterests.indexOf(key));
		emit removedHatedInterest(key);
	} else if(domain == "wishlist") {
		if (mWishlist.indexOf(key) != -1)
			mWishlist.removeAt(mWishlist.indexOf(key));
		emit removedWishItem(key);
	}
}

void Museeq::slotLeftRoom(const QString& room) {
	if (mJoinedRooms.indexOf(room) != -1)
		mJoinedRooms.removeAt(mJoinedRooms.indexOf(room));
}

void Museeq::startDaemon() {
	mMainWin->doDaemon();
}

void Museeq::stopDaemon() {
    driver()->disconnect();
	mMainWin->stopDaemon();
}

void Museeq::addWishItem(const QString& query) {
	mDriver->doAddWishItem(query);
}

void Museeq::removeWishItem(const QString& query) {
	mDriver->doRemoveWishItem(query);
}

void Museeq::addInterest(const QString& interest) {
	mDriver->doAddInterest(interest);
}

void Museeq::addHatedInterest(const QString& interest) {
	mDriver->doAddHatedInterest(interest);
}

void Museeq::removeInterest(const QString& interest) {
	mDriver->doRemoveInterest(interest);
}

void Museeq::removeHatedInterest(const QString& interest) {
	mDriver->doRemoveHatedInterest(interest);
}


void Museeq::setAutoJoin(const QString& room, bool on) {
	if(on)
		mDriver->setConfig("autojoin", room, "");
	else
		mDriver->removeConfig("autojoin", room);
}

void Museeq::editComments(const QString& user) {
	if (museeq->isBuddy(user)) {
	    QString current = mBuddies[user];
	    bool res;
		QString c = QInputDialog::getText(mainwin(), tr("Comments"), tr("Comments for %1").arg(user), QLineEdit::Normal, current, &res);
		if (res)
			addBuddy(user, c);
    }
}

void Museeq::addBuddy(const QString& user, const QString& comments) {
	mDriver->setConfig("buddies", user, comments);
}

void Museeq::removeBuddy(const QString& user) {
	mDriver->removeConfig("buddies", user);
}

void Museeq::addAlert(const QString& user) {
	mDriver->setConfig("alerts", user, "");
}

void Museeq::removeAlert(const QString& user)
{
	mDriver->removeConfig("alerts", user);
}

void Museeq::addBanned(const QString& user, const QString& comments) {
	mDriver->setConfig("banned", user, comments);
}

void Museeq::removeBanned(const QString& user) {
	mDriver->removeConfig("banned", user);
}

void Museeq::addIgnored(const QString& user, const QString& comments) {
	mDriver->setConfig("ignored", user, comments);
}

void Museeq::removeIgnored(const QString& user) {
	mDriver->removeConfig("ignored", user);
}

void Museeq::addTrusted(const QString& user, const QString& comments) {
	mDriver->setConfig("trusted", user, comments);
}

void Museeq::removeTrusted(const QString& user) {
	mDriver->removeConfig("trusted", user);
}

void Museeq::joinRoom(const QString& room, bool priv) {
	mDriver->doJoinRoom(room, priv);
}

void Museeq::leaveRoom(const QString& room) {
	mDriver->doLeaveRoom(room);
}

void Museeq::updateRoomList() {
	mDriver->getRoomList();
}

void Museeq::updateGlobalRecommendations() {
	mDriver->getGlobalRecommendations();
}
void Museeq::updateRecommendations() {
	mDriver->getRecommendations();
}
void Museeq::updateItemRecommendations(const QString& item) {
	mDriver->getItemRecommendations(item);
}
void Museeq::updateSimilarUsers() {
	mDriver->getSimilarUsers();
}
void Museeq::updateItemSimilarUsers(const QString& item) {
	mDriver->getItemSimilarUsers(item);
}

void Museeq::sayRoom(const QString& room, const QString& line) {
	QString _line = line;
#ifdef HAVE_QTSCRIPT
	_line = handleInput(false, room, _line);
#endif //HAVE_QTSCRIPT
	if(! _line.isEmpty())
		mDriver->doSayChatroom(room, _line);
}

void Museeq::sayPrivate(const QString& user, const QString& line) {
	QString _line = line;
#ifdef HAVE_QTSCRIPT
	_line = handleInput(true, user, _line);
#endif //HAVE_QTSCRIPT
	if(! _line.isEmpty())
		mDriver->doSendPrivateMessage(user, _line);
}

void Museeq::messageBuddies(const QString& line) {
	if(! line.isEmpty())
		mDriver->doMessageBuddies(line);
}

void Museeq::messageDownloadingUsers(const QString& line) {
	if(! line.isEmpty())
		mDriver->doMessageDownloadingUsers(line);
}

void Museeq::messageUsers(const QString& line, const QStringList& users) {
	if(! line.isEmpty())
		mDriver->doMessageUsers(line, users);
}

void Museeq::downloadFile(const QString& user, const QString& path, qint64 size) {
	mDriver->doDownloadFile(user, path, size);
}

void Museeq::downloadFileTo(const QString& user, const QString& path, const QString& localpath, qint64 size) {
	mDriver->doDownloadFileTo(user, path, localpath, size);
}

void Museeq::downloadFolder(const QString& user, const QString& path) {
	mDriver->getFolderContents(user, path);
}

void Museeq::downloadFolderTo(const QString& user, const QString& path, const QString& localpath) {
	mDriver->doDownloadFolderTo(user, path, localpath);
}

void Museeq::uploadFolder(const QString& user, const QString& path) {
	mDriver->doUploadFolder(user, path);
}

void Museeq::uploadFile(const QString& user, const QString& path) {
	mDriver->doUploadFile(user, path);
}

void Museeq::search(const QString& query) {
	mDriver->doStartSearch(0, query);
}

void Museeq::buddySearch(const QString& query) {
	mDriver->doStartSearch(1, query);
}

void Museeq::roomSearch(const QString& query) {
	mDriver->doStartSearch(2, query);
}
void Museeq::userSearch(const QString& user, const QString& query) {
	mDriver->doStartUserSearch(user, query);
}
void Museeq::wishListSearch(const QString& query) {
	mDriver->doStartWishListSearch(query);
}
void Museeq::terminateSearch(uint token) {
	mDriver->doStopSearch(token);
}

void Museeq::getUserInfo(const QString& user) {
	mDriver->getUserInfo(user);
}

void Museeq::getUserInterests(const QString& user) {
	mDriver->getUserInterests(user);
}

void Museeq::getUserShares(const QString& user) {
	mDriver->getUserShares(user);
}

void Museeq::updateTransfer(const QString& user, const QString& path) {
	mDriver->updateTransfer(user, path);
}

void Museeq::removeDownload(const QString& user, const QString& path) {
	mDriver->doRemoveTransfer(false, user, path);
}

void Museeq::removeDownloads(const QList<QPair<QString, QString> >& downloads) {
	QList<QPair<QString, QString> >::const_iterator it = downloads.begin();
	for(; it != downloads.end(); ++it)
		removeDownload((*it).first, (*it).second);
}

void Museeq::removeUpload(const QString& user, const QString& path) {
	mDriver->doRemoveTransfer(true, user, path);
}

void Museeq::removeUploads(const QList<QPair<QString, QString> >& uploads) {
	QList<QPair<QString, QString> >::const_iterator it = uploads.begin();
	for(; it != uploads.end(); ++it)
		removeUpload((*it).first, (*it).second);
}

void Museeq::abortDownload(const QString& user, const QString& path) {
	mDriver->doAbortTransfer(false, user, path);
}

void Museeq::abortDownloads(const QList<QPair<QString, QString> >& downloads) {
	QList<QPair<QString, QString> >::const_iterator it = downloads.begin();
	for(; it != downloads.end(); ++it)
		abortDownload((*it).first, (*it).second);
}

void Museeq::abortUpload(const QString& user, const QString& path) {
	mDriver->doAbortTransfer(true, user, path);
}

void Museeq::abortUploads(const QList<QPair<QString, QString> >& uploads) {
	QList<QPair<QString, QString> >::const_iterator it = uploads.begin();
	for(; it != uploads.end(); ++it)
		abortUpload((*it).first, (*it).second);
}

void Museeq::slotTransferRemoved(bool isUpload, const QString& user, const QString& path) {
	if(isUpload)
		emit uploadRemoved(user, path);
	else
		emit downloadRemoved(user, path);
}

void Museeq::receivedPrivRoomAlterableMembers(const QString& room, const QStringList& members) {
    mPrivRoomAlterableMembers[room] = members;
}

void Museeq::receivedPrivRoomAlterableOperators(const QString& room, const QStringList& operators) {
    mPrivRoomAlterableOperators[room] = operators;
}

void Museeq::privRoomAddedUser(const QString& room, const QString& user) {
    mPrivRoomAlterableMembers[room].push_back(user);
}

void Museeq::privRoomRemovedUser(const QString& room, const QString& user) {
    int index = mPrivRoomAlterableMembers[room].indexOf(user);
    if ((index < 0) || index >= mPrivRoomAlterableMembers[room].size()) // Didn't find
        return;

    mPrivRoomAlterableMembers[room].removeAt(index);
}

void Museeq::privRoomAddedOperator(const QString& room, const QString& user) {
    mPrivRoomAlterableOperators[room].push_back(user);
}

void Museeq::privRoomRemovedOperator(const QString& room, const QString& user) {
    int index = mPrivRoomAlterableOperators[room].indexOf(user);
    if ((index < 0) || index >= mPrivRoomAlterableOperators[room].size()) // Didn't find
        return;

    mPrivRoomAlterableOperators[room].removeAt(index);
}

void Museeq::setConfig(const QString& domain, const QString& key, const QString& value) {
	mDriver->setConfig(domain, key, value);
}

void Museeq::setNewPassword(const QString& newPass) {
    mDriver->setNewPassword(newPass);
}

void Museeq::removeConfig(const QString& domain, const QString& key) {
	mDriver->removeConfig(domain, key);
}

const QString& Museeq::config(const QString& domain, const QString& key) {
	static const QString null;
	QMap<QString, QMap<QString, QString> >::const_iterator dom_it = mConfig.find(domain);
	if(dom_it == mConfig.end())
		return null;

	QMap<QString, QString>::const_iterator key_it = dom_it.value().find(key);
	if(key_it == dom_it.value().end())
		return null;

	return key_it.value();
}
void Museeq::connectServer() {
	mDriver->doConnectServer();
}
void Museeq::disconnectServer() {
	mDriver->doDisconnectServer();
}
void Museeq::reloadShares() {
	mDriver->doReloadShares();
}
void Museeq::saveSettings() {
	mMainWin->saveSettings();
}
void Museeq::showURL(const QUrl& rawUrl) {
	QString protocol = rawUrl.scheme();
	if(! mProtocolHandlers.contains(protocol) and protocol != "slsk") {
		QMessageBox::warning(mMainWin, "Error", QString("No protocol handler defined for protocol %1").arg(protocol));
		return;
	}
	if (protocol == "slsk") {
		QString user = rawUrl.userName();
        if (user.isEmpty())
            user = rawUrl.host();
        if (user.isEmpty())
            return;

		QString path = rawUrl.path();
		if (!path.isEmpty()) {
            uint size  = 0;
            path.replace("/", "\\");
            mDriver->doDownloadFile(user, path, size);
		}
		else
            museeq->mainwin()->showUserInfo(user);
		return;
    }

	QString handler = mProtocolHandlers[protocol];
	handler.replace("$", QString(rawUrl.toEncoded()));
	QStringList handlerargs = handler.split(" ", QString::KeepEmptyParts);
	QString executable (handlerargs.value(0));
	handlerargs.removeAt(0);
	QProcess p;
	p.startDetached(executable, handlerargs);
}

void Museeq::setProtocolHandlers(const QMap<QString, QString>& h) {
	mProtocolHandlers = h;

	QStringList old = mSettings->value("ProtocolHandlers").toStringList();

	mSettings->beginGroup("ProtocolHandlers");

	QStringList::const_iterator it, end = old.end();
	for(it = old.begin(); it != end; ++it)
		if(! h.contains(*it))
			mSettings->remove(*it);

	QMap<QString, QString>::ConstIterator hit, hend = h.end();
	for(hit = h.begin(); hit != hend; ++hit)
		mSettings->setValue(hit.key(), hit.value());

	mSettings->endGroup();
}

void Museeq::slotStatusSet(uint status) {
	mAway = status;
}
void Museeq::setAway(bool away) {
	mDriver->setStatus(away);
}

void Museeq::setTicker(const QString& room, const QString& message) {
	mDriver->setTicker(room, message);
}

void Museeq::flush() {
	if(mApplication->hasPendingEvents())
		mApplication->processEvents();
}

int Museeq::scriptCallbackId() {
#ifdef HAVE_QTSCRIPT
    mCallbackCount++;
    return mCallbackCount;
#endif // HAVE_QTSCRIPT

    return 0;
}

bool Museeq::hasScript(const QString& name) {
#ifdef HAVE_QTSCRIPT
    QList<Script*>::const_iterator it = mScripts.begin();
    for (; it != mScripts.end(); it++) {
        if (name == (*it)->scriptName())
            return true;
    }
#endif // HAVE_QTSCRIPT

    return false;
}

void Museeq::loadScript(const QString& script)
{
#ifdef HAVE_QTSCRIPT
    QString code;
    if (script.isEmpty())
        return;

    QFile f(script);
    if(f.open(QIODevice::ReadOnly))
    {
        code = f.readAll();
        f.close();
    }

    // Where is this script?
    QDir path(script);
    path.cdUp();

    Script * newScript = new Script(code, path.absolutePath());

    // Register known menus to this new script
    QMap<QString, QMenu*>::const_iterator it;
    for (it = mMenus.begin(); it != mMenus.end(); it++) {
        newScript->registerMenu(it.key(), (*it));
    }

    // Store this script
    mScripts.push_back(newScript);

	newScript->init();

	mMainWin->addScript(newScript->scriptName());
#endif // HAVE_QTSCRIPT
}

void Museeq::unloadScript(const QString& scriptName)
{
#ifdef HAVE_QTSCRIPT
    QList<Script*>::iterator it;

    for (it = mScripts.begin(); it != mScripts.end();) {
        if((*it)->scriptName() == scriptName) {
            mMainWin->removeScript((*it)->scriptName());

            delete (*it);
            mScripts.erase(it++);
        }
        else
            ++it;
    }
#endif // HAVE_QTSCRIPT
}

void Museeq::registerMenu(const QString& title, QMenu* menu) {
#ifdef HAVE_QTSCRIPT
    QList<Script*>::iterator it;

    for (it = mScripts.begin(); it != mScripts.end(); it++) {
        (*it)->registerMenu(title, menu);
    }

    mMenus[title] = menu;
#endif // HAVE_QTSCRIPT
}

QString Museeq::handleInput(bool privateMessage, const QString& target, QString line) {
#ifdef HAVE_QTSCRIPT
    QList<Script*>::iterator it;

    for (it = mScripts.begin(); it != mScripts.end(); it++) {
        line = (*it)->handleInput(privateMessage, target, line);
    }
#endif // HAVE_QTSCRIPT
	return line;
}

void Museeq::trayicon_hide() {
	if (trayicon())
		trayicon()->hide();
}

void Museeq::trayicon_show() {
	if (trayicon())
		trayicon()->show();
}

void Museeq::trayicon_setIcon(const QString& icon) {
	if (trayicon())
		trayicon()->setIcon(IMG(icon));
}

void Museeq::trayicon_load() {
	if (mUsetray && settings()->value("showTrayIcon", true).toBool())
		mTray->show();

	mainwin()->setTrayIconInitState();

	mTrayMenu->show();
}

void Museeq::output(const QString& message) {
	std::cout << message.toStdString() << std::endl;
	mMainWin->appendToLogWindow(message);
}
Museeq* museeq = 0;

int main(int argc, char **argv) {
	QApplication app(argc, argv);

    // translation file for application strings
	QTranslator muTranslation(0);
	QString language, muLocalePath, muTransFile;
	language = QLocale::system().name();
    muLocalePath = QString(DATADIR) + QString("/museek/museeq/translations/");

	// mid to fix \ shorten long locales like from "fr_FR.utf8" to "fr_FR"
	muTransFile = QString("museeq_") + language.mid(0,5) + QString(".qm");
	QFileInfo fi( muLocalePath + muTransFile );
	if ( !fi.exists() ) {
		// if longer locale isn't found, try two-character locale such as "fr"
		muTransFile = QString("museeq_") + language.mid(0,2) + QString(".qm");
		QFileInfo fi( muLocalePath + muTransFile );
		if ( fi.exists() ) {
			muTranslation.load( muTransFile, muLocalePath );
            app.installTranslator( &muTranslation );
		}
	} else {
		muTranslation.load( muTransFile, muLocalePath );
        app.installTranslator( &muTranslation );
	}

	QTranslator qtTranslation(0);
    QString qtLocalePath = QString(DATADIR) + QString("/qt4/translations/");
    QString qtTransFile = QString( "qt_%1.qm").arg( language );

	QFileInfo fiqt( qtLocalePath + qtTransFile );
	if ( !fiqt.exists() ) {
        // try fallback
        qtTransFile = QString( "qt_%1.qm").arg( language.toLower().left(2) );
        QFileInfo fiqt( qtLocalePath + qtTransFile );
		if ( fiqt.exists() ) {
            qtTranslation.load( qtTransFile, qtLocalePath );
            app.installTranslator( &qtTranslation );
		}
	}
	else {
        qtTranslation.load( qtTransFile, qtLocalePath );
        app.installTranslator( &qtTranslation );
	}

	new Museeq(&app);

	std::string usetray = string("yes");
	std::string version = string("museeq ") + museeq->mainwin()->mVersion.toStdString();

	for(int i = 1; i < argc; i++) {
		string arg = argv[i];

		if(arg == "--version" || arg == "-V" ) {
			std::cout << version << std::endl << std::endl;
			return 0;
		}
		else if(arg == "--no-tray" )
			usetray = string("no");
		else if(arg == "--help" || arg == "-h") {
			std::cout << version << std::endl;
			std::cout << QObject::tr("Syntax: museeq [options]").toStdString() << std::endl << std::endl;
			std::cout << QObject::tr("Options:").toStdString() << std::endl;
			std::cout << QObject::tr("-V --version\t\tDisplay museeq version and quit").toStdString() << std::endl << std::endl;
			std::cout << QObject::tr("-h --help\t\tDisplay this message and quit").toStdString() << std::endl;
			std::cout << QObject::tr("--no-tray\t\tDon't load TrayIcon").toStdString() << std::endl;
			std::cout << std::endl;
			return 0;
		}

	}

	if (usetray == "yes")
		museeq->mUsetray = true;

	museeq->trayicon_load();
	app.setQuitOnLastWindowClosed( false );
	museeq->mainwin()->showWithRestoredSize();
	museeq->mainwin()->connectToMuseek(true);

	return app.exec();

}

void Museeq::privRoomListReceived(const NPrivRoomList& list) {
    m_privRoomListCache = list;
    emit privRoomList(list);
}

NPrivRoomList Museeq::getPrivRoomList() {
    return m_privRoomListCache;
}

bool Museeq::isRoomOperated(const QString& room) {
    return (m_privRoomListCache.contains(room) && (m_privRoomListCache[room].second >= 1));
}

bool Museeq::isRoomOwned(const QString& room) {
    return (m_privRoomListCache.contains(room) && (m_privRoomListCache[room].second == 2));
}

bool Museeq::isRoomPrivate(const QString& room) {
    return (m_privRoomListCache.contains(room));
}

bool Museeq::canAddAsMember(const QString& room, const QString& user) { // A room where we're op without this user or if user == ourself
    bool ourSelf = (user == nickname());
    return (isRoomOperated(room) && !ourSelf && !mPrivRoomAlterableMembers[room].contains(user));
}

bool Museeq::canDismember(const QString& room, const QString& user) { // A room where we're op with this user as simple member (not op) or with this user != ourself
    bool ourSelf = (user == nickname());
    return (!ourSelf && isRoomOperated(room) && mPrivRoomAlterableMembers[room].contains(user) && !mPrivRoomAlterableOperators[room].contains(user));
}

bool Museeq::canAddAsOperator(const QString& room, const QString& user) { // A room where we're op with this user as simple member (not op)
    bool ourSelf = (user == nickname());
    return (!ourSelf && isRoomOwned(room) && mPrivRoomAlterableMembers[room].contains(user) && !mPrivRoomAlterableOperators[room].contains(user));
}

bool Museeq::canDisop(const QString& room, const QString& user) { // A room where we're op with this user as op or with this user == ourself
    bool ourSelf = (user == nickname());
    return (!ourSelf && isRoomOperated(room) && mPrivRoomAlterableOperators[room].contains(user));
}
