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

#include "settingsdialog.h"
#include "museeq.h"
#include "codeccombo.h"
#include "images.h"

#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QColor>
#include <QFontDialog>
#include <QLayout>
#include <QFileDialog>
#include <QDir>
#include <QCloseEvent>
#include <QSettings>

SettingsDialog::SettingsDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent ), mSharesDirty(false)
{
	// Layout Containing everything
	QVBoxLayout* vLayout= new QVBoxLayout( this);
	vLayout->setMargin(5);
	vLayout->setSpacing(5);

	// Tabs, without geometry set will resize to fill the dialog
	mTabHolder = new QTabWidget( this );

	// Add tabs to vLayout
	vLayout->addWidget(mTabHolder);

	QHBoxLayout* buttonsLayout= new QHBoxLayout;

	// Ok, Save, Cancel buttons
	QSpacerItem* spacer5 = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonsLayout->addItem( spacer5 );

	mOK = new QPushButton( this );
	mOK->setIcon(IMG("ok"));
	buttonsLayout->addWidget(mOK);

	mSave = new QPushButton( this );
	mSave->setIcon(IMG("save"));
	buttonsLayout->addWidget(mSave);

	mCancel = new QPushButton( this);
	mCancel->setIcon(IMG("cancel"));
	mCancel->setDefault( TRUE );
	buttonsLayout->addWidget(mCancel);

	// Add buttons to vLayout
	vLayout->addLayout(buttonsLayout);

	mTabHolder->setTabPosition( QTabWidget::North );
	mMuseekdTabs = new QTabWidget( mTabHolder );


	mTabHolder->addTab( mMuseekdTabs, QString::fromLatin1("") );
	mMuseeqTabs = new QTabWidget( this);
	mTabHolder->addTab( mMuseeqTabs, QString::fromLatin1("") );
	// SERVER TAB
	serverTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( serverTab, QString::fromLatin1("") );
	QVBoxLayout * ServerLayout = new QVBoxLayout( serverTab);

	QGroupBox * groupBox = new QGroupBox(tr("Host"), serverTab);
	ServerLayout->addWidget(groupBox);
	ServerGrid = new QGridLayout(groupBox);

	// Server Host
	serverHostLabel = new QLabel( serverTab);
	ServerGrid->addWidget( serverHostLabel, 0, 0 );


	SServerHost = new QLineEdit( serverTab);
	SServerHost->setMaxLength( 50 );
	ServerGrid->addWidget( SServerHost, 0, 1, 1, 1 );

	// Server Port
	serverPortLabel = new QLabel( serverTab );
	serverPortLabel->setMargin( 0 );

	ServerGrid->addWidget( serverPortLabel, 1, 0 );
	SServerPort = new QSpinBox( serverTab);
	SServerPort->setMaximum( 65535 );
	SServerPort->setValue( 0 );

	ServerGrid->addWidget( SServerPort, 1, 1, 1, 1 );
	// Server Username
	usernamelabel = new QLabel( serverTab );
	ServerGrid->addWidget( usernamelabel, 2, 0 );

	SSoulseekUsername = new QLineEdit( serverTab);
	ServerGrid->addWidget( SSoulseekUsername, 2, 1, 1, 1 );
	// Server Password
	passwordLabel = new QLabel( serverTab );
	ServerGrid->addWidget( passwordLabel, 3, 0 );

	SSoulseekPassword = new QLineEdit( serverTab );
	SSoulseekPassword->setEchoMode(QLineEdit::Password);
	ServerGrid->addWidget( SSoulseekPassword, 3, 1, 1, 1 );

	// Connect / Disconnect
	QHBoxLayout * SideLayout = new QHBoxLayout;
	ServerLayout->addLayout(SideLayout);
	QGroupBox * groupBox2 = new QGroupBox(tr("Server status"), serverTab);
	SideLayout->addWidget(groupBox2);
	QVBoxLayout * DaemonCLayout = new QVBoxLayout;
	groupBox2->setLayout(DaemonCLayout);
	SConnect = new QPushButton( serverTab);
	SConnect->setIcon(IMG("connect"));
	DaemonCLayout->addWidget( SConnect );

	SDisconnect = new QPushButton( serverTab);
	SDisconnect->setIcon(IMG("disconnect"));

	DaemonCLayout->addWidget( SDisconnect );
	// Filesystem encoding
	QGridLayout * ServerGrid2 = new QGridLayout;
	SideLayout->addLayout(ServerGrid2);
	fEncodingLabel = new QLabel( serverTab );
	ServerGrid2->addWidget( fEncodingLabel, 6, 0 );
	SFileSystemEncoding =  new CodecCombo("encoding", "filesystem", serverTab, "encoding");
	ServerGrid2->addWidget( SFileSystemEncoding, 6, 1 , 1, 1 );
	// Network Encoding
	nEncodingLabel = new QLabel( serverTab );
	ServerGrid2->addWidget( nEncodingLabel, 7, 0 );
	SNetworkEncoding =  new CodecCombo("encoding", "network", serverTab, "encoding");
	ServerGrid2->addWidget( SNetworkEncoding, 7, 1, 1, 1 );


	QHBoxLayout * SConfigLayout = new QHBoxLayout;
	ServerLayout->addLayout(SConfigLayout);
	configLabel = new QLabel( serverTab );
	SConfigLayout->addWidget( configLabel);

	SConfigFile = new QLineEdit( serverTab );
	SConfigLayout->addWidget( SConfigFile);

	SConfigButton = new QPushButton( serverTab );
	SConfigButton->setIcon(IMG("open"));
	SConfigLayout->addWidget( SConfigButton);
	spacerServer = new QSpacerItem( 20, 20, QSizePolicy::Preferred, QSizePolicy::Expanding );
	ServerLayout->addItem(spacerServer);



	// SHARES TAB
	sharesTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( sharesTab, QString::fromLatin1("") );
	SharesGrid = new QGridLayout( sharesTab);

	downloadLabel = new QLabel( sharesTab);
	downloadLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( downloadLabel, 4, 0 );

	SDownDir = new QLineEdit( sharesTab);
	SharesGrid->addWidget( SDownDir, 4, 1);

	SDownloadButton = new QPushButton( sharesTab );
	SDownloadButton->setIcon(IMG("open"));
	SharesGrid->addWidget( SDownloadButton, 4, 2);

	incompleteLabel = new QLabel( sharesTab);
	incompleteLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( incompleteLabel, 5, 0);

	SIncompleteDir = new QLineEdit( sharesTab);
	SharesGrid->addWidget( SIncompleteDir, 5, 1);

	SIncompleteButton = new QPushButton( sharesTab);
	SIncompleteButton->setIcon(IMG("open"));
	SharesGrid->addWidget( SIncompleteButton, 5, 2);


	// Shares List
	ListNormalShares = new QTreeWidget(sharesTab);
	ListNormalShares->setRootIsDecorated(false);
	QStringList NormalSharesHeaders;
	NormalSharesHeaders <<  tr("Directories");
	ListNormalShares->setHeaderLabels(NormalSharesHeaders);
	SharesGrid->addWidget( ListNormalShares, 6,  0,  1, 2);
	// Shares List Buttons
	QVBoxLayout* sharesListButtons = new QVBoxLayout;
	SharesGrid->addLayout(sharesListButtons, 6, 2);

	NSharesRefresh = new QPushButton( sharesTab );
	NSharesRefresh->setIcon( IMG("reload"));
	sharesListButtons->addWidget( NSharesRefresh);
	NSharesUpdate = new QPushButton( sharesTab );
	NSharesUpdate->setIcon(IMG("redo"));
	sharesListButtons->addWidget( NSharesUpdate);
	NSharesRescan = new QPushButton( sharesTab);
	NSharesRescan->setIcon(IMG("rescan"));
	sharesListButtons->addWidget( NSharesRescan);
	NSharesAdd = new QPushButton( sharesTab);
	NSharesAdd->setIcon(IMG("add"));
	sharesListButtons->addWidget( NSharesAdd);
	NSharesRemove = new QPushButton( sharesTab);
	NSharesRemove->setIcon(IMG("remove"));
	sharesListButtons->addWidget( NSharesRemove);

	// Buddy Shares List

	SBuddiesShares = new QCheckBox( sharesTab);
	SharesGrid->addWidget( SBuddiesShares, 7, 0, 1, 2 );

	ListBuddyShares = new QTreeWidget(sharesTab);
	ListBuddyShares->setRootIsDecorated(false);
	QStringList BuddySharesHeaders;
	BuddySharesHeaders <<  tr("Directories");
	ListBuddyShares->setHeaderLabels(BuddySharesHeaders);
	SharesGrid->addWidget( ListBuddyShares, 8,  0, 1, 2);
	// Buddy Shares List Buttons
	QVBoxLayout* sharesBuddyListButtons = new QVBoxLayout;
	SharesGrid->addLayout(sharesBuddyListButtons, 8, 2);
	SharesGrid->setRowStretch(6, 2);
	SharesGrid->setRowStretch(8, 2);
// 	SharesGrid->setRowStretch(10, 1);
	BSharesRefresh = new QPushButton( sharesTab);
	BSharesRefresh->setIcon( IMG("reload"));
	sharesBuddyListButtons->addWidget( BSharesRefresh);
	BSharesUpdate = new QPushButton( sharesTab );
	BSharesUpdate->setIcon(IMG("redo"));
	sharesBuddyListButtons->addWidget( BSharesUpdate);
	BSharesRescan = new QPushButton( sharesTab);
	BSharesRescan->setIcon(IMG("rescan"));
	sharesBuddyListButtons->addWidget( BSharesRescan);
	BSharesAdd = new QPushButton( sharesTab);
	BSharesAdd->setIcon(IMG("add"));
	sharesBuddyListButtons->addWidget( BSharesAdd);
	BSharesRemove = new QPushButton( sharesTab);
	BSharesRemove->setIcon(IMG("remove"));
	sharesBuddyListButtons->addWidget( BSharesRemove);



	// Connections Tab
	connectionsTab = new QWidget( mMuseekdTabs );
	mMuseekdTabs->addTab( connectionsTab, QString::fromLatin1("") );
	ConnectionsGrid = new QGridLayout( connectionsTab);
// 	buttonGroup2 = new QHButtonGroup( connectionsTab );
	QGroupBox * connectionsBox = new QGroupBox(tr("Peer Connections"), connectionsTab);
	ConnectionsGrid->addWidget(connectionsBox, 0, 0, 1, 4);
	QHBoxLayout * cboxLayout = new QHBoxLayout(connectionsBox);
// 	connectionsBox->setLayout();
	SActive = new QRadioButton( connectionsTab );
	SPassive = new QRadioButton( SActive);

	cboxLayout->addWidget(SActive);
	cboxLayout->addWidget(SPassive);

	listenPortsLabel = new QLabel( connectionsTab);
	listenPortsLabel->setTextFormat(Qt::RichText);
	ConnectionsGrid->addWidget( listenPortsLabel, 1, 0, 1, 4);
	listenPortsStartLabel = new QLabel( connectionsTab);
	ConnectionsGrid->addWidget( listenPortsStartLabel, 2, 0, Qt::AlignCenter);

	CPortStart = new QSpinBox( connectionsTab );
	CPortStart->setMaximum( 65535 );
	CPortStart->setValue( 0 );
	ConnectionsGrid->addWidget( CPortStart, 2, 1, Qt::AlignCenter);

	listenPortsEndLabel = new QLabel( connectionsTab );
	ConnectionsGrid->addWidget( listenPortsEndLabel, 2, 2, Qt::AlignCenter);

	CPortEnd = new QSpinBox( connectionsTab);
	CPortEnd->setMaximum( 65535 );
	CPortEnd->setValue( 0 );

	ConnectionsGrid->addWidget( CPortEnd, 2, 3, Qt::AlignCenter);

	ConnectionsGrid->setRowStretch(3, 10);


	// USERS Options Tab
	usersTab = new QWidget( mMuseekdTabs);
	mMuseekdTabs->addTab( usersTab, QString::fromLatin1("") );
	UsersGrid = new QGridLayout( usersTab);


	SBuddiesPrivileged = new QCheckBox( usersTab);
	UsersGrid->addWidget( SBuddiesPrivileged, 0, 0);

	SShareBuddiesOnly = new QCheckBox( usersTab );
	UsersGrid->addWidget( SShareBuddiesOnly, 1, 0);

	STrustedUsers = new QCheckBox( usersTab);
	UsersGrid->addWidget( STrustedUsers, 2, 0);

	SUserWarnings = new QCheckBox( usersTab );
	UsersGrid->addWidget( SUserWarnings, 3, 0);

	UsersGrid->setRowStretch(4, 10);
	// Museeq Appearance
	AppearanceTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( AppearanceTab, QString::fromLatin1("") );
	AppearanceGrid = new QGridLayout( AppearanceTab);

	SOnlineAlerts = new QCheckBox( AppearanceTab );
	AppearanceGrid->addWidget( SOnlineAlerts, 0, 0, 1, 2  );

	SIPLog = new QCheckBox( AppearanceTab);
	AppearanceGrid->addWidget( SIPLog, 1, 0, 1, 2  );

	IconsAlignment = new QCheckBox( AppearanceTab);
	AppearanceGrid->addWidget( IconsAlignment, 2, 0, 1, 2  );

	TickerLengthLabel = new QLabel(AppearanceTab);
	AppearanceGrid->addWidget( TickerLengthLabel, 3, 0);

	TickerLength = new QSpinBox( AppearanceTab);
	TickerLength->setMaximum( 500 );
	TickerLength->setMinimum( 20 );
	TickerLength->setValue( 20 );
	AppearanceGrid->addWidget( TickerLength, 3, 1 );
	AppearanceGrid->setColumnStretch(0, 8);
	AppearanceGrid->setRowStretch(4, 10);

	// Logging
	LoggingTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( LoggingTab, QString::fromLatin1("") );
	LoggingGrid = new QGridLayout( LoggingTab);

	LoggingPrivate = new QCheckBox( LoggingTab );
	LoggingGrid->addWidget( LoggingPrivate, 0, 0);


	LoggingPrivateDir = new QLineEdit( LoggingTab);
	LoggingGrid->addWidget( LoggingPrivateDir, 1, 0);

	LoggingPrivateButton = new QPushButton( LoggingTab);
	LoggingPrivateButton->setIcon(IMG("open"));
	LoggingGrid->addWidget( LoggingPrivateButton, 1, 1);

	LoggingRooms = new QCheckBox( LoggingTab);
	LoggingGrid->addWidget( LoggingRooms, 2, 0);

	LoggingRoomDir = new QLineEdit( LoggingTab);
	LoggingGrid->addWidget( LoggingRoomDir, 3, 0);

	LoggingRoomButton = new QPushButton( LoggingTab);
	LoggingRoomButton->setIcon(IMG("open"));
	LoggingGrid->addWidget( LoggingRoomButton, 3, 1);
	LoggingGrid->setRowStretch(4, 10);

	// Userinfo
	UserInfoTab = new QWidget( mMuseekdTabs );
	mMuseekdTabs->addTab( UserInfoTab, QString::fromLatin1("") );
	UserInfoGrid = new QGridLayout( UserInfoTab);

	mInfoText = new QTextEdit( UserInfoTab );

	UserInfoGrid->addWidget( mInfoText, 0, 0, 1, 2 );

	mClear = new QRadioButton( UserInfoTab);

	UserInfoGrid->addWidget( mClear, 3, 0 );

	mDontTouch = new QRadioButton( mClear);
	mDontTouch->setChecked( TRUE );

	UserInfoGrid->addWidget( mDontTouch, 1, 0 );

	mImage = new QLineEdit( UserInfoTab );

	UserInfoGrid->addWidget( mImage, 2, 1 );

	mUpload = new QRadioButton( mClear );

	UserInfoGrid->addWidget( mUpload, 2, 0 );

	mBrowse = new QPushButton( mClear );
	mBrowse->setIcon(IMG("open"));
	UserInfoGrid->addWidget( mBrowse, 2, 2 );
	connect( mBrowse, SIGNAL( clicked() ), this, SLOT( UserImageBrowse_clicked() ) );

// 	UserInfoGrid->addWidget( buttonGroup1, 1, 1, 0, 2 );

	// Protocol Handlers Tab
	ProtocolTab = new QWidget( mMuseeqTabs);
	mMuseeqTabs->addTab( ProtocolTab, QString::fromLatin1("") );

	ProtocolGrid = new QGridLayout( ProtocolTab);

	mProtocols = new QTreeWidget( ProtocolTab);

	QStringList ProtocolsHeaders;
	ProtocolsHeaders <<  tr("Protocol") << tr("Handler");
	mProtocols->setHeaderLabels(ProtocolsHeaders);

	mProtocols->sortItems(0, Qt::AscendingOrder);
	mProtocols->setRootIsDecorated(false);
	mProtocols->setEditTriggers(QAbstractItemView::DoubleClicked);
	mProtocols->setAllColumnsShowFocus( TRUE );

	ProtocolGrid->addWidget( mProtocols, 0, 0, 1, 4 );

	mNewHandler = new QPushButton( ProtocolTab);
	mNewHandler->setIcon(IMG("new"));
	ProtocolGrid->addWidget( mNewHandler, 2, 2 );
	protocolSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	ProtocolGrid->addItem( protocolSpacer, 2, 0 );
	mModifyHandler = new QPushButton( ProtocolTab);
	mModifyHandler->setIcon(IMG("comments"));
	ProtocolGrid->addWidget( mModifyHandler, 2, 1 );
	// Colors And Fonts
	ColorsAndFontsTab = new QWidget( mMuseeqTabs );
	mMuseeqTabs->addTab( ColorsAndFontsTab, QString::fromLatin1("") );
	ColorsGrid = new QGridLayout( ColorsAndFontsTab);
	MeColorLabel = new QLabel( ColorsAndFontsTab);
	ColorsGrid->addWidget( MeColorLabel, 0, 0 );

	SMeText = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( SMeText, 0, 1 );

	MeColorButton = new QPushButton( ColorsAndFontsTab);
	MeColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( MeColorButton, 0, 2 );

	RemoteColorLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( RemoteColorLabel, 1, 0 );

	SRemoteText = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( SRemoteText, 1, 1 );

	RemoteColorButton = new QPushButton( ColorsAndFontsTab);
	RemoteColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( RemoteColorButton, 1, 2 );

	LocalTextLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( LocalTextLabel, 2, 0 );

	SNicknameText = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( SNicknameText, 2, 1 );

	NicknameColorButton = new QPushButton( ColorsAndFontsTab );
	NicknameColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( NicknameColorButton, 2, 2 );

	BuddiedColorLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( BuddiedColorLabel, 3, 0 );

	SBuddiedText = new QLineEdit( ColorsAndFontsTab);
	ColorsGrid->addWidget( SBuddiedText, 3, 1 );

	BuddiedColorButton = new QPushButton( ColorsAndFontsTab );
	BuddiedColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( BuddiedColorButton, 3, 2 );

	BannedColorLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( BannedColorLabel, 4, 0 );

	SBannedText = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( SBannedText, 4, 1 );

	BannedColorButton = new QPushButton( ColorsAndFontsTab );
	BannedColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( BannedColorButton, 4, 2 );

	TrustColorLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( TrustColorLabel, 5, 0 );

	STrustedText = new QLineEdit( ColorsAndFontsTab);
	ColorsGrid->addWidget( STrustedText, 5, 1 );

	TrustColorButton = new QPushButton( ColorsAndFontsTab);
	TrustColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( TrustColorButton, 5, 2 );

	TimeColorLabel = new QLabel( ColorsAndFontsTab);
	TimeColorLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeColorLabel, 6, 0 );

	STimeText = new QLineEdit( ColorsAndFontsTab);
	ColorsGrid->addWidget( STimeText, 6, 1 );

	TimeColorButton = new QPushButton( ColorsAndFontsTab );
	TimeColorButton->setIcon(IMG("colorpicker"));
	ColorsGrid->addWidget( TimeColorButton, 6, 2 );

	TimeFontLabel = new QLabel( ColorsAndFontsTab);
	TimeFontLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeFontLabel, 7, 0 );

	STimeFont = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( STimeFont, 7, 1 );

	TimeFontButton = new QPushButton( ColorsAndFontsTab);
	TimeFontButton->setIcon(IMG("font"));
	ColorsGrid->addWidget( TimeFontButton, 7, 2 );

	MessageFontLabel = new QLabel( ColorsAndFontsTab );
	ColorsGrid->addWidget( MessageFontLabel, 8, 0 );

	SMessageFont = new QLineEdit( ColorsAndFontsTab );
	ColorsGrid->addWidget( SMessageFont, 8, 1 );

	MessageFontButton = new QPushButton( ColorsAndFontsTab );
	MessageFontButton->setIcon(IMG("font"));
	ColorsGrid->addWidget( MessageFontButton, 8, 2 );

	connect( MeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_me() ) );
	connect( BannedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_banned() ) );
	connect( BuddiedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_buddied() ) );
	connect( MessageFontButton, SIGNAL( clicked() ), this, SLOT( font_text_message() ) );
	connect( NicknameColorButton, SIGNAL( clicked() ), this, SLOT( color_text_nickname() ) );
	connect( RemoteColorButton, SIGNAL( clicked() ), this, SLOT( color_text_remote() ) );
	connect( TimeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_time() ) );
	connect( TrustColorButton, SIGNAL( clicked() ), this, SLOT( color_text_trusted() ) );
	connect( TimeFontButton, SIGNAL( clicked() ), this, SLOT( font_text_time() ) );

	// Translate
	languageChange();
	resize( QSize(450, 600).expandedTo(minimumSizeHint()) );


	// signals and slots connections
	connect( mOK, SIGNAL( clicked() ), this, SLOT( acceptSettings() ) );
	connect( mSave, SIGNAL( clicked() ), this, SLOT( save() ) );
	connect( mCancel, SIGNAL( clicked() ), this, SLOT( rejectSettings() ) );
	connect( SConnect, SIGNAL( clicked() ), this, SLOT( SConnect_clicked() ) );
	connect( SDisconnect, SIGNAL( clicked() ), this, SLOT( SDisconnect_clicked() ) );
	connect( SDownloadButton, SIGNAL( clicked() ), this, SLOT( SDownload_clicked() ) );
	connect( SIncompleteButton, SIGNAL( clicked() ), this, SLOT( SIncomplete_clicked() ) );

	connect( SConfigButton, SIGNAL( clicked() ), this, SLOT( SConfig_clicked() ) );
	connect( SBuddiesShares, SIGNAL( toggled(bool) ), SLOT( SBuddiesSharesToggled(bool) ) );

	connect( NSharesRefresh, SIGNAL( clicked() ), this, SLOT( NormalSharesRefresh() ) );
	connect( NSharesUpdate, SIGNAL( clicked() ), this, SLOT( NormalSharesUpdate() ) );
	connect( NSharesRescan, SIGNAL( clicked() ), this, SLOT( NormalSharesRescan() ) );
	connect( NSharesAdd, SIGNAL( clicked() ), this, SLOT( NormalSharesAdd() ) );
	connect( NSharesRemove, SIGNAL( clicked() ), this, SLOT( NormalSharesRemove() ) );


	connect( BSharesRefresh, SIGNAL( clicked() ), this, SLOT( BuddySharesRefresh() ) );
	SBuddiesSharesToggled(false);
	connect( BSharesUpdate, SIGNAL( clicked() ), this, SLOT( BuddySharesUpdate() ) );
	connect( BSharesRescan, SIGNAL( clicked() ), this, SLOT( BuddySharesRescan() ) );
	connect( BSharesAdd, SIGNAL( clicked() ), this, SLOT( BuddySharesAdd() ) );
	connect( BSharesRemove, SIGNAL( clicked() ), this, SLOT( BuddySharesRemove() ) );

	connect( LoggingPrivateButton, SIGNAL( clicked() ), this, SLOT( PrivateDirSelect() ) );
	connect( LoggingRoomButton, SIGNAL( clicked() ), this, SLOT( RoomDirSelect() ) );
	// Protocol treewidget signals
	mProtocols->setContextMenuPolicy(Qt::CustomContextMenu);
	mProtocolsMenu = new QMenu(this);

	ActionDeleteHandler = new QAction(IMG("remove"),tr("Delete handler"), this);
	connect(ActionDeleteHandler, SIGNAL(triggered()), this, SLOT(mProtocols_itemDelete()));
	mProtocolsMenu->addAction(ActionDeleteHandler);

	connect(mProtocols, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotProtocolContextMenu(const QPoint&)));
	connect( mNewHandler, SIGNAL( clicked() ), this, SLOT( mNewHandler_clicked() ) );
	connect( mModifyHandler, SIGNAL( clicked() ), this, SLOT( mModifyHandler_clicked() ) );
// 	connect( mProtocols, SIGNAL( itemRenamed(QTreeWidgetItem*,int) ), this, SLOT( mProtocols_itemRenamed(QTreeWidgetItem*,int) ) );

	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

}

void SettingsDialog::loadSettings() {
    // museekd settings
    SServerHost->setText(museeq->config("server", "host"));
    SSoulseekUsername->setText(museeq->config("server", "username"));
    SSoulseekPassword->setText(museeq->config("server", "password"));
    SServerPort->setValue(museeq->config("server", "port").toInt());
	SDownDir->setText(museeq->config("transfers", "download-dir"));
    SIncompleteDir->setText(museeq->config("transfers", "incomplete-dir"));
	mInfoText->setText(museeq->config("userinfo", "text"));
    CPortStart->setValue(museeq->config("clients.bind", "first").toInt());
    CPortEnd->setValue(museeq->config("clients.bind", "last").toInt());
    SConfigFile->setText(QString (museeq->config("shares", "database")).replace(".shares", ".xml"));
    NormalSharesRefresh();
    BuddySharesRefresh();

    SBuddiesShares->setChecked(museeq->config("transfers", "have_buddy_shares") == "true");
    SShareBuddiesOnly->setChecked(museeq->config("transfers", "only_buddies") == "true");
    SBuddiesPrivileged->setChecked(museeq->config("transfers", "privilege_buddies") == "true");
    STrustedUsers->setChecked(museeq->config("transfers", "trusting_uploads") == "true");
    SUserWarnings->setChecked(museeq->config("transfers", "user_warnings") == "true");
    SActive->setChecked(museeq->config("clients", "connectmode") == "active");

    // museeq settings
	IconsAlignment->setChecked(museeq->settings()->value("VerticalIconBox").toBool());
	SMessageFont->setText(museeq->mFontMessage);
	STimeFont->setText(museeq->mFontTime);
	STimeText->setText(museeq->mColorTime);
	SRemoteText->setText(museeq->mColorRemote);
	SMeText->setText(museeq->mColorMe);
	SNicknameText->setText(museeq->mColorNickname);
	SBuddiedText->setText(museeq->mColorBuddied);
	SBannedText->setText(museeq->mColorBanned);
	STrustedText->setText(museeq->mColorTrusted);
	LoggingRoomDir->setText(museeq->mRoomLogDir);
	LoggingPrivateDir->setText(museeq->mPrivateLogDir);
	LoggingPrivate->setChecked(museeq->mLogPrivate);
	LoggingRooms->setChecked(museeq->mLogRooms);
	SOnlineAlerts->setChecked(museeq->mOnlineAlert);
	SIPLog->setChecked(museeq->mIPLog);
	TickerLength->setValue(museeq->mTickerLength);
}

void SettingsDialog::acceptSettings() {
    save();
    hide();
}

void SettingsDialog::rejectSettings() {
    loadSettings();
    hide();
}

void SettingsDialog::slotConfigChanged(const QString& domain, const QString& key, const QString& value) {
	if(domain == "server" && key == "host") {
		SServerHost->setText(value);
	} else if(domain == "server" && key == "username") {
		SSoulseekUsername->setText(value);
	} else if(domain == "server" && key == "password") {
		SSoulseekPassword->setText(value);
	} else if(domain == "server" && key == "port") {
		SServerPort->setValue(value.toInt());
	} else if(domain == "transfers" && key == "have_buddy_shares") {
		if  (value == "true")  { SBuddiesShares->setChecked(true); }
		else if (value == "false") { SBuddiesShares->setChecked(false); }
	} else if(domain == "shares" && key == "database") {
		SConfigFile->setText(QString (value).replace(".shares", ".xml"));
		NormalSharesRefresh();
		BuddySharesRefresh();
	} else if(domain == "transfers" && key == "only_buddies") {
		if  (value == "true")  { SShareBuddiesOnly->setChecked(true); }
		else if (value == "false") { SShareBuddiesOnly->setChecked(false); }
	} else if(domain == "transfers" && key == "privilege_buddies") {
		if (value == "true") SBuddiesPrivileged->setChecked(true);
		else if (value == "false") SBuddiesPrivileged->setChecked(false);
	} else if(domain == "transfers" && key == "trusting_uploads") {
		if (value == "true") STrustedUsers->setChecked(true);
		else if ( value == "false") STrustedUsers->setChecked(false);
	} else if(domain == "transfers" && key == "user_warnings") {
		if (value == "true") SUserWarnings->setChecked(true);
		else if ( value == "false") SUserWarnings->setChecked(false);
	} else if(domain == "transfers" && key == "download-dir") {
		SDownDir->setText(value);
	} else if(domain == "transfers" && key == "incomplete-dir") {
		SIncompleteDir->setText(value);

	} else if(domain == "clients" && key == "connectmode") {
		if (value == "active") SActive->setChecked(true);
		else if (value == "passive") SPassive->setChecked(true);

	} else if (domain == "clients.bind") {
		if (key =="first") {
			CPortStart->setValue(value.toInt());
		} else if (key == "last") {
			CPortEnd->setValue(value.toInt());
		}
	} else if(domain == "userinfo" && key == "text") {
		mInfoText->setText(value);

	}

}

void SettingsDialog::SBuddiesSharesToggled(bool on) {
	ListBuddyShares->setEnabled(on);
	EnableBuddyButtons(on);
    mSharesDirty = true;
}

void SettingsDialog::NormalSharesRefresh() {
	ListNormalShares->clear();
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(readyReadStandardOutput()), this, SLOT(readNormal()) );
	connect( proc1, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(finishedListNormal( int, QProcess::ExitStatus)) );
	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-l" );

	proc1->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesRefresh() {
	ListBuddyShares->clear();
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(readyReadStandardOutput()), this, SLOT(readBuddy()) );
	connect( proc2, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(finishedListBuddy( int, QProcess::ExitStatus)) );
	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-l" );
	arguments.append("-b" );

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesAdd() {
	QFileDialog * fd = new QFileDialog(this, QDir::homePath());
	fd->setFileMode(QFileDialog::Directory);
	fd->setWindowTitle(tr("Select a Directory to add to your Buddy Shares."));
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{

		EnableBuddyButtons(false);
		proc2 = new QProcess( this );
		connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy( int, QProcess::ExitStatus)) );

		QStringList arguments ;
		arguments.append("-c");
		arguments.append( SConfigFile->text() );
		arguments.append("-b" );
		arguments.append("-s" );
		arguments.append(fd->selectedFiles().at(0) );
		proc2->start( "muscan", arguments );

	}
	delete fd;
    mSharesDirty = true;
}


void SettingsDialog::PrivateDirSelect() {
	QString path = LoggingPrivateDir->text();
	if (path.isEmpty())
		path = QDir::homePath();
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to write Private Chat log files."), path);
	fd->setFileMode(QFileDialog::Directory);
// 	fd->setShowHiddenFiles(true);
	fd->setViewMode(QFileDialog::Detail);
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		LoggingPrivateDir->setText(fd->selectedFiles().at(0));

	}
	delete fd;
}

void SettingsDialog::RoomDirSelect() {

	QString  path = LoggingRoomDir->text();
	if (path.isEmpty())
		path = QDir::homePath();
	QFileDialog * fd = new QFileDialog(this, tr("Select a Directory to write Chat Room log files."), path);
	fd->setFileMode(QFileDialog::Directory);
	fd->setViewMode(QFileDialog::Detail);
// 	fd->setShowHiddenFiles(true);

	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		LoggingRoomDir->setText(fd->selectedFiles().at(0));

	}
	delete fd;
}

void SettingsDialog::BuddySharesRescan() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-b");
	arguments.append("-r");

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesUpdate() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-b" );

	proc2->start( "muscan", arguments );
    mSharesDirty = true;
}

void SettingsDialog::NormalSharesAdd() {
	QFileDialog * fd = new QFileDialog(this, QDir::homePath());
	fd->setFileMode(QFileDialog::Directory);
	fd->setWindowTitle(tr("Select a Directory to add to your Normal Shares."));
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		EnableNormalButtons(false);
		proc1 = new QProcess( this );
		connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );

		QStringList arguments ;
		arguments.append("-c");
		arguments.append( SConfigFile->text() );
		arguments.append("-s" );
		arguments.append(fd->selectedFiles().at(0) );
		proc1->start( "muscan", arguments );

	}
	delete fd;
    mSharesDirty = true;
}

void SettingsDialog::BuddySharesRemove() {
	QTreeWidgetItem* item = ListBuddyShares->currentItem();
	if (! item ||  item->text(0).isEmpty())
		return;
	QString directory (item->text(0));

	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedBuddy(int, QProcess::ExitStatus)) );

	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-b" );
	arguments.append("-u" );
	arguments.append(directory );
	proc2->start( "muscan", arguments );

// 		EnableBuddyButtons(true);
    mSharesDirty = true;

}

void SettingsDialog::NormalSharesRemove() {
	QTreeWidgetItem* item = ListNormalShares->currentItem();
	if (! item ||  item->text(0).isEmpty())
		return;

	QString directory (item->text(0));

	EnableNormalButtons(false);
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-u" );
	arguments.append(directory );
	proc1->start( "muscan", arguments );
// 		EnableNormalButtons(true);

    mSharesDirty = true;
}

void SettingsDialog::NormalSharesRescan() {
	EnableNormalButtons(false);

	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );
	arguments.append("-r" );

	proc1->start( "muscan", arguments );

// 		EnableNormalButtons(true);
    mSharesDirty = true;

}
void SettingsDialog::NormalSharesUpdate() {
	EnableNormalButtons(false);

	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finishedNormal(int, QProcess::ExitStatus)) );
	QStringList arguments ;
	arguments.append("-c");
	arguments.append( SConfigFile->text() );

	proc1->start( "muscan", arguments );

// 		EnableNormalButtons(true);
    mSharesDirty = true;
}


void SettingsDialog::EnableBuddyButtons(bool on) {
	BSharesRefresh->setEnabled(on);
	BSharesAdd->setEnabled(on);
	BSharesRemove->setEnabled(on);
	BSharesRescan->setEnabled(on);
	BSharesUpdate->setEnabled(on);
}
void SettingsDialog::EnableNormalButtons(bool on) {
	NSharesRefresh->setEnabled(on);
	NSharesAdd->setEnabled(on);
	NSharesRemove->setEnabled(on);
	NSharesRescan->setEnabled(on);
	NSharesUpdate->setEnabled(on);
}

void SettingsDialog::readNormal() {
	while (proc1->bytesAvailable()) {
		QString output = proc1->readLine();
		if (! output.isEmpty() ) {
			QTreeWidgetItem * item = new QTreeWidgetItem(ListNormalShares);
			item->setText(0, output.replace("\n", ""));
		}
	}
}

void SettingsDialog::readBuddy() {
	while (proc2->bytesAvailable()) {
		QString output = proc2->readLine();
		if (! output.isEmpty() ) {
			QTreeWidgetItem * item = new QTreeWidgetItem(ListBuddyShares);
			item->setText(0, output.replace("\n", ""));
		}
	}
}
void SettingsDialog::finishedListNormal( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableNormalButtons(true);
}
void SettingsDialog::finishedListBuddy( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableBuddyButtons(SBuddiesShares->isChecked());
}
void SettingsDialog::finishedNormal( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableNormalButtons(true);
	NormalSharesRefresh();
}
void SettingsDialog::finishedBuddy( int exitCode, QProcess::ExitStatus exitStatus) {
	EnableBuddyButtons(SBuddiesShares->isChecked());
	BuddySharesRefresh();
}

void SettingsDialog::SConnect_clicked()
{
    museeq->connectServer();
}

void SettingsDialog::SDisconnect_clicked()
{
    museeq->disconnectServer();
}

void SettingsDialog::save()
{
    museeq->saveSettings();
}

void SettingsDialog::SDownload_clicked()
{
    QFileDialog * fd = new QFileDialog(this, QDir::homePath());
    fd->setFileMode(QFileDialog::Directory);
    fd->setWindowTitle(tr("Select a Directory to store your downloaded files."));
    fd->setFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
    {
        SDownDir->setText( fd->selectedFiles().at(0));
    }

    delete fd;
}

void SettingsDialog::SConfig_clicked()
{
	QFileDialog * fd = new QFileDialog(this, QDir::homePath());
	fd->setFileMode(QFileDialog::ExistingFile );
// 	fd->setShowHiddenFiles(true);
	fd->setWindowTitle(tr("Select the museekd config file."));
	fd->setFilter(tr("XML files (*.xml)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		SConfigFile->setText( fd->selectedFiles().at(0));
	}

	delete fd;
}

void SettingsDialog::SIncomplete_clicked()
{
    QFileDialog * fd = new QFileDialog(this, QDir::homePath());
    fd->setFileMode(QFileDialog::Directory);
    fd->setWindowTitle(tr("Select a Directory to store your incomplete downloading files."));
    fd->setFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
    {
	SIncompleteDir->setText( fd->selectedFiles().at(0));
    }

    delete fd;
}

void SettingsDialog::UserImageBrowse_clicked()
{
	QFileDialog * fd = new QFileDialog(this, QDir::homePath(), tr("Images (*.png *.gif *.jpg *.jpeg)"));
	fd->setFileMode(QFileDialog::ExistingFile);
	fd->setWindowTitle(tr("Select an image for your User info"));
	fd->setFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
	{
		mImage->setText(fd->selectedFiles().at(0));
		mUpload->setChecked(true);
	}

	delete fd;
}

void SettingsDialog::mNewHandler_clicked()
{
	QTreeWidgetItem* item = new QTreeWidgetItem(mProtocols);
	item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled );
	mProtocols->editItem(item, 0);
}

void SettingsDialog::mModifyHandler_clicked()
{
	QTreeWidgetItem* item = mProtocols->currentItem();
	if (item) {
		mProtocols->editItem(item, 1);
	}
}
void SettingsDialog::slotProtocolContextMenu(const QPoint& pos) {
	QTreeWidgetItem * item = mProtocols->itemAt(pos);
	if(! item)
		return;
	mProtocolsMenu->exec(mProtocols->mapToGlobal(pos));
}

void SettingsDialog::mProtocols_itemDelete() {
	QTreeWidgetItem * item = mProtocols->currentItem();
	if (! item)
		return;
	delete item;
}


void SettingsDialog::color_text_me()
{
   QColor c = QColorDialog::getColor( SMeText->text(), this );
    if ( c.isValid() )
	SMeText->setText(c.name());
}


void SettingsDialog::color_text_buddied()
{
   QColor c = QColorDialog::getColor( SBuddiedText->text(), this );
    if ( c.isValid() )
	SBuddiedText->setText(c.name());
}


void SettingsDialog::color_text_nickname()
{
   QColor c = QColorDialog::getColor( SNicknameText->text(), this );
    if ( c.isValid() )
	SNicknameText->setText(c.name());
}


void SettingsDialog::color_text_banned()
{
   QColor c = QColorDialog::getColor( SBannedText->text(), this );
    if ( c.isValid() )
	SBannedText->setText(c.name());
}




void SettingsDialog::color_text_remote()
{
   QColor c = QColorDialog::getColor( SRemoteText->text(), this );
    if ( c.isValid() )
	SRemoteText->setText(c.name());
}


void SettingsDialog::color_text_time()
{
   QColor c = QColorDialog::getColor( STimeText->text(), this );
    if ( c.isValid() )
	STimeText->setText(c.name());
}

void SettingsDialog::color_text_trusted()
{
   QColor c = QColorDialog::getColor( STrustedText->text(), this );
    if ( c.isValid() )
	STrustedText->setText(c.name());
}

void SettingsDialog::font_text_time()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, STimeFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		STimeFont->setText(c);

    }
}


void SettingsDialog::font_text_message()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, SMessageFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		SMessageFont->setText(c);

    }
}
/*
 *  Destroys the object and frees any allocated resources
 */
SettingsDialog::~SettingsDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SettingsDialog::languageChange()
{
	setWindowTitle( tr( "Museeq Settings" ) );
	mOK->setText( tr( "Ok" ) );
	mSave->setText( tr( "Save" ) );
	mCancel->setText( tr( "Cancel" ) );
	configLabel->setText( tr( "Museekd Config:" ) );
	SConfigButton->setText( tr( "Select.." ) );
	NSharesRefresh->setText( tr( "Refresh list" ) );
	NSharesUpdate->setText( tr( "Update" ) );
	NSharesRescan->setText( tr( "Rescan" ) );
	NSharesAdd->setText( tr( "Add" ) );
	NSharesRemove->setText( tr( "Remove" ) );

	BSharesRefresh->setText( tr( "Refresh list" ) );
	BSharesUpdate->setText( tr( "Update" ) );
	BSharesRescan->setText( tr( "Rescan" ) );
	BSharesAdd->setText( tr( "Add" ) );
	BSharesRemove->setText( tr( "Remove" ) );
	listenPortsLabel->setText( tr( "Listen port range (the first available port will be used when museekd starts up)" ) );
	listenPortsStartLabel->setText( tr( "First port:" ) );
	listenPortsEndLabel->setText( tr( "Last port:" ) );


// 	buttonGroup1->setTitle( tr( "Image" ) );
	mClear->setText( tr( "Clear" ) );
	mDontTouch->setText( tr( "Don't touch" ) );
	mUpload->setText( tr( "Upload:" ) );
	mBrowse->setText( tr( "Select.." ) );


	LoggingPrivate->setText( tr( "Log Private Chats" ) );
	LoggingRooms->setText( tr( "Log Chat Rooms" ) );
	LoggingPrivateButton->setText( tr( "Select.." ) );
	LoggingRoomButton->setText( tr( "Select.." ) );

	fEncodingLabel->setText( tr( "Filesystem Encoding:" ) );
	nEncodingLabel->setText( tr( "Network Encoding:" ) );
	SConnect->setText( tr( "Connect" ) );
	SDisconnect->setText( tr( "Disconnect" ) );
	serverPortLabel->setText( tr( "Server Port:" ) );
	serverHostLabel->setText( tr( "Server Host:" ) );
	usernamelabel->setText( tr( "Soulseek Username:" ) );
	passwordLabel->setText( tr( "Soulseek Password:" ) );
	SSoulseekPassword->setInputMask( QString::null );


// 	mProtocols->header()->setLabel( 0, tr( "Protocol" ) );
// 	mProtocols->header()->setLabel( 1, tr( "Handler" ) );
	mNewHandler->setText( tr( "New" ) );
	mModifyHandler->setText( tr( "Modify" ) );
	// Museekd Tabs
	mTabHolder->setTabText( mTabHolder->indexOf(mMuseekdTabs), tr( "Museek Daemon" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(serverTab), tr( "Server" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(sharesTab), tr( "Shares" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(connectionsTab), tr( "Connections" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(usersTab), tr( "User Options" ) );
	mMuseekdTabs->setTabText( mMuseekdTabs->indexOf(UserInfoTab), tr( "User Info" ) );
	// Museeq tabs
	mTabHolder->setTabText( mTabHolder->indexOf(mMuseeqTabs), tr( "Museeq" ) );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(AppearanceTab), tr("Appearance") );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(ColorsAndFontsTab), tr("Fonts and Colors") );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(LoggingTab), tr( "Logging" ) );
	mMuseeqTabs->setTabText( mMuseeqTabs->indexOf(ProtocolTab), tr( "Protocol handlers" ) );
	// Fonts and Colors

	TimeFontLabel->setText( tr( "Time & Brackets Font" ) );
	TimeFontButton->setText( tr( "Pick Font" ) );
	TimeColorLabel->setText( tr( "Time & Brackets Text Color" ) );
	MeColorLabel->setText( tr( "/Me Text" ) );
	MeColorButton->setText( tr( "Pick Color" ) );
	NicknameColorButton->setText( tr( "Pick Color" ) );
	TimeColorButton->setText( tr( "Pick Color" ) );
	MessageFontLabel->setText( tr( "Message Font" ) );
	MessageFontButton->setText( tr( "Pick Font" ) );
	BuddiedColorLabel->setText( tr( "Buddied Users" ) );
	BuddiedColorButton->setText( tr( "Pick Color" ) );
	LocalTextLabel->setText( tr( "My Text" ) );
	TrustColorLabel->setText( tr( "Trusted Users" ) );
	TrustColorButton->setText( tr( "Pick Color" ) );
	BannedColorLabel->setText( tr( "Banned Users" ) );
	BannedColorButton->setText( tr( "Pick Color" ) );
	RemoteColorLabel->setText( tr( "Remote Text" ) );
	RemoteColorButton->setText( tr( "Pick Color" ) );



	// Connections and Ports
// 	buttonGroup2->setTitle( tr( "Connections" ) );
	SActive->setText( tr( "Active Connections" ) );
	SPassive->setText( tr( "Passive Connections" ) );
	SDownloadButton->setText( tr( "Select.." ) );
	downloadLabel->setText( tr( "Download Dir:" ) );
	SIncompleteButton->setText( tr( "Select.." ) );
	incompleteLabel->setText( tr( "Incomplete Dir:" ) );


	SBuddiesPrivileged->setText( tr( "Buddies are Privileged" ) );
	SOnlineAlerts->setText( tr( "Online Alerts in Log Window instead of popup" ) );
	SShareBuddiesOnly->setText( tr( "Share to Buddies Only" ) );
	STrustedUsers->setText( tr( "Trusted users can Send you Files" ) );
	SBuddiesShares->setText( tr( "Seperate Shares list for Buddies" ) );
	SUserWarnings->setText( tr( "Send automatic warnings to users via Private Chat" ) );
	SIPLog->setText( tr( "IP addresses in Log Window instead of popup" ) );
	TickerLengthLabel->setText( tr( "Maximum length of ticker messages:" ) );
	IconsAlignment->setText( tr( "Align Mode Icons Vertically" ) );
}

void SettingsDialog::closeEvent(QCloseEvent * ev) {
    rejectSettings();
    ev->ignore();
}
