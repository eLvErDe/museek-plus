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

#include "slskdrag.h"

#include <qurl.h>

SlskDrag::SlskDrag(QWidget* w)
         : QUriDrag(w) {
};

void SlskDrag::append(const QString& user, const QString& path, Q_INT64 size) {
	QString tmp = user;
	QUrl::encode(tmp);
	
	QString url = "slsk://";
	
	if(size != 0)
		url += QString::number(size) + "@";
	
	url += tmp;
	
	if(! path.isNull()) {
		QStringList l = QStringList::split("\\", path, true);
		QStringList::const_iterator it = l.begin();
		for(; it != l.end(); ++it) {
			tmp = QString(*it);
			if(! tmp.isEmpty())
				QUrl::encode(tmp);
			url += "/" + tmp;
		}
	}
	
	mUris << url;
	setUnicodeUris(mUris);
}

bool SlskDrag::canDecode(const QMimeSource* e, bool needPath) {
	QStringList l;
	if(! QUriDrag::decodeToUnicodeUris(e, l))
		return false;
	
	QStringList::iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url(*it);
		if(url.protocol() == "slsk" && url.hasHost() && (! needPath || url.hasPath()))
			return true;
	}
	return false;
}

bool SlskDrag::decode(const QMimeSource* e, QStringList& r) {
	QStringList l;
	if(! QUriDrag::decodeToUnicodeUris(e, l))
		return false;
	
	QStringList::iterator it = l.begin();
	for(; it != l.end(); ++it) {
		QUrl url(*it);
		if(url.protocol() == "slsk" && url.hasHost())
			r.append(*it);
	}
	return true;
}

int SlskDrag::count() const {
	return mUris.count();
}
