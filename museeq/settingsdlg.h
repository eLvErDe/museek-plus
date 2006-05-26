/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/settingsdlg.ui'
**
** Created: Sun May 14 16:22:32 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QTabWidget;
class QWidget;
class QLabel;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QButtonGroup;
class QRadioButton;
class QCheckBox;

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    SettingsDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SettingsDlg();

    QPushButton* mOK;
    QPushButton* mCancel;
    QTabWidget* tabWidget2;
    QWidget* tab;
    QLabel* textLabel6;
    QComboBox* SDefaultEncoding;
    QPushButton* SConnect;
    QPushButton* SDisconnect;
    QLabel* textLabel4;
    QLabel* textLabel3;
    QLabel* usernamelabel;
    QLabel* textLabel2;
    QLineEdit* SServerHost;
    QSpinBox* SServerPort;
    QLineEdit* SSoulseekPassword;
    QLineEdit* SSoulseekUsername;
    QWidget* tab_2;
    QLabel* textLabel5;
    QButtonGroup* buttonGroup2;
    QRadioButton* SActive;
    QRadioButton* SPassive;
    QPushButton* SReloadShares;
    QWidget* TabPage;
    QCheckBox* SBuddiesPrivileged;
    QCheckBox* SShareBuddiesOnly;
    QCheckBox* STrustedUsers;
    QCheckBox* SBuddiesShares;

public slots:
    void SConnect_clicked();
    void SDisconnect_clicked();
    void SReloadShares_clicked();

protected:
    QHBoxLayout* layout13;
    QSpacerItem* spacer5;
    QHBoxLayout* layout17;
    QVBoxLayout* layout9;
    QGridLayout* layout5;
    QSpacerItem* spacer14;
    QGridLayout* layout18;
    QSpacerItem* spacer16;
    QSpacerItem* spacer15_2;
    QVBoxLayout* layout32;

protected slots:
    virtual void languageChange();

};

#endif // SETTINGSDLG_H
