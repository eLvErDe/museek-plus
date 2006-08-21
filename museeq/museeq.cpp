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
#include "prefix.h"
#include <system.h>

#include "museekdriver.h"
#include "mainwin.h"
#include "onlinealert.h"
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qinputdialog.h>
#include <qpopupmenu.h>
#include <qtextedit.h>
#include <qdir.h>
#include <qfile.h>
#include "icon.xpm"
#include <qcanvas.h>
#include <qfileinfo.h>
#include "images.h"

#ifdef HAVE_QSA
# include <qmenubar.h>
# include <qsinterpreter.h>
# include <qvaluestack.h>
# ifdef HAVE_QSA_UTIL
#  include <qsutilfactory.h>
# endif // HAVE_QSA_UTIL
# ifdef HAVE_QSA_DIALOG
#  include <qsinputdialogfactory.h>
# endif // HAVE_QSA_DIALOG

# ifdef RELAYED_LIBQSA
extern int libqsa_is_present;
# else
int libqsa_is_present = 1;
# endif

#endif // HAVE_QSA
#include <string>
#include <iostream>
using std::string;
Museeq::Museeq(QApplication * app)
      : QObject(0, "Museeq"), mApplication(app), mConnected(false) {
#ifdef HAVE_QSA
	mCallbackCount = 1000;
#endif // HAVE_QSA
	
	museeq = this;
	mDriver = new MuseekDriver(this, "driver");
	mUsetray = false;
	mShowTickers = true;
	mShowStatusLog = false;
	mFontTime = "font-family:fixed-width";
	mFontMessage =  "";
	mOnlineAlert =  false;
	mColorBanned = mColorBuddied = mColorTime = mColorNickname = mColorTrusted = mColorRemote = mColorMe = "";
	mShowTimestamps = true;
	mIPLog = true;
	connect(mDriver, SIGNAL(connectionClosed()), SLOT(slotConnectionClosed()));
	connect(mDriver, SIGNAL(error(int)), SLOT(slotError(int)));
	connect(mDriver, SIGNAL(loggedIn(bool, const QString&)), SLOT(slotLoggedIn(bool, const QString&)));
	connect(mDriver, SIGNAL(serverState(bool, const QString&)), SLOT(slotServerState(bool, const QString&)));
	connect(mDriver, SIGNAL(statusSet(uint)), SLOT(slotStatusSet(uint)));
	
	connect(mDriver, SIGNAL(roomState(const NRoomList&, const NRooms&, const NTickerMap&)), SLOT(slotRoomState(const NRoomList&, const NRooms&, const NTickerMap&)));
	connect(mDriver, SIGNAL(transferState(const NTransfers&, const NTransfers&)), SLOT(slotTransferState(const NTransfers&, const NTransfers&)));
	connect(mDriver, SIGNAL(transferUpdate(bool, const NTransfer&)), SLOT(slotTransferUpdate(bool, const NTransfer&)));
	connect(mDriver, SIGNAL(configState(const QMap<QString, QMap<QString, QString> >&)), SLOT(slotConfigState(const QMap<QString, QMap<QString, QString> >&)));
	connect(mDriver, SIGNAL(configSet(const QString&, const QString&, const QString&)), SLOT(slotConfigSet(const QString&, const QString&, const QString&)));
	connect(mDriver, SIGNAL(configRemove(const QString&, const QString&)), SLOT(slotConfigRemove(const QString&, const QString&)));
	connect(mDriver, SIGNAL(userStatus(const QString&, uint)), SIGNAL(userStatus(const QString&, uint)));
	connect(mDriver, SIGNAL(userData(const QString&, uint, uint)), SIGNAL(userData(const QString&, uint, uint)));
	connect(mDriver, SIGNAL(joinRoom(const QString&, const NRoom&)), SIGNAL(joinedRoom(const QString&, const NRoom&)));
	connect(mDriver, SIGNAL(joinRoom(const QString&, const NRoom&)), SLOT(slotJoinedRoom(const QString&, const NRoom&)));
	connect(mDriver, SIGNAL(leaveRoom(const QString&)), SIGNAL(leftRoom(const QString&)));
	connect(mDriver, SIGNAL(leaveRoom(const QString&)), SLOT(slotLeftRoom(const QString&)));
	connect(mDriver, SIGNAL(userJoined(const QString&, const QString&, const NUserData&)), SIGNAL(userJoinedRoom(const QString&, const QString&, const NUserData&)));
	connect(mDriver, SIGNAL(userLeft(const QString&, const QString&)), SIGNAL(userLeftRoom(const QString&, const QString&)));

// 	connect(mDriver, SIGNAL(userExists(const QString&)), SIGNAL(slotUserExists(const QString&)));

	connect(mDriver, SIGNAL(sayChatroom(const QString&, const QString&, const QString&)), SIGNAL(saidChatroom(const QString&, const QString&, const QString&)));
	connect(mDriver, SIGNAL(privateMessage(uint, uint, const QString&, const QString&)), SIGNAL(privateMessage(uint, uint, const QString&, const QString&)));
	connect(mDriver, SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)), SIGNAL(searchResults(uint, const QString&, bool, uint, uint, const NFolder&)));
	connect(mDriver, SIGNAL(searchToken(const QString&, uint)), SIGNAL(searchToken(const QString&, uint)));
	connect(mDriver, SIGNAL(userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)), SIGNAL(userInfo(const QString&, const QString&, const QByteArray&, uint, uint, bool)));
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
	


	connect(mDriver, SIGNAL(transferRemove(bool, const QString&, const QString&)), SLOT(slotTransferRemoved(bool, const QString&, const QString&)));

	QSettings settings;

	QString s = settings.readEntry("/TheGraveyard.org/Museeq/IconTheme");
	if (s.isEmpty())
		mIconTheme = "default";
	else
		mIconTheme = s;


	QStringList handlers = settings.entryList("/TheGraveyard.org/Museeq/ProtocolHandlers");
	if(! handlers.empty()) {
		settings.beginGroup("/TheGraveyard.org/Museeq/ProtocolHandlers");
		QStringList::const_iterator it, end = handlers.end();
		for(it = handlers.begin(); it != end; ++it)
			mProtocolHandlers[*it] = settings.readEntry(*it);
	} else
		mProtocolHandlers["http"] = "firefox -a firefox --remote openURL($,new-tab)";

	mMainWin = new MainWindow(0, "mainWin");

	emit disconnectedFromServer();
	emit connectedToServer(false);

#ifdef HAVE_QSA
	if(libqsa_is_present)
	{
		QDir dir = QDir::home();
		if(dir.cd(".museeq")) {
			QStringList scripts = dir.entryList("*.qs");
			QStringList::iterator it = scripts.begin();
			for(; it != scripts.end(); ++it) {
				QString fn = dir.absPath() + QDir::separator() + *it;
				QFile f(fn);
				if(f.open(IO_ReadOnly)) {
					loadScript(f.readAll());
					f.close();
				}
			}
		}
	}
#endif // HAVE_QSA

	menutray = new QPopupMenu();
	menutray->insertItem(QT_TR_NOOP("&Restore"), mMainWin , SLOT( showNormal() ) );
	menutray->insertItem(QT_TR_NOOP("&Hide"),  mMainWin , SLOT( hide()  ) );
	menutray->insertSeparator();
	menutray->insertItem( QT_TR_NOOP("&Quit"),  mMainWin , SLOT( close() ) );
	
	mTray =  new TrayIcon( QPixmap( (char**)icon_xpm),  QT_TR_NOOP("MuseeqTray"), menutray );
	QObject::connect( mTray, SIGNAL(clicked(const QPoint&, int )),  mMainWin, SLOT(toggleVisibility() ) );
	
}

void Museeq::slotConnectionClosed() {
	mBuddies.clear();
	mBanned.clear();
	mIgnored.clear();
	mTrusted.clear();
	
	emit disconnected();
}

void Museeq::slotError(int err) {
	if(err == QSocket::ErrSocketRead)
		slotConnectionClosed();
}

void Museeq::slotUserExists(const QString& user) {
	mDriver->getUserExists(user);
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

void Museeq::slotRoomState(const NRoomList& list, const NRooms& rooms, const NTickerMap& tickers) {
	emit roomList(list);
	
	mJoinedRooms.clear();
	
	NRooms::const_iterator it = rooms.begin();
	for(; it != rooms.end(); ++it) {
		mJoinedRooms << it.key();
		emit joinedRoom(it.key(), it.data());
	}
	
	NTickerMap::const_iterator tit = tickers.begin();
	for(; tit != tickers.end(); ++tit)
		emit roomTickers(tit.key(), tit.data());
}

void Museeq::slotTransferState(const NTransfers& downloads, const NTransfers& uploads) {
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
}

void Museeq::slotTransferUpdate(bool isUpload, const NTransfer& transfer) {
	if(isUpload)
		emit uploadUpdated(transfer);
	else
		emit downloadUpdated(transfer);
}

void Museeq::slotConfigState(const QMap<QString, QMap<QString, QString> >& _config) {
	mConfig = _config;
	
	QMap<QString, QMap<QString, QString> >::const_iterator it = _config.begin();
	for(; it != _config.end(); ++it) {
		QMap<QString, QString>::const_iterator it2 = (*it).begin();
		for(; it2 != (*it).end(); ++it2)
			slotConfigSet(it.key(), it2.key(), *it2);
	}
}

void Museeq::slotConfigSet(const QString& domain, const QString& key, const QString& value) {
	if(domain == "buddies") {
		mBuddies += key;
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
	} else if(domain == "museeq.tickers" && key =="show") {
		if(value == "true" || value == true) {
			mShowTickers = true;
			mMainWin->mMenuSettings->setItemChecked(5, museeq->mShowTickers);
			emit showAllTickers();
			}
		else if(value == "false" || value == false)
			{
			mShowTickers = false;
			mMainWin->mMenuSettings->setItemChecked(5, museeq->mShowTickers);
			emit hideAllTickers();
			}
	} else if(domain == "museeq.statuslog") {
		if (key =="show") {
			if(value == "true" || value == true) {
				mShowStatusLog = true;
				mMainWin->mMenuSettings->setItemChecked(6, museeq->mShowStatusLog);
				mMainWin->mLog->show();
				}
			else if(value == "false" || value == false)
				{
				mShowStatusLog = false;
				mMainWin->mMenuSettings->setItemChecked(6, museeq->mShowStatusLog);
				mMainWin->mLog->hide();
				}
		} else if (key =="ip") {
			if(value == "true" || value == true)
				mIPLog = true;
			else if(value == "false" || value == false)
				mIPLog = false;
		}
	} else if(domain == "museeq.alerts") {
		if (key == "log_window") {
			if(value == "true" || value == true) {
				mOnlineAlert = true;
			} else if(value == "false" || value == false) {
				mOnlineAlert = false;
			}
		}
	} else if(domain == "museeq.text") {
		if (key == "fontTime") {
			mFontTime = value;}
		else if (key == "fontMessage") {
			mFontMessage = value;}
		else if (key == "showTimestamps") {
			if(value == "true" || value == true) {
				mShowTimestamps = true;
				mMainWin->mMenuSettings->setItemChecked(7, true);
			} else if(value == "false" || value == false) {
				mShowTimestamps = false;
				mMainWin->mMenuSettings->setItemChecked(7, false);
			}
		} else if (key == "colorBanned") {
			mColorBanned = value;
		} else if (key == "colorBuddied") {
			mColorBuddied = value;
		} else if (key == "colorTrusted") {
			mColorTrusted = value;
		} else if (key == "colorMe") {
			mColorMe = value;
		} else if (key == "colorNickname") {
			mColorNickname = value;
		} else if (key == "colorRemote") {
			mColorRemote = value;
		} else if (key == "colorTime") {
			mColorTime = value;
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
	}
	mConfig[domain][key] = value;
	emit configChanged(domain, key, value);
}

void Museeq::slotConfigRemove(const QString& domain, const QString& key) {
	if(domain == "buddies") {
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
		mBanned.remove(key);
		emit removedBanned(key);
		emit doUpdateStatus(key);
	} else if(domain == "ignored") {
		mIgnored.remove(key);
		emit removedIgnored(key);
		emit doUpdateStatus(key);
	} else if(domain == "trusted") {
		mTrusted.remove(key);
		emit removedTrusted(key);
		emit doUpdateStatus(key);
	} else if(domain == "autojoin") {
		mAutoJoin.remove(key);
		emit autoJoin(key, false);
	} else if(domain == "interests.like") {
		mLovedInterests.remove(key);
		emit removedInterest(key);
	} else if(domain == "interests.hate") {
		mHatedInterests.remove(key);
		emit removedHatedInterest(key);
	}
}

void Museeq::slotJoinedRoom(const QString& room, const NRoom&) {
	mJoinedRooms << room;
}

void Museeq::slotLeftRoom(const QString& room) {
	mJoinedRooms.remove(room);
}
void Museeq::startDaemon() {
	mMainWin->doDaemon();
}
void Museeq::stopDaemon() {
	mMainWin->stopDaemon();
}
void Museeq::saveConnectConfig() { 
	mMainWin->saveConnectConfig(); 
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
		QString c = QInputDialog::getText("Comments", "Comments for " + user, QLineEdit::Normal, "");
		if (c != "")
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
	mDriver->setConfig("trusted", user, "");
}

void Museeq::removeTrusted(const QString& user) {
	mDriver->removeConfig("trusted", user);
}

void Museeq::joinRoom(const QString& room) {
	mDriver->doJoinRoom(room);
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
#ifdef HAVE_QSA
	_line = handleInput(false, room, _line);
#endif
	if(! _line.isEmpty())
		mDriver->doSayChatroom(room, _line);
}

void Museeq::sayPrivate(const QString& user, const QString& line) {
	QString _line = line;
#ifdef HAVE_QSA
	_line = handleInput(true, user, _line);
#endif
	if(! _line.isEmpty())
		mDriver->doSendPrivateMessage(user, _line);
}

void Museeq::downloadFile(const QString& user, const QString& path, Q_INT64 size) {
	mDriver->doDownloadFile(user, path, size);
}

void Museeq::downloadFileTo(const QString& user, const QString& path, const QString& localpath, Q_INT64 size) {
	mDriver->doDownloadFileTo(user, path, localpath, size);
}

void Museeq::downloadFolder(const QString& user, const QString& path) {
	mDriver->getFolderContents(user, path);
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

void Museeq::getUserShares(const QString& user) {
	mDriver->getUserShares(user);
}

void Museeq::updateTransfer(const QString& user, const QString& path) {
	mDriver->updateTransfer(user, path);
}

void Museeq::removeDownload(const QString& user, const QString& path) {
	mDriver->doRemoveTransfer(false, user, path);
}

void Museeq::removeDownloads(const QValueList<QPair<QString, QString> >& downloads) {
	QValueList<QPair<QString, QString> >::const_iterator it = downloads.begin();
	for(; it != downloads.end(); ++it)
		removeDownload((*it).first, (*it).second);
}

void Museeq::removeUpload(const QString& user, const QString& path) {
	mDriver->doRemoveTransfer(true, user, path);
}

void Museeq::removeUploads(const QValueList<QPair<QString, QString> >& uploads) {
	QValueList<QPair<QString, QString> >::const_iterator it = uploads.begin();
	for(; it != uploads.end(); ++it)
		removeUpload((*it).first, (*it).second);
}

void Museeq::abortDownload(const QString& user, const QString& path) {
	mDriver->doAbortTransfer(false, user, path);
}

void Museeq::abortDownloads(const QValueList<QPair<QString, QString> >& downloads) {
	QValueList<QPair<QString, QString> >::const_iterator it = downloads.begin();
	for(; it != downloads.end(); ++it)
		abortDownload((*it).first, (*it).second);
}

void Museeq::abortUpload(const QString& user, const QString& path) {
	mDriver->doAbortTransfer(true, user, path);
}

void Museeq::abortUploads(const QValueList<QPair<QString, QString> >& uploads) {
	QValueList<QPair<QString, QString> >::const_iterator it = uploads.begin();
	for(; it != uploads.end(); ++it)
		abortUpload((*it).first, (*it).second);
}

void Museeq::slotTransferRemoved(bool isUpload, const QString& user, const QString& path) {
	if(isUpload)
		emit uploadRemoved(user, path);
	else
		emit downloadRemoved(user, path);
}

void Museeq::setConfig(const QString& domain, const QString& key, const QString& value) {
	mDriver->setConfig(domain, key, value);
}

const QString& Museeq::config(const QString& domain, const QString& key) {
	QMap<QString, QMap<QString, QString> >::const_iterator dom_it = mConfig.find(domain);
	if(dom_it == mConfig.end())
		return QString::null;
	
	QMap<QString, QString>::const_iterator key_it = dom_it.data().find(key);
	if(key_it == dom_it.data().end())
		return QString::null;
	
	return key_it.data();
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
void Museeq::showURL(const QString& url) {
	QString protocol = url.left(url.find(':'));
	if(! mProtocolHandlers.contains(protocol) and protocol != "slsk") {
		QMessageBox::warning(mMainWin, "Error", QString("No protocol handler defined for protocol %1").arg(protocol));
		return;
	}
	if (protocol == "slsk") {
		QString rest = url.mid(7);
		uint num = rest.find("/");
		QString user = rest.mid(0, num);
		QString path = rest.mid(num);
		uint size  = 0;
		path.replace("/", "\\");
 		mDriver->doDownloadFile(user, path, size);

		return;
		}
	QString handler = mProtocolHandlers[protocol];
	handler.replace("$", url);
	QProcess p(QStringList::split(" ", handler));
	if(! p.start())
		QMessageBox::warning(mMainWin, "Error", QString("Couldn't launch:\n%1").arg(handler));
}

void Museeq::setProtocolHandlers(const QMap<QString, QString>& h) {
	mProtocolHandlers = h;
	
	QSettings settings;
	QStringList old = settings.entryList("/TheGraveyard.org/Museeq/ProtocolHandlers");
	
	settings.beginGroup("/TheGraveyard.org/Museeq/ProtocolHandlers");
	
	QStringList::const_iterator it, end = old.end();
	for(it = old.begin(); it != end; ++it)
		if(! h.contains(*it))
			settings.removeEntry(*it);
	
	QMap<QString, QString>::ConstIterator hit, hend = h.end();
	for(hit = h.begin(); hit != hend; ++hit)
		settings.writeEntry(hit.key(), hit.data());
	
	settings.endGroup();
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

void Museeq::loadScript(const QString& script)
{
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	QSProject *proj = new QSProject(this);

# ifdef HAVE_QSA_UTIL
	proj->interpreter()->addObjectFactory(new QSUtilFactory);
# endif // HAVE_QSA_UTIL

# ifdef HAVE_QSA_DIALOG
	proj->interpreter()->addObjectFactory(new QSInputDialogFactory);
# endif // HAVE_QSA_DIALOG
	
	proj->addObject(mMainWin);
	proj->interpreter()->evaluate(script, this);
	
	QString scriptname;
	
	if(proj->interpreter()->hasFunction(QString("Museeq.init")))
	{
		mScriptContext.push(proj);
		QSArgument result = proj->interpreter()->call("init", QSArgumentList(), this);
		mScriptContext.pop();
		if(result.type() == QSArgument::Variant)
			scriptname = result.variant().toString();
	}
	
	if(scriptname.isEmpty())
		scriptname = QString("anonymous.%1").arg(mProjects.size());
	
	while(mProjects.find(scriptname) != mProjects.end())
		scriptname += "~";
	
	mProjects[scriptname] = proj;
	mMainWin->addScript(scriptname);
#endif // HAVE_QSA
}

void Museeq::unloadScript(const QString& scriptName)
{
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	QMap<QString, QSProject *>::iterator it = mProjects.find(scriptName);
	if(it == mProjects.end())
		return;
	
	/* Call script destructor */
	if(it.data()->interpreter()->hasFunction(QString("Museeq.destroy")))
	{
		mScriptContext.push(it.data());
		QSArgument result = it.data()->interpreter()->call("destroy", QSArgumentList(), this);
		mScriptContext.pop();
		if(result.type() == QSArgument::Variant && result.variant().toBool())
			return;
	}
	
	/* Erase menu entries and empty menus */
	QValueStack<int> erase;
	QMap<int, QPair<QSProject*, QString> >::iterator it2 = mCallbacks.begin();
	for(; it2 != mCallbacks.end(); ++it2)
		if(it2.data().first == it.data())
			erase.push(it2.key());
	
	while(! erase.isEmpty())
	{
		QValueStack<QString> eraseMenus;
		int id = erase.top();
		
		mCallbacks.erase(id);
		
		QMap<QString, QPopupMenu*>::iterator it3 = mMenus.begin();
		for(; it3 != mMenus.end(); ++it3)
		{
			it3.data()->removeItem(id);
			if(it3.data()->count() == 0)
				eraseMenus.push(it3.key());
		}
		
		while(! eraseMenus.isEmpty())
		{
			it3 = mMenus.find(eraseMenus.top());
			if(it3 != mMenus.end())
			{
				delete it3.data();
				mMenus.erase(it3);
			}
			eraseMenus.pop();
		}
		
		erase.pop();
	}
	
	/* Erase input handlers */
	QValueStack<QPair<QSProject *, QString> > herase;
	QValueList<QPair<QSProject *, QString> >::iterator hit, hend = mInputHandlers.end();
	for(hit = mInputHandlers.begin(); hit != hend; ++hit)
		if((*hit).first == it.data())
			herase.push_back(*hit);
	while(! herase.isEmpty())
		mInputHandlers.erase(mInputHandlers.find(herase.pop()));
	
	/* Destroy the script project */
	delete it.data();
	mProjects.erase(it);
	
	mMainWin->removeScript(scriptName);
#endif // HAVE_QSA
}

void Museeq::registerMenu(const QString& title, QPopupMenu* menu) {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	mMenus[title] = menu;
	connect(menu, SIGNAL(activated(int)), this, SLOT(slotMenuActivated(int)));
#endif // HAVE_QSA
}

void Museeq::addMenu(const QString& menu, const QString& item, const QString& callback) {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	QPopupMenu * m;
	QMap<QString, QPopupMenu*>::iterator it = mMenus.find(menu);
	if(it == mMenus.end()) {
		m = new QPopupMenu();
		mMainWin->menuBar()->insertItem(menu, m);
		registerMenu(menu, m);
	} else
		m = it.data();
	mCallbacks[mCallbackCount] = QPair<QSProject*, QString>(mScriptContext.top(), callback);
	m->insertItem(item, mCallbackCount);
	mCallbackCount++;
#endif // HAVE_QSA
}

void Museeq::addInputHandler(const QString& callback) {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	mInputHandlers.push_back(QPair<QSProject *, QString>(mScriptContext.top(), callback));
#endif // HAVE_QSA
	return;
}

void Museeq::slotMenuActivated(int callback) {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	QMap<int, QPair<QSProject*, QString> >::iterator it = mCallbacks.find(callback);
	if(it != mCallbacks.end())
	{
		mScriptContext.push(it.data().first);
		it.data().first->interpreter()->call(it.data().second, QSArgumentList(), this);
		mScriptContext.pop();
	}
#endif // HAVE_QSA
}

QString Museeq::handleInput(bool privateMessage, const QString& target, const QString& line) {
#ifdef HAVE_QSA
	if(libqsa_is_present)
	{
		QSArgumentList args(QValueList<QVariant>() << QVariant(privateMessage, 0) << QVariant(target) << QVariant(line));
		QValueList<QPair<QSProject*, QString> >::iterator it, end = mInputHandlers.end();
		for(it = mInputHandlers.begin(); it != end; ++it)
		{
			mScriptContext.push((*it).first);
			QSArgument result = (*it).first->interpreter()->call((*it).second, args, this);
			mScriptContext.pop();
			if(result.type() == QSArgument::Variant)
			{
				QString sResult = result.variant().toString();
				if(! sResult.isNull())
					return sResult;
			}
		}
	}
#endif // HAVE_QSA
	return line;
}
void Museeq::trayicon_hide() {
	if (mTray) {
		trayicon()->hide();
		mUsetray = false;
		mMainWin->mMenuFile->setItemChecked(5, mUsetray);
	}
}
void Museeq::trayicon_show() {
	if (mTray) {
		trayicon()->show();
		mUsetray = true;
		mMainWin->mMenuFile->setItemChecked(5, mUsetray);
	}
}

void Museeq::trayicon_setIcon(const QString& icon) {
	if (mTray) {
		trayicon()->setIcon(IMG(icon));
	}
}
void Museeq::trayicon_load() {

// 	QPopupMenu menutray;

	if (mUsetray == true)
		mTray->show();
	menutray->show();
	mMainWin->mMenuFile->setItemChecked(5, mUsetray);
}

Museeq* museeq = 0;

int main(int argc, char **argv) {
	QApplication a(argc, argv);

	
        // translation file for application strings
	QTranslator translation( 0 );
	QString lang, lang2, langpath;
	lang = QString(QTextCodec::locale());
	langpath = lang.mid(0,5); // to fix \ shorten long locales like from "fr_FR.utf8" to "fr_FR"
	lang2 =  (QString(DATADIR) + QString("/museek/museeq/translations/museeq_") + langpath + QString(".qm") );
	QFileInfo fi( lang2 );
	if ( !fi.exists() ) {
		// if longer locale isn't found, try two-character locale such as "fr"
		langpath = lang.mid(0,2);
		lang2 =  (QString(DATADIR) + QString("/museek/museeq/translations/museeq_") + langpath + QString(".qm") );
		QFileInfo fi( lang2 );
		if ( fi.exists() ) {
			translation.load( lang2);
			a.installTranslator( &translation );
		}
	} else {
		translation.load( lang2);
		a.installTranslator( &translation );
	}
	
	
	new Museeq(&a);
	a.setMainWidget(museeq->mainwin());
	
	std::string usetray = string("yes");
	std::string version = string("museeq ") + museeq->mainwin()->mVersion + string( QT_TR_NOOP(" Language: ") )+ lang; 
	
	for(int i = 1; i < argc; i++) {
		string arg = argv[i];

		if(arg == "--version" || arg == "-V" ) {
			std::cout << version << std::endl << std::endl;
			return 0;
		} else if(arg == "--no-tray" ) {
			usetray = string("no");
			
		} else if(arg == "--help" || arg == "-h") {
			std::cout << version << std::endl;
			std::cout << QT_TR_NOOP("Syntax: museeq [options]") << std::endl << std::endl;
			std::cout << QT_TR_NOOP("Options:") << std::endl;
			std::cout << QT_TR_NOOP("-V --version\t\tDisplay museeq version and quit") << std::endl << std::endl; 
			std::cout << QT_TR_NOOP("-h --help\t\tDisplay this message and quit") << std::endl;
			std::cout << QT_TR_NOOP("--no-tray\t\tDon't load TrayIcon") << std::endl;
			std::cout << std::endl;
			return 0;
		}
			
	}
	if (usetray == "yes") {
		museeq->mUsetray = true;
	}
	museeq->trayicon_load();
	museeq->mainwin()->show();
	museeq->mainwin()->connectToMuseek();

	return a.exec();
	
}
