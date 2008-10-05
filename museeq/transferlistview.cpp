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

#include "transferlistview.h"
#include "transferlistitem.h"
#include "museeq.h"
#include "util.h"

#include <QDropEvent>
#include <QList>
#include <QUrl>
#include <QPainter>

TransferListView::TransferListView(bool place, QWidget* _p, const char* _n)
                 : QTreeWidget(_p), mGroupMode(None) {

	QStringList headers;
	headers << tr("User") << tr("File") << tr("Status") << tr("Progress") << tr("Place") << tr("Position") << tr("Size") << tr("Speed") << tr("Time Left") << tr("Path") << QString::null;

    setAllColumnsShowFocus(true);
	setHeaderLabels(headers);
	setSortingEnabled ( false );
	setDragEnabled(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	if(! place)
		setColumnHidden(4, true);

	setColumnWidth ( 0, 100 );
	setColumnWidth ( 1, 250 );
	setColumnWidth ( 2, 100 );
	setColumnWidth ( 3, 115 );
	setColumnWidth ( 4, 50 );
	setColumnWidth ( 5, 75 );
	setColumnWidth ( 6, 75 );
	setColumnWidth ( 7, 75 );
	setColumnWidth ( 8, 75 );
	setColumnWidth ( 9, 250 );
	setColumnWidth ( 10, 0 );

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(museeq, SIGNAL(disconnected()), SLOT(clear()));
}

TransferListItem* TransferListView::findTransfer(const QString& _u, const QString& _p) {
	QTreeWidgetItemIterator it(this);
	while (*it) {
		if (((static_cast<TransferListItem *>(*it))->user() == _u) && ((static_cast<TransferListItem *>(*it))->path() == _p))
 			return static_cast<TransferListItem *>(*it);
		++it;
	}

	return static_cast<TransferListItem *>(0);
}

TransferListItem* TransferListView::findParent(const QString& user) {
	if (mGroupMode == None)
		return static_cast<TransferListItem *>(invisibleRootItem());

	TransferListItem * parent = 0;
	QList<QTreeWidgetItem *> Groups = TransferListView::findItems(user, Qt::MatchExactly, 0);
	QList<QTreeWidgetItem *>::iterator transfers_it = Groups.begin();
	for(; transfers_it != Groups.end();  ++transfers_it) {
		if (static_cast<TransferListItem *>(*transfers_it)->path().isNull()) {
			parent = static_cast<TransferListItem *>(*transfers_it);
		}
	}

	if (! parent)
		parent = new TransferListItem(this, user, QString::null);

	addTopLevelItem(parent);
	return parent;

}

void TransferListView::update(const NTransfer& transfer) {
	TransferListItem* file = findTransfer(transfer.user, transfer.filename);
	if (!file )
		file = new TransferListItem(findParent(transfer.user), transfer.user, transfer.filename);

	file->update(transfer);
}

void TransferListView::remove(const QString& _u, const QString& _p) {
	TransferListItem* item = findTransfer(_u, _p);
	if (! item )
		return;

	if(item->user() == _u && (mGroupMode != None || item->path() == _p)) {
		if(mGroupMode == None) {
			museeq->output("delete item");
			delete static_cast<QTreeWidgetItem *>(item);
		} else {
            delete item;
            TransferListItem* parent = findParent(_u);
			if(parent->childCount() == 0)
                delete parent;
            else
                parent->updateStats();
		}
		return;
	}

}

TransferListView::GroupMode TransferListView::groupMode() const {
	return mGroupMode;
}

void TransferListView::setGroupMode(GroupMode mode) {
	if(mode == mGroupMode)
		return;

	mGroupMode = mode;

	if(mGroupMode == None) {
		QList<QTreeWidgetItem *> items;
		QList<QTreeWidgetItem *> subitems;
		QList<QTreeWidgetItem *>::iterator sitx;
		items = invisibleRootItem()->takeChildren ();
		QList<QTreeWidgetItem *>::iterator itx = items.begin();
		for(; itx != items.end(); ++itx) {
			if ( ! ( static_cast<TransferListItem *>(*itx))->text(1).isNull()) {
				invisibleRootItem()->addChild(*itx);
				static_cast<TransferListItem *>(*itx)->updateProgressBar();
			}
			else {
				subitems = (*itx)->takeChildren();
				sitx = subitems.begin();
				for(; sitx != subitems.end(); ++sitx) {
					invisibleRootItem()->addChild(*sitx);
                    static_cast<TransferListItem *>(*sitx)->updateProgressBar();
				}
			}
		}

		setRootIsDecorated(false);
	}
	else {
		QList<QTreeWidgetItem *> items;

		items = invisibleRootItem()->takeChildren ();

		QList<QTreeWidgetItem *>::iterator itx = items.begin();
		for(; itx != items.end(); ++itx) {
			if ( (static_cast<TransferListItem *>(*itx))->text(1).isNull()) {
				invisibleRootItem()->addChild(*itx);
				static_cast<TransferListItem *>(*itx)->updateProgressBar();
			}
		}
		itx = items.begin();
		for(; itx != items.end(); ++itx) {
			if (! (static_cast<TransferListItem *>(*itx))->text(1).isNull()) {
				findParent((static_cast<TransferListItem *>(*itx))->user())->addChild(*itx);
				static_cast<TransferListItem *>(*itx)->updateProgressBar();
			}
		}

		updateParentsStats();

		setRootIsDecorated(true);
	}
}

void TransferListView::updateParentsStats() {
	if(mGroupMode == None)
		return;

	int topit = 0;

	for(; topit < topLevelItemCount(); topit++) {
		static_cast<TransferListItem *>(topLevelItem(topit))->updateStats();
	}
}

void TransferListView::dragMoveEvent(QDragMoveEvent* event) {
    event->acceptProposedAction();
}

void TransferListView::dragEnterEvent(QDragEnterEvent* event) {
    event->acceptProposedAction();
}

void TransferListView::dropEvent(QDropEvent* event) {
    event->acceptProposedAction();

    if (Util::hasSlskUrls(event, true) && acceptDrops())
        emit dropSlsk(event->mimeData()->urls());
}

/**
  * User have press mouse button in this widget
  */
void TransferListView::mousePressEvent(QMouseEvent *event)
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
void TransferListView::mouseMoveEvent(QMouseEvent *event)
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
	    TransferListItem * item = static_cast<TransferListItem*>(*it);

        if(users.indexOf(item->user()) == -1)
            users << item->user();

	    // slsk protocol: in QUrl, hostname is always lower case.
	    // So we put username as hostname for compatibility, and as username to have the correct case.
	    // Ex: slsk://MuSeEk@museek/path/to/a/file
	    // Code should first look at QUrl::userName() and if not present, try QUrl::host()
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

	QString x(tr("%n transfer(s)", "", items.count()) + tr(" (%n user(s))", "", users.count()));
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
