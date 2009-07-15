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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QtNetwork/QTcpSocket>

class QListWidgetItem;
class IconListBox;
class QTextEdit;
class QStackedWidget;
class QLabel;
class QTreeWidgetItem;
class QMenu;
class QResizeEvent;
class QCloseEvent;
class QMoveEvent;
class QSplitter;

class IPDialog;
class SettingsDialog;
class ChatRooms;
class PrivateChats;
class Transfers;
class Searches;
class UserInfos;
class Browsers;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0, const char* name = "");

	const QPoint & lastPos() const { return mLastPos; }
	const QSize & lastSize() const { return mLastSize; }
	ChatRooms* chatRooms() { return mChatRooms;}
	bool askPassword(QString & pass);
	bool isSavingTransfersLayout() const {return mSaveTransfersLayout;}
	bool isSavingAllLayouts() const {return mSaveAllLayouts;}

	QMenu* mMenuFile, *  mMenuSettings, * mMenuModes;
	QString  mVersion;
	QTextEdit *mLog;

	QAction * ActionConnect, * ActionDisconnect, * ActionAway, * ActionCheckPrivileges, * ActionBrowseMyShares, * ActionExit;
	QAction * ActionLoadScript;
	QAction * ActionSettings, * ActionToggleAutoConnect;
	QAction * ActionChatRooms, * ActionPrivateChat, * ActionTransfers, * ActionSearch, * ActionUserInfo, * ActionBrowseShares;
	QAction * ActionAbout, * ActionCommands, * ActionHelp;
	bool mVerticalIconBox;
public slots:
	void changeCMode();
	void changePMode();
	void changeSMode();
	void changeTMode();
	void changeBMode();
	void changeUMode();
	void changeMode(uint);
	void connectToMuseek(bool autoConnectAsked = false);
	void connectToMuseekPS(const QString&, const QString&);
	void saveConnectConfig();
	void readSettings();
	void saveSettings();
	void doDaemon();
	void stopDaemon();
	void toggleAway();
	void setTrayIconInitState();
	void checkPrivileges();
	void getOwnShares();
	void toggleTickers();
	void toggleLog();
	void toggleVisibility();

	void showIPDialog();
	void showIPDialog(const QString&);

	void displayAboutDialog();
	void displayAboutQt();
	void displayHelpDialog();
	void displayCommandsDialog();

	void changeColors();

	void changeSettings();
	void changeTheme();

	void givePrivileges(const QString&);

	void doPrivRoomAddUser(const QString& room, const QString& user);
	void doPrivRoomRemoveUser(const QString& room, const QString& user);
	void doPrivRoomDismember(const QString& room);
	void doPrivRoomDisown(const QString& room);
	void doPrivRoomAddOperator(const QString& room, const QString& user);
	void doPrivRoomRemoveOperator(const QString& room, const QString& user);

	void startSearch(const QString&);
	void showPrivateChat(const QString&);
	void showUserInfo(const QString&);
	void showBrowser(const QString&);

	void addScript(const QString&);
	void removeScript(const QString&);
	void appendToLogWindow(const QString&);

	void slotClose();

	void showWithRestoredSize();

signals:
	void showAllTickers();
	void hideAllTickers();
	void toggleCountries(bool);
	void closingMuseeq();

private slots:
	void loadScript();
	void unloadScript(QAction*);

	void slotError(QAbstractSocket::SocketError);
	void slotConnected();
	void slotHostFound();
	void slotDisconnected();
	void slotLoggedIn(bool, const QString&);
	void slotStatusMessage(bool, const QString&);
	void slotUserStatus( const QString&, uint );
	void slotConnectedToServer(bool);
	void slotStatusSet(uint status);
	void slotConfigChanged(const QString&, const QString&, const QString&);
	void slotUserAddress(const QString& user, const QString& ip, uint port);
	void slotPrivilegesLeft(uint);

protected slots:
	void changePage(QListWidgetItem*, QListWidgetItem*);

protected:
	void moveEvent(QMoveEvent *);
	void resizeEvent(QResizeEvent *);
	void closeEvent(QCloseEvent *);

private:
	bool mWaitingPrivs;
	bool mCloseFromMenu;
	IconListBox* mIcons;
	QStackedWidget* mStack;
	QSplitter *mSplit;
	QLabel* mTitle;
	QLabel* statusLabel, * messageLabel;
	QMenu* mMenuScripts, * mMenuUnloadScripts,  * mMenuHelp;
	IPDialog* mIPDialog;

	SettingsDialog* mSettingsDialog;

	ChatRooms* mChatRooms;
	PrivateChats* mPrivateChats;
	Transfers* mTransfers;
	Searches* mSearches;
	UserInfos* mUserInfos;
	Browsers* mBrowsers;
	int mMoves;
	QPoint mLastPos;
	QSize mLastSize;
	bool mSaveTransfersLayout, mSaveAllLayouts;
};

#endif // MAINWINDOW_H
