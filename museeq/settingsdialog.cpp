
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
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "museeq.h"
#include <qfiledialog.h>
#include <qdir.h>
#include <qsplitter.h>
#include "codeccombo.h"

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
	QHBox* hbox = new QHBox;
	hbox->setSpacing(5);
	hbox->setMargin(5);
	QHBoxLayout* buttonsLayout= new QHBoxLayout; 
	
	// Ok, Save, Cancel buttons
	QSpacerItem* spacer5 = new QSpacerItem( 200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonsLayout->addItem( spacer5 );

	mOK = new QPushButton( this, "mOK" );
	buttonsLayout->addWidget(mOK);

	mSave = new QPushButton( this, "mSave" );
	buttonsLayout->addWidget(mSave);

	mCancel = new QPushButton( this , "mCancel" );
	mCancel->setDefault( TRUE );
	buttonsLayout->addWidget(mCancel);

	// Add buttons to vLayout
	vLayout->addLayout(buttonsLayout);

	mTabHolder->setTabPosition( QTabWidget::Top );
	// SERVER TAB
	serverTab = new QWidget( mTabHolder, "serverTab" );
	QVBoxLayout* serverLayout = new QVBoxLayout( serverTab, 0, 0, "serverLayout" );
	QHBoxLayout* serverLayout_host = new QHBoxLayout; 
	QHBoxLayout* serverLayout_port = new QHBoxLayout; 
	QHBoxLayout* serverLayout_username = new QHBoxLayout; 
	QHBoxLayout* serverLayout_password = new QHBoxLayout; 
	QHBoxLayout* serverLayout_connect = new QHBoxLayout;
	QHBoxLayout* serverLayout_Fencoding = new QHBoxLayout; 
	QHBoxLayout* serverLayout_Nencoding = new QHBoxLayout;
// 	QHBoxLayout* serverLayout_8 = new QHBoxLayout;

	serverLayout->setMargin(5);
	serverLayout->setSpacing(5);

	// Server Host
	serverHostLabel = new QLabel( serverTab, "serverHostLabel" );
	
	serverLayout_host->addWidget( serverHostLabel);
	spacer16 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	serverLayout_host->addItem( spacer16 );

	SServerHost = new QLineEdit( serverTab, "SServerHost" );
	SServerHost->setMaxLength( 50 );
	
	serverLayout_host->addWidget( SServerHost);
	// Server Port
	serverPortLabel = new QLabel( serverTab, "serverPortLabel" );
	serverPortLabel->setMargin( 0 );
	
	serverLayout_port->addWidget( serverPortLabel);
	SServerPort = new QSpinBox( serverTab, "SServerPort" );
	SServerPort->setMaxValue( 65535 );
	SServerPort->setValue( 0 );
	
	serverLayout_port->addWidget( SServerPort);

	// Server Username

	usernamelabel = new QLabel( serverTab, "usernamelabel" );
	serverLayout_username->addWidget( usernamelabel);
	spacer15 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	serverLayout_username->addItem( spacer15 );
	SSoulseekUsername = new QLineEdit( serverTab, "SSoulseekUsername" );
	serverLayout_username->addWidget( SSoulseekUsername);

	// Server Password	
	passwordLabel = new QLabel( serverTab, "passwordLabel"  );
	
	serverLayout_password->addWidget( passwordLabel);
	spacer14 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );

	serverLayout_password->addItem( spacer14 );
		
	SSoulseekPassword = new QLineEdit( serverTab, "SSoulseekPassword" );
	SSoulseekPassword->setEchoMode(QLineEdit::Password);
	
	serverLayout_password->addWidget( SSoulseekPassword);

	// Connect / Disconnect
	spacer13 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	serverLayout_connect->addItem( spacer13 );
	SConnect = new QPushButton( serverTab, "SConnect" );
	serverLayout_connect->addWidget( SConnect );
	
	SDisconnect = new QPushButton( serverTab, "SDisconnect" );
	serverLayout_connect->addWidget( SDisconnect );

	// Filesystem encoding
	fEncodingLabel = new QLabel( serverTab, "fEncodingLabel" );
	serverLayout_Fencoding->addWidget( fEncodingLabel );
	SFileSystemEncoding =  new CodecCombo("encoding", "filesystem", serverTab, "encoding");
	serverLayout_Fencoding->addWidget( SFileSystemEncoding );
	// Network Encoding
	nEncodingLabel = new QLabel( serverTab, "nEncodingLabel" );
	serverLayout_Nencoding->addWidget( nEncodingLabel );
	SNetworkEncoding =  new CodecCombo("encoding", "network", serverTab, "encoding");
	serverLayout_Nencoding->addWidget( SNetworkEncoding );

	
	// Put the sublayouts in serverlayout
	serverLayout->addLayout(serverLayout_host);
	serverLayout->addLayout(serverLayout_port);
	serverLayout->addLayout(serverLayout_username);
	serverLayout->addLayout(serverLayout_password);
	serverLayout->addLayout(serverLayout_connect);
	serverLayout->addLayout(serverLayout_Fencoding);
	serverLayout->addLayout(serverLayout_Nencoding);

	mTabHolder->insertTab( serverTab, QString::fromLatin1("") );

	// SHARES TAB
	sharesTab = new QWidget( mTabHolder, "sharesTab" );
	QVBoxLayout* sharesLayout = new QVBoxLayout( sharesTab, 0, 0, "sharesLayout" );
	QHBoxLayout* sharesLayout_1 = new QHBoxLayout; 
	QHBoxLayout* sharesLayout_2 = new QHBoxLayout; 
	sharesLayout->setMargin(5);
	sharesLayout->setSpacing(5);
	
	instructionsLabel = new QLabel( sharesTab, "instructionsLabel" );
	instructionsLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );
	sharesLayout->addWidget(instructionsLabel);
	
	SReloadShares = new QPushButton( sharesTab, "SReloadShares" );
	
	sharesLayout->addWidget( SReloadShares, 1, 1 );

	buttonGroup2 = new QHButtonGroup( sharesTab, "buttonGroup2" );

	SActive = new QRadioButton( buttonGroup2, "SActive" );
	SPassive = new QRadioButton( buttonGroup2, "SPassive" );

	sharesLayout->addWidget(buttonGroup2);


	downloadLabel = new QLabel( sharesTab, "downloadLabel" );
	sharesLayout_1->addWidget( downloadLabel );
	
	SDownDir = new QLineEdit( sharesTab, "SDownDir" );
	sharesLayout_1->addWidget( SDownDir);
	
	SDownloadButton = new QPushButton( sharesTab, "SDownloadButton" );
	sharesLayout_1->addWidget( SDownloadButton);

	incompleteLabel = new QLabel( sharesTab, "incompleteLabel" );
	sharesLayout_2->addWidget( incompleteLabel);
	
	SIncompleteDir = new QLineEdit( sharesTab, "SIncompleteDir" );
	sharesLayout_2->addWidget( SIncompleteDir);
	
	SIncompleteButton = new QPushButton( sharesTab, "SIncompleteButton" );
	sharesLayout_2->addWidget( SIncompleteButton);
	
	sharesLayout->addLayout(sharesLayout_1);
	sharesLayout->addLayout(sharesLayout_2);

	mTabHolder->insertTab( sharesTab, QString::fromLatin1("") );

	// USERS Options Tab

	usersTab = new QWidget( mTabHolder, "usersTab" );

	QVBoxLayout* usersLayout = new QVBoxLayout( usersTab, 0, 0, "usersLayout" );

	usersLayout->setMargin(5);
	usersLayout->setSpacing(5);

	SBuddiesPrivileged = new QCheckBox( usersTab, "SBuddiesPrivileged" );
	usersLayout->addWidget( SBuddiesPrivileged );
	
	SShareBuddiesOnly = new QCheckBox( usersTab, "SShareBuddiesOnly" );
	usersLayout->addWidget( SShareBuddiesOnly );
	
	STrustedUsers = new QCheckBox( usersTab, "STrustedUsers" );
	usersLayout->addWidget( STrustedUsers );
	
	SBuddiesShares = new QCheckBox( usersTab, "SBuddiesShares" );
	usersLayout->addWidget( SBuddiesShares );
	
	SUserWarnings = new QCheckBox( usersTab, "SUserWarnings" );
	usersLayout->addWidget( SUserWarnings );

	SOnlineAlerts = new QCheckBox( usersTab, "SOnlineAlerts" );
	usersLayout->addWidget( SOnlineAlerts );

	SIPLog = new QCheckBox( usersTab, "SIPLog" );
	usersLayout->addWidget( SIPLog );

	mTabHolder->insertTab( usersTab, QString::fromLatin1("") );
	languageChange();
	resize( QSize(400, 450).expandedTo(minimumSizeHint()) );

	
	// signals and slots connections
	connect( mOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( mSave, SIGNAL( clicked() ), this, SLOT( save() ) );
	connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( SConnect, SIGNAL( clicked() ), this, SLOT( SConnect_clicked() ) );
	connect( SDisconnect, SIGNAL( clicked() ), this, SLOT( SDisconnect_clicked() ) );
	connect( SReloadShares, SIGNAL( clicked() ), this, SLOT( SReloadShares_clicked() ) );
	connect( SDownloadButton, SIGNAL( clicked() ), this, SLOT( SDownload_clicked() ) );
	connect( SIncompleteButton, SIGNAL( clicked() ), this, SLOT( SIncomplete_clicked() ) );
	

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
    setCaption( tr( "Museek Settings" ) );
    mOK->setText( tr( "Ok" ) );
    mSave->setText( tr( "Save" ) );
    mCancel->setText( tr( "Cancel" ) );
    fEncodingLabel->setText( tr( "Filesystem Encoding:" ) );
    nEncodingLabel->setText( tr( "Network Encoding:" ) );
    SConnect->setText( tr( "Connect" ) );
    SDisconnect->setText( tr( "Disconnect" ) );
    serverPortLabel->setText( tr( "Server Port:" ) );
    serverHostLabel->setText( tr( "Server Host:" ) );
    usernamelabel->setText( tr( "Soulseek Username:" ) );
    passwordLabel->setText( tr( "Soulseek Password:" ) );
    SSoulseekPassword->setInputMask( QString::null );
    mTabHolder->changeTab( serverTab, tr( "Server" ) );
    instructionsLabel->setText( tr( "<p>Configure your shares with musetup, or run <u>muscan -r </u>to rescan your shares and then press:</p>" ) );
    buttonGroup2->setTitle( tr( "Connections" ) );
    SActive->setText( tr( "Active Connections" ) );
    SPassive->setText( tr( "Passive Connections" ) );
    SReloadShares->setText( tr( "Reload Shares" ) );
    SDownloadButton->setText( tr( "Select.." ) );
    downloadLabel->setText( tr( "Download Dir:" ) );
    SIncompleteButton->setText( tr( "Select.." ) );
    incompleteLabel->setText( tr( "Incomplete Dir:" ) );
    mTabHolder->changeTab( sharesTab, tr( "Shares and Connections" ) );
    SBuddiesPrivileged->setText( tr( "Buddies are Privileged" ) );
    SOnlineAlerts->setText( tr( "Online Alerts in Log Window instead of popup" ) );
    SShareBuddiesOnly->setText( tr( "Share to Buddies Only" ) );
    STrustedUsers->setText( tr( "Trusted users can Send you Files" ) );
    SBuddiesShares->setText( tr( "Seperate Shares list for Buddies" ) );
    SUserWarnings->setText( tr( "Send automatic warnings to users via Private Chat" ) );
    SIPLog->setText( tr( "IP addresses in Log Window instead of popup" ) );
    mTabHolder->changeTab( usersTab, tr( "User Options" ) );
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
