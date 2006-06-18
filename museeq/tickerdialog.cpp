
#include "tickerdialog.h"

#include <qvariant.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a TickerDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
TickerDialog::TickerDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "TickerDialog" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 400, 0 ) );
    TickerDialogLayout = new QVBoxLayout( this, 11, 6, "TickerDialogLayout"); 

    buttonGroup1 = new QButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setColumnLayout(0, Qt::Vertical );
    buttonGroup1->layout()->setSpacing( 6 );
    buttonGroup1->layout()->setMargin( 11 );
    buttonGroup1Layout = new QGridLayout( buttonGroup1->layout() );
    buttonGroup1Layout->setAlignment( Qt::AlignTop );

    mThisTime = new QRadioButton( buttonGroup1, "mThisTime" );
    mThisTime->setEnabled( TRUE );
    mThisTime->setChecked( TRUE );

    buttonGroup1Layout->addWidget( mThisTime, 0, 1 );

    mAlways = new QRadioButton( buttonGroup1, "mAlways" );
    mAlways->setEnabled( TRUE );

    buttonGroup1Layout->addWidget( mAlways, 1, 1 );

    mDefault = new QRadioButton( buttonGroup1, "mDefault" );
    mDefault->setEnabled( TRUE );

    buttonGroup1Layout->addWidget( mDefault, 2, 1 );
    spacer1 = new QSpacerItem( 20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding );
    buttonGroup1Layout->addMultiCell( spacer1, 1, 2, 0, 0 );

    mMessage = new QLineEdit( buttonGroup1, "mMessage" );

    buttonGroup1Layout->addWidget( mMessage, 0, 0 );
    TickerDialogLayout->addWidget( buttonGroup1 );

    layout1 = new QHBoxLayout( 0, 0, 6, "layout1"); 
    spacer2 = new QSpacerItem( 121, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer2 );

    mCancel = new QPushButton( this, "mCancel" );
    layout1->addWidget( mCancel );

    pushButton1 = new QPushButton( this, "pushButton1" );
    pushButton1->setDefault( TRUE );
    layout1->addWidget( pushButton1 );
    TickerDialogLayout->addLayout( layout1 );
    languageChange();
    resize( QSize(400, 152).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( pushButton1, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
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
    setCaption( tr( "Set ticker..." ) );
    buttonGroup1->setTitle( tr( "Set ticker to:" ) );
    mThisTime->setText( tr( "Just this time" ) );
    mAlways->setText( tr( "Always for this room" ) );
    mDefault->setText( tr( "Default for all rooms" ) );
    mCancel->setText( tr( "&Cancel" ) );
    mCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    pushButton1->setText( tr( "&OK" ) );
    pushButton1->setAccel( QKeySequence( tr( "Alt+O" ) ) );
}

