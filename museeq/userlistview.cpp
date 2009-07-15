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

#include "userlistview.h"
#include "userlistitem.h"
#include "usermenu.h"
#include "museeq.h"
#include "mainwin.h"
#include "images.h"
#include "util.h"

#include <QPixmap>
#include <QDropEvent>
#include <QList>
#include <QUrl>
#include <QPainter>
#include <QApplication>
#include <QDrag>
#include <QMouseEvent>
#include <QHeaderView>
#include <QSettings>

UserListView::UserListView(bool comments, QWidget * parent, const QString& name)
	: QTreeWidget(parent) {

	mUsermenu = new Usermenu(this);

	mName = name;

    setAcceptDrops(true);
	setDragEnabled(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setColumnCount(5);

	QStringList headers;
	headers  << QString::null << tr("User") << tr("Speed") << tr("Files") << tr("Country");
	if(comments) {
		headers << (tr("Comments"));
        setColumnCount(6);
	}
	setHeaderLabels(headers);

	setAlternatingRowColors(true);
	setSortingEnabled(true);
	sortItems(1, Qt::AscendingOrder);
	QPixmap& p(IMG("online"));
	int iconColumnWidth = (p.width()/p.height()) * size().height();
	setColumnWidth (0, iconColumnWidth);
	setColumnWidth ( 1, 100 );
	setColumnWidth ( 2, 90 );
	setColumnWidth ( 3, 90 );
	setColumnWidth ( 4, 70 );

	setAllColumnsShowFocus(true);
	setRootIsDecorated(false);
	setContextMenuPolicy(Qt::CustomContextMenu);

    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = mName+"_Layout";
        header()->restoreState(museeq->settings()->value(optionName).toByteArray());
    }

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));

	connect(museeq, SIGNAL(sortingEnabled(bool)), this, SLOT(sorting(bool)));
	connect(museeq, SIGNAL(userStatus(const QString&, uint)), SLOT(setStatus(const QString&, uint)));
	connect(museeq, SIGNAL(doUpdateStatus(const QString&)), SLOT(updateStatus(const QString&)));
	connect(museeq, SIGNAL(userData(const QString&, uint, uint, const QString&)), SLOT(setData(const QString&, uint, uint, const QString&)));
	connect(museeq, SIGNAL(toggleCountries(bool)), this, SLOT(countryToggled(bool)));
	connect(museeq, SIGNAL(closingMuseeq()), this, SLOT(onClosingMuseeq()));
}

void UserListView::onClosingMuseeq() {
    if (museeq->settings()->value("saveAllLayouts", false).toBool()) {
        QString optionName = mName+"_Layout";
        museeq->settings()->setValue(optionName, header()->saveState());
    }
}

void UserListView::sorting(bool sort) {
	setSortingEnabled(sort);
}

UserListItem* UserListView::findItem(const QString& _u) {
	QList<QTreeWidgetItem *> items = findItems(_u, Qt::MatchExactly, 1);
	if (!items.isEmpty())
    	return dynamic_cast<UserListItem *>(items.at(0));
    else
        return NULL;
}

uint UserListView::status(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->status();
}

uint UserListView::speed(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->speed();
}

uint UserListView::files(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->files();
}

QString UserListView::comments(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return QString::null;
	return item->comments();
}

QString UserListView::country(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return 0;
	return item->country();
}

void UserListView::setStatus(const QString& _u, uint _s) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setStatus(_s);

	adaptColumnSize(0);
}

void UserListView::updateStatus(const QString& _u) {
	UserListItem* item = findItem(_u);
	if(! item)
		return;
	item->updateUserStatus();

	adaptColumnSize(0);
}

void UserListView::setData(const QString& _u, uint _s, uint _f, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setSpeed(_s);
	item->setFiles(_f);
	item->setCountry(_c);

	adaptColumnSize(2);
	adaptColumnSize(3);
	adaptColumnSize(4);
}

void UserListView::setComments(const QString& _u, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setComments(_c);
}

void UserListView::setCountry(const QString& _u, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setCountry(_c);

	adaptColumnSize(4);
}

void UserListView::setOperator(const QString& _u, bool _o) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setOperator(_o);

	adaptColumnSize(1);
}

void UserListView::setOwner(const QString& _u, bool _o) {
	UserListItem *item = findItem(_u);
	if(! item)
		return;
	item->setOwner(_o);

	adaptColumnSize(1);
}

void UserListView::add(const QString& _u, uint _st, uint _s, uint _f, const QString& _c, const QString& _co) {
	UserListItem *item = findItem(_u);
	if(item) {
		item->setAll(_st, _s, _f, _c, _co);
		return;
	}
	new UserListItem(this, _u, _st, _s, _f, _c, _co);

	adaptColumnSize(0);
	adaptColumnSize(1);
	adaptColumnSize(2);
	adaptColumnSize(3);
	adaptColumnSize(4);
}

void UserListView::add(const QString& _u, const QString& _c) {
	UserListItem *item = findItem(_u);
	if(item) {
		item->setComments(_c);
		return;
	}
	new UserListItem(this, _u, 0, 0, 0, _c, QString::null);

	adaptColumnSize(0);
	adaptColumnSize(1);
	adaptColumnSize(2);
	adaptColumnSize(3);
	adaptColumnSize(4);
}

void UserListView::remove(const QString& _u) {
	UserListItem *item = findItem(_u);
	if(! item) {
		printf("warning, couldn't find user!\n");
		return;
	}
	delete item;

	adaptColumnSize(0);
	adaptColumnSize(1);
	adaptColumnSize(2);
	adaptColumnSize(3);
	adaptColumnSize(4);
}

void UserListView::slotActivate(QTreeWidgetItem* item, int column) {
	slotActivate( item);
}
void UserListView::slotActivate(QTreeWidgetItem* item) {
	UserListItem* _item = dynamic_cast<UserListItem*>(item);
	if(! _item)
		return;
	emit activated(_item->user());
	emit activated(_item->user(), _item->comments());
}

/**
  * Search TreeWidget users column for users matching keypresses (only first letter)
  */
void UserListView::keyboardSearch(const QString& string) {
    // Search for a user starting by this letter
	QList<QTreeWidgetItem *> searchUsers = QTreeWidget::findItems(string, Qt::MatchStartsWith, 1);

	// If we've already searched for this letter earlier, we should display next one
	if (mLastSearch != string) {
		mSearchPosition = 0;
		mLastSearch = QString(string);
	} else {
		mSearchPosition += 1;
	}

    // Do nothing if we didn't find anything
	if (searchUsers.isEmpty())
        return;

    // If we're on the last user having this letter, next one will be the first one
    if (static_cast<int>(mSearchPosition) >= searchUsers.size())
		mSearchPosition = 0;

    // Get the found item
	QTreeWidgetItem * item = 0;
	item = searchUsers.at(mSearchPosition);

	if (! item)
		return;

    // Select found item
	setCurrentItem(item);
}

/**
  * User have press mouse button in this widget
  */
void UserListView::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();

    QTreeWidget::mousePressEvent(event);
}

/**
  * User have moved his mouse in this widget
  */
void UserListView::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();

    // Should we start dragging?
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    // Create drag object
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> urls;

	QList<QTreeWidgetItem*> items = selectedItems();
    QList<QTreeWidgetItem*>::const_iterator it = items.begin();
	for(; it != items.end(); ++it) {
	    // slsk protocol: in QUrl, hostname is always lower case.
	    // So we put username as hostname for compatibility, and as username to have the correct case.
	    // Ex: slsk://MuSeEk@museek/path/to/a/file
	    // Code should first look at QUrl::userName() and if not present, try QUrl::host()
	    UserListItem * item = dynamic_cast<UserListItem*>(*it);
	    if (!item)
            continue;
	    QUrl url("slsk://" + item->user());
	    url.setUserName(item->user());

	    // There may be spaces in username so url may not be valid. It will work, but QUrl::isValid() should not be used
	    urls.push_back(url);
	}

	if(urls.count() == 0)
		return;

	// Add the urls to the mimedata
    mimeData->setUrls(urls);
    // Add them too in text format if we want to paste it in a text area
    QString textUrls;
    QList<QUrl>::const_iterator uit;
    for(uit = urls.begin(); uit != urls.end(); uit++)
        textUrls += uit->toString() + "\n";
    mimeData->setText(textUrls);

    // And now set this mimedata into drag object
    drag->setMimeData(mimeData);

    // Make visible what we're dragging
    if(items.count() < 6) {
		QSize dest(0, 0);
		for(it = items.begin(); it != items.end(); ++it) {
            UserListItem * item = dynamic_cast<UserListItem*>(*it);
            if (!item)
                continue;

			QSize textSize = viewport()->fontMetrics().size(Qt::TextSingleLine, item->user());

			QPixmap icon = item->icon(0).pixmap(50, 50); // 50x50 is the maximum size, if image is smaller, the pixmap will be too
			QSize iconSize = icon.size();

			QSize lineSize = textSize.expandedTo(QSize(textSize.width() + iconSize.width() + 2, iconSize.height()));
			dest = dest.expandedTo(QSize(lineSize.width(), lineSize.height() + 2));
		}

		QSize s(dest.width() + 6, dest.height() * items.count() + 4);

		QPixmap pix(s);
		QPainter p(&pix);
		p.setFont(viewport()->font());
        p.setPen(viewport()->palette().color(QPalette::WindowText));

		p.fillRect(QRect(QPoint(0, 0), s), viewport()->palette().color(QPalette::Background));
		p.drawRect(QRect(QPoint(0, 0), s));

		int y = 2;
		for(it = items.begin(); it != items.end(); ++it) {
            UserListItem * item = dynamic_cast<UserListItem*>(*it);
            if (!item)
                continue;

			const QPixmap icon = item->icon(0).pixmap(50, 50);
			p.drawPixmap(3, y + ((dest.height() - icon.height()) / 2), icon);
			QRect r(QPoint(5 + icon.width(), y),
			        dest - QSize(icon.width() + 2, 0));
			p.drawText(r, Qt::AlignLeft |  Qt::AlignVCenter, item->user());
			y += dest.height();
		}

		p.end();

        drag->setHotSpot(QPoint(20, 20));
		drag->setPixmap(pix);
    }
	else {
		QString x(QString(tr("%n user(s)", "", items.count())));
		QSize s = viewport()->fontMetrics().size(Qt::TextSingleLine, x) + QSize(6, 4);

		QPixmap pix(s);
		QPainter p(&pix);
		p.setFont(viewport()->font());
        p.setPen(viewport()->palette().color(QPalette::WindowText));

		p.fillRect(QRect(QPoint(0, 0), s), viewport()->palette().color(QPalette::Background));
		p.drawRect(QRect(QPoint(0, 0), s));
		p.drawText(QRect(QPoint(3, 3), s - QSize(3, 3)), Qt::AlignLeft | Qt::AlignVCenter, x);

		p.end();

        drag->setHotSpot(QPoint(20, 20));
		drag->setPixmap(pix);
	}

    // Begin dragging
    drag->exec();
}

void UserListView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void UserListView::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void UserListView::dropEvent(QDropEvent* event) {
    event->acceptProposedAction();

    if (Util::hasSlskUrls(event) && acceptDrops())
        emit dropSlsk(event->mimeData()->urls());
}

void UserListView::slotContextMenu(const QPoint& pos) {
	QTreeWidgetItem * item = 0 ;
	item = itemAt(pos) ;
	if (! item ) {
		return;
	}
	mUsermenu->exec(item->text(1), mapToGlobal(pos));
}

void UserListView::slotContextMenu(QTreeWidgetItem* item, const QPoint& pos, int) {
    UserListItem * _item = dynamic_cast<UserListItem *>(item);
	if(_item)
        mUsermenu->exec(_item->user(), pos);
}

void UserListView::countryToggled(bool t) {
    setColumnHidden(4, !t);
}

void UserListView::adaptColumnSize(int column) {
    if (!museeq->mainwin()->isSavingAllLayouts())
        resizeColumnToContents(column);
}
