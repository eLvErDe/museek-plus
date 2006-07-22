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

#include "searchlistview.h"

#include "slskdrag.h"
#include "util.h"
#include "usermenu.h"
#include "museeq.h"
#include "searchfilter.h"
#include <qfile.h>
#include <qfiledialog.h>
#include <qpopupmenu.h>

SearchListView::SearchListView(SearchFilter* filter, QWidget* parent, const char* name)
               : QListView(parent, name), mFilter(filter), mN(0) {
	
	setSelectionMode(Extended);
	setShowSortIndicator(true);
	setAllColumnsShowFocus(true);
	
	addColumn("");
	addColumn(tr("User"), 100);
	addColumn(tr("Filename"), 250);
	addColumn(tr("Size"), 100);
	addColumn(tr("Speed"), 100);
	addColumn(tr("Queued"), 75);
	addColumn(tr("Imm."),20);
	addColumn(tr("Length"), 75);
	addColumn(tr("Bitrate"), 75);
	addColumn(tr("Path"));
	
	setColumnAlignment(0, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(3, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(4, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(5, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(6, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(7, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(8, Qt::AlignRight|Qt::AlignVCenter);
	
	mPopupMenu = new QPopupMenu(this);
	mPopupMenu->insertItem(tr("Download file(s)"), this, SLOT(downloadFiles()));
	mPopupMenu->insertItem(tr("Download file(s) to.."), this, SLOT(downloadFilesTo()));
	mPopupMenu->insertItem(tr("Download selected folder(s)"), this, SLOT(downloadFolders()));
	mPopupMenu->insertSeparator();
	mUsersMenu = new QPopupMenu(mPopupMenu);
	mPopupMenu->insertItem(tr("Users"), mUsersMenu);
	
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(popupMenu(QListViewItem*, const QPoint&, int)));
}

void SearchListView::setupUsers() {
	mUsersMenu->clear();
	
	QValueList<QString> users;
	QListViewItemIterator it(this, QListViewItemIterator::Selected|QListViewItemIterator::Visible);
	for(; *it; ++it) {
		SearchListItem* item = static_cast<SearchListItem*>(*it);
		if(users.find(item->user()) == users.end())
		{
			users << item->user();
			Usermenu *m = new Usermenu(mUsersMenu);
			m->setup(item->user());
			mUsersMenu->insertItem(item->user(), m);
		}
	}
}

void SearchListView::popupMenu(QListViewItem* item, const QPoint& pos, int col) {
	setupUsers();
	mPopupMenu->popup(pos);
}

void SearchListView::downloadFiles() {
	QListViewItemIterator it(this, QListViewItemIterator::Selected|QListViewItemIterator::Visible);
	for(; *it; ++it) {
		SearchListItem* item = static_cast<SearchListItem*>(*it);
		museeq->downloadFile(item->user(), item->path(), item->size());
	}
}

void SearchListView::downloadFilesTo() {
	QListViewItemIterator it(this, QListViewItemIterator::Selected|QListViewItemIterator::Visible);

	QFileDialog * fd = new QFileDialog( QDir::homeDirPath(), "", this);
	fd->setCaption(tr("Select a Directory for current download(s)"));
	fd->setMode(QFileDialog::Directory);
	if(fd->exec() == QDialog::Accepted){
		QString localpath = fd->dirPath();
		QString filename;
		for(; *it; ++it) {
			SearchListItem* item = static_cast<SearchListItem*>(*it);
			int ix = item->path().findRev('\\');
			if(ix != -1) {
				filename = item->path().mid(ix + 1);
			} else {
				filename = item->path();
			}
			museeq->downloadFileTo(item->user(), item->path(), localpath +"/"+ filename,  item->size());
			
		}
	}
	delete fd;
}

void SearchListView::downloadFolders() {
	QMap<QString, QStringList> folders;
	QListViewItemIterator it(this, QListViewItemIterator::Selected|QListViewItemIterator::Visible);
	for(; *it; ++it) {
		SearchListItem* item = static_cast<SearchListItem*>(*it);
		QStringList& dirs = folders[item->user()];
		if(dirs.find(item->dir()) == dirs.end()) {
			QString d = item->dir();
			if ( "\\" == d.right(1) || "/" == d.right(1 ))
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
		item->setVisible(mFilter->match(item));
	}
}

QDragObject* SearchListView::dragObject() {
	SlskDrag* drag = new SlskDrag(viewport());
	
	QValueList<SearchListItem*> items;
	QStringList users;
	
	SearchListItem* item = static_cast<SearchListItem*>(firstChild());
	for(; item != 0; item = static_cast<SearchListItem*>(item->nextSibling()))
		if(isSelected(item) && item->isVisible()) {
			if(users.find(item->user()) == users.end())
				users << item->user();
			drag->append(item->user(), item->path());
			items << item;
		}
	
	if(! items.count()) {
		delete drag;
		return 0;
	}
	
	QString x;
	if(items.count() == 1)
		x = tr("1 search result (1 user)");
	else if(users.count() == 1)
		x = QString(tr("%1 search results (1 user)") ).arg(items.count());
	else
		x = QString(tr("%1 search results (%2 users)") ).arg(items.count()).arg(users.count());
	QSize s = viewport()->fontMetrics().size(Qt::SingleLine, x) + QSize(6, 4);
	
	QPixmap pix(s);
	QPainter p(&pix);
	p.setFont(viewport()->font());
	p.setPen(viewport()->foregroundColor());
	
	p.fillRect(QRect(QPoint(0, 0), s), viewport()->eraseColor());
	p.drawRect(QRect(QPoint(0, 0), s));
	p.drawText(QRect(QPoint(3, 3), s - QSize(3, 3)), AlignAuto | AlignVCenter, x);
	
	p.end();
	
	drag->setPixmap(pix, QPoint(-25, -25));
	
	return drag;
}

SearchListItem::SearchListItem(QListView* parent, Q_UINT64 n, const QString& user, bool free, uint speed, uint inQueue,
                               const QString& path, Q_INT64 size, uint length, uint bitrate, bool vbr)
               : QListViewItem(parent), mN(n), mUser(user), mPath(path), mSpeed(speed), mInQueue(inQueue),
                 mLength(length), mBitrate(bitrate), mSize(size), mFree(free), mVBR(vbr) {
	
	setDragEnabled(true);
	
	setText(0, QString::number(n));
	setText(1, mUser);
	int ix = mPath.findRev('\\');
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
	setText(4, Util::makeSize(mSpeed) + QT_TR_NOOP("/s"));
	setText(5, QString::number(mInQueue));
	setText(6, mFree ? QT_TR_NOOP("Y") : QT_TR_NOOP("N"));
	if(mLength)
		setText(7, Util::makeTime(mLength));
	if(mBitrate)
		setText(8, Util::makeBitrate(mBitrate, mVBR));
}

int SearchListItem::compare(QListViewItem* i, int col, bool) const {
	SearchListItem* item = static_cast<SearchListItem*>(i);
	switch(col) {
	case 0: return Util::cmp(mN, item->mN);
	case 1: return mUser.localeAwareCompare(item->mUser);
	case 2: return mFilename.localeAwareCompare(item->mFilename);
	case 3: return Util::cmp(mSize, item->mSize);
	case 5: return Util::cmp(mInQueue, item->mInQueue);
	case 4: return Util::cmp(mSpeed, item->mSpeed);
	case 6: return Util::cmp(mFree, item->mFree);
	case 7: return Util::cmp(mLength, item->mLength);
	case 8: return Util::cmp(mBitrate, item->mBitrate);
	case 9: return mDir.localeAwareCompare(item->mDir);
	default:
		return 0;
	}
}
