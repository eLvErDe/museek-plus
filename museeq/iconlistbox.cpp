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

#include "iconlistbox.h"
#include "util.h"

#include <QPainter>
#include <QBitmap>
#include <QPixmap>
#include <QScrollBar>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QUrl>

IconListBox::IconListBox( QWidget *parent, const char *name, bool verticalIconBox )
  : QListWidget( parent )
{
    setAcceptDrops(true);
    setViewMode ( QListView::IconMode);
    setWordWrap (true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(slotCurrentChanged(QListWidgetItem*, QListWidgetItem*)));

    mVerticalIconBox = verticalIconBox;
    if (!verticalIconBox)
        {
                setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                setFlow(QListView::TopToBottom);
        }
    else
        {
                setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                setFlow(QListView::LeftToRight);
        }
}

void IconListBox::updateMinimumHeight()
{
    if (! mVerticalIconBox) {
        int maxHeight = 10;

        QList<QListWidgetItem*> icons = findItems(QString("*"),Qt::MatchWildcard );
        QList<QListWidgetItem *>::iterator it = icons.begin();
        for(; it != icons.end();  ++it) {
            IconListItem* ilitem = dynamic_cast<IconListItem*>(*it);
            if (!ilitem)
                continue;
            int w = ilitem->height(this);
            maxHeight = qMax( w, maxHeight );
        }
        it = icons.begin();
        for(; it != icons.end();  ++it) {
            IconListItem* item = dynamic_cast<IconListItem*>(*it);
            if (!item)
                continue;
            item->setSizeHint(QSize(item->width(this), maxHeight));
        }

        if( verticalScrollBar()->isVisible() ) {
            maxHeight += horizontalScrollBar()->sizeHint().height();
        }
        setMaximumHeight( maxHeight + 10 );
    }
    else {
        int h = 0;
        QList<QListWidgetItem *> icons = findItems(QString("*"),Qt::MatchWildcard );
        QList<QListWidgetItem *>::const_iterator it = icons.begin();

        for (; it != icons.end(); it++) {
            IconListItem* item = dynamic_cast<IconListItem*>(*it);
            if (item)
                h += item->height(this);
        }

        setMinimumHeight( h );
    }
}

void IconListBox::updateMinimumWidth()
{
    if (! mVerticalIconBox) {
        int w = 0;
        QList<QListWidgetItem *> icons = findItems(QString("*"),Qt::MatchWildcard );
        QList<QListWidgetItem *>::const_iterator it = icons.begin();

        for (; it != icons.end(); it++) {
            IconListItem* item = dynamic_cast<IconListItem*>(*it);
            if (item)
                w += item->width(this) + 5;
        }

        setMinimumWidth( w );
    }
    else {
        int maxWidth = 10;

        QList<QListWidgetItem*> icons = findItems(QString("*"),Qt::MatchWildcard );
        QList<QListWidgetItem *>::iterator it = icons.begin();
        for(; it != icons.end();  ++it) {
            IconListItem* item = dynamic_cast<IconListItem*>(*it);
            if (!item)
                continue;
            int w = item->width(this);
            maxWidth = qMax( w, maxWidth );
        }
        it = icons.begin();
        for(; it != icons.end();  ++it) {
            IconListItem* item = dynamic_cast<IconListItem*>(*it);
            if (item)
                item->setSizeHint(QSize(maxWidth - 5, item->height(this)));
        }

        if( verticalScrollBar()->isVisible() ) {
            maxWidth += verticalScrollBar()->sizeHint().width();
        }
        setFixedWidth( maxWidth );
    }
}

QStyleOptionViewItem IconListBox::viewOptions () const {
    QStyleOptionViewItem item;
    item = QListWidget::viewOptions();
    item.decorationAlignment = Qt::AlignCenter;
    return item;
}

void IconListBox::slotCurrentChanged(QListWidgetItem* item, QListWidgetItem* old)
{
  IconListItem* _item = dynamic_cast<IconListItem*>(item);
  if (_item)
    _item->selected();
}

void IconListBox::dragMoveEvent(QDragMoveEvent* event)
{
    // Find the item
    QListWidgetItem* item = itemAt(event->pos());

    // Switch to the item if any found
    if(item && (currentItem() != item))
        setCurrentItem(item);

    // We can drop urls directly on the icon: accept
    IconListItem* it = dynamic_cast<IconListItem*>(item);
    if(Util::hasSlskUrls(event) && it && it->canDrop())
        event->acceptProposedAction();
}

void IconListBox::dragEnterEvent(QDragEnterEvent* event)
{
    event->acceptProposedAction();
}

void IconListBox::dropEvent(QDropEvent* event)
{
    IconListItem* item = dynamic_cast<IconListItem*>(currentItem());

    if (item && Util::hasSlskUrls(event, item->dropNeedPath()) && item->canDrop())
        item->emitDropSlsk(event->mimeData()->urls());

    event->acceptProposedAction();
}


IconListItem::IconListItem( QListWidget *listbox, const QPixmap &pixmap,
                                          const QString &text )
  : QObject( listbox ), QListWidgetItem( listbox ), mPixmap(pixmap), mHighlight(0), mCanDrop(false)
{
    mDropNeedPath = false;
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled);
    if( mPixmap.isNull() )
        mPixmap = defaultPixmap();
    setIcon(QIcon(mPixmap));
    setText( text );
}

const QPixmap &IconListItem::defaultPixmap()
{
  static QPixmap *pix=0;
  if( pix == 0 )
  {
    pix = new QPixmap( 32, 32 );
    QPainter p( pix );
    p.eraseRect( 0, 0, pix->width(), pix->height() );
    p.setPen( Qt::red );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    QBitmap mask( pix->width(), pix->height() ); //, true
    mask.fill( Qt::black );
    p.begin( &mask );
    p.setPen( Qt::white );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    pix->setMask( mask );
  }
  return( *pix );
}

int IconListItem::height( const QListWidget *lb ) const
{
  if( text().isEmpty() )
    return( mPixmap.height() + 10);
  else {
    int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter | Qt::TextWordWrap, text() ).height();
    return( mPixmap.height() + ht + 10 );
  }
}

int IconListItem::width( const QListWidget *lb ) const
{
  int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter | Qt::TextWordWrap, text() ).width() + 10;
  int wp = mPixmap.width() + 10;
  int w  = qMax( wt, wp );
  return w;
}

/**
  * Does this item accept droping on it?
  */
bool IconListItem::canDrop() const
{
  return mCanDrop;
}

void IconListItem::setCanDrop(bool b)
{
  mCanDrop = b;
}

/**
  * Does this item needs a path when droping a slsk QUrl?
  */
bool IconListItem::dropNeedPath() const
{
  return mDropNeedPath;
}

void IconListItem::setDropNeedPath(bool b)
{
  mDropNeedPath = b;
}

void IconListItem::emitDropSlsk(const QList<QUrl>& l)
{
    emit dropSlsk(l);
}

/**
  * Alert the user
  */
void IconListItem::setHighlight(int level)
{
	if(! isSelected() && level > highlighted()) {
		setHighlighted(level);
		QFont oldfont = font();
		if(highlighted() > 0)
			oldfont.setUnderline(true);

		if(highlighted() > 1)
			setForeground(QBrush(QColor(255, 0, 0)));
		setFont(oldfont);
  }
}

/**
  * The user has seen the alert
  */
void IconListItem::selected()
{
	if(highlighted() != 0) {
		QFont oldfont = font();
		setHighlighted(0);
		oldfont.setUnderline(false);
		setFont(oldfont);
		setForeground(QBrush(listWidget()->palette().color(QPalette::Foreground)));
    }
}

