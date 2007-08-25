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
class QTextEdit;
class QLineEdit;
class QListView;
class QListViewItem;
class QListView;
class QSpinBox;
class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QProcess;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
	SettingsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~SettingsDialog();
	
	QTabWidget* mTabHolder, * mMuseeqTabs, * mMuseekdTabs;
	QPushButton* mOK, * mSave, * mCancel;
	QPushButton* SConnect;
	QPushButton* SDisconnect;
	QPushButton* SReloadShares;
	QPushButton* SIncompleteButton;
	QPushButton* SDownloadButton;
	QPushButton* SConfigButton;
	
	QPushButton* NSharesRefresh, * NSharesRescan, * NSharesAdd, * NSharesRemove, * NSharesUpdate;

	QPushButton* BSharesRefresh, * BSharesRescan, * BSharesAdd, * BSharesRemove, * BSharesUpdate;
	
	QWidget* sharesTab, * usersTab, * serverTab, * ColorsAndFontsTab, * AppearanceTab;
	QWidget* connectionsTab, * LoggingTab, * UserInfoTab, * ProtocolTab;
	
	QComboBox* SFileSystemEncoding, * SNetworkEncoding;
	QLabel* fEncodingLabel, * nEncodingLabel;
	QLabel* serverPortLabel;
	QLabel* serverHostLabel;
	QLabel* usernamelabel, *configLabel;
	QLabel* passwordLabel;
	QLabel* instructionsLabel;
	QLabel* downloadLabel;
	QLabel* incompleteLabel;
	
	QLabel* listenPortsLabel, * listenPortsStartLabel, * listenPortsEndLabel;

	QPushButton* LoggingPrivateButton, * LoggingRoomButton;
	QLineEdit* LoggingPrivateDir, * LoggingRoomDir;
	
	QTextEdit* mInfoText;
	QButtonGroup* buttonGroup1;
	QRadioButton* mClear;
	QRadioButton* mDontTouch;
	QLineEdit* mImage;
	QRadioButton* mUpload;
	QPushButton* mBrowse;

	QSpinBox* SServerPort;
	QButtonGroup* buttonGroup2;
	QRadioButton* SActive;
	QRadioButton* SPassive;
	QLineEdit* SSoulseekPassword;
	QLineEdit* SSoulseekUsername;
	QLineEdit* SServerHost;
	QLineEdit* SDownDir;
	QLineEdit* SIncompleteDir, * SConfigFile;

	QPushButton* mNewHandler;
	QPushButton* mModifyHandler;
	QListView* mProtocols;
	
	QLabel * TimeFontLabel, * TimeColorLabel, * MeColorLabel, * MessageFontLabel, * BuddiedColorLabel, * LocalTextLabel, * TrustColorLabel, * BannedColorLabel, * RemoteColorLabel;
	QPushButton * MeColorButton, * TimeFontButton, * NicknameColorButton, * MessageFontButton, * BuddiedColorButton, * TrustColorButton, * BannedColorButton, * RemoteColorButton, * TimeColorButton;
	QLineEdit* SRemoteText, * SNicknameText, * STrustedText, * SBannedText, * STimeText, * SMessageFont, * SMeText, * STimeFont, * SBuddiedText;
	
	QSpinBox* CPortStart, * CPortEnd;
	QCheckBox* SBuddiesPrivileged, * SOnlineAlerts,* SShareBuddiesOnly, * STrustedUsers, * SBuddiesShares, * SUserWarnings, * SIPLog, * LoggingPrivate, * LoggingRooms;
	QListView* ListNormalShares, * ListBuddyShares;
	
public slots:
	void SConnect_clicked();
	void SDisconnect_clicked();
	void SReloadShares_clicked();
	void save();
	virtual void SDownload_clicked();
	virtual void SIncomplete_clicked();
	
	void BuddySharesAdd();
	void BuddySharesRefresh();
	void BuddySharesRemove();
	void BuddySharesRescan();
	void BuddySharesUpdate();

	void NormalSharesAdd();
	void NormalSharesRefresh();
	void NormalSharesRemove();
	void NormalSharesRescan();
	void NormalSharesUpdate();
	
	void UserImageBrowse_clicked();

	void mNewHandler_clicked();
	void mModifyHandler_clicked();
	void mProtocols_itemRenamed( QListViewItem * item, int col );
	 
	void slotConfigChanged(const QString&, const QString&, const QString&);
	void SBuddiesSharesToggled(bool);
	void readNormal();
	void readBuddy();
	void PrivateDirSelect();
	void RoomDirSelect();

	virtual void color_text_me();
	virtual void color_text_buddied();
	virtual void color_text_nickname();
	virtual void color_text_banned();
	virtual void color_text_remote();
	virtual void color_text_time();
	virtual void color_text_trusted();
	virtual void font_text_time();
	virtual void font_text_message();
protected:
	QHBoxLayout* buttonsLayout;
	QSpacerItem* spacer14, * spacer16, * spacer15, * spacer13, * spacer5, * protocolSpacer;
	QGridLayout* ServerGrid, * SharesGrid, *ConnectionsGrid, *UsersGrid,
	*LoggingGrid, * UserInfoGrid, * buttonGroup1Layout, * ProtocolGrid,
	* ColorsGrid, * AppearanceGrid;
	QProcess * proc1;
	QProcess * proc2;	

protected slots:
	virtual void languageChange();

	void SConfig_clicked();
	void MuscanBuddyDone();
	void MuscanNormalDone();
	void EnableNormalButtons(bool);
	void EnableBuddyButtons(bool);
};

#endif // SETTINGSDIALOG_H
