/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/protocoldlg.ui'
**
** Created: Sun May 14 16:22:32 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PROTOCOLDLG_H
#define PROTOCOLDLG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;
class QPushButton;

class ProtocolDlg : public QDialog
{
    Q_OBJECT

public:
    ProtocolDlg( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ProtocolDlg();

    QListView* mProtocols;
    QPushButton* mOk;
    QPushButton* mCancel;
    QPushButton* mNew;

public slots:
    void mNew_clicked();
    void mProtocols_itemRenamed( QListViewItem * item, int col );

protected:
    QGridLayout* ProtocolDlgLayout;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

};

#endif // PROTOCOLDLG_H
