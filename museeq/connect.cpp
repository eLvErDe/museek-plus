#include <qcheckbox.h>
#include <qlabel.h> 
#include <qhbox.h> 
#include <qvbox.h> 
#include <qlayout.h> 
#include <qlineedit.h> 
#include <qpushbutton.h> 
#include "connect.h" 
#include <qdir.h>
#include "museeq.h"
#include <qfiledialog.h>

#include <qvariant.h>
#include <qvbuttongroup.h>
#include <qbuttongroup.h>

#include <qcombobox.h>
#include <qradiobutton.h>

#include <qtooltip.h>
#include <qwhatsthis.h>

ConnectDialog::ConnectDialog(QWidget *parent, const char *name) 
  : QDialog(parent, name) 
{ 
	setCaption( tr( "Connect to Museekd..." ) );
	setMinimumSize( QSize( 430, 280 ) );
	setModal( TRUE );

	QHBox *boxsd = new QHBox(this, "boxsd");
	boxsd->setSpacing(5);
	QVBox *boxconnect = new QVBox(boxsd, "boxconnect");
	boxconnect->setSpacing(5);
	QHBox *boxhost = new QHBox(boxconnect, "boxhostpath");
	boxhost->setSpacing(5);
	textLabel2 = new QLabel( boxhost, "textLabel2" );
	textLabel2->setText( tr( "Host / path:" ) );
	mAddress = new QComboBox( FALSE, boxhost, "mAddress" );
	mAddress->setEditable( TRUE );
	mAddress->setSizePolicy (QSizePolicy::Expanding,QSizePolicy::Preferred);
	QHBox *boxpass = new QHBox(boxconnect, "boxpassword");
	boxpass->setSpacing(5);
	textLabel1 = new QLabel( boxpass, "textLabel1" );
	textLabel1->setText( tr( "Password:" ) );
	mPassword = new QLineEdit( boxpass, "mPassword" );
	mPassword->setEchoMode( QLineEdit::Password );
	mSavePassword = new QCheckBox( boxpass, "mSavePassword" );
	mSavePassword->setText( tr( "&Save Password" ) );
	mSavePassword->setAccel( QKeySequence( tr( "Alt+S" ) ) );

	buttonGroup4 = new QVButtonGroup( boxsd, "buttonGroup4" );	
	buttonGroup4->setTitle( tr( "Connect to:" ) );
	mTCP = new QRadioButton( buttonGroup4, "mTCP" );
	mTCP->setChecked( TRUE );
	mUnix = new QRadioButton( buttonGroup4, "mUnix" );
	mUnix->setEnabled( TRUE );
	mTCP->setText( tr( "&TCP" ) );
	mTCP->setAccel( QKeySequence( tr( "Alt+T" ) ) );
	mUnix->setText( tr( "&Unix socket" ) );
	mUnix->setAccel( QKeySequence( tr( "Alt+U" ) ) );
	buttonGroup4->insert( mUnix, 1 );
	
	spacer1 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	mExtra = new QPushButton( this, "ExtraOptions" );
	mExtra->setText( tr( "Daemon O&ptions..." ) );
	
	startDaemonButton = new QPushButton( this, "startDaemonButton" );
	startDaemonButton->setDefault( FALSE );
	spacer2 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	stopDaemonButton = new QPushButton( this, "stopDaemonButton" );
	stopDaemonButton->setDefault( FALSE );
	startDaemonButton->setText( tr( "Start &Daemon" ) );
	startDaemonButton->setAccel( QKeySequence( tr( "Alt+D" ) ) );
	stopDaemonButton->setText( tr( "St&op Daemon" ) );
	stopDaemonButton->setAccel( QKeySequence( tr( "Alt+O" ) ) );	

	box2 = new QHBox(this, "centralWidget");
	box2->setSpacing(5);
	configLabel = new QLabel( box2, "configLabel" );
	configLabel->setText( tr( "Museek Daemon Config:" ) );
	mMuseekConfig = new QLineEdit( box2, "mMuseekConfig" );
	selectButton = new QPushButton( box2, "selectButton" );
	selectButton->setText( tr( "Se&lect..." ) );
	selectButton->setAccel( QKeySequence( tr( "Alt+L" ) ) );	

	spacer3 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	connectButton = new QPushButton( this, "connectButton" );
	connectButton->setDefault( TRUE );
	saveButton = new QPushButton( this, "saveButton" );
	saveButton->setDefault( FALSE );
	cancelButton = new QPushButton( this, "cancelButton" );
	connectButton->setText( tr( "Co&nnect" ) );
	connectButton->setAccel( QKeySequence( tr( "Alt+N" ) ) );
	saveButton->setText( tr( "Save" ) );
	saveButton->setAccel( QKeySequence( QString::null ) );
	cancelButton->setText( tr( "&Cancel" ) );
	cancelButton->setAccel( QKeySequence( tr( "Alt+C" ) ) );

	box3 = new QHBox(this, "centralWidget");
	box3->setSpacing(5);
	mAutoStartDaemon = new QCheckBox( box3, "mAutoStartDaemon" );
	mShutDownDaemonOnExit = new QCheckBox( box3, "mShutDownDaemonOnExit" );
	mAutoStartDaemon->setText( tr( "&AutoStart Museek Daemon" ) );
	mAutoStartDaemon->setAccel( QKeySequence( tr( "Alt+A" ) ) );
	mShutDownDaemonOnExit->setText( tr( "S&hutDown Daemon on Exit" ) );
	mShutDownDaemonOnExit->setAccel( QKeySequence( tr( "Alt+H" ) ) );


	extra = true;
	// signals and slots connections
	connect( connectButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( mExtra, SIGNAL( clicked() ), this, SLOT( extraOptions() ) );
	connect( startDaemonButton, SIGNAL( clicked() ), this, SLOT( startDaemon() ) );
	connect( selectButton, SIGNAL( clicked() ), this, SLOT( selectConfig() ) );
	connect( stopDaemonButton, SIGNAL( clicked() ), this, SLOT( stopDaemon() ) );
	connect( saveButton, SIGNAL( clicked() ), this, SLOT( save() ) );

	setTabOrder( mPassword, mAutoStartDaemon );
	setTabOrder( mAutoStartDaemon, connectButton );
	setTabOrder( connectButton, cancelButton );
	setTabOrder( cancelButton, mTCP );

	extraLayout = new QHBoxLayout; 
	extraLayout->addItem(spacer1); 
	extraLayout->addWidget(mExtra); 

	controldLayout = new QHBoxLayout; 
	controldLayout->addWidget(startDaemonButton); 
	controldLayout->addItem(spacer2); 
	controldLayout->addWidget(stopDaemonButton); 

	connectLayout = new QHBoxLayout; 
	connectLayout->addItem(spacer3); 
	connectLayout->addWidget(connectButton); 
	connectLayout->addWidget(saveButton);
	connectLayout->addWidget(cancelButton);

	QVBoxLayout *leftLayout = new QVBoxLayout; 
	leftLayout->addWidget(boxsd); 

	leftLayout->addLayout(extraLayout);
	leftLayout->addWidget(box3); 
	leftLayout->addLayout(controldLayout);
	leftLayout->addWidget(box2); 

	leftLayout->addLayout(connectLayout); 
 	QHBoxLayout *mainLayout = new QHBoxLayout(this); 
 	mainLayout->setMargin(11); 
 	mainLayout->setSpacing(6); 
 	mainLayout->addLayout(leftLayout); 
	extraOptions();

} 

// below be dragons
void ConnectDialog::startDaemon()
{
    museeq->startDaemon();
}

void ConnectDialog::extraOptions()
{	
	if ( extra) {
		box2->hide();
		box3->hide();
		startDaemonButton->hide();
		stopDaemonButton->hide();
		extra = false;
	} else {
		box2->show();
		box3->show();
		startDaemonButton->show();
		stopDaemonButton->show();
		extra = true;
	}
}

void ConnectDialog::selectConfig()
{
    QDir dir = QDir::home();
    QFileDialog * fd = new QFileDialog(dir.path()+"/.museekd", "Museek Daemon Config (*.xml)", this);
    fd->setCaption("Select a Museek Daemon Config File");
    fd->setMode(QFileDialog::ExistingFile);
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	mMuseekConfig->setText(fd->selectedFile());
    }
    
    delete fd;
}


void ConnectDialog::stopDaemon()
{
  museeq->stopDaemon();
}




void ConnectDialog::save()
{
  museeq->saveConnectConfig();

}
