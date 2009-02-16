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

#include "chatticker.h"

ChatTicker::ChatTicker(QWidget* _p, const char* _n, uint size)
           : Marquee(QString::null, _p, _n), mSize(size) {  };

void ChatTicker::setText(const QString& _u, const QString& _m) {

	if(! _m.isEmpty()) {
		mTickers[_u] = QString(_m).replace("\n", " ");
	} else
		mTickers.remove(_u);
	updateText();
}

void ChatTicker::setText(const NTickers& _t) {

	mTickers.clear();
	NTickers::const_iterator it = _t.begin();
	for(; it != _t.end(); ++it)
	{
		mTickers[it.key()] = QString(it.value()).replace("\n", " ");
	}
	updateText();
}

void ChatTicker::setSize(uint size) {
	mSize = size;
	updateText();
}

void ChatTicker::updateText() {

	QString t;

	NTickers::const_iterator it = mTickers.begin();
	for(; it != mTickers.end(); ++it) {
		QString message = it.value();
		message.truncate(mSize);
		t += "[" + it.key() + "] " + message + " -- ";
	}
	Marquee::setText(t);
}
