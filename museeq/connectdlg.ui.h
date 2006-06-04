#include <qdir.h>
#include "museeq.h"
#include <qfiledialog.h>
void ConnectDlg::startDaemon()
{
    museeq->startDaemon();
}


void ConnectDlg::selectConfig()
{
    QDir dir = QDir::home();
    QFileDialog * fd = new QFileDialog(dir.path()+"/.museekd", "Museek Daemon Config (*.xml)", this);
    fd->setMode(QFileDialog::ExistingFile);
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	mMuseekConfig->setText(fd->selectedFile());
    }
    
    delete fd;
}


void ConnectDlg::stopDaemon()
{
  museeq->stopDaemon();
}




void ConnectDlg::save()
{
  museeq->saveConnectConfig();

}
