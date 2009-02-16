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

#ifndef TICKERDIALOG_H
#define TICKERDIALOG_H

#include <QDialog>

class QGroupBox;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QRadioButton;
class QLineEdit;
class QPushButton;
class QDialogButtonBox;
class QAbstractButton;

class TickerDialog : public QDialog
{
    Q_OBJECT

public:
    TickerDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~TickerDialog();

    QGroupBox* buttonGroup1;
    QRadioButton* mThisTime;
    QRadioButton* mAlways;
    QRadioButton* mDefault;
    QLineEdit* mMessage;
    QDialogButtonBox * mButtonBox;

protected:
    QVBoxLayout* TickerDialogLayout;
    QGridLayout* buttonGroup1Layout;
    QSpacerItem* spacer1;
    QHBoxLayout* layout1;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};

#endif // TICKERDIALOG_H
