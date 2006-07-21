/****************************************************************************
** Form interface generated from reading ui file 'build-i686-linux/museeq/settingsdlg.ui'
**
** Created: Sun Jun 18 18:01:33 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

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

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
	SettingsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~SettingsDialog();
	
	QTabWidget* mTabHolder;
	QPushButton* mOK, * mSave, * mCancel;
	QPushButton* SConnect;
	QPushButton* SDisconnect;
	QPushButton* SReloadShares;
	QPushButton* SIncompleteButton;
	QPushButton* SDownloadButton;
	QWidget* sharesTab;
	QWidget* usersTab;
	QWidget* serverTab;
	QComboBox* SDefaultEncoding, * SFileSystemEncoding, * SNetworkEncoding;
	QLabel* dEncodingLabel, * fEncodingLabel, * nEncodingLabel;
	QLabel* serverPortLabel;
	QLabel* serverHostLabel;
	QLabel* usernamelabel;
	QLabel* passwordLabel;
	QLabel* instructionsLabel;
	QLabel* downloadLabel;
	QLabel* incompleteLabel;
	QSpinBox* SServerPort;
	QButtonGroup* buttonGroup2;
	QRadioButton* SActive;
	QRadioButton* SPassive;
	QLineEdit* SSoulseekPassword;
	QLineEdit* SSoulseekUsername;
	QLineEdit* SServerHost;
	QLineEdit* SDownDir;
	QLineEdit* SIncompleteDir;
	QCheckBox* SBuddiesPrivileged, * SOnlineAlerts,* SShareBuddiesOnly, * STrustedUsers, * SBuddiesShares, * SUserWarnings, * SIPLog;

public slots:
	void SConnect_clicked();
	void SDisconnect_clicked();
	void SReloadShares_clicked();
	void save();
	virtual void SDownload_clicked();
	virtual void SIncomplete_clicked();

protected:
	QHBoxLayout* buttonsLayout;
	QSpacerItem* spacer14, * spacer16, * spacer15, * spacer13, * spacer5;



protected slots:
    virtual void languageChange();

};

#endif // SETTINGSDIALOG_H
