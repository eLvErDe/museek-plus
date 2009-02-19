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

#include "ipdialog.h"
#include "images.h"

#include <QTreeWidget>
#include <QPushButton>
#include <QLayout>
#include <QDialogButtonBox>

IPDialog::IPDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
    : QDialog( parent)
{
	setMinimumSize( QSize( 400, 0 ) );
	IPDialogLayout = new QGridLayout( this);

	mIPListView = new QTreeWidget( this );
	QStringList headers;
	headers << tr( "User" ) <<  tr( "IP" ) << tr( "Port" ) << tr( "Hostname" );
	mIPListView->setHeaderLabels(headers);
	mIPListView->setSortingEnabled(true);
	mIPListView->setRootIsDecorated(false);
     mIPListView->setAllColumnsShowFocus(true);

	IPDialogLayout->addWidget( mIPListView, 0, 0, 1, 3 );

	spacer5 = new QSpacerItem( 120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	IPDialogLayout->addItem( spacer5, 1, 0 );

	mClear = new QPushButton( this );
	IPDialogLayout->addWidget( mClear, 1, 1 );
	mClear->setIcon(QIcon(IMG("remove")));

    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mButtonBox->addButton(mClear, QDialogButtonBox::ActionRole);
	IPDialogLayout->addWidget( mButtonBox, 1, 1 );

	languageChange();
	resize( QSize(400, 220).expandedTo(minimumSizeHint()) );

	// signals and slots connections
	connect( mClear, SIGNAL( clicked() ), mIPListView, SLOT( clear() ) );
	connect( mButtonBox, SIGNAL( clicked(QAbstractButton*) ), this, SLOT( buttonClicked(QAbstractButton*) ) );
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void IPDialog::languageChange()
{
    setWindowTitle( tr( "IP addresses" ) );
    mClear->setText( tr( "Clear" ) );
}

void IPDialog::buttonClicked(QAbstractButton* ab) {
    int role = mButtonBox->standardButton(ab);
    if ( role == QDialogButtonBox::Close )
        hide();
}
