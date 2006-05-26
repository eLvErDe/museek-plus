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

#include "images.h"
#include "prefix.h"
#include <qmap.h>
#include <qfile.h>
#include <qdir.h>
#include "museeq.h"


static QMap<QString, QPixmap>* iconcache = 0;

QPixmap& IMG(const QString& icon) {
	if(! iconcache)
		iconcache = new QMap<QString, QPixmap>();
	
	if(iconcache->find(icon) == iconcache->end()) {
		if (museeq->mIconTheme != "default") {
			QDir dir;
			if(dir.cd(museeq->mIconTheme)) {
				QFile f(QString(dir.path() + "/" + icon + ".png"));
				if (f.exists()) {
					QPixmap p = QPixmap(dir.path() + "/" + icon + ".png");
					(*iconcache)[icon] = p;
					return (*iconcache)[icon];
					}
			}
			QString place =  (QString(DATADIR) + "/museek/museeq/" + QString(museeq->mIconTheme) + "/" + icon + ".png");
			QFile ic( place);
			if (ic.exists()) {
				QPixmap p = QPixmap(place);
				(*iconcache)[icon] = p;
				}
			else {
				QPixmap p = QPixmap(QString(DATADIR) + "/museek/museeq/" + icon + ".png");
				(*iconcache)[icon] = p;
				}
			
		} else {
				QPixmap p = QPixmap(QString(DATADIR) + "/museek/museeq/" + icon + ".png");
				(*iconcache)[icon] = p;
		}
	}
	return (*iconcache)[icon];
}
