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

void UserInfoDlg::mBrowse_clicked()
{
    QFileDialog * fd = new QFileDialog(QDir::homeDirPath(), "Images (*.png *.gif *.jpg *.jpeg)", this);
    fd->setMode(QFileDialog::ExistingFile);
    fd->setCaption("Select an Image for you User info");
    fd->addFilter("All files (*)");
    if(fd->exec() == QDialog::Accepted && ! fd->selectedFile().isEmpty())
    {
	mImage->setText(fd->selectedFile());
	mUpload->toggle();
    }
    
    delete fd;
}
