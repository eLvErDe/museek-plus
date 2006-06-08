/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <qfiledialog.h>
#include <qdir.h>


void SettingsDlg::SConnect_clicked()
{  museeq->connectServer();
}
void SettingsDlg::SDisconnect_clicked()
{   museeq->disconnectServer();
}
void SettingsDlg::SReloadShares_clicked()
{   museeq->reloadShares();
}



void SettingsDlg::SDownload_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
    fd->setMode(QFileDialog::Directory);
    fd->setCaption("Select a Directory to store your downloaded files.");
    fd->addFilter("All files (*)");
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	SDownDir->setText( fd->dirPath());
    }
    
    delete fd;

}


void SettingsDlg::SIncomplete_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "", this);
    fd->setMode(QFileDialog::Directory);
    fd->setCaption("Select a Directory to store your incomplete downloading files.");
    fd->addFilter("All files (*)");
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	SIncompleteDir->setText( fd->dirPath());
    }
    
    delete fd;
}
