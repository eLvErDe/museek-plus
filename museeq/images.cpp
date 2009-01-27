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

#include "prefix.h"
#include "museeq.h"
#include "images.h"

#include <QMap>
#include <QFile>
#include <QPixmap>

static QMap<QString, QPixmap> iconcache;

QPixmap& IMG(const QString& icon) {
	if(iconcache.find(icon) == iconcache.end()) {
		// Try the theme directory
		if (museeq->mIconTheme != "default") {
			QString file (QString(museeq->mIconTheme) + "/" + QString(icon) + ".png");
 			if (QFile::exists(file)) {
				QPixmap p = QPixmap(file);
				iconcache[icon] = p;
				return iconcache[icon];
			}
		}
		// Try the DATADIR path
		QString file (QString(DATADIR) + "/museek/museeq/themes/default/" + icon + ".png");

		if (QFile::exists(file)) {
			QPixmap p = QPixmap(file);
			iconcache[icon] = p;
			return iconcache[icon];
		} else {
			// Try /usr/share/
			file = ("/usr/share/museek/museeq/themes/default/" + icon + ".png");

			if (QFile::exists(file)) {
				QPixmap p = QPixmap(file);
				iconcache[icon] = p;
			} else {
				// Give up
				QPixmap p = QPixmap();
				iconcache[icon] = p;
				}
			}


	}
	return iconcache[icon];
}
