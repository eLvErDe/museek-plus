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

#ifndef MARQUEE_H
#define MARQUEE_H

#include <QWidget>

class QPixmap;
class QTimer;
class QMouseEvent;
class QEvent;
class QPaintEvent;

class Marquee : public QWidget {
	Q_OBJECT
public:
	Marquee(const QString& = QString::null, QWidget* = 0, const char* = 0);

	QString text() const;
	void setText(const QString&);

signals:
	void clicked();

protected:
	void paintEvent(QPaintEvent *);
	void updateHeight();
	void paletteChange(const QPalette& oldPalette);
	void fontChange(const QFont& oldFont);
	void styleChange(QStyle& oldStyle);

	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);

protected slots:
	void timerTimeout();

private:
	void invalidate();

	QString mText;
	QTimer* mTimer;
	bool mUpdate;
	int mScroll;
	bool mPressed, mEntered;
};

#endif // MARQUEE_H
