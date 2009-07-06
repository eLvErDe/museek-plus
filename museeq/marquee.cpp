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

#include "marquee.h"

#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

Marquee::Marquee(const QString& _t, QWidget* _p, const char* _n)
        : QWidget(_p), mPressed(false), mEntered(true) {

	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	mScroll = 0 - width();

	mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), SLOT(timerTimeout()));

	setText(_t);
}

QString Marquee::text() const {
	return mText;
}

void Marquee::setText(const QString& _t) {
	mText = _t;
	mUpdate = true;
	updateHeight();
}

void Marquee::timerTimeout() {
	mUpdate = true;
	update();
}

void Marquee::paintEvent(QPaintEvent*) {
    QPainter p(this);

	if(mUpdate) {
		if(! mText.isEmpty()) {
			int textWidth = fontMetrics().size(Qt::TextSingleLine, mText).width();
			if (textWidth >= 32000)
				textWidth = 31999;
 			p.setBackground(palette().color(QPalette::Background));
			p.setFont(font());
			p.setPen(palette().color(QPalette::Foreground));
			p.drawText(0 - mScroll, height() - fontMetrics().descent() - 2, mText);
			p.end();

            mScroll += 1;

            if(mScroll >= textWidth)
                mScroll = 0 - width();
		} else
			p.end();

		mUpdate = false;
	}
}

void Marquee::updateHeight() {
    QSize s = fontMetrics().size(Qt::TextSingleLine, QString::null);
    setMinimumHeight(s.height() + 2);

	if(mText.isEmpty())
		mTimer->stop();
	else
		mTimer->start(25);
}

void Marquee::invalidate() {
	mUpdate = true;
	updateHeight();
}

void Marquee::paletteChange(const QPalette& oldPalette) {
	invalidate();
}

void Marquee::fontChange(const QFont& oldFont) {
	invalidate();
}

void Marquee::styleChange(QStyle& oldStyle) {
	invalidate();
}

void Marquee::mousePressEvent(QMouseEvent * e) {
	if(e->button() == 1)
		mPressed = true;
}

void Marquee::mouseReleaseEvent(QMouseEvent * e) {
	if(e->button() == 1)
	{
		if(mPressed && mEntered)
			emit clicked();
		mPressed = false;
	}
}

void Marquee::enterEvent(QEvent *) {
	mEntered = true;
}

void Marquee::leaveEvent(QEvent *) {
	mEntered = false;
}
