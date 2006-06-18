
#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QListView;
class QListViewItem;
class QPushButton;

class ProtocolDialog : public QDialog
{
    Q_OBJECT

public:
    ProtocolDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ProtocolDialog();

    QListView* mProtocols;
    QPushButton* mOk;
    QPushButton* mCancel;
    QPushButton* mNew;

public slots:
    void mNew_clicked();
    void mProtocols_itemRenamed( QListViewItem * item, int col );

protected:
    QGridLayout* ProtocolDialogLayout;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

};

#endif // PROTOCOLDIALOG_H
