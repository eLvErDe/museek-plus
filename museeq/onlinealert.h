/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/onlinealert.ui'
**
** Created: Sun May 14 16:23:27 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ONLINEALERT_H
#define ONLINEALERT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QFrame;
class QPushButton;

class OnlineAlert : public QDialog
{
    Q_OBJECT

public:
    OnlineAlert( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~OnlineAlert();

    QLabel* mLabel;
    QFrame* frame3;
    QPushButton* mRemove;
    QPushButton* mOK;

    void setUser( const QString & user );

public slots:
    virtual void mRemove_clicked();

signals:
    void removeAlert(const QString&);

protected:
    QString mUser;

    QVBoxLayout* OnlineAlertLayout;
    QHBoxLayout* layout3;
    QSpacerItem* spacer1;

protected slots:
    virtual void languageChange();

    void slotUserStatus( const QString & user, uint status );


};

#endif // ONLINEALERT_H
