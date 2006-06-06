#ifndef CONNECTDIALOG_H  
#define CONNECTDIALOG_H 
#include <qdialog.h> 
class QCheckBox; 
class QLabel; 
class QLineEdit; 
class QPushButton; 
class QVBoxLayout;  
class QHBoxLayout;  
class QHBox;  
class QGridLayout;  
class QSpacerItem;  
class QPushButton;  
class QLineEdit;
class QLabel; 
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QRadioButton;
class QSpacerItem;
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
	QLineEdit *mMuseekConfig;
	QLabel *configLabel;
	QButtonGroup *buttonGroup4;
	QComboBox *mAddress;
	QLineEdit *mPassword;
	QRadioButton *mUnix;
	QRadioButton *mTCP;
	QLabel *textLabel2;
	QLabel *textLabel1;
	QCheckBox *mSavePassword;
	QCheckBox *mAutoStartDaemon;
	QCheckBox *mShutDownDaemonOnExit;
	QHBoxLayout *extraLayout, *controldLayout, *connectLayout;
	QHBox *box1, *box2, *box3;
	bool extra;
	QSpacerItem* spacer1, * spacer2, * spacer3;
public slots:   
	virtual void startDaemon(); 
	virtual void selectConfig();    
	virtual void stopDaemon();  
	virtual void save();
	virtual void extraOptions();

private: 
	QLabel *label; 
	QLineEdit *lineEdit; 
	QCheckBox *caseCheckBox;
	QCheckBox *backwardCheckBox;

}; 
#endif 

