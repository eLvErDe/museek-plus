
#ifndef IPDIALOG_H
#define IPDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;
class QPushButton;

class IPDialog : public QDialog
{
    Q_OBJECT

public:
    IPDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~IPDialog();

    QListView* mIPListView;
    QPushButton* mOK;
    QPushButton* pushButton4;

protected:
    QGridLayout* IPDialogLayout;
    QSpacerItem* spacer5;

protected slots:
    virtual void languageChange();

};

#endif // IPDIALOG_H
