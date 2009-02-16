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

#include "tickerdialog.h"

#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QLayout>
#include <QDialogButtonBox>


/*
 *  Constructs a TickerDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TickerDialog::TickerDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl )
	: QDialog( parent )
{

	setMinimumSize( QSize( 400, 150 ) );
	TickerDialogLayout = new QVBoxLayout( this);

	buttonGroup1 = new QGroupBox( this );


	TickerDialogLayout->addWidget(buttonGroup1);
	buttonGroup1Layout = new QGridLayout;
	buttonGroup1->setLayout(buttonGroup1Layout);
	buttonGroup1Layout->setAlignment( Qt::AlignTop );

	mMessage = new QLineEdit( this );
	buttonGroup1Layout->addWidget( mMessage, 0, 0 );

	mThisTime = new QRadioButton( this);
	mThisTime->setEnabled( TRUE );
	mThisTime->setChecked( TRUE );

	buttonGroup1Layout->addWidget( mThisTime, 0, 1 );

	mAlways = new QRadioButton( mThisTime );
	mAlways->setEnabled( TRUE );

	buttonGroup1Layout->addWidget( mAlways, 1, 1 );

	mDefault = new QRadioButton( mThisTime );
	mDefault->setEnabled( TRUE );

	buttonGroup1Layout->addWidget( mDefault, 2, 1 );
	spacer1 = new QSpacerItem( 20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding );
	buttonGroup1Layout->addItem( spacer1, 1, 2, 1, 1 );


	layout1 = new QHBoxLayout;
	TickerDialogLayout->addLayout( layout1 );

	spacer2 = new QSpacerItem( 121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	layout1->addItem( spacer2 );

    mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	layout1->addWidget( mButtonBox);

	languageChange();
	resize( QSize(400, 152).expandedTo(minimumSizeHint()) );

	// signals and slots connections
	connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
	connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
TickerDialog::~TickerDialog()
{
// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void TickerDialog::languageChange()
{
	setWindowTitle( tr( "Set ticker..." ) );
	buttonGroup1->setTitle( tr( "Set ticker to:" ) );
	mThisTime->setText( tr( "Just this time" ) );
	mAlways->setText( tr( "Always for this room" ) );
	mDefault->setText( tr( "Default for all rooms" ) );
}

