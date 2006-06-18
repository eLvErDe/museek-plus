
#ifndef TICKERDIALOG_H
#define TICKERDIALOG_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QRadioButton;
class QLineEdit;
class QPushButton;

class TickerDialog : public QDialog
{
    Q_OBJECT

public:
    TickerDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~TickerDialog();

    QButtonGroup* buttonGroup1;
    QRadioButton* mThisTime;
    QRadioButton* mAlways;
    QRadioButton* mDefault;
    QLineEdit* mMessage;
    QPushButton* mCancel;
    QPushButton* pushButton1;

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
