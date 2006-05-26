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

#include "marquee.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>

Marquee::Marquee(const QString& _t, QWidget* _p, const char* _n)
        : QWidget(_p, _n), mBuffer(0), mBuffer2(0),
          mPressed(false), mEntered(true) {

	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	mScroll = 0;

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
	QPainter p;

	if(mBuffer && mBuffer->size() != size()) {
		delete mBuffer;
		mBuffer = 0;
	}
	
	if(! mBuffer) {
		mBuffer = new QPixmap(width(), height());
		p.begin(mBuffer);
		p.setBackgroundColor(colorGroup().background());
		p.eraseRect(0, 0, width(), height());
		p.end();
		
		delete mBuffer2;
		mBuffer2 = 0;
		mScroll = 0;
	}
	
	if(!mBuffer2 || (mScroll == 0 && mUpdate)) {
		if(mBuffer2) {
			delete mBuffer2;
			mBuffer2 = 0;
		}

		if(! mText.isEmpty()) {
			mBuffer2 = new QPixmap(fontMetrics().size(Qt::SingleLine, mText).width(), height());
			p.begin(mBuffer2);
			p.setBackgroundColor(colorGroup().background());
			p.setFont(font());
			p.setPen(colorGroup().foreground());
			p.eraseRect(0, 0, mBuffer2->width(), height());
			p.drawText(0, height() - p.fontMetrics().descent() - 2, mText);
			p.end();
		} else {
			mBuffer2 = new QPixmap(1, height());
			p.begin(mBuffer2);
			p.setBackgroundColor(colorGroup().background());
			p.eraseRect(0, 0, 1, height());
			p.end();
		}
		mUpdate = false;
		
	}
	
	bitBlt(mBuffer, 0, 0, mBuffer, 1, 0, width() - 1, height());
	bitBlt(mBuffer, width() - 1, 0, mBuffer2, mScroll, 0, 1, height());
	
	mScroll += 1;
	
	if(mScroll >= mBuffer2->width())
		mScroll = 0;
	
	repaint(false);
}

void Marquee::paintEvent(QPaintEvent*) {
	if(! mBuffer)
		return;

	QPainter p(this);
	p.drawPixmap(0, 0, *mBuffer);
}

void Marquee::updateHeight() {
	if(mText.isEmpty()) {
		setMinimumHeight(0);
		mTimer->stop();
	} else {
		QSize s = QPainter(this).fontMetrics().size(Qt::SingleLine, QString::null);
		setMinimumHeight(s.height() + 2);
		mTimer->start(25, false);
	}
}

void Marquee::invalidate() {
	delete mBuffer;
	delete mBuffer2;
	mBuffer = mBuffer2 = 0;
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
