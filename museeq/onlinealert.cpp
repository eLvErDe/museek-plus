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

#include "onlinealert.h"
#include "museeq.h"

#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QDateTime>
#include <QDialogButtonBox>

OnlineAlert::OnlineAlert( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent )
{
    OnlineAlertLayout = new QVBoxLayout( this);

    mLabel = new QLabel( this);
    OnlineAlertLayout->addWidget( mLabel );

    frame3 = new QFrame( this );
    frame3->setFrameShape( QFrame::HLine );
    frame3->setFrameShadow( QFrame::Raised );
    OnlineAlertLayout->addWidget( frame3 );

    layout3 = new QHBoxLayout;
    OnlineAlertLayout->addLayout( layout3 );
    spacer1 = new QSpacerItem( 111, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer1 );

    mRemove = new QPushButton( this );
    layout3->addWidget( mRemove );

    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    mButtonBox->addButton(mRemove, QDialogButtonBox::ActionRole);
	layout3->addWidget( mButtonBox);

    languageChange();
    resize( QSize(281, 92).expandedTo(minimumSizeHint()) );

    // signals and slots connections
    connect( mRemove, SIGNAL( clicked() ), this, SLOT( slotRemoveAlert() ) );
	connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OnlineAlert::languageChange()
{
    setWindowTitle( tr( "Online alert" ) );
    mLabel->setText( QString::null );
    mRemove->setText( tr( "&Remove" ) );
}

void OnlineAlert::setUser( const QString &user )
{
    mUser = user;
    connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(slotUserStatus(const QString&, uint)));
}


void OnlineAlert::slotUserStatus( const QString & user, uint status )
{
    if (museeq->mOnlineAlert)
        return;
    if(user == mUser && (status == 2 || isVisible())) {
        QString s = (status == 0) ? tr("offline") : ((status == 1) ? tr("away") : tr("online"));
        QString t = QTime::currentTime().toString();
        mLabel->setText(QString(tr("%1 user %2 is now %3")).arg(t).arg(mUser).arg(s));
        if(isVisible())
            raise();
        else
            show();
    }
}

void OnlineAlert::slotRemoveAlert()
{
    emit removeAlert(mUser);
    accept();
}
