
#include "protocoldialog.h"

#include <qvariant.h>
#include <qheader.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a ProtocolDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ProtocolDialog::ProtocolDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ProtocolDialog" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 400, 0 ) );
    ProtocolDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "ProtocolDialogLayout"); 

    mProtocols = new QListView( this, "mProtocols" );
    mProtocols->addColumn( tr( "Protocol" ) );
    mProtocols->addColumn( tr( "Handler" ) );
    mProtocols->setAllColumnsShowFocus( TRUE );

    ProtocolDialogLayout->addMultiCellWidget( mProtocols, 0, 0, 0, 3 );

    mOk = new QPushButton( this, "mOk" );

    ProtocolDialogLayout->addWidget( mOk, 1, 2 );

    mCancel = new QPushButton( this, "mCancel" );

    ProtocolDialogLayout->addWidget( mCancel, 1, 3 );

    mNew = new QPushButton( this, "mNew" );

    ProtocolDialogLayout->addWidget( mNew, 1, 1 );
    spacer1 = new QSpacerItem( 91, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    ProtocolDialogLayout->addItem( spacer1, 1, 0 );
    languageChange();
    resize( QSize(400, 224).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( mOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( mNew, SIGNAL( clicked() ), this, SLOT( mNew_clicked() ) );
    connect( mProtocols, SIGNAL( itemRenamed(QListViewItem*,int) ), this, SLOT( mProtocols_itemRenamed(QListViewItem*,int) ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProtocolDialog::~ProtocolDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ProtocolDialog::languageChange()
{
    setCaption( tr( "Protocol handlers" ) );
    mProtocols->header()->setLabel( 0, tr( "Protocol" ) );
    mProtocols->header()->setLabel( 1, tr( "Handler" ) );
    mOk->setText( tr( "OK" ) );
    mCancel->setText( tr( "Cancel" ) );
    mNew->setText( tr( "New" ) );
}


void ProtocolDialog::mNew_clicked()
{
    QListViewItem* item = new QListViewItem(mProtocols, "", "");
    item->setRenameEnabled(0, true);
    item->setRenameEnabled(1, true);
    item->startRename(0);
}


void ProtocolDialog::mProtocols_itemRenamed( QListViewItem *item, int col)
{
    if(col == 0)
	item->startRename(1);
}
