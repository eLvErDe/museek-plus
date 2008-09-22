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

#include "museeq.h"
#include "browser.h"
#include "util.h"
#include "images.h"
#include "codeccombo.h"

#include <QLineEdit>
#include <QInputDialog>
#include <QFileDialog>
#include <QSplitter>
#include <QClipboard>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QTreeWidget>
#include <QMenu>
#include <QIcon>
#include <QApplication>
#include <QMouseEvent>
#include <QUrl>
#include <QPainter>

class SharesData {
public:
	QString path;
	QMap<QString, SharesData*> folders;
	NFolder files;

	SharesData(const QString& _path) : path(_path), folders(), files() { }
	~SharesData() {
		QMap<QString, SharesData*>::Iterator it, end = folders.end();
		for(it = folders.begin(); it != end; ++it)
			delete it.value();
		folders.clear();
		files.clear();
	}

	SharesData* get(const QStringList& path, int i = 0) {
		if(i == path.size())
			return this;

		const QString& piece = path[i];
		SharesData* n;
		if(folders.contains(piece))
			n = folders[piece];
		else {
// 			QStringList::ConstIterator it, end = path.indexOf(i + 1);
			QString p;
			p = path.join("\\");
// 			for(it = path.begin(); it != end; ++it)
// 				p += *it + "\\";
			n = new SharesData(p);
			folders[piece] = n;
		}
		return n->get(path, i + 1);
	}
};


Browser::Browser(const QString& user, QWidget* parent, const char* name)
       : UserWidget(user, parent, name) {
	QVBoxLayout * MainLayout = new QVBoxLayout(this);
	QWidget* topWidget = new QWidget(this);
	MainLayout->addWidget(topWidget);
	QHBoxLayout * TopLayout = new QHBoxLayout(topWidget);
	TopLayout->setMargin(0);
	mUser = user;
// 	hbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	TopLayout->addWidget(new QLabel(tr("Search files and folders"), topWidget));
	mEntry = new QLineEdit(topWidget); /* "search"*/
	TopLayout->addWidget(mEntry);
	QFrame* frame = new QFrame(topWidget);
	TopLayout->addWidget(frame);
	frame->setFrameShape(QFrame::VLine);
// 	frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	mFileCount = new QLabel(tr("Haven't received shares"), topWidget);
	TopLayout->addWidget(mFileCount);
	TopLayout->addWidget(new CodecCombo("encoding.users", user, topWidget, "encoding"));
	QIcon refreshIcon = IMG("refresh");
	mRefresh = new QPushButton(tr("Refresh"), topWidget);
	mRefresh->setIcon(refreshIcon);
	connect(mRefresh, SIGNAL(clicked()), SLOT(getShares()));
	TopLayout->addWidget(mRefresh);
	QSplitter* split = new QSplitter(this);
	split->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	MainLayout->addWidget(split);

	// Folders panel
	mFolders = new FolderListView(user, split, "folders");
	mFolders->setEnabled(false);
	mFolders->resize(250, -1);

    // Files panel
	mFiles = new FileListView(user, split, "files");
	mFiles->setEnabled(false);
	split->setStretchFactor(1,10);
// 	mFiles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
	emit(highlight(1));
}

void Browser::getShares() {
	museeq->getUserShares(mUser);
}

/**
  * Search for the first files matching the query in these shared files. Will search next ones when using the same query several times.
  */
void Browser::doSearch() {
    // Find the query
	QString q = mEntry->text();
	if(q.isEmpty() || mShares.isEmpty())
		return;

    // It's a new query
	if(q != mQuery) {
		mQuery = q;
		mCurrentResult = mShares.begin();
	}

	// Where do we need to search now?
	NShares::const_iterator end = mShares.end();
	NShares::const_iterator start = mCurrentResult;

	while(1)
	{
		QStringList p = mCurrentResult.key().split('\\', QString::KeepEmptyParts);

		NFolder::const_iterator it;
		NFolder::const_iterator end2 = mCurrentResult.value().end();
		// Navigate in current folder
		for(it = mCurrentResult.value().begin(); it != end2; ++it)
		{
            // See if some file match query
			if(it.key().contains(mQuery, Qt::CaseInsensitive))
			{
			    // We've found some file!
				mFolders->show(p); // Show this folder content
				mFiles->match(mQuery); // Select every matching file
				++mCurrentResult; // Next search will be in next folder
				return;
			}
		}

		if(p.back().contains(mQuery, Qt::CaseInsensitive))
		{
		    // The folder name contains the query
			mFolders->show(p); // Show this folder content
			++mCurrentResult; // Next search will be in next folder
			return;
		}

        // No result found, go to next folder
		++mCurrentResult;

        // If it's the last folder, go back to start
		if(mCurrentResult == end)
			mCurrentResult = mShares.begin();

		// We've searched in every folder, stop it now
		if(mCurrentResult == start)
			return;
	}
}


class FolderListItem : public QTreeWidgetItem {
public:
	FolderListItem(QTreeWidgetItem* p, SharesData* data, const QString& title, const QString& fullpath )
	        : QTreeWidgetItem(p), mData(data)
        {
            setText(0, title); setText(1, fullpath);
        };
	FolderListItem(QTreeWidget* p, SharesData* data, const QString& title, const QString& fullpath)
	        : QTreeWidgetItem(p), mData(data)
		{
			setText(0, title);
			setText(1, fullpath);
			setExpanded(true);
		};

	SharesData* data() const { return mData; };

private:
	SharesData* mData;
};

FolderListView::FolderListView(const QString& user, QWidget* parent, const char* name)
                        : QTreeWidget(parent), mUser(user), mShares(0) {

	QStringList headers;
	headers << tr("Folder");
	setHeaderLabels(headers);
	setSortingEnabled(true);
 	setAllColumnsShowFocus(true);

	setRootIsDecorated(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setDragEnabled(true);
	setDropIndicatorShown(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	mPopupMenu = new QMenu(this);

	QAction * ActionDownloadFolder, * ActionDownloadFolderTo, * ActionUploadFolder, * ActionCopyURL;

	ActionDownloadFolder = new QAction( tr("Download Folder"), this);
	connect(ActionDownloadFolder, SIGNAL(triggered()), this, SLOT(doDownloadFolder()));
	mPopupMenu->addAction(ActionDownloadFolder);

	ActionDownloadFolderTo = new QAction( tr("Download Folder to..."), this);
	connect(ActionDownloadFolderTo, SIGNAL(triggered()), this, SLOT(doDownloadFolderTo()));
	mPopupMenu->addAction(ActionDownloadFolderTo);

	ActionUploadFolder = new QAction( tr("Upload Folder"), this);
	connect(ActionUploadFolder, SIGNAL(triggered()), this, SLOT(doUploadFolder()));
	if (museeq->nickname() == mUser)
		mPopupMenu->addAction(ActionUploadFolder);

	ActionCopyURL = new QAction( tr("Copy URL"), this);
	connect(ActionCopyURL, SIGNAL(triggered()), this, SLOT(doCopyURL()));
	mPopupMenu->addAction(ActionCopyURL);

// 	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));
	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(doCurrentChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
}

FolderListView::~FolderListView() {
	delete mShares;
}

/**
  * User have press mouse button in this widget
  */
void FolderListView::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    if (event->button() == Qt::LeftButton)
        mDragStartPosition = event->pos();

    QTreeWidget::mousePressEvent(event);
}

/**
  * User have moved his mouse in this widget
  */
void FolderListView::mouseMoveEvent(QMouseEvent *event)
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
        FolderListItem * item = static_cast<FolderListItem*>(*it);
        // slsk protocol: in QUrl, hostname is always lower case.
        // So we put username as hostname for compatibility, and as username to have the correct case.
        // Ex: slsk://MuSeEk:filesize@museek/path/to/a/file
        // Code should first look at QUrl::userName() and if not present, try QUrl::host()
        QUrl url("slsk://" + mUser);
        url.setUserName(mUser);
        url.setPath(item->data()->path.replace("\\", "/") + "/");

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

	QString x(tr("%n folder(s)", "", urls.count()));
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

void FolderListView::slotActivate(QTreeWidgetItem* item, int column) {
	mPopped = static_cast<FolderListItem*>(item);
	if(! mPopped)
		return;

	doDownloadFolder();
}
void FolderListView::slotContextMenu(const QPoint& pos) {
// 	QTreeWidgetItem * item = 0 ;
	mPopped = itemAt(pos) ;
	if (! mPopped ) {

		return;
	}
	mPopupMenu->exec(mapToGlobal(pos));
}

void FolderListView::doPopupMenu(QTreeWidgetItem* item, const QPoint& pos, int col) {
	mPopped = item;
	mPopupMenu->popup(pos);
}

void FolderListView::doDownloadFolder()
{
	if(mPopped) {
		QString d = static_cast<FolderListItem *>(mPopped)->text(1);
		if ( "\\" == d.right(1) || "/" == d.right(1 ))
			museeq->downloadFolder(mUser, d.mid(0, d.length()-1));
		else
			museeq->downloadFolder(mUser, d);
	}
}

void FolderListView::doDownloadFolderTo()
{
	if(mPopped) {
        QList<QTreeWidgetItem *> downloads = selectedItems ();
        QFileDialog * fd = new QFileDialog(this, QDir::homePath());
        fd->setWindowTitle(tr("Select a Directory for current download(s)"));
        fd->setFileMode(QFileDialog::Directory);
        if(fd->exec() == QDialog::Accepted){
            QString localpath = fd->directory().path();
            QList<QTreeWidgetItem *>::iterator it = downloads.begin();
            for(; it != downloads.end(); ++it) {
                QString d = static_cast<FolderListItem *>(mPopped)->text(1);
                if ( "\\" == d.right(1) || "/" == d.right(1 ))
                    d = d.mid(0, d.length()-1);
                museeq->downloadFolderTo(mUser, d, localpath);
            }
        }
        delete fd;
	}
}

void FolderListView::doUploadFolder()
{
	if(mPopped) {
        bool ok = false;

        QStringList buddies = museeq->buddies();
        const QString& user = QInputDialog::getItem(this, tr("Upload Folder"),
                     tr("Which user do you wish to upload this to?"),
                     buddies, 0, true, &ok);
        if(ok && ! user.isEmpty()) {
            QString d = static_cast<FolderListItem *>(mPopped)->text(1);
            if ( "\\" == d.right(1) || "/" == d.right(1 ))
                museeq->uploadFolder(user, d.mid(0, d.length()-1));
            else
                museeq->uploadFolder(user, d);
        }
	}
}

void FolderListView::doCopyURL() {
	if(mPopped) {
		QClipboard *cb = QApplication::clipboard();
        QUrl url("slsk://" + mUser);
        url.setUserName(mUser);
        url.setPath(static_cast<FolderListItem *>(mPopped)->data()->path.replace("\\", "/") + "/");
		QString link = url.toString();
		cb->setText( link , QClipboard::Clipboard);
	}
}

QString  FolderListView::parentPath(const QString& parent) {

	QStringList newpath = parent.split("\\");
	newpath.removeLast();
	return newpath.join("\\");
}

FolderListItem * FolderListView::findParent(const QStringList& p) {
	QString path = parentPath(p.join("\\"));
	if (path.isEmpty()) {

		return static_cast<FolderListItem *>(invisibleRootItem());
	}
	QTreeWidgetItemIterator it(this);
	while (*it) {
		if ((*it)->text(1) == path)
			return static_cast<FolderListItem *>(*it);
// 			treeWidget->setItemSelected(*it, true);
		++it;
	}

	return new FolderListItem(findParent(path.split("\\")), mShares->get(path.split("\\")), path.split("\\").last(), path);

}

void FolderListView::setShares(const NShares& shares) {
	clear();
	delete mShares;
	mShares = new SharesData("");

	NShares::const_iterator it = shares.begin();
	for(; it != shares.end(); ++it) {
		QString path;
		QStringList p;
		p = it.key().split("\\", QString::KeepEmptyParts);
		SharesData* sfolder = mShares->get(p);
		sfolder->files = it.value();

		QList<QTreeWidgetItem *> Folders = findItems(it.key(), Qt::MatchExactly, 1);
		if (Folders.isEmpty()) {
			new FolderListItem(findParent(p), sfolder, p.last(), it.key());
		}
	}

	sortItems(0, Qt::AscendingOrder);
}

void FolderListView::doCurrentChanged(QTreeWidgetItem* _item, QTreeWidgetItem* lastItem) {
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
	int position;
	for(it = p.begin(); it != end; ++it)
	{
		position = 0;
		FolderListItem* i;
		if(item == 0)
			i = static_cast<FolderListItem*>(invisibleRootItem ());
		else
			i = static_cast<FolderListItem*>(item->child(position));

		for(; i; i = static_cast<FolderListItem*>(item->child(position)))
		{
			if(i->text(0) == *it)
				break;
			position += 1;
		}
		if(! i)
			return;
		item = i;
		i->setExpanded(true);
	}
	clearSelection();
 	item->setSelected(item);
	setCurrentItem(item);
	scrollTo(indexFromItem(item));
}


class FileListItem : public QTreeWidgetItem {
public:
	FileListItem(FileListView* parent, const QString& fn, const NFileData& data);


// 	: QTreeWidgetItem(parent), mFilename(fn), mData(data) { }


	QString filename() const { return mFilename; };
	NFileData data() const { return mData; };
	bool operator<(const QTreeWidgetItem & other) const;
private:
	QString mFilename;
	NFileData mData;
};
FileListItem::FileListItem(FileListView* parent, const QString& fn, const NFileData& data)
	: QTreeWidgetItem(parent), mFilename(fn), mData(data) {
// 	mFilename = fn;
// 	mData = data;
	setText(0, filename());
	setText(1, Util::makeSize(data.size));
	setText(2, Util::makeTime(data.length));
	setText(3, Util::makeBitrate(data.bitrate, data.vbr));
}

bool FileListItem::operator<(const QTreeWidgetItem & other_) const {
  const FileListItem * other = static_cast<const FileListItem *>(&other_);
  int col = 0;
  if(treeWidget())
    col = treeWidget()->sortColumn();

  switch(col)
  {
    case 0:
      return filename().toLower() < other->filename().toLower();
    case 1:
      if(data().size == other->data().size)
        return filename().toLower() < other->filename().toLower();
      return data().size < other->data().size;
    case 2:
      if(data().bitrate == other->data().bitrate)
        return filename().toLower() < other->filename().toLower();
      return data().bitrate < other->data().bitrate;

  }

  return false;
}

FileListView::FileListView(const QString& user, QWidget* parent, const char* name)
             : QTreeWidget(parent), mUser(user), mPath(QString::null) {
	QStringList headers;
	headers << tr("Filename") << tr("Size") << tr("Length") << tr("Bitrate");
	setHeaderLabels(headers);
	setSortingEnabled(true);
	setRootIsDecorated(false);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setAlternatingRowColors(true);
	setDragEnabled(true);
 	setAllColumnsShowFocus(true);
 	setSelectionMode(QAbstractItemView::ExtendedSelection);

	mPopupMenu = new QMenu(this);
	QAction * ActionDownloadFiles, * ActionDownloadFilesTo, * ActionUploadFiles, * ActionCopyURL;

	ActionDownloadFiles = new QAction( tr("Download files"), this);
	connect(ActionDownloadFiles, SIGNAL(triggered()), this, SLOT(doDownloadFiles()));
	mPopupMenu->addAction(ActionDownloadFiles);

	ActionDownloadFilesTo = new QAction( tr("Download files to.."), this);
	connect(ActionDownloadFilesTo, SIGNAL(triggered()), this, SLOT(doDownloadFilesTo()));
	mPopupMenu->addAction(ActionDownloadFilesTo);

	ActionUploadFiles = new QAction( tr("Upload files"), this);
	connect(ActionUploadFiles, SIGNAL(triggered()), this, SLOT(doUploadFiles()));
	if (museeq->nickname() == mUser) {
		mPopupMenu->addAction(ActionUploadFiles);
	}

	ActionCopyURL = new QAction( tr("Copy URL"), this);
	connect(ActionCopyURL, SIGNAL(triggered()), this, SLOT(doCopyURL()));
	mPopupMenu->addAction(ActionCopyURL);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(slotActivate(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotContextMenu(const QPoint&)));

}

/**
  * User have press mouse button in this widget
  */
void FileListView::mousePressEvent(QMouseEvent *event)
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
void FileListView::mouseMoveEvent(QMouseEvent *event)
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

	QList<QTreeWidgetItem*> items = selectedItems();
    QList<QTreeWidgetItem*>::const_iterator it = items.begin();
	for(; it != items.end(); ++it) {
        FileListItem * item = static_cast<FileListItem*>(*it);

        // slsk protocol: in QUrl, hostname is always lower case.
        // So we put username as hostname for compatibility, and as username to have the correct case.
        // Ex: slsk://MuSeEk:filesize@museek/path/to/a/file
        // Code should first look at QUrl::userName() and if not present, try QUrl::host()
        QUrl url("slsk://" + mUser);
        url.setUserName(mUser);
        url.setPath(mPath.replace("\\", "/") + "/" + item->filename());
        url.setPassword(QString::number(item->data().size));

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

	QString x(tr("%n file(s)", "", urls.count()));
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

void FileListView::slotActivate(QTreeWidgetItem* item, int column) {

	FileListItem* _item = static_cast<FileListItem*>(item);
	if(! _item)
		return;
	doDownloadFiles();
}
void FileListView::slotContextMenu(const QPoint& pos) {
	QTreeWidgetItem * item = 0 ;
	item = itemAt(pos) ;
	if (! item ) {

		return;
	}
	mPopupMenu->exec(mapToGlobal(pos));
}

void FileListView::setFiles(const QString& path, const NFolder& folder) {
	clear();

	mPath = path;

	if(path.isNull())
		return;

	NFolder::const_iterator it = folder.begin();
	for(; it != folder.end(); ++it) {
		new FileListItem(this, it.key(), *it);
	}
	sortItems(0, Qt::AscendingOrder);
	resizeColumnToContents(0);
}

void FileListView::doPopupMenu(QTreeWidgetItem* item, const QPoint& pos, int col) {
	mPopupMenu->popup(pos);
}

void FileListView::doDownloadFiles() {

	QList<QTreeWidgetItem *> downloads = selectedItems ();
	QList<QTreeWidgetItem *>::iterator it = downloads.begin();
	for(; it != downloads.end(); ++it) {
		FileListItem* _item = static_cast<FileListItem*>(*it);
		NFileData data = _item->data();
		museeq->downloadFile(mUser, mPath +"\\"+  _item->filename(), data.size);
	}

}

void FileListView::doDownloadFilesTo() {
	QList<QTreeWidgetItem *> downloads = selectedItems ();
	QFileDialog * fd = new QFileDialog(this, QDir::homePath());
	fd->setWindowTitle(tr("Select a Directory for current download(s)"));
	fd->setFileMode(QFileDialog::Directory);
	if(fd->exec() == QDialog::Accepted){
		QString localpath = fd->directory().path();
		QList<QTreeWidgetItem *>::iterator it = downloads.begin();
		for(; it != downloads.end(); ++it) {
			FileListItem* _item = static_cast<FileListItem*>(*it);
			NFileData data = _item->data();
			museeq->downloadFileTo(mUser, mPath +"\\"+  _item->filename(), localpath, data.size);
		}
	}
	delete fd;
}

void FileListView::doCopyURL() {
	QClipboard *cb = QApplication::clipboard();
	QList<QTreeWidgetItem *> downloads = selectedItems ();

	if (! downloads.isEmpty()) {
		FileListItem* _item = static_cast<FileListItem*>(downloads.at(0));
		NFileData data = _item->data();
		QString link;
		link  = ( "slsk://" +  mUser +  "/"+ mPath + _item->filename() );
		link.replace("\\", "/"); link.replace(" ", "%20"); link.replace("(", "%28"); link.replace(")", "%29");  link.replace("[", "%5B"); link.replace("]", "%5D");  link.replace("+", "%2B"); link.replace("~", "%7E"); link.replace("`", "%60"); link.replace("$", "%24"); link.replace("{", "%7B"); link.replace("}", "%7D"); link.replace('"', "%22"); link.replace(">", "%3E"); link.replace(",", "%2C"); link.replace("<", "%3C");
		cb->setText( link  , QClipboard::Clipboard );


	}

}

void FileListView::doUploadFiles() {
	bool ok = false;

	QStringList buddies = museeq->buddies();
	const QString& user = QInputDialog::getItem(this, tr("Upload File(s)"),
	             tr("Which user do you wish to upload these to?"),
	             buddies, 0, true, &ok);
	if(ok && ! user.isEmpty()) {
		QList<QTreeWidgetItem *> uploads = selectedItems ();
		QList<QTreeWidgetItem *>::iterator it = uploads.begin();
		for(; it != uploads.end(); ++it) {
			FileListItem* _item = static_cast<FileListItem*>(*it);
			NFileData data = _item->data();
			museeq->uploadFile(user, mPath +"\\"+ _item->filename());
		}

	}
}

/**
  * Select every file matching the query
  */
void FileListView::match(const QString& query) {
	clearSelection();
    QList<QTreeWidgetItem *> items = findItems(query, Qt::MatchContains);
    QList<QTreeWidgetItem *>::iterator it = items.begin();
    for(; it != items.end(); ++it) {
        (*it)->setSelected(true);
    }
}
