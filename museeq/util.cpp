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

#include "util.h"
#include "museeq.h"
#include "system.h"

#include <QDropEvent>
#include <QUrl>

QString Util::makeSize(qint64 i) {
	double d = i;
	if(d < 1000)
		return QString(QObject::tr("%1 B")).arg(d, 0, 'f', 2);
	d /= 1024;
	if(d < 1024)
		return QString(QObject::tr("%1 KiB")).arg(d, 0, 'f', 2);
	d /= 1024;
	if(d < 1024)
		return QString(QObject::tr("%1 MiB")).arg(d, 0, 'f', 2);
	d /= 1024;
	if(d < 1024)
		return QString(QObject::tr("%1 GiB")).arg(d, 0, 'f', 2);
	d /= 1024;
	if(d < 1024)
		return QString(QObject::tr("%1 TiB")).arg(d, 0, 'f', 2);
	d /= 1024;
	return QString(QObject::tr("%1 PiB")).arg(d, 0, 'f', 2);
}

int Util::cmp(qint64 a, qint64 b) {
	if(a > b)
		return 1;
	if(a == b)
		return 0;
	return -1;
}

QString Util::makeTime(uint length) {
	if(! length)
		return QString::null;
	else
		if(length < 3600)
			return QString(QObject::tr("%1:%2")).arg(length / 60).arg(length % 60, 2, 'f', 0, '0');
		else
			return QString(QObject::tr("%1:%2:%3")).arg(length / 3600).arg((length / 60) % 60, 2, 'f', 0, '0').arg(length % 60, 2, 'f', 0, '0');
}

QString Util::makeBitrate(uint bitrate, bool vbr) {
	if(! bitrate)
		return QString::null;
	else
		if(vbr)
			return QString(QObject::tr("(vbr) %1")).arg(bitrate);
		else
			return QString::number(bitrate);
}

/**
  * Tells if the given QDropEvent transports some slsk:// urls (with or without path)
  */
bool Util::hasSlskUrls(const QDropEvent* e, bool needPath) {
    // FIXME drag & drop doesn't work with usernames containing '[', ']' or ':' (putting the username as hostname of the url is the problem)
    if (!e->mimeData()->hasUrls())
        return false;

    QList<QUrl> urls = e->mimeData()->urls();

	QList<QUrl>::iterator it = urls.begin();
	for(; it != urls.end(); ++it) {
		QUrl url(*it);
		if(url.scheme() == "slsk" && !url.host().isEmpty() && (! needPath || !url.path().isEmpty()))
			return true;
	}
	return false;
}

/* Returns true if museekd is already running. */
bool Util::getMuseekdLock()
{
# ifdef HAVE_FCNTL_H
  struct flock fl;
  int fdlock;

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = 0;
  fl.l_len = 1;

  if((fdlock = open("/tmp/museekd.lock", O_WRONLY|O_CREAT, 0666)) == -1)
    return true;

  if(fcntl(fdlock, F_SETLK, &fl) == -1)
    return true;

  close(fdlock);

# endif // HAVE_FCNTL_H

  return false;
}
