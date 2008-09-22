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
#include "connect.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QComboBox>
#include <QRadioButton>
#include <QDir>

ConnectDialog::ConnectDialog(QWidget *parent, const char *name)
  : QDialog(parent)
{
	setWindowTitle( tr( "Connect to Museekd..." ) );
	setMinimumSize( QSize( 500, 200 ) );
	setModal( TRUE );

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(0);

	QHBoxLayout *LeftRight = new QHBoxLayout;
	mainLayout->addLayout(LeftRight);
	LeftRight->setSpacing(5);
	LeftRight->setMargin(5);

	QVBoxLayout *SocketPassLayout = new QVBoxLayout;
	SocketPassLayout->setSpacing(5);
	LeftRight->addLayout(SocketPassLayout);
	// Host, path, clear button
	QHBoxLayout *HostLayout = new QHBoxLayout;
	SocketPassLayout->addLayout(HostLayout);

	HostLayout->setSpacing(5);
	HostLabel = new QLabel(this);
	HostLayout->addWidget(HostLabel);
	HostLabel->setText( tr( "Host / path:" ) );

	mAddress = new QComboBox(this);
	HostLayout->addWidget(mAddress);
	mAddress->setEditable( TRUE );
	mAddress->setSizePolicy (QSizePolicy::Expanding,QSizePolicy::Preferred);
	clearButton = new QPushButton( this);
	clearButton->setText( tr( "Clear" ) );

	HostLayout->addWidget(clearButton);
	QHBoxLayout *boxpass = new QHBoxLayout;
	SocketPassLayout->addLayout(boxpass);
	boxpass->setSpacing(5);

	// museekd Password
	PasswordLabel = new QLabel(this);
	boxpass->addWidget(PasswordLabel);
	PasswordLabel->setText( tr( "Password:" ) );
	mPassword = new QLineEdit;
	boxpass->addWidget(mPassword);
	mPassword->setEchoMode( QLineEdit::Password );
	mSavePassword = new QCheckBox(this);
	mSavePassword->setText( tr( "&Save Password" ) );
	boxpass->addWidget(mSavePassword);

	mAutoConnect = new QCheckBox(this);
	SocketPassLayout->addWidget(mAutoConnect);
	mAutoConnect->setText( tr( "Auto-Conn&ect to Daemon" ) );

	// Connection method
	QGroupBox * groupbox = new QGroupBox(tr( "Connect to:" ), this);
	LeftRight->addWidget(groupbox);
	QVBoxLayout * MethodLayout = new QVBoxLayout;
	groupbox->setLayout(MethodLayout);

	mTCP = new QRadioButton( groupbox );
	mTCP->setChecked( TRUE );
	MethodLayout->addWidget(mTCP);
	mUnix = new QRadioButton( groupbox );
	mUnix->setEnabled( TRUE );
	MethodLayout->addWidget(mUnix);
	mTCP->setText( tr( "&TCP" ) );
	mUnix->setText( tr( "&Unix socket" ) );

	// autoconnect
	extraLayout = new QHBoxLayout;
	mainLayout->addLayout(extraLayout);
	extraLayout->setMargin(5);
	spacer1 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	mExtra = new QPushButton( this);
	mExtra->setText( tr( "Daemon O&ptions..." ) );
	extraLayout->addItem(spacer1);
	extraLayout->addWidget(mExtra);

	DaemonItems = new QWidget(this);
	mainLayout->addWidget(DaemonItems);
	QVBoxLayout * DaemonBox = new QVBoxLayout(DaemonItems);
	DaemonBox->setMargin(5);

	controldLayout = new QHBoxLayout;
	DaemonBox->addLayout(controldLayout);
	spacer2 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	controldLayout->addItem(spacer2);

	startDaemonButton = new QPushButton( this);
	startDaemonButton->setDefault( FALSE );
	startDaemonButton->setText( tr( "Start &Daemon" ) );
	controldLayout->addWidget(startDaemonButton);

	stopDaemonButton = new QPushButton( this );
	stopDaemonButton->setDefault( FALSE );
	stopDaemonButton->setText( tr( "St&op Daemon" ) );
	controldLayout->addWidget(stopDaemonButton);

	QHBoxLayout * DaemonButtonsLayout = new QHBoxLayout;
	DaemonButtonsLayout->setSpacing(5);
	DaemonBox->addLayout(DaemonButtonsLayout);

	mAutoStartDaemon = new QCheckBox( this);
	mAutoStartDaemon->setText( tr( "&AutoStart Museek Daemon" ) );
	DaemonButtonsLayout->addWidget(mAutoStartDaemon);

	mShutDownDaemonOnExit = new QCheckBox( this );
	mShutDownDaemonOnExit->setText( tr( "S&hutDown Daemon on Exit" ) );
	DaemonButtonsLayout->addWidget(mShutDownDaemonOnExit);


	QHBoxLayout * ConfigLayout = new QHBoxLayout;
	DaemonBox->addLayout(ConfigLayout);
	ConfigLayout->setSpacing(5);
	configLabel = new QLabel( this);
	ConfigLayout->addWidget(configLabel);
	configLabel->setText( tr( "Museek Daemon Config:\n(leave empty for default)" ) );
	mMuseekConfig = new QLineEdit( this );
	ConfigLayout->addWidget(mMuseekConfig);
	selectButton = new QPushButton( this );
	ConfigLayout->addWidget(selectButton);
	selectButton->setText( tr( "Se&lect..." ) );

	QSpacerItem * spacerFill =  new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
	mainLayout->addItem(spacerFill);
	extra = true;
	connectLayout = new QHBoxLayout;
	spacer3 =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	connectLayout->addItem(spacer3);
	connectLayout->setSpacing(5);
	connectLayout->setMargin(5);

	mainLayout->addLayout(connectLayout);

	connectButton = new QPushButton( this);
	connectButton->setDefault( TRUE );
	connectButton->setText( tr( "Co&nnect" ) );
	connectLayout->addWidget(connectButton);

	saveButton = new QPushButton( this );
	saveButton->setText( tr( "Save" ) );
	saveButton->setDefault( FALSE );
	connectLayout->addWidget(saveButton);

	cancelButton = new QPushButton( this );
	cancelButton->setText( tr( "&Cancel" ) );
	connectLayout->addWidget(cancelButton);


	extraOptions();
	// signals and slots connections
	connect( connectButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( cancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( mExtra, SIGNAL( clicked() ), this, SLOT( extraOptions() ) );
	connect( startDaemonButton, SIGNAL( clicked() ), this, SLOT( startDaemon() ) );
	connect( selectButton, SIGNAL( clicked() ), this, SLOT( selectConfig() ) );
	connect( stopDaemonButton, SIGNAL( clicked() ), this, SLOT( stopDaemon() ) );
	connect( saveButton, SIGNAL( clicked() ), this, SLOT( save() ) );
	connect( clearButton, SIGNAL( clicked() ), this, SLOT( clearSockets() ) );
}

// below be dragons
void ConnectDialog::startDaemon()
{
    museeq->startDaemon();
}

void ConnectDialog::clearSockets()
{
	mAddress->clear();
}

void ConnectDialog::extraOptions()
{
	if ( extra) {

		DaemonItems->hide();
		mExtra->setDown(false);
		resize( QSize( 500, 200 ) );
		extra = false;
	} else {

		DaemonItems->show();
		mExtra->setDown(true);
		resize( QSize( 500, 300 ) );
		extra = true;
	}
}

void ConnectDialog::selectConfig()
{
    QDir dir = QDir::home();
    QFileDialog * fd = new QFileDialog(this, dir.path()+"/.museekd", "Museek Daemon Config (*.xml)");
    fd->setWindowTitle("Select a Museek Daemon Config File");
    fd->setFileMode(QFileDialog::ExistingFile);
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFiles().isEmpty())
    {
	mMuseekConfig->setText(fd->selectedFiles().at(0));
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
