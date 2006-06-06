#ifndef CONNECTDIALOG_H  
#define CONNECTDIALOG_H 
#include <qdialog.h> 
class QCheckBox; 
class QLabel; 
class QLineEdit; 
class QPushButton; 
class QVBoxLayout;  
class QHBoxLayout;  
class QGridLayout;  
class QSpacerItem;  
class QPushButton;  
class QLineEdit;  
class QLabel; 
class QButtonGroup; 
class QCheckBox;  
class QComboBox;  
class QRadioButton;   
 class ConnectDialog : public QDialog  
{ 
	Q_OBJECT 
public: 
	ConnectDialog(QWidget *parent = 0, const char *name = 0); 

	QPushButton *startDaemonButton;
	QPushButton *stopDaemonButton;
	QPushButton *connectButton;
	QPushButton *saveButton;
	QPushButton *cancelButton;
	QLineEdit *mMuseekConfig;
	QPushButton *selectButton;
	QLabel *configLabel;
	QButtonGroup *buttonGroup4;
	QCheckBox *mSavePassword;
	QComboBox *mAddress;
	QRadioButton *mTCP;
	QLineEdit *mPassword;
	QRadioButton *mUnix;
	QLabel *textLabel2;
	QLabel *textLabel1;
	QCheckBox *mAutoStartDaemon;
	QCheckBox *mShutDownDaemonOnExit;

public slots:   
	virtual void startDaemon(); 
	virtual void selectConfig();    
	virtual void stopDaemon();  
	virtual void save();   
signals: 
	void connectNext(const QString &str, bool caseSensitive); 
	void connectPrev(const QString &str, bool caseSensitive); 

private slots: 
	void connectClicked(); 
	void enableConnectButton(const QString &text); 
private: 
	QLabel *label; 
	QLineEdit *lineEdit; 
	QCheckBox *caseCheckBox;
	QCheckBox *backwardCheckBox;

}; 
#endif 

