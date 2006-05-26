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

#include "museeq.h"
#include <qdatetime.h>

void OnlineAlert::setUser( const QString &user )
{
    mUser = user;
    connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(slotUserStatus(const QString&, uint)));
}


void OnlineAlert::slotUserStatus( const QString & user, uint status )
{
    if(user == mUser && (status == 2 || isShown()))
    {
	QString s = (status == 0) ? "offline" : ((status == 1) ? "away" : "online");
	QString t = QTime::currentTime().toString();
	mLabel->setText(QString("%1 user %2 is now %3").arg(t).arg(mUser).arg(s));
	if(isShown())
	    raise();
	else
	    show();
    }
}


void OnlineAlert::mRemove_clicked()
{
    emit removeAlert(mUser);
    accept();
}
