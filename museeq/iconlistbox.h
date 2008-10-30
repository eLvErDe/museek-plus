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

#ifndef ICONLISTBOX_H
#define ICONLISTBOX_H

#include <QListWidget>

class IconListItem : public QObject, public QListWidgetItem
{
	Q_OBJECT
  public:
    IconListItem( QListWidget *listbox, const QPixmap &pixmap,
		   const QString &text );
    virtual int height( const QListWidget *lb ) const;
    virtual int width( const QListWidget *lb ) const;
    bool canDrop() const;
    bool dropNeedPath() const;
	int highlighted() const {return mHighlight;};

  public slots:
    void setCanDrop(bool);
    void setDropNeedPath(bool);
    void emitDropSlsk(const QList<QUrl>&);
    void setHighlight(int);
	void setHighlighted(int newH) {mHighlight = newH;};
    void selected();

  signals:
    void dropSlsk(const QList<QUrl>&);

  protected:
    const QPixmap &defaultPixmap();
    void paint( QPainter *painter );

  private:
    QPixmap mPixmap;
    int mHighlight;
    bool mCanDrop, mDropNeedPath;
};

class IconListBox : public QListWidget
{
    Q_OBJECT
  public:
    IconListBox( QWidget *parent=0, const char *name=0, bool align=true );
    void updateMinimumHeight();
    void updateMinimumWidth();

  protected:
    void dragMoveEvent(QDragMoveEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    QStyleOptionViewItem viewOptions () const;

  protected slots:
    void slotCurrentChanged(QListWidgetItem*, QListWidgetItem*);

  private:
    bool mVerticalIconBox;
};

#endif // ICONLISTBOX_H
