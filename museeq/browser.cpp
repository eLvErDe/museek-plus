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

#include "browser.h"

#include "util.h"

#include <qlineedit.h>
#include <qinputdialog.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qlabel.h>
#include <iostream>
#include <qpushbutton.h>
#include "codeccombo.h"
#include "slskdrag.h"
#include "qpainter.h"
#include "museeq.h"
#include <qclipboard.h>
#include <qtranslator.h>
class SharesData {
public:
	QString path;
	QMap<QString, SharesData*> folders;
	NFolder files;
	
	SharesData(const QString& _path) : path(_path), folders(), files() { }
	~SharesData() {
		QMap<QString, SharesData*>::Iterator it, end = folders.end();
		for(it = folders.begin(); it != end; ++it)
			delete it.data();
		folders.clear();
		files.clear();
	}
	
	SharesData* get(const QStringList& path, uint i = 0) {
		if(i == path.size())
			return this;
		
		const QString& piece = path[i];
		SharesData* n;
		if(folders.contains(piece))
			n = folders[piece];
		else {
			QStringList::ConstIterator it, end = path.at(i + 1);
			QString p;
			for(it = path.begin(); it != end; ++it)
				p += *it + "\\";
			n = new SharesData(p);
			folders[piece] = n;
		}
		return n->get(path, i + 1);
	}
};


Browser::Browser(const QString& user, QWidget* parent, const char* name)
       : UserWidget(user, parent, name) {
	
	QHBox* hbox = new QHBox(this);
	hbox->setSpacing(5);
	mUser = user;
	hbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	new QLabel(tr("Search files and folders"), hbox);
	mEntry = new QLineEdit(hbox, "search");
	
	QFrame* frame = new QFrame(hbox);
	frame->setFrameShape(QFrame::VLine);
	frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	mFileCount = new QLabel(tr("Haven't recieved shares"), hbox);	
	new CodecCombo("encoding.users", user, hbox, "encoding");
	mRefresh = new QPushButton(tr("Refresh"), hbox);
	connect(mRefresh, SIGNAL(clicked()), SLOT(getShares()));

	QSplitter* split = new QSplitter(this);
	mFolders = new FolderListView(user, split, "folders");
	mFolders->setEnabled(false);
	mFolders->resize(250, -1);
	
	mFiles = new FileListView(user, split, "files");
	mFiles->setEnabled(false);
	
	connect(mEntry, SIGNAL(returnPressed()), SLOT(doSearch()));
	connect(mFolders, SIGNAL(currentChanged(const QString&, const NFolder&)), mFiles, SLOT(setFiles(const QString&, const NFolder&)));
}

void Browser::setShares(const NShares& shares) {
	mShares = shares;
	
	if (shares.size()) {
		uint z = shares.size();
		QVariant p (z);
		mFileCount->setText(p.toString()+ " "+  tr("directories"));
		}
	else 
		mFileCount->setText(tr("Sharing nothing.."));
	mCurrentResult = mShares.begin();
	mFolders->setShares(shares);
	mFolders->setEnabled(true);
	mFiles->setEnabled(true);
}
void Browser::getShares() {
	museeq->getUserShares(mUser);
}
void Browser::doSearch() {
	QString q = mEntry->text();
	if(q.isEmpty() || mShares.isEmpty())
		return;
	
	if(q != mQuery)
	{
		mQuery = q;
		mCurrentResult = mShares.begin();
	}
	
	NShares::const_iterator end = mShares.end(), start = mCurrentResult;
	
	if(start == mShares.begin())
		start = mShares.end();
	else
		--start;
	
	while(1)
	{
		if(mCurrentResult == end)
			mCurrentResult = mShares.begin();
		
		QStringList p = QStringList::split('\\', mCurrentResult.key(), true);
		
		NFolder::const_iterator it, end2 = mCurrentResult.data().end();
		for(it = mCurrentResult.data().begin(); it != end2; ++it)
		{
			if(it.key().contains(mQuery, false))
			{
				mFolders->show(p);
				mFiles->match(mQuery);
				++mCurrentResult;
				return;
			}
		}
		if(p.back().contains(mQuery, false))
		{
			mFolders->show(p);
			++mCurrentResult;
			return;
		}
		
		++mCurrentResult;
		if(mCurrentResult == start)
			break;
	}
}


class FolderListItem : public QListViewItem {
public:
	FolderListItem(QListViewItem* p, SharesData* data, const QString& title)
	              : QListViewItem(p), mData(data), mInited(false) { setText(0, title); setDragEnabled(true); };
	FolderListItem(QListView* p, SharesData* data, const QString& title)
	              : QListViewItem(p), mData(data), mInited(false) { setText(0, title); setDragEnabled(true); };
	SharesData* data() const { return mData; };
	
	void setup() {
		setExpandable(! mData->folders.empty());
		QListViewItem::setup();
	}
	
	void setOpen(bool b) {
		if(b && ! mInited) {
			QMap<QString, SharesData*>::ConstIterator it, end = mData->folders.end();
			for(it = mData->folders.begin(); it != end; ++it)
				new FolderListItem(this, it.data(), it.key());
			mInited = true;
		}
		QListViewItem::setOpen(b);
	}
	
private:
	SharesData* mData;
	bool mInited;
};

FolderListView::FolderListView(const QString& user, QWidget* parent, const char* name)
                        : QListView(parent, name), mUser(user), mShares(0) {
	addColumn(tr("Folder"));
	setRootIsDecorated(true);
	
	mPopupMenu = new QPopupMenu(this);
	mPopupMenu->insertItem(tr("Download folder"), this, SLOT(doDownloadFolder()));
	mPopupMenu->insertItem(tr("Copy URL"), this, SLOT(doCopyURL()));
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(doPopupMenu(QListViewItem*, const QPoint&, int)));	
	connect(this, SIGNAL(currentChanged(QListViewItem*)), SLOT(doCurrentChanged(QListViewItem*)));
}

FolderListView::~FolderListView() {
	delete mShares;
}

QDragObject* FolderListView::dragObject() {
	SlskDrag* drag = new SlskDrag(viewport());
	
	QListViewItemIterator item(firstChild(), QListViewItemIterator::Selected);
	for(; item.current() != 0; ++item) {
		FolderListItem* item2 = static_cast<FolderListItem*>(item.current());
		drag->append(mUser, item2->data()->path);
	}
	
	if(! drag->count()) {
		delete drag;
		return 0;
	}
	
	QString x;
	if(drag->count() == 1)
		x = tr("1 folder");
	else
		x = QString(tr("%1 folders")).arg(drag->count());
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

void FolderListView::doPopupMenu(QListViewItem* item, const QPoint& pos, int col) {
	mPopped = item;
	mPopupMenu->popup(pos);
}

void FolderListView::doDownloadFolder()
{
	if(mPopped) {
		QString d = static_cast<FolderListItem *>(mPopped)->data()->path;
		if ( "\\" == d.right(1) || "/" == d.right(1 ))
			museeq->downloadFolder(mUser, d.mid(0, d.length()-1));
		else
			museeq->downloadFolder(mUser, d);
	}
}
void FolderListView::doCopyURL() {
	if(mPopped) {
		QClipboard *cb = QApplication::clipboard();
		QString link;
		link  = ( "slsk://" +  mUser +  "/"+ static_cast<FolderListItem *>(mPopped)->data()->path);
		link.replace("\\", "/"); link.replace(" ", "%20");
		cb->setText( link , QClipboard::Clipboard);
}
		
	
}
void FolderListView::setShares(const NShares& shares) {
	clear();
	delete mShares;
	mShares = new SharesData("");
	
	NShares::const_iterator it = shares.begin();
	for(; it != shares.end(); ++it) {
		QString path;
		QStringList p = QStringList::split('\\', it.key(), true);
		SharesData* sfolder = mShares->get(p);
		sfolder->files = it.data();
	}
	
	QMap<QString, SharesData*>::Iterator dit, dend = mShares->folders.end();
	for(dit = mShares->folders.begin(); dit != dend; ++dit)
		new FolderListItem(this, dit.data(), dit.key());

}

void FolderListView::doCurrentChanged(QListViewItem* _item) {
	if(_item) {
		FolderListItem* item = static_cast<FolderListItem*>(_item);
		emit currentChanged(item->data()->path, item->data()->files);
	} else
		emit currentChanged("", NFolder());
}

void FolderListView::show(const QStringList& p)
{
	QStringList::const_iterator it, end = p.end();
	FolderListItem* item = 0;
	for(it = p.begin(); it != end; ++it)
	{
		FolderListItem* i;
		if(item == 0)
			i = static_cast<FolderListItem*>(firstChild());
		else
			i = static_cast<FolderListItem*>(item->firstChild());
		for(; i; i = static_cast<FolderListItem*>(i->nextSibling()))
			if(i->text(0) == *it)
				break;
		if(! i)
			return;
		item = i;
		setOpen(item, true);
	}
	clearSelection();
	setSelected(item, true);
	setCurrentItem(item);
	ensureItemVisible(item);
}


class FileListItem : public QListViewItem {
public:
	FileListItem(FileListView* parent, const QString& fn, const NFileData& data)
	            : QListViewItem(parent, fn, Util::makeSize(data.size), Util::makeTime(data.length), Util::makeBitrate(data.bitrate, data.vbr)),
	              mFilename(fn), mData(data) { };
	
	QString filename() const { return mFilename; };
	NFileData data() const { return mData; };
private:
	QString mFilename;
	NFileData mData;
};

FileListView::FileListView(const QString& user, QWidget* parent, const char* name)
             : QListView(parent, name), mUser(user), mPath(QString::null) {

	addColumn(tr("Filename"), 250);
	addColumn(tr("Size"), 100);
	addColumn(tr("Length"), 75);
	addColumn(tr("Bitrate"), 75);
	setColumnAlignment(1, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(2, Qt::AlignRight|Qt::AlignVCenter);
	setColumnAlignment(3, Qt::AlignRight|Qt::AlignVCenter);

	setSorting(0);
	setSelectionMode(Extended);
	setShowSortIndicator(true);
	setAllColumnsShowFocus(true);
	
	mPopupMenu = new QPopupMenu(this);
	mPopupMenu->insertItem(tr("Download files"), this, SLOT(doDownloadFiles()));
	if (museeq->nickname() == mUser) {
		mPopupMenu->insertItem(tr("Upload files"), this, SLOT(doUploadFiles()));
		}
	mPopupMenu->insertItem(tr("Copy URL"), this, SLOT(doCopyURL()));
	connect(this, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), SLOT(doPopupMenu(QListViewItem*, const QPoint&, int)));
}

void FileListView::setFiles(const QString& path, const NFolder& folder) {
	clear();
	
	mPath = path;
	
	if(path.isNull())
		return;
	
	NFolder::const_iterator it = folder.begin();
	for(; it != folder.end(); ++it) {
		QListViewItem* item = new FileListItem(this, it.key(), *it);
		item->setDragEnabled(true);
	}
}

QDragObject* FileListView::dragObject() {
	SlskDrag* drag = new SlskDrag(viewport());
	
	QListViewItemIterator item(firstChild(), QListViewItemIterator::Selected);
	for(; item.current() != 0; ++item) {
		FileListItem* _item = static_cast<FileListItem*>(item.current());
		NFileData data = _item->data();
		drag->append(mUser, mPath + _item->filename(), data.size);
	}
	
	if(! drag->count()) {
		delete drag;
		return 0;
	}
	
	QString x;
	if(drag->count() == 1)
		x = tr("1 file");
	else
		x = QString(tr("%1 files")).arg(drag->count());
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

void FileListView::doPopupMenu(QListViewItem* item, const QPoint& pos, int col) {
	mPopupMenu->popup(pos);
}

void FileListView::doDownloadFiles() {
	QListViewItemIterator item(firstChild(), QListViewItemIterator::Selected);
	for(; item.current() != 0; ++item) {
		FileListItem* _item = static_cast<FileListItem*>(item.current());
		NFileData data = _item->data();
		museeq->downloadFile(mUser, mPath + _item->filename(), data.size);
	}
}
void FileListView::doCopyURL() {
	QClipboard *cb = QApplication::clipboard();
	QListViewItemIterator item(firstChild(), QListViewItemIterator::Selected);
	for(; item.current() != 0; ++item) {
		FileListItem* _item = static_cast<FileListItem*>(item.current());
		NFileData data = _item->data();
		QString link;
		link  = ( "slsk://" +  mUser +  "/"+ mPath + _item->filename() );
		link.replace("\\", "/"); link.replace(" ", "%20"); link.replace("(", "%28"); link.replace(")", "%29");  link.replace("[", "%5B"); link.replace("]", "%5D");  link.replace("+", "%2B"); link.replace("~", "%7E"); link.replace("`", "%60"); link.replace("$", "%24"); link.replace("{", "%7B"); link.replace("}", "%7D"); link.replace('"', "%22"); link.replace(">", "%3E"); link.replace(",", "%2C"); link.replace("<", "%3C");
		cb->setText( link  , QClipboard::Clipboard );

		
		}
}

void FileListView::doUploadFiles() {
	bool ok = false;

	const QString& user = QInputDialog::getText(tr("Upload File(s)"),
	             tr("Which user do you wish to upload these to?"),
	             QLineEdit::Normal, QString::null, &ok, this);
	if(ok && user) {
		QListViewItemIterator item(firstChild(), QListViewItemIterator::Selected);
		for(; item.current() != 0; ++item) {
			FileListItem* _item = static_cast<FileListItem*>(item.current());
			NFileData data = _item->data();
			museeq->uploadFile(user, mPath + _item->filename());
		}
	}
}
void FileListView::match(const QString& query) {
	clearSelection();
	QListViewItemIterator item(firstChild());
	for(; item.current(); ++item)
		if(item.current()->text(0).contains(query, false))
			setSelected(item.current(), true);
}
