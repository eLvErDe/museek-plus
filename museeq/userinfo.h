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

#ifndef USERINFO_H
#define USERINFO_H

#include "usertabwidget.h"
#include <qscrollview.h>

class QTextEdit;
class QLabel;
class QCanvasView;
class ScrollImage;
class QPopupMenu;
class QPushButton;
class UserInfo : public UserWidget {
	Q_OBJECT
public:
	UserInfo(const QString&, QWidget* = 0, const char* = 0);
	
public slots:
	void setInfo(const QString&, const QByteArray&, uint, uint, bool);
	void getUserInfo();
	
private:
	QTextEdit* mDescr;
	QLabel* mSlots, * mQueue, * mAvail;
	ScrollImage* mView;
	QPushButton* mRefresh;
	QString mUser;
};

class ScrollImage : public QScrollView {
	Q_OBJECT
public:
	ScrollImage(QWidget* = 0, const char* = 0);
	void setPixmap(const QPixmap&, const QString&);
protected:
	void resizeEvent(QResizeEvent*);
	void mouseReleaseEvent(QMouseEvent* e);
protected slots:
	void recenterImage();
	void savePicture();
private:
	QString mBaseName;
	QLabel* mLabel;
	QPopupMenu* mPopupMenu;
};

#endif // USERINFO_H
