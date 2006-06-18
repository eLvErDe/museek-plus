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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>

class IconListBox;
class QTextEdit;
class QWidgetStack;
class QLabel;
class QComboBox;
class QPopupMenu;
class QListViewItem;
class QProcess;
class ConnectDialog;
class IPDialog;
class SettingsDialog;
class UserInfoDialog;
class ProtocolDialog;
class FontsAndColorsDialog;
class QMessageBox;
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
	QPopupMenu*  mMenuSettings, * mMenuModes;
	QString  mVersion;
	QTextEdit *mLog;
public slots:
	void changeCMode();
	void changePMode();
	void changeSMode();
	void changeTMode();
	void changeBMode();
	void changeUMode();
	void changeMode(uint);
	void connectToMuseek();
	void saveConnectConfig();
	void saveSettings();
	void doDaemon();
	void stopDaemon();
	void daemonExited();
	void readFromStdout();
	void toggleAway();
	void checkPrivileges();
	void getOwnShares();
	void toggleTickers();
	void toggleLog();
	void toggleVisibility();

	void showIPDialog();
	void showIPDialog(const QString&);

	void displayAboutDialog();
	void displayHelpDialog();
	void displayCommandsDialog();

	void changeColors();
	void changeUserInfo();
	void changeSettings();
	void changeTheme();
	void protocolHandlers();
	void givePrivileges(const QString&);
	
	void startSearch(const QString&);
	void showPrivateChat(const QString&);
	void showUserInfo(const QString&);
	void showBrowser(const QString&);
	
	void addScript(const QString&);
	void removeScript(const QString&);
	
private slots:
	void loadScript();
	void unloadScript(int);
	
	void slotError(int);
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
	
	void slotAddressActivated(const QString&);
	void slotAddressChanged(const QString&);
	
	void protocolHandlerMenu(QListViewItem*, const QPoint&, int);
	void ipDialogMenu(QListViewItem*, const QPoint&, int);

protected slots:
	void changePage();
	
protected:
	void moveEvent(QMoveEvent *);
	void resizeEvent(QResizeEvent *);
	void closeEvent(QCloseEvent *);
	
private:
	bool mWaitingPrivs;

	IconListBox* mIcons;
	QWidgetStack* mStack;
	QLabel* mTitle;
	QPopupMenu* mMenuFile, * mMenuScripts, * mMenuUnloadScripts,  * mMenuHelp;
	QString museekConfig;
	QProcess *daemon;
	ConnectDialog* mConnectDialog;
	IPDialog* mIPDialog;
	UserInfoDialog* mUserInfoDialog;
	SettingsDialog* mSettingsDialog;
	FontsAndColorsDialog* mColorsDialog;
	ProtocolDialog* mProtocolDialog;

	ChatRooms* mChatRooms;
	PrivateChats* mPrivateChats;
	Transfers* mTransfers;
	Searches* mSearches;
	UserInfos* mUserInfos;
	Browsers* mBrowsers;
	int mMoves;
	QPoint mLastPos;
	QSize mLastSize;
};

#endif // MAINWINDOW_H
