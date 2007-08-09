
#include "settingsdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qlistview.h>
#include <qheader.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include <qfontdialog.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "museeq.h"
#include <qfiledialog.h>
#include <qdir.h>
#include <qsplitter.h>
#include "codeccombo.h"
#include "images.h"

#include <qprocess.h>
#include <qtooltip.h>

SettingsDialog::SettingsDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
	if ( !name )
		setName( "SettingsDialog" );

	// Layout Containing everything 
	QVBoxLayout* vLayout= new QVBoxLayout( this, 11, 6, "vLayout"); 
	vLayout->setMargin(5);
	vLayout->setSpacing(5);
	
	// Tabs, without geometry set will resize to fill the dialog
	mTabHolder = new QTabWidget( this, "mTabHolder" );

	// Add tabs to vLayout
	vLayout->addWidget(mTabHolder);

	QHBoxLayout* buttonsLayout= new QHBoxLayout; 
	
	// Ok, Save, Cancel buttons
	QSpacerItem* spacer5 = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonsLayout->addItem( spacer5 );

	mOK = new QPushButton( this, "mOK" );
	mOK->setIconSet(IMG("ok"));
	buttonsLayout->addWidget(mOK);

	mSave = new QPushButton( this, "mSave" );
	mSave->setIconSet(IMG("save"));
	buttonsLayout->addWidget(mSave);

	mCancel = new QPushButton( this , "mCancel" );
	mCancel->setIconSet(IMG("cancel"));
	mCancel->setDefault( TRUE );
	buttonsLayout->addWidget(mCancel);

	// Add buttons to vLayout
	vLayout->addLayout(buttonsLayout);

	mTabHolder->setTabPosition( QTabWidget::Top );
	mMuseekdTabs = new QTabWidget( mTabHolder, "mMuseekdTabs" );
	
	
	mTabHolder->insertTab( mMuseekdTabs, QString::fromLatin1("") );
	mMuseeqTabs = new QTabWidget( this, "mMuseeqTabs" );
	mTabHolder->insertTab( mMuseeqTabs, QString::fromLatin1("") );
	// SERVER TAB
	serverTab = new QWidget( mMuseekdTabs, "serverTab" );
	mMuseekdTabs->insertTab( serverTab, QString::fromLatin1("") );

	ServerGrid = new QGridLayout( serverTab, 1, 1, 11, 6, "ServerGrid");


	// Server Host
	serverHostLabel = new QLabel( serverTab, "serverHostLabel" );
	ServerGrid->addWidget( serverHostLabel, 0, 0 );


	SServerHost = new QLineEdit( serverTab, "SServerHost" );
	SServerHost->setMaxLength( 50 );
	ServerGrid->addMultiCellWidget( SServerHost, 0, 0, 1, 2 );

	// Server Port
	serverPortLabel = new QLabel( serverTab, "serverPortLabel" );
	serverPortLabel->setMargin( 0 );
	
	ServerGrid->addWidget( serverPortLabel, 1, 0 );
	SServerPort = new QSpinBox( serverTab, "SServerPort" );
	SServerPort->setMaxValue( 65535 );
	SServerPort->setValue( 0 );
	
	ServerGrid->addMultiCellWidget( SServerPort, 1, 1, 2, 2 );
	// Server Username
	usernamelabel = new QLabel( serverTab, "usernamelabel" );
	ServerGrid->addWidget( usernamelabel, 2, 0 );
	
	SSoulseekUsername = new QLineEdit( serverTab, "SSoulseekUsername" );
	ServerGrid->addMultiCellWidget( SSoulseekUsername, 2, 2, 1, 2 );
	// Server Password	
	passwordLabel = new QLabel( serverTab, "passwordLabel"  );
	ServerGrid->addWidget( passwordLabel, 3, 0 );

	SSoulseekPassword = new QLineEdit( serverTab, "SSoulseekPassword" );
	SSoulseekPassword->setEchoMode(QLineEdit::Password);
	ServerGrid->addMultiCellWidget( SSoulseekPassword, 3, 3, 1, 2 );

	// Connect / Disconnect
	SConnect = new QPushButton( serverTab, "SConnect" );
	SConnect->setIconSet(IMG("connect"));
	ServerGrid->addWidget( SConnect, 4, 2 );

	SDisconnect = new QPushButton( serverTab, "SDisconnect" );
	SDisconnect->setIconSet(IMG("disconnect"));
	ServerGrid->addWidget( SDisconnect, 5, 2 );
	// Filesystem encoding
	fEncodingLabel = new QLabel( serverTab, "fEncodingLabel" );
	ServerGrid->addWidget( fEncodingLabel, 6, 0 );
	SFileSystemEncoding =  new CodecCombo("encoding", "filesystem", serverTab, "encoding");
	ServerGrid->addMultiCellWidget( SFileSystemEncoding, 6, 6 , 2, 2 );
	// Network Encoding
	nEncodingLabel = new QLabel( serverTab, "nEncodingLabel" );
	ServerGrid->addWidget( nEncodingLabel, 7, 0 );
	SNetworkEncoding =  new CodecCombo("encoding", "network", serverTab, "encoding");
	ServerGrid->addMultiCellWidget( SNetworkEncoding, 7, 7, 2, 2 );


	configLabel = new QLabel( serverTab, "configLabel" );
	ServerGrid->addWidget( configLabel, 8, 0 );
	
	SConfigFile = new QLineEdit( serverTab, "SConfigFile" );
	ServerGrid->addWidget( SConfigFile, 8, 1);
	
	SConfigButton = new QPushButton( serverTab, "SConfigButton" );
	SConfigButton->setIconSet(IMG("open"));
	ServerGrid->addWidget( SConfigButton, 8, 2);
	ServerGrid->setRowStretch(9, 10);
		
	

	// SHARES TAB
	sharesTab = new QWidget( mMuseekdTabs, "sharesTab" );
	mMuseekdTabs->insertTab( sharesTab, QString::fromLatin1("") );
	SharesGrid = new QGridLayout( sharesTab, 1, 1, 5, 5, "SharesGrid");

	SReloadShares = new QPushButton( sharesTab, "SReloadShares" );
	SReloadShares->setIconSet(IMG("rescan"));
	QToolTip::add(  SReloadShares, tr("To configure your shares, use one of the following: musetup, musetup-gtk, muscan (in a terminal), or the buttons below. Press <u>Reload Shares</u> to activate your changes.") );
	SharesGrid->addWidget( SReloadShares, 0, 2 );

	downloadLabel = new QLabel( sharesTab, "downloadLabel"  , Qt::WordBreak);
	downloadLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( downloadLabel, 4, 0 );
	
	SDownDir = new QLineEdit( sharesTab, "SDownDir" );
	SharesGrid->addWidget( SDownDir, 4, 1);
	
	SDownloadButton = new QPushButton( sharesTab, "SDownloadButton" );
	SDownloadButton->setIconSet(IMG("open"));
	SharesGrid->addWidget( SDownloadButton, 4, 2);

	incompleteLabel = new QLabel( sharesTab, "incompleteLabel" , Qt::WordBreak);
	incompleteLabel->setTextFormat(Qt::RichText);
	SharesGrid->addWidget( incompleteLabel, 5, 0);
	
	SIncompleteDir = new QLineEdit( sharesTab, "SIncompleteDir" );
	SharesGrid->addWidget( SIncompleteDir, 5, 1);
	
	SIncompleteButton = new QPushButton( sharesTab, "SIncompleteButton" );
	SIncompleteButton->setIconSet(IMG("open"));
	SharesGrid->addWidget( SIncompleteButton, 5, 2);

	
	// Shares List
	ListNormalShares = new QListView(sharesTab, "ListNormalShares");
	ListNormalShares->addColumn(tr("Directories"), -1);
	SharesGrid->addMultiCellWidget( ListNormalShares, 7, 7, 0, 1);
	// Shares List Buttons
	QVBoxLayout* sharesListButtons = new QVBoxLayout( 0, 0, 0, "sharesListButtons" );
	SharesGrid->addLayout(sharesListButtons, 7, 2);

	NSharesRefresh = new QPushButton( sharesTab, "NSharesRefresh" );
	NSharesRefresh->setIconSet( IMG("reload"));
	sharesListButtons->addWidget( NSharesRefresh);
	NSharesUpdate = new QPushButton( sharesTab, "NSharesUpdate" );
	NSharesUpdate->setIconSet(IMG("redo"));
	sharesListButtons->addWidget( NSharesUpdate);
	NSharesRescan = new QPushButton( sharesTab, "NSharesRescan" );
	NSharesRescan->setIconSet(IMG("rescan"));
	sharesListButtons->addWidget( NSharesRescan);
	NSharesAdd = new QPushButton( sharesTab, "NSharesAdd" );
	NSharesAdd->setIconSet(IMG("add"));
	sharesListButtons->addWidget( NSharesAdd);
	NSharesRemove = new QPushButton( sharesTab, "NSharesRemove" );
	NSharesRemove->setIconSet(IMG("remove"));
	sharesListButtons->addWidget( NSharesRemove);
		
	// Buddy Shares List

	SBuddiesShares = new QCheckBox( sharesTab, "SBuddiesShares" );
	SharesGrid->addMultiCellWidget( SBuddiesShares, 8, 8, 0, 2 );
		
	ListBuddyShares = new QListView(sharesTab, "ListBuddyShares");
	ListBuddyShares->addColumn(tr("Directories"), -1);
	SharesGrid->addMultiCellWidget( ListBuddyShares, 9, 9, 0, 1);
	// Buddy Shares List Buttons
	QVBoxLayout* sharesBuddyListButtons = new QVBoxLayout( 0, 0, 0, "sharesListButtons" );
	SharesGrid->addLayout(sharesBuddyListButtons, 9, 2);
	SharesGrid->setRowStretch(7, 2);
	SharesGrid->setRowStretch(9, 2);
// 	SharesGrid->setRowStretch(10, 1);
	BSharesRefresh = new QPushButton( sharesTab, "NSharesRefresh" );
	BSharesRefresh->setIconSet( IMG("reload"));
	sharesBuddyListButtons->addWidget( BSharesRefresh);
	BSharesUpdate = new QPushButton( sharesTab, "BSharesUpdate" );
	BSharesUpdate->setIconSet(IMG("redo"));
	sharesBuddyListButtons->addWidget( BSharesUpdate);
	BSharesRescan = new QPushButton( sharesTab, "BSharesRescan" );
	BSharesRescan->setIconSet(IMG("rescan"));
	sharesBuddyListButtons->addWidget( BSharesRescan);
	BSharesAdd = new QPushButton( sharesTab, "BSharesAdd" );
	BSharesAdd->setIconSet(IMG("add"));
	sharesBuddyListButtons->addWidget( BSharesAdd);
	BSharesRemove = new QPushButton( sharesTab, "BSharesRemove" );
	BSharesRemove->setIconSet(IMG("remove"));
	sharesBuddyListButtons->addWidget( BSharesRemove);
		
	

	// Connections Tab
	connectionsTab = new QWidget( mMuseekdTabs, "connectionsTab" );
	mMuseekdTabs->insertTab( connectionsTab, QString::fromLatin1("") );
	ConnectionsGrid = new QGridLayout( connectionsTab, 1, 1, 5, 5, "ConnectionsGrid");
	buttonGroup2 = new QHButtonGroup( connectionsTab, "buttonGroup2" );
	
	SActive = new QRadioButton( buttonGroup2, "SActive" );
	SPassive = new QRadioButton( buttonGroup2, "SPassive" );

	ConnectionsGrid->addMultiCellWidget(buttonGroup2, 0, 0, 0, 3);

	listenPortsLabel = new QLabel( connectionsTab, "listenPortsLabel" , Qt::WordBreak);
	listenPortsLabel->setTextFormat(Qt::RichText);
	ConnectionsGrid->addMultiCellWidget( listenPortsLabel, 1, 1, 0, 3);
	listenPortsStartLabel = new QLabel( connectionsTab, "listenPortsStartLabel" );
	ConnectionsGrid->addWidget( listenPortsStartLabel, 2, 0, Qt::AlignTop);

	CPortStart = new QSpinBox( connectionsTab, "CPortStart" );
	CPortStart->setMaxValue( 65535 );
	CPortStart->setValue( 0 );
	ConnectionsGrid->addWidget( CPortStart, 2, 1, Qt::AlignTop);

	listenPortsEndLabel = new QLabel( connectionsTab, "listenPortsEndLabel" );
	ConnectionsGrid->addWidget( listenPortsEndLabel, 2, 2, Qt::AlignTop);

	CPortEnd = new QSpinBox( connectionsTab, "CPortEnd" );
	CPortEnd->setMaxValue( 65535 );
	CPortEnd->setValue( 0 );
		
	ConnectionsGrid->addWidget( CPortEnd, 2, 3, Qt::AlignTop);

	ConnectionsGrid->setRowStretch(2, 10);
	
		
	// USERS Options Tab	
	usersTab = new QWidget( mMuseekdTabs, "usersTab" );
	mMuseekdTabs->insertTab( usersTab, QString::fromLatin1("") );
	UsersGrid = new QGridLayout( usersTab, 1, 1, 5, 5, "UsersGrid");


	SBuddiesPrivileged = new QCheckBox( usersTab, "SBuddiesPrivileged" );
	UsersGrid->addWidget( SBuddiesPrivileged, 0, 0);
	
	SShareBuddiesOnly = new QCheckBox( usersTab, "SShareBuddiesOnly" );
	UsersGrid->addWidget( SShareBuddiesOnly, 1, 0);
	
	STrustedUsers = new QCheckBox( usersTab, "STrustedUsers" );
	UsersGrid->addWidget( STrustedUsers, 2, 0);
		
	SUserWarnings = new QCheckBox( usersTab, "SUserWarnings" );
	UsersGrid->addWidget( SUserWarnings, 3, 0);
		
	UsersGrid->setRowStretch(4, 10);
	// Museeq Appearance
	AppearanceTab = new QWidget( mMuseeqTabs, "AppearanceTab" );
	mMuseeqTabs->insertTab( AppearanceTab, QString::fromLatin1("") );
	AppearanceGrid = new QGridLayout( AppearanceTab, 1, 1, 5, 5, "AppearanceGrid");

	SOnlineAlerts = new QCheckBox( AppearanceTab, "SOnlineAlerts" );
	AppearanceGrid->addWidget( SOnlineAlerts, 0, 0 );

	SIPLog = new QCheckBox( AppearanceTab, "SIPLog" );
	AppearanceGrid->addWidget( SIPLog, 1, 0 );
	AppearanceGrid->setRowStretch(2, 10);
		
	// Logging
	LoggingTab = new QWidget( mMuseeqTabs, "LoggingTab" );
	mMuseeqTabs->insertTab( LoggingTab, QString::fromLatin1("") );
	LoggingGrid = new QGridLayout( LoggingTab, 1, 1, 5, 5, "LoggingGrid");

	LoggingPrivate = new QCheckBox( LoggingTab, "LoggingPrivate" );
	LoggingGrid->addWidget( LoggingPrivate, 0, 0);

	
	LoggingPrivateDir = new QLineEdit( LoggingTab, "LoggingPrivateDir" );
	LoggingGrid->addWidget( LoggingPrivateDir, 1, 0);
	
	LoggingPrivateButton = new QPushButton( LoggingTab, "LoggingPrivateButton" );
	LoggingPrivateButton->setIconSet(IMG("open"));
	LoggingGrid->addWidget( LoggingPrivateButton, 1, 1);

	LoggingRooms = new QCheckBox( LoggingTab, "LoggingRooms" );
	LoggingGrid->addWidget( LoggingRooms, 2, 0);
	
	LoggingRoomDir = new QLineEdit( LoggingTab, "LoggingRoomDir" );
	LoggingGrid->addWidget( LoggingRoomDir, 3, 0);
	
	LoggingRoomButton = new QPushButton( LoggingTab, "LoggingRoomButton" );
	LoggingRoomButton->setIconSet(IMG("open"));
	LoggingGrid->addWidget( LoggingRoomButton, 3, 1);
	LoggingGrid->setRowStretch(4, 10);
	
	// Userinfo
	UserInfoTab = new QWidget( mMuseekdTabs, "UserInfoTab" );
	mMuseekdTabs->insertTab( UserInfoTab, QString::fromLatin1("") );
	UserInfoGrid = new QGridLayout( UserInfoTab, 1, 1, 11, 6, "UserInfoGrid");

	mInfoText = new QTextEdit( UserInfoTab, "mText" );

	UserInfoGrid->addMultiCellWidget( mInfoText, 0, 0, 0, 2 );

	buttonGroup1 = new QButtonGroup( UserInfoTab, "buttonGroup1" );
	buttonGroup1->setColumnLayout(0, Qt::Vertical );
	buttonGroup1->layout()->setSpacing( 6 );
	buttonGroup1->layout()->setMargin( 11 );
	buttonGroup1Layout = new QGridLayout( buttonGroup1->layout() );
	buttonGroup1Layout->setAlignment( Qt::AlignTop );

	mClear = new QRadioButton( buttonGroup1, "mClear" );

	buttonGroup1Layout->addWidget( mClear, 2, 0 );

	mDontTouch = new QRadioButton( buttonGroup1, "mDontTouch" );
	mDontTouch->setChecked( TRUE );

	buttonGroup1Layout->addWidget( mDontTouch, 0, 0 );

	mImage = new QLineEdit( buttonGroup1, "mImage" );

	buttonGroup1Layout->addWidget( mImage, 1, 1 );

	mUpload = new QRadioButton( buttonGroup1, "mUpload" );

	buttonGroup1Layout->addWidget( mUpload, 1, 0 );

	mBrowse = new QPushButton( buttonGroup1, "mBrowse" );
	mBrowse->setIconSet(IMG("open"));
	buttonGroup1Layout->addWidget( mBrowse, 1, 2 );
	connect( mBrowse, SIGNAL( clicked() ), this, SLOT( UserImageBrowse_clicked() ) );
		
	UserInfoGrid->addMultiCellWidget( buttonGroup1, 1, 1, 0, 2 );
	
	// Protocol Handlers Tab
	ProtocolTab = new QWidget( mMuseeqTabs, "UserInfoTab" );
	mMuseeqTabs->insertTab( ProtocolTab, QString::fromLatin1("") );

	ProtocolGrid = new QGridLayout( ProtocolTab, 1, 1, 5, 5, "ProtocolGrid");

	mProtocols = new QListView( ProtocolTab, "mProtocols" );
	mProtocols->addColumn( tr( "Protocol" ) );
	mProtocols->addColumn( tr( "Handler" ) );
	mProtocols->setAllColumnsShowFocus( TRUE );

	ProtocolGrid->addMultiCellWidget( mProtocols, 0, 0, 0, 4 );

	mNewHandler = new QPushButton( ProtocolTab, "mNew" );
	mNewHandler->setIconSet(IMG("new"));
	ProtocolGrid->addWidget( mNewHandler, 1, 2 );
	protocolSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	ProtocolGrid->addItem( protocolSpacer, 1, 0 );
	mModifyHandler = new QPushButton( ProtocolTab, "mModify" );
	mModifyHandler->setIconSet(IMG("comments"));
	ProtocolGrid->addWidget( mModifyHandler, 1, 1 );
	// Colors And Fonts
	ColorsAndFontsTab = new QWidget( mMuseeqTabs, "ColorsAndFontsTab" );
	mMuseeqTabs->insertTab( ColorsAndFontsTab, QString::fromLatin1("") );
	ColorsGrid = new QGridLayout( ColorsAndFontsTab, 1, 1, 5, 5, "ColorsGrid");
	MeColorLabel = new QLabel( ColorsAndFontsTab, "MeColorLabel" );
	ColorsGrid->addWidget( MeColorLabel, 0, 0 );
	
	SMeText = new QLineEdit( ColorsAndFontsTab, "SMeText" );
	ColorsGrid->addWidget( SMeText, 0, 1 );
	
	MeColorButton = new QPushButton( ColorsAndFontsTab, "MeColorButton" );
	MeColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( MeColorButton, 0, 2 );

	RemoteColorLabel = new QLabel( ColorsAndFontsTab, "RemoteColorLabel" );
	ColorsGrid->addWidget( RemoteColorLabel, 1, 0 );
	
	SRemoteText = new QLineEdit( ColorsAndFontsTab, "SRemoteText" );
	ColorsGrid->addWidget( SRemoteText, 1, 1 );

	RemoteColorButton = new QPushButton( ColorsAndFontsTab, "RemoteColorButton" );
	RemoteColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( RemoteColorButton, 1, 2 );

	LocalTextLabel = new QLabel( ColorsAndFontsTab, "LocalTextLabel" );
	ColorsGrid->addWidget( LocalTextLabel, 2, 0 );
	
	SNicknameText = new QLineEdit( ColorsAndFontsTab, "SNicknameText" );
	ColorsGrid->addWidget( SNicknameText, 2, 1 );

	NicknameColorButton = new QPushButton( ColorsAndFontsTab, "NicknameColorButton" );
	NicknameColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( NicknameColorButton, 2, 2 );
	
	BuddiedColorLabel = new QLabel( ColorsAndFontsTab, "BuddiedColorLabel" );
	ColorsGrid->addWidget( BuddiedColorLabel, 3, 0 );
	
	SBuddiedText = new QLineEdit( ColorsAndFontsTab, "SBuddiedText" );
	ColorsGrid->addWidget( SBuddiedText, 3, 1 );

	BuddiedColorButton = new QPushButton( ColorsAndFontsTab, "BuddiedColorButton" );
	BuddiedColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( BuddiedColorButton, 3, 2 );
	
	BannedColorLabel = new QLabel( ColorsAndFontsTab, "BannedColorLabel" );
	ColorsGrid->addWidget( BannedColorLabel, 4, 0 );
		
	SBannedText = new QLineEdit( ColorsAndFontsTab, "SBannedText" );
	ColorsGrid->addWidget( SBannedText, 4, 1 );
	
	BannedColorButton = new QPushButton( ColorsAndFontsTab, "BannedColorButton" );
	BannedColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( BannedColorButton, 4, 2 );

	TrustColorLabel = new QLabel( ColorsAndFontsTab, "TrustColorLabel" );
	ColorsGrid->addWidget( TrustColorLabel, 5, 0 );
	
	STrustedText = new QLineEdit( ColorsAndFontsTab, "STrustedText" );
	ColorsGrid->addWidget( STrustedText, 5, 1 );

	TrustColorButton = new QPushButton( ColorsAndFontsTab, "TrustColorButton" );
	TrustColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( TrustColorButton, 5, 2 );

	TimeColorLabel = new QLabel( ColorsAndFontsTab, "TimeColorLabel" , Qt::WordBreak);
	TimeColorLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeColorLabel, 6, 0 );

	STimeText = new QLineEdit( ColorsAndFontsTab, "STimeText" );
	ColorsGrid->addWidget( STimeText, 6, 1 );

	TimeColorButton = new QPushButton( ColorsAndFontsTab, "TimeColorButton" );
	TimeColorButton->setIconSet(IMG("colorpicker"));
	ColorsGrid->addWidget( TimeColorButton, 6, 2 );
	
	TimeFontLabel = new QLabel( ColorsAndFontsTab, "TimeFontLabel" , Qt::WordBreak);
	TimeFontLabel->setTextFormat(Qt::RichText);
	ColorsGrid->addWidget( TimeFontLabel, 7, 0 );

	STimeFont = new QLineEdit( ColorsAndFontsTab, "STimeFont" );
	ColorsGrid->addWidget( STimeFont, 7, 1 );
	
	TimeFontButton = new QPushButton( ColorsAndFontsTab, "TimeFontButton" );
	TimeFontButton->setIconSet(IMG("font"));
	ColorsGrid->addWidget( TimeFontButton, 7, 2 );

	MessageFontLabel = new QLabel( ColorsAndFontsTab, "MessageFontLabel" );
	ColorsGrid->addWidget( MessageFontLabel, 8, 0 );
	
	SMessageFont = new QLineEdit( ColorsAndFontsTab, "SMessageFont" );
	ColorsGrid->addWidget( SMessageFont, 8, 1 );

	MessageFontButton = new QPushButton( ColorsAndFontsTab, "MessageFontButton" );
	MessageFontButton->setIconSet(IMG("font"));
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
	connect( mOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( mSave, SIGNAL( clicked() ), this, SLOT( save() ) );
	connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( SConnect, SIGNAL( clicked() ), this, SLOT( SConnect_clicked() ) );
	connect( SDisconnect, SIGNAL( clicked() ), this, SLOT( SDisconnect_clicked() ) );
	connect( SReloadShares, SIGNAL( clicked() ), this, SLOT( SReloadShares_clicked() ) );
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
		
	connect( mNewHandler, SIGNAL( clicked() ), this, SLOT( mNewHandler_clicked() ) );
	connect( mModifyHandler, SIGNAL( clicked() ), this, SLOT( mModifyHandler_clicked() ) );
	connect( mProtocols, SIGNAL( itemRenamed(QListViewItem*,int) ), this, SLOT( mProtocols_itemRenamed(QListViewItem*,int) ) );
	connect(museeq, SIGNAL(configChanged(const QString&, const QString&, const QString&)), SLOT(slotConfigChanged(const QString&, const QString&, const QString&)));

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

}
void SettingsDialog::NormalSharesRefresh() {
	ListNormalShares->clear();
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(readyReadStdout()), this, SLOT(readNormal()) );
	proc1->addArgument( "muscan" );
	proc1->addArgument( "-c" );
	proc1->addArgument( SConfigFile->text() );
	proc1->addArgument( "-l" );

	if ( ! proc1->start() ) {
		// error handling
	}
}
void SettingsDialog::BuddySharesRefresh() {
	ListBuddyShares->clear();
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(readyReadStdout()), this, SLOT(readBuddy()) );
	proc2->addArgument( "muscan" );
	proc2->addArgument( "-c" );
	proc2->addArgument( SConfigFile->text() );
	proc2->addArgument( "-l" );
	proc2->addArgument( "-b" );

	if ( ! proc2->start() ) {
		// error handling
	}
}

void SettingsDialog::BuddySharesAdd() {
	QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
	fd->setMode(QFileDialog::Directory);
	fd->setCaption(tr("Select a Directory to add to your Buddy Shares."));
	fd->addFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->dirPath().isEmpty())
	{
		
		EnableBuddyButtons(false);
		proc2 = new QProcess( this );
		connect( proc2, SIGNAL(processExited ()), this, SLOT(MuscanBuddyDone()) );
		proc2->addArgument( "muscan" );
		proc2->addArgument( "-c" );
		proc2->addArgument( SConfigFile->text() );
		proc2->addArgument( "-b" );
		proc2->addArgument( "-s" );
		proc2->addArgument( fd->dirPath());
		
		if ( ! proc2->start() ) {
			EnableBuddyButtons(true);
		}
	}
	delete fd;
}


void SettingsDialog::PrivateDirSelect() {
	QFileDialog * fd = new QFileDialog(LoggingPrivateDir->text(), "", this);
	fd->setMode(QFileDialog::Directory);
	fd->setShowHiddenFiles(true);
	fd->setCaption(tr("Select a Directory to write Private Chat log files."));
	fd->addFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->dirPath().isEmpty())
	{
		LoggingPrivateDir->setText(fd->dirPath());
	
	}
	delete fd;
}

void SettingsDialog::RoomDirSelect() {
	QFileDialog * fd = new QFileDialog(LoggingRoomDir->text(), "", this);
	fd->setMode(QFileDialog::Directory);
	fd->setShowHiddenFiles(true);
	fd->setCaption(tr("Select a Directory to write Chat Room log files."));
	fd->addFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->dirPath().isEmpty())
	{
		LoggingRoomDir->setText(fd->dirPath());
	
	}
	delete fd;
}
		
void SettingsDialog::BuddySharesRescan() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(processExited ()), this, SLOT(MuscanBuddyDone()) );
	proc2->addArgument( "muscan" );
	proc2->addArgument( "-c" );
	proc2->addArgument( SConfigFile->text() );
	proc2->addArgument( "-b" );
	proc2->addArgument( "-r" );

	if ( ! proc2->start() ) {
		EnableBuddyButtons(true);
	}
}

void SettingsDialog::BuddySharesUpdate() {
	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(processExited ()), this, SLOT(MuscanBuddyDone()) );
	proc2->addArgument( "muscan" );
	proc2->addArgument( "-c" );
	proc2->addArgument( SConfigFile->text() );
	proc2->addArgument( "-b" );

	if ( ! proc2->start() ) {
		EnableBuddyButtons(true);
	}
}

void SettingsDialog::NormalSharesAdd() {
	QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
	fd->setMode(QFileDialog::Directory);
	fd->setCaption(tr("Select a Directory to add to your Normal Shares."));
	fd->addFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->dirPath().isEmpty())
	{
		EnableNormalButtons(false);
		proc1 = new QProcess( this );
		connect( proc1, SIGNAL(processExited ()), this, SLOT(MuscanNormalDone()) );
		proc1->addArgument( "muscan" );
		proc1->addArgument( "-c" );
		proc1->addArgument( SConfigFile->text() );
		proc1->addArgument( "-s" );
		proc1->addArgument( fd->dirPath());
		
		if ( ! proc1->start() ) {
			EnableNormalButtons(true);
		}
	}
	delete fd;
}

void SettingsDialog::BuddySharesRemove() {
	QListViewItem* item = ListBuddyShares->selectedItem();
	if (! item ||  item->text(0).isEmpty())
		return;
	QString directory (item->text(0));

	EnableBuddyButtons(false);
	proc2 = new QProcess( this );
	connect( proc2, SIGNAL(processExited ()), this, SLOT(MuscanBuddyDone()) );
	proc2->addArgument( "muscan" );
	proc2->addArgument( "-c" );
	proc2->addArgument( SConfigFile->text() );
	proc2->addArgument( "-b" );
	proc2->addArgument( "-u" );
	proc2->addArgument( directory);

	if ( ! proc2->start() ) {
		EnableBuddyButtons(true);
	}
	
}

void SettingsDialog::NormalSharesRemove() {
	QListViewItem* item = ListNormalShares->selectedItem();
	if (! item ||  item->text(0).isEmpty())
		return;

	QString directory (item->text(0));

	EnableNormalButtons(false);
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(processExited ()), this, SLOT(MuscanNormalDone()) );
	proc1->addArgument( "muscan" );
	proc1->addArgument( "-c" );
	proc1->addArgument( SConfigFile->text() );
	proc1->addArgument( "-u" );
	proc1->addArgument( directory);

	if ( ! proc1->start() ) {
		EnableNormalButtons(true);
	}

}

void SettingsDialog::NormalSharesRescan() {
	EnableNormalButtons(false);
	
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(processExited ()), this, SLOT(MuscanNormalDone()) );
	proc1->addArgument( "muscan" );
	proc1->addArgument( "-c" );
	proc1->addArgument( SConfigFile->text() );
	proc1->addArgument( "-r" );

	if ( ! proc1->start() ) {
		EnableNormalButtons(true);
	}
}
void SettingsDialog::NormalSharesUpdate() {
	EnableNormalButtons(false);
	
	proc1 = new QProcess( this );
	connect( proc1, SIGNAL(processExited ()), this, SLOT(MuscanNormalDone()) );
	proc1->addArgument( "muscan" );
	proc1->addArgument( "-c" );
	proc1->addArgument( SConfigFile->text() );

	if ( ! proc1->start() ) {
		EnableNormalButtons(true);
	}
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
void SettingsDialog::SettingsDialog::MuscanBuddyDone() {
	if (SBuddiesShares->isChecked ()) {
		EnableBuddyButtons(true);
		
	}
	BuddySharesRefresh();
}
void SettingsDialog::MuscanNormalDone() {
	EnableNormalButtons(true);
	NormalSharesRefresh();
}


void SettingsDialog::readNormal()
    {
	
        while (proc1->canReadLineStdout()) {
		new QListViewItem(ListNormalShares, proc1->readLineStdout());
	}
    }
void SettingsDialog::readBuddy()
    {
	
        while (proc2->canReadLineStdout()) {
		new QListViewItem(ListBuddyShares, proc2->readLineStdout());
	}
    }


void SettingsDialog::SConnect_clicked()
{  museeq->connectServer();
}
void SettingsDialog::SDisconnect_clicked()
{   museeq->disconnectServer();
}
void SettingsDialog::SReloadShares_clicked()
{   museeq->reloadShares();
}

void SettingsDialog::save()
{   museeq->saveSettings();
}

void SettingsDialog::SDownload_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
    fd->setMode(QFileDialog::Directory);
    fd->setCaption(tr("Select a Directory to store your downloaded files."));
    fd->addFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	SDownDir->setText( fd->dirPath());
    }
    
    delete fd;
}
void SettingsDialog::SConfig_clicked()
{
	QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
	fd->setMode(QFileDialog::ExistingFile );
	fd->setShowHiddenFiles(true);
	fd->setCaption(tr("Select the museekd config file."));
	fd->addFilter(tr("XML files (*.xml)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
	{
		SConfigFile->setText( fd->selectedFile());
	}
	
	delete fd;
}
void SettingsDialog::SIncomplete_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
    fd->setMode(QFileDialog::Directory);
    fd->setCaption(tr("Select a Directory to store your incomplete downloading files."));
    fd->addFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	SIncompleteDir->setText( fd->dirPath());
    }
    
    delete fd;
}

void SettingsDialog::UserImageBrowse_clicked()
{
	QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), tr("Images (*.png *.gif *.jpg *.jpeg)"), this);
	fd->setMode(QFileDialog::ExistingFile);
	fd->setCaption(tr("Select an Image for you User info"));
	fd->addFilter(tr("All files (*)"));
	if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
	{
		mImage->setText(fd->selectedFile());
		mUpload->setChecked(true);
	}
	
	delete fd;
}

void SettingsDialog::mNewHandler_clicked()
{
	QListViewItem* item = new QListViewItem(mProtocols, "", "");
	item->setRenameEnabled(0, true);
	item->setRenameEnabled(1, true);
	item->startRename(0);
}
			    
void SettingsDialog::mModifyHandler_clicked()
{
	QListViewItem* item = mProtocols->selectedItem();
	if (item) {
		item->setRenameEnabled(0, true);
		item->setRenameEnabled(1, true);
		item->startRename(1);
	}
}

void SettingsDialog::mProtocols_itemRenamed( QListViewItem *item, int col)
{
	if(col == 0)
		item->startRename(1);
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
	setCaption( tr( "Museeq Settings" ) );
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

	
	buttonGroup1->setTitle( tr( "Image" ) );
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
	

	mProtocols->header()->setLabel( 0, tr( "Protocol" ) );
	mProtocols->header()->setLabel( 1, tr( "Handler" ) );
	mNewHandler->setText( tr( "New" ) );
	mModifyHandler->setText( tr( "Modify" ) );
	// Museekd Tabs
	mTabHolder->changeTab( mMuseekdTabs, tr( "Museek Daemon" ) );
	mMuseekdTabs->changeTab( serverTab, tr( "Server" ) );
	mMuseekdTabs->changeTab( sharesTab, tr( "Shares" ) );
	mMuseekdTabs->changeTab( connectionsTab, tr( "Connections" ) );
	mMuseekdTabs->changeTab( usersTab, tr( "User Options" ) );
	mMuseekdTabs->changeTab( UserInfoTab, tr( "User Info" ) );
	// Museeq tabs
	mTabHolder->changeTab( mMuseeqTabs, tr( "Museeq" ) );
	mMuseeqTabs->changeTab( AppearanceTab, tr("Appearance") );
	mMuseeqTabs->changeTab( ColorsAndFontsTab, tr("Fonts and Colors") );
	mMuseeqTabs->changeTab( LoggingTab, tr( "Logging" ) );
	mMuseeqTabs->changeTab( ProtocolTab, tr( "Protocol handlers" ) );
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
	buttonGroup2->setTitle( tr( "Connections" ) );
	SActive->setText( tr( "Active Connections" ) );
	SPassive->setText( tr( "Passive Connections" ) );
	SReloadShares->setText( tr( "Reload Shares" ) );
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
	
}

