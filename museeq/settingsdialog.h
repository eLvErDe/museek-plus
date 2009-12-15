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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QtNetwork/QTcpSocket>

class QMenu;
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
class QTreeWidget;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QCloseEvent;
class QDialogButtonBox;
class QAbstractButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
	SettingsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
	~SettingsDialog();

	bool areSharesDirty() {return mSharesDirty;};
	void setSharesDirty(bool dirty) {mSharesDirty = dirty;};

	QTabWidget* mTabHolder, * mMuseeqTabs, * mMuseekdTabs;
	QPushButton* SConnect;
	QPushButton* SDisconnect;
	QPushButton* SIncompleteButton;
	QPushButton* SDownloadButton;
	QPushButton* SConfigButton;

	QPushButton* NSharesRefresh, * NSharesRescan, * NSharesAdd, * NSharesRemove, * NSharesUpdate;

	QPushButton* BSharesRefresh, * BSharesRescan, * BSharesAdd, * BSharesRemove, * BSharesUpdate;

	QWidget * mSharesTab, * mUsersTab, * mServerTab, * mColorsAndFontsTab, * mAppearanceTab, * mConnectionsTab, * mChatTab, * mUserInfoTab, * mProtocolTab, * mDConnectionTab;

	QComboBox* SFileSystemEncoding, * SNetworkEncoding;
	QLabel* fEncodingLabel, * nEncodingLabel;
	QLabel* serverPortLabel;
	QLabel* serverHostLabel;
	QLabel* usernamelabel, *configLabel;
	QLabel* passwordLabel;
	QLabel* changePasswordLabel;
	QLabel* instructionsLabel;
	QLabel* downloadLabel;
	QLabel* incompleteLabel;

	QLabel* listenPortsLabel, * listenPortsStartLabel, * listenPortsEndLabel;

	QPushButton* LoggingPrivateButton, * LoggingRoomButton;
	QLineEdit* LoggingPrivateDir, * LoggingRoomDir;

	QTextEdit* mInfoText;
	QRadioButton* mClear;
	QRadioButton* mDontTouch;
	QLineEdit* mImage;
	QRadioButton* mUpload;
	QPushButton* mBrowse;

	QSpinBox* SServerPort;
	QRadioButton* SActive;
	QRadioButton* SPassive;
	QLineEdit* SSoulseekPassword;
	QLineEdit* SSoulseekUsername;
	QPushButton * SSoulseekChangePassword;
	QLineEdit* SServerHost;
	QLineEdit* SDownDir;
	QLineEdit* SIncompleteDir;

	QPushButton* mNewHandler;
	QPushButton* mModifyHandler;
	QTreeWidget* mProtocols;

	QLabel * TimeFontLabel, * TimeColorLabel, * MeColorLabel, * MessageFontLabel, * BuddiedColorLabel, * LocalTextLabel, * TrustColorLabel, * BannedColorLabel, * RemoteColorLabel, * TickerLengthLabel;
	QPushButton * MeColorButton, * TimeFontButton, * NicknameColorButton, * MessageFontButton, * BuddiedColorButton, * TrustColorButton, * BannedColorButton, * RemoteColorButton, * TimeColorButton;
	QLineEdit* SRemoteText, * SNicknameText, * STrustedText, * SBannedText, * STimeText, * SMessageFont, * SMeText, * STimeFont, * SBuddiedText;

	QSpinBox* CPortStart, * CPortEnd, * TickerLength;
	QCheckBox* SBuddiesPrivileged, * SOnlineAlerts,* SShareBuddiesOnly, * STrustedUsers, * SPrivRoom, * SBuddiesShares, * SIPLog, * LoggingPrivate, * LoggingRooms, * IconsAlignment, * mToggleTickers, * mToggleCountry, * mToggleSaveTransfersLayout, * mToggleSaveAllLayouts, * mToggleLog, * mToggleTimestamps, * mToggleTrayicon, * mAutoClearFinishedDownloads, * mAutoClearFinishedUploads, * mAutoRetryDownloads;
	QTreeWidget* ListNormalShares, * ListBuddyShares;
	QLabel* mBlacklistLabel;
	QLineEdit* mBlacklistDownload;

    QComboBox * mDAddress, * mDConnectType;
    QPushButton * mSelectConfigFileButton, * mStartDaemonButton, * mStopDaemonButton/*, * mMusetupButton*/, * mConnectToDaemonButton, * mDisconnectFromDaemonButton, * mIconTheme;
    QLineEdit * mDPassword, * mMuseekConfigFile;
    QCheckBox * mAutoStartDaemon, * mDAutoConnect, * mShowExitDialog, * mShutDownDaemonOnExit, * mDSavePassword;

    QDialogButtonBox * mButtonBox;

public slots:
    void changeSlskPassword();
	void SConnect_clicked();
	void SDisconnect_clicked();
	void save();
	virtual void SDownload_clicked();
	virtual void SIncomplete_clicked();
	virtual void startDaemon();
	virtual void stopDaemon();
	void showEvent( QShowEvent * event );

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
	void mProtocols_itemDelete();

	void slotNewPasswordSet(const QString&);
	void slotConfigChanged(const QString&, const QString&, const QString&);
	void SBuddiesSharesToggled(bool);
	void readNormal();
	void readBuddy();
	void PrivateDirSelect();
	void RoomDirSelect();

    void loadSettings();

    void selectConfig();

    void buttonClicked(QAbstractButton*);
	void slotAddressActivated(const QString&);
	void slotAddressChanged(const QString&);

	bool getPrivRoomEnabled();
	void setPrivRoomEnabled(bool);
	void setPrivRoomEnabledFromServer(bool);

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
    void populateDConnectionTab();

	QAction * ActionDeleteHandler;
	QMenu * mProtocolsMenu;
	QHBoxLayout* buttonsLayout;
	QSpacerItem * spacer5, * protocolSpacer, * spacerServer;
	QGridLayout* ServerGrid, * SharesGrid, *ConnectionsGrid, *UsersGrid,
	*mChatGrid, * UserInfoGrid, * buttonGroup1Layout, * ProtocolGrid,
	* ColorsGrid, * AppearanceGrid, * mDConnectionGrid;
	QProcess* proc1;
	QProcess * proc2;

protected slots:
	virtual void languageChange();
	void slotProtocolContextMenu(const QPoint&);
	void EnableNormalButtons(bool);
	void EnableBuddyButtons(bool);
	void finishedListNormal(int, QProcess::ExitStatus);
	void finishedListBuddy(int, QProcess::ExitStatus);
	void finishedNormal(int, QProcess::ExitStatus);
	void finishedBuddy(int, QProcess::ExitStatus);
	void acceptSettings();
	void rejectSettings();
	void clearSockets();
	//void launchMusetup();
	void toggleSavePassword(bool);
	//void musetupError( QProcess::ProcessError);
	void loggedIn(bool, const QString&);
	void slotDisconnected();
	void slotError(QAbstractSocket::SocketError);
	void connectionTypeChanged(int);
	void slotConnectedToServer(bool);

protected:
	void closeEvent(QCloseEvent *);

private:
    bool mSharesDirty, mPrivRoomEnabled;
    QLabel * mHostLabel, * mDPasswordLabel, * mConfigFileLabel;
    QProcess * mSetupProc;
};

#endif // SETTINGSDIALOG_H
