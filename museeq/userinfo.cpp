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

#include "userinfo.h"
#include "images.h"
#include "codeccombo.h"
#include "museeq.h"
#include "interestlist.h"

#include <QSplitter>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QLayout>
#include <QTimer>
#include <QFileDialog>
#include <QIcon>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>

ScrollImage::ScrollImage(QWidget* parent, const char* name)
	   : QScrollArea(parent) {

	setFrameStyle(NoFrame);

	mLabel = new QLabel((QWidget*)0);
	setWidget(mLabel);

 	QPalette palet;
    palet.setColor(backgroundRole(), palette().color(QPalette::Background));
    widget()->setPalette(palet);
	mPopupMenu = new QMenu(this);
	QAction * ActionSave = new QAction(IMG("save"), tr("Save picture..."), this);
	connect(ActionSave, SIGNAL(triggered()), this, SLOT(savePicture()));
	mPopupMenu->addAction(ActionSave);

	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(ensureCentered()));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(ensureCentered()));
}

void ScrollImage::resizeEvent(QResizeEvent* event) {
	QScrollArea::resizeEvent(event);
	ensureCentered();
}

/**
  * Sets the image to the correct initial position.
  */
void ScrollImage::centerImage() {
 	if(width() > mLabel->size().width())
 		mLabel->move((width() - mLabel->width()) / 2, mLabel->pos().y());
 	else
 		mLabel->move(0, mLabel->pos().y());
 	if(height() > mLabel->size().height())
 		mLabel->move(mLabel->pos().x(), (height() - mLabel->height()) / 2);
 	else
 		mLabel->move(mLabel->pos().x(), 0);
}

/**
  * Ensure that the image is correctly centered at any time if its size is smaller than the area.
  */
void ScrollImage::ensureCentered() {
 	if(width() > mLabel->size().width())
 		mLabel->move((width() - mLabel->width()) / 2, mLabel->pos().y());
 	if(height() > mLabel->size().height())
 		mLabel->move(mLabel->pos().x(), (height() - mLabel->height()) / 2);
}

void ScrollImage::setPixmap(const QPixmap& p, const QString& baseName) {
	mBaseName = baseName;
	mLabel->setPixmap(p);
	mLabel->setFixedSize(p.size());
	QTimer::singleShot(10, this, SLOT(centerImage()));
}

void ScrollImage::mouseReleaseEvent(QMouseEvent* e) {
	if(e->button() == 2 && mLabel->pixmap() && ! mLabel->pixmap()->isNull())
		mPopupMenu->popup(e->globalPos());
}

void ScrollImage::savePicture() {
	QString fn = QFileDialog::getSaveFileName(this, mBaseName + ".png");
	if(!fn.isEmpty())
		mLabel->pixmap()->save(fn, "PNG");

}

UserInfo::UserInfo(const QString& user, QWidget* parent, const char* name)
         : UserWidget(user, parent, name) {
	QVBoxLayout* MainLayout = new QVBoxLayout(this);
	QSplitter* split = new QSplitter(this);
	MainLayout->addWidget(split);
	mUser = user;
	QWidget* statsWidget  = new QWidget(split);
	QVBoxLayout* vbox = new QVBoxLayout(statsWidget);
	vbox->setMargin(5);
	vbox->setSpacing(5);


	QGroupBox* frame = new QGroupBox(tr("Description"), statsWidget);
	vbox->addWidget(frame);
	QVBoxLayout* DescriptionLayout = new QVBoxLayout(frame);
	DescriptionLayout->addWidget(new CodecCombo("encoding.users", user, frame));

	mDescr = new QTextEdit(frame);
	mDescr->setReadOnly(true);
	DescriptionLayout->addWidget(mDescr);

	frame = new QGroupBox(tr("Information"), statsWidget);
	vbox->addWidget(frame);
	QGridLayout* grid = new QGridLayout(frame);
	grid->setSpacing(5);

	grid->addWidget(new QLabel(tr("Uploads allowed"), statsWidget), 0, 0);
	mSlots = new QLabel(tr("unknown"));
	grid->addWidget(mSlots, 0, 1);
	mSlots->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	grid->addWidget(new QLabel(tr("Queue size"), statsWidget), 1, 0);
	mQueue = new QLabel(tr("unknown"), statsWidget);
	grid->addWidget(mQueue, 1, 1);
	mQueue->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	grid->addWidget(new QLabel(tr("Slots available"), statsWidget), 2, 0);
	mAvail = new QLabel(tr("unknown"), statsWidget);
	mAvail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	grid->addWidget(mAvail, 2, 1);
	QIcon refreshIcon = IMG("refresh");
	mRefresh = new QPushButton(tr("Refresh"), statsWidget);
	mRefresh->setIcon(refreshIcon);
	grid->addWidget(mRefresh);
	connect(mRefresh, SIGNAL(clicked()), SLOT(getUserInfo()));


	frame = new QGroupBox(tr("Interests"), split);
	vbox = new QVBoxLayout(frame);
	vbox->setMargin(5);

	mHeLoves = new InterestList(tr("He likes:"), 0, true);
	vbox->addWidget(mHeLoves);
	mHeHates = new InterestList(tr("He hates:"), 0, true);
	vbox->addWidget(mHeHates);


	frame = new QGroupBox(tr("Picture"), split);
	vbox = new QVBoxLayout(frame);
	vbox->addStrut(3 * MainLayout->sizeHint().width() / 4);
	vbox->setMargin(5);
	mView = new ScrollImage(frame);
	vbox->addWidget(mView);
}

void UserInfo::getUserInfo() {
	museeq->getUserInfo(mUser);
	museeq->getUserInterests(mUser);
}

void UserInfo::setInfo(const QString& info, const QByteArray& picture, uint upslots, uint queue, bool free) {
	mDescr->setText(info);
	mSlots->setText(QString::number(upslots));
	mQueue->setText(QString::number(queue));
	mAvail->setText(free ? tr("yes") : tr("no"));
	if(picture.size()) {
		QPixmap p;
		p.loadFromData(picture);
		mView->setPixmap(p, user());
	} else
		mView->setPixmap(QPixmap(), user());
	emit(highlight(1));
}

void UserInfo::setInterests(const QStringList& likes, const QStringList& hates) {
    QStringList::const_iterator it = likes.begin();
    for (; it != likes.end(); it++)
        mHeLoves->added(*it);

    for (it = hates.begin(); it != hates.end(); it++)
        mHeHates->added(*it);

	emit(highlight(1));
}
