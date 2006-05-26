/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/userinfodlg.ui'
**
** Created: Sun May 14 16:22:32 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef USERINFODLG_H
#define USERINFODLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTextEdit;
class QButtonGroup;
class QRadioButton;
class QLineEdit;
class QToolButton;
class QPushButton;

class UserInfoDlg : public QDialog
{
    Q_OBJECT

public:
    UserInfoDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UserInfoDlg();

    QTextEdit* mText;
    QButtonGroup* buttonGroup1;
    QRadioButton* mClear;
    QRadioButton* mDontTouch;
    QLineEdit* mImage;
    QRadioButton* mUpload;
    QToolButton* mBrowse;
    QPushButton* mOK;
    QPushButton* mCancel;

public slots:
    void mBrowse_clicked();

protected:
    QGridLayout* UserInfoDlgLayout;
    QSpacerItem* spacer5;
    QGridLayout* buttonGroup1Layout;

protected slots:
    virtual void languageChange();

};

#endif // USERINFODLG_H
