/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/userinfodlg.ui'
**
** Created: Tue Jun 13 20:07:23 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

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

class UserInfoDialog : public QDialog
{
    Q_OBJECT

public:
    UserInfoDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UserInfoDialog();

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
    QGridLayout* UserInfoDialogLayout;
    QSpacerItem* spacer5;
    QGridLayout* buttonGroup1Layout;

protected slots:
    virtual void languageChange();

};

#endif // USERINFODIALOG_H
