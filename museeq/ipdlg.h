/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/ipdlg.ui'
**
** Created: Sun May 14 16:22:29 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef IPDLG_H
#define IPDLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;
class QPushButton;

class IPDlg : public QDialog
{
    Q_OBJECT

public:
    IPDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~IPDlg();

    QListView* mIPListView;
    QPushButton* mOK;
    QPushButton* pushButton4;

protected:
    QGridLayout* IPDlgLayout;
    QSpacerItem* spacer5;

protected slots:
    virtual void languageChange();

};

#endif // IPDLG_H
