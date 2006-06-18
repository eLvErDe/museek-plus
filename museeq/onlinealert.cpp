
#include "onlinealert.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>


/*
 *  Constructs a OnlineAlert as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
OnlineAlert::OnlineAlert( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "OnlineAlert" );
    OnlineAlertLayout = new QVBoxLayout( this, 11, 6, "OnlineAlertLayout"); 

    mLabel = new QLabel( this, "mLabel" );
    OnlineAlertLayout->addWidget( mLabel );

    frame3 = new QFrame( this, "frame3" );
    frame3->setFrameShape( QFrame::HLine );
    frame3->setFrameShadow( QFrame::Raised );
    OnlineAlertLayout->addWidget( frame3 );

    layout3 = new QHBoxLayout( 0, 0, 6, "layout3"); 
    spacer1 = new QSpacerItem( 111, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer1 );

    mRemove = new QPushButton( this, "mRemove" );
    layout3->addWidget( mRemove );

    mOK = new QPushButton( this, "mOK" );
    layout3->addWidget( mOK );
    OnlineAlertLayout->addLayout( layout3 );
    languageChange();
    resize( QSize(281, 92).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( mRemove, SIGNAL( clicked() ), this, SLOT( mRemove_clicked() ) );
    connect( mOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
OnlineAlert::~OnlineAlert()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OnlineAlert::languageChange()
{
    setCaption( tr( "Online alert" ) );
    mLabel->setText( QString::null );
    mRemove->setText( tr( "&Remove" ) );
    mRemove->setAccel( QKeySequence( tr( "Alt+R" ) ) );
    mOK->setText( tr( "&OK" ) );
    mOK->setAccel( QKeySequence( tr( "Alt+O" ) ) );
}


#include "museeq.h"
#include <qdatetime.h>

void OnlineAlert::setUser( const QString &user )
{
    mUser = user;
    connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(slotUserStatus(const QString&, uint)));
}


void OnlineAlert::slotUserStatus( const QString & user, uint status )
{
    if (museeq->mOnlineAlert)
	return;
    if(user == mUser && (status == 2 || isShown()))
    {
	QString s = (status == 0) ? QT_TR_NOOP("offline") : ((status == 1) ? QT_TR_NOOP("away") : QT_TR_NOOP("online"));
	QString t = QTime::currentTime().toString();
	mLabel->setText(QString(tr("%1 user %2 is now %3")).arg(t).arg(mUser).arg(s));
	if(isShown())
	    raise();
	else
	    show();
    }
}

void OnlineAlert::mRemove_clicked()
{
    emit removeAlert(mUser);
    accept();
}
