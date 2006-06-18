/* Ported from ipdlg.ui */
#include "ipdialog.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a IPDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
IPDialog::IPDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "IPDialog" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 400, 0 ) );
    IPDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "IPDialogLayout"); 

    mIPListView = new QListView( this, "mIPListView" );
    mIPListView->addColumn( tr( "User" ) );
    mIPListView->addColumn( tr( "IP" ) );
    mIPListView->addColumn( tr( "Port" ) );
    mIPListView->addColumn( tr( "Hostname" ) );
    mIPListView->setFocusPolicy( QListView::ClickFocus );
    mIPListView->setAllColumnsShowFocus( TRUE );
    mIPListView->setShowSortIndicator( TRUE );
    mIPListView->setDefaultRenameAction( QListView::Accept );

    IPDialogLayout->addMultiCellWidget( mIPListView, 0, 0, 0, 2 );

    mOK = new QPushButton( this, "mOK" );

    IPDialogLayout->addWidget( mOK, 1, 2 );

    pushButton4 = new QPushButton( this, "pushButton4" );

    IPDialogLayout->addWidget( pushButton4, 1, 1 );
    spacer5 = new QSpacerItem( 120, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    IPDialogLayout->addItem( spacer5, 1, 0 );
    languageChange();
    resize( QSize(400, 220).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( pushButton4, SIGNAL( clicked() ), mIPListView, SLOT( clear() ) );
    connect( mOK, SIGNAL( clicked() ), this, SLOT( hide() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
IPDialog::~IPDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void IPDialog::languageChange()
{
    setCaption( tr( "IP Addresses" ) );
    mIPListView->header()->setLabel( 0, tr( "User" ) );
    mIPListView->header()->setLabel( 1, tr( "IP" ) );
    mIPListView->header()->setLabel( 2, tr( "Port" ) );
    mIPListView->header()->setLabel( 3, tr( "Hostname" ) );
    mOK->setText( tr( "&Close" ) );
    mOK->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    pushButton4->setText( tr( "Clear" ) );
}

