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

#include "iconlistbox.h"
#include <qpainter.h>
#include <qbitmap.h>

#include "slskdrag.h"

IconListBox::IconListBox( QWidget *parent, const char *name )
  : QListBox( parent, name )
{
  setAcceptDrops(TRUE);
  connect(this, SIGNAL(currentChanged(QListBoxItem*)), SLOT(slotCurrentChanged(QListBoxItem*)));
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}


void IconListBox::dragMoveEvent(QDragMoveEvent* event)
{
  bool canDrop = true;
  if(!SlskDrag::canDecode(event))
    canDrop = false;
  
  QListBoxItem* item = itemAt(event->pos());

  if(item && ! isSelected(item))
    setSelected(item, true);

  event->accept(canDrop && item && static_cast<IconListItem*>(item)->canDrop());
}


void IconListBox::dropEvent(QDropEvent* event)
{
  QStringList l;
  if(SlskDrag::decode(event, l)) {
    IconListItem* item = static_cast<IconListItem*>(selectedItem());
    if(item)
      item->emitDropSlsk(l);
  }
}


void IconListBox::updateMinimumHeight()
{
  int h = frameWidth()*2;
  for( QListBoxItem *i = item(0); i != 0; i = i->next() )
  {
    h += i->height( this );
  }
  setMinimumHeight( h );
}


void IconListBox::updateWidth()
{
  int maxWidth = 10;
  for( QListBoxItem *i = item(0); i != 0; i = i->next() )
  {
    int w = ((IconListItem *)i)->width(this);
    maxWidth = QMAX( w, maxWidth );
  }
  for( QListBoxItem *i = item(0); i != 0; i = i->next() )
  {
    ((IconListItem *)i)->expandMinimumWidth( maxWidth );
  }
  if( verticalScrollBar()->isVisible() )
  {
    maxWidth += verticalScrollBar()->sizeHint().width();
  }
  setFixedWidth( maxWidth + frameWidth()*2 );
}

void IconListBox::slotCurrentChanged(QListBoxItem* item)
{
  static_cast<IconListItem*>(item)->selected();
}


IconListItem::IconListItem( QListBox *listbox, const QPixmap &pixmap,
                                          const QString &text )
  : QObject( listbox ), QListBoxItem( listbox ), mPixmap(pixmap), mHighlight(0), mCanDrop(false)
{
  if( mPixmap.isNull() == true )
    mPixmap = defaultPixmap();
  
  setText( text );
  mMinimumWidth = 0;
}


int IconListItem::expandMinimumWidth( int width )
{
  mMinimumWidth = QMAX( mMinimumWidth, width );
  return( mMinimumWidth );
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

    QBitmap mask( pix->width(), pix->height(), true );
    mask.fill( Qt::black );
    p.begin( &mask );
    p.setPen( Qt::white );
    p.drawRect ( 0, 0, pix->width(), pix->height() );
    p.end();

    pix->setMask( mask );
  }
  return( *pix );
}


void IconListItem::paint( QPainter *painter )
{
  QFontMetrics fm = painter->fontMetrics();
  int ht = fm.boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
  int wp = mPixmap.width();
  int hp = mPixmap.height();

  painter->drawPixmap( (mMinimumWidth-wp)/2, 5, mPixmap );
  if( text().isEmpty() == false )
  {
    QPen oldpen = painter->pen();
    QFont oldfont = painter->font(), font = oldfont;
    
    if(mHighlight > 0)
      font.setUnderline(true);
    painter->setFont(font);
    
    if(mHighlight > 1)
      painter->setPen(QColor(255, 0, 0));
    
    painter->drawText( 0, hp+7, mMinimumWidth, ht, Qt::AlignCenter, text() );
    
    painter->setFont(oldfont);
    painter->setPen(oldpen);
  }
}

int IconListItem::height( const QListBox *lb ) const
{
  if( text().isEmpty() == true )
  {
    return( mPixmap.height() );
  }
  else
  {
    int ht = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).height();
    return( mPixmap.height() + ht + 10 );
  }
}


int IconListItem::width( const QListBox *lb ) const
{
  int wt = lb->fontMetrics().boundingRect( 0, 0, 0, 0, Qt::AlignCenter, text() ).width() + 10;
  int wp = mPixmap.width() + 10;
  int w  = QMAX( wt, wp );
  return( QMAX( w, mMinimumWidth ) );
}


bool IconListItem::canDrop() const
{
  return mCanDrop;
}

void IconListItem::setCanDrop(bool b)
{
  mCanDrop = b;
}

void IconListItem::emitDropSlsk(const QStringList& l)
{
  emit dropSlsk(l);
}

void IconListItem::setHighlight(int level)
{
  if(! isSelected() && level > mHighlight) {
    mHighlight = level;
    static_cast<QListBox*>(parent())->triggerUpdate(false);
  }
}

void IconListItem::selected()
{
  if(mHighlight != 0) {
    mHighlight = 0;
    static_cast<QListBox*>(parent())->triggerUpdate(false);
  }
}

