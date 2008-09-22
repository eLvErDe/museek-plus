/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
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

#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QLayout>

class QLineEdit;
class QLabel;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QRadioButton;
class QSpacerItem; 
class QPushButton; 
class QCheckBox; 
class QLabel; 
class QLineEdit; 

 class ConnectDialog : public QDialog
{
	Q_OBJECT
public:
	ConnectDialog(QWidget *parent = 0, const char *name = 0);

	QPushButton *startDaemonButton;
	QPushButton *stopDaemonButton;
	QPushButton *connectButton;
	QPushButton *saveButton;
	QPushButton *cancelButton, *mExtra;
	QPushButton *selectButton;
	QPushButton *clearButton;
	QLineEdit *mMuseekConfig;
	QLabel *configLabel;
	QButtonGroup *buttonGroup4;
	QComboBox *mAddress;
	QLineEdit *mPassword;
	QRadioButton *mUnix;
	QRadioButton *mTCP;
	QLabel *HostLabel;
	QLabel *PasswordLabel;
	QCheckBox *mSavePassword, *mAutoStartDaemon, *mShutDownDaemonOnExit, *mAutoConnect;
	QHBoxLayout *extraLayout, *controldLayout, *connectLayout;
	QHBoxLayout *box1,  *box3;
	QVBoxLayout *DaemonBox;
	bool extra;
	QSpacerItem* spacer1, * spacer2, * spacer3;
public slots:
	virtual void startDaemon();
	virtual void selectConfig();
	virtual void stopDaemon();
	virtual void save();
	virtual void extraOptions();

private:
	QWidget * DaemonItems;
	QLabel *label;
	QLineEdit *lineEdit;
	QCheckBox *caseCheckBox;
	QCheckBox *backwardCheckBox;
private slots:
	void clearSockets();
};
#endif

