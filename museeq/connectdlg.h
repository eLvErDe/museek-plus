/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/connectdlg.ui'
**
** Created: Sun May 14 16:22:09 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CONNECTDLG_H
#define CONNECTDLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QLabel;
class QLineEdit;
class QRadioButton;
class QComboBox;
class QCheckBox;
class QPushButton;

class ConnectDlg : public QDialog
{
    Q_OBJECT

public:
    ConnectDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ConnectDlg();

    QButtonGroup* buttonGroup4;
    QLabel* textLabel2;
    QLineEdit* mPassword;
    QLabel* textLabel1;
    QRadioButton* mUnix;
    QRadioButton* mTCP;
    QComboBox* mAddress;
    QCheckBox* mLocal;
    QPushButton* pushButton1;
    QPushButton* pushButton2;

protected:
    QVBoxLayout* ConnectDlgLayout;
    QGridLayout* buttonGroup4Layout;
    QHBoxLayout* layout3;
    QSpacerItem* spacer2;
    QHBoxLayout* layout2;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

};

#endif // CONNECTDLG_H
