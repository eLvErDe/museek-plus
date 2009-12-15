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
#include <QLayout>

#define _TIME QString("<span style='"+museeq->mFontTime+";color:"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</span> "

MainWindow::MainWindow(QWidget* parent, const char* name) : QMainWindow(0, 0), mWaitingPrivs(false) {
	mVersion = "0.3";
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
 	mSaveTransfersLayout = museeq->settings()->value("saveTransfersLayout", false).toBool();
 	mSaveAllLayouts = museeq->settings()->value("saveAllLayouts", false).toBool();

	mMenuFile = menuBar()->addMenu(tr("&File"));
	ActionConnect = new QAction(IMG("connect"), tr("&Connect to daemon"), this);
	ActionConnect->setShortcut(tr("Alt+C"));
	connect(ActionConnect, SIGNAL(triggered()), this, SLOT(connectToMuseek()));
	mMenuFile->addAction(ActionConnect);

	ActionDisconnect = new QAction(IMG("disconnect"), tr("&Disconnect from daemon"), this);
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

	ActionBrowseMyShares = new QAction(IMG("browser-small"), tr("&Browse my shares"), this);
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

	mMenuModes = menuBar()->addMenu(tr("&Modes"));

	ActionChatRooms = new QAction(IMG("chatroom-small"), tr("&Chat rooms"), this);
	connect(ActionChatRooms, SIGNAL(triggered()), this, SLOT(changeCMode()));
	mMenuModes->addAction(ActionChatRooms);

	ActionPrivateChat = new QAction(IMG("privatechat-small"), tr("&Private chat"), this);
	connect(ActionPrivateChat, SIGNAL(triggered()), this, SLOT(changePMode()));
	mMenuModes->addAction(ActionPrivateChat);

	ActionTransfers = new QAction(IMG("transfer-small"), tr("&Transfers"), this);
	connect(ActionTransfers, SIGNAL(triggered()), this, SLOT(changeTMode()));
	mMenuModes->addAction(ActionTransfers);

	ActionSearch = new QAction(IMG("search-small"), tr("&Search"), this);
	connect(ActionSearch, SIGNAL(triggered()), this, SLOT(changeSMode()));
	mMenuModes->addAction(ActionSearch);

	ActionUserInfo = new QAction(IMG("userinfo-small"), tr("&User info"), this);
	connect(ActionUserInfo, SIGNAL(triggered()), this, SLOT(changeUMode()));
	mMenuModes->addAction(ActionUserInfo);

	ActionBrowseShares = new QAction(IMG("browser-small"), tr("&Browse shares"), this);
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

	mIPDialog = new IPDialog(this, "ipDialog");

	mSettingsDialog = new SettingsDialog(this, "settingsDialog");

	QWidget * MainWidget = new QWidget(this);
	setCentralWidget(MainWidget);

	connect( mSettingsDialog->mDisconnectFromDaemonButton, SIGNAL(clicked()), museeq->driver(), SLOT(disconnect()));

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

	mSplit = new QSplitter(MainWidget);
	mSplit->setOrientation(Qt::Vertical);
	header->addWidget(mSplit);

	mStack = new QStackedWidget(mSplit);
	mStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	header->setStretchFactor(mStack, 10);
	mChatRooms = new ChatRooms(MainWidget);

	mStack->addWidget(mChatRooms);
	mPrivateChats = new PrivateChats(MainWidget, "privateChats");

	mStack->addWidget(mPrivateChats);

	mTransfers = new Transfers(MainWidget);
	mStack->addWidget(mTransfers);

	mSearches = new Searches(MainWidget);
	mStack->addWidget(mSearches);

	mUserInfos = new UserInfos(MainWidget, "userInfo");
	mStack->addWidget(mUserInfos);

	mBrowsers = new Browsers(MainWidget, "browser");
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

	mLog = new QTextEdit(mSplit);
	mLog->setReadOnly(true);
	mLog->setAcceptRichText(true);
	mLog->setFocusPolicy(Qt::NoFocus);
	mLog->resize(0, 100);
	if ( ! museeq->settings()->value("showStatusLog", false).toBool())
		mLog->hide();

	mChatRooms->updateTickers();

	// Disable Museekd settings
	mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), false);

	changeCMode();

    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "logSplitter_Layout";
        mSplit->restoreState(museeq->settings()->value(optionName).toByteArray());
    }
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
    if (Util::getMuseekdLock())
        return;

    QString museekConfig = museeq->settings()->value("MuseekConfigFile").toString();
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
		messageLabel->setText(tr("Terminating museek daemon..."));
	} else {
		messageLabel->setText(tr("Museek daemon not running, no need to stop it..."));
	}

}

void MainWindow::readSettings() {
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
	QString server = mSettingsDialog->mDAddress->currentText();
    QString password = mSettingsDialog->mDPassword->text();

    bool savePass = mSettingsDialog->mDSavePassword->isChecked();
    museeq->settings()->setValue("SavePassword", savePass);
	if(savePass)
		museeq->settings()->setValue("Password", password);
	else
		museeq->settings()->remove("Password");

	museeq->settings()->remove("Servers");
	museeq->settings()->beginGroup("Servers");
	int ix = 1;
	for(int i = 0; i < mSettingsDialog->mDAddress->count(); ++i)
	{
		QString s = mSettingsDialog->mDAddress->itemText(i);
		if(s != server)
		{
			museeq->settings()->setValue(QString::number(ix), s);
			++ix;
		}
	}
	museeq->settings()->setValue(QString::number(ix), server);
	museeq->settings()->endGroup();
}

void MainWindow::connectToMuseek(bool autoConnectAsked) {
	ActionConnect->setEnabled(false);

 	if (museeq->settings()->value("LaunchMuseekDaemon").toBool() && !Util::getMuseekdLock()) {
        doDaemon();
        // Wait a bit before trying to connect to the daemon
        QTimer::singleShot(2000, this, SLOT(connectToMuseek()));
        return;
 	}

	if (!autoConnectAsked || museeq->settings()->value("AutoConnect").toBool()) {
		QString server = mSettingsDialog->mDAddress->currentText();
		QString password = mSettingsDialog->mDPassword->text();
		if (museeq->settings()->value("SavePassword").toBool()  && (! password.isEmpty()) )
            connectToMuseekPS(server, password);
		else {
		    QString pass;
		    bool savePass = askPassword(pass);
		    if (!pass.isEmpty()) {
		        connectToMuseekPS(server, pass);
		        if (savePass) {
		            museeq->settings()->setValue("SavePassword", true);
                    museeq->settings()->setValue("Password", pass);
                    mSettingsDialog->loadSettings();
		        }
		    }
		}
	}
	else { // We don't have to connect now
		ActionConnect->setEnabled(true);
	}
}

void MainWindow::connectToMuseekPS(const QString& server, const QString& password) {
	ActionDisconnect->setEnabled(true);
	if(mSettingsDialog->mDConnectType->currentIndex() == 0) {
		int ix = server.indexOf(':');
		quint16 port = server.mid(ix+1).toUInt();
		qDebug("Connecting to the daemon... Looking up host");
		messageLabel->setText(tr("Connecting to the daemon... Looking up host"));
		museeq->driver()->connectToHost(server.left(ix), port, password);
	} else {
		qDebug("Connecting to the daemon...");
		messageLabel->setText(tr("Connecting to the daemon..."));
		museeq->driver()->connectToUnix(server, password);
	}
}

void MainWindow::slotHostFound() {
	qDebug("Connecting to the daemon... Connecting");
	messageLabel->setText(tr("Connecting to the daemon... Connecting"));
}

void MainWindow::slotConnected() {
	qDebug("Connecting to the daemon... Logging in");
	messageLabel->setText(tr("Connecting to the daemon... Logging in"));
}

void MainWindow::slotDisconnected() {
	centralWidget()->setEnabled(false);
	messageLabel->setText(tr("Disconnected from the daemon"));
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
		messageLabel->setText(tr("Cannot connect to the daemon... Connection refused"));
		break;
	case QAbstractSocket::HostNotFoundError:
		messageLabel->setText(tr("Cannot connect to the daemon... Host not found"));
		break;
	default:
		messageLabel->setText(tr("socket error"));
		break;
	}
	statusLabel->setText(tr("Status:")+" "+tr("Disconnected"));
	qDebug() << "socket error: " << e;
	ActionConnect->setEnabled(true);
	ActionDisconnect->setEnabled(false);
	ActionBrowseMyShares->setEnabled(false);
}


void MainWindow::slotLoggedIn(bool success, const QString& msg) {
	if(success) {
		messageLabel->setText(tr("Logged in to the daemon"));
		statusLabel->setText(tr("Status:")+" "+tr("Connected"));
		centralWidget()->setEnabled(true);
		mSettingsDialog->mTabHolder->setTabEnabled(mSettingsDialog->mTabHolder->indexOf(mSettingsDialog->mMuseekdTabs), true);

		ActionConnect->setEnabled(false);
		ActionDisconnect->setEnabled(true);
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
		museeq->trayicon_setIcon("disconnect");
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

void MainWindow::changeTheme() {
	QString _path = QString(DATADIR) + "/museek/museeq/themes/";
	QDir dir  (_path);
	QFileDialog * fd = new QFileDialog(this, tr("Enter a Museeq icon theme directory"), dir.path());
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
    bool current = museeq->settings()->value("showTickers", true).toBool();
    museeq->settings()->setValue("showTickers", !current);
    mSettingsDialog->loadSettings();
	if (current)
		emit hideAllTickers();
	else
		emit showAllTickers();
}

void MainWindow::toggleLog() {
    bool current = museeq->settings()->value("showStatusLog", false).toBool();
    museeq->settings()->setValue("showStatusLog", !current);
    mSettingsDialog->loadSettings();
	if (current)
		mLog->hide();
	else
		mLog->show();
}

void MainWindow::changeColors() {

	mSettingsDialog->mTabHolder->setCurrentWidget(mSettingsDialog->mMuseeqTabs);
	mSettingsDialog->mMuseeqTabs->setCurrentWidget(mSettingsDialog->mColorsAndFontsTab);
	changeSettings();
}


void MainWindow::saveSettings() {
    saveConnectConfig();

    bool showTimestamps = mSettingsDialog->mToggleTimestamps->isChecked();
    museeq->settings()->setValue("showTimestamps", showTimestamps);
    museeq->mShowTimestamps = showTimestamps;

    bool showTray = mSettingsDialog->mToggleTrayicon->isChecked();
    if (museeq->mUsetray && !showTray)
		museeq->trayicon_hide();
	else if (showTray)
		museeq->trayicon_show();
    if (museeq->mUsetray)
        museeq->settings()->setValue("showTrayIcon", showTray);

    bool enableTickers = mSettingsDialog->mToggleTickers->isChecked();
    museeq->settings()->setValue("showTickers", enableTickers);
	if (!enableTickers)
		emit hideAllTickers();
	else
		emit showAllTickers();

    bool showCountries = mSettingsDialog->mToggleCountry->isChecked();
    museeq->settings()->setValue("showCountries", showCountries);
    emit toggleCountries(showCountries);

    mSaveTransfersLayout = mSettingsDialog->mToggleSaveTransfersLayout->isChecked();
    museeq->settings()->setValue("saveTransfersLayout", mSaveTransfersLayout);

    mSaveAllLayouts = mSettingsDialog->mToggleSaveAllLayouts->isChecked();
    museeq->settings()->setValue("saveAllLayouts", mSaveAllLayouts);

    bool showLog = mSettingsDialog->mToggleLog->isChecked();
    museeq->settings()->setValue("showStatusLog", showLog);
    if (!showLog)
		mLog->hide();
	else
		mLog->show();

    QString previousMuseekConfig = museeq->settings()->value("MuseekConfigFile").toString();
    museeq->settings()->setValue("MuseekConfigFile", mSettingsDialog->mMuseekConfigFile->text());

	museeq->settings()->setValue("LaunchMuseekDaemon", mSettingsDialog->mAutoStartDaemon->isChecked());

	museeq->settings()->setValue("AutoConnect", mSettingsDialog->mDAutoConnect->isChecked());

	museeq->settings()->setValue("ShowExitDialog", !mSettingsDialog->mShowExitDialog->isChecked());

	museeq->settings()->setValue("ShutDownDaemonOnExit", mSettingsDialog->mShutDownDaemonOnExit->isChecked());

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

	if(mSettingsDialog->mAutoClearFinishedDownloads->isChecked())
		museeq->setConfig("transfers", "autoclear_finished_downloads", "true");
	else
        museeq->setConfig("transfers", "autoclear_finished_downloads", "false");

	if(mSettingsDialog->mAutoClearFinishedUploads->isChecked())
		museeq->setConfig("transfers", "autoclear_finished_uploads", "true");
	else
        museeq->setConfig("transfers", "autoclear_finished_uploads", "false");

	if(mSettingsDialog->mAutoRetryDownloads->isChecked())
		museeq->setConfig("transfers", "autoretry_downloads", "true");
	else
        museeq->setConfig("transfers", "autoretry_downloads", "false");

    if (mSettingsDialog->SPrivRoom->isChecked() != mSettingsDialog->getPrivRoomEnabled()) {
        mSettingsDialog->setPrivRoomEnabled(mSettingsDialog->SPrivRoom->isChecked());
        museeq->driver()->doPrivRoomToggle(mSettingsDialog->getPrivRoomEnabled());
    }

    museeq->setConfig("transfers", "download_blacklist", mSettingsDialog->mBlacklistDownload->text());

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


    // If we changed the config file ask to restart the daemon
    if ((previousMuseekConfig != mSettingsDialog->mMuseekConfigFile->text()) && Util::getMuseekdLock()) {
        QMessageBox::StandardButton resp = QMessageBox::question(this, tr("New configuration file"), tr("You have chosen a new configuration file for the daemon. Do you want to restart it now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );
        if (resp == QMessageBox::Yes) {
            museeq->stopDaemon();
            if (mSettingsDialog->mDAutoConnect->isChecked())
                connectToMuseek(true);
        }
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
	QDialog * about = new QDialog(this);
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
	QMessageBox::information(this, tr("Museeq commands"), tr("<h3>While in a chat window, such as a chat room, or a private chat, there are a number of commands available for use.</h3><b>/c /chat</b>   <i>(Switch to chat rooms)</i><br><b>/pm /private</b> &lt;nothing | username&gt;  <i>(Switch to private chat and start chatting with a user, if inputed)</i><br><b>/transfers</b>   <i>(Switch to transfers)</i><br><b>/s /search</b> &lt;nothing | query>   <i>(Switch to searches and start a search with &lt;query&gt; if inputed)</i><br><b>/u /userinfo</b> &lt;username&gt;   <i>(Switch to user info, and attempt to get a user's info, if inputed)</i><br><b>/b /browse</b> &lt;username&gt;    <i>(Switch to browse and initate browsing a user, if inputed)</i><br><b>/ip</b> &lt;username&gt;   <i>(Get the IP of a user)</i><br><b>/log</b>    <i>(Toggle displaying the special message Log)</i><br><b>/t /ticker /tickers</b>   <i>(Toggle the showing of tickers)</i><br><b>/setticker</b> &lt;short text&gt; <i>(Set the inputed text as ticker for the current room)</i> <br><b>/f /fonts /colors</b>   <i>(Open the fonts and colors settings dialog)</i><br><b>/clear</b><i> (Clear the chat history)</i><br><b>/ban /unban</b> &lt;username&gt;   <i>(Disallow/allow a user to receive your shares and download from you)</i><br><b>/ignore /unignore</b> &lt;username&gt;    <i>(Block/unblock chat messages from a user)</i><br><b>/buddy /unbuddy</b> &lt;username&gt;   <i>(Add/remove a user to keep track of it and add comments about it)</i><br><b>/trust /distrust</b> &lt;username&gt;    <i>(Add/remove a user to the optional list of users who can send files to you)</i><br><b>/me</b> <does something>    <i>(Say something in the third-person)</i><br><b>/slap</b> &lt;username&gt;   <i>(Typical trout-slapping)</i><br><b>/j /join</b> &lt;room&gt;    <i>(Join a chat room)</i><br><b>/jp /joinpriv</b> &lt;room&gt;    <i>(Join a private chat room)</i><br><b>/l /p /leave /part</b> &lt;nothing | room&gt;    <i>(Leave the current room or inputed room)</i><br><b>/about /help /commands</b>    <i>(Display information)</i><br><b>/settings</b>    <i>(Display settings dialog)</i><br><br>Do not type the brackets, they are there only to make clear that something (or nothing) can be typed after the /command."));
}

void MainWindow::displayHelpDialog() {
	QMessageBox::information(this, tr("Museeq help"), tr("<h3>What's going on? I can't connect to a Soulseek server with museeq!</h3>You connect to museekd with museeq, so you need to have <b>museekd</b> configured, running <u>and</u> connected to a <b>Soulseek</b> or Soulfind server. <br> <h3>Running for the first time?</h3> Before you start museekd for the first time, you need to configure <b>museekd</b> with <b>musetup</b>,  a command-line configuration script.<br><br> In musetup you <b>MUST</b> configure the following items: Server, Username, Password, Interface Password, Download Dir.<br> Also, take note of your interfaces, if you change them from the default localhost:2240 and /tmp/museek.<tt>USERNAME</tt>, you'll need to know them for logging in with museeq. <br><br> When you start museeq or choose Settings->Configure... from the menu, you are asked to input the host and port, or Unix Socket of museekd, <b>not</b> the server.<br>Once you're connected to museekd, change museekd options via Settings->Museek daemon. <h3>Want to send someone a file?</h3> Browse yourself, select file(s), and right-click->Upload. Input their name in the dialog box, and the upload should start, but it depends on if the user has place you on their \"trusted\" or \"uploads\" users list."));
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

void MainWindow::setTrayIconInitState() {
    if (!museeq->mUsetray) {
        mSettingsDialog->mToggleTrayicon->setChecked(false);
        mSettingsDialog->mToggleTrayicon->setEnabled(false);
    }
    else
        mSettingsDialog->mToggleTrayicon->setChecked(museeq->settings()->value("showTrayIcon", true).toBool());
}

void MainWindow::checkPrivileges() {
	mWaitingPrivs = true;
	museeq->driver()->checkPrivileges();
}

void MainWindow::getOwnShares() {
	showBrowser(museeq->nickname());
}

void MainWindow::doPrivRoomAddUser(const QString& room, const QString& user)
{
    museeq->driver()->doPrivRoomAddUser(room, user);
}

void MainWindow::doPrivRoomRemoveUser(const QString& room, const QString& user)
{
    museeq->driver()->doPrivRoomRemoveUser(room, user);
}

void MainWindow::doPrivRoomDismember(const QString& room)
{
    museeq->driver()->doPrivRoomDismember(room);
}

void MainWindow::doPrivRoomDisown(const QString& room)
{
    museeq->driver()->doPrivRoomDisown(room);
}

void MainWindow::doPrivRoomAddOperator(const QString& room, const QString& user)
{
    museeq->driver()->doPrivRoomAddOperator(room, user);
}

void MainWindow::doPrivRoomRemoveOperator(const QString& room, const QString& user)
{
    museeq->driver()->doPrivRoomRemoveOperator(room, user);
}

void MainWindow::slotPrivilegesLeft(uint seconds) {
	if(mWaitingPrivs) {
		mWaitingPrivs = false;
		QMessageBox::information(this, tr("Soulseek network privileges"), QString(tr("You have %1 days, %2 hours, %3 minutes and %4 seconds of privileges left")).arg(seconds/(24*60*60)).arg((seconds/(60*60)) % 24).arg((seconds / 60) % 60).arg(seconds % 60));
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
    if (museeq->mUsetray && museeq->settings()->value("showTrayIcon", true).toBool() && !mCloseFromMenu) {
        ev->ignore();
        toggleVisibility();
        return;
    }

    bool shutdownDaemon = museeq->settings()->value("ShutDownDaemonOnExit", false).toBool();

    if ( museeq->settings()->value("ShowExitDialog", true).toBool()) {
        bool museekdRunning = Util::getMuseekdLock();
		if (museekdRunning) {
		    QMessageBox::StandardButton resp = QMessageBox::question(this, tr("Close Museeq"), tr("The Museek daemon is still running. Do you want to close it?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel );
			if (resp == QMessageBox::Cancel ) {
                ev->ignore();
				return;
			}
			else
			    shutdownDaemon = (resp == QMessageBox::Yes);
		}
		else {
			if ( QMessageBox::question(this, tr("Close Museeq"), tr("It's safe to close Museeq, but are you sure you want to?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) ==  QMessageBox::No) {
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

    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = "logSplitter_Layout";
        museeq->settings()->setValue(optionName, mSplit->saveState());
    }

	emit closingMuseeq();

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
	QString fn = QFileDialog::getOpenFileName(this, tr("Load script"), "", "*.qs");
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

/**
  * Create a dialog to ask the museekd connection password.
  * Returns true if the given password should be saved.
  * The given password is stored in pass.
  */
bool MainWindow::askPassword(QString & pass) {
    QDialog * askPass = new QDialog (this);
	askPass->setWindowTitle( tr("Connecting to daemon..."));
	QVBoxLayout * vLayout = new QVBoxLayout(askPass);

	QLabel * question = new QLabel(askPass);
	question->setText(tr("Please, insert the museekd password:"));
	vLayout->addWidget(question);

	QLineEdit * password = new QLineEdit( askPass );
	password->setEchoMode(QLineEdit::Password);
	vLayout->addWidget(password);

	QCheckBox * savePassword = new QCheckBox( askPass );
	savePassword->setText( tr( "&Save password" ) );
	vLayout->addWidget(savePassword);

	QHBoxLayout * buttonLayout = new QHBoxLayout;
	vLayout->addLayout(buttonLayout);

	QSpacerItem * spacer5 = new QSpacerItem( 120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonLayout->addItem( spacer5 );

	QPushButton * cancel = new QPushButton( this);
	cancel->setText( tr( "&Cancel" ) );
	QPushButton * ok = new QPushButton( this);
	ok->setText( tr( "&Ok" ) );
	ok->setDefault(true);
	buttonLayout->addWidget( cancel );
	buttonLayout->addWidget( ok );

	connect( ok, SIGNAL( clicked() ), askPass, SLOT( accept() ) );
	connect( cancel, SIGNAL( clicked() ), askPass, SLOT( reject() ) );

	int res = askPass->exec();

	if (res == QDialog::Accepted) {
	    QString p = password->text();
	    if (!p.isEmpty()) {
            pass = p;
	    }
	}

    bool returnValue = savePassword->isChecked();

	delete savePassword;
	delete password;
	delete question;
	delete vLayout;
	delete askPass;

	return returnValue;
}
