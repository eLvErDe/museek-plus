/* ported from the userinfodlg moc and uic files*/
#include "userinfodialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
// #include "userinfodlg.ui.h"
#include <qfiledialog.h>
#include <qdir.h>

/*
 *  Constructs a UserInfoDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
UserInfoDialog::UserInfoDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "UserInfoDialog" );
    UserInfoDialogLayout = new QGridLayout( this, 1, 1, 11, 6, "UserInfoDialogLayout"); 

    mText = new QTextEdit( this, "mText" );

    UserInfoDialogLayout->addMultiCellWidget( mText, 0, 0, 0, 2 );

    buttonGroup1 = new QButtonGroup( this, "buttonGroup1" );
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

    mBrowse = new QToolButton( buttonGroup1, "mBrowse" );

    buttonGroup1Layout->addWidget( mBrowse, 1, 2 );

    UserInfoDialogLayout->addMultiCellWidget( buttonGroup1, 1, 1, 0, 2 );

    mOK = new QPushButton( this, "mOK" );

    UserInfoDialogLayout->addWidget( mOK, 2, 1 );

    mCancel = new QPushButton( this, "mCancel" );

    UserInfoDialogLayout->addWidget( mCancel, 2, 2 );
    spacer5 = new QSpacerItem( 171, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    UserInfoDialogLayout->addItem( spacer5, 2, 0 );
    languageChange();
    resize( QSize(344, 323).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( mOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( mBrowse, SIGNAL( clicked() ), this, SLOT( mBrowse_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
UserInfoDialog::~UserInfoDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void UserInfoDialog::languageChange()
{
    setCaption( tr( "Change user info" ) );
    buttonGroup1->setTitle( tr( "Image" ) );
    mClear->setText( tr( "Clear" ) );
    mDontTouch->setText( tr( "Don't touch" ) );
    mUpload->setText( tr( "Upload:" ) );
    mBrowse->setText( tr( "..." ) );
    mOK->setText( tr( "OK" ) );
    mCancel->setText( tr( "Cancel" ) );
}


void UserInfoDialog::mBrowse_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), tr("Images (*.png *.gif *.jpg *.jpeg)"), this);
    fd->setMode(QFileDialog::ExistingFile);
    fd->setCaption(tr("Select an Image for you User info"));
    fd->addFilter(tr("All files (*)"));
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	mImage->setText(fd->selectedFile());
	mUpload->toggle();
    }
    
    delete fd;
}
