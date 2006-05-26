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


void ProtocolDlg::mNew_clicked()
{
    QListViewItem* item = new QListViewItem(mProtocols, "", "");
    item->setRenameEnabled(0, true);
    item->setRenameEnabled(1, true);
    item->startRename(0);
}


void ProtocolDlg::mProtocols_itemRenamed( QListViewItem *item, int col)
{
    if(col == 0)
	item->startRename(1);
}
