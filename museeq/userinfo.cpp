/* museeq - a Qt client to museekd
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
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

#include "userinfo.h"

#include <qsplitter.h>
#include <qvgroupbox.h>
#include <qvbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qcanvas.h>
#include <qgrid.h>
#include <qfile.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <qiconset.h>
#include "images.h"
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include "codeccombo.h"
#include "museeq.h"

ScrollImage::ScrollImage(QWidget* parent, const char* name)
	   : QScrollView(parent, name) {
	
	viewport()->setEraseColor(eraseColor());
	setFrameStyle(NoFrame);
	
	mLabel = new QLabel((QWidget*)0);
	addChild(mLabel, 0, 0);
	
	mPopupMenu = new QPopupMenu(this);
	mPopupMenu->insertItem(tr("Save picture..."), this, SLOT(savePicture()));
}

void ScrollImage::resizeEvent(QResizeEvent* event) {
	QScrollView::resizeEvent(event);
	recenterImage();
}

void ScrollImage::recenterImage() {
	if(visibleWidth() > mLabel->size().width())
		moveChild(mLabel, (visibleWidth() - mLabel->width()) / 2, childY(mLabel));
	else
		moveChild(mLabel, 0, childY(mLabel));
	if(visibleHeight() > mLabel->size().height())
		moveChild(mLabel, childX(mLabel), (visibleHeight() - mLabel->height()) / 2);
	else
		moveChild(mLabel, childX(mLabel), 0);
}

void ScrollImage::setPixmap(const QPixmap& p, const QString& baseName) {
	mBaseName = baseName;
	mLabel->setPixmap(p);
	mLabel->setFixedSize(p.size());
	QTimer::singleShot(10, this, SLOT(recenterImage()));
}

void ScrollImage::mouseReleaseEvent(QMouseEvent* e) {
	if(e->button() == 2 && mLabel->pixmap() && ! mLabel->pixmap()->isNull())
		mPopupMenu->popup(e->globalPos());
}

void ScrollImage::savePicture() {
	QString fn = QFileDialog::getSaveFileName(mBaseName + ".png");
	if(!fn.isEmpty())
		mLabel->pixmap()->save(fn, "PNG");
                            
}

UserInfo::UserInfo(const QString& user, QWidget* parent, const char* name)
         : UserWidget(user, parent, name) {
	
	QSplitter* split = new QSplitter(this);
	mUser = user;
	QVBox* vbox = new QVBox(split);
	vbox->setMargin(5);
	vbox->setSpacing(5);
	split->setResizeMode(vbox, QSplitter::KeepSize);
	
	QVGroupBox* frame = new QVGroupBox(tr("Description"), vbox);
	
	new CodecCombo("encoding.users", user, frame);
	
	mDescr = new QTextEdit(frame);
	mDescr->setReadOnly(true);
	
	frame = new QVGroupBox(tr("Information"), vbox);
	QGrid* grid = new QGrid(2, frame);
	grid->setSpacing(5);
	
	new QLabel(tr("Uploads allowed"), grid);
	(mSlots = new QLabel(tr("unknown"), grid))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	new QLabel(tr("Queue size"), grid);
	(mQueue = new QLabel(tr("unknown"), grid))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	
	new QLabel(tr("Slots available"), grid);
	(mAvail = new QLabel(tr("unknown"), grid))->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QIconSet refreshIcon = IMG("refresh");
	mRefresh = new QPushButton(tr("Refresh"), grid);
	mRefresh->setIconSet(refreshIcon);
	connect(mRefresh, SIGNAL(clicked()), SLOT(getUserInfo()));
	
	vbox = new QVBox(split);
	vbox->setMargin(5);
	frame = new QVGroupBox(tr("Picture"), vbox);
	
	mView = new ScrollImage(frame);
}
void UserInfo::getUserInfo() {
	museeq->getUserInfo(mUser);
}
void UserInfo::setInfo(const QString& info, const QByteArray& picture, uint upslots, uint queue, bool free) {
	mDescr->setText(info);
	mSlots->setText(QString::number(upslots));
	mQueue->setText(QString::number(queue));
	mAvail->setText(free ? tr("yes") : tr("no"));
	if(picture.size()) {
		QPixmap p(picture);
		mView->setPixmap(p, user());
	} else
		mView->setPixmap(QPixmap(), user());
}
