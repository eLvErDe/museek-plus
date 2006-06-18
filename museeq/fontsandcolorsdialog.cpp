/* Ported from the uic and moc generated cpp FontsAndColorsDlg*/

#include "fontsandcolorsdialog.h"
#include <qvariant.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qcolordialog.h>
#include <qcolor.h>
#include <qfontdialog.h>
#include <qvariant.h>

/*
 *  Constructs a FontsAndColorsDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
FontsAndColorsDialog::FontsAndColorsDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "FontsAndColorsDialog" );

    mOK = new QPushButton( this, "mOK" );
    mOK->setGeometry( QRect( 200, 480, 111, 31 ) );

    mCancel = new QPushButton( this, "mCancel" );
    mCancel->setGeometry( QRect( 330, 481, 101, 30 ) );
    mCancel->setDefault( TRUE );

    QWidget* privateLayoutWidget = new QWidget( this, "layout3" );
    privateLayoutWidget->setGeometry( QRect( 10, 10, 440, 460 ) );
    layout3 = new QGridLayout( privateLayoutWidget, 1, 1, 11, 6, "layout3"); 

    SRemoteText = new QLineEdit( privateLayoutWidget, "SRemoteText" );

    layout3->addWidget( SRemoteText, 1, 1 );

    SNicknameText = new QLineEdit( privateLayoutWidget, "SNicknameText" );

    layout3->addWidget( SNicknameText, 2, 1 );

    STrustedText = new QLineEdit( privateLayoutWidget, "STrustedText" );

    layout3->addWidget( STrustedText, 5, 1 );

    BannedColorButton = new QPushButton( privateLayoutWidget, "BannedColorButton" );

    layout3->addWidget( BannedColorButton, 4, 2 );

    TimeFontButton = new QPushButton( privateLayoutWidget, "TimeFontButton" );

    layout3->addWidget( TimeFontButton, 7, 2 );

    MessageFontButton = new QPushButton( privateLayoutWidget, "MessageFontButton" );

    layout3->addWidget( MessageFontButton, 8, 2 );

    textLabel1_6 = new QLabel( privateLayoutWidget, "textLabel1_6" );

    layout3->addWidget( textLabel1_6, 6, 0 );

    SBannedText = new QLineEdit( privateLayoutWidget, "SBannedText" );

    layout3->addWidget( SBannedText, 4, 1 );

    STimeText = new QLineEdit( privateLayoutWidget, "STimeText" );

    layout3->addWidget( STimeText, 6, 1 );

    textLabel1_7 = new QLabel( privateLayoutWidget, "textLabel1_7" );

    layout3->addWidget( textLabel1_7, 7, 0 );

    textLabel1 = new QLabel( privateLayoutWidget, "textLabel1" );

    layout3->addWidget( textLabel1, 0, 0 );

    MeColorButton = new QPushButton( privateLayoutWidget, "MeColorButton" );

    layout3->addWidget( MeColorButton, 0, 2 );

    TrustColorButton = new QPushButton( privateLayoutWidget, "TrustColorButton" );

    layout3->addWidget( TrustColorButton, 5, 2 );

    SMessageFont = new QLineEdit( privateLayoutWidget, "SMessageFont" );

    layout3->addWidget( SMessageFont, 8, 1 );

    NicknameColorButton = new QPushButton( privateLayoutWidget, "NicknameColorButton" );

    layout3->addWidget( NicknameColorButton, 2, 2 );

    TimeColorButton = new QPushButton( privateLayoutWidget, "TimeColorButton" );

    layout3->addWidget( TimeColorButton, 6, 2 );

    textLabel1_5 = new QLabel( privateLayoutWidget, "textLabel1_5" );

    layout3->addWidget( textLabel1_5, 5, 0 );

    remote_text = new QLabel( privateLayoutWidget, "remote_text" );

    layout3->addWidget( remote_text, 1, 0 );

    textLabel1_8 = new QLabel( privateLayoutWidget, "textLabel1_8" );

    layout3->addWidget( textLabel1_8, 8, 0 );

    SMeText = new QLineEdit( privateLayoutWidget, "SMeText" );

    layout3->addWidget( SMeText, 0, 1 );

    textLabel1_3 = new QLabel( privateLayoutWidget, "textLabel1_3" );

    layout3->addWidget( textLabel1_3, 3, 0 );

    BuddiedColorButton = new QPushButton( privateLayoutWidget, "BuddiedColorButton" );

    layout3->addWidget( BuddiedColorButton, 3, 2 );

    textLabel1_2 = new QLabel( privateLayoutWidget, "textLabel1_2" );

    layout3->addWidget( textLabel1_2, 2, 0 );

    textLabel1_4 = new QLabel( privateLayoutWidget, "textLabel1_4" );

    layout3->addWidget( textLabel1_4, 4, 0 );

    STimeFont = new QLineEdit( privateLayoutWidget, "STimeFont" );

    layout3->addWidget( STimeFont, 7, 1 );

    SBuddiedText = new QLineEdit( privateLayoutWidget, "SBuddiedText" );

    layout3->addWidget( SBuddiedText, 3, 1 );

    RemoteColorButton = new QPushButton( privateLayoutWidget, "RemoteColorButton" );

    layout3->addWidget( RemoteColorButton, 1, 2 );
    languageChange();
    resize( QSize(463, 514).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( mOK, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( mCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( MeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_me() ) );
    connect( BannedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_banned() ) );
    connect( BuddiedColorButton, SIGNAL( clicked() ), this, SLOT( color_text_buddied() ) );
    connect( MessageFontButton, SIGNAL( clicked() ), this, SLOT( font_text_message() ) );
    connect( NicknameColorButton, SIGNAL( clicked() ), this, SLOT( color_text_nickname() ) );
    connect( RemoteColorButton, SIGNAL( clicked() ), this, SLOT( color_text_remote() ) );
    connect( TimeColorButton, SIGNAL( clicked() ), this, SLOT( color_text_time() ) );
    connect( TrustColorButton, SIGNAL( clicked() ), this, SLOT( color_text_trusted() ) );
    connect( TimeFontButton, SIGNAL( clicked() ), this, SLOT( font_text_time() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
FontsAndColorsDialog::~FontsAndColorsDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void FontsAndColorsDialog::languageChange()
{
    setCaption( tr( "Fonts & Colors" ) );
    mOK->setText( tr( "OK" ) );
    mCancel->setText( tr( "Cancel" ) );
    BannedColorButton->setText( tr( "Pick Color" ) );
    TimeFontButton->setText( tr( "Pick Font" ) );
    MessageFontButton->setText( tr( "Pick Font" ) );
    textLabel1_6->setText( tr( "Time & Brackets Text Color" ) );
    textLabel1_7->setText( tr( "Time & Brackets Font" ) );
    textLabel1->setText( tr( "/Me Text" ) );
    MeColorButton->setText( tr( "Pick Color" ) );
    TrustColorButton->setText( tr( "Pick Color" ) );
    NicknameColorButton->setText( tr( "Pick Color" ) );
    TimeColorButton->setText( tr( "Pick Color" ) );
    textLabel1_5->setText( tr( "Trusted Users" ) );
    remote_text->setText( tr( "Remote Text" ) );
    textLabel1_8->setText( tr( "Message Font" ) );
    textLabel1_3->setText( tr( "Buddied Users" ) );
    BuddiedColorButton->setText( tr( "Pick Color" ) );
    textLabel1_2->setText( tr( "My Text" ) );
    textLabel1_4->setText( tr( "Banned Users" ) );
    RemoteColorButton->setText( tr( "Pick Color" ) );
}

void FontsAndColorsDialog::color_text_me()
{
   QColor c = QColorDialog::getColor( SMeText->text(), this );
    if ( c.isValid() )
	SMeText->setText(c.name());
}


void FontsAndColorsDialog::color_text_buddied()
{
   QColor c = QColorDialog::getColor( SBuddiedText->text(), this );
    if ( c.isValid() )
	SBuddiedText->setText(c.name());
}


void FontsAndColorsDialog::color_text_nickname()
{
   QColor c = QColorDialog::getColor( SNicknameText->text(), this );
    if ( c.isValid() )
	SNicknameText->setText(c.name());
}


void FontsAndColorsDialog::color_text_banned()
{
   QColor c = QColorDialog::getColor( SBannedText->text(), this );
    if ( c.isValid() )
	SBannedText->setText(c.name());
}




void FontsAndColorsDialog::color_text_remote()
{
   QColor c = QColorDialog::getColor( SRemoteText->text(), this );
    if ( c.isValid() )
	SRemoteText->setText(c.name());
}


void FontsAndColorsDialog::color_text_time()
{
   QColor c = QColorDialog::getColor( STimeText->text(), this );
    if ( c.isValid() )
	STimeText->setText(c.name());
}

void FontsAndColorsDialog::color_text_trusted()
{
   QColor c = QColorDialog::getColor( STrustedText->text(), this );
    if ( c.isValid() )
	STrustedText->setText(c.name());
}

void FontsAndColorsDialog::font_text_time()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, STimeFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		STimeFont->setText(c);

    } 
}


void FontsAndColorsDialog::font_text_message()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, SMessageFont );
    if ( ok ) {
		QVariant s (font.pointSize());
 		QVariant w ( font.weight());
		QString c = ("font-family:"+ font.family() +";weight:"+w.toString()+";font-size:"+ s.toString()+"pt");
		SMessageFont->setText(c);

    } 
}
