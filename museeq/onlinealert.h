/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ONLINEALERT_H
#define ONLINEALERT_H

#include <QDialog>
#include <QLayout>

class QSpacerItem;
class QLabel;
class QFrame;
class QPushButton;
class QDialogButtonBox;
class QAbstractButton;

class OnlineAlert : public QDialog
{
    Q_OBJECT

public:
    OnlineAlert( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~OnlineAlert() {};

    void setUser( const QString & user );

public slots:
    virtual void slotRemoveAlert();

signals:
    void removeAlert(const QString&);

protected:
    QString mUser;

    QVBoxLayout* OnlineAlertLayout;
    QHBoxLayout* layout3;
    QSpacerItem* spacer1;

    QLabel* mLabel;
    QFrame* frame3;
    QPushButton* mRemove;
    QDialogButtonBox * mButtonBox;

protected slots:
    virtual void languageChange();

    void slotUserStatus( const QString & user, uint status );


};

#endif // ONLINEALERT_H
