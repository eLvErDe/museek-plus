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

#include <system.h>

#include "mainwin.h"

#include <qprocess.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlistview.h>
#include <qwidgetstack.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qclipboard.h>
#include <qtextedit.h>
#include <qsplitter.h>
#include <iostream>

#include "iconlistbox.h"
#include "chatrooms.h"
#include "privatechats.h"
#include "transfers.h"
#include "searches.h"
#include "transferlistview.h"
#include "userinfos.h"
#include "browsers.h"
#include "museekdriver.h"
#include "connect.h"
#include "ipdialog.h"
#include "userinfodialog.h"
#include "settingsdialog.h"
#include "protocoldialog.h"
#include "fontsandcolorsdialog.h"
#include "prefix.h"
#include "museeq.h"

#include "images.h"

#ifdef HAVE_QSA
extern int libqsa_is_present; // defined in either museeq.cpp or the relay stub
#endif



MainWindow::MainWindow(QWidget* parent, const char* name) : QMainWindow(parent, name), mWaitingPrivs(false) {
	mVersion = "0.1.12";
	setCaption(tr("museeq ")+mVersion);
	setIcon(IMG("icon"));
	connect(museeq->driver(), SIGNAL(hostFound()), SLOT(slotHostFound()));
	connect(museeq->driver(), SIGNAL(connected()), SLOT(slotConnected()));
	connect(museeq->driver(), SIGNAL(error(int)), SLOT(slotError(int)));
	connect(museeq->driver(), SIGNAL(loggedIn(bool, const QString&)), SLOT(slotLoggedIn(bool, const QString&)));
	connect(museeq->driver(), SIGNAL(statusMessage(bool, const QString&)), SLOT(slotStatusMessage(bool, const QString&)));
	connect(museeq->driver(), SIGNAL(userAddress(const QString&, const QString&, uint)), SLOT(slotUserAddress(const QString&, const QString&, uint)));
	connect(museeq->driver(), SIGNAL(privilegesLeft(uint)), SLOT(slotPrivilegesLeft(uint)));
	connect(museeq, SIGNAL(disconnected()), SLOT(slotDisconnected()));
	connect(museeq, SIGNAL(connectedToServer(bool)), SLOT(slotConnectedToServer(bool)));
	connect(museeq->driver(), SIGNAL(statusSet(uint)), SLOT(slotStatusSet(uint)));
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));
	
	mMenuFile = new QPopupMenu(this);
	mMenuFile->insertItem(IMG("connect"), tr("&Connect..."), this, SLOT(connectToMuseek()), ALT + Key_C, 0);
	mMenuFile->insertItem(IMG("disconnect"), tr("&Disconnect"), museeq->driver(), SLOT(disconnect()), ALT + Key_D, 1);
	mMenuFile->insertSeparator();
	mMenuFile->insertItem( tr("Toggle &away"), this, SLOT(toggleAway()), ALT + Key_A, 2);
	mMenuFile->insertItem(tr("Check &privileges"), this, SLOT(checkPrivileges()), 0, 3);
	mMenuFile->insertItem( IMG("browser-small"),tr("&Browse My Shares"), this, SLOT(getOwnShares()), ALT + Key_B, 4); // , 
	mMenuFile->insertSeparator();
	mMenuFile->insertItem(IMG("exit"), tr("E&xit"), this, SLOT(close()), ALT + Key_X);
	mMenuFile->setItemEnabled(1, false);
	mMenuFile->setItemEnabled(2, false);
	mMenuFile->setItemEnabled(3, false);
	mMenuFile->setItemEnabled(4, false);
	menuBar()->insertItem(tr("&File"), mMenuFile);
	
	mMenuSettings = new QPopupMenu(this);
	mMenuSettings->insertItem(IMG("protocol"), tr("&Protocol handlers..."), this, SLOT(protocolHandlers()), 0, 0);
	mMenuSettings->insertSeparator();
	mMenuSettings->insertItem( IMG("userinfo-small"), tr("&User info..."), this, SLOT(changeUserInfo()), 0, 1);
	mMenuSettings->insertSeparator();
	mMenuSettings->insertItem(IMG("settings"),tr("&Museek..."), this, SLOT(changeSettings()), 0, 2);
	mMenuSettings->insertItem( IMG("colors"), tr("&Colors and Fonts..."), this, SLOT(changeColors()), 0, 3);
	mMenuSettings->insertItem(tr("Pick &Icon Theme... (Requires Restart)"), this, SLOT(changeTheme()), 0, 4);
	mMenuSettings->insertItem(tr("Show &Tickers"), this, SLOT(toggleTickers()), 0, 5);
	mMenuSettings->insertItem(tr("Show &Log"), this, SLOT(toggleLog()), 0, 6);
	mMenuSettings->insertItem(tr("Show T&imestamps"), this, SLOT(toggleTimestamps()), 0, 7);
	mMenuSettings->insertItem(tr("Auto-Connect to Daemon"), this, SLOT(toggleAutoConnect()), 0, 8);
	mMenuSettings->insertItem(tr("Show Exit Dialog"), this, SLOT(toggleExitDialog()), 0, 9);
#ifdef HAVE_TRAYICON
	mMenuSettings->insertItem(tr("Enable &Trayicon"), this, SLOT(toggleTrayicon()), ALT + Key_T, 10); // ,
#endif // HAVE_TRAYICON
	mMenuSettings->insertSeparator();
	mMenuSettings->setItemEnabled(1, false);
	mMenuSettings->setItemEnabled(2, false);
	mMenuSettings->setItemEnabled(3, false);

	mMenuSettings->setItemEnabled(5, false);
	mMenuSettings->setItemChecked(5, museeq->mShowTickers);
	mMenuSettings->setItemEnabled(6, false);
	mMenuSettings->setItemChecked(6, museeq->mShowStatusLog);
	mMenuSettings->setItemEnabled(7, false);
	mMenuSettings->setItemChecked(7, museeq->mShowTimestamps);
	mMenuSettings->setItemEnabled(8, true);
	
	menuBar()->insertItem(tr("&Settings"), mMenuSettings);
	mMenuModes = new QPopupMenu(this);
	mMenuModes->insertItem( IMG("chatroom-small"), tr("&Chat Rooms"), this, SLOT(changeCMode()), 0, 0);
	mMenuModes->insertItem( IMG("privatechat-small"), tr("&Private Chat"), this, SLOT(changePMode()), 0, 1);
	mMenuModes->insertItem( IMG("transfer-small"), tr("&Transfers"), this, SLOT(changeTMode()), 0, 2);
	mMenuModes->insertItem( IMG("search-small"), tr("&Search"), this, SLOT(changeSMode()), 0, 3);
	mMenuModes->insertItem( IMG("userinfo-small"), tr("&User Info"), this, SLOT(changeUMode()), 0, 4);
	mMenuModes->insertItem( IMG("browser-small"), tr("&Browse Shares"), this, SLOT(changeBMode()), 0, 5);

	menuBar()->insertItem(tr("&Modes"), mMenuModes);
	
	mMenuHelp = new QPopupMenu(this);
	mMenuHelp->insertItem(IMG("help"), tr("&About..."), this, SLOT(displayAboutDialog()), 0, 0);
	mMenuHelp->insertItem(IMG("help"), tr("&Commands..."), this, SLOT(displayCommandsDialog()), 0, 1);
	mMenuHelp->insertItem(IMG("help"), tr("&Help..."), this, SLOT(displayHelpDialog()), 0, 2);
	

	menuBar()->insertItem(tr("&Help"), mMenuHelp);
#ifdef HAVE_QSA
	if(libqsa_is_present)
	{
		mMenuScripts = new QPopupMenu(this);
		mMenuUnloadScripts = new QPopupMenu(mMenuScripts);
		connect(mMenuUnloadScripts, SIGNAL(activated(int)), SLOT(unloadScript(int)));
		mMenuScripts->insertItem(tr("&Load script..."), this, SLOT(loadScript()));
		mMenuScripts->insertItem(tr("&Unload script"), mMenuUnloadScripts);
		mMenuScripts->insertSeparator();
		menuBar()->insertItem(tr("Sc&ripts"), mMenuScripts, -1 , 3);
		
		museeq->registerMenu("File", mMenuFile);
		museeq->registerMenu("Settings", mMenuSettings);
		museeq->registerMenu("Scripts", mMenuScripts);
		museeq->registerMenu("Help", mMenuHelp);
	}
#endif // HAVE_QSA
	
	statusBar()->message(tr("Welcome to Museeq"));
	
	mConnectDialog = new ConnectDialog(this, "connectDialog");

#ifdef HAVE_SYS_UN_H
	connect(mConnectDialog->mAddress, SIGNAL(activated(const QString&)), SLOT(slotAddressActivated(const QString&)));
	connect(mConnectDialog->mAddress, SIGNAL(textChanged(const QString&)), SLOT(slotAddressChanged(const QString&)));
#else
	mConnectDialog->mUnix->setDisabled(true);
#endif
	mIPDialog = new IPDialog(this, "ipDialog");
	connect(mIPDialog->mIPListView, SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)), SLOT(ipDialogMenu(QListViewItem*, const QPoint&, int)));

	mUserInfoDialog = new UserInfoDialog(this, "userInfoDialog");
	mSettingsDialog = new SettingsDialog(this, "settingsDialog");
	mProtocolDialog = new ProtocolDialog(this, "protocolDialog");
	mColorsDialog = new FontsAndColorsDialog(this, "colorsDialog");
	connect(mProtocolDialog->mProtocols, SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)), SLOT(protocolHandlerMenu(QListViewItem*, const QPoint&, int)));
	
	QHBox *box = new QHBox(this, "centralWidget");
	setCentralWidget(box);
	box->setSpacing(5);
	
	mIcons = new IconListBox(box, "iconListBox");
	
	QVBox* vbox = new QVBox(box, "vbox"),
	     * header = new QVBox(vbox, "header");
	
	vbox->setSpacing(3);
	header->setMargin(2);
	header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	mTitle = new QLabel(header, "title");
	QFont f = mTitle->font();
	f.setBold(true);
	mTitle->setFont(f);
	
	QFrame* frame = new QFrame(header, "line");
	frame->setFrameShape(QFrame::HLine);
	frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	QSplitter *split = new QSplitter(vbox);

	mStack = new QWidgetStack(split, "stack");
	
	mChatRooms = new ChatRooms(mStack, "chatRooms");
	mStack->addWidget(mChatRooms, 0);
	
	mPrivateChats = new PrivateChats(mStack, "privateChats");
	mStack->addWidget(mPrivateChats, 1);
	
	mTransfers = new Transfers(mStack, "transfers");
	mStack->addWidget(mTransfers, 2);
	
	mSearches = new Searches(mStack, "searches");
	mStack->addWidget(mSearches, 3);
	
	mUserInfos = new UserInfos(mStack, "userInfo");
	mStack->addWidget(mUserInfos, 4);
	
	mBrowsers = new Browsers(mStack, "userBrowse");
	mStack->addWidget(mBrowsers, 5);
	
	IconListItem* item = new IconListItem(mIcons, IMG("chatroom"), tr("Chat rooms"));
	connect(mChatRooms, SIGNAL(highlight(int)), item, SLOT(setHighlight(int)));
	
	item = new IconListItem(mIcons, IMG("privatechat"), tr("Private chat"));
	connect(museeq, SIGNAL(connectedToServer(bool)), item, SLOT(setCanDrop(bool)));
	connect(item, SIGNAL(dropSlsk(const QStringList&)), mPrivateChats, SLOT(dropSlsk(const QStringList&)));
	connect(mPrivateChats, SIGNAL(highlight(int)), item, SLOT(setHighlight(int)));
	
	item = new IconListItem(mIcons, IMG("transfer"), tr("Transfers"));
	item->setCanDrop(true);
	connect(item, SIGNAL(dropSlsk(const QStringList&)), mTransfers, SLOT(dropSlsk(const QStringList&)));
	
	item = new IconListItem(mIcons, IMG("search"), tr("Search"));
	connect(mSearches, SIGNAL(highlight(int)), item, SLOT(setHighlight(int)));
	
	item = new IconListItem(mIcons, IMG("userinfo"), tr("User info"));
	connect(museeq, SIGNAL(connectedToServer(bool)), item, SLOT(setCanDrop(bool)));
	connect(item, SIGNAL(dropSlsk(const QStringList&)), mUserInfos, SLOT(dropSlsk(const QStringList&)));
	connect(mUserInfos, SIGNAL(highlight(int)), item, SLOT(setHighlight(int)));
	
	item = new IconListItem(mIcons, IMG("browser"), tr("Browse"));
	connect(museeq, SIGNAL(connectedToServer(bool)), item, SLOT(setCanDrop(bool)));
	connect(item, SIGNAL(dropSlsk(const QStringList&)), mBrowsers, SLOT(dropSlsk(const QStringList&)));
	connect(mBrowsers, SIGNAL(highlight(int)), item, SLOT(setHighlight(int)));

	mIcons->updateWidth();
	mIcons->updateMinimumHeight();
	
	QObject::connect(mIcons, SIGNAL(selectionChanged()), SLOT(changePage()));
	mIcons->setSelected(0, true);

	split->setOrientation(QSplitter::Vertical);
	mLog = new QTextEdit(split, "log");
 	split->setResizeMode(mLog, QSplitter::Auto);
	mLog->setReadOnly(true);
	mLog->setTextFormat(Qt::RichText);
	mLog->setFocusPolicy(NoFocus);
	mLog->resize(0, 100);	
	if ( ! museeq->mShowStatusLog)
		mLog->hide();

	connect(museeq->driver(), SIGNAL(userStatus(const QString&, uint)), SLOT(slotUserStatus(const QString&, uint)));
	QSettings settings;
	QString exitdialog = settings.readEntry("/TheGraveyard.org/Museeq/ShowExitDialog");
	
	if (exitdialog.isEmpty() || exitdialog == "yes") {
		settings.writeEntry("/TheGraveyard.org/Museeq/ShowExitDialog", "yes");
		mMenuSettings->setItemChecked(9, true);
	}
	mMoves = 0;
	int w = settings.readNumEntry("/TheGraveyard.org/Museeq/Width", 600);
	int h = settings.readNumEntry("/TheGraveyard.org/Museeq/Height", -1);
	resize(w, h);
	
	bool ok = false;
	int x = settings.readNumEntry("/TheGraveyard.org/Museeq/X", 0, &ok);
	if(ok)
	{
		int y = settings.readNumEntry("/TheGraveyard.org/Museeq/Y", 0, &ok);
		if(ok)
			move(x, y);
	}
	
	box->setEnabled(false);
	daemon = new QProcess(this);
	connect( daemon, SIGNAL(readyReadStdout()), this,   SLOT(readFromStdout()) );
	connect( daemon, SIGNAL(processExited()),  this, SLOT(daemonExited()) );
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
	mIcons->setCurrentItem(page);
	mTitle->setText(mIcons->text(page));
	mStack->raiseWidget(page);
}
void MainWindow::changePage() {
	int ix = mIcons->currentItem();
	mTitle->setText(mIcons->text(ix));
	mStack->raiseWidget(ix);
}
void MainWindow::doDaemon() {

 	if (! daemon->isRunning()) {
		QSettings settings;
 		museekConfig = settings.readEntry("/TheGraveyard.org/Museeq/MuseekConfigFile");
		if (! museekConfig.isEmpty() ) {
			daemon->clearArguments();
			daemon->addArgument( "museekd" );
			daemon->addArgument( "--config" );
			daemon->addArgument( museekConfig );

			if (daemon->start()) {
				statusBar()->message(tr("Launched Museek Daemon..."));
				mConnectDialog->startDaemonButton->setDisabled(true);
				mConnectDialog->stopDaemonButton->setDisabled(false);
			} else {
				statusBar()->message(tr("Failed Launching Museek Daemon..."));
				mConnectDialog->startDaemonButton->setDisabled(false);
				mConnectDialog->stopDaemonButton->setDisabled(true);
			}
		} else {
			statusBar()->message(tr("No Config for Museek Daemon selected, giving up..."));
		}
	} else {
		statusBar()->message(tr("Museek Daemon is already running..."));
	}
}

void MainWindow::stopDaemon() {
	if (daemon->isRunning()) {
		daemon->tryTerminate();
		statusBar()->message(tr("Terminating Museek Daemon..."));
	} else {
		statusBar()->message(tr("Museek Daemon not running, no need to stop it..."));
	}

}

void MainWindow::daemonExited() {
	statusBar()->message(tr("Museek Daemon has Shut Down..."));
	mConnectDialog->startDaemonButton->setDisabled(false);
	mConnectDialog->stopDaemonButton->setDisabled(true);
}

void MainWindow::readFromStdout() {
// 	while (daemon->canReadLineStdout()) {
// 		printf( daemon->readLineStdout() );
// 		printf( "\n");
// 	}
}

void MainWindow::saveConnectConfig() {
	QSettings settings;
	QString server = mConnectDialog->mAddress->currentText(),
		password = mConnectDialog->mPassword->text().utf8();
	if(mConnectDialog->mAutoStartDaemon->isChecked()) {
		settings.writeEntry("/TheGraveyard.org/Museeq/LaunchMuseekDaemon", "yes");
		
	} else {
		settings.removeEntry("/TheGraveyard.org/Museeq/LaunchMuseekDaemon");
	}
	if(mConnectDialog->mAutoConnect->isChecked()) {
		settings.writeEntry("/TheGraveyard.org/Museeq/AutoConnect", "yes");
	} else {
		settings.removeEntry("/TheGraveyard.org/Museeq/AutoConnect");
	}
	
	if ( ! mConnectDialog->mMuseekConfig->text().isEmpty() )
		settings.writeEntry("/TheGraveyard.org/Museeq/MuseekConfigFile", mConnectDialog->mMuseekConfig->text() );
	else
		settings.removeEntry("/TheGraveyard.org/Museeq/MuseekConfigFile");
	if(mConnectDialog->mSavePassword->isChecked()) {
		settings.writeEntry("/TheGraveyard.org/Museeq/SavePassword", "yes");
		settings.writeEntry("/TheGraveyard.org/Museeq/Password", password);
	} else {
		settings.writeEntry("/TheGraveyard.org/Museeq/SavePassword", "no");
		settings.removeEntry("/TheGraveyard.org/Museeq/Password");
	}
	if(mConnectDialog->mShutDownDaemonOnExit->isChecked()) {
		settings.writeEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit", "yes");
	} else {
		settings.writeEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit", "no");
	}
	settings.beginGroup("/TheGraveyard.org/Museeq/Servers");
	int ix = 1;
	for(int i = 0; i < mConnectDialog->mAddress->count(); ++i)
	{
		QString s = mConnectDialog->mAddress->text(i);
		if(s != server)
		{
			settings.writeEntry(QString::number(ix), s);
			++ix;
		}
	}
	settings.writeEntry(QString::number(ix), server);
	settings.endGroup();
		
}

void MainWindow::connectToMuseek() {
	mMenuFile->setItemEnabled(0, false);

	mConnectDialog->mAddress->clear();
	QSettings settings;
	QString museekConfig;
	QString password;

	QString savePassword = settings.readEntry("/TheGraveyard.org/Museeq/SavePassword");
	
 	if (! savePassword.isEmpty())	
		if (savePassword == "yes") {
			mConnectDialog->mSavePassword->setChecked(true);
			password = settings.readEntry("/TheGraveyard.org/Museeq/Password");
			if ( !  password.isEmpty())
				mConnectDialog->mPassword->setText(password);
		} else  {
			mConnectDialog->mSavePassword->setChecked(false);
		}
	museekConfig= settings.readEntry("/TheGraveyard.org/Museeq/MuseekConfigFile");
	if (! museekConfig.isEmpty()) { 
		mConnectDialog->mMuseekConfig->setText(museekConfig);	
	}
	QString ShutDownDaemonOnExit = settings.readEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit");
	if (!  ShutDownDaemonOnExit.isEmpty())	 {
		if (ShutDownDaemonOnExit == "yes")
			mConnectDialog->mShutDownDaemonOnExit->setChecked(true);
		else
			mConnectDialog->mShutDownDaemonOnExit->setChecked(false);
		}
	else
		mConnectDialog->mShutDownDaemonOnExit->setChecked(false);
	QString launchMuseekDaemon = settings.readEntry("/TheGraveyard.org/Museeq/LaunchMuseekDaemon");

 	if (!  launchMuseekDaemon.isEmpty())	 {
		if ( launchMuseekDaemon == "yes") {
			mConnectDialog->mAutoStartDaemon->setChecked(true);
			mConnectDialog->mMuseekConfig->show();
			if (! museekConfig.isEmpty()) { 
				doDaemon();
			}
		} else  {
			mConnectDialog->mAutoStartDaemon->setChecked(false);
		}
	} else  {
		mConnectDialog->mAutoStartDaemon->setChecked(false);
	}
	if (daemon->isRunning()) {
		mConnectDialog->startDaemonButton->setDisabled(true);
		mConnectDialog->stopDaemonButton->setDisabled(false);
	} else {
		mConnectDialog->startDaemonButton->setDisabled(false);
		mConnectDialog->stopDaemonButton->setDisabled(true);
;
	}
	QStringList s_keys = settings.entryList("/TheGraveyard.org/Museeq/Servers");
	QString cServer;
	if(! s_keys.isEmpty()) {
		for(QStringList::Iterator it = s_keys.begin(); it != s_keys.end(); ++it)
		{
			cServer = settings.readEntry("/TheGraveyard.org/Museeq/Servers/" + (*it));
			mConnectDialog->mAddress->insertItem(cServer);
		}
	} else {
		cServer = "localhost:2240";
		mConnectDialog->mAddress->insertItem(cServer);
#ifdef HAVE_SYS_UN_H
# ifdef HAVE_PWD_H
		struct passwd *pw = getpwuid(getuid());
		if(pw)
			mConnectDialog->mAddress->insertItem(QString("/tmp/museekd.") + pw->pw_name);
# endif
#endif
	}
	mConnectDialog->mAddress->setCurrentItem(mConnectDialog->mAddress->count() - 1);
	slotAddressActivated(mConnectDialog->mAddress->currentText());
	// Display Connect Dialog
	QString autoConnect = settings.readEntry("/TheGraveyard.org/Museeq/AutoConnect");
	if (! autoConnect.isEmpty())	 {
		if ( autoConnect == "yes") {
			mConnectDialog->mAutoConnect->setChecked(true);
			if (savePassword == "yes" and ! password.isEmpty() ) {
				connectToMuseekPS(cServer, password);
				return;
			}
		} else {
			mConnectDialog->mAutoConnect->setChecked(false);
		}
	}
	
	if(mConnectDialog->exec() == QDialog::Accepted) {
		QString server = mConnectDialog->mAddress->currentText(),
			password = mConnectDialog->mPassword->text().utf8();
		saveConnectConfig();
		connectToMuseekPS(server, password);
		

	} else {
		mMenuFile->setItemEnabled(0, true);
	}
}
void MainWindow::connectToMuseekPS(const QString& server, const QString& password) {
	mMenuFile->setItemEnabled(1, true);
	if(mConnectDialog->mTCP->isChecked()) {
		int ix = server.find(':');
		Q_UINT16 port = server.mid(ix+1).toUInt();
		statusBar()->message(tr("Connecting to museek... Looking up host"));
		museeq->driver()->connectToHost(server.left(ix), port, password);
	} else {
		statusBar()->message(tr("Connecting to museek..."));
		museeq->driver()->connectToUnix(server, password);
	}
}
void MainWindow::slotHostFound() {
	statusBar()->message(tr("Connecting to museek... Connecting"));
}

void MainWindow::slotConnected() {
	statusBar()->message(tr("Connecting to museek... Logging in"));
}

void MainWindow::slotDisconnected() {
	centralWidget()->setEnabled(false);
	statusBar()->message(tr("Disconnected from museek"));
	
	mMenuFile->setItemEnabled(0, true);
	mMenuFile->setItemEnabled(1, false);
	mMenuFile->setItemEnabled(2, false);
	mMenuFile->setItemEnabled(3, false);
	mMenuFile->setItemEnabled(4, false);

	mMenuSettings->setItemEnabled(1, false);
	mMenuSettings->setItemEnabled(2, false);
	mMenuSettings->setItemEnabled(3, false);

	mMenuSettings->setItemEnabled(5, false);
	mMenuSettings->setItemEnabled(6, false);
	mMenuSettings->setItemEnabled(7, false);
#ifdef HAVE_TRAYICON
	museeq->trayicon_setIcon("disconnect");
#endif // HAVE_TRAYICON
}

void MainWindow::slotError(int e) {
	switch(e) {
	case QSocket::ErrConnectionRefused:
		statusBar()->message(tr("Cannot connect to museek... Connection refused"));
		break;
	case QSocket::ErrHostNotFound:
		statusBar()->message(tr("Cannot connect to museek... Host not found"));
		break;
	}
	doNotAutoConnect();	
	mMenuFile->setItemEnabled(0, true);
	mMenuFile->setItemEnabled(1, false);
	
}

void MainWindow::slotLoggedIn(bool success, const QString& msg) {
	if(success) {
		statusBar()->message(tr("Logged in to museek"));
		
		centralWidget()->setEnabled(true);

		mMenuSettings->setItemEnabled(1, true);
		mMenuSettings->setItemEnabled(2, true);
		mMenuSettings->setItemEnabled(3, true);

		mMenuSettings->setItemEnabled(5, true);
		mMenuSettings->setItemChecked(5, museeq->mShowTickers);
		mMenuSettings->setItemEnabled(6, true);
		mMenuSettings->setItemChecked(6, museeq->mShowStatusLog);
		mMenuSettings->setItemEnabled(7, true);
		mMenuSettings->setItemChecked(7, museeq->mShowTimestamps);
	} else {
		statusBar()->message(tr("Login error: ") + msg);
		mMenuFile->setItemEnabled(0, true);
		mMenuFile->setItemEnabled(1, false);
		mMenuFile->setItemEnabled(2, false);
		mMenuFile->setItemEnabled(3, false);
		doNotAutoConnect();
#ifdef HAVE_TRAYICON
		museeq->trayicon_setIcon("disconnect");
#endif // HAVE_TRAYICON
	}
}
void MainWindow::doNotAutoConnect() {
	if(mConnectDialog->mAutoConnect->isChecked()) {
		QSettings settings;
		settings.removeEntry("/TheGraveyard.org/Museeq/AutoConnect");
		mConnectDialog->mAutoConnect->setChecked(false);
		mMenuSettings->setItemChecked(8, false);
	}
}

#define escape QStyleSheet::escape

#define _TIME QString("<span style='"+museeq->mFontTime+"'><font color='"+museeq->mColorTime+"'>") + QDateTime::currentDateTime().toString("hh:mm:ss") + "</font></span> "
void MainWindow::slotStatusMessage(bool type, const QString& msg) {

	QString Message = msg;
	QStringList wm = QStringList::split("\n", msg, true);
	QStringList::iterator it = wm.begin();
	for(; it != wm.end(); ++it) {
		if (museeq->mShowTimestamps)
			mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+escape(*it)+"</font></span>"));
		else
			mLog->append(QString("<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+escape(*it)+"</font></span>"));
	}
}
void MainWindow::slotUserStatus( const QString & user, uint status ) {
 	if (museeq->mOnlineAlert  && museeq->hasAlert(user)) {
		QString s = (status == 0) ? "offline" : ((status == 1) ? "away" : "online");
		mLog->append(QString(_TIME)+QString("<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>user %2 is now %3</font></span>").arg(escape(user)).arg(s)) ;
		
	}
}
void MainWindow::slotStatusSet(uint status) {
	if (status) {
		statusBar()->message(tr("Connected to soulseek, your nickname: ") + museeq->nickname() + tr(" Status: Away") );
		mMenuFile->setItemChecked(2, true);
#ifdef HAVE_TRAYICON
		museeq->trayicon_setIcon("away");
#endif // HAVE_TRAYICON
	} else {
		statusBar()->message(tr("Connected to soulseek, your nickname: ") + museeq->nickname() + tr(" Status: Online") );
		mMenuFile->setItemChecked(2, false);
#ifdef HAVE_TRAYICON
		museeq->trayicon_setIcon("online");
#endif // HAVE_TRAYICON
	}
}
void MainWindow::slotConnectedToServer(bool connected) {
	if(connected) {
		statusBar()->message(tr("Connected to soulseek, your nickname: ") + museeq->nickname());
		mMenuFile->setItemEnabled(2, true);
		mMenuFile->setItemEnabled(3, true);
		mMenuFile->setItemEnabled(4, true);
#ifdef HAVE_TRAYICON
		museeq->trayicon_setIcon("connect");
#endif // HAVE_TRAYICON
	} else {
		statusBar()->message(tr("Disconnected from soulseek"));
		mMenuFile->setItemEnabled(2, false);
		mMenuFile->setItemEnabled(3, false);
		mMenuFile->setItemEnabled(4, false);
	}
}

void MainWindow::showIPDialog() {
	mIPDialog->show();
}

void MainWindow::showIPDialog(const QString& user) {
	QListViewItem *item = mIPDialog->mIPListView->findItem(user, 0);
	if(item) {
		item->setText(1, tr("waiting"));
		item->setText(2, "");
		item->setText(3, "");
	} else  {
		item = new QListViewItem(mIPDialog->mIPListView, user, tr("waiting"), "", "");
		item->setSelectable(false);
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

	QSettings settings;
	QString _path = QString(DATADIR) + "/museek/museeq";
	QDir dir  (_path);
	QFileDialog * fd = new QFileDialog(dir.path(), "", this);
	fd->setCaption(tr("Enter a Museeq Icon Theme Directory"));
	fd->setMode(QFileDialog::Directory);
	fd->addFilter(tr("Museeq's Icon Theme (*.png)")); 
	if(fd->exec() == QDialog::Accepted){
		museeq->mIconTheme = fd->dirPath();
		settings.beginGroup("/TheGraveyard.org/Museeq");
		settings.writeEntry("IconTheme", museeq->mIconTheme);
		settings.endGroup();
	}
	delete fd;
	
}
void MainWindow::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {
	if(domain == "museeq.group") {
		bool on = value == "true";
		if(key == "downloads")
			mTransfers->groupDownloads(on);
		else if(key == "uploads")
			mTransfers->groupUploads(on);
	} else if(domain == "museeq.text") {
		
		if (key == "fontTime") {
			mColorsDialog->STimeFont->setText(value);}
		else if (key == "fontMessage") {
			mColorsDialog->SMessageFont->setText(value);}
		else if (key == "colorBanned") {
			mColorsDialog->SBannedText->setText(value);}
		else if (key == "colorBuddied") {
			mColorsDialog->SBuddiedText->setText(value);}
		else if (key == "colorMe") {
			mColorsDialog->SMeText->setText(value);}
		else if (key == "colorNickname") {
			mColorsDialog->SNicknameText->setText(value);}
		else if (key == "colorTrusted") {
			mColorsDialog->STrustedText->setText(value);}
		else if (key == "colorRemote") {
			mColorsDialog->SRemoteText->setText(value);}
		else if (key == "colorTime") {
			mColorsDialog->STimeText->setText(value);}
	} else if (domain == "museeq.statuslog") {
		if (key =="ip") {
			if(value == "true")
				mSettingsDialog->SIPLog->setChecked(true);
			else if(value == "false")
				mSettingsDialog->SIPLog->setChecked(false);
		}
	} else if(domain == "userinfo" && key == "text") {
		mUserInfoDialog->mText->setText(value);
	} else if(domain == "server" && key == "host") {
		mSettingsDialog->SServerHost->setText(value);
	} else if(domain == "server" && key == "username") {
		mSettingsDialog->SSoulseekUsername->setText(value);
	} else if(domain == "server" && key == "password") {
		mSettingsDialog->SSoulseekPassword->setText(value);
	} else if(domain == "server" && key == "port") {
		mSettingsDialog->SServerPort->setValue(value.toInt());
	} else if(domain == "transfers" && key == "have_buddy_shares") {
		if  (value == "true")  { mSettingsDialog->SBuddiesShares->setChecked(true); }
		else if (value == "false") { mSettingsDialog->SBuddiesShares->setChecked(false); }

	} else if(domain == "transfers" && key == "only_buddies") {
		if  (value == "true")  { mSettingsDialog->SShareBuddiesOnly->setChecked(true); }
		else if (value == "false") { mSettingsDialog->SShareBuddiesOnly->setChecked(false); }
	} else if(domain == "transfers" && key == "privilege_buddies") {
		if (value == "true") mSettingsDialog->SBuddiesPrivileged->setChecked(true);
		else if (value == "false") mSettingsDialog->SBuddiesPrivileged->setChecked(false);
	} else if(domain == "transfers" && key == "trusting_uploads") {
		if (value == "true") mSettingsDialog->STrustedUsers->setChecked(true);
		else if ( value == "false") mSettingsDialog->STrustedUsers->setChecked(false);
	} else if(domain == "transfers" && key == "user_warnings") {
		if (value == "true") mSettingsDialog->SUserWarnings->setChecked(true);
		else if ( value == "false") mSettingsDialog->SUserWarnings->setChecked(false);
	} else if(domain == "transfers" && key == "download-dir") {
		mSettingsDialog->SDownDir->setText(value);
	} else if(domain == "transfers" && key == "incomplete-dir") {
		mSettingsDialog->SIncompleteDir->setText(value);
	} else if(domain == "museeq.alerts" && key == "log_window") {
		if (value == "true") mSettingsDialog->SOnlineAlerts->setChecked(true);
		else if ( value == "false") mSettingsDialog->SOnlineAlerts->setChecked(false);
	} else if(domain == "clients" && key == "connectmode") {
		if (value == "active") mSettingsDialog->SActive->setChecked(true);
		else if (value == "passive") mSettingsDialog->SPassive->setChecked(true);
	}
}



void MainWindow::startSearch(const QString& query) {
	mSearches->doSearch(query);
	mIcons->setCurrentItem(3);
}

void MainWindow::showPrivateChat(const QString& user) {
	mPrivateChats->setPage(user);
	mIcons->setCurrentItem(1);
}

void MainWindow::showUserInfo(const QString& user) {
	mUserInfos->setPage(user);
	mIcons->setCurrentItem(4);
}

void MainWindow::showBrowser(const QString& user) {
	mBrowsers->setPage(user);
	mIcons->setCurrentItem(5);
}

void MainWindow::slotUserAddress(const QString& user, const QString& ip, uint port) {
	QListViewItem* item = mIPDialog->mIPListView->findItem(user, 0);
	if(item) {
		if(ip == "0.0.0.0") {
			item->setText(1, tr("Offline"));
			item->setText(2, "");
		} else {
			item->setText(1, ip);
			item->setText(2, QString::number(port));
#ifdef HAVE_NETDB_H
			struct hostent *addr = gethostbyname(ip);
			if(addr && addr->h_length) {
				struct hostent *addr2 = gethostbyaddr(addr->h_addr_list[0], 4, AF_INET);
				if(addr2 && addr2->h_name)
					item->setText(3, addr2->h_name);
			}
#endif // HAVE_NETDB_H
		}

		if (museeq->mIPLog) {
			if (museeq->mShowTimestamps) {
					mLog->append(QString(_TIME+"<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("IP of ")+escape(user)+": "+ ip +" "+ tr("Port:")+" "+QString::number(port)+"</font></span>"));
				} else {
					mLog->append(QString("<span style='"+museeq->mFontMessage+"'><font color='"+museeq->mColorRemote+"'>"+tr("IP of ")+escape(user)+": "+ ip +" "+ tr("Port:")+" "+QString::number(port)+"</font></span>"));
				}
			}
	}
}
void MainWindow::toggleTickers() {
	if (museeq->mShowTickers == true)
		museeq->setConfig("museeq.tickers", "show", "false");
	else if (museeq->mShowTickers == false)
		museeq->setConfig("museeq.tickers", "show", "true");
}
void MainWindow::toggleTimestamps() {
	if (museeq->mShowTimestamps == true)
		museeq->setConfig("museeq.text", "showTimestamps", "false");
	else if (museeq->mShowTimestamps == false)
		museeq->setConfig("museeq.text", "showTimestamps", "true");
}
void MainWindow::toggleLog() {
	if (museeq->mShowStatusLog == true)
		museeq->setConfig("museeq.statuslog", "show", "false");
	else if (museeq->mShowStatusLog == false)
		museeq->setConfig("museeq.statuslog", "show", "true");
}
void MainWindow::toggleAutoConnect() {
	QSettings settings;
	if(mConnectDialog->mAutoConnect->isChecked()) {
		settings.removeEntry("/TheGraveyard.org/Museeq/AutoConnect");
		mMenuSettings->setItemChecked(8, false);
	} else {
		settings.writeEntry("/TheGraveyard.org/Museeq/AutoConnect", "yes");
		mMenuSettings->setItemChecked(8, true);
	}
}
void MainWindow::toggleExitDialog() {
	QSettings settings;
	if(settings.readEntry("/TheGraveyard.org/Museeq/ShowExitDialog") == "yes") {
		settings.writeEntry("/TheGraveyard.org/Museeq/ShowExitDialog", "no");
		mMenuSettings->setItemChecked(9, false);
	} else {
		settings.writeEntry("/TheGraveyard.org/Museeq/ShowExitDialog", "yes");
		mMenuSettings->setItemChecked(9, true);
	}
}
void MainWindow::changeColors() {
	if(mColorsDialog->exec() == QDialog::Accepted) {
		if (! mColorsDialog->SMessageFont->text().isEmpty() )
			museeq->setConfig("museeq.text", "fontMessage", mColorsDialog->SMessageFont->text());
		if (! mColorsDialog->STimeFont->text().isEmpty() )
			museeq->setConfig("museeq.text", "fontTime", mColorsDialog->STimeFont->text());
		if (! mColorsDialog->STimeText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorTime", mColorsDialog->STimeText->text());
		if (! mColorsDialog->SRemoteText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorRemote", mColorsDialog->SRemoteText->text());
		if (! mColorsDialog->SMeText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorMe", mColorsDialog->SMeText->text());
		if (! mColorsDialog->SNicknameText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorNickname", mColorsDialog->SNicknameText->text());
		if (! mColorsDialog->SBuddiedText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorBuddied", mColorsDialog->SBuddiedText->text());
		if (! mColorsDialog->SBannedText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorBanned", mColorsDialog->SBannedText->text());
		if (! mColorsDialog->STrustedText->text().isEmpty() )
			museeq->setConfig("museeq.text", "colorTrusted", mColorsDialog->STrustedText->text());
	}
}

void MainWindow::changeUserInfo() {
	if(mUserInfoDialog->exec() == QDialog::Accepted) {
		museeq->setConfig("userinfo", "text", mUserInfoDialog->mText->text());
		if(mUserInfoDialog->mUpload->isChecked()) {
			QFile f(mUserInfoDialog->mImage->text());
			if(f.open(IO_ReadOnly)) {
				QByteArray data = f.readAll();
				f.close();
				museeq->driver()->setUserImage(data);
				mUserInfoDialog->mDontTouch->toggle();
			} else
				QMessageBox::warning(this, tr("Error"), tr("Couldn't open image file for reading"));
		} else if(mUserInfoDialog->mClear->isChecked()) {
			museeq->driver()->setUserImage(QByteArray());
		}
	}
}

void MainWindow::saveSettings() {

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
	if (! mSettingsDialog->SIncompleteDir->text().isEmpty() )
		museeq->setConfig("transfers", "incomplete-dir", mSettingsDialog->SIncompleteDir->text());
	if(mSettingsDialog->SOnlineAlerts->isChecked()) {
		museeq->setConfig("museeq.alerts", "log_window", "true");
	} else {  museeq->setConfig("museeq.alerts", "log_window", "false");  }
	if (mSettingsDialog->SIPLog->isChecked()) {
		museeq->setConfig("museeq.statuslog", "ip", "true");
	} else { museeq->setConfig("museeq.statuslog", "ip", "false"); }

	if(mSettingsDialog->SActive->isChecked()) {
		museeq->setConfig("clients", "connectmode", "active");
	}
	else if (mSettingsDialog->SPassive->isChecked()) {
		museeq->setConfig("clients", "connectmode", "passive");
	}
	if(mSettingsDialog->SBuddiesShares->isChecked()) {
		museeq->setConfig("transfers", "have_buddy_shares", "true");
	}
	else {  museeq->setConfig("transfers", "have_buddy_shares", "false");  }
	if(mSettingsDialog->SShareBuddiesOnly->isChecked()) {
		museeq->setConfig("transfers", "only_buddies", "true");
	}
	else {  museeq->setConfig("transfers", "only_buddies", "false");  }
	if(mSettingsDialog->SBuddiesPrivileged->isChecked()) {
		museeq->setConfig("transfers", "privilege_buddies", "true");
	}
	else { museeq->setConfig("transfers", "privilege_buddies", "false"); }

	if(mSettingsDialog->STrustedUsers->isChecked()) {
		museeq->setConfig("transfers", "trusting_uploads", "true");
	}
	else { museeq->setConfig("transfers", "trusting_uploads", "false"); }
	if(mSettingsDialog->SUserWarnings->isChecked()) {
		museeq->setConfig("transfers", "user_warnings", "true");
	}
	else { museeq->setConfig("transfers", "user_warnings", "false"); }

}

void MainWindow::changeSettings() {
	if(mSettingsDialog->exec() == QDialog::Accepted) {
		saveSettings();
	}
}

// MainWindow::MainWindow(QWidget* parent, const char* name) : QMainWindow(parent, name), mWaitingPrivs(false) {
void MainWindow::displayAboutDialog() {
	QMessageBox::about(this, tr("About Museeq"), tr("<p align=\"center\">Museeq ") + mVersion + tr(" is a GUI for the Museek Daemon</p>The programs, museeq and museekd and muscan, were created by Hyriand 2003-2005<br><br>Additions by Daelstorm and SeeSchloss in 2006<br>This project is released under the GPL license.<br>Code and ideas taken from other opensource projects and people are mentioned in the CREDITS file included in the source tarball."));
}
void MainWindow::displayCommandsDialog() {
	QMessageBox::information(this, tr("Museeq Commands"), tr("<h3>While in a chat window, such as a Chat Room, or a Private Chat, there are a number of commands available for use.</h3><b>/c /chat</b>   <i>(Switch to Chat Rooms)</i><br><b>/pm /private</b> &lt;nothing | username&gt;  <i>(Switch to Private Chat and start chatting with a user, if inputed)</i><br><b>/transfers</b>   <i>(Switch to Transfers)</i><br><b>/s /search</b> &lt;nothing | query>   <i>(Switch to Searchs and start a Search with &lt;query&gt; if inputed)</i><br><b>/u /userinfo</b> &lt;username&gt;   <i>(Switch to userinfo, and attempt to get a user's info, if inputed)</i><br><b>/b /browse</b> &lt;username&gt;    <i>(Switch to Browse and initate browsing a user, if inputed)</i><br><b>/ip</b> &lt;username&gt;   <i>(Get the IP of a user)</i><br><b>/log</b>    <i>(Toggle displaying the Special Message Log)</i><br><b>/t /ticker /tickers</b>   <i>(Toggle the showing of Tickers)</i> <br><b>/f /fonts /c /colors</b>   <i>(Open the Fonts and Colors settings dialog)</i><br><b>/ban /unban</b> &lt;username&gt;   <i>(Disallow/Allow a user to recieve your shares and download from you)</i><br><b>/ignore /unignore</b> &lt;username&gt;    <i>(Block/Unblock chat messages from a user)</i><br><b>/buddy /unbuddy</b> &lt;username&gt;   <i>(Add/Remove a user to keep track of it and add comments about it)</i><br><b>/trust /distrust</b> &lt;username&gt;    <i>(Add/Remove a user to the optional list of users who can send files to you)</i><br><b>/me</b> <does something>    <i>(Say something in the Third-Person)</i><br><b>/slap</b> &lt;username&gt;   <i>(Typical Trout-slapping)</i><br><b>/j /join</b> &lt;room&gt;    <i>(Join a Chat Room)</i><br><b>/l /p /leave /part</b> &lt;nothing | room&gt;    <i>(Leave the current room or inputed room)</i><br><b>/about /help /commands</b>    <i>(Display information)</i><br><br>Do not type the brackets, they are there only to make clear that something (or nothing) can be typed after the /command."));
}

void MainWindow::displayHelpDialog() {
	QMessageBox::information(this, tr("Museeq Help"), tr("<h3>What's going on? I can't connect to a Soulseek Server with museeq!</h3> You connect to museekd with museeq, so you need to have <b>museekd</b> configured, running <u>and</u> connected to a <b>Soulseek</b> or Soulfind server. <br> <h3>Running for the first time?</h3> Before you start museekd for the first time, you need to configure <b>museekd</b> with <b>musetup</b>,  a command-line configuration script.<br><br> In musetup you <b>MUST</b> configure the following items: Server, Username, Password, Interface Password, Download Dir<br> Also, take note of your interfaces, if you change them from the default localhost:2240 and /tmp/museek.<tt>USERNAME</tt>, you'll need to know them for logging in with museeq. <br><br> When you start museeq or choose File->Connect from the menu, you are asked to input the host and port, or Unix Socket of museekd, <b>not</b> the Server.<br> <h3>Want to send someone a file?</h3> Browse yourself, select file(s), and right-click->Upload. Input their name in the dialog box, and the upload should start, but it depends on if the user has place you on their \"trusted\" or \"uploads\" users list .<br>Once you're connected to museekd, change museekd options via Settings->Museek"));


}

void MainWindow::protocolHandlers() {
	mProtocolDialog->mProtocols->clear();
	
	QMap<QString, QString> handlers = museeq->protocolHandlers();
	QMap<QString, QString>::ConstIterator it, end = handlers.end();
	for(it = handlers.begin(); it != end; ++it)
		new QListViewItem(mProtocolDialog->mProtocols, it.key(), it.data());
	
	if(mProtocolDialog->exec() == QDialog::Accepted) {
		handlers.clear();
		QListViewItemIterator it = QListViewItemIterator(mProtocolDialog->mProtocols);
		while(it.current()) {
			handlers[it.current()->text(0)] = it.current()->text(1);
			++it;
		}
		museeq->setProtocolHandlers(handlers);
	}
}

void MainWindow::protocolHandlerMenu(QListViewItem *item, const QPoint& pos, int) {
	if(! item)
		return;
	QPopupMenu menu;
	int id = menu.insertItem(tr("Delete handler"));
	if(menu.exec(pos) == id)
		delete item;
}
// added by d vv
void MainWindow::ipDialogMenu(QListViewItem *item, const QPoint& pos, int) {
	if(! item)
		return;
	//QClipboard *cb
	QPopupMenu menu;
	int id = menu.insertItem(tr("Delete"));

		if(menu.exec(pos) == id)
			delete item;
	


}
void QClipboard()
//(QClipboard *cb)
{
	return;
}

 // added by d ^^
void MainWindow::givePrivileges(const QString& user)
{
	bool ok = false;
	int days = QInputDialog::getInteger(tr("Give privileges"),
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
#ifdef HAVE_TRAYICON
	if (museeq->mUsetray == true) {
		museeq->trayicon_hide();
	} else if (museeq->mUsetray == false) {
		museeq->trayicon_show();
	}
#endif // HAVE_TRAYICON
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
	if(mMoves < 2)
	{
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
	QSettings settings;
	if ( settings.readEntry("/TheGraveyard.org/Museeq/ShowExitDialog") == "yes") {
		if (daemon->isRunning() && settings.readEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit") == "yes") {
			if (QMessageBox::question(this, tr("Shutdown Museeq"), tr("The Museek Daemon was launched by Museeq and is still running, and will be shut down if you close Museeq, are you sure you want to?"), tr("&Yes"), tr("&No"), QString::null, 1 ) ) {
				return;
			}
		} else if (daemon->isRunning() && settings.readEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit") == "no")  {
			if (QMessageBox::question(this, tr("Shutdown Museeq"), tr("The Museek Daemon was launched by Museeq and is still running, but will <b>not</b> be shut down if you close Museeq. Are you sure you want to?"), tr("&Yes"), tr("&No"), QString::null, 1 ) ) {
				return;
			}
		} else {
			if (QMessageBox::question(this, tr("Shutdown Museeq"), tr("It's safe to close Museeq, but are you sure you want to?"), tr("&Yes"), tr("&No"), QString::null, 1 ) ) {
				return;
			}
		}
	}
	settings.beginGroup("/TheGraveyard.org/Museeq");
	settings.writeEntry("X", mLastPos.x());
	settings.writeEntry("Y", mLastPos.y());
	settings.writeEntry("Width", mLastSize.width());
	settings.writeEntry("Height", mLastSize.height());
	settings.endGroup();
	if ( settings.readEntry("/TheGraveyard.org/Museeq/ShutDownDaemonOnExit") == "yes")
		stopDaemon();
	QMainWindow::closeEvent(ev);

}

void MainWindow::loadScript() {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	QString fn = QFileDialog::getOpenFileName("", "*.qs", this, 0, tr("Load Script"));
	if(! fn.isEmpty()) {
		QFile f(fn);
		if(f.open(IO_ReadOnly))
		{
			museeq->loadScript(f.readAll());
			f.close();
		}
	}
#endif // HAVE_QSA
}

void MainWindow::unloadScript(int i) {
#ifdef HAVE_QSA
	if(libqsa_is_present)
		museeq->unloadScript(mMenuUnloadScripts->text(i));
#endif // HAVE_QSA
}

void MainWindow::addScript(const QString& scriptname) {
#ifdef HAVE_QSA
	if(libqsa_is_present)
		mMenuUnloadScripts->insertItem(scriptname);
#endif // HAVE_QSA
}

void MainWindow::removeScript(const QString& scriptname) {
#ifdef HAVE_QSA
	if(! libqsa_is_present)
		return;
	
	for(int i = 0; i < (int)mMenuUnloadScripts->count(); i++) {
		int id = mMenuUnloadScripts->idAt(i);
		if(mMenuUnloadScripts->text(id) == scriptname) {
			mMenuUnloadScripts->removeItem(id);
			return;
		}
	}
#endif // HAVE_QSA
}
