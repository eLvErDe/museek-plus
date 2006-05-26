/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/fontsandcolorsdlg.ui'
**
** Created: Sun May 14 16:22:13 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef FONTSANDCOLORSDLG_H
#define FONTSANDCOLORSDLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QLineEdit;
class QLabel;

class FontsAndColorsDlg : public QDialog
{
    Q_OBJECT

public:
    FontsAndColorsDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~FontsAndColorsDlg();

    QPushButton* mOK;
    QPushButton* mCancel;
    QLineEdit* SRemoteText;
    QLineEdit* SNicknameText;
    QLineEdit* STrustedText;
    QPushButton* BannedColorButton;
    QPushButton* TimeFontButton;
    QPushButton* MessageFontButton;
    QLabel* textLabel1_6;
    QLineEdit* SBannedText;
    QLineEdit* STimeText;
    QLabel* textLabel1_7;
    QLabel* textLabel1;
    QPushButton* MeColorButton;
    QPushButton* TrustColorButton;
    QLineEdit* SMessageFont;
    QPushButton* NicknameColorButton;
    QPushButton* TimeColorButton;
    QLabel* textLabel1_5;
    QLabel* remote_text;
    QLabel* textLabel1_8;
    QLineEdit* SMeText;
    QLabel* textLabel1_3;
    QPushButton* BuddiedColorButton;
    QLabel* textLabel1_2;
    QLabel* textLabel1_4;
    QLineEdit* STimeFont;
    QLineEdit* SBuddiedText;
    QPushButton* RemoteColorButton;

public slots:
    virtual void color_text_me();
    virtual void color_text_buddied();
    virtual void color_text_nickname();
    virtual void color_text_banned();
    virtual void color_text_remote();
    virtual void color_text_time();
    virtual void color_text_trusted();
    virtual void font_text_time();
    virtual void font_text_message();

protected:
    QGridLayout* layout3;

protected slots:
    virtual void languageChange();

};

#endif // FONTSANDCOLORSDLG_H
