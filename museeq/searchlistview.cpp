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

#include "searchlistview.h"

#include "util.h"
#include "usermenu.h"
#include "museeq.h"
#include "searchfilter.h"

#include <QFileDialog>
#include <QMenu>
#include <QList>
#include <QUrl>
#include <QDrag>
#include <QMouseEvent>
#include <QPainter>
#include <QHeaderView>

SearchListView::SearchListView(SearchFilter* filter, QWidget* parent, const char* name)
               : QTreeWidget(parent), mFilter(filter), mN(0) {
	QStringList headers;
	headers << QString::null << tr("User") << tr("Filename") << tr("Size") << tr("Speed") << tr("Queued") << tr("Imm.") << tr("Length") << tr("Bitrate") << tr("Path") << QString::null;
	setHeaderLabels(headers);
	setSortingEnabled(false);
	header()->setClickable(true);
	setRootIsDecorated(false);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
 	setAllColumnsShowFocus(true);

	setColumnWidth ( 0, 30 );
	setColumnWidth ( 1, 100 );
	setColumnWidth ( 2, 250 );
	setColumnWidth ( 3, 100 );
	setColumnWidth ( 4, 100 );
	setColumnWidth ( 5, 75 );
	setColumnWidth ( 6, 30 );
	setColumnWidth ( 7, 75 );
	setColumnWidth ( 8, 75 );
	setColumnWidth ( 9, 150 );
	setColumnWidth ( 10, 250 );
	setColumnWidth ( 11, 0 );

	mPopupMenu = new QMenu(this);
	QAction * ActionDownloadFiles, * ActionDownloadFilesTo, * ActionDownloadFolders;
	ActionDownloadFiles = new QAction(tr("Download file(s)"), this);
	connect(ActionDownloadFiles, SIGNAL(triggered()), this, SLOT(downloadFiles()));
	mPopupMenu->addAction(ActionDownloadFiles);

	ActionDownloadFilesTo = new QAction(tr("Download file(s) to..."), this);
	connect(ActionDownloadFilesTo, SIGNAL(triggered()), this, SLOT(downloadFilesTo()));
	mPopupMenu->addAction(ActionDownloadFilesTo);

	ActionDownloadFolders = new QAction(tr("Download selected folder(s)"), this);
	connect(ActionDownloadFolders, SIGNAL(triggered()), this, SLOT(downloadFolders()));
	mPopupMenu->addAction(ActionDownloadFolders);

	mPopupMenu->addSeparator();
	mUsersMenu = mPopupMenu->addMenu(tr("Users"));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));

	connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(headerClicked(int)));

	setDragEnabled(true);
}

void SearchListView::slotContextMenu(const QPoint& pos) {
	SearchListItem* item = dynamic_cast<SearchListItem*>(itemAt(pos));

	if (! item )
		return;

	setupUsers();
	mPopupMenu->exec(mapToGlobal(pos));
}

void SearchListView::slotActivate(QTreeWidgetItem* item, int column) {

	SearchListItem* _item = dynamic_cast<SearchListItem*>(item);
	if(item)
		museeq->downloadFile(_item->user(), _item->path(), _item->size());
}

void SearchListView::setupUsers() {
	mUsersMenu->clear();

	QStringList users;
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);

	while (*it) {
		SearchListItem* item = dynamic_cast<SearchListItem*>(*it);

		if(item && users.indexOf(item->user()) == -1)
		{
			users << item->user();
		}
		++it;
	}


    int numusers = users.size();
    mPopupMenu->removeAction(mUsersMenu->menuAction());
    delete mUsersMenu;
    if (numusers <= 0) {
        mUsersMenu = mPopupMenu->addMenu(tr("Users"));
        return;
    }
    else if (numusers == 1) {
        mUsersMenu = new Usermenu(mPopupMenu);
        dynamic_cast<Usermenu*>(mUsersMenu)->setup(users.first());
        QAction * usermenu = mPopupMenu->addMenu(mUsersMenu);
        usermenu->setText(tr("User '%1'").arg(users.first()));
    }
    else {
        QStringListIterator usersIt(users);
        mUsersMenu = mPopupMenu->addMenu(tr("Users"));
        while (usersIt.hasNext()) {
            QString username = usersIt.next();
            Usermenu *m = new Usermenu(mUsersMenu);
            m->setup(username);
            QAction * usermenu = mUsersMenu->addMenu(static_cast<QMenu*>(m));
            usermenu->setText(username);
        }
    }
}

void SearchListView::downloadFiles() {
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);

	for(; *it; ++it) {
		SearchListItem* item = dynamic_cast<SearchListItem*>(*it);
		if (item)
            museeq->downloadFile(item->user(), item->path(), item->size());
	}
}

void SearchListView::downloadFilesTo() {
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
	QFileDialog * fd = new QFileDialog(this, tr("Select a directory for current download(s)"), QDir::homePath());
	fd->setFileMode(QFileDialog::Directory);
	if(fd->exec() == QDialog::Accepted){
		QString localpath = fd->directory().path();
		for(; *it; ++it) {
			SearchListItem* item = dynamic_cast<SearchListItem*>(*it);
			if (item)
                museeq->downloadFileTo(item->user(), item->path(), localpath,  item->size());
		}
	}
	delete fd;
}

void SearchListView::downloadFolders() {
	QMap<QString, QStringList> folders;
	QTreeWidgetItemIterator it(this, QTreeWidgetItemIterator::Selected | QTreeWidgetItemIterator::NotHidden);
 	for(; *it; ++it) {
 		SearchListItem* item = dynamic_cast<SearchListItem*>(*it);
 		QStringList& dirs = folders[item->user()];
 		if(item && dirs.indexOf(item->dir()) == -1) {
 			QString d = item->dir();
 			if ( "\\" == d.right(1))
 				museeq->downloadFolder(item->user(), d.mid(0, d.length()-1));
 			else
 				museeq->downloadFolder(item->user(), item->dir());
 			folders[item->user()] << item->dir();
 		}
 	}
}

void SearchListView::append(const QString& u, bool f, uint s, uint q, const NFolder& r) {
	NFolder::const_iterator it = r.begin();
	for(; it != r.end(); ++it) {
		SearchListItem *item = new SearchListItem(this, ++mN, u, f, s, q, it.key(), (*it).size, (*it).length, (*it).bitrate, (*it).vbr);
 		item->setHidden(!mFilter->match(item));
	}
}

/**
  * User clicked the header
  * Sorting is disable when starting a new search to avoid annoying automatic moving in the list when adding new items
  * So enable it when the user really wants to sort results
  */
void SearchListView::headerClicked(int i)
{
    if (!isSortingEnabled()) {
        setSortingEnabled(true);
        sortItems(i, Qt::AscendingOrder);
    }
}

/**
  * User have press mouse button in this widget
  */
void SearchListView::mousePressEvent(QMouseEvent *event)
{
    QList<QTreeWidgetItem *> oldItems = selectedItems();
    event->accept();

    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();

    QTreeWidget::mousePressEvent(event);
    setDragEnabled(oldItems == selectedItems());
}

/**
  * User have moved his mouse in this widget
  */
void SearchListView::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();

    // Should we start dragging?
    if (!dragEnabled()) {
        // We change selection: don't stop dragging and restore extendedselection
        setSelection(QRect(mDragStartPosition, event->pos()), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        return;
    }
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    // Create drag object
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> urls;
    QStringList users;

	QList<QTreeWidgetItem*> items = selectedItems();
    QList<QTreeWidgetItem*>::const_iterator it = items.begin();
	for(; it != items.end(); ++it) {
        SearchListItem * item = dynamic_cast<SearchListItem*>(*it);
 		if(item && item->isSelected() && !item->isHidden()) {
 			if(users.indexOf(item->user()) == -1)
 				users << item->user();

            // slsk protocol: in QUrl, hostname is always lower case.
            // So we put username as hostname for compatibility, and as username to have the correct case.
            // Ex: slsk://MuSeEk:filesize@museek/path/to/a/file
            // Code should first look at QUrl::userName() and if not present, try QUrl::host()
            QUrl url("slsk://" + item->user());
            url.setUserName(item->user());
            url.setPath(item->path().replace("\\", "/"));
            url.setPassword(QString::number(item->size()));

            // There may be spaces in username so url may not be valid. It will work, but QUrl::isValid() should not be used
            urls.push_back(url);
 		}
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

	QString x(tr("%n search result(s)", "", urls.count()) + tr(" (%n user(s))", "", users.count()));
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

    // Begin dragging
    drag->exec();
}

SearchListItem::SearchListItem(QTreeWidget* parent, quint64 n, const QString& user, bool free, uint speed, uint inQueue, const QString& path, quint64 size, uint length, uint bitrate, bool vbr)
               : QTreeWidgetItem(parent), mN(n), mUser(user), mPath(path), mSpeed(speed), mInQueue(inQueue),
                 mLength(length), mBitrate(bitrate), mSize(size), mFree(free), mVBR(vbr)
{
	setText(0, QString::number(n));
	setText(1, mUser);
	int ix = mPath.lastIndexOf('\\');
	if(ix != -1) {
		mFilename = mPath.mid(ix + 1);
		mDir = mPath.left(ix + 1);
		setText(2, mFilename);
		setText(9, mDir);
	} else {
		mFilename = mPath;
		mDir = "";
		setText(2, mFilename);
	}
	setText(3, Util::makeSize(mSize));
	setText(4, Util::makeSize(mSpeed) + SearchListView::tr("/s"));
	setText(5, QString::number(mInQueue));
	setText(6, mFree ? SearchListView::tr("Y") : SearchListView::tr("N"));
	if(mLength)
		setText(7, Util::makeTime(mLength));
	if(mBitrate)
		setText(8, Util::makeBitrate(mBitrate, mVBR));

    if (!mFree) {
        QBrush brush = QBrush(QApplication::palette().color(QPalette::Disabled, QPalette::Text));
        for (int i = 0; i < 10; ++i) {
            setForeground(i, brush);
        }
    }
}

bool SearchListItem::operator<(const QTreeWidgetItem & other_) const {
    const SearchListItem * other = dynamic_cast<const SearchListItem*>(&other_);
    if (!other)
        return false;

	int col = 0;
	if(treeWidget())
	col = treeWidget()->sortColumn();

	switch(col) {
	case 0:
		return n() < other->n();
	case 1:
		return user() < other->user();
	case 2:
		return text(2) < other->text(2);
	case 3:
		return size() < other->size();
	case 4:
		return speed() < other->speed();
	case 5:
		return inQueue() < other->inQueue();
	case 6:
		return freeSlot();
	case 7:
		return length() < other->length();
	case 8:
		return bitrate() < other->bitrate();
	case 9:
		return dir() < other->dir() ;
	}

  return false;
}
