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

#include "prefix.h"
#include "museeq.h"
#include "mainwin.h"
#include <system.h>

#include "iconlistbox.h"
#include "chatrooms.h"
#include "privatechats.h"
#include "transfers.h"
#include "searches.h"
#include "userinfos.h"
#include "browsers.h"
#include "museekdriver.h"
#include "connect.h"
#include "ipdialog.h"
#include "settingsdialog.h"
#include "images.h"
#include "util.h"

#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QSplitter>
#include <QFrame>
#include <QMoveEvent>
#include <QCloseEvent>
#include <QDateTime>
#include <QSettings>
#include <QTimer>

#define _TIME QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</span> "

MainWindow::MainWindow(QWidget* parent, const char* name) : QMainWindow(0, 0), mWaitingPrivs(false) {
	mVersion = "0.2";
	setWindowTitle(tr("museeq ")+mVersion);
	setWindowIcon(IMG("icon"));
	connect(museeq->driver(), SIGNAL(hostFound()), SLOT(slotHostFound()));
	connect(museeq->driver(), SIGNAL(connected()), SLOT(slotConnected()));
	connect(museeq->driver(), SIGNAL(error(QAbstractSocket::SocketError)), SLOT(slotError(QAbstractSocket::SocketError)));
	connect(museeq->driver(), SIGNAL(loggedIn(bool, const QString&)), SLOT(slotLoggedIn(bool, const QString&)));
	connect(museeq->driver(), SIGNAL(statusMessage(bool, const QString&)), SLOT(slotStatusMessage(bool, const QString&)));
	connect(museeq->driver(), SIGNAL(userAddress(const QString&, const QString&, uint)), SLOT(slotUserAddress(const QString&, const QString&, uint)));
	connect(museeq->driver(), SIGNAL(privilegesLeft(uint)), SLOT(slotPrivilegesLeft(uint)));
	connect(museeq, SIGNAL(disconnected()), SLOT(slotDisconnected()));
	connect(museeq, SIGNAL(connectedToServer(bool)), SLOT(slotConnectedToServer(bool)));
	connect(museeq->driver(), SIGNAL(statusSet(uint)), SLOT(slotStatusSet(uint)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));


    mCloseFromMenu = false;

	mMenuFile = menuBar()->addMenu(tr("&File"));
	ActionConnect = new QAction(IMG("connect"), tr("&Connect..."), this);
	ActionConnect->setShortcut(tr("Alt+C"));
	connect(ActionConnect, SIGNAL(triggered()), this, SLOT(connectToMuseek()));
	mMenuFile->addAction(ActionConnect);

	ActionDisconnect = new QAction(IMG("disconnect"), tr("&Disconnect..."), this);
	ActionDisconnect->setShortcut(tr("Alt+D"));
	connect(ActionDisconnect, SIGNAL(triggered()), museeq->driver(), SLOT(disconnect()));
	mMenuFile->addAction(ActionDisconnect);

	mMenuFile->addSeparator();

	ActionAway = new QAction( tr("Toggle &away"), this);
	ActionAway->setCheckable(true);
	ActionAway->setShortcut(tr("Alt+A"));
	connect(ActionAway, SIGNAL(triggered()), this, SLOT(toggleAway()));
	mMenuFile->addAction(ActionAway);

	ActionCheckPrivileges = new QAction( tr("Check &privileges"), this);
	connect(ActionCheckPrivileges, SIGNAL(triggered()), this, SLOT(checkPrivileges()));
	mMenuFile->addAction(ActionCheckPrivileges);

	ActionBrowseMyShares = new QAction(IMG("browser-small"), tr("&Browse My Shares"), this);
	ActionBrowseMyShares->setShortcut(tr("Alt+B"));
	connect(ActionBrowseMyShares, SIGNAL(triggered()), this, SLOT(getOwnShares()));
	mMenuFile->addAction(ActionBrowseMyShares);

	mMenuFile->addSeparator();

	ActionExit = new QAction(IMG("exit"), tr("E&xit"), this);
	ActionExit->setShortcut(tr("Alt+X"));
	connect(ActionExit, SIGNAL(triggered()), this, SLOT(slotClose()));
	mMenuFile->addAction(ActionExit);

	ActionDisconnect->setEnabled(false);
	ActionAway->setEnabled(false);
	ActionCheckPrivileges->setEnabled(false);
	ActionBrowseMyShares->setEnabled(false);

	mMenuSettings = menuBar()->addMenu(tr("&Settings"));


	ActionSettings = new QAction(IMG("settings"),tr("Confi&gure..."), this);
	ActionSettings->setShortcut(tr("Alt+G"));
	connect(ActionSettings, SIGNAL(triggered()), this, SLOT(changeSettings()));
	mMenuSettings->addAction(ActionSettings);

	mMenuSettings->addSeparator();

	ActionIconTheme = new QAction(tr("Pick &Icon Theme... (Requires Restart)"), this);
	connect(ActionIconTheme, SIGNAL(triggered()), this, SLOT(changeTheme()));
	mMenuSettings->addAction(ActionIconTheme);

	ActionToggleTickers = new QAction(tr("Show &Tickers"), this);
	ActionToggleTickers->setCheckable(true);
	connect(ActionToggleTickers, SIGNAL(triggered()), this, SLOT(toggleTickers()));
	mMenuSettings->addAction(ActionToggleTickers);

	ActionToggleLog = new QAction(tr("Show &Log"), this);
	ActionToggleLog->setCheckable(true);
	connect(ActionToggleLog, SIGNAL(triggered()), this, SLOT(toggleLog()));
	mMenuSettings->addAction(ActionToggleLog);

	ActionToggleTimestamps = new QAction(tr("Show T&imestamps"), this);
	ActionToggleTimestamps->setCheckable(true);
	connect(ActionToggleTimestamps, SIGNAL(triggered()), this, SLOT(toggleTimestamps()));
	mMenuSettings->addAction(ActionToggleTimestamps);

	ActionToggleAutoConnect = new QAction(tr("Auto-Connect to Daemon"), this);
	ActionToggleAutoConnect->setCheckable(true);
	connect(ActionToggleAutoConnect, SIGNAL(triggered()), this, SLOT(toggleAutoConnect()));
	mMenuSettings->addAction(ActionToggleAutoConnect);

	ActionToggleExitDialog = new QAction(tr("Show Exit Dialog"), this);
	ActionToggleExitDialog->setCheckable(true);
	connect(ActionToggleExitDialog, SIGNAL(triggered()), this, SLOT(toggleExitDialog()));
	mMenuSettings->addAction(ActionToggleExitDialog);
	ActionToggleTrayicon = new QAction(tr("Enable &Trayicon"), this);
	ActionToggleTrayicon->setCheckable(true);
	ActionToggleTrayicon->setShortcut(tr("Alt+T"));
	connect(ActionToggleTrayicon, SIGNAL(triggered()), this, SLOT(toggleTrayicon()));
	mMenuSettings->addAction(ActionToggleTrayicon);
	mMenuSettings->addSeparator();
	ActionIconTheme->setEnabled(true);

	museeq->mShowTickers = museeq->settings()->value("showTickers").toBool();
	museeq->mShowStatusLog = museeq->settings()->value("showStatusLog").toBool();

 	ActionToggleTickers->setChecked(museeq->mShowTickers);
 	ActionToggleLog->setChecked(museeq->mShowStatusLog);
 	ActionToggleTimestamps->setChecked(museeq->mShowTimestamps);

	mMenuModes = menuBar()->addMenu(tr("&Modes"));

	ActionChatRooms = new QAction(IMG("chatroom-small"), tr("&Chat Rooms"), this);
	connect(ActionChatRooms, SIGNAL(triggered()), this, SLOT(changeCMode()));
	mMenuModes->addAction(ActionChatRooms);

	ActionPrivateChat = new QAction(IMG("privatechat-small"), tr("&Private Chat"), this);
	connect(ActionPrivateChat, SIGNAL(triggered()), this, SLOT(changePMode()));
	mMenuModes->addAction(ActionPrivateChat);

	ActionTransfers = new QAction(IMG("transfer-small"), tr("&Transfers"), this);
	connect(ActionTransfers, SIGNAL(triggered()), this, SLOT(changeTMode()));
	mMenuModes->addAction(ActionTransfers);

	ActionSearch = new QAction(IMG("search-small"), tr("&Search"), this);
	connect(ActionSearch, SIGNAL(triggered()), this, SLOT(changeSMode()));
	mMenuModes->addAction(ActionSearch);

	ActionUserInfo = new QAction(IMG("userinfo-small"), tr("&User Info"), this);
	connect(ActionUserInfo, SIGNAL(triggered()), this, SLOT(changeUMode()));
	mMenuModes->addAction(ActionUserInfo);

	ActionBrowseShares = new QAction(IMG("browser-small"), tr("&Browse Shares"), this);
	connect(ActionBrowseShares, SIGNAL(triggered()), this, SLOT(changeBMode()));
	mMenuModes->addAction(ActionBrowseShares);

	mMenuHelp = menuBar()->addMenu(tr("&Help"));

	ActionCommands = new QAction(IMG("help"), tr("&Commands..."), this);
	connect(ActionCommands, SIGNAL(triggered()), this, SLOT(displayCommandsDialog()));
	mMenuHelp->addAction(ActionCommands);

	ActionHelp = new QAction(IMG("help"), tr("&Help..."), this);
	connect(ActionHelp, SIGNAL(triggered()), this, SLOT(displayHelpDialog()));
	mMenuHelp->addAction(ActionHelp);

	mMenuHelp->addSeparator();

	QAction * ActionAboutQt = new QAction(IMG("help"), tr("About Qt"), this);
	connect(ActionAboutQt, SIGNAL(triggered()), this, SLOT(displayAboutQt()));
	mMenuHelp->addAction(ActionAboutQt);

	ActionAbout = new QAction(IMG("help"), tr("&About Museeq"), this);
	connect(ActionAbout, SIGNAL(triggered()), this, SLOT(displayAboutDialog()));
	mMenuHelp->addAction(ActionAbout);

#ifdef HAVE_QTSCRIPT
    mMenuScripts = menuBar()->addMenu(tr("Sc&ripts"));

    ActionLoadScript = new QAction(tr("&Load script..."), this);
	connect(ActionLoadScript, SIGNAL(triggered()), this, SLOT(loadScript()));

    mMenuScripts->addAction(ActionLoadScript);
	mMenuUnloadScripts = mMenuScripts->addMenu(tr("&Unload script"));

    mMenuScripts->addSeparator();

    museeq->registerMenu("File", mMenuFile);
    museeq->registerMenu("Settings", mMenuSettings);
    museeq->registerMenu("Modes", mMenuModes);
    museeq->registerMenu("Scripts", mMenuScripts);
    museeq->registerMenu("Help", mMenuHelp);
#endif // HAVE_QTSCRIPT

	statusLabel = new QLabel(this);
	messageLabel = new QLabel(this);
	statusBar()->addWidget(messageLabel, 10);
	statusBar()->addWidget(statusLabel, 0);
	messageLabel->setText(tr("Welcome to Museeq"));
	statusLabel->setText(tr("Status:")+" "+tr("Disconnected"));
	mConnectDialog = new ConnectDialog(this, "connectDialog");

#ifdef HAVE_SYS_UN_H
	connect(mConnectDialog->mAddress, SIGNAL(activated(const QString&)), SLOT(slotAddressActivated(const QString&)));
	connect(mConnectDialog->mAddress, SIGNAL(textChanged(const QString&)), SLOT(slotAddressChanged(const QString&)));
#else
	mConnectDialog->mUnix->setDisabled(true);
#endif
	mIPDialog = new IPDialog(this, "ipDialog");

	mSettingsDialog = new SettingsDialog(this, "settingsDialog");
	QWidget * MainWidget = new QWidget(this);
	setCentralWidget(MainWidget);

	QVBoxLayout *box = new QVBoxLayout(MainWidget);

	box->setSpacing(5);
	box->setMargin(0);
	QHBoxLayout *Hbox = new QHBoxLayout;
	box->addLayout(Hbox);

	readSettings();

	QVBoxLayout* vbox = new QVBoxLayout;
	QVBoxLayout* header = new QVBoxLayout;
	if (mVerticalIconBox) {
		Hbox->addLayout(vbox);
		Hbox->addLayout(header);
	} else
		box->addLayout(header);

	vbox->setSpacing(5);
	vbox->setMargin(0);
	header->setSpacing(5);
	header->setMargin(0);


	mIcons = new IconListBox(MainWidget, "", mVerticalIconBox);
	if (mVerticalIconBox)
		vbox->addWidget(mIcons);
	else
		header->addWidget(mIcons);

	IconListItem* chatIcon = new IconListItem(mIcons, IMG("chatroom"), tr("Chat rooms"));

	IconListItem* privatechatIcon = new IconListItem(mIcons, IMG("privatechat"), tr("Private chat"));


	IconListItem* transferIcon = new IconListItem(mIcons, IMG("transfer"), tr("Transfers"));
	transferIcon->setCanDrop(true);
	transferIcon->setDropNeedPath(true);


	IconListItem* searchIcon = new IconListItem(mIcons, IMG("search"), tr("Search"));


	IconListItem* infoIcon = new IconListItem(mIcons, IMG("userinfo"), tr("User info"));


	IconListItem* browserIcon = new IconListItem(mIcons, IMG("browser"), tr("Browse"));


	mIcons->updateMinimumWidth();
	mIcons->updateMinimumHeight();


	mIcons->setCurrentRow(0);

	mTitle = new QLabel(MainWidget);
	header->addWidget(mTitle);
	QFont f = mTitle->font();
	f.setBold(true);
	mTitle->setFont(f);

	QFrame* frame = new QFrame(MainWidget);
	header->addWidget(frame);
	frame->setFrameShape(QFrame::HLine);

	QSplitter *split = new QSplitter(MainWidget);
	split->setOrientation(Qt::Vertical);
	header->addWidget(split);

	mStack = new QStackedWidget(split);
	mStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	header->setStretchFactor(mStack, 10);
	mChatRooms = new ChatRooms(MainWidget);

	mStack->addWidget(mChatRooms);
	mPrivateChats = new PrivateChats(MainWidget);

	mStack->addWidget(mPrivateChats);

	mTransfers = new Transfers(MainWidget);
	mStack->addWidget(mTransfers);

	mSearches = new Searches(MainWidget);
	mStack->addWidget(mSearches);

	mUserInfos = new UserInfos(MainWidget, "userInfo");
	mStack->addWidget(mUserInfos);

	mBrowsers = new Browsers(MainWidget);
	mStack->addWidget(mBrowsers);

	connect(mChatRooms, SIGNAL(highlight(int)), chatIcon, SLOT(setHighlight(int)));
	connect(museeq, SIGNAL(connectedToServer(bool)), privatechatIcon, SLOT(setCanDrop(bool)));
	connect(privatechatIcon, SIGNAL(dropSlsk(const QList<QUrl>&)), mPrivateChats, SLOT(dropSlsk(const QList<QUrl>&)));
	connect(mPrivateChats, SIGNAL(highlight(int)), privatechatIcon, SLOT(setHighlight(int)));

	connect(transferIcon, SIGNAL(dropSlsk(const QList<QUrl>&)), mTransfers, SLOT(dropSlsk(const QList<QUrl>&)));
	connect(mSearches, SIGNAL(highlight(int)), searchIcon, SLOT(setHighlight(int)));
	connect(museeq, SIGNAL(connectedToServer(bool)), infoIcon, SLOT(setCanDrop(bool)));
	connect(infoIcon, SIGNAL(dropSlsk(const QList<QUrl>&)), mUserInfos, SLOT(dropSlsk(const QList<QUrl>&)));
	connect(mUserInfos, SIGNAL(highlight(int)), infoIcon, SLOT(setHighlight(int)));
	connect(museeq, SIGNAL(connectedToServer(bool)), browserIcon, SLOT(setCanDrop(bool)));
	connect(browserIcon, SIGNAL(dropSlsk(const QList<QUrl>&)), mBrowsers, SLOT(dropSlsk(const QList<QUrl>&)));
	connect(mBrowsers, SIGNAL(highlight(int)), browserIcon, SLOT(setHighlight(int)));
	QObject::connect(mIcons, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));

	connect(museeq->driver(), SIGNAL(userStatus(const QString&, uint)), SLOT(slotUserStatus(const QString&, uint)));
	mLog = new QTextEdit(split);

	mLog->setReadOnly(true);
	mLog->setAcceptRichText(true);
	mLog->setFocusPolicy(Qt::NoFocus);
	mLog->resize(0, 100);
	if ( ! museeq->mShowStatusLog)
		mLog->hide();

	mChatRooms->updateTickers();

	// Disable Museekd settings
	mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), false);

	changeCMode();
}

void MainWindow::showWithRestoredSize() {
	bool max = museeq->settings()->value("Maximized", false).toBool();

	if (max)
        showMaximized();
    else
        show();
}

void MainWindow::toggleVisibility() {
	if ( museeq->mainwin()->isVisible() )
		museeq->mainwin()->hide();
	else
		museeq->mainwin()->show();
}
void MainWindow::changeCMode() {
	uint page =0;
	changeMode(page);
}
void MainWindow::changePMode() {
	uint page =1;
	changeMode(page);
}
void MainWindow::changeTMode() {
	uint page =2;
	changeMode(page);
}
void MainWindow::changeSMode() {
	uint page =3;
	changeMode(page);
}
void MainWindow::changeUMode() {
	uint page =4;
	changeMode(page);
}
void MainWindow::changeBMode() {
	uint page =5;
	changeMode(page);
}
void MainWindow::changeMode(uint page) {
	mIcons->setCurrentRow(page);
	mTitle->setText(mIcons->currentItem()->text());
	mStack->setCurrentIndex(page);
}
void MainWindow::changePage(QListWidgetItem* current, QListWidgetItem* last) {
	mTitle->setText(mIcons->currentItem()->text());
	mStack->setCurrentIndex(mIcons->row(mIcons->currentItem()));
}

void MainWindow::doDaemon() {
    museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    QStringList arguments;
    if (! museekConfig.isEmpty() ) {
        arguments.append("--config");
        arguments.append(museekConfig);
    }

    QProcess::startDetached("museekd", arguments);
}

void MainWindow::stopDaemon() {
	if (Util::getMuseekdLock()) {
		QProcess::startDetached("killall museekd");
		messageLabel->setText(tr("Terminating Museek Daemon..."));
	} else {
		messageLabel->setText(tr("Museek Daemon not running, no need to stop it..."));
	}

}

void MainWindow::readSettings() {
	if (museeq->settings()->value("ShowExitDialog", true).toBool()) {
		museeq->settings()->setValue("ShowExitDialog", true);
		ActionToggleExitDialog->setChecked(true);
	}
	mMoves = 0;
	int w = museeq->settings()->value("Width", 600).toInt();
	int h = museeq->settings()->value("Height", -1).toInt();
	resize(w, h);

	if(museeq->settings()->contains("Y") and museeq->settings()->contains("X"))
	{
		int x = museeq->settings()->value("X").toInt();
		int y = museeq->settings()->value("Y").toInt();
		move(x, y);
	}
	if (! museeq->settings()->contains("VerticalIconBox"))
		museeq->settings()->setValue("VerticalIconBox", true);
	mVerticalIconBox = museeq->settings()->value("VerticalIconBox").toBool();
	mSettingsDialog->IconsAlignment->setChecked(mVerticalIconBox);

	museeq->mFontMessage = museeq->settings()->value("fontMessage").toString();
	mSettingsDialog->SMessageFont->setText(museeq->mFontMessage);
	museeq->mFontTime = museeq->settings()->value("fontTime").toString();
	mSettingsDialog->STimeFont->setText(museeq->mFontTime);
	museeq->mColorTime = museeq->settings()->value("colorTime", "black").toString();
	if (museeq->mColorTime.isEmpty()) museeq->mColorTime = "black";
	mSettingsDialog->STimeText->setText(museeq->mColorTime);
	museeq->mColorRemote = museeq->settings()->value("colorRemote", "black").toString();
	if (museeq->mColorRemote.isEmpty()) museeq->mColorRemote = "black";
	mSettingsDialog->SRemoteText->setText(museeq->mColorRemote);
	museeq->mColorMe = museeq->settings()->value("colorMe", "red").toString();
	if (museeq->mColorMe.isEmpty()) museeq->mColorMe = "red";
	mSettingsDialog->SMeText->setText(museeq->mColorMe);
	museeq->mColorNickname = museeq->settings()->value("colorNickname", "blue").toString();
	if (museeq->mColorNickname.isEmpty()) museeq->mColorNickname = "blue";
	mSettingsDialog->SNicknameText->setText(museeq->mColorNickname);
	museeq->mColorBuddied = museeq->settings()->value("colorBuddied", "black").toString();
	if (museeq->mColorBuddied.isEmpty()) museeq->mColorBuddied = "black";
	mSettingsDialog->SBuddiedText->setText(museeq->mColorBuddied);
	museeq->mColorBanned = museeq->settings()->value("colorBanned", "black").toString();
	if (museeq->mColorBanned.isEmpty()) museeq->mColorBanned = "black";
	mSettingsDialog->SBannedText->setText(museeq->mColorBanned);
	museeq->mColorTrusted = museeq->settings()->value("colorTrusted", "black").toString();
	if (museeq->mColorTrusted.isEmpty()) museeq->mColorTrusted = "black";
	mSettingsDialog->STrustedText->setText(museeq->mColorTrusted);

	// Private logging
	museeq->mRoomLogDir = museeq->settings()->value("RoomLogDir").toString();
	museeq->mPrivateLogDir = museeq->settings()->value("PrivateLogDir").toString();
	museeq->mLogRooms = museeq->settings()->value("LogRoomChat").toBool();
	museeq->mLogPrivate = museeq->settings()->value("LogPrivateChat").toBool();

	mSettingsDialog->LoggingRoomDir->setText(museeq->mRoomLogDir);
	mSettingsDialog->LoggingPrivateDir->setText(museeq->mPrivateLogDir);
	mSettingsDialog->LoggingPrivate->setChecked(museeq->mLogPrivate);
	mSettingsDialog->LoggingRooms->setChecked(museeq->mLogRooms);
	QDir dir;
	if (! museeq->mRoomLogDir.isEmpty() && ! dir.exists(museeq->mRoomLogDir))
		dir.mkpath(museeq->mRoomLogDir);
	if (! museeq->mPrivateLogDir.isEmpty() && ! dir.exists(museeq->mPrivateLogDir))
		dir.mkpath(museeq->mPrivateLogDir);

	museeq->mOnlineAlert = museeq->settings()->value("showAlertsInLog").toBool();
	museeq->mIPLog = museeq->settings()->value("showIPinLog").toBool();

	mSettingsDialog->SOnlineAlerts->setChecked(museeq->mOnlineAlert);
	mSettingsDialog->SIPLog->setChecked(museeq->mIPLog);


	museeq->mTickerLength = museeq->settings()->value("TickerLength").toUInt();
	if (museeq->mTickerLength == 0)
		museeq->mTickerLength = 50;
	if (museeq->mTickerLength < 20)
		museeq->mTickerLength = 20;
	mSettingsDialog->TickerLength->setValue(museeq->mTickerLength);

}
void MainWindow::saveConnectConfig() {
	QString server = mConnectDialog->mAddress->currentText(),
		password = mConnectDialog->mPassword->text();

	bool autoStart = mConnectDialog->mAutoStartDaemon->isChecked();
	museeq->settings()->setValue("LaunchMuseekDaemon", autoStart);

    bool autoConnect = mConnectDialog->mAutoConnect->isChecked();
	museeq->settings()->setValue("AutoConnect", autoConnect);

    ActionToggleAutoConnect->setChecked(autoConnect);

	if ( ! mConnectDialog->mMuseekConfig->text().isEmpty() )
		museeq->settings()->setValue("MuseekConfigFile", mConnectDialog->mMuseekConfig->text() );
	else
		museeq->settings()->remove("MuseekConfigFile");

    bool savePass = mConnectDialog->mSavePassword->isChecked();
    museeq->settings()->setValue("SavePassword", savePass);
	if(savePass)
		museeq->settings()->setValue("Password", password);
	else
		museeq->settings()->remove("Password");

    bool shutDown = mConnectDialog->mShutDownDaemonOnExit->isChecked();
	museeq->settings()->setValue("ShutDownDaemonOnExit", shutDown);

	museeq->settings()->remove("Servers");
	museeq->settings()->beginGroup("Servers");
	int ix = 1;
	for(int i = 0; i < mConnectDialog->mAddress->count(); ++i)
	{
		QString s = mConnectDialog->mAddress->itemText(i);
		if(s != server)
		{
			museeq->settings()->setValue(QString::number(ix), s);
			++ix;
		}
	}
	museeq->settings()->setValue(QString::number(ix), server);
	museeq->settings()->endGroup();

}

void MainWindow::connectToMuseek() {
	ActionConnect->setEnabled(false);

	mConnectDialog->mAddress->clear();
	QString museekConfig;
	QString password;

	bool savePassword = museeq->settings()->value("SavePassword").toBool();

 	if (savePassword)
	{
		mConnectDialog->mSavePassword->setChecked(true);
		password = museeq->settings()->value("Password").toString();
		if ( !  password.isEmpty())
			mConnectDialog->mPassword->setText(password);
	} else  {
		mConnectDialog->mSavePassword->setChecked(false);
	}
	museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
	if (! museekConfig.isEmpty())
		mConnectDialog->mMuseekConfig->setText(museekConfig);

	bool ShutDownDaemonOnExit = museeq->settings()->value("ShutDownDaemonOnExit").toBool();
	mConnectDialog->mShutDownDaemonOnExit->setChecked(ShutDownDaemonOnExit);

	bool launchMuseekDaemon = museeq->settings()->value("LaunchMuseekDaemon").toBool();

 	if (launchMuseekDaemon)	 {
		mConnectDialog->mAutoStartDaemon->setChecked(true);
		mConnectDialog->mMuseekConfig->show();
        doDaemon();
	} else  {
		mConnectDialog->mAutoStartDaemon->setChecked(false);
	}
	museeq->settings()->beginGroup("Servers");
	QStringList s_keys = museeq->settings()->childKeys();
	museeq->settings()->endGroup();
// 	appendToLogWindow(
	QString cServer;
	if(! s_keys.isEmpty()) {
		for(QStringList::Iterator it = s_keys.begin(); it != s_keys.end(); ++it)
		{
			cServer = museeq->settings()->value("Servers/" + (*it)).toString();
			mConnectDialog->mAddress->addItem(cServer);
		}
		mConnectDialog->mPassword->setFocus();
	} else {
		cServer = "localhost:2240";
		mConnectDialog->mAddress->addItem(cServer);
		mConnectDialog->mAddress->setFocus();
#ifdef HAVE_SYS_UN_H
# ifdef HAVE_PWD_H
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			mConnectDialog->mAddress->addItem(QString("/tmp/museekd.") + QString(pw->pw_name));
# endif
#endif
	}
	mConnectDialog->mAddress->setCurrentIndex(mConnectDialog->mAddress->count() - 1);
	slotAddressActivated(mConnectDialog->mAddress->currentText());
	// Display Connect Dialog
	bool autoConnect = museeq->settings()->value("AutoConnect").toBool();
	if (autoConnect) {
		mConnectDialog->mAutoConnect->setChecked(true);
		ActionToggleAutoConnect->setChecked(true);
		if (savePassword  and (! password.isEmpty()) ) {
            autoConnectServer = cServer;
            autoConnectPassword = password;
			QTimer::singleShot(2000, this, SLOT(doAutoConnect()));
			return;
		}
	} else {
		mConnectDialog->mAutoConnect->setChecked(false);
		ActionToggleAutoConnect->setChecked(false);
	}

	if(mConnectDialog->exec() == QDialog::Accepted) {
		QString server = mConnectDialog->mAddress->currentText(),
			password = mConnectDialog->mPassword->text();
		saveConnectConfig();
		connectToMuseekPS(server, password);
	} else {
		ActionConnect->setEnabled(true);
	}
}

void MainWindow::doAutoConnect() {
    if (!autoConnectServer.isEmpty()) {
        connectToMuseekPS(autoConnectServer, autoConnectPassword);
    }
}

void MainWindow::connectToMuseekPS(const QString& server, const QString& password) {
	ActionDisconnect->setEnabled(true);
	if(mConnectDialog->mTCP->isChecked()) {
		int ix = server.indexOf(':');
		quint16 port = server.mid(ix+1).toUInt();
		qDebug("Connecting to museek... Looking up host");
		messageLabel->setText(tr("Connecting to museek... Looking up host"));
		museeq->driver()->connectToHost(server.left(ix), port, password);
	} else {
		qDebug("Connecting to museek...");
		messageLabel->setText(tr("Connecting to museek..."));
		museeq->driver()->connectToUnix(server, password);
	}
}
void MainWindow::slotHostFound() {
	qDebug("Connecting to museek... Connecting");
	messageLabel->setText(tr("Connecting to museek... Connecting"));
}

void MainWindow::slotConnected() {
	qDebug("Connecting to museek... Logging in");
	messageLabel->setText(tr("Connecting to museek... Logging in"));
}

void MainWindow::slotDisconnected() {
	centralWidget()->setEnabled(false);
	messageLabel->setText(tr("Disconnected from museek"));
	statusLabel->setText(tr("Status:")+" "+tr("Disconnected"));
	ActionConnect->setEnabled(true);
	ActionDisconnect->setEnabled(false);
	ActionAway->setEnabled(false);
	ActionCheckPrivileges->setEnabled(false);
	ActionBrowseMyShares->setEnabled(false);
	mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), false);
	museeq->trayicon_setIcon("disconnect");
}

void MainWindow::slotError(QAbstractSocket::SocketError e) {
	switch(e) {
	case QAbstractSocket::ConnectionRefusedError:
		messageLabel->setText(tr("Cannot connect to museek... Connection refused"));
		break;
	case QAbstractSocket::HostNotFoundError:
		messageLabel->setText(tr("Cannot connect to museek... Host not found"));
		break;
	default:
		messageLabel->setText(tr("socket error"));
		break;
	}
	statusLabel->setText(tr("Status:")+" "+tr("Disconnected"));
	qDebug() << "socket error: " << e;
	doNotAutoConnect();
	ActionConnect->setEnabled(true);
	ActionDisconnect->setEnabled(false);
	ActionBrowseMyShares->setEnabled(false);
}


void MainWindow::slotLoggedIn(bool success, const QString& msg) {
	if(success) {
		messageLabel->setText(tr("Logged in to museek"));
		statusLabel->setText(tr("Status:")+" "+tr("Connected"));
		centralWidget()->setEnabled(true);
		mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), true);

		ActionConnect->setEnabled(false);
		ActionDisconnect->setEnabled(true);
		ActionToggleTickers->setChecked(museeq->mShowTickers);
		ActionToggleLog->setChecked(museeq->mShowStatusLog);
		ActionToggleTimestamps->setChecked(museeq->mShowTimestamps);
		ActionBrowseMyShares->setEnabled(true);
	} else {
		mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), false);
		messageLabel->setText(tr("Login error: ") + msg);
		statusLabel->setText(tr("Status:")+" "+tr("Disconnected"));
		ActionConnect->setEnabled(true);
		ActionDisconnect->setEnabled(false);
		ActionAway->setEnabled(false);
		ActionCheckPrivileges->setEnabled(false);
		ActionBrowseMyShares->setEnabled(false);
		doNotAutoConnect();
		museeq->trayicon_setIcon("disconnect");
	}
}
void MainWindow::doNotAutoConnect() {
	if(mConnectDialog->mAutoConnect->isChecked()) {
		museeq->settings()->setValue("AutoConnect", false);
		mConnectDialog->mAutoConnect->setChecked(false);
		ActionToggleAutoConnect->setChecked(false);
	}
}

void MainWindow::slotStatusMessage(bool type, const QString& msg) {
	appendToLogWindow(msg);
}

void MainWindow::appendToLogWindow(const QString& msg) {
	QString Message = msg;
	QStringList wm = msg.split("\n", QString::KeepEmptyParts);
	QStringList::iterator it = wm.begin();
	for(; it != wm.end(); ++it) {
		if (museeq->mShowTimestamps)
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+Qt::escape(*it)+"</span>"));
		else
			mLog->append(QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+Qt::escape(*it)+"</span>"));
	}
}
void MainWindow::slotUserStatus( const QString & user, uint status ) {
 	if (museeq->mOnlineAlert  && museeq->hasAlert(user)) {
		QString s = (status == 0) ? "offline" : ((status == 1) ? "away" : "online");
		mLog->append(QString(_TIME)+QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>user %2 is now %3</span>").arg(Qt::escape(user)).arg(s)) ;

	}
}
void MainWindow::slotStatusSet(uint status) {
	if (status) {
		statusLabel->setText(tr("Status: ")+tr("Away"));
		ActionAway->setChecked(true);
		museeq->trayicon_setIcon("away");
	} else { // Starting from 157, the server doesn't send us our status when switching from away to online. see IFaceManager.
		statusLabel->setText(tr("Status: ")+tr("Online"));
		ActionAway->setChecked(false);
		museeq->trayicon_setIcon("online");
	}
}
void MainWindow::slotConnectedToServer(bool connected) {
	if(connected) {
		messageLabel->setText(tr("Connected to soulseek, your nickname: ") + museeq->nickname());
		ActionAway->setEnabled(true);
		ActionCheckPrivileges->setEnabled(true);

		museeq->trayicon_setIcon("connect");
	} else {
		messageLabel->setText(tr("Disconnected from soulseek"));
		statusLabel->setText(tr("Status:")+" "+tr("Offline"));
		ActionAway->setEnabled(false);
		ActionCheckPrivileges->setEnabled(false);

	}
}

void MainWindow::showIPDialog() {
	mIPDialog->show();
}

void MainWindow::showIPDialog(const QString& user) {
    QList<QTreeWidgetItem *> items = mIPDialog->mIPListView->findItems(user, 0);
	QTreeWidgetItem *item;
	if(!items.empty()) {
	    item = items.at(0);
		item->setText(1, tr("waiting"));
		item->setText(2, "");
		item->setText(3, "");
	} else  {
		QStringList args;
		args << user << tr("waiting") << "" << "";
		item = new QTreeWidgetItem(mIPDialog->mIPListView, args);
	}

	museeq->driver()->doGetIPAddress(user);
	if (! museeq->mIPLog) {
		mIPDialog->show();
	}
}

void MainWindow::slotAddressActivated(const QString& server) {
#ifdef HAVE_SYS_UN_H
	if(! server.isEmpty() && server[0] == '/')
		mConnectDialog->mUnix->setChecked(true);
	else
		mConnectDialog->mTCP->setChecked(true);
#endif
}

void MainWindow::slotAddressChanged(const QString& text) {
	if(text.length() == 1)
	{
		if(text[0] == '/')
			mConnectDialog->mUnix->setChecked(true);
		else
			mConnectDialog->mTCP->setChecked(true);
	}
}
void MainWindow::changeTheme() {
	QString _path = QString(DATADIR) + "/museek/museeq/themes";
	QDir dir  (_path);
	QFileDialog * fd = new QFileDialog(this, dir.path());
	fd->setWindowTitle(tr("Enter a Museeq Icon Theme Directory"));
	fd->setFileMode(QFileDialog::DirectoryOnly);

	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty()){
		museeq->mIconTheme = fd->selectedFiles().at(0);
		museeq->settings()->setValue("IconTheme", museeq->mIconTheme);

	}
	delete fd;

}

void MainWindow::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {

}



void MainWindow::startSearch(const QString& query) {
	mSearches->doSearch(query);
	mIcons->setCurrentRow(3);
}

void MainWindow::showPrivateChat(const QString& user) {
	mPrivateChats->setPage(user);
	mIcons->setCurrentRow(1);
}

void MainWindow::showUserInfo(const QString& user) {
	mUserInfos->setPage(user);
	mIcons->setCurrentRow(4);
}

void MainWindow::showBrowser(const QString& user) {
	mBrowsers->setPage(user);
	mIcons->setCurrentRow(5);
}

void MainWindow::slotUserAddress(const QString& user, const QString& ip, uint port) {
    QList<QTreeWidgetItem *> items = mIPDialog->mIPListView->findItems(user, 0);
    if(items.isEmpty()) {
        return;
	}
    QTreeWidgetItem* item = items.at(0);
	if(!item) {
		return;
	}
	QString hostname(ip);
	if(ip == "0.0.0.0") {
		item->setText(1, tr("Offline"));
		item->setText(2, "");
	} else {
		item->setText(1, ip);
		item->setText(2, QString::number(port));
#ifdef HAVE_NETDB_H
		struct hostent *addr = gethostbyname(ip.toAscii());
		if(addr && addr->h_length) {
			struct hostent *addr2 = gethostbyaddr(addr->h_addr_list[0], 4, AF_INET);
			if(addr2 && addr2->h_name) {
				item->setText(3, addr2->h_name);
				hostname = addr2->h_name;
			}
		}
#endif // HAVE_NETDB_H
	}

	if (museeq->mIPLog) {
		if (museeq->mShowTimestamps) {
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("IP of ")+Qt::escape(user)+": "+ ip +" "+ tr("Port:")+" "+QString::number(port)+" ("+Qt::escape(hostname)+")</span>"));
		} else {
			mLog->append(QString("<span style='"+museeq->mFontMessage+";color:"+museeq->mColorRemote+"'>"+tr("IP of ")+Qt::escape(user)+": "+ ip +" "+ tr("Port:")+" "+QString::number(port)+" ("+Qt::escape(hostname)+")</span>"));
		}
	}

}
void MainWindow::toggleTickers() {
	if (museeq->mShowTickers) {
		museeq->settings()->setValue("showTickers", false);
		museeq->mShowTickers = false;
		ActionToggleTickers->setChecked(museeq->mShowTickers);
		emit hideAllTickers();
	}
	else {
		museeq->settings()->setValue("showTickers", true);
		museeq->mShowTickers = true;
		ActionToggleTickers->setChecked(museeq->mShowTickers);
		emit showAllTickers();
	}
}
void MainWindow::toggleTimestamps() {
	if (museeq->mShowTimestamps) {
		museeq->settings()->setValue("showTimestamps", false);
		museeq->mShowTimestamps = false;
	}
	else {
		museeq->settings()->setValue("showTimestamps", true);
		museeq->mShowTimestamps = true;
	}
	ActionToggleTimestamps->setChecked(museeq->mShowTimestamps);
}
void MainWindow::toggleLog() {
	if (museeq->mShowStatusLog) {
		museeq->settings()->setValue("showStatusLog", false);
		museeq->mShowStatusLog = false;
		mLog->hide();
	}
	else {
		museeq->settings()->setValue("showStatusLog", true);
		museeq->mShowStatusLog = true;
		mLog->show();
	}
	ActionToggleLog->setChecked(museeq->mShowStatusLog);
}
void MainWindow::toggleAutoConnect() {
	bool autoConnect = museeq->settings()->value("AutoConnect").toBool();
	if (autoConnect) {
		museeq->settings()->setValue("AutoConnect", false);
		ActionToggleAutoConnect->setChecked(false);
		mConnectDialog->mAutoConnect->setChecked(false);
	} else {
		museeq->settings()->setValue("AutoConnect", true);
		mConnectDialog->mAutoConnect->setChecked(true);
		ActionToggleAutoConnect->setChecked(true);
	}
}
void MainWindow::toggleExitDialog() {
	if(museeq->settings()->value("ShowExitDialog").toBool()) {
		museeq->settings()->setValue("ShowExitDialog", false);
		ActionToggleExitDialog->setChecked(false);
	} else {
		museeq->settings()->setValue("ShowExitDialog", true);
		ActionToggleExitDialog->setChecked(true);
	}
}
void MainWindow::changeColors() {

	mSettingsDialog->mTabHolder->setCurrentWidget(mSettingsDialog->mMuseeqTabs);
	mSettingsDialog->mMuseeqTabs->setCurrentWidget(mSettingsDialog->ColorsAndFontsTab);
	changeSettings();
}


void MainWindow::saveSettings() {
	museeq->mRoomLogDir = mSettingsDialog->LoggingRoomDir->text();
	museeq->settings()->setValue("RoomLogDir", museeq->mRoomLogDir);

	museeq->mPrivateLogDir = mSettingsDialog->LoggingPrivateDir->text();
	museeq->settings()->setValue("PrivateLogDir", museeq->mPrivateLogDir);

	if (mSettingsDialog->LoggingPrivate->isChecked()) {
		museeq->settings()->setValue("LogPrivateChat", true);
		museeq->mLogPrivate = true;
	} else {
		museeq->settings()->setValue("LogPrivateChat", false);
		museeq->mLogPrivate = false;
	}
	if (mSettingsDialog->LoggingRooms->isChecked()) {
		museeq->settings()->setValue("LogRoomChat", true);
		museeq->mLogRooms = true;
	} else {
		museeq->settings()->setValue("LogRoomChat", false);
		museeq->mLogRooms = false;
	}
	if (! mSettingsDialog->SServerHost->text().isEmpty() )
		museeq->setConfig("server", "host", mSettingsDialog->SServerHost->text());
	QVariant p (mSettingsDialog->SServerPort->value());
	museeq->setConfig("server", "port", p.toString());
	if (! mSettingsDialog->SSoulseekUsername->text().isEmpty() )
		museeq->setConfig("server", "username", mSettingsDialog->SSoulseekUsername->text());
	if (! mSettingsDialog->SSoulseekPassword->text().isEmpty() )
		museeq->setConfig("server", "password", mSettingsDialog->SSoulseekPassword->text());
	if (! mSettingsDialog->SDownDir->text().isEmpty() )
		museeq->setConfig("transfers", "download-dir", mSettingsDialog->SDownDir->text());
	museeq->setConfig("transfers", "incomplete-dir", mSettingsDialog->SIncompleteDir->text());
	if(mSettingsDialog->SOnlineAlerts->isChecked()) {
		museeq->settings()->setValue("showAlertsInLog", true);
		museeq->mOnlineAlert = true;
	} else {
		museeq->settings()->setValue("showAlertsInLog", false);
		museeq->mOnlineAlert = false;
	}
	mVerticalIconBox = mSettingsDialog->IconsAlignment->isChecked();

	museeq->settings()->setValue("VerticalIconBox", mVerticalIconBox);

	if (mSettingsDialog->SIPLog->isChecked()) {
		museeq->settings()->setValue("showIPinLog", true);
		museeq->mIPLog = true;
	} else {
		museeq->settings()->setValue("showIPinLog", false);
		museeq->mIPLog = false;
	}

	if(mSettingsDialog->SActive->isChecked())
		museeq->setConfig("clients", "connectmode", "active");
	else if (mSettingsDialog->SPassive->isChecked())
		museeq->setConfig("clients", "connectmode", "passive");

	if(mSettingsDialog->SBuddiesShares->isChecked())
		museeq->setConfig("transfers", "have_buddy_shares", "true");
	else
        museeq->setConfig("transfers", "have_buddy_shares", "false");

	if(mSettingsDialog->SShareBuddiesOnly->isChecked())
		museeq->setConfig("transfers", "only_buddies", "true");
	else
        museeq->setConfig("transfers", "only_buddies", "false");

	if(mSettingsDialog->SBuddiesPrivileged->isChecked())
		museeq->setConfig("transfers", "privilege_buddies", "true");
	else
        museeq->setConfig("transfers", "privilege_buddies", "false");

	if(mSettingsDialog->STrustedUsers->isChecked())
		museeq->setConfig("transfers", "trusting_uploads", "true");
	else
        museeq->setConfig("transfers", "trusting_uploads", "false");

	if(mSettingsDialog->SUserWarnings->isChecked())
		museeq->setConfig("transfers", "user_warnings", "true");
	else
        museeq->setConfig("transfers", "user_warnings", "false");

	// listen ports
	QVariant ps (mSettingsDialog->CPortStart->value());
	museeq->setConfig("clients.bind", "first", ps.toString());
	QVariant pe (mSettingsDialog->CPortEnd->value());
	museeq->setConfig("clients.bind", "last", pe.toString());
	// userinfo
	museeq->setConfig("userinfo", "text", mSettingsDialog->mInfoText->toPlainText());
	if(mSettingsDialog->mUpload->isChecked()) {
		QFile f(mSettingsDialog->mImage->text());
		if(f.open(QIODevice::ReadOnly)) {
			QByteArray data = f.readAll();
			f.close();
			museeq->driver()->setUserImage(data);
			mSettingsDialog->mDontTouch->toggle();
		} else
			QMessageBox::warning(this, tr("Error"), tr("Couldn't open image file for reading"));
	} else if(mSettingsDialog->mClear->isChecked()) {
		museeq->driver()->setUserImage(QByteArray());
	}


	museeq->mFontMessage = mSettingsDialog->SMessageFont->text();
	museeq->mFontTime = mSettingsDialog->STimeFont->text();
	museeq->mColorTime = mSettingsDialog->STimeText->text();
	museeq->mColorRemote = mSettingsDialog->SRemoteText->text();
	museeq->mColorMe = mSettingsDialog->SMeText->text();
	museeq->mColorNickname = mSettingsDialog->SNicknameText->text();
	museeq->mColorBuddied = mSettingsDialog->SBuddiedText->text();
	museeq->mColorBanned = mSettingsDialog->SBannedText->text();
	museeq->mColorTrusted = mSettingsDialog->STrustedText->text();

	museeq->settings()->setValue("fontTime", museeq->mFontTime);
	museeq->settings()->setValue("fontMessage", museeq->mFontMessage);
	museeq->settings()->setValue("colorBanned", museeq->mColorBanned);
	museeq->settings()->setValue("colorBuddied", museeq->mColorBuddied);
	museeq->settings()->setValue("colorMe", museeq->mColorMe);
	museeq->settings()->setValue("colorNickname", museeq->mColorNickname);
	museeq->settings()->setValue("colorTrusted", museeq->mColorTrusted);
	museeq->settings()->setValue("colorRemote", museeq->mColorRemote);
	museeq->settings()->setValue("colorTime", museeq->mColorTime);
	// Save protocol Handlers

	QMap<QString, QString> handlers = museeq->protocolHandlers();
	handlers.clear();
	QTreeWidgetItemIterator it(mSettingsDialog->mProtocols);
	while (*it) {
		if (! (*it)->text(0).isEmpty() && ! (*it)->text(1).isEmpty())
			handlers[(*it)->text(0)] = (*it)->text(1);
		++it;
	}

	museeq->setProtocolHandlers(handlers);

	museeq->mTickerLength = mSettingsDialog->TickerLength->value();
	museeq->settings()->setValue("TickerLength", museeq->mTickerLength);
	mChatRooms->updateTickers();

	if (mSettingsDialog->areSharesDirty()) {
	    museeq->reloadShares();
	    mSettingsDialog->setSharesDirty(false);
	}
}


void MainWindow::changeSettings() {
	// Update protocol handlers from live setting
	mSettingsDialog->mProtocols->clear();

	QMap<QString, QString> handlers = museeq->protocolHandlers();
	QMap<QString, QString>::ConstIterator it, end = handlers.end();
	for(it = handlers.begin(); it != end; ++it) {
		QTreeWidgetItem * handler = new QTreeWidgetItem(mSettingsDialog->mProtocols);
		handler->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled );
		handler->setText(0, it.key());
		handler->setText(1, it.value());
	}
	mSettingsDialog->setModal(true);
	mSettingsDialog->show();
}


void MainWindow::displayAboutDialog() {
	QDialog * about = new QDialog;
	about->setWindowTitle( tr("About Museeq"));
	about->setMinimumSize( QSize( 400, 200 ) );
	QVBoxLayout * aboutLayout = new QVBoxLayout(about);
	QLabel * image = new QLabel(about);
	image->setPixmap(IMG("logo"));
	aboutLayout->addWidget(image);

	QHBoxLayout * HLayout = new QHBoxLayout;
	aboutLayout->addLayout(HLayout);

	QLabel * text = new QLabel(about);
	HLayout->addWidget(text);

	text->setText(tr("<p align=\"center\">Museeq ") + mVersion + tr(" is a GUI for the Museek Daemon</p>The programs, museeq and museekd and muscan, were created by Hyriand 2003-2005<br><br>Additions by daelstorm, SeeSchloss and others in 2006-2008<br>This project is released under the GPL license.<br>Code and ideas taken from other opensource projects and people are mentioned in the CREDITS file included in the source tarball."));
	text->setWordWrap(true);
	QHBoxLayout * ButtonLayout = new QHBoxLayout;
	aboutLayout->addLayout(ButtonLayout);
	QSpacerItem * spacer5 = new QSpacerItem( 120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	ButtonLayout->addItem( spacer5 );

	QPushButton * mOK = new QPushButton( this);
	mOK->setText( tr( "&Ok" ) );
	ButtonLayout->addWidget( mOK );
	connect( mOK, SIGNAL( clicked() ), about, SLOT( accept() ) );
	about->exec();

}
void MainWindow::displayAboutQt() {
	QMessageBox::aboutQt(this, tr("About Qt"));
}
void MainWindow::displayCommandsDialog() {
	QMessageBox::information(this, tr("Museeq Commands"), tr("<h3>While in a chat window, such as a Chat Room, or a Private Chat, there are a number of commands available for use.</h3><b>/c /chat</b>   <i>(Switch to Chat Rooms)</i><br><b>/pm /private</b> &lt;nothing | username&gt;  <i>(Switch to Private Chat and start chatting with a user, if inputed)</i><br><b>/transfers</b>   <i>(Switch to Transfers)</i><br><b>/s /search</b> &lt;nothing | query>   <i>(Switch to Searchs and start a Search with &lt;query&gt; if inputed)</i><br><b>/u /userinfo</b> &lt;username&gt;   <i>(Switch to userinfo, and attempt to get a user's info, if inputed)</i><br><b>/b /browse</b> &lt;username&gt;    <i>(Switch to Browse and initate browsing a user, if inputed)</i><br><b>/ip</b> &lt;username&gt;   <i>(Get the IP of a user)</i><br><b>/log</b>    <i>(Toggle displaying the Special Message Log)</i><br><b>/t /ticker /tickers</b>   <i>(Toggle the showing of Tickers)</i> <br><b>/f /fonts /c /colors</b>   <i>(Open the Fonts and Colors settings dialog)</i><br><b>/ban /unban</b> &lt;username&gt;   <i>(Disallow/Allow a user to recieve your shares and download from you)</i><br><b>/ignore /unignore</b> &lt;username&gt;    <i>(Block/Unblock chat messages from a user)</i><br><b>/buddy /unbuddy</b> &lt;username&gt;   <i>(Add/Remove a user to keep track of it and add comments about it)</i><br><b>/trust /distrust</b> &lt;username&gt;    <i>(Add/Remove a user to the optional list of users who can send files to you)</i><br><b>/me</b> <does something>    <i>(Say something in the Third-Person)</i><br><b>/slap</b> &lt;username&gt;   <i>(Typical Trout-slapping)</i><br><b>/j /join</b> &lt;room&gt;    <i>(Join a Chat Room)</i><br><b>/l /p /leave /part</b> &lt;nothing | room&gt;    <i>(Leave the current room or inputed room)</i><br><b>/about /help /commands</b>    <i>(Display information)</i><br><br>Do not type the brackets, they are there only to make clear that something (or nothing) can be typed after the /command."));
}

void MainWindow::displayHelpDialog() {
	QMessageBox::information(this, tr("Museeq Help"), tr("<h3>What's going on? I can't connect to a Soulseek Server with museeq!</h3> You connect to museekd with museeq, so you need to have <b>museekd</b> configured, running <u>and</u> connected to a <b>Soulseek</b> or Soulfind server. <br> <h3>Running for the first time?</h3> Before you start museekd for the first time, you need to configure <b>museekd</b> with <b>musetup</b>,  a command-line configuration script.<br><br> In musetup you <b>MUST</b> configure the following items: Server, Username, Password, Interface Password, Download Dir<br> Also, take note of your interfaces, if you change them from the default localhost:2240 and /tmp/museek.<tt>USERNAME</tt>, you'll need to know them for logging in with museeq. <br><br> When you start museeq or choose File->Connect from the menu, you are asked to input the host and port, or Unix Socket of museekd, <b>not</b> the Server.<br> <h3>Want to send someone a file?</h3> Browse yourself, select file(s), and right-click->Upload. Input their name in the dialog box, and the upload should start, but it depends on if the user has place you on their \"trusted\" or \"uploads\" users list .<br>Once you're connected to museekd, change museekd options via Settings->Museek"));


}

 // added by d ^^
void MainWindow::givePrivileges(const QString& user)
{
	bool ok = false;
	int days = QInputDialog::getInteger(0, tr("Give privileges"),
	             tr("How many days worth of privileges \n") +
	             tr("do you wish to give to user ") + user + "?",
	             0, 0, 999, 1, &ok);
	if(ok && days)
		museeq->driver()->givePrivileges(user, days);
}

void MainWindow::toggleAway() {
	museeq->setAway((museeq->isAway() + 1) & 1);
}

void MainWindow::toggleTrayicon() {
	if (museeq->mUsetray)
		museeq->trayicon_hide();
	else
		museeq->trayicon_show();
}

void MainWindow::checkPrivileges() {
	mWaitingPrivs = true;
	museeq->driver()->checkPrivileges();
}

void MainWindow::getOwnShares() {
	showBrowser(museeq->nickname());
}

void MainWindow::slotPrivilegesLeft(uint seconds) {
	if(mWaitingPrivs) {
		mWaitingPrivs = false;
		QMessageBox::information(this, tr("Museeq - Soulseek Privileges"), QString(tr("You have %1 days, %2 hours, %3 minutes and %4 seconds of privileges left")).arg(seconds/(24*60*60)).arg((seconds/(60*60)) % 24).arg((seconds / 60) % 60).arg(seconds % 60));
	}
}

void MainWindow::moveEvent(QMoveEvent * ev) {
	QMainWindow::moveEvent(ev);
	if(mMoves < 2) {
		mMoves++;
		return;
	}
	mLastPos = pos();
}

void MainWindow::resizeEvent(QResizeEvent * ev) {
	QMainWindow::resizeEvent(ev);
	mLastSize = ev->size();
}

void MainWindow::closeEvent(QCloseEvent * ev) {
    // If we asked to close from the X button (top right), send to the tray.
    if (!mCloseFromMenu) {
        ev->ignore();
        toggleVisibility();
        return;
    }

    bool shutdownDaemon = museeq->settings()->value("ShutDownDaemonOnExit", false).toBool();

    if ( museeq->settings()->value("ShowExitDialog", false).toBool()) {
        bool museekdRunning = Util::getMuseekdLock();
		if (museekdRunning) {
		    QMessageBox::StandardButton resp = QMessageBox::question(this, tr("Shutdown Museeq"), tr("The Museek Daemon is still running. Do you want to close it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel );
			if (resp == QMessageBox::Cancel ) {
                ev->ignore();
				return;
			}
			else
			    shutdownDaemon = (resp == QMessageBox::Yes);
		}
		else {
			if ( QMessageBox::question(this, tr("Shutdown Museeq"), tr("It's safe to close Museeq, but are you sure you want to?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) ==  QMessageBox::No) {
                ev->ignore();
				return;
			}
		}
	}

	museeq->settings()->setValue("X", mLastPos.x());
	museeq->settings()->setValue("Y", mLastPos.y());
	museeq->settings()->setValue("Width", mLastSize.width());
	museeq->settings()->setValue("Height", mLastSize.height());
	museeq->settings()->setValue("Maximized", isMaximized());

    museeq->settings()->sync();

	if ( shutdownDaemon )
		stopDaemon();
	ev->accept();
	QApplication::instance()->quit();
}

void MainWindow::slotClose() {
    mCloseFromMenu = true;
    close();
    mCloseFromMenu = false;
}

void MainWindow::loadScript() {
#ifdef HAVE_QTSCRIPT
	QString fn = QFileDialog::getOpenFileName(this, tr("Load Script"), "", "*.qs");
	if(! fn.isEmpty())
        museeq->loadScript(fn);
#endif // HAVE_QTSCRIPT
}

void MainWindow::unloadScript(QAction* s) {
#ifdef HAVE_QTSCRIPT
    museeq->unloadScript(s->text());
#endif // HAVE_QTSCRIPT
}

void MainWindow::addScript(const QString& scriptname) {
#ifdef HAVE_QTSCRIPT
    mMenuUnloadScripts->addAction(scriptname);
    connect(mMenuUnloadScripts, SIGNAL(triggered(QAction*)), this, SLOT(unloadScript(QAction*)));
#endif // HAVE_QTSCRIPT
}

void MainWindow::removeScript(const QString& scriptname) {
#ifdef HAVE_QTSCRIPT
    QList<QAction*> actions = mMenuUnloadScripts->actions();
    QList<QAction*>::iterator it = actions.begin();

	for(; it != actions.end(); it++) {
		if((*it)->text() == scriptname) {
			mMenuUnloadScripts->removeAction(*it);
			return;
		}
	}
#endif // HAVE_QTSCRIPT
}
